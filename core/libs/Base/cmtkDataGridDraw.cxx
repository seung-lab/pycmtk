/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//  Copyright 2004-2009 SRI International
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
//  $Revision: 5806 $
//
//  $LastChangedDate: 2009-05-29 13:36:00 -0700 (Fri, 29 May 2009) $
//
//  $LastChangedBy: torsten $
//
*/

#include <cmtkDataGrid.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{

void
DataGrid::DrawSphere
( const Vector3D& center, const Types::Coordinate radius, const Types::DataItem value )
{
  size_t offset = 0;
  for ( int k = 0; k < Dims[2]; ++k )
    {
    for ( int j = 0; j < Dims[1]; ++j )
      {
      for ( int i = 0; i < Dims[0]; ++i, ++offset )
	{
	Vector3D v( i, j, k );
	if ( (v-center).EuclidNorm() <= radius )
	  this->Data->Set( value, offset );
	}
      }
    }
}

void
DataGrid::DrawBox
( const IntROI3D& box, const Types::DataItem value )
{
  for ( int k = box.From[2]; k < box.To[2]; ++k )
    {
    for ( int j = box.From[1]; j < box.To[1]; ++j )
      {
      for ( int i = box.From[0]; i < box.To[0]; ++i )
	{
	this->Data->Set( value, this->GetOffsetFromIndex( i, j, k ) );
	}
      }
    }
}

} // namespace cmtk
