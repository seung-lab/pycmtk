/*
//
//  Copyright 2016 Google, Inc.
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2012 SRI International
//
//  This file is part of the Computational Morphometry Toolkit.
//
//  http://www.nitrc.org/projects/cmtk/
//
//  The Computational Morphometry Toolkit is free software: you can
//  redistribute it and/or modify it under the terms of the GNU General Public
//  License as published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  The Computational Morphometry Toolkit is distributed in the hope that it
//  will be useful, but WITHOUT ANY WARRANTY; without even the implied
//  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with the Computational Morphometry Toolkit.  If not, see
//  <http://www.gnu.org/licenses/>.
//
//  $Revision$
//
//  $LastChangedDate$
//
//  $LastChangedBy$
//
*/

#include "cmtkSplineWarpGroupwiseRegistrationRMIFunctional.h"

#include <Base/cmtkMathUtil.h>
#include <Base/cmtkMatrix.h>

#include <System/cmtkThreadParameterArray.h>
#include <System/cmtkDebugOutput.h>

#include <algorithm>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

void
SplineWarpGroupwiseRegistrationRMIFunctional
::UpdateInformationByControlPoint()
{
  this->m_NeedsUpdateInformationByControlPoint = false;

  const size_t numberOfControlPoints = this->m_VolumeOfInfluenceArray.size();
  this->m_InformationByControlPoint.resize( numberOfControlPoints );

  const byte paddingValue = this->m_PaddingValue;

  const size_t beginCP = 0;
  const size_t endCP = numberOfControlPoints;

  for ( size_t cp = beginCP; cp < endCP; ++cp ) 
    {
    this->m_InformationByControlPoint[cp] = 0;

    std::vector<DataGrid::RegionType>::const_iterator voi = this->m_VolumeOfInfluenceArray.begin() + cp;
    for ( size_t img = this->m_ActiveImagesFrom; img < this->m_ActiveImagesTo; ++img )
      {
      const byte* dataPtrImg = this->m_Data[img];
      
      byte voiMin = 255, voiMax = 0;
      for ( Types::GridIndexType z = voi->From()[2]; z < voi->To()[2]; ++z ) 
	{
	for ( Types::GridIndexType y = voi->From()[1]; y < voi->To()[1]; ++y )
	  {
	  size_t ofs = this->m_TemplateGrid->GetOffsetFromIndex( voi->From()[0], y, z );
	  for ( Types::GridIndexType x = voi->From()[0]; x < voi->To()[0]; ++x, ++ofs )
	    {
	    const byte data = dataPtrImg[ofs];
	    if ( data != paddingValue )
	      {
	      voiMin = std::min( data, voiMin );
	      voiMax = std::max( data, voiMax );
	      }
	    }
	  }
	}
      this->m_InformationByControlPoint[cp] = std::max( (byte)(voiMax-voiMin), this->m_InformationByControlPoint[cp] );    
      }
    }

  this->UpdateActiveControlPoints();
}
  
void
SplineWarpGroupwiseRegistrationRMIFunctional::UpdateControlPointSchedule()
{
  const SplineWarpXform* xform0 = this->GetXformByIndex(0);
  this->m_ControlPointSchedule.resize( xform0->GetNumberOfControlPoints() );
  this->m_ControlPointScheduleOverlapFreeMaxLength = (xform0->m_Dims[0] / 4) * (xform0->m_Dims[1] / 4) * (xform0->m_Dims[2] / 4);
  
  size_t ofs = 0;
  for ( int z = 0; z < 4; ++z )
    {
    for ( int y = 0; y < 4; ++y )
      {
      for ( int x = 0; x < 4; ++x )
	{
	for ( int k = z; k < xform0->m_Dims[2]; k += 4 )
	  {
	  for ( int j = y; j < xform0->m_Dims[1]; j += 4 )
	    {
	    for ( int i = x; i < xform0->m_Dims[0]; i += 4, ++ofs )
	      {
	      this->m_ControlPointSchedule[ofs] = i + xform0->m_Dims[0] * ( j + xform0->m_Dims[1] * k );
	      }
	    }
	  }
	}
      }
    }
}

void
SplineWarpGroupwiseRegistrationRMIFunctional::UpdateActiveControlPoints()
{
  Superclass::UpdateActiveControlPoints();
  
  if ( this->m_DeactivateUninformativeMode )
    {
    const size_t numberOfControlPoints = this->m_VolumeOfInfluenceArray.size();

    const Vector3D templateFrom( this->m_TemplateGrid->m_Offset );
    const Vector3D templateTo(  this->m_TemplateGrid->m_Offset + this->m_TemplateGrid->m_Size );
    Vector3D fromVOI, toVOI;
    
    std::vector<DataGrid::RegionType>::const_iterator voi = this->m_VolumeOfInfluenceArray.begin();
    for ( size_t cp = 0; cp < numberOfControlPoints; ++cp, ++voi )
      {
      if ( this->m_ActiveControlPointFlags[cp] )
	{
	this->m_ActiveControlPointFlags[cp] = (this->m_InformationByControlPoint[cp] > (this->m_HistogramBins / 4) );
	}

      if ( !this->m_ActiveControlPointFlags[cp] ) 
	--this->m_NumberOfActiveControlPoints;
      }
    
    DebugOutput( 2 ) << "Enabled " << this->m_NumberOfActiveControlPoints << "/" << this->m_ParametersPerXform / 3 << " control points as informative.\n";
    }
  
  this->UpdateParamStepArray();
  this->UpdateControlPointSchedule();
}

SplineWarpGroupwiseRegistrationRMIFunctional::ReturnType
SplineWarpGroupwiseRegistrationRMIFunctional::Evaluate()
{
  return this->Superclass::Evaluate();
}

//@}

} // namespace cmtk

#include "cmtkSplineWarpGroupwiseRegistrationRMIFunctional.txx"
