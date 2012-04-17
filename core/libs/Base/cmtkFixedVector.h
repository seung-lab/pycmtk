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

#ifndef __cmtkFixedVector_h_included_
#define __cmtkFixedVector_h_included_

#include <cmtkconfig.h>

#include <System/cmtkSmartPtr.h>
#include <System/cmtkSmartConstPtr.h>

#include <math.h>

#include <algorithm>
#include <iostream>

namespace
cmtk
{
/// Class for n-dimensional image index.
template<size_t NDIM,typename T=int>
class FixedVector
{
public:
  /// This class.
  typedef FixedVector<NDIM,T> Self;

  /// Type of the stored values.
  typedef T ValueType;

  /// Smart pointer to this class.
  typedef SmartPointer<Self> SmartPtr;

  /// Smart pointer-to-const to this class.
  typedef SmartConstPointer<Self> SmartConstPtr;

  /// Return vector size.
  size_t Size() const { return NDIM; }

  /// Zero operator.
  static const Self Zero()
  {
    Self v;
    std::fill( v.begin(), v.end(), 0 );
    return v;
  }

  /// Default constructor.
  FixedVector() {}

  /// Initialization class: use this to select initialization constructor.
  class Init
  {
  public:
    /// Constructor: set initialization value.
    explicit Init( const T value ) : m_Value( value ) {}
    
    /// Array initialization value.
    T m_Value;
  };
  
  /// Initialization constructor.
  FixedVector( const typename Self::Init& init )
  {
    std::fill( this->begin(), this->end(), init.m_Value );
  }

  /// Constructor from const pointer.
  template<class T2>
  explicit FixedVector( const T2 *const ptr ) 
  { 
    for ( size_t i = 0; i < NDIM; ++i )
      this->m_Data[i] = ptr[i];
  }

  /// Type conversion constructor template.
  template<class T2>
  FixedVector( const FixedVector<NDIM,T2>& rhs )
  {
    for ( size_t i = 0; i < NDIM; ++i )
      this->m_Data[i] = static_cast<T>( rhs[i] );
  }

  /// Get element reference.
  T& operator[]( const size_t i )
  {
    return this->m_Data[i];
  }

  /// Get const element reference.
  const T& operator[]( const size_t i ) const
  {
    return this->m_Data[i];
  }

  /// In-place addition operator.
  Self& operator+=( const Self& rhs )
  {
    for ( size_t i = 0; i<NDIM; ++i )
      this->m_Data[i] += rhs.m_Data[i];
    return *this;
  }
  
  /// In-place subtraction operator.
  Self& operator-=( const Self& rhs )
  {
    for ( size_t i = 0; i<NDIM; ++i )
      this->m_Data[i] -= rhs.m_Data[i];
    return *this;
  }

  /// Equality operator.
  bool operator==( const Self& rhs ) const
  {
    for ( size_t i = 0; i<NDIM; ++i )
      if ( this->m_Data[i] != rhs.m_Data[i] )
	return false;
    return true;
  }

  /// Inequality operator.
  bool operator!=( const Self& rhs ) const
  {
    return !this->operator==( rhs );
  }

  /// Multiply by a scalar.
  Self& operator*= ( const T a ) 
  {
    for ( size_t i=0; i<NDIM; ++i ) 
      this->m_Data[i] *= a;
    return *this;
  }
  
  /// Elementwise multiplication with another vector.
  Self& operator*=( const Self& rhs )
  {
    for ( size_t i=0; i<NDIM; ++i ) 
      this->m_Data[i] *= rhs[i];
    return *this;
  }

  /// Divide by a scalar.
  Self& operator/= ( const T a ) 
  {
    for ( size_t i=0; i<NDIM; ++i ) 
      this->m_Data[i] /= a;
    return *this;
  }
  
  /// Elementwise dvision with another vector.
  Self& operator/=( const Self& rhs )
  {
    for ( size_t i=0; i<NDIM; ++i ) 
      this->m_Data[i] /= rhs[i];
    return *this;
  }

  /// Unary minus.
  const Self operator-() 
  { 
    Self minus;
    for ( size_t i=0; i<NDIM; ++i ) 
      minus.m_Data = -this->m_Data;

    return minus;
  }

  /// Add scalar value to all vector elements.
  Self& AddScalar( const T& value )
  {
    for ( size_t i=0; i<NDIM; ++i ) 
      this->m_Data[i] += value;
    return *this;
  }
  
  /// Pointer to first array element.
  T* begin()
  {
    return this->m_Data;
  }

  /// Pointer behind last array element.
  T* end()
  {
    return this->m_Data+NDIM;
  }

  /// Pointer to first array element.
  const T* begin() const
  {
    return this->m_Data;
  }

  /// Pointer behind last array element.
  const T* end() const
  {
    return this->m_Data+NDIM;
  }

  /// Maximum value.
  T MaxValue() const
  {
    return *(std::max_element( this->begin(), this->end() ) );
  }

  /// Maximum element index.
  size_t MaxIndex() const
  {
    return std::max_element( this->begin(), this->end() ) - this->m_Data;
  }

  /// Minimum value.
  T MinValue() const
  {
    return *(std::min_element( this->begin(), this->end() ) );
  }

  /// Minimum element index.
  size_t MinIndex() const
  {
    return std::min_element( this->begin(), this->end() ) - this->m_Data;
  }

  /// Calculate sum of vector elements.
  T Sum() const 
  {
    T sum = this->m_Data[0];
    for ( size_t i = 1; i < NDIM; ++i )
      sum += this->m_Data[i];

    return sum;
  }

  /// Calculate product of vector elements.
  T Product() const 
  {
    T product = this->m_Data[0];
    for ( size_t i = 1; i < NDIM; ++i )
      product *= this->m_Data[i];

    return product;
  }

  /// Calculate sum of squares of vector elements (that is, square of Euclid's norm).
  T SumOfSquares() const 
  {
    T sq = 0;
    for ( size_t i = 0; i < NDIM; ++i )
      sq += this->m_Data[i] * this->m_Data[i];

    return sq;
  }

  /// Calculate maximum absolute value in the vector.
  T MaxAbsValue() const 
  {
    T maxAbs = fabs( this->m_Data[0] );
    for ( size_t i = 1; i < NDIM; ++i )
      maxAbs = std::max<T>( maxAbs, fabs( this->m_Data[i] ) );
    
    return maxAbs;
  }

  /// Shorthand for root of sum of squares (i.e., Euclid's norm)
  T RootSumOfSquares() const 
  {
    return sqrt( this->SumOfSquares() );
  }

private:
  /// The actual index array.
  T m_Data[NDIM];
};

/// Addition operator.
template<size_t NDIM,typename T>
const FixedVector<NDIM,T>
operator+( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  return FixedVector<NDIM,T>( lhs ) += rhs;
}

/// Subtraction operator.
template<size_t NDIM,typename T>
const FixedVector<NDIM,T>
operator-( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  return FixedVector<NDIM,T>( lhs ) -= rhs;
}

/// Componentwise division operator.
template<size_t NDIM,typename T>
const FixedVector<NDIM,T>
ComponentDivide( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  return FixedVector<NDIM,T>( lhs ) /= rhs;
}

/// Componentwise multiplication operator.
template<size_t NDIM,typename T>
const FixedVector<NDIM,T>
ComponentMultiply( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  return FixedVector<NDIM,T>( lhs ) *= rhs;
}

/// Scalar product operator.
template<size_t NDIM,typename T>
T
operator* ( const FixedVector<NDIM,T> &lhs, const FixedVector<NDIM,T>& rhs ) 
{
  T product = lhs[0] * rhs[0];
  for ( size_t i = 1; i<NDIM; ++i )
    product += lhs[i] * rhs[i];
  return product;
}

/// Scalar multiplication operator.
template<size_t NDIM,typename T,typename T2>
const FixedVector<NDIM,T>
operator*( const T2 lhs, const FixedVector<NDIM,T>& rhs )
{
  FixedVector<NDIM,T> result( rhs );
  for ( size_t i = 0; i<NDIM; ++i )
    result[i] *= lhs;
  return result;
}

/// Elementwise maximum operator.
template<size_t NDIM,typename T>
const FixedVector<NDIM,T>
Max( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  FixedVector<NDIM,T> result;
  for ( size_t i = 0; i < NDIM; ++i )
    result[i] = std::max( lhs[i], rhs[i] );
  return result;
}

/// Elementwise minimum operator.
template<size_t NDIM,typename T>
const FixedVector<NDIM,T>
Min( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  FixedVector<NDIM,T> result;
  for ( size_t i = 0; i < NDIM; ++i )
    result[i] = std::min( lhs[i], rhs[i] );
  return result;
}

/// Element-wise less-than operator.
template<size_t NDIM,typename T>
bool
operator<( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  for ( size_t i = 0; i < NDIM; ++i )
    if ( ! (lhs[i] < rhs[i] ) )
      return false;
  return true;
}

/// Element-wise less-than-or-equal operator.
template<size_t NDIM,typename T>
bool
operator<=( const FixedVector<NDIM,T>& lhs, const FixedVector<NDIM,T>& rhs )
{
  for ( size_t i = 0; i < NDIM; ++i )
    if ( lhs[i] > rhs[i] )
      return false;
  return true;
}

/// Stream output operator.
template<size_t NDIM,typename T>
std::ostream& operator<<( std::ostream& stream, const FixedVector<NDIM,T>& index )
{
  for ( size_t i = 0; i < NDIM; ++i )
    stream << index[i] << " ";

  return stream;
}

/// Stream input operator.
template<size_t NDIM,typename T>
std::istream& operator>>( std::istream& stream, FixedVector<NDIM,T>& index )
{
  for ( size_t i = 0; i < NDIM; ++i )
    stream >> index[i];
  return stream;
}

/// Fixed vector static initializer class template.
template<size_t NDIM,typename T>
class FixedVectorStaticInitializer
{
};

/// Fixed vector static initializer class template.
template<typename T>
class FixedVectorStaticInitializer<3,T>
{
public:
  /// The vector type.
  typedef FixedVector<3,T> VectorType;

  /// Static initializer for three-dimensional vector.
  static const VectorType Init( const T& x0, const T& x1, const T& x2 )
  {
    VectorType v;

    v[0] = x0;
    v[1] = x1;
    v[2] = x2;
    
    return v;
  }
};

} // namespace cmtk

#endif // #ifndef __cmtkFixedVector_h_included_
