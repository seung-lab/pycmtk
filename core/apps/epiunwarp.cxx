/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011 SRI International
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

#include <cmtkconfig.h>

#include <System/cmtkCommandLine.h>
#include <System/cmtkExitException.h>
#include <System/cmtkSmartPtr.h>

#include <IO/cmtkVolumeIO.h>

#include <Registration/cmtkEchoPlanarUnwarpFunctional.h>

int
doMain
( const int argc, const char *argv[] )
{
  const char* inputImagePath1 = NULL;
  const char* inputImagePath2 = NULL;
  const char* outputImagePath1 = NULL;
  const char* outputImagePath2 = NULL;

  byte phaseEncodeDirection = 1;

  bool noInitCOM = false;

  try
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Unwarp Echo Planar Images" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Correct B0 field inhomogeneity-induced distortion in Echo Planar Images (e.g., diffusion-weighted images) using two images acquired with opposing phase encoding directions." );

    typedef cmtk::CommandLine::Key Key;

    cl.BeginGroup( "General", "General Parameters" );    
    cl.EndGroup();

    cmtk::CommandLine::EnumGroup<byte>::SmartPtr phaseEncodeGroup = cl.AddEnum( "phase-encode", &phaseEncodeDirection, "Define the phase-encoded image coordinate direction." );
    phaseEncodeGroup->AddSwitch( Key( 'y', "phase-encode-ap" ), 1, "Anterior/posterior phase encoding (this is the most common case)" );
    phaseEncodeGroup->AddSwitch( Key( 'z', "phase-encode-is" ), 2, "Top/bottom phase encoding" );
    phaseEncodeGroup->AddSwitch( Key( 'x', "phase-encode-lr" ), 0, "Lateral, left/right phase encoding (this is very rare)" );

    cl.AddSwitch( Key( "no-init-com" ), &noInitCOM, true, "Disable initialization of deformation field based on center-of-mass alignment." );

    cl.AddParameter( &inputImagePath1, "InputImage1", "First input image path - this is the standard b=0 image." )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameter( &inputImagePath2, "InputImage2", "Second input image path - this is the b=0 image with reversed phase encoding direction." )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameter( &outputImagePath1, "OutputImage1", "First output image path - this is the unwarped, corrected first image." )->SetProperties( cmtk::CommandLine::PROPS_IMAGE | cmtk::CommandLine::PROPS_OUTPUT );
    cl.AddParameter( &outputImagePath1, "OutputImage2", "Second output image path - this is the unwarped, corrected second image." )->SetProperties( cmtk::CommandLine::PROPS_IMAGE | cmtk::CommandLine::PROPS_OUTPUT );
    
    cl.Parse( argc, argv );
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    throw cmtk::ExitException( 1 );
    }

  cmtk::UniformVolume::SmartPtr inputImage1 = cmtk::VolumeIO::ReadOriented( inputImagePath1 );
  cmtk::UniformVolume::SmartPtr inputImage2 = cmtk::VolumeIO::ReadOriented( inputImagePath2 );

  cmtk::EchoPlanarUnwarpFunctional func( inputImage1, inputImage2, phaseEncodeDirection );

  cmtk::VolumeIO::Write( *func.GetCorrectedImage( 0 ), outputImagePath1 );
  cmtk::VolumeIO::Write( *func.GetCorrectedImage( 1 ), outputImagePath2 );
  
  return 0;
}

#include "cmtkSafeMain"
