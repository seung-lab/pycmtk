/*
//
//  Copyright 2016 Google, Inc.
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010, 2013 SRI International
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

#include <algorithm>

#include "cmtkInverseInterpolationVolumeReconstructionBase.h"

#include <Base/cmtkHistogramBase.h>
#include <System/cmtkProgress.h>

namespace
cmtk
{

InverseInterpolationVolumeReconstructionBase
::InverseInterpolationVolumeReconstructionBase( const UniformVolume* originalImage, const Types::GridIndexType interleaveFactor, const int interleaveAxis )
  : VolumeInjectionReconstruction( originalImage, interleaveFactor, interleaveAxis ),
    m_RegionalIntensityTruncation( true ),
    m_LowestMaxError( 1e12 ),
    m_FourthOrderError( false ),
    m_ConstraintWeightLNorm( 0.0 ),
    m_MeanSquaredError( 0.0 ),
    m_MaximumError( 0.0 ),
    m_FunctionAndGradient( NULL )
{
}

InverseInterpolationVolumeReconstructionBase
::InverseInterpolationVolumeReconstructionBase( const UniformVolume* reconstructionGrid, std::vector<UniformVolume::SmartPtr>& images )
  : VolumeInjectionReconstruction( reconstructionGrid, images ),
    m_RegionalIntensityTruncation( true ),
    m_LowestMaxError( 1e12 ),
    m_FourthOrderError( false ),
    m_ConstraintWeightLNorm( 0.0 ),
    m_MeanSquaredError( 0.0 ),
    m_MaximumError( 0.0 ),
    m_FunctionAndGradient( NULL )
{
}

double
InverseInterpolationVolumeReconstructionBase
::ComputeApproximationError()
{
  this->m_MeanSquaredError = 0;
  this->m_MaximumError = 0;

  this->m_DifferencePassImages.clear();

  double squaredError = 0;
  size_t totalNumberOfPixels = 0;
  for ( int pass = 0; pass < this->m_NumberOfPasses; ++pass )
    {
    UniformVolume::SmartPtr differencePassImage( this->m_InterpolatedPassImages[pass]->CloneGrid());
    differencePassImage->CreateDataArray( TYPE_FLOAT, true/*setToZero*/ );

    const int numberOfPixels = this->m_InterpolatedPassImages[pass]->GetNumberOfPixels();
    
    for ( int idx = 0; idx < numberOfPixels; ++idx )
      {
      Types::DataItem originalData, interpolatedData;    
      if ( !this->m_OriginalPassImages[pass]->GetDataAt( originalData, idx ) )
	originalData = 0.0;

      if ( this->m_InterpolatedPassImages[pass]->GetDataAt( interpolatedData, idx ) )
	{
	const double difference = interpolatedData - originalData;
	differencePassImage->SetDataAt( difference, idx );
	if ( this->m_FourthOrderError )
	  squaredError += difference*difference*difference*difference;
	else
	  squaredError += difference*difference;

	this->m_MaximumError = std::max<double>( fabs(difference), this->m_MaximumError );
	++totalNumberOfPixels;
	}
      else
	{
	differencePassImage->GetData()->SetPaddingAt( idx );
	}
      }
    
    this->m_DifferencePassImages.push_back( differencePassImage );
    }

  if ( totalNumberOfPixels )
    this->m_MeanSquaredError = squaredError / totalNumberOfPixels;
  else
    this->m_MeanSquaredError = 0.0;

  return this->m_MeanSquaredError;
}

void
InverseInterpolationVolumeReconstructionBase
::Optimize( const int numberOfIterations )
{
  const int numberOfPixels = this->m_CorrectedImage->GetNumberOfPixels();
  ap::real_1d_array x;
  x.setbounds( 1, numberOfPixels );
  for ( int i = 1; i <= numberOfPixels; ++i )
    {
    x(i) = this->m_CorrectedImage->GetDataAt( i-1 );
    }

  const int nbdAll = this->m_RegionalIntensityTruncation ? 2 : 0;
  ap::integer_1d_array nbd;
  nbd.setbounds( 1, numberOfPixels );
  for ( int i = 1; i <= numberOfPixels; ++i )
    {
    nbd(i) = nbdAll;
    if ( this->m_NeighorhoodMinPixelValues(i) > this->m_NeighorhoodMaxPixelValues(i) )
      {
      this->m_NeighorhoodMinPixelValues(i) = this->m_OriginalImageRange.m_LowerBound;
      this->m_NeighorhoodMaxPixelValues(i) = this->m_OriginalImageRange.m_UpperBound;
      }
    }

  Progress::Begin( 0, numberOfIterations, 1, "Inverse Interpolation" );
  
  int info;
  ap::lbfgsbminimize( this->m_FunctionAndGradient, numberOfPixels, 5, x, 1e-10 /*epsg*/, 1e-10 /*epsf*/, 1e-10 /*epsx*/, numberOfIterations, 
		      nbd, this->m_NeighorhoodMinPixelValues, this->m_NeighorhoodMaxPixelValues, info );

  Progress::Done();
  
  if ( info < 0 )
    StdErr << "ERROR: lbfgsbminimize returned status code " << info << "\n";
  else
    for ( int i = 1; i <= numberOfPixels; ++i )
      this->m_CorrectedImage->SetDataAt( x(i), i-1 );
}

} // namespace cmtk

