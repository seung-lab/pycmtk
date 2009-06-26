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
//  $Revision$
//
//  $LastChangedDate$
//
//  $LastChangedBy$
//
*/

#include <cmtkconfig.h>

#include <cmtkConsole.h>
#include <cmtkCommandLine.h>
#include <cmtkTimers.h>

#include <cmtkUniformVolume.h>
#include <cmtkFilterVolume.h>

#include <cmtkVolumeIO.h>
#include <cmtkUniformVolume.h>
#include <cmtkClassStream.h>

#include <cmtkGroupwiseRegistrationFunctionalAffineInitializer.h>
#include <cmtkGroupwiseRegistrationOutput.h>

#include <vector>

bool Verbose = false;

const char* PreDefinedTemplatePath = NULL;
cmtk::UniformVolume::SmartPtr PreDefinedTemplate;

const char* OutputRootDirectory = NULL;
const char* OutputArchive = "groupwise_init.xforms";
const char* OutputStudyListGroup = "groupwise_init.list";
const char* OutputStudyListIndividual = "groupwise_init_pairs";
const char* AverageImagePath = "groupwise_init_average.hdr";
cmtk::Interpolators::InterpolationEnum AverageImageInterpolation = cmtk::Interpolators::LINEAR;

bool AlignCentersOfMass = false;
bool InitScales = false;

// this vector holds all target image filenames
std::vector<const char*> fileNameList;

// this vector holds the original (not downsampled) images.
std::vector<cmtk::UniformVolume::SmartPtr> imageListOriginal;

int
main( int argc, char* argv[] )
{
#ifdef CMTK_BUILD_MPI
#  ifdef CMTK_BUILD_SMP
  const int threadLevelSupportedMPI = MPI::Init_thread( argc, argv, MPI::THREAD_FUNNELED );
  if ( threadLevelSupportedMPI < MPI::THREAD_FUNNELED )
    {
    cmtk::StdErr << "WARNING: your MPI implementation does not seem to support THREAD_FUNNELED.\n";
    }
#  else
  MPI::Init( argc, argv );
#  endif
#endif

  try 
    {
    cmtk::CommandLine cl( argc, argv );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Affine initialization for groupwise registration" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Compute initial affine alignment for a group of input images, which can be used as an input for groupwise registration" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "[options] image0 [image1 ...]" );
    
    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( 'v', "verbose" ), &Verbose, true, "Verbose operation." );

    cl.AddOption( Key( 't', "template" ), &PreDefinedTemplatePath, "Input filename for pre-defined template image." );

    cl.AddOption( Key( 'O', "output-root" ), &OutputRootDirectory, "Root directory for all output files." );
    cl.AddOption( Key( 'o', "output" ), &OutputArchive, "Output filename for groupwise registration archive." );
    cl.AddOption( Key( "output-average" ), &AverageImagePath, "Output filename for registered average image." );
    cl.AddSwitch( Key( "average-cubic" ), &AverageImageInterpolation, cmtk::Interpolators::CUBIC, "Use cubic interpolation for average image (default: linear)" );
    cl.AddSwitch( Key( "no-output-average" ), &AverageImagePath, NULL, "Do not write average image." );

    cl.AddSwitch( Key( "align-centers-of-mass" ), &AlignCentersOfMass, true, "Initially align centers of mass [default: centers of Bounding Boxes]" );
    cl.AddSwitch( Key( "init-scales" ), &InitScales, true, "Initialize scale factors using first-order moments [default: off]" );

    cl.Parse();

    const char* next = cl.GetNext();
    while ( next )
      {      
      fileNameList.push_back( next );
      next = cl.GetNextOptional();
      }
    }
  catch ( cmtk::CommandLine::Exception e )
    {
    cmtk::StdErr << e << "\n";
    exit( 1 );
    }

  cmtk::GroupwiseRegistrationFunctionalAffineInitializer::SmartPtr initializer( new cmtk::GroupwiseRegistrationFunctionalAffineInitializer );

  int idx = 0;
  for ( std::vector<const char*>::const_iterator fnIt = fileNameList.begin(); fnIt != fileNameList.end(); ++fnIt, ++idx )
    {
    cmtk::UniformVolume::SmartPtr nextImage;
    cmtk::UniformVolume::SmartPtr image( cmtk::VolumeIO::ReadOriented( *fnIt, Verbose ) );
    if ( ! image || ! image->GetData() )
      {
      cmtk::StdErr << "ERROR: Could not read image " << *fnIt << "\n";
      exit( 1 );
      }
    nextImage = image;
    imageListOriginal.push_back( nextImage );
    }

  initializer->SetTargetImages( imageListOriginal );
  if ( PreDefinedTemplatePath )
    {
    PreDefinedTemplate = cmtk::UniformVolume::SmartPtr( cmtk::VolumeIO::ReadGridOriented( PreDefinedTemplatePath, Verbose ) );
    }
  
  if ( PreDefinedTemplate )
    initializer->SetTemplateGrid( PreDefinedTemplate );
  else
    initializer->CreateTemplateGridFromTargets( imageListOriginal );
  
  cmtk::UniformVolume::SmartPtr templateGrid = initializer->GetTemplateGrid();
  
  if ( Verbose )
    {
    cmtk::StdErr.printf( "Template grid is %d x %d x %d pixels of size %f x %f x %f\n",
			 templateGrid->m_Dims[0], templateGrid->m_Dims[1], templateGrid->m_Dims[2], templateGrid->m_Delta[0], templateGrid->m_Delta[1], templateGrid->m_Delta[2] );
    }
  
  initializer->InitializeXforms( true /*alignCenters*/, AlignCentersOfMass, InitScales );

  cmtk::GroupwiseRegistrationOutput output;
  if ( PreDefinedTemplatePath )
    {
    output.SetExistingTemplatePath( true );
    }
  else
    {
    PreDefinedTemplatePath = AverageImagePath;
    }
  
  output.SetFunctional( initializer );
  output.SetOutputRootDirectory( OutputRootDirectory );
  output.WriteGroupwiseArchive( OutputArchive );
  output.WriteXformsSeparateArchives( OutputStudyListIndividual, PreDefinedTemplatePath );
  output.WriteAverageImage( AverageImagePath, AverageImageInterpolation );
  
#ifdef CMTK_BUILD_MPI    
  MPI::Finalize();
#endif

  return 0;
}

