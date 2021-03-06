/*
//
//  Copyright 2012 SRI International
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

#ifndef __cmtkSphereDetectionBipolarMatchedFilterFFT_h_included_
#define __cmtkSphereDetectionBipolarMatchedFilterFFT_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkUniformVolume.h>

#include <System/cmtkFFTW.h>

namespace
cmtk
{

/** Detect spheres in an image using FFT-based matched bipolar filter.
 * An object of this class can be used to detect spheres of different sizes with only a single FFT applied to the test image (each sphere size 
 * does require a different filter kernel and thus a repeated FFT of the kernel).
 *
 * The filter kernel is bipolar, i.e., +1 inside the sphere and -1 outside the sphere, each within a user-provided margin inside and outside the
 * sphere surface. This makes the filter robust to intensity differences across the images, although probably not to the same degree as a truely
 * self-normalizing filter (e.g., Padberg's FFT-based NCC).
 *\note This class requires CMTK to be configured with FFTW3 support ("CMTK_USE_FFTW" CMake option).
 *\todo The current implementation does not take advantage of the real-valued image and filter data, which could be used to reduce the storage
 * requirement of the FT data (and probably the computational cost of the transform) by almost 50%. On the other hand, capitalizing on these
 * savings would either require out-of-place, rather than in-place, transforms, or substantially complicate memory layout of the input data.
 */
class SphereDetectionBipolarMatchedFilterFFT
{
public:
  /// Constructor: initialize FFTW plans and compute image FT.
  SphereDetectionBipolarMatchedFilterFFT( const UniformVolume& image );

  /// Destructor: destroy FFTW plans and de-allocate transformed image memories.
  virtual ~SphereDetectionBipolarMatchedFilterFFT();

  /// Get image filtered with spherical matched filter kernel.
  cmtk::TypedArray::SmartPtr GetFilteredImageData( const Types::Coordinate sphereRadius /*!< Radius of detected spheres in world coordinate units (e.g., mm) */, 
						   const int marginWidth = 1 /*!< Half width of the filter margin in pixels: positive filter coefficients in a band of this width inside radius, negative coeffiecients outside radius.*/ );

private:
  /// Image number of pixels.
  size_t m_NumberOfPixels;

  /// Image dimensions.
  DataGrid::IndexType m_ImageDims;

  /// Image pixel size.
  UniformVolume::SpaceVectorType m_PixelSize;

  /// The Fourier-transformed image.
  fftw_complex* m_ImageFT;

  /// The Fourier-transformed matched filter.
  fftw_complex* m_FilterFT;

  /// The filter FFT plan.
  fftw_plan m_PlanFilter;

  /// The backward (filtered data to space domain) FFT plan.
  fftw_plan m_PlanBackward;

  /** Make the filter kernel.
   *\return Number of non-zero elements in the filter kernel.
   */
  size_t MakeFilter( const Types::Coordinate sphereRadius, const int marginWidth );
};

} // namespace cmtk

#endif // #ifndef __cmtkSphereDetectionBipolarMatchedFilterFFT_h_included_
