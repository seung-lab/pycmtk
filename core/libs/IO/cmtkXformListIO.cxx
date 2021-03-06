/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010, 2013-2014 SRI International
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

#include "cmtkXformListIO.h"

#include <IO/cmtkXformIO.h>

#include <System/cmtkExitException.h>

#include <Base/cmtkPolynomial.h>

cmtk::XformList
cmtk::XformListIO::MakeFromStringList( const std::vector<std::string>& stringList )
{
  XformList xformList;
  for ( std::vector<std::string>::const_iterator it = stringList.begin(); it != stringList.end(); ++it )
    {
    const bool inverse = (*it == "-i" ) || (*it == "--inverse" );
    if ( inverse ) 
      {
      ++it;
      if ( it == stringList.end() )
	{
	StdErr << "ERROR: '--inverse' / '-i' must be followed by at least one more transformation\n";
	throw ExitException( 1 );
	}
      }
    
    try
      {
      Xform::SmartPtr xform( XformIO::Read( it->c_str() ) );
      if ( ! xform ) 
	{
	StdErr << "ERROR: could not read target-to-reference transformation from " << *it << "\n";
	throw ExitException( 1 );
	}

      xformList.Add( xform, inverse );
      }
    catch ( const AffineXform::MatrixType::SingularMatrixException& )
      {
      StdErr << "ERROR: singular matrix encountered reading transformation from " << *it << "\n";
      throw ExitException( 1 );
      }
    catch ( const PolynomialHelper::DegreeUnsupported& ex )
      {
      StdErr << "ERROR: polynomial degree unsupported in " << *it << "\n";
      StdErr << ex.what() << "\n";
      throw ExitException( 1 );
      }
    }
  
  return xformList;
}
