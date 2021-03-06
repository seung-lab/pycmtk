/*
//
//  Copyright 1997-2009 Torsten Rohlfing
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

#include "cmtkImagePairAffineRegistrationCommandLine.h"

#include <System/cmtkConsole.h>
#include <System/cmtkDebugOutput.h>
#include <System/cmtkThreads.h>
#include <System/cmtkTimers.h>
#include <System/cmtkCommandLine.h>
#include <System/cmtkExitException.h>
#include <System/cmtkCompressedStream.h>
#include <System/cmtkMountPoints.h>

#include <Base/cmtkTypes.h>
#include <Base/cmtkAnatomicalOrientation.h>
#include <Base/cmtkTransformChangeToSpaceAffine.h>
#include <Base/cmtkTransformChangeFromSpaceAffine.h>

#include <Registration/cmtkRegistrationCallback.h>
#include <Registration/cmtkProtocolCallback.h>
#include <Registration/cmtkMakeInitialAffineTransformation.h>

#include <IO/cmtkVolumeIO.h>
#include <IO/cmtkClassStreamInput.h>
#include <IO/cmtkClassStreamOutput.h>
#include <IO/cmtkClassStreamAffineXform.h>
#include <IO/cmtkXformIO.h>
#include <IO/cmtkAffineXformITKIO.h>

#ifdef CMTK_USE_SQLITE
#  include <Registration/cmtkImageXformDB.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#ifdef HAVE_SYS_UTSNAME_H
#  include <sys/utsname.h>
#endif

#ifdef _MSC_VER
#  include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <iostream>

namespace 
cmtk
{

/** \addtogroup Registration */
//@{

ImagePairAffineRegistrationCommandLine
::ImagePairAffineRegistrationCommandLine 
( const int argc, const char* argv[] ) 
{
  this->m_Metric = 0;

  this->m_AutoMultiLevels = 0;
  this->m_CoarsestResolution = -1;
  this->m_MaxStepSize = 8;
  this->m_MinStepSize = 0.1;
  this->m_Sampling = 1.0;

  bool forceOutsideFlag = false;
  Types::DataItem forceOutsideValue = 0;

  std::string inStudylist;
  std::string InitialStudylist;

  std::string clArg1; // input studylist or reference image
  std::string clArg2; // empty or floating image

  try 
    {
    CommandLine cl( CommandLine::PROPS_XML );
    cl.SetProgramInfo( CommandLine::PRG_TITLE, "Rigid and affine registration" );
    cl.SetProgramInfo( CommandLine::PRG_DESCR, "This program performs rigid and affine image registration using multi-resolution optimization of voxel-based image similarity measures." );
    cl.SetProgramInfo( CommandLine::PRG_CATEG, "CMTK.Registration.Experimental" );

    typedef CommandLine::Key Key;
    cl.BeginGroup( "Automation", "Automation Options" );
    cl.AddOption( Key( "auto-multi-levels" ), &this->m_AutoMultiLevels, "Automatic optimization and resolution parameter generation for <n> levels" );

    cl.BeginGroup( "Optimization", "Optimization settings" );
    cl.AddOption( Key( "max-stepsize" ), &this->m_MaxStepSize, "Maximum optimizer step size, which determines search space exploration." );
    cl.AddOption( Key( "min-stepsize" ), &this->m_MinStepSize, "Minimum optimizer step size, which determines precision." );
    cl.AddOption( Key( "stepfactor" ), &this->m_OptimizerStepFactor, "Factor for search step size reduction. Must be > 0.0 and < 1.0" );
    cl.AddOption( Key( "delta-f-threshold" ), &this->m_DeltaFThreshold, "Optional threshold to terminate optimization (level) if relative change of target function drops below this value." );
    cl.EndGroup();

    cl.BeginGroup( "Resolution", "Image resolution parameters" );
    cl.AddOption( Key( 's', "sampling" ), &this->m_Sampling, "Image sampling (finest resampled image resolution)" );
    cl.AddOption( Key( "coarsest" ), &this->m_CoarsestResolution, "Upper limit for image sampling in multiresolution hierarchy" );

    cl.AddSwitch( Key( "omit-original-data" ), &this->m_UseOriginalData, false, "Do not use original data in full resolution, omit final stage in multiresolution hierarchy, thus reducing computation time." );
    cl.EndGroup();

    cl.BeginGroup( "Transformation", "Transformation parameters" );
    cl.AddVector( Key( "dofs" ), this->NumberDOFs, "Add number of degrees of freedom. This can be 3 (translation), 6 (rigid: translation and rotation), "
		  "7 (rigid plus global scale), 9 (rigid plus anisotropic scales), 12 (rigid plus scales plus shears), 3003 (translation plus shear), 3033 (translation, shear, and scale) or 3303 (rigid plus shears, but no scale). "
		  "This option can be repeated, in which case DOFs are used for successive optimization runs in the order that they appear." );
    cl.AddVector( Key( "dofs-final" ), this->NumberDOFsFinal, "Add number of degrees of freedom for final level only [can be repeated]" );
    cl.AddSwitch( Key( "symmetric" ), &this->m_SymmetricFwdBwd, true, "Use symmetric registration functional to simultaneously estimate forward and inverse transformation. "
		  "This increases ragistration time substantially but produces a result that is invariant under exchange of fixed and moving image. "
		  "It may also be more robust and/or more accurate than forward-only registration." );

    CommandLine::EnumGroup<int>::SmartPtr
      inPlaneGroup = cl.AddEnum( "restrict-in-plane", &this->m_RestrictToInPlane, "Restrict the affine transformation to be in-plane for planes perpendicular to a given coordinate axis." );
    inPlaneGroup->AddSwitch( Key( "xy" ), 2, "Transformation restricted to in-plane for 'xy' plane (perpendicular to z coordinate axis)." );
    inPlaneGroup->AddSwitch( Key( "xz" ), 1, "Transformation restricted to in-plane for 'xz' plane (perpendicular to y coordinate axis)." );
    inPlaneGroup->AddSwitch( Key( "yz" ), 0, "Transformation restricted to in-plane for 'yz' plane (perpendicular to x coordinate axis)." );
    inPlaneGroup->AddSwitch( Key( "none" ), -1, "Full 3D affine transformation is computed." );
    
    CommandLine::EnumGroup<MakeInitialAffineTransformation::Mode>::SmartPtr
      initGroup = cl.AddEnum( "init", &this->m_Initializer, "Select initializer for the affine trasnformation." );
    initGroup->AddSwitch( Key( "none" ), MakeInitialAffineTransformation::NONE, "Use input transformation, or identity transformation if none was provided." );
    initGroup->AddSwitch( Key( "fov" ), MakeInitialAffineTransformation::FOV, "Align centers of field of view (or crop regions) using a translation." );
    initGroup->AddSwitch( Key( "com" ), MakeInitialAffineTransformation::COM, "Align centers of mass using a translation." );
    initGroup->AddSwitch( Key( "pax" ), MakeInitialAffineTransformation::PAX, "Align images by rotation using principal axes and translation using centers of mass." );
    initGroup->AddSwitch( Key( "physical" ), MakeInitialAffineTransformation::PHYS, "Align images by rotation using direction vectors stored in input images and translation using image origins." );
    
    cl.AddOption( Key( "initial" ), &InitialStudylist, "Initialize transformation from given path" )->SetProperties( CommandLine::PROPS_XFORM );
    cl.AddSwitch( Key( "initial-is-inverse" ), &this->m_InitialXformIsInverse, true, "Invert initial transformation before initializing registration" );
    cl.EndGroup();

    cl.BeginGroup( "Image data", "Image data" );
    CommandLine::EnumGroup<int>::SmartPtr
      metricGroup = cl.AddEnum( "registration-metric", &this->m_Metric, "Registration metric for motion estimation by image-to-image registration." );
    metricGroup->AddSwitch( Key( "nmi" ), 0, "Normalized Mutual Information metric" );
    metricGroup->AddSwitch( Key( "mi" ), 1, "Standard Mutual Information metric" );
    metricGroup->AddSwitch( Key( "cr" ), 2, "Correlation Ratio metric" );
    metricGroup->AddSwitch( Key( "rms" ), 3, "Root of Mean Squaresa metric (this is the square root of MSD)" );
    metricGroup->AddSwitch( Key( "msd" ), 4, "Mean Squared Difference metric" );
    metricGroup->AddSwitch( Key( "ncc" ), 5, "Normalized Cross Correlation metric" );

    cl.BeginGroup( "Interpolation", "Floating Image Interpolation Options" );
    cmtk::CommandLine::EnumGroup<Interpolators::InterpolationEnum>::SmartPtr kernelGroup = 
      cl.AddEnum( "interpolation", &this->m_FloatingImageInterpolation, "Interpolation method for floating image sampling:" );
    kernelGroup->AddSwitch( Key( "nearest-neighbor" ), Interpolators::NEAREST_NEIGHBOR, "Nearest neighbor interpolation (for intensity and label data)" );
    kernelGroup->AddSwitch( Key( "linear" ), Interpolators::LINEAR, "Trilinear interpolation" );
    kernelGroup->AddSwitch( Key( "cubic" ), Interpolators::CUBIC, "Tricubic interpolation" );
    kernelGroup->AddSwitch( Key( "cosine-sinc" ), Interpolators::COSINE_SINC, "Cosine-windowed sinc interpolation (most accurate but slowest)" );
    kernelGroup->AddSwitch( Key( "partial-volume" ), Interpolators::PARTIALVOLUME, "Partial volume interpolation (for label data)" );
    kernelGroup->AddSwitch( Key( "automatic" ), Interpolators::DEFAULT, "Select interpolation automatically based on data type: linear for grey-level data, nearest neighbor for label data." );

    cl.AddSwitch( Key( "match-histograms" ), &this->m_MatchFltToRefHistogram, true, "Match floating image histogram to reference image histogram." );
    cl.AddOption( Key( "force-outside-value" ), &forceOutsideValue, "Force values outside field of view to this value rather than drop incomplete pixel pairs", &forceOutsideFlag );

    this->m_PreprocessorRef.AttachToCommandLine( cl );
    this->m_PreprocessorFlt.AttachToCommandLine( cl );

    cl.BeginGroup( "Output", "Output parameters" )->SetProperties( CommandLine::PROPS_NOXML );
    cl.AddOption( Key( 'o', "output" ), &this->Studylist, "Output path for final transformation" );
    cl.AddOption( Key( "write-matrix" ), &this->OutMatrixName, "Output path for final transformation in matrix format" );
    cl.AddOption( Key( "write-parameters" ), &this->OutParametersName, "Output path for final transformation in plain parameter list format" );
    cl.AddOption( Key( "write-protocol" ), &this->m_ProtocolFileName, "Optimization protocol output file name" );
    cl.AddOption( Key( "write-time" ), &this->Time, "Computation time statistics output file name" );
    cl.EndGroup();

    cl.BeginGroup( "SlicerImport", "Import Results into Slicer" );
    cl.AddOption( Key( "write-itk" ), &this->m_OutputPathITK, "Output path for final transformation in ITK format" )
      ->SetProperties( CommandLine::PROPS_XFORM | CommandLine::PROPS_OUTPUT )
      ->SetAttribute( "reference", "FloatingImage" );
    cl.AddOption( Key( "write-reformatted" ), &this->m_ReformattedImagePath, "Write reformatted floating image." )->SetProperties( CommandLine::PROPS_IMAGE | CommandLine::PROPS_OUTPUT );
    cl.EndGroup();
    
#ifdef CMTK_USE_SQLITE
    cl.BeginGroup( "Database", "Image/Transformation Database" );
    cl.AddOption( Key( "db" ), &this->m_UpdateDB, "Path to image/transformation database that should be updated with the new registration and/or reformatted image." );
    cl.EndGroup();
#endif

    cl.AddParameter( &clArg1, "ReferenceImage", "Reference (fixed) image path" )->SetProperties( CommandLine::PROPS_IMAGE );
    cl.AddParameter( &clArg2, "FloatingImage", "Floating (moving) image path" )->SetProperties( CommandLine::PROPS_IMAGE | CommandLine::PROPS_OPTIONAL );

    cl.Parse( argc, argv );
    }
  catch ( const CommandLine::Exception& ex )
    {
    StdErr << ex << "\n";
    throw cmtk::ExitException( 1 );
    }
  
  if ( (this->m_OptimizerStepFactor <= 0) || (this->m_OptimizerStepFactor >= 1) ) 
    {
    StdErr << "ERROR: step factor value " << this->m_OptimizerStepFactor << " is invalid. Must be in range (0..1)\n";
    throw cmtk::ExitException( 1 );
    }

  // check for supported numbers of degrees of freedom
  const std::set<short> supportedDOFs = AffineXform::GetSupportedDOFs();
  for ( std::vector<short>::iterator it = this->NumberDOFs.begin(); it != this->NumberDOFs.end(); ++it )
    {
    if ( *it == 603 ) // fix legacy value
      *it = 3303;

    if ( supportedDOFs.find( *it ) == supportedDOFs.end() )
      {
      StdErr << "ERROR: DOF number " << *it << " is not supported.\n";
      throw cmtk::ExitException( 1 );
      }
    }
  // check for supported numbers of degrees of freedom
  for ( std::vector<short>::iterator it = this->NumberDOFsFinal.begin(); it != this->NumberDOFsFinal.end(); ++it )
    {
    if ( *it == 603 ) // fix legacy value
      *it = 3303;

    if ( supportedDOFs.find( *it ) == supportedDOFs.end() )
      {
      StdErr << "ERROR: DOF number " << *it << " is not supported.\n";
      throw cmtk::ExitException( 1 );
      }
    }

  if ( ! clArg2.empty() ) 
    {
    this->Study1 = clArg1;
    this->Study2 = clArg2;
    } 
  else
    {
    inStudylist = clArg1;
    if ( ! InitialStudylist.empty() ) 
      {
      StdErr << "WARNING: transformation of input studylist will be overriden by transformation provided with '--initial'.\n";
      }
    
    DebugOutput( 1 ) << "Reading input studylist " << inStudylist << ".\n";
    
    ClassStreamInput typedStream( MountPoints::Translate(inStudylist), "registration" );
    if ( ! typedStream.IsValid() ) 
      {
      StdErr << "ERROR: could not open studylist archive " << inStudylist << ".\n";
      throw cmtk::ExitException( 1 );
      }

    typedStream.Seek ( "registration" );
    this->Study1 = typedStream.ReadStdString( "reference_study" );
    this->Study2 = typedStream.ReadStdString( "floating_study" );
    if ( !this->Study2.empty() )
      {
      AffineXform::SmartPtr affineXform;
      typedStream >> affineXform;
      this->SetInitialTransformation( affineXform );
      }
    else
      {
      // legacy studylists have inverse transformation in them
      this->Study2 = typedStream.ReadStdString( "model_study" );
      AffineXform::SmartPtr affineXform;
      typedStream >> affineXform;
      try
	{
	this->SetInitialTransformation( affineXform->GetInverse() );
	}
      catch ( const AffineXform::MatrixType::SingularMatrixException& )
	{
	StdErr << "ERROR: singular matrix from legacy transformation file cannot be inverted in ImagePairAffineRegistrationCommandLine constructor.\n";
	throw ExitException( 1 );
	}
      }

    typedStream.Close();
    }

  if ( this->Study1.empty() )
    {
    StdErr << "ERROR: reference image path is empty.\n";
    throw cmtk::ExitException( 1 );
    }
  
  if ( this->Study2.empty() )
    {
    StdErr << "ERROR: floating image path is empty.\n";
    throw cmtk::ExitException( 1 );
    }
  
  UniformVolume::SmartPtr volume( VolumeIO::ReadOriented( this->Study1 ) );
  if ( !volume )
    {
    StdErr << "ERROR: volume " << this->Study1 << " could not be read\n";
    throw cmtk::ExitException( 1 );
    }
  this->SetVolume_1( UniformVolume::SmartPtr( this->m_PreprocessorRef.GetProcessedImage( volume ) ) );

  volume = UniformVolume::SmartPtr( VolumeIO::ReadOriented( this->Study2 ) );
  if ( !volume )
    {
    StdErr << "ERROR: volume " << this->Study2 << " could not be read\n";
    throw cmtk::ExitException( 1 );
    }
  this->SetVolume_2(  UniformVolume::SmartPtr( this->m_PreprocessorFlt.GetProcessedImage( volume ) ) );

  if ( ! InitialStudylist.empty() ) 
    {
    Xform::SmartPtr xform( XformIO::Read( InitialStudylist ) );
    if ( ! xform ) 
      {
      StdErr << "ERROR: could not read transformation from " << InitialStudylist << "\n";
      throw cmtk::ExitException( 1 );
      }
    
    AffineXform::SmartPtr affine( AffineXform::SmartPtr::DynamicCastFrom( xform ) );
    if ( ! affine )
      {
      StdErr << "ERROR: transformation " << InitialStudylist << " is not affine.\n";
      throw cmtk::ExitException( 1 );
      }

    if ( affine->GetMetaInfo( META_SPACE ) != AnatomicalOrientation::ORIENTATION_STANDARD )
      {
      try
	{
	TransformChangeFromSpaceAffine toStandardSpace( *affine, *(this->m_Volume_1), *(this->m_Volume_2) );
	*affine = toStandardSpace.GetTransformation();
	affine->SetMetaInfo(META_SPACE, AnatomicalOrientation::ORIENTATION_STANDARD );
	}
      catch ( const AffineXform::MatrixType::SingularMatrixException& )
	{
	StdErr << "ERROR: singular matrix cannot be inverted to change transformation to standard space in ImagePairAffineRegistrationCommandLine constructor.\n";
	throw ExitException( 1 );
	}
      }
    
    this->SetInitialTransformation( affine );
    }
  
  if ( this->m_Initializer != MakeInitialAffineTransformation::NONE ) 
    {
    if ( ! (inStudylist.empty() && InitialStudylist.empty()) ) 
      {
      StdErr << "INFO: initial transformation was provided. Selected transformation initializer will be ignored.\n";
      } 
    }
  
  if ( !this->m_ProtocolFileName.empty() ) 
    {
    RegistrationCallback::SmartPtr callback( new ProtocolCallback( this->m_ProtocolFileName ) );
    this->SetCallback( callback );
    }

  if ( forceOutsideFlag )
    {
    this->SetForceOutside( true, forceOutsideValue );
    }
}

CallbackResult
ImagePairAffineRegistrationCommandLine::InitRegistration ()
{
  CallbackResult Result = Superclass::InitRegistration();
  return Result;
}
	
void
ImagePairAffineRegistrationCommandLine::OutputResultMatrix( const std::string& matrixName ) const
{
  const AffineXform::MatrixType& matrix = this->GetTransformation()->Matrix;
  
  FILE* mfile = fopen( matrixName.c_str(), "w" );
  if ( mfile )
    {
    for ( int i = 0; i < 4; ++i )
      {
      fprintf( mfile, "%e\t%e\t%e\t%e\n", static_cast<float>( matrix[0][i] ), static_cast<float>( matrix[1][i] ), static_cast<float>( matrix[2][i] ), static_cast<float>( matrix[3][i] ) );
      }
    fclose( mfile );
    }
}

void
ImagePairAffineRegistrationCommandLine::OutputResultParameters
( const std::string& paramsName, const CoordinateVector& v ) const
{
  FILE* pfile = fopen( paramsName.c_str(), "w" );
  if ( pfile )
    {
    for ( unsigned int idx=0; idx < v.Dim; ++idx )
      fprintf( pfile, "#%u: %f\n", idx, v.Elements[idx] );
    fclose( pfile );
    }
}

void
ImagePairAffineRegistrationCommandLine::OutputResultList( const std::string& studyList ) const
{
  ClassStreamOutput classStream( studyList, "studylist", ClassStreamOutput::MODE_WRITE );
  if ( !classStream.IsValid() ) return;
  
  classStream.Begin( "studylist" );
  classStream.WriteInt( "num_sources", 2 );
  classStream.End();
    
  classStream.Begin( "source" );
  classStream.WriteString( "studyname", CompressedStream::GetBaseName( Study1 ) );
  classStream.End();
    
  classStream.Begin( "source" );
  classStream.WriteString( "studyname", CompressedStream::GetBaseName( Study2 ) );
  classStream.End();
    
  classStream.Close();
    
  classStream.Open( studyList, "registration", ClassStreamOutput::MODE_WRITE );
    
  classStream.Begin( "registration" );
  classStream.WriteString( "reference_study", CompressedStream::GetBaseName( Study1 ) );
  classStream.WriteString( "floating_study", CompressedStream::GetBaseName( Study2 ) );
    
  classStream << *(this->GetTransformation());
    
  classStream.End();
  classStream.Close();
    
  classStream.Open( studyList, "settings", ClassStreamOutput::MODE_WRITE );
  classStream.WriteDouble( "exploration", this->m_MaxStepSize );
  classStream.WriteDouble( "accuracy", this->m_MinStepSize );
  classStream.WriteDouble( "min_sampling", this->m_Sampling );
  classStream.WriteDouble( "coarsest_resolution", this->m_CoarsestResolution );
  classStream.WriteInt( "metric", this->m_Metric );
  classStream.WriteDouble( "optimizer_step_factor", this->m_OptimizerStepFactor );
  classStream.WriteString( "initializer", MakeInitialAffineTransformation::GetModeName( this->m_Initializer ) );

  this->m_PreprocessorRef.WriteSettings( classStream );  
  this->m_PreprocessorFlt.WriteSettings( classStream );  

  classStream.Close();
    
  classStream.Open( studyList, "statistics", ClassStreamOutput::MODE_WRITE );
  classStream.WriteDouble( "time", this->GetTotalElapsedTime() );
  classStream.WriteDouble( "walltime", this->GetTotalElapsedWalltime() );
#ifdef CMTK_USE_PTHREADS
  classStream.WriteDouble( "thread_time", this->GetThreadTotalElapsedTime() );
#endif
    
#ifndef _MSC_VER
  struct utsname name;
  if ( uname( &name ) >= 0 ) 
    {
    classStream.WriteString( "host", name.nodename );
    classStream.WriteString( "system", name.sysname );
    }
#endif
  classStream.Close();
}

void
ImagePairAffineRegistrationCommandLine::OutputResult ( const CoordinateVector* v, const CallbackResult irq )
{
  DebugOutput( 1 ) << "Resulting transformation parameters: \n";
  for ( unsigned int idx=0; idx<v->Dim; ++idx )
    DebugOutput( 1 ).GetStream().printf( "#%u: %f\n", idx, v->Elements[idx] );
  
  if ( !this->OutMatrixName.empty() )
    {
    if ( irq != CALLBACK_OK )
      this->OutputResultMatrix( this->OutMatrixName + "-partial" );
    else
      this->OutputResultMatrix( this->OutMatrixName );
    }

  if ( !this->OutParametersName.empty() )
    {
    if ( irq != CALLBACK_OK )
      this->OutputResultParameters( this->OutParametersName + "-partial", *v );
    else
      this->OutputResultParameters( this->OutParametersName, *v );
    }

  if ( !this->Studylist.empty() ) 
    {
    if ( irq != CALLBACK_OK )
      this->OutputResultList( this->Studylist + "-partial" );
    else
      this->OutputResultList( this->Studylist );
    }

  if ( !this->m_OutputPathITK.empty() ) 
    {
    try
      {
      TransformChangeToSpaceAffine toNative( *(this->GetTransformation()), *(this->m_Volume_1), *(this->m_Volume_2), AnatomicalOrientationBase::SPACE_ITK );
      if ( irq != CALLBACK_OK )
	AffineXformITKIO::Write( this->m_OutputPathITK + "-partial", toNative.GetTransformation() );
      else
	AffineXformITKIO::Write( this->m_OutputPathITK, toNative.GetTransformation() );
      }
    catch ( const AffineXform::MatrixType::SingularMatrixException& )
      {
      StdErr << "ERROR: singular matrix cannot be inverted to change transformation to standard space in ImagePairAffineRegistrationCommandLine::OutputResult\n";
      throw ExitException( 1 );
      }
    }

  if ( !this->m_ReformattedImagePath.empty() )
    {
    if ( irq != CALLBACK_OK )
      VolumeIO::Write( *(this->GetReformattedFloatingImage()), this->m_ReformattedImagePath + "-partial" );
    else
      VolumeIO::Write( *(this->GetReformattedFloatingImage()), this->m_ReformattedImagePath );
    }

#ifdef CMTK_USE_SQLITE
  if ( !this->m_UpdateDB.empty() )
    {
    try
      {
      ImageXformDB db( this->m_UpdateDB );
      
      if ( !this->m_ReformattedImagePath.empty() )
	{
	db.AddImage( this->m_ReformattedImagePath, this->m_ReferenceVolume->GetMetaInfo( META_FS_PATH ) );
	}
      
      if ( ! this->Studylist.empty() )
	{
	if ( ! this->m_InitialXformPath.empty() ) 
	  {
	  db.AddRefinedXform( this->Studylist, true /*invertible*/, this->m_InitialXformPath, this->m_InitialXformIsInverse );
	  }
	else
	  {
	  db.AddImagePairXform( this->Studylist, true /*invertible*/, this->m_ReferenceVolume->GetMetaInfo( META_FS_PATH ), this->m_FloatingVolume->GetMetaInfo( META_FS_PATH ) );
	  }
	}
      }
    catch ( const ImageXformDB::Exception& ex )
      {
      StdErr << "DB ERROR: " << ex.what() << " on database " << this->m_UpdateDB << "\n";
      }
    }
#endif
}

void
ImagePairAffineRegistrationCommandLine::EnterResolution
( CoordinateVector::SmartPtr& v, Functional::SmartPtr& f,
  const int index, const int total )
{
  DebugOutput( 1 ).GetStream().printf( "\rEntering resolution level %d out of %d...\n", index, total );
  this->Superclass::EnterResolution( v, f, index, total );
}

CallbackResult
ImagePairAffineRegistrationCommandLine::Register ()
{
  const double baselineTime = Timers::GetTimeProcess();
  CallbackResult Result = Superclass::Register();
  const int elapsed = static_cast<int>( Timers::GetTimeProcess() - baselineTime );

  if ( !this->Time.empty() ) 
    {
    FILE *tfp = fopen( this->Time.c_str(), "w" );
    
    if ( tfp ) 
      {
      fprintf( tfp, "%d\n", elapsed );
      fclose( tfp );
      } 
    else 
      {
      std::cerr << "Could not open time file " << Time << "\n";
      }
    }
  return Result;
}

} // namespace cmtk

