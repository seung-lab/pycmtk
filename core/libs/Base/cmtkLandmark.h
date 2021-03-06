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

#ifndef __cmtkLandmark_h_included_
#define __cmtkLandmark_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkMacros.h>
#include <Base/cmtkTypes.h>
#include <Base/cmtkFixedVector.h>

#include <System/cmtkSmartPtr.h>
#include <System/cmtkSmartConstPtr.h>

#include <string>

namespace
cmtk
{

/** \addtogroup Base */
//@{
/// Coordinates of an (anatomical) landmark.
class Landmark
{
public:
  /// This class.
  typedef Landmark Self;

  /// Space vector type.
  typedef FixedVector<3,Types::Coordinate> SpaceVectorType;

  /// Default constructor.
  Landmark() {};

  /// Explicit constructor.
  Landmark( const std::string& name /*!< Name of this landmark */, const Self::SpaceVectorType& location /*!< Location of this landmark */ );

  /// Name of this landmark.
  std::string m_Name;

  /// Coordinates of this landmark.
  Self::SpaceVectorType m_Location;
};


//@}

} // namespace cmtk

#endif // #ifndef __cmtkLandmarks_h_included_
