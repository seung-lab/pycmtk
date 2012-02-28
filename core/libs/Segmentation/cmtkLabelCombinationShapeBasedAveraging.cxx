/*
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

#include "cmtkLabelCombinationShapeBasedAveraging.h"

#include <System/cmtkDebugOutput.h>

#include <Base/cmtkUniformDistanceMap.h>
#include <Base/cmtkTypedArray.h>
#include <Base/cmtkTemplateArray.h>

#include <vector>
#include <algorithm>

namespace
cmtk
{

/** \addtogroup Segmentation */
//@{

LabelCombinationShapeBasedAveraging::LabelCombinationShapeBasedAveraging( const std::vector<UniformVolume::SmartConstPtr>& labelImages, const LabelIndexType numberOfLabels )
  : m_NumberOfLabels( numberOfLabels ),
    m_LabelImages( labelImages )
{
  if ( ! this->m_NumberOfLabels )
    {
    this->m_NumberOfLabels = 1;
    for ( size_t k = 0; k < this->m_LabelImages.size(); ++k )
      {
      const Types::DataItemRange range = this->m_LabelImages[k]->GetData()->GetRange();
      this->m_NumberOfLabels = std::max( this->m_NumberOfLabels, static_cast<LabelIndexType>( 1 + range.m_UpperBound ) );
      }
    
    DebugOutput( 9 ) << "Determined number of labels to be " << this->m_NumberOfLabels << "\n";
    }
}

TypedArray::SmartPtr
LabelCombinationShapeBasedAveraging::GetResult() const
{
  const int distanceMapFlags = cmtk::UniformDistanceMap<float>::VALUE_EXACT + cmtk::UniformDistanceMap<float>::SIGNED;

  const size_t numPixels = this->m_LabelImages[0]->GetNumberOfPixels();
  
  std::vector<bool> labelFlags( this->m_NumberOfLabels, false );  
  for ( size_t k = 0; k < this->m_LabelImages.size(); ++k )
    {
    const cmtk::TypedArray& data = *(this->m_LabelImages[k]->GetData());
    
    cmtk::Types::DataItem l;
    for ( size_t i = 0; i < numPixels; ++i )
      {
      if ( data.Get( l, i ) )
	labelFlags[static_cast<unsigned short>( l )] = true;
      }
    }
  
  cmtk::TypedArray::SmartPtr result( cmtk::TypedArray::Create( cmtk::TYPE_USHORT, numPixels ) );
  result->BlockSet( 0 /*value*/, 0 /*idx*/, numPixels /*len*/ );
  unsigned short* resultPtr = static_cast<unsigned short*>( result->GetDataPtr() );
  
  cmtk::FloatArray::SmartPtr totalDistance( new cmtk::FloatArray( numPixels ) );
  float* totalDistancePtr = totalDistance->GetDataPtrTemplate();

  cmtk::FloatArray::SmartPtr inOutDistance( new cmtk::FloatArray(numPixels ) );
  float* inOutDistancePtr = inOutDistance->GetDataPtrTemplate();

  totalDistance->BlockSet( 0 /*value*/, 0 /*idx*/, numPixels /*len*/ );
  for ( int label = 0; label < this->m_NumberOfLabels; ++label )
    {
    /// skip labels that are not in any image.
    if ( ! labelFlags[label] ) continue;

    cmtk::DebugOutput( 1 ) << "Processing label #" << label << "\r";

    inOutDistance->BlockSet( 0 /*value*/, 0 /*idx*/, numPixels /*len*/ );

    for ( size_t k = 0; k < this->m_LabelImages.size(); ++k )
      {
      cmtk::UniformVolume::SmartPtr signedDistanceMap = cmtk::UniformDistanceMap<float>( *(this->m_LabelImages[k]), distanceMapFlags, label ).Get();
      const float* signedDistancePtr = (const float*)signedDistanceMap->GetData()->GetDataPtr();

      // if this is the first label, write directly to accumulation distance map
      if ( !label )
	{
#ifdef CMTK_USE_GCD
    const cmtk::Threads::Stride stride( numPixels );
    dispatch_apply( stride.NBlocks(), dispatch_get_global_queue(0, 0), ^(size_t b)
		    { for ( size_t i = stride.From( b ); i < stride.To( b ); ++i )
#else
#pragma omp parallel for
			for ( int i = 0; i < static_cast<int>( numPixels ); ++i )
#endif
	  {
	  totalDistancePtr[i] += signedDistancePtr[i];
	  }
#ifdef CMTK_USE_GCD
		    });
#endif
	}
      else
	// for all other labels, add to label distance map
	{
#ifdef CMTK_USE_GCD
    const cmtk::Threads::Stride stride( numPixels );
    dispatch_apply( stride.NBlocks(), dispatch_get_global_queue(0, 0), ^(size_t b)
		    { for ( size_t i = stride.From( b ); i < stride.To( b ); ++i )
#else
#pragma omp parallel for
			for ( int i = 0; i < static_cast<int>( numPixels ); ++i )
#endif
	  {
	  inOutDistancePtr[i] += signedDistancePtr[i];
	  }
#ifdef CMTK_USE_GCD
		    });
#endif
	}
      }

    // if this is not the first label, compare this label's sum distance map
    // (over all volumes) pixel by pixel and set this label where it is
    // closer than previous closest label
    if ( label )
      {
#ifdef CMTK_USE_GCD
      const cmtk::Threads::Stride stride( numPixels );
      dispatch_apply( stride.NBlocks(), dispatch_get_global_queue(0, 0), ^(size_t b)
		      { for ( size_t i = stride.From( b ); i < stride.To( b ); ++i )
#else
#pragma omp parallel for
			  for ( int i = 0; i < static_cast<int>( numPixels ); ++i )
#endif
	{
	if ( inOutDistancePtr[i] < totalDistancePtr[i] )
	  {
	  totalDistancePtr[i] = inOutDistancePtr[i];
	  resultPtr[i] = label;
	  }
	else
	  if ( !(inOutDistancePtr[i] > totalDistancePtr[i]) )
	    {
	    resultPtr[i] = this->m_NumberOfLabels;
	    }	  
	}
#ifdef CMTK_USE_GCD
		      });
#endif
      }
    }
  
  return result;
}

} // namespace cmtk