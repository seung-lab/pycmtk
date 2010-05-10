/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010 SRI International
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
//  $Revision: 386 $
//
//  $LastChangedDate: 2009-08-04 14:31:09 -0700 (Tue, 04 Aug 2009) $
//
//  $LastChangedBy: torstenrohlfing $
//
*/

#include <cmtkconfig.h>

#include <cmtkConsole.h>
#include <cmtkCommandLine.h>

#include <cmtkTypedArray.h>
#include <cmtkAffineXform.h>
#include <cmtkSplineWarpXform.h>
#include <cmtkDeformationField.h>

#include <cmtkVolumeIO.h>
#include <cmtkXformIO.h>
#include <cmtkXformList.h>

bool Verbose = false;

bool WarpOnly = false;

namespace
cmtk
{
/// Mode for scalar value extraction.
typedef enum {
  /// Extract x component.
  X2S_EXTRACT_X,
  /// Extract y component.
  X2S_EXTRACT_Y,
  /// Extract z component.
  X2S_EXTRACT_Z,
  /// Extract Jacobian determinant.
  X2S_MAGNITUDE
} XformToScalarMode;
}

/// Mode for scalar value extraction.
cmtk::XformToScalarMode Mode = cmtk::X2S_MAGNITUDE;

/// Data type for scalar data.
cmtk::ScalarDataType DataType = cmtk::TYPE_DOUBLE;

const char* InputGridPath = NULL;
const char* OutImagePath = NULL;

std::vector<std::string> InputXformPaths;

int
main ( const int argc, const char* argv[] ) 
{
  try
    {
    cmtk::CommandLine cl( argc, argv, cmtk::CommandLine::PROPS_XML );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Extract scalar measures from transformations and deformation fields" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "This tool extracts scalar measures from transformations and deformation fields, sampled at grid locations, and writes the results to an image. "
		       "Examples of supported scalar measures are: x,y,z component of the transformation, magnitude of the transformation, and Jacobian determinants." );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_CATEG, "CMTK.Data Processing" );

    typedef cmtk::CommandLine::Key Key;    
    cl.AddSwitch( Key( 'v', "verbose" ), &Verbose, true, "Verbose mode on." );

    cl.BeginGroup( "operation", "Operating Options" );
    cmtk::CommandLine::EnumGroup<cmtk::XformToScalarMode>::SmartPtr
      modeGroup = cl.AddEnum( "mode", &Mode, "Mode of operation: type of scalar measure to be extracted." );
    modeGroup->AddSwitch( Key( "x-component" ), cmtk::X2S_EXTRACT_X, "X component of transformation vector." );
    modeGroup->AddSwitch( Key( "y-component" ), cmtk::X2S_EXTRACT_Y, "Y component of transformation vector." );
    modeGroup->AddSwitch( Key( "z-component" ), cmtk::X2S_EXTRACT_Z, "Z component of transformation vector." );
    modeGroup->AddSwitch( Key( "magnitude" ), cmtk::X2S_MAGNITUDE, "Magnitude of the transformation vector." );

    cl.AddSwitch( Key( 'w', "warp-only" ), &WarpOnly, true, "Output warp component only (excluding affine)." );
    cl.EndGroup();
        
    cl.BeginGroup( "output", "Output Options" );
    cmtk::CommandLine::EnumGroup<cmtk::ScalarDataType>::SmartPtr
      typeGroup = cl.AddEnum( "type", &DataType, "Scalar data type of output image." );
    typeGroup->AddSwitch( Key( "float" ), cmtk::TYPE_FLOAT, "Single-precision float." );
    typeGroup->AddSwitch( Key( "double" ), cmtk::TYPE_DOUBLE, "Double-precision float." );

    cl.AddOption( Key( 'o', "output" ), &OutImagePath, "Output path for image with extracted scalar data." )->SetProperties( cmtk::CommandLine::PROPS_IMAGE | cmtk::CommandLine::PROPS_OUTPUT );
    
    cl.AddParameter( &InputGridPath, "InputImage", "Input grid path" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameterVector( &InputXformPaths, "InputTransformation", "Input transformation paths. Use '--inverse' prefix to invert the following transformation." );

    cl.Parse();
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    exit( 1 );
    }

  cmtk::UniformVolume::SmartPtr scalarImage( cmtk::VolumeIO::ReadGridOriented( InputGridPath, Verbose ) );
  if ( ! scalarImage ) 
    {
    cmtk::StdErr << "Could not read grid from image " << InputGridPath << "\n";
    exit(1);
    }
  scalarImage->CreateDataArray( DataType );

  cmtk::XformList xformList;
  cmtk::XformList xformListAffine;

  for ( size_t i = 0; i < InputXformPaths.size(); ++i )
    {
    bool inverse = false;
    if ( InputXformPaths[i] == std::string( "--inverse" ) )
      {
      inverse = true;
      ++i;

      if ( i >= InputXformPaths.size() )
	{
	cmtk::StdErr << "ERROR: '--inverse' cannot be the last command line argument\n";
	exit(1);
	}
      }

    cmtk::Xform::SmartPtr xform( cmtk::XformIO::Read( InputXformPaths[i].c_str(), Verbose ) );
    if ( ! xform ) 
      {
      cmtk::StdErr << "Could not read transformation from file " << InputXformPaths[i] << "\n";
      exit(1);
      }

    xformList.Add( xform, inverse );

    const cmtk::SplineWarpXform* splineWarp = cmtk::SplineWarpXform::SmartPtr ::DynamicCastFrom( xform );  
    if ( splineWarp )
      {
	  const cmtk::Xform::SmartPtr affine( cmtk::Xform::SmartPtr::DynamicCastFrom( splineWarp->GetInitialAffineXform() ) );
      xformListAffine.Add( affine );
      }
    else
      {
      xformListAffine.Add( xform );
      }
    }  
  
  const cmtk::DataGrid::IndexType& dims = scalarImage->GetDims();
#pragma omp parallel for
  for ( int z = 0; z < dims[2]; ++z )
    {
    size_t offset = z * dims[0] * dims[1];
    for ( int y = 0; y < dims[1]; ++y )
      {
      for ( int x = 0; x < dims[0]; ++x, ++offset )
	{
	// Get current grid location.
	cmtk::Vector3D v;
	scalarImage->GetGridLocation( v, x, y, z );
	const cmtk::Vector3D v0( v );

	// Apply transformation and subtract original coordinate
	xformList.ApplyInPlace( v );
	v -= v0;

	// Is this is a B-spline warp and we're only interesting in the nonrigid component?
	if ( WarpOnly )
	  {
	  // Transform current location also using affine transformation
	  cmtk::Vector3D vAffine( v0 );
	  xformListAffine.ApplyInPlace( vAffine );
	  vAffine -= v0;
	  
	  // subtract affine-transformed from total transformed
	  v -= vAffine;
	  }
	
	switch ( Mode )
	  {
	  case cmtk::X2S_EXTRACT_X: scalarImage->SetDataAt( v[0], offset ); break;
	  case cmtk::X2S_EXTRACT_Y: scalarImage->SetDataAt( v[1], offset ); break;
	  case cmtk::X2S_EXTRACT_Z: scalarImage->SetDataAt( v[2], offset ); break;
	  case cmtk::X2S_MAGNITUDE: scalarImage->SetDataAt( v.RootSumOfSquares(), offset ); break;
	  default: break;
	  }
	}
      }
    }

  if ( OutImagePath ) 	 
    {
    cmtk::VolumeIO::Write( *scalarImage, OutImagePath, Verbose );
    }
}

