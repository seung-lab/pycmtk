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

#include "cmtkAffineXformUniformVolume.h"

cmtk::AffineXformUniformVolume::AffineXformUniformVolume( const UniformVolume& volume, const AffineXform& xform )
  : m_VolumeAxesX( volume.m_Dims[0] ),
    m_VolumeAxesY( volume.m_Dims[1] ),
    m_VolumeAxesZ( volume.m_Dims[2] )
{
  // define volume corners
  const UniformVolume::CoordinateVectorType V =  xform.Apply( FixedVectorStaticInitializer<3,Types::Coordinate>::Init(0,0,0) );
  const UniformVolume::CoordinateVectorType dX = xform.Apply( FixedVectorStaticInitializer<3,Types::Coordinate>::Init(1,0,0) ) - V;
  const UniformVolume::CoordinateVectorType dY = xform.Apply( FixedVectorStaticInitializer<3,Types::Coordinate>::Init(0,1,0) ) - V;
  const UniformVolume::CoordinateVectorType dZ = xform.Apply( FixedVectorStaticInitializer<3,Types::Coordinate>::Init(0,0,1) ) - V;
  
  const Types::Coordinate deltaX = volume.m_Delta[0];
  const Types::Coordinate deltaY = volume.m_Delta[1];
  const Types::Coordinate deltaZ = volume.m_Delta[2];

  for ( size_t idx = 0; idx < static_cast<size_t>( volume.m_Dims[0] ); ++idx )
    this->m_VolumeAxesX[idx] = deltaX*idx*dX;
  for ( size_t idx = 0; idx < static_cast<size_t>( volume.m_Dims[1] ); ++idx )
    this->m_VolumeAxesY[idx] = deltaY*idx*dY;
  for ( size_t idx = 0; idx < static_cast<size_t>( volume.m_Dims[2] ); ++idx )
    (this->m_VolumeAxesZ[idx] = deltaZ*idx*dZ) += V;
}

