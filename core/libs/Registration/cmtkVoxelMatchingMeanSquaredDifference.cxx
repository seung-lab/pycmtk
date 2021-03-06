/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010, 2013 SRI International
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

#include "cmtkVoxelMatchingMeanSquaredDifference.h"

namespace
cmtk
{

/** \addtogroup Registration */
//@{

VoxelMatchingMeanSquaredDifference::VoxelMatchingMeanSquaredDifference
( const UniformVolume* refVolume, const UniformVolume* fltVolume ) :
  VoxelMatchingMetricShort( refVolume, fltVolume ),
  Sum( 0.0 ),
  Samples( 0 )
{}

} // namespace cmtk
