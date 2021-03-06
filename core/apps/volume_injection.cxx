/*
//
//  Copyright 1997-2009 Torsten Rohlfing
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

#include <System/cmtkCommandLine.h>
#include <System/cmtkConsole.h>
#include <System/cmtkDebugOutput.h>

#include <Base/cmtkUniformVolume.h>
#include <Base/cmtkVector3D.h>
#include <Base/cmtkTransformChangeFromSpaceAffine.h>

#include <Registration/cmtkAffineRegistration.h>
#include <Registration/cmtkProtocolCallback.h>

#include <Recon/cmtkVolumeInjectionReconstruction.h>

#include <IO/cmtkXformIO.h>
#include <IO/cmtkVolumeIO.h>

#include <algorithm>
#include <map>
#include <vector>

const char* ReconstructionGridPath = NULL;
bool ExcludeFirstImage = false;

std::vector<const char*> XformPaths;
std::vector<const char*> ImagePaths;

std::vector<cmtk::Xform::SmartPtr> Xforms;
std::vector<cmtk::UniformVolume::SmartPtr> Images;

const char* OutputImagePath = "volume_injection.nii";
bool WriteImagesAsFloat = false;

bool VolumeInjectionIsotropic = false;
double VolumeInjectionSigma = 1;
int VolumeInjectionRadius = 0;

std::map<size_t,float> PassWeights;

void
CallbackSetPassWeight( const char* argv )
{
  int pass = 0;
  float weight = 1.0;
  if ( 2 == sscanf( argv, "%4d:%10f", &pass, &weight ) )
    {
    PassWeights[pass] = weight;
    }
  else
    {
    cmtk::StdErr << "ERROR: pass weights must be given as 'pass:weight', where 'pass' is an integer and 'weight' is a number between 0 and 1.\n"
		 << "       Parameter provided was '" << argv << "'\n";
    throw cmtk::ExitException( 1 );
    }
}

bool UseCropRegion = false;
cmtk::DataGrid::RegionType CropRegion;

void
CallbackCrop( const char* arg )
{
  int cropFrom[3], cropTo[3];
  UseCropRegion = (6 == sscanf( arg, "%6d,%6d,%6d,%6d,%6d,%6d", cropFrom, cropFrom+1, cropFrom+2, cropTo,cropTo+1,cropTo+2 ) );

  if ( UseCropRegion )
    {
    CropRegion = cmtk::DataGrid::RegionType( cmtk::DataGrid::IndexType::FromPointer( cropFrom ), cmtk::DataGrid::IndexType::FromPointer( cropTo ) );
    }
  else
    {
    cmtk::StdErr.printf( "ERROR: string '%s' does not describe a valid crop region\n", arg );
    throw cmtk::ExitException( 1 );
    }
}

cmtk::UniformVolume::SmartPtr ReconGrid( NULL );

void
CallbackReconGrid( const char* arg )
{
  int gridDims[3] = { 0, 0, 0 };
  float gridDelta[3] = { 0, 0, 0 };
  float gridOffset[3] = { 0, 0, 0 };

  const size_t numArgs = sscanf( arg, "%6d,%6d,%6d:%15f,%15f,%15f:%15f,%15f,%15f", gridDims, gridDims+1, gridDims+2, gridDelta, gridDelta+1, gridDelta+2, gridOffset, gridOffset+1, gridOffset+2 );
  if ( (numArgs != 6) && (numArgs != 9) )
    {
    cmtk::StdErr << "ERROR: reconstruction volume definition must be int,int,int:float,float,float or int,int,int:float,float,float:float,float,float\n";
    throw cmtk::ExitException( 1 );
    }
  
  ReconGrid = cmtk::UniformVolume::SmartPtr( new cmtk::UniformVolume( cmtk::UniformVolume::IndexType::FromPointer( gridDims ), gridDelta[0], gridDelta[1], gridDelta[2] ) );
  ReconGrid->SetMetaInfo( cmtk::META_SPACE, cmtk::AnatomicalOrientation::ORIENTATION_STANDARD );
  ReconGrid->SetMetaInfo( cmtk::META_SPACE_ORIGINAL, cmtk::AnatomicalOrientation::ORIENTATION_STANDARD );
  ReconGrid->SetMetaInfo( cmtk::META_IMAGE_ORIENTATION, cmtk::AnatomicalOrientation::ORIENTATION_STANDARD );
  ReconGrid->SetMetaInfo( cmtk::META_IMAGE_ORIENTATION_ORIGINAL, cmtk::AnatomicalOrientation::ORIENTATION_STANDARD );
  
  if ( numArgs == 9 )
    {
    ReconGrid->SetOffset( cmtk::UniformVolume::CoordinateVectorType::FromPointer( gridOffset ) );
    }
}

void
WriteOutputImage( cmtk::UniformVolume::SmartPtr& image, const char* path )
{
  cmtk::UniformVolume::SmartPtr outputImage = image;

  const cmtk::ScalarDataType type = Images[0]->GetData()->GetType();
  if ( !WriteImagesAsFloat && (outputImage->GetData()->GetType() != type) )
    {
    outputImage = cmtk::UniformVolume::SmartPtr( outputImage->CloneGrid() );
    outputImage->SetData( cmtk::TypedArray::SmartPtr( image->GetData()->Convert( type ) ) );
    }
  cmtk::VolumeIO::Write( *outputImage, path );
}

int
doMain( const int argc, const char* argv[] )
{
  /*
  // Parse command line
  */
  try
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Volume injection" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Reconstruction a high-resolution volume from multiple co-registered (low-resolution) images using forward volume injection" ); 
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "volume_injection [options] refImage xform0 inImage0 [xform1 inImage1 ...]" );
    
    typedef cmtk::CommandLine::Key Key;
    cl.BeginGroup( "Input", "Input Options" );
    cl.AddSwitch( Key( 'x', "exclude-first-image" ), &ExcludeFirstImage, true, "Exclude first image from reconstruction as a separate registration target image)" );
    cl.AddCallback( Key( 'W', "pass-weight" ), CallbackSetPassWeight, "Set contribution weight for a pass in the form 'pass:weight'" );
    cl.EndGroup();

    cl.BeginGroup( "Grid", "Reconstruction Grid Options" );
    cl.AddCallback( Key( "recon-grid" ), CallbackReconGrid, "Define reconstruction grid as Nx,Ny,Nz:dX,dY,dZ[:Ox,Oy,Oz] (dims:pixel:offset)" );
    cl.AddOption( Key( 'R', "recon-grid-path" ), &ReconstructionGridPath, "Give path to grid that defines reconstructed image grid [including offset]" );
    cl.AddCallback( Key( "crop" ), CallbackCrop, "Crop reference to pixel region x0,y0,z1:x1,y1,z1" );
    cl.EndGroup();

    cl.BeginGroup( "Injection", "Volume Injection Options" );
    cl.AddSwitch( Key( "isotropic-injection" ), &VolumeInjectionIsotropic, true, "Use isotropic volume injection [default: scaled with pass image pixel size per dimension]" );
    cl.AddOption( Key( 'S', "injection-kernel-sigma" ), &VolumeInjectionSigma, "Gauss contribution" );
    cl.AddOption( Key( 'r', "injection-kernel-radius" ), &VolumeInjectionRadius, "VolumeInjectionRadius of affected pixel" );
    cl.EndGroup();

    cl.BeginGroup( "Output", "Output Options" );
    cl.AddOption( Key( 'o', "output" ), &OutputImagePath, "Output image path" );
    cl.AddSwitch( Key( 'F', "write-images-as-float" ), &WriteImagesAsFloat, true, "Write output images as floating point [default: same as input]" );
    cl.EndGroup();

    cl.Parse( argc, argv );

    ImagePaths.push_back( cl.GetNext() );
    XformPaths.push_back( NULL );
    
    const char* nextXform = cl.GetNext();
    const char* nextImage = cl.GetNext();
    while ( nextXform && nextImage )
      {
      XformPaths.push_back( nextXform );
      ImagePaths.push_back( nextImage );

      nextXform = cl.GetNextOptional();
      nextImage = cl.GetNextOptional();
      }
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    return 1;
    }

  if ( ExcludeFirstImage )
    ReconstructionGridPath = ImagePaths[0];

  if ( ReconstructionGridPath )
    {
    ReconGrid = cmtk::UniformVolume::SmartPtr( cmtk::VolumeIO::ReadOriented( ReconstructionGridPath ) );
    if ( ! ReconGrid )
      {
      cmtk::StdErr << "ERROR: Could not read reconstruction grid from image " << ReconstructionGridPath << "\n";
      throw cmtk::ExitException( 1 );
      }
    }
  
  for ( size_t idx = (ExcludeFirstImage?1:0); idx < ImagePaths.size(); ++idx )
    {
    cmtk::UniformVolume::SmartPtr image( cmtk::VolumeIO::ReadOriented( ImagePaths[idx] ) );
    if ( ! image || ! image->GetData() )
      {
      cmtk::StdErr << "ERROR: Could not read image " << ImagePaths[idx] << "\n";
      throw cmtk::ExitException( 1 );
      }

    cmtk::Xform::SmartPtr xform( new cmtk::AffineXform );
    if ( XformPaths[idx] && strcmp( XformPaths[idx], "--" ) )
      {
      xform = cmtk::Xform::SmartPtr( cmtk::XformIO::Read( XformPaths[idx] ) );
      if ( ! xform )
	{
	cmtk::StdErr << "ERROR: Could read transformation from file" << XformPaths[idx] << "\n";
	}
      }

    cmtk::AffineXform::SmartPtr affineXform( cmtk::AffineXform::SmartPtr::DynamicCastFrom( xform ) );
    if ( affineXform && (affineXform->GetMetaInfo( cmtk::META_SPACE ) != cmtk::AnatomicalOrientation::ORIENTATION_STANDARD) )
      {
      try
	{
	cmtk::TransformChangeFromSpaceAffine toStandardSpace( *affineXform, *ReconGrid, *image );
	*affineXform = toStandardSpace.GetTransformation();
	}
      catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
	{
	cmtk::StdErr << "ERROR: singular matrix encountered in cmtk::TransformChangeFromSpaceAffine constructor\n";
	throw cmtk::ExitException( 1 );
	}      
      affineXform->SetMetaInfo( cmtk::META_SPACE, cmtk::AnatomicalOrientation::ORIENTATION_STANDARD );
      }

    Images.push_back( image );
    Xforms.push_back( xform );
    }

  if ( ! ReconGrid )
    {
    // No recon grid from command line: use first input image.
    ReconGrid = Images[0];
    }
  else
    {
    // If we have a pre-defined reconstruction grid, make its physical coordinates match the first input image
    // First, get the recon grid offset
    const cmtk::UniformVolume::CoordinateVectorType offset = ReconGrid->m_Offset;
    // Convert offset to input image index coordinates
    const cmtk::UniformVolume::CoordinateVectorType indexOffset = cmtk::ComponentDivide( offset, Images[0]->m_Delta );
    // New offset is the index grid offset transformed to physical space
    const cmtk::UniformVolume::CoordinateVectorType newOffset = Images[0]->IndexToPhysical( indexOffset );
    // Copy image-to-physical matrix from input to recon image
    ReconGrid->SetImageToPhysicalMatrix( Images[0]->GetImageToPhysicalMatrix() );
    // Finally, copy new offset into recon image-to-physical matrix.
    for ( int i = 0; i < 3; ++i )
      {
      ReconGrid->m_IndexToPhysicalMatrix[3][i] = newOffset[i];
      }
    }
  
  if ( UseCropRegion )
    {
    ReconGrid->CropRegion() = CropRegion;
    ReconGrid = cmtk::UniformVolume::SmartPtr( ReconGrid->GetCroppedVolume() );
    }

  cmtk::DebugOutput( 1 ).GetStream().printf( "Reconstruction grid: %dx%dx%d pixels, %fx%fx%f pixel size, offset=%f,%f,%f\n",
					     ReconGrid->m_Dims[0], ReconGrid->m_Dims[1], ReconGrid->m_Dims[2], (float)ReconGrid->m_Delta[0], (float)ReconGrid->m_Delta[1], (float)ReconGrid->m_Delta[2],
					     (float)ReconGrid->m_Offset[0], (float)ReconGrid->m_Offset[1], (float)ReconGrid->m_Offset[2] );
  
  cmtk::VolumeInjectionReconstruction injection( ReconGrid, Images );
  try
    {
    injection.SetTransformationsToPassImages( Xforms );
    }
  catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
    {
    cmtk::StdErr << "ERROR: singular matrix encountered in cmtk::VolumeInjection::SetTransformationsToPassImages()\n";
    throw cmtk::ExitException( 1 );
    }
  
  for ( std::map<size_t,float>::const_iterator it = PassWeights.begin(); it != PassWeights.end(); ++it )
    {
    injection.SetPassWeight( it->first, it->second );
    }
  
  try
    {
    if ( VolumeInjectionIsotropic )
      injection.VolumeInjectionIsotropic( VolumeInjectionSigma, VolumeInjectionRadius );
    else
      injection.VolumeInjectionAnisotropic( VolumeInjectionSigma, VolumeInjectionRadius );
    }
  catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
    {
    cmtk::StdErr << "ERROR: singular coordinate transformation matrix encountered in volume injection function\n";
    throw cmtk::ExitException( 1 );
    }

  if ( OutputImagePath )
    {
    WriteOutputImage( injection.GetCorrectedImage(), OutputImagePath );
    }
  
  return 0;
}

#include "cmtkSafeMain"
