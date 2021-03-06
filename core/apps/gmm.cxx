/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011, 2013 SRI International
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

#include <Base/cmtkGaussianKernel.h>

#include <vector>
#include <string>

int
doMain
( const int argc, const char *argv[] )
{
  std::string inputImagePath;
  std::string maskImagePath;
  std::string outputImagePath;

  bool writeProbMaps = false;
  bool priorsInitOnly = false;

  byte nClasses = 3;
  byte nIterations = 10;

  double priorEpsilon = 0;

  std::vector<std::string> priorImagePaths;

  try
    {
    cmtk::CommandLine cl( cmtk::CommandLine::PROPS_XML );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Gaussian mixture model segmentation" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Segment an image into c classes using the EM algorithm for Gaussian mixtures with optional priors." );

    typedef cmtk::CommandLine::Key Key;

    cl.BeginGroup( "General", "General Classification Parameters" );    
    cl.AddOption( Key( 'm', "mask" ), &maskImagePath, "Path to foreground mask image. If this is not provided, the input image is used as its own mask, but this does not work properly if the input image itself has pixels with "
		  "zero or negative values." );
    cl.AddOption( Key( 'c', "classes" ), &nClasses, "Number of classes." );    
    cl.AddOption( Key( 'n', "iterations" ), &nIterations, "Number of EM iterations." );
    cl.EndGroup();

    cl.BeginGroup( "Priors", "Handling of Priors" );
    cl.AddSwitch( Key( "priors-init-only" ), &priorsInitOnly, true, "Use priors for initialization only." );
    cl.AddOption( Key( 'e', "prior-epsilon" ), &priorEpsilon, "Small value to add to all class priors to eliminate zero priors." );
    
    cl.BeginGroup( "Output", "Output Parameters" );
    cl.AddSwitch( Key( 'p', "probability-maps" ), &writeProbMaps, true, "Write probability maps. The file names for these maps will be generated from the output image path by inserting '_prob#' before the file format suffix, "
		  "where '#' is the index of the respective class, numbered starting at 1 (zero is background)." );
    cl.EndGroup();

    cl.AddParameter( &inputImagePath, "InputImage", "Input image path" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameter( &outputImagePath, "OutputImage", "Output image path" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE | cmtk::CommandLine::PROPS_OUTPUT );
    cl.AddParameterVector( &priorImagePaths, "PriorImages", "Prior image paths" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );

    cl.Parse( argc, argv );
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    throw cmtk::ExitException( 1 );
    }

  if ( priorImagePaths.size() && (priorImagePaths.size() != nClasses) )
    {
    cmtk::StdErr << "ERROR: must provide one prior image per class.\n";
    throw cmtk::ExitException( 1 );
    }

  cmtk::UniformVolume::SmartPtr inputImage = cmtk::VolumeIO::ReadOriented( inputImagePath );

  cmtk::UniformVolume::SmartConstPtr maskImage = inputImage;
  if ( !maskImagePath.empty() )
    {
    maskImage = cmtk::VolumeIO::ReadOriented( maskImagePath );
    if ( ! inputImage->GridMatches( *(maskImage) ) )
      {
      cmtk::StdErr << "ERROR: mask image must have the same discrete grid as the input image.\n";
      throw cmtk::ExitException( 1 );      
      }
    }
  
  const size_t nPixels = inputImage->GetNumberOfPixels();
  
  std::vector<cmtk::UniformVolume::SmartConstPtr> priorImages( nClasses );
  for ( size_t k = 0; k < nClasses; ++k )
    {
    priorImages[k] = cmtk::VolumeIO::ReadOriented( priorImagePaths[k] );

    if ( ! inputImage->GridMatches( *(priorImages[k]) ) )
      {
      cmtk::StdErr << "ERROR: all prior images must have the same discrete grid as the input image.\n";
      throw cmtk::ExitException( 1 );      
      }
    }
  
  // based on tutorial by Carlo Tomasi:
  // http://www.cs.duke.edu/courses/spring04/cps196.1/handouts/EM/tomasiEM.pdf

  std::vector<double> classMu( nClasses ), classSigma( nClasses ), pTotal( nClasses );

  // initialize probabilities with priors
  std::vector<cmtk::UniformVolume::SmartPtr> pMaps( nClasses );
  for ( size_t k = 0; k < nClasses; ++k )
    {
    pMaps[k] = priorImages[k]->CloneGrid();
    pMaps[k]->SetData( priorImages[k]->GetData()->Convert( cmtk::TYPE_DOUBLE ) );
    }
  
  // Properly mask all non-foreground pixels  
  for ( size_t n = 0; n < nPixels; ++n )
      {
      if ( maskImage->GetDataAt( n ) <= 0 )
	{
	for ( size_t k = 0; k < nClasses; ++k )
	  {
	  pMaps[k]->SetDataAt( 0, n );
	  }
	}
      }

  // run EM iterations
  for ( size_t i = 0; i < nIterations; ++i )
    {
#pragma omp parallel for
    for ( int k = 0; k < nClasses; ++k )
      {
      classMu[k] = pTotal[k] = 0;
      for ( size_t n = 0; n < nPixels; ++n )
	{
	if ( maskImage->GetDataAt( n ) > 0 )
	  {
	  const double w = pMaps[k]->GetDataAt( n );
	  classMu[k] += w * inputImage->GetDataAt( n );
	  pTotal[k] += w;
	  }
	}
      
      classMu[k] /= pTotal[k];
      
      classSigma[k] = 0;
      for ( size_t n = 0; n < nPixels; ++n )
	{
	if ( maskImage->GetDataAt( n ) > 0 )
	  {
	  const double w = pMaps[k]->GetDataAt( n );
	  classSigma[k] += w * cmtk::MathUtil::Square( classMu[k] - inputImage->GetDataAt( n ) );
	  }
	}
      
      classSigma[k] = sqrt( classSigma[k] / pTotal[k] );
      }
    
    cmtk::StdOut.printf( "Iteration %u\n", i );
    for ( size_t k = 0; k < nClasses; ++k )
      {
      cmtk::StdOut.printf( "Class %u: %f +/- %f\t", k, classMu[k], classSigma[k] );
      }
    cmtk::StdOut << "\n";

#pragma omp parallel for    
    for ( int n = 0; n < static_cast<int>( nPixels ); ++n )
      {
      if ( maskImage->GetDataAt( n ) > 0 )
	{
	double pTotalPixel = 0;
	for ( size_t k = 0; k < nClasses; ++k )
	  {
	  double kernel = cmtk::GaussianKernel<double>::GetValue( inputImage->GetDataAt( n ), classMu[k], classSigma[k] );
	  if ( ! priorsInitOnly )
	    kernel *= priorImages[k]->GetDataAt( n ) + priorEpsilon;
	  
	  pMaps[k]->SetDataAt( kernel, n );
	  pTotalPixel += kernel;
	  }
	
	if ( pTotalPixel > 0 )
	  {
	  for ( size_t k = 0; k < nClasses; ++k )
	    {
	    pMaps[k]->SetDataAt( pMaps[k]->GetDataAt( n ) / pTotalPixel, n );
	    }
	  }
	}
      }
    }

  if ( writeProbMaps )
    {
    char path[PATH_MAX+1];
    for ( size_t k = 0; k < nClasses; ++k )
      {
      strncpy( path, outputImagePath.c_str(), PATH_MAX );
      path[PATH_MAX] = 0; // for safety - terminate string

      char* slash = strrchr( path, '/' );
      if ( ! slash )
	slash = path;
      
      char* period = strchr( slash, '.' );
      if ( ! period )
	period = path + strlen( path );
      
      snprintf( period, PATH_MAX - (period-path), "_prob%d%s", static_cast<int>( 1+k ), outputImagePath.c_str() + (period-path) );
      cmtk::VolumeIO::Write( *(pMaps[k]), path );
      }
    }
  
#pragma omp parallel for
  for ( int n = 0; n < static_cast<int>( nPixels ); ++n )
    {
    if ( maskImage->GetDataAt( n ) > 0 )
      {
      byte maxLabel = 0;
      double maxValue = pMaps[0]->GetDataAt( n );
      
      for ( size_t k = 1; k < nClasses; ++k )
	{
	const double pClass = pMaps[k]->GetDataAt( n );
	// if two classes have same probability, pick the one with max prior.
	if ( (pClass > maxValue) || ( ( pClass == maxValue ) && ( priorImages[k]->GetDataAt( n ) > priorImages[maxLabel]->GetDataAt( n ) ) ) )
	  {
	  maxLabel = k;
	  maxValue = pClass;
	  }
	}
      
      inputImage->SetDataAt( 1+maxLabel, n );
      }
    else
      {
      inputImage->SetDataAt( 0, n );
      }
    }

  cmtk::VolumeIO::Write( *(inputImage), outputImagePath );
  
  return 0;
}

#include "cmtkSafeMain"
