/*
//
//  Copyright 1997-2010 Torsten Rohlfing
//
//  Copyright 2004-2014 SRI International
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

#include <System/cmtkConsole.h>
#include <System/cmtkCommandLine.h>
#include <System/cmtkExitException.h>
#include <System/cmtkDebugOutput.h>

#include <IO/cmtkPhantomIO.h>
#include <IO/cmtkVolumeIO.h>
#include <IO/cmtkXformIO.h>

#include <Base/cmtkDetectedPhantomMagphanEMR051.h>
#include <Base/cmtkLandmarkList.h>
#include <Base/cmtkLandmarkPairList.h>
#include <Base/cmtkFitPolynomialToLandmarks.h>
#include <Base/cmtkFitAffineToLandmarks.h>
#include <Base/cmtkFitSplineWarpToLandmarks.h>

#include <vector>

cmtk::Xform::SmartConstPtr
doFitPoly( const cmtk::LandmarkPairList& pairList, const byte polyDegree )
{
  cmtk::PolynomialXform::SmartConstPtr polyXform;
  try
    {
    // fit polynomial transformation to landmark pairs.
    polyXform = cmtk::FitPolynomialToLandmarks( pairList, polyDegree ).GetPolynomialXform();
    }
  catch ( cmtk::PolynomialHelper::DegreeUnsupported& ex )
    {
    cmtk::StdErr << "ERROR: library does not support polynomial degree " << polyDegree << "\n";
    cmtk::StdErr << ex.what() << "\n";
    throw cmtk::ExitException( 1 );
    }

  return polyXform;
}

cmtk::Xform::SmartConstPtr
doFitSpline( const cmtk::LandmarkPairList& pairList, const cmtk::UniformVolume& unwarpImage, const std::string& gridDims, const cmtk::Types::Coordinate gridSpacing, const cmtk::FitSplineWarpToLandmarks::Parameters& fittingParameters, const bool affineFirst )
{
  // check for inconsistent parameters
  if ( (gridSpacing > 0)  && !gridDims.empty() )
    {
    cmtk::StdErr << "ERROR: must specify either output spline control point spacing or grid dimensions, but not both.\n";
    throw cmtk::ExitException( 1 );
    }

  // fit spline warp, potentially preceded by linear transformation, to landmark pairs.
  cmtk::AffineXform::SmartConstPtr affineXform;
  if ( affineFirst )
    {
    try
      {
      affineXform = cmtk::FitAffineToLandmarks( pairList ).GetAffineXform();
      }
    catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
      {
      cmtk::StdErr << "ERROR: fitted affine transformation has singular matrix\n";
      throw cmtk::ExitException( 1 );
      }
    }

  // fit by final spacing
  cmtk::SplineWarpXform::SmartConstPtr splineWarp;
  if ( gridSpacing )
    {
    splineWarp = cmtk::FitSplineWarpToLandmarks( pairList ).Fit( unwarpImage.m_Size, gridSpacing, affineXform.GetPtr(), fittingParameters );
    }
  else
    {
    // or fit by final control point grid dimension
    if ( !gridDims.empty() )
      {
      double dims[3];
      if ( 3 != sscanf( gridDims.c_str(), "%20lf,%20lf,%20lf", &(dims[0]), &(dims[1]), &(dims[2]) ) )
	{
	cmtk::StdErr << "ERROR: grid dimensions must be specified as dimsX,dimsY,dimsZ\n";
	throw cmtk::ExitException( 1 );
	}
      
      splineWarp = cmtk::FitSplineWarpToLandmarks( pairList ).Fit( unwarpImage.m_Size, cmtk::FixedVector<3,double>::FromPointer( dims ), affineXform.GetPtr(), fittingParameters );
      }
    else
      {
      cmtk::StdErr << "ERROR: must specify either output spline control point spacing or grid dimensions.\n";
      throw cmtk::ExitException( 1 );
      }
    }
  return splineWarp;
}

int
doMain( const int argc, const char* argv[] )
{
  // input file system paths
  std::string inputPhantomPath;
  std::string inputImagePath;

  // landmark exclusion threshold
  cmtk::Types::Coordinate residualThreshold = 5.0;
 
  // fitting options
  bool fitInverse = false;
  bool fitSpline = false;

  // polynomial fitting options
  byte polyDegree = 4;

  // B-spline fitting options
  std::string gridDims;
  cmtk::Types::Coordinate gridSpacing = 0;
  cmtk::FitSplineWarpToLandmarks::Parameters splineFittingParameters;  
  bool affineFirst = true;

  // output path
  std::string outputXform;

  try
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Create transformation to unwarp an image based on a phantom description" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "This tool computes either a polynomial transformation or B-spline free-form deformation to unwarp an image. The transformation is based on expected and detected landmarks in an image of a structural phantom "
		       "acquired on the same scanner. Use the 'detect_adni_phantom' tool to detect landmarks of the ADNI Phantom in an image and generate a phantom description file suitable for use with this tool." );

    typedef cmtk::CommandLine::Key Key;    
    cl.BeginGroup( "Fitting", "Fitting Options" );
    cl.AddSwitch( Key( "fit-inverse" ), &fitInverse, true, "Fit inverse transformation (mapping actual to expected landmark locations). "
		  "This is useful for computing a Jacobian volume correction map (using 'reformatx') without having to numerically invert the fitted unwarping transformation." );
    cl.AddSwitch( Key( "fit-forward" ), &fitInverse, false, "Fit forward transformation (mapping expected to actual landmark locations). This is useful for rectifying the image via reslicing (using 'reformatx')." );
    cl.EndGroup();

    cl.BeginGroup( "Xform", "Transformation Models" );
    cl.AddSwitch( Key( "spline" ), &fitSpline, true, "Fit B-spline free-form deformation." );
    cl.AddSwitch( Key( "poly" ), &fitSpline, false, "Fit polynomial transformation." );
    cl.EndGroup();

    cl.BeginGroup( "PolyFitting", "Polynomial Fitting Options (with --poly)" );
    cl.AddOption( Key( "degree" ), &polyDegree, "Degree of the fitted polynomial transformation." );
    cl.EndGroup();

    cl.BeginGroup( "SplineFitting", "B-Spline Fitting Options (with --spline)" );
    cl.AddOption( Key( "final-cp-spacing" ), &gridSpacing, "Final control point grid spacing of the output B-spline transformation." );
    cl.AddOption( Key( "final-cp-dims" ), &gridDims, "Final control point grid dimensions (i.e., number of control points) of the output B-spline transformation. To be provided as 'dimX,dimY,dimZ'." );
    cl.AddOption( Key( "levels" ), &splineFittingParameters.m_Levels, "Number of levels in the multi-level B-spline approximation procedure." );
    cl.AddOption( Key( "iterations-per-level" ), &splineFittingParameters.m_IterationsPerLevel, "Maximum number of spline coefficient update iterations per level in the multi-level B-spline approximation procedure." );
    cl.AddOption( Key( "rms-threshold" ), &splineFittingParameters.m_ResidualThreshold, "Threshold for relative improvement of the RMS fitting residual. "
		  "The fitting iteration terminates if (rmsAfterUpdate-rmsBeforeUpdate)/rmsBeforeUpdate < threshold." );
    cl.AddSwitch( Key( "no-fit-affine" ), &affineFirst, false, "Disable fitting of affine transformation to initialize spline. Instead, fit spline directly. This usually gives worse results and is discouraged." );
    cl.EndGroup();

    cl.AddParameter( &inputPhantomPath, "InputPhantom", "Input path of the XML file describing a phantom previously detected in an image." );
    cl.AddParameter( &inputImagePath, "InputImage", "Input image path. This is the image that is unwarped. It is important that this image be acquired on the same scanner (not only the same model but the very machine) "
		     "on which the phantom image was also acquired, preferably in close temporal proximity. Also, both this and the phantom image must share and specify the same physical image coordinates, i.e., only images in "
		     "NIFTI or NRRD format can be used." )
      ->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameter( &outputXform, "OutputXform", "Output transformation path. This is the affine phantom-to-image coordinate transformation fitted to the detected landmark spheres." )
      ->SetProperties( cmtk::CommandLine::PROPS_XFORM | cmtk::CommandLine::PROPS_OUTPUT );
    
    cl.Parse( argc, argv );
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    return 1;
    }

  // read phantom description
  cmtk::DetectedPhantomMagphanEMR051::SmartPtr phantom( cmtk::PhantomIO::Read( inputPhantomPath ) );
  cmtk::DebugOutput( 5 ) << "INFO: read phantom with " << phantom->LandmarkPairsList().size() << " landmarks.\n";  

  cmtk::UniformVolume::SmartConstPtr unwarpImage = cmtk::VolumeIO::ReadOriented( inputImagePath );
  try
    {
    phantom->ApplyXformToLandmarks( cmtk::AffineXform( unwarpImage->GetImageToPhysicalMatrix().GetInverse() ) );
    }
  catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
    {
    cmtk::StdErr << "ERROR: singular image-to-physical space matrix cannot be inverted.\n";
    throw cmtk::ExitException( 1 );
    }

  cmtk::LandmarkPairList pairList;
  for ( std::list<cmtk::LandmarkPair>::const_iterator it = phantom->LandmarkPairsList().begin(); it != phantom->LandmarkPairsList().end(); ++it )
    {
    if ( it->m_Precise ) // exclude all unprecise landmarks
      {
      if ( it->m_Residual < residualThreshold ) // exclude outliers based on residual
	{
	if ( fitInverse )
	  {
	  pairList.push_back( it->GetSwapSourceTarget() );
	  }
	else
	  {
	  pairList.push_back( *it );
	  }
	}
      }
    }

  cmtk::DebugOutput( 2 ) << "INFO: using " << pairList.size() << " out of " << phantom->LandmarkPairsList().size() << " total phantom landmarks as fiducials.\n";
  
  cmtk::Xform::SmartConstPtr xform;
  if ( fitSpline )
    {
      xform = doFitSpline( pairList, *unwarpImage, gridDims, gridSpacing, splineFittingParameters, affineFirst );
    }
  else
    {
      xform = doFitPoly( pairList, polyDegree );
    }

  // writing resulting transformation
  cmtk::XformIO::Write( xform, outputXform );

  return 0;
}

#include "cmtkSafeMain"
