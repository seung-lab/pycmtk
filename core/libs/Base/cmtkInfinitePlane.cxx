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

#include <cmtkInfinitePlane.h>

#include <cmtkMathUtil.h>

namespace
cmtk
{

/** \addtogroup Base */
//@{

InfinitePlane::InfinitePlane()
{
  Origin.Set( 0, 0, 0 );
  Rho = Theta = Phi = 0;
  this->Update();
}

void InfinitePlane::Update()
{
  Types::Coordinate radTheta = static_cast<Types::Coordinate>( MathUtil::DegToRad( Theta ) );
  Types::Coordinate radPhi = static_cast<Types::Coordinate>( MathUtil::DegToRad( Phi ) );

  Normal.XYZ[0] = cos( radTheta ) * sin( radPhi );
  Normal.XYZ[1] = sin( radTheta ) * sin( radPhi );
  Normal.XYZ[2] = cos( radPhi );

  SquareNormal = Normal * Normal;
}

AffineXform* 
InfinitePlane::GetAlignmentXform( const byte axis ) const
{
  Types::Coordinate angles[3] = { 0, 0, 0 };
  Types::Coordinate xlate[3] = { 0, 0, 0 };
  
  AffineXform *alignment = new AffineXform;

  switch ( axis ) 
    {
    case 0: 
      angles[2] = static_cast<Types::Coordinate>( -MathUtil::RadToDeg( atan2( Normal[1], Normal[0]  ) ) );
      
      // compute z component of normal vector after first rotation; remember
      // that y component will be zero after this rotation.
      Types::Coordinate newNormal0 = MathUtil::Sign( Normal[0] ) * sqrt( 1 - Normal[2]*Normal[2] );
	  angles[1] = static_cast<Types::Coordinate>( -MathUtil::RadToDeg( atan2( Normal[2], newNormal0 ) ) );
      
      alignment->ChangeCenter( this->GetOrigin().XYZ );
      alignment->SetAngles( angles );
      break;
    }
  
  xlate[axis] = Rho;
  alignment->SetXlate( xlate );
  return alignment;
}

} // namespace cmtk
