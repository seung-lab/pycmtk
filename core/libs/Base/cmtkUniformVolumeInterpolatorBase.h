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

#ifndef __cmtkUniformVolumeInterpolatorBase_h_included_
#define __cmtkUniformVolumeInterpolatorBase_h_included_

#include <cmtkconfig.h>

#include <System/cmtkSmartPtr.h>
#include <System/cmtkSmartConstPtr.h>

#include <Base/cmtkVector3D.h>
#include <Base/cmtkUniformVolume.h>
#include <Base/cmtkInterpolator.h>

#include <vector>

namespace
cmtk
{

/** \addtogroup Base */
//@{
/** Base class for kernel-based uniform volume.
 */
class UniformVolumeInterpolatorBase
{
public:
  /// This class type.
  typedef UniformVolumeInterpolatorBase Self;
  
  /// Smart pointer to this class.
  typedef SmartPointer<Self> SmartPtr;

  /// Smart pointer-to-const to this class.
  typedef SmartConstPointer<Self> SmartConstPtr;

  /// Constructor.
  UniformVolumeInterpolatorBase( const UniformVolume& volume )
  {
    this->SetVolume( volume );
  }

  /// Virtual dummy destructor.
  virtual ~UniformVolumeInterpolatorBase() {};

  /** Set volume.
   * This function sets a smart pointer to the volume the class will interpolate
   * from. It may also perform some pre-computations to speed up interpolation,
   * such as indexing etc. It does not perform any interpolation itself.
   */
  virtual void SetVolume( const UniformVolume& volume /*!< Source volume for interpolation */ );
  
  /** Get data at location.
   *
   * This function performs interpolation of one value from m_Volume at location
   * v using the interpolation function given as the class template parameter.
   *
   * This function should return true if a value can be interpolated from
   * m_Volume at v, and it should return false if v is outside the range
   * where a value can be interpolated (i.e., outside the volume boundaries).
   *
   *\return True is the interpolation was successful and a valid value is returned in "value".
   */
  virtual bool GetDataAt( const Vector3D& v /*!< Location for interpolation.*/, Types::DataItem& value /*!< The interpolated value is stored via this reference*/ ) const = 0;

  /** Get data at a pre-computed relative pixel index. This is faster if we already know the pixel index and fractional coordinate of a location.
   *\return Interpolated value.
   */
  virtual Types::DataItem GetDataDirect( const Types::GridIndexType* imageGridPoint /*!< Grid index in image */, const Types::Coordinate* insidePixel /*!< Relative position inside indexed pixel */ ) const = 0;

protected:
  /// Pointer to volume data array.
  std::vector<Types::DataItem> m_VolumeDataArray;

  /// Image dimensions.
  DataGrid::IndexType m_VolumeDims;
  
  /// Image pixel size.
  UniformVolume::CoordinateVectorType m_VolumeDeltas;

  /// Image offset vector.
  UniformVolume::CoordinateVectorType m_VolumeOffset;

  /// Index increment when increasing "j" pixel index (i.e., moving to next image row).
  Types::GridIndexType m_NextJ;

  /// Index increment when increasing "k" pixel index (i.e., moving to next image plane).
  Types::GridIndexType m_NextK;

  /// Get offset from pixel index.
  size_t GetOffsetFromIndex( const Types::GridIndexType i /*!< Grid index #0 */, const Types::GridIndexType j /*!< Grid index #1 */, const Types::GridIndexType k /*!< Grid index #2 */ ) const
  {
    return i + j * this->m_NextJ + k * this->m_NextK;
  }
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkUniformVolumeInterpolatorBase_h_included_

