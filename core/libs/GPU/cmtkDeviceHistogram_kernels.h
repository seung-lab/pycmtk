/*
//
//  Copyright 2010 SRI International
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

#ifndef __cmtkDeviceHistogram_kernels_h_included_
#define __cmtkDeviceHistogram_kernels_h_included_

#include <cmtkconfig.h>

/** \addtogroup GPU */
//@{

/// Populate histogram from data, entirely on device.
void cmtkDeviceHistogramPopulate( float* histPtr, /*!< Device pointer to histogram data (output). */
				  const float* dataPtr, /*!< Device pointer to input image data. */
				  const float rangeFrom, /*!< Histogram range from this value. */
				  const float rangeTo, /*!< Histogram range to this value. */
				  const bool logScale, /*!< Use log scale for value-to-bin mapping to make entropy scale-invariant. */
				  const int numberOfBins, /*!< Number of histogram bins. */
				  const int numberOfSamples /*!< Number of data samples */ );

/// Populate histogram from data using binary mask, entirely on device.
void cmtkDeviceHistogramPopulate( float* histPtr, /*!< Device pointer to histogram data (output). */
				  const float* dataPtr, /*!< Device pointer to input image data. */
				  const int* maskPtr, /*!< Device pointer to binary mask data (0=background, 1=foreground). */ 
				  const float rangeFrom, /*!< Histogram range from this value. */
				  const float rangeTo, /*!< Histogram range to this value. */
				  const bool logScale, /*!< Use log scale for value-to-bin mapping to make entropy scale-invariant. */
				  const int numberOfBins, /*!< Number of histogram bins. */
				  const int numberOfSamples /*!< Number of data samples */ );

/// Compute entropy from histogram on device.
void cmtkDeviceHistogramEntropy( float* result, const float* dataPtr, int numberOfBins );

//@}

#endif // #ifndef __cmtkDeviceHistogram_kernels_h_included_
