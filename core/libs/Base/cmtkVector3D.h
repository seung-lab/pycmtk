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

#ifndef __cmtkVector3D_h_included_
#define __cmtkVector3D_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkTypes.h>
#include <Base/cmtkFixedVector.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{

/// Convenience typedef: vectors in 3D coordinate space.
typedef FixedVector<3,Types::Coordinate> Vector3D;

/// Cross-product of two 3D vectors.
template<class T>
FixedVector<3,T> 
CrossProduct( const FixedVector<3,T>& u, const FixedVector<3,T>& v )
{
  const T result[3] = { u[1]*v[2] - u[2]*v[1], u[2]*v[0] - u[0]*v[2], u[0]*v[1] - u[1]*v[0] };
  return FixedVector<3,T>::FromPointer( result );
}

//@}

} // namespace cmtk

#endif // #ifdef __cmtkVector3D_h_included_
