/*
//
//  Copyright 2010-2012 SRI International
//
//  Copyright 2010 Torsten Rohlfing
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

#ifndef __cmtkRegion_h_included_
#define __cmtkRegion_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkFixedVector.h>

#include <System/cmtkSmartPtr.h>
#include <System/cmtkSmartConstPtr.h>

#include <fstream>

namespace
cmtk
{
/// Class for n-dimensional image index.
template<size_t NDIM,typename T=int>
class Region
{
public:
  /// This class.
  typedef Region<NDIM,T> Self;

  /// The region dimension.
  static const size_t Dimension = NDIM;

  /// The region index variable type.
  typedef T ScalarType;

  /// Index type.
  typedef FixedVector<NDIM,T> IndexType;

  /// Smart pointer to this class.
  typedef SmartPointer<Self> SmartPtr;

  /// Smart pointer-to-const to this class.
  typedef SmartConstPointer<Self> SmartConstPtr;

  /// Default constructor.
  Region() {}

  /// Constructor from two indexes, from and to.
  Region( const typename Self::IndexType& fromIndex /*!< Region goes from this index. */, 
	  const typename Self::IndexType& toIndex /*!< Region goes to this index. */ )
  {
    this->m_RegionFrom = fromIndex;
    this->m_RegionTo = toIndex;
  }
  
  /// Get "from".
  typename Self::IndexType& From()
  {
    return this->m_RegionFrom;
  }

  /// Get const "from".
  const typename Self::IndexType& From() const
  {
    return this->m_RegionFrom;
  }

  /// Get "from".
  typename Self::IndexType& To()
  {
    return this->m_RegionTo;
  }

  /// Get const "from".
  const typename Self::IndexType& To() const
  {
    return this->m_RegionTo;
  }

  /// Compute region size (e.g., number of pixels for grid regions).
  T Size() const
  {
    T size = std::max<T>( 0, (this->m_RegionTo[0]-this->m_RegionFrom[0]) );
    for ( size_t i = 1; i < NDIM; ++i )
      size *= std::max<T>( 0, (this->m_RegionTo[i]-this->m_RegionFrom[i]) );
    return size;
  }

  /// Check if an index is inside region.
  bool IsInside( const typename Self::IndexType idx ) const
  {
    return (this->m_RegionFrom <= idx) && (idx < this->m_RegionTo);
  }
  
  /// Get one face of the region, as a region
  const Self GetFaceRegion( const int dim /*!< Returned face is orthogonal to this index dimension */, const bool upper = false /*!< If true, upper face is returned, otherwise lower face.*/ ) const;
  
private:
  /// Lower limit of the region.
  typename Self::IndexType m_RegionFrom;

  /// Upper limit of the region.
  typename Self::IndexType m_RegionTo;
};

} // namespace cmtk

#include "cmtkRegion.txx"

#endif // #ifndef __cmtkRegion_h_included_
