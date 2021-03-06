/*
//
//  Copyright 1997-2010 Torsten Rohlfing
//
//  Copyright 2004-2013 SRI International
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
#include <System/cmtkDebugOutput.h>
#include <System/cmtkCommandLine.h>
#include <System/cmtkExitException.h>

#include <IO/cmtkVolumeIO.h>

#include <Base/cmtkUniformVolume.h>
#include <Base/cmtkUniformVolumeInterpolator.h>
#include <Base/cmtkLinearInterpolator.h>
#include <Base/cmtkCubicInterpolator.h>
#include <Base/cmtkUniformVolumeGaussianFilter.h>

#include <Registration/cmtkAffineMultiChannelRegistrationFunctional.h>
#include <Registration/cmtkMultiChannelRMIRegistrationFunctional.h>
#include <Registration/cmtkMultiChannelHistogramRegistrationFunctional.h>
#include <Registration/cmtkBestNeighbourOptimizer.h>
#include <Registration/cmtkRegistrationCallback.h>

#include <IO/cmtkClassStreamOutput.h>
#include <IO/cmtkClassStreamMultiChannelRegistration.h>
#include <IO/cmtkXformIO.h>

#include <vector>
#include <algorithm>

#if defined(HAVE_STDINT_H)
#  include <stdint.h>
#else
typedef long int uint64_t;
#endif

std::list<const char*> fileListRef;
std::list<const char*> fileListFlt;

const char* initialXformPath = NULL;
const char* outArchive = NULL;

std::list<cmtk::UniformVolume::SmartPtr> refChannelList;
std::list<cmtk::UniformVolume::SmartPtr> fltChannelList;

std::vector<int> numberDOFs;

cmtk::Types::Coordinate initialStepSize = 1.0;
cmtk::Types::Coordinate finalStepSize = 0.125;
cmtk::Optimizer::ReturnType optimizerDeltaFThreshold = 0;

bool alignCenters = true;
bool metricNMI = true;
bool useHistograms = true;
bool useCubicInterpolation = false;

int downsampleFrom = 1;
int downsampleTo = 1;
bool downsampleWithAverage = false;

float smoothSigmaFactor = 0.0;
cmtk::Types::Coordinate minPixelSize = FLT_MAX;

const char* cropReferenceFromIndex = NULL;
const char* cropReferenceToIndex = NULL;

cmtk::UniformVolume::SmartPtr
MakeDownsampled( cmtk::UniformVolume::SmartConstPtr& image, const int downsample )
{
  if ( downsampleWithAverage )
    return cmtk::UniformVolume::SmartPtr( image->GetDownsampled( downsample * image->GetMinDelta() ) );

  cmtk::UniformVolume::SmartPtr result( image->CloneGrid() );

  if ( (smoothSigmaFactor > 0) && downsample )
    {
    const cmtk::Units::GaussianSigma sigma( smoothSigmaFactor * downsample * image->GetMinDelta() );
    result->SetData( cmtk::UniformVolumeGaussianFilter( image ).GetFiltered3D( sigma ) );
    }
  else
    {
    result->SetData( image->GetData() );
    }

  if ( downsample > 1 )
    result = cmtk::UniformVolume::SmartPtr( result->GetDownsampledAndAveraged( downsample, true /*approxIsotropic*/ ) );
  return result;
}

template<class TMetricFunctional>
void
DoRegistration() 
{
  typedef cmtk::AffineMultiChannelRegistrationFunctional<TMetricFunctional> FunctionalType;
  typename FunctionalType::SmartPtr functional( new FunctionalType );  
  functional->SetNormalizedMI( metricNMI );

  // determine initial transformation parameters based on bounding boxes.
  cmtk::CoordinateVector params;
  if ( initialXformPath )
    {
    cmtk::Xform::SmartPtr xform( cmtk::XformIO::Read( initialXformPath ) );
    if ( xform )
      {
      const cmtk::AffineXform::SmartPtr affine = cmtk::AffineXform::SmartPtr::DynamicCastFrom( xform );
      if ( affine )
	{
	affine->GetParamVector( params );
	}
      else
	{
	const cmtk::WarpXform::SmartPtr warp = cmtk::WarpXform::SmartPtr::DynamicCastFrom( xform );
	if ( warp )
	  {
	  warp->GetInitialAffineXform()->GetParamVector( params );
	  }
	}
      functional->SetParamVector( params );
      }
    else
      {
      cmtk::StdErr << "ERROR: unable to read initial transformation from " << initialXformPath << "\n";
      throw cmtk::ExitException( 2 );
      }
    }
  else
    {
    functional->AddReferenceChannel( *(refChannelList.begin()) );
    functional->AddFloatingChannel( *(fltChannelList.begin()) );
    functional->InitTransformation( alignCenters );
    functional->GetParamVector( params );
    }

  cmtk::BestNeighbourOptimizer optimizer;
  optimizer.SetDeltaFThreshold( optimizerDeltaFThreshold );
  optimizer.SetCallback( cmtk::RegistrationCallback::SmartPtr( new cmtk::RegistrationCallback ) );
  optimizer.SetFunctional( functional );

  for ( int downsample = std::max(downsampleFrom, downsampleTo); downsample >= std::min(downsampleFrom, downsampleTo); --downsample )
    {
    cmtk::DebugOutput( 1 ).GetStream().printf( "Downsampling stage 1:%d\n", downsample );

    functional->ClearAllChannels();
    for ( std::list<cmtk::UniformVolume::SmartPtr>::iterator it = refChannelList.begin(); it != refChannelList.end(); ++it )
      {
      cmtk::UniformVolume::SmartPtr image = MakeDownsampled( (*it), downsample );
      image->CopyMetaInfo( **it, cmtk::META_FS_PATH );
      functional->AddReferenceChannel( image );
      }

    for ( std::list<cmtk::UniformVolume::SmartPtr>::iterator it = fltChannelList.begin(); it != fltChannelList.end(); ++it )
      {
      cmtk::UniformVolume::SmartPtr image = MakeDownsampled( (*it), downsample );
      image->CopyMetaInfo( **it, cmtk::META_FS_PATH );
      functional->AddFloatingChannel( image );
      }

    for ( std::vector<int>::const_iterator itDOF = numberDOFs.begin(); itDOF != numberDOFs.end(); ++itDOF )
      {
      cmtk::DebugOutput( 1 ).GetStream().printf( "Setting number of DOFs to %d\n", *itDOF );
      
      functional->SetNumberDOFs( *itDOF );
      const cmtk::Types::Coordinate effectiveMinPixelSize = std::max( 1, downsample ) * minPixelSize;
      optimizer.Optimize( params, initialStepSize * effectiveMinPixelSize, finalStepSize * effectiveMinPixelSize );
      }
    }

  if ( outArchive )
    {
    cmtk::ClassStreamOutput stream( outArchive, cmtk::ClassStreamOutput::MODE_WRITE );
    if ( stream.IsValid() )
      {
      stream << *functional;
      stream.Close();
      }
    else
      {
      cmtk::StdErr << "ERROR: could not open archive " << outArchive << " for writing.\n";
      }
    }
}

int
doMain( const int argc, const char* argv[] ) 
{
  try 
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Multi-channel affine registration" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Multi-channel affine image registration using histogram-based or covariance-based joint entropy measures" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "mcaffine [options] refChannel0 [refChannel1 ...] -- fltChannel0 [fltChannel1 ...]" );    
    cl.SetProgramInfo( cmtk::CommandLine::PRG_CATEG, "CMTK.Image Registration" );

    typedef cmtk::CommandLine::Key Key;
    cl.AddOption( Key( 'o', "out-archive" ), &outArchive, "Output archive path." );
    cl.AddOption( Key( "initial-xform" ), &initialXformPath, "Optional path of a file with the initial transformation." );

    cl.AddOption( Key( 'd', "downsample-from" ), &downsampleFrom, "Initial downsampling factor [1]." );
    cl.AddOption( Key( 'D', "downsample-to" ), &downsampleTo, "Final downsampling factor [1]. Factor 0 is full resolution with smoothing turned off" );
    cl.AddOption( Key( "smooth" ), &smoothSigmaFactor, "Sigma of Gaussian smoothing kernel in multiples of template image pixel size [default: off] )" );
    cl.AddOption( Key( "downsample-average" ), &downsampleWithAverage, "Downsample using sliding-window averaging [default: off] )" );

    cl.AddVector( Key( "dofs" ), numberDOFs, "Set sequence of numbers of DOFs for optimization schedule [can be repeated]. Supported values are: 0, 3, 6, 7, 9, 12." );

    cl.AddSwitch( Key( "nmi" ), &metricNMI, true, "Use normalized mutual information metric [default]" );
    cl.AddSwitch( Key( "mi" ), &metricNMI, false, "Use standard mutual information metric" );

    cl.AddSwitch( Key( 'H', "histograms" ), &useHistograms, true, "Use multi-dimensional histograms to compute entropies [default]" );
    cl.AddSwitch( Key( 'C', "covariance" ), &useHistograms, false, "Use covariance matrix determinants to compute entropies" );
    cl.AddSwitch( Key( 'c', "cubic" ), &useCubicInterpolation, true, "Use cubic interpolation [default: linear]" );

    cl.AddOption( Key( "initial-step-size" ), &initialStepSize, "Initial optimizer step size in pixels." );
    cl.AddOption( Key( "final-step-size" ), &finalStepSize, "Initial optimizer step size in pixels." );
    cl.AddOption( Key( "delta-f-threshold" ), &optimizerDeltaFThreshold, "Optional threshold to terminate optimization (level) if relative change of target function drops below this value." );

    cl.AddOption( Key( "crop-reference-from-index" ), &cropReferenceFromIndex, "Crop reference image from index x,y,z." );
    cl.AddOption( Key( "crop-reference-to-index" ), &cropReferenceToIndex, "Crop reference image to index x,y,z." );

    cl.Parse( argc, argv );

    const char* next = cl.GetNext();
    while ( next && strcmp( next, "--" ) ) 
      {
      fileListRef.push_back( next );
      next = cl.GetNextOptional();
      }
    
    next = cl.GetNext();
    while ( next ) 
      {
      fileListFlt.push_back( next );
      next = cl.GetNextOptional();
      }
    }
  catch ( const cmtk::CommandLine::Exception& e ) 
    {
    cmtk::StdErr << e << "\n";
    throw cmtk::ExitException( 1 );
    }

  if ( numberDOFs.empty() )
    numberDOFs.push_back( 6 );

  for ( std::list<const char*>::const_iterator refIt = fileListRef.begin(); refIt != fileListRef.end(); ++refIt )
    {
    cmtk::UniformVolume::SmartPtr volume( cmtk::VolumeIO::ReadOriented( *refIt ) );
    if ( !volume || !volume->GetData() )
      {
      cmtk::StdErr << "ERROR: Cannot read image " << *refIt << "\n";
      throw cmtk::ExitException( 1 );
      }
    minPixelSize = std::min( volume->GetMinDelta(), minPixelSize );
    refChannelList.push_back( volume );
    }

  if ( cropReferenceFromIndex )
    {
    int xyz[3];
    if ( 3 != sscanf( cropReferenceFromIndex, "%6d,%6d,%6d", &xyz[0], &xyz[1], &xyz[2] ) )
      {
      cmtk::StdErr << "ERROR: reference crop from index could not parse index '" << cropReferenceFromIndex << "' as valid x,y,z index.\n";
      throw cmtk::ExitException( 1 );
      }
    for ( std::list<cmtk::UniformVolume::SmartPtr>::iterator refIt = refChannelList.begin(); refIt != refChannelList.end(); ++refIt )
      {
      (*refIt)->CropRegion() = cmtk::DataGrid::RegionType( cmtk::DataGrid::IndexType::FromPointer( xyz ), (*refIt)->CropRegion().To() );
      }
    }

  if ( cropReferenceToIndex )
    {
    int xyz[3];
    if ( 3 != sscanf( cropReferenceToIndex, "%6d,%6d,%6d", &xyz[0], &xyz[1], &xyz[2] ) )
      {
      cmtk::StdErr << "ERROR: reference crop to index could not parse index '" << cropReferenceToIndex << "' as valid x,y,z index.\n";
      throw cmtk::ExitException( 1 );
      }
    for ( std::list<cmtk::UniformVolume::SmartPtr>::iterator refIt = refChannelList.begin(); refIt != refChannelList.end(); ++refIt )
      {
      (*refIt)->CropRegion() = cmtk::DataGrid::RegionType( (*refIt)->CropRegion().From(), cmtk::DataGrid::IndexType::FromPointer( xyz ) );
      }
    }

  for ( std::list<const char*>::const_iterator fltIt = fileListFlt.begin(); fltIt != fileListFlt.end(); ++fltIt )
    {
    cmtk::UniformVolume::SmartPtr volume( cmtk::VolumeIO::ReadOriented( *fltIt ) );
    if ( !volume || !volume->GetData() )
      {
      cmtk::StdErr << "ERROR: Cannot read image " << *fltIt << "\n";
      throw cmtk::ExitException( 1 );
      }
    minPixelSize = std::min( volume->GetMinDelta(), minPixelSize );
    fltChannelList.push_back( volume );
    }

  if ( useCubicInterpolation )
    {
    typedef cmtk::UniformVolumeInterpolator<cmtk::Interpolators::Cubic> InterpolatorType;
    if ( useHistograms )
      {
      typedef cmtk::MultiChannelHistogramRegistrationFunctional<float,InterpolatorType,uint64_t,6> MetricFunctionalType;
      DoRegistration<MetricFunctionalType>();
      }
    else
      {
      typedef cmtk::MultiChannelRMIRegistrationFunctional<float> MetricFunctionalType;
      DoRegistration<MetricFunctionalType>();
      }
    }
  else
    {
    typedef cmtk::UniformVolumeInterpolator<cmtk::Interpolators::Linear> InterpolatorType;
    if ( useHistograms )
      {
      typedef cmtk::MultiChannelHistogramRegistrationFunctional<float,InterpolatorType,uint64_t,6> MetricFunctionalType;
      DoRegistration<MetricFunctionalType>();
      }
    else
      {
      typedef cmtk::MultiChannelRMIRegistrationFunctional<float> MetricFunctionalType;
      DoRegistration<MetricFunctionalType>();
      }
    }
  
  return 0;
}

#include "cmtkSafeMain"
