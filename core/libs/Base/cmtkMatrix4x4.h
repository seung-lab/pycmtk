/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010 SRI International
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

#ifndef __cmtkMatrix4x4_h_included_
#define __cmtkMatrix4x4_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkTypes.h>
#include <Base/cmtkFixedVector.h>
#include <Base/cmtkMatrix3x3.h>

#include <System/cmtkConsole.h>
#include <System/cmtkSmartPtr.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{

/// Homogeneous 4x4 transformation matrix.
template<class T=Types::Coordinate>
class Matrix4x4
{
public:
  /// This type instance.
  typedef Matrix4x4<T> Self;

  /// Smart pointer.
  typedef SmartPointer<Self> SmartPtr;

  /// Identity transformation matrix.
  static const Self IdentityMatrix;

  /** Default constructor: make identity matrix.
   *\note In order to create an uninitialized matrix (for speed), use
   * Matrix4x4<>( NULL ).
   */
  Matrix4x4();

  /// Top left submatrix copy constructor.
  Matrix4x4( const Matrix3x3<T>& other );

  /** Array constructor.
   * If a NULL parameter is given, an uninitialized matrix is generated. This
   * is intended behaviour.
   */
  Matrix4x4( const T *const values ) 
  {
    if ( values ) this->Set( values );
  }

  /// 2D array constructor.
  template<class T2>
  Matrix4x4( const T2 (&matrix)[4][4] ) 
  {
    for ( size_t j = 0; j < 4; ++j )
      for ( size_t i = 0; i < 4; ++i )
	this->Matrix[j][i] = matrix[j][i];
  }
  
  /// Set from array of entries.
  Self& Set( const T *const values );

  /// Inversion operator (in place).
  const Self GetInverse() const;

  /// Transpose operator.
  Self GetTranspose() const;

  /// Index operator.
  T* operator[]( const size_t i ) { return &this->Matrix[i][0]; }

  /// Constant index operator.
  const T* operator[]( const size_t i ) const { return &this->Matrix[i][0]; }

  /// Compose from canonical parameters.
  Self& Compose( const Types::Coordinate params[15], const bool logScaleFactors = false );
  
  /// Decompose into affine parameters.
  bool Decompose( Types::Coordinate params[12], const Types::Coordinate *center = NULL, const bool logScaleFactors = false ) const;

  /// In-place multiplication operator.
  Self& operator*=( const Self& other );
  
  /// Multiplication operator.
  const Self operator*( const Self& other ) const;

  /// 3x3 submatrix assignment operator.
  Self& operator=( const Matrix3x3<T>& other );

  /** Change reference coordinate system.
   */
  Self& ChangeCoordinateSystem( const FixedVector<3,T>& newX, const  FixedVector<3,T>& newY );

  /// Return rotation around x axis.
  static Self RotateX( const T angle );
  
  /// Return rotation around y axis.
  static Self RotateY( const T angle );
  
  /// Return rotation around z axis.
  static Self RotateZ( const T angle );
  
  /// Get Frobenius norm.
  T FrobeniusNorm() const;

  /// Print this matrix.
  void Print( Console& console ) const
  {
    for ( int j = 0; j < 4; ++j )
      {
      for ( int i = 0; i < 4; ++i )
	console.printf( "%f\t", (float)this->Matrix[j][i] );
      console << "\n";
      }
  }

private:
  /// The actual matrix.
  T Matrix[4][4];
};

template<typename T> const Matrix4x4<T> Matrix4x4<T>::IdentityMatrix;

/// In-place multiplication with 3d vector operation (will implicitly be made homogeneous).
template<class T,class T2> 
FixedVector<3,T2>&
operator*=( FixedVector<3,T2>& u, const Matrix4x4<T>& M )
{
  return u = u*M;
}

/// Multiplication with 3d vector operation (will implicitly be made homogeneous).
template<class T,class T2> 
FixedVector<3,T2>
operator*( const FixedVector<3,T2>& u, const Matrix4x4<T>& M )
{
  FixedVector<3,T2> v;
  for ( int idx=0; idx<3; ++idx ) 
    v[idx] = u[0]*M[0][idx] + u[1]*M[1][idx] + u[2]*M[2][idx] + M[3][idx];
  return v;
}

/// Output object to console.
template<class T>
inline
Console& operator<< ( Console& stream, const Matrix4x4<T>& m )
{
  stream << "4x4 Matrix:\n";
  for ( int i = 0; i < 4; ++i ) 
    {
    for ( int j = 0; j < 4; ++j )
      stream << "[" << i << "][" << j << "]=" << m[i][j] << "\t";
    stream << "\n";
    }
  return stream;
}

//@}

} // namespace cmtk

#endif // #ifndef __cmtkMatrix4x4_h_included_
