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

#include <cmtkXformListEntry.h>

cmtk::XformListEntry::XformListEntry
( Xform::SmartPtr& xform, const bool inverse, const Types::Coordinate globalScale )
  : Xform( xform ), 
    InverseAffineXform( NULL ), 
    m_WarpXform( NULL ),
    Inverse( inverse ), 
    GlobalScale( globalScale )
{
  if ( this->Xform ) 
    {
    this->m_WarpXform = dynamic_cast<WarpXform*>( this->Xform.GetPtr() );
    
    AffineXform::SmartPtr affineXform( AffineXform::SmartPtr::DynamicCastFrom( this->Xform ) );
    if ( affineXform ) 
      {
      this->InverseAffineXform = affineXform->MakeInverse();
      }
    }
}

cmtk::XformListEntry::~XformListEntry()
{
  // we got the inverse affine from AffineXform::MakeInverse, so we
  // need to get rid of it explicitly.
  if ( this->InverseAffineXform )
    delete this->InverseAffineXform;
}
