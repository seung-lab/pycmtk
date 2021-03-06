/*
//
//  Copyright 2009-2010 SRI International
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

#include "cmtkImageOperationRevert.h"

cmtk::UniformVolume::SmartPtr
cmtk::ImageOperationRevert::Apply( cmtk::UniformVolume::SmartPtr& volume )
{
  const size_t nPixels = volume->GetNumberOfPixels();
  for ( size_t n = 0; n < nPixels; ++n )
    {
    if ( volume->GetDataAt( n ) )
      volume->SetDataAt( 0, n );
    else
      volume->SetDataAt( 1, n );
    }
  
  return volume;
}
