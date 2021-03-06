/*
//
//  Copyright 2009-2010, 2013 SRI International
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

#include "cmtkAffineXformITKIO.h"

#include <fstream>
#include <string>
#include <typeinfo>

void
cmtk::AffineXformITKIO
::Write( const std::string& filename, const AffineXform& affineXform )
{
  std::ofstream stream( filename.c_str() );
  if ( stream.good() )
    {
    // write header
    stream << "#Insight Transform File V1.0\n";
    Self::Write( stream, affineXform, 0 );
    stream.close();
    }
}

void
cmtk::AffineXformITKIO
::Write( std::ofstream& stream, const AffineXform& affineXform, const size_t idx )
{
  stream << "# Transform " << idx << "\n";
  
  // write ID depending on whether CMTK is using single or double precision floats for coordinates
  if ( typeid( Types::Coordinate ) == typeid( double ) )
    {
    stream << "Transform: AffineTransform_double_3_3\n";
    }
  else
    {
    stream << "Transform: AffineTransform_float_3_3\n";
    }
  
  // write parameters, 3x3 transformation matrix first
  stream << "Parameters: ";
  for ( int i = 0; i < 3; ++i )
    {
    for ( int j = 0; j < 3; ++j )
      {
      stream << affineXform.Matrix[j][i] << " ";
      }
    }
  
  // write translations
  for ( int i = 0; i < 3; ++i )
    {
    stream << affineXform.Matrix[3][i] << " ";
    }
  
  // finish up with (all-zero) fixed parameters
  stream << "\n"
	 << "FixedParameters: 0 0 0\n";
}

cmtk::AffineXform::SmartPtr
cmtk::AffineXformITKIO
::Read( const std::string& filename )
{
  std::ifstream stream( filename.c_str() );
  if ( stream.good() )
    {
    std::string line;
    std::getline( stream, line );
    if ( line != "#Insight Transform File V1.0" )
      return AffineXform::SmartPtr( NULL );

    std::getline( stream, line );
    if ( line != "# Transform 0" )
      return AffineXform::SmartPtr( NULL );

    std::getline( stream, line );
    if ( line == "Transform: AffineTransform_double_3_3" || line == "Transform: AffineTransform_float_3_3" )
      {
      std::getline( stream, line, ' ' );
      Types::Coordinate matrix[4][4] = { {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,1} };
      
      for ( int i = 0; i < 3; ++i )
	{
	for ( int j = 0; j < 3; ++j )
	  {
	  stream >> matrix[j][i];
	  }
	}
      for ( int i = 0; i < 3; ++i )
	{
	stream >> matrix[3][i];
	}
      try
	{	
	AffineXform::SmartPtr xform( new AffineXform( matrix ) );
	xform->SetMetaInfo( META_SPACE, AnatomicalOrientationBase::SPACE_ITK );
	return xform;
	}
      catch ( const AffineXform::MatrixType::SingularMatrixException& )
	{
	StdErr << "ERROR: singular matrix in cmtk::AffineXformITKIO::Read()\n";
	}
      }
    }
  
  return AffineXform::SmartPtr( NULL );
}
