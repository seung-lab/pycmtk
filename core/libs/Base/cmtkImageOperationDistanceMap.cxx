/*
//
//  Copyright 2009-2012 SRI International
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

#include "cmtkImageOperationDistanceMap.h"

cmtk::UniformVolume::SmartPtr  
cmtk::ImageOperationDistanceMap
::Apply( cmtk::UniformVolume::SmartPtr& volume )
{
  if ( this->m_SignedDistance )
    {
    UniformVolume::SmartPtr iMap = DistanceMapType( *volume, DistanceMap::INSIDE ).Get();
    UniformVolume::SmartPtr oMap = DistanceMapType( *volume ).Get();
    
    const size_t nPixels = volume->GetNumberOfPixels();
#pragma omp parallel for
    for ( int n = 0; n < static_cast<int>( nPixels ); ++n )
      {
      Types::DataItem iValue = iMap->GetDataAt( n );
      if ( iValue > 0 )
	oMap->SetDataAt( -iValue, n );
      }
    return oMap;
    }
  else
    {
    return DistanceMapType( *volume ).Get();
    }
}
