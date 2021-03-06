/*
//
//  Copyright 2016 Google, Inc.
//
//  Copyright 2004-2013 SRI International
//
//  Copyright 1997-2010 Torsten Rohlfing
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

#ifndef __cmtkDataGrid_h_included_
#define __cmtkDataGrid_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkMacros.h>
#include <Base/cmtkTypes.h>
#include <Base/cmtkTypedArray.h>
#include <Base/cmtkFixedVector.h>
#include <Base/cmtkScalarImage.h>
#include <Base/cmtkRegion.h>
#include <Base/cmtkMetaInformationObject.h>
#include <Base/cmtkAnatomicalOrientation.h>

#include <System/cmtkSmartPtr.h>
#include <System/cmtkThreads.h>

#include <vector>

namespace
cmtk
{

/** \addtogroup Base */
//@{

/** Grid topology of data arranged in a 3D lattice.
 * This class extends the plain data handling functions of TypedArray
 * with a 3D topology. Real world coordinates, however, are not considered and
 * need to be handled by derived classes. Thus, this class provides the coordinate
 * independent services such as median filtering and, to a certain extent,
 * interpolation.
 */
class DataGrid :
  /// Inherit class that handles meta information.
  public MetaInformationObject
{
public:
  /// This class.
  typedef DataGrid Self;

  /// Smart pointer to DataGrid.
  typedef SmartPointer<Self> SmartPtr;

  /// Smart pointer-to-const to DataGrid.
  typedef SmartConstPointer<Self> SmartConstPtr;

  /// Region type.
  typedef Region<3,Types::GridIndexType> RegionType;

  /// Index type.
  typedef RegionType::IndexType IndexType;

  /// Space vector type.
  typedef FixedVector<3,Types::Coordinate> SpaceVectorType;

  /// Number of grid samples in the three spatial dimensions
  Self::IndexType m_Dims;

  /// Offset increments when moving to the next pixel in each of the three grid dimensions.
  Self::IndexType m_GridIncrements;

  /// Data array (element type is variable)
  cmtkGetSetMacro(TypedArray::SmartPtr,Data);

public:
  /// Copy constructor.
  DataGrid( const Self& other );
  
  /// Constructor.
  DataGrid( const Self::IndexType& dims, TypedArray::SmartPtr& data = TypedArray::SmartPtr::Null() ) 
    : m_Dims( dims ), 
      m_Data( data )
  {
    this->ComputeGridIncrements();
    this->m_CropRegion = this->GetWholeImageRegion();
  }
  
  /// Virtual destructor.
  virtual ~DataGrid() {}

  /** Create a physical copy of this object.
   *\param copyData If true, the associated data array is also copied.
   */
  Self::SmartPtr Clone( const bool copyData )
  {
    return Self::SmartPtr( this->CloneVirtual( copyData ) );
  }

  /** Create a physical copy of this object.
   */
  Self::SmartPtr Clone() const
  {
    return Self::SmartPtr( this->CloneVirtual() );
  }

  /// Test whether this grid matches another one, i.e., has the same dimensions.
  bool GridMatches( const Self& other ) const
  {
    return (this->m_Dims == other.m_Dims);
  }

  /// Downsampling and pixel-averaging constructor function.
  virtual DataGrid* GetDownsampledAndAveraged( const Types::GridIndexType (&downsample)[3] ) const;

  /// Downsampling without averaging constructor function.
  virtual DataGrid* GetDownsampled( const Types::GridIndexType (&downsample)[3] ) const;

  /** Reorientation constructor function.
   *\param newOrientation Three letter orientation code that specifies the anatomically-based
   * orientation of the reoriented volume. Each letter can be one of the following: R, L, A, 
   * P, I, S. These stand for Right, Left, Anterior, Posterior, Inferior, Superior. 
   *
   * The three letters in the orientation string define the directions of the positive x, y, 
   * and z axes, in this order. For example, "RAS", the standard orientation for this software, 
   * means that the pixels along the x axis are arranged from the subject's Left to the Right 
   * side, along the y axis from the subject's Posterior (back) to Anterior (front), and along
   * the z axis from Inferior (feet) to Superior (head).
   *
   * The current orientation of this volume is to be taken from its meta information,
   * as this->m_MetaInformation[CMTK_META_IMAGE_ORIENTATION]. This is also a three-letter string of the
   * same form as the one given to this function as a parameter.
   *
   * If the current orientation is not set, a warning message should be printed to StdErr, and
   * a NULL pointer returned.
   *
   *\return Reoriented data grid with permuted pixels in this->Data and permuted grid dimensions
   * in this->Dims. The returned pointers points to a newly allocated object, which can be
   * wrapped in an SmartPointer.
   */
  const DataGrid::SmartPtr GetReoriented( const char* newOrientation = AnatomicalOrientation::ORIENTATION_STANDARD ) const;
  
  /// Get dimensions array.
  const Self::IndexType GetDims() const
  {
    return this->m_Dims;
  }

  /** Create data array.
   *\param dataType ID of scalar data type for the array. This is the image pixel type.
   *\param setToZero If this flag is set, all values in the newly created array will be initialized to zero.
   */
  virtual TypedArray::SmartPtr CreateDataArray( const ScalarDataType dataType, const bool setToZero = false );

  /// Get number of data items in the volume.
  size_t GetNumberOfPixels () const { return this->m_Dims[0]*this->m_Dims[1]*this->m_Dims[2]; }

  /// Check whether given pixel index is inside grid.
  bool IndexIsInRange( const Types::GridIndexType x, const Types::GridIndexType y, const Types::GridIndexType z ) const
  {
    return (x>=0) && (x<this->m_Dims[0]) && (y>=0) && (y<this->m_Dims[1]) && (z>=0) && (z<this->m_Dims[2]);
  }

  /// Get offset of a pixel.
  Types::GridIndexType GetOffsetFromIndex( const Types::GridIndexType x, const Types::GridIndexType y, const Types::GridIndexType z ) const 
  {
    return x + nextJ * y + nextK * z;
  }

  /// Get offset of a pixel.
  Types::GridIndexType GetOffsetFromIndex( const Self::IndexType& index ) const 
  {
    return index[0] + this->nextJ * index[1] + this->nextK * index[2];
  }

  /// Get index of a pixel identified by its offset.
  void GetIndexFromOffset( const size_t offset, Types::GridIndexType& x, Types::GridIndexType& y, Types::GridIndexType& z ) const 
  {
    z = offset / nextK;
    y = (offset % nextK) / nextJ;
    x = (offset % nextK) % nextJ;
  }

  /// Get index of a pixel identified by its offset.
  Self::IndexType GetIndexFromOffset( const Types::GridIndexType offset ) const 
  {
    Self::IndexType index;
    index[2] = offset / nextK;
    index[1] = (offset % nextK) / nextJ;
    index[0] = (offset % nextK) % nextJ;
    return index;
  }

  /// Return data at specified offset
  bool GetDataAt ( Types::DataItem& data, const size_t offset ) const 
  {
    return this->m_Data->Get( data, offset );
  }

  /// Return data at specified grid point.
  bool GetDataAt ( Types::DataItem& data, const Types::GridIndexType x, const Types::GridIndexType y, const Types::GridIndexType z ) const
  {
    return this->m_Data->Get( data, this->GetOffsetFromIndex( x, y, z ) );
  }

  /// Set data at specified offset
  void SetDataAt ( const Types::DataItem data, const size_t offset )
  {
    this->m_Data->Set( data, offset );
  }
  
  /// Set data at specified grid point.
  void SetDataAt ( const Types::DataItem data, const Types::GridIndexType x, const Types::GridIndexType y, const Types::GridIndexType z )
  {
    this->SetDataAt( data, this->GetOffsetFromIndex( x, y, z ) );
  }

  /// Return data at specified grid point, or a given default value if no data exists there.
  Types::DataItem GetDataAt ( const Types::GridIndexType x, const Types::GridIndexType y, const Types::GridIndexType z, const Types::DataItem defaultValue = 0.0 ) const
  {
    Types::DataItem value;
    if ( this->GetDataAt( value, this->GetOffsetFromIndex( x, y, z ) ) )
      return value;
    else
      return defaultValue;
  }

  /// Return data at specified grid offset, or a given default value if no data exists there.
  Types::DataItem GetDataAt ( const size_t offset, const Types::DataItem defaultValue = 0.0 ) const
  {
    return this->m_Data->ValueAt( offset, defaultValue );
  }

  /** Return data after mirroring.
   *\param axis Coordinate axis normal to mirror plane. Default is AXIS_X
   * (mirror about mid-sagittal plane).
   */
  TypedArray::SmartPtr GetDataMirrorPlane( const int axis = AXIS_X ) const;

  /// Replace data with mirrored version.
  void ApplyMirrorPlane( const int axis = AXIS_X );

public:
  /** Get cropped region reference.
   */
  Self::RegionType& CropRegion()
  {
    return this->m_CropRegion;
  }

  /** Set cropped region.
   * This function can deal with negative crop region values, which refer to the upper grid
   * boundary and are automatically converted.
   */
  virtual void SetCropRegion( const Self::RegionType& region );

  /** Get cropped region reference.
   */
  const Self::RegionType& CropRegion() const
  {
    return this->m_CropRegion;
  }

  /// Get whole image region.
  const Self::RegionType GetWholeImageRegion() const;

  /// Get region covering one slice of the image.
  const Self::RegionType GetSliceRegion( const int axis /*!< Coordinate axis perpendicular to the slice, i.e., 0 for x, 1 for y, 2 for z.*/, const Types::GridIndexType slice /*!< Index of selected slice. */ ) const;

  /// Get index increments for crop region.
  const Self::IndexType GetCropRegionIncrements() const;

  /** Automatically crop to minimal region above given threshold.
   *\param threshold The cropping threshold.
   *\param recrop If this flag is true, then the cropping will be performed
   * inside an already existing cropping region. If this flag is false 
   * (default), then any pre-set crop region is ignored.
   *\param margin Width of additional margin added around the threshold-cropped region.
   *\return The crop region that was applied.
   */
  Self::RegionType AutoCrop( const Types::DataItem threshold, const bool recrop = false, const Types::GridIndexType margin = 0 );

  /// Fill volume outside current crop region with constant value.
  void FillCropBackground( const Types::DataItem value );

  /// Return data for a region of the volume.
  TypedArray::SmartPtr GetRegionData( const Self::RegionType& region ) const;

  /// Accessor functions for protected member variables
  Types::GridIndexType GetNextI() const { return nextI; }
  Types::GridIndexType GetNextJ() const { return nextJ; }
  Types::GridIndexType GetNextK() const { return nextK; }
  Types::GridIndexType GetNextIJ() const { return nextIJ; }
  Types::GridIndexType GetNextIK() const { return nextIK; }
  Types::GridIndexType GetNextJK() const { return nextJK; }
  Types::GridIndexType GetNextIJK() const { return nextIJK; }
  
  /// Get center of mass of pixel data.
  virtual FixedVector<3,Types::Coordinate> GetCenterOfMassGrid() const;
  
  /// Get center of mass and first-order moments of pixel data.
  virtual FixedVector<3,Types::Coordinate> GetCenterOfMassGrid( FixedVector<3,Types::Coordinate>& firstOrderMoment ) const;
  
  /** Return orthogonal slice as a 2D image.
   */
  virtual ScalarImage::SmartPtr GetOrthoSlice( const int axis, const Types::GridIndexType plane ) const;
  
  /** Extract orthogonal slice as a data grid object.
   */
  Self::SmartPtr ExtractSlice( const int axis /*!< Coordinate axis perpendicular to extracted plane*/, const Types::GridIndexType plane /*!< Index of extracted plane */ ) const;

  /** Set orthogonal slice from a 2D image.
   */
  virtual void SetOrthoSlice( const int axis, const Types::GridIndexType idx, const ScalarImage* slice );

  /// Print object.
  void Print() const;

protected:
  /** Create a physical copy of this object.
   *\param copyData If true, the associated data array is also copied.
   */
  virtual Self* CloneVirtual( const bool copyData );
  virtual Self* CloneVirtual() const;

  /** Utility function for trilinear interpolation.
   *\param data This reference is set to the interpolated data value. It is 
   * valid if and only if this function returns 1.
   *\param location 3D coordinate to interpolate data at.
   *\param x Grid index x.
   *\param y Grid index y.
   *\param z Grid index z.
   *\param location Location within grid cell.
   *\param cellFrom 3D coordinate of the lower-left-front voxel of the cell
   * enclosing the given location.
   *\param cellTo 3D coordinate of the upper-right-rear voxel of the cell
   * enclosing the given location.
   *\return True if there is valid data for all eight voxels enclosing the 
   * given location, so that the interpolation could be completed successfully,
   * False otherwise.
   */
  bool TrilinearInterpolation( Types::DataItem& data, const Types::GridIndexType x, const Types::GridIndexType y, const Types::GridIndexType z, const Self::SpaceVectorType& location, const Types::Coordinate* cellFrom, const Types::Coordinate* cellTo ) const;

  /** Utility function for trilinear interpolation from a primitive data array.
   * This function is provided for computational efficiency when a large number 
   * of interpolations from a given data volume of known pixel type are required.
   *
   *\param dataPtr Pointer to the primitive data array.
   *\param x Grid index x.
   *\param y Grid index y.
   *\param z Grid index z.
   *\param gridPosition (x,y,z) indices of the voxel containing the given
   * location.
   *\param cellFrom 3D coordinate of the lower-left-front voxel of the cell
   * enclosing the given location.
   *\param cellTo 3D coordinate of the upper-right-rear voxel of the cell
   * enclosing the given location.
   *\return The interpolated data value..
   */
  template<class TData>
  inline TData TrilinearInterpolation
  ( const TData* dataPtr, const Types::GridIndexType x, const Types::GridIndexType y, const Types::GridIndexType z, const Self::SpaceVectorType& gridPosition, const Types::Coordinate* cellFrom, const Types::Coordinate* cellTo ) const;

  /** Utility function for trilinear interpolation from multiple primitive data arrays of identical grid structure.
   * This function is provided for computational efficiency when a large number 
   * of interpolations from a given data volume of known pixel type are required.
   */
  template<class TData,class TOutputIterator>
  inline void TrilinearInterpolation
  ( TOutputIterator result /*!< Output iterator to store interpolated values.*/,
    const std::vector<TData*>& dataPtr /*!< Vector of data arrays to interpolate from */, 
    const Types::GridIndexType x /*!< Grid position x */, 
    const Types::GridIndexType y /*!< Grid position y */, 
    const Types::GridIndexType z /*!< Grid position z */,
    const Types::Coordinate fracX /*!< Fractional coordinate X within pixel */, 
    const Types::Coordinate fracY /*!< Fractional coordinate Y within pixel */, 
    const Types::Coordinate fracZ /*!< Fractional coordinate Z within pixel */ ) const;

  /// Offset to next voxel column.
  Types::GridIndexType nextI;

  /// Offset to next voxel row.
  Types::GridIndexType nextJ;

  /// Offset to next voxel plane.
  Types::GridIndexType nextK;

  /// Offset to next column and row.
  Types::GridIndexType nextIJ;
  
  /// Offset to next column and plane.
  Types::GridIndexType nextIK;

  /// Offset to next row and plane.
  Types::GridIndexType nextJK;

  /// Offset to next column, row, and plane.
  Types::GridIndexType nextIJK;

private:
  /** Crop region.
   */
  Self::RegionType m_CropRegion;

  /** Compute grid increments for fast n-dimensional offset computations.
   */
  virtual void ComputeGridIncrements();

  /// Mirror about plane without allocating additional memory.
  static void MirrorPlaneInPlace( TypedArray& data, const Self::IndexType& dims, const int axis = AXIS_X );

};

//@}

} // namespace cmtk

#include "cmtkDataGrid.txx"

#endif // #ifndef __cmtkDataGrid_h_included_
