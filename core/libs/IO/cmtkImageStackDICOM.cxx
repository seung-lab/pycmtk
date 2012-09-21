/*
//
//  Copyright 2004-2012 SRI International
//
//  Copyright 1997-2009 Torsten Rohlfing
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
//  $Revision: 4497 $
//
//  $LastChangedDate: 2012-08-24 13:46:21 -0700 (Fri, 24 Aug 2012) $
//
//  $LastChangedBy: torstenrohlfing $
//
*/

#include "cmtkImageStackDICOM.h"

#include <System/cmtkConsole.h>
#include <System/cmtkDebugOutput.h>

#include <IO/cmtkStudy.h>
#include <IO/cmtkStudyImageSet.h>
#include <IO/cmtkVolumeFromStudy.h>
#include <IO/cmtkVolumeFromFile.h>
#include <IO/cmtkVolumeIO.h>

namespace
cmtk
{

bool
ImageStackDICOM::Match ( const ImageFileDICOM& newImage, const Types::Coordinate numericalTolerance, const bool disableCheckOrientation, const bool ignoreAcquisitionNumber ) const
{
  if ( this->empty() ) 
    return true; // first image always matches

  ImageFileDICOM::SmartConstPtr check = this->front();
  if ( check )
    {
    if ( !check->Match( newImage, numericalTolerance, disableCheckOrientation, ignoreAcquisitionNumber ) )
      return 0;

    for ( const_iterator it = this->begin(); it != this->end(); ++it )
      {
      // if we already have an image in same location in this study, 
      // then bump to next study
      if ( (*it)->ImagePositionPatient == newImage.ImagePositionPatient )
	return 0;
      }
    return true;
    }
  else
    return false;
}

void
ImageStackDICOM::AddImageFile ( ImageFileDICOM::SmartConstPtr& newImage )
{
  iterator it = begin();
  for ( ; it != end(); ++it )
    if ( newImage->InstanceNumber < (*it)->InstanceNumber ) break;
  insert( it, newImage );
}

const char *
ImageStackDICOM::WhitespaceWriteMiniXML( mxml_node_t* node, int where)
{
  const char* name = node->value.element.name;
  
  typedef struct _wsLookupType
  {
    /// XML element name.
    const char* name;
    /// Table of whitespace sequences.
    const char* ws[4];
  } wsLookupType;

  static const wsLookupType wsLookup[] = 
  {
    { "manufacturer",    { "\t", NULL, NULL, "\n" } },
    { "model",           { "\t", NULL, NULL, "\n" } },
    { "tr",              { "\t", NULL, NULL, "\n" } },
    { "te",              { "\t", NULL, NULL, "\n" } },
    { "type",            { "\t", NULL, NULL, "\n" } },
    { "dwi",             { "\t", "\n", "\t", "\n" } },
    { "bValue",          { "\t\t", NULL, NULL, "\n" } },
    { "bVector",         { "\t\t", NULL, NULL, "\n" } },
    { "bVectorImage",    { "\t\t", NULL, NULL, "\n" } },
    { "bVectorStandard", { "\t\t", NULL, NULL, "\n" } },
    { "dcmPath",         { NULL, NULL, NULL, "\n" } },
    { "dcmFile",         { "\t", NULL, NULL, "\n" } },
    { NULL, {NULL, NULL, NULL, NULL} }
  };

  if ( (where >= 0) && (where < 4) )
    {
    for ( size_t idx = 0; wsLookup[idx].name; ++idx )
      {
      if ( ! strcmp( name, wsLookup[idx].name ) )
	return wsLookup[idx].ws[where];
      }
    }

  switch ( where )
    {
    case MXML_WS_BEFORE_OPEN:
      return NULL;
    case MXML_WS_AFTER_OPEN:
      return "\n";
    case MXML_WS_BEFORE_CLOSE:
      return NULL;
    case MXML_WS_AFTER_CLOSE:
      return "\n";
    }

  return NULL;
}

// wrap tolower() - on Mac, system function is not compatible with std::transform()
static int cmtkWrapToLower( const int c )
{
  return tolower( c );
}

void
ImageStackDICOM::WriteXML( const std::string& fname, const cmtk::UniformVolume& volume ) const
{
  mxmlSetWrapMargin( 120 ); // make enough room for indented bVectorStandard
  mxml_node_t *x_root = mxmlNewElement( NULL, "?xml version=\"1.0\" encoding=\"utf-8\"?" );

  mxml_node_t *x_device = mxmlNewElement( x_root, "device" );

  mxml_node_t *x_manufacturer = mxmlNewElement( x_device, "manufacturer" );
  mxmlNewText( x_manufacturer, 0, this->front()->Manufacturer.c_str() );
    
  mxml_node_t *x_model = mxmlNewElement( x_device, "model" );
  mxmlNewText( x_model, 0, this->front()->ManufacturerModel.c_str() );

  std::string modality = this->front()->Modality;
  std::transform( modality.begin(), modality.end(), modality.begin(), cmtkWrapToLower );
  
  mxml_node_t *x_modality = mxmlNewElement( x_root, modality.c_str() );
  if ( modality == "mr" )
    {
    mxml_node_t *x_tr = mxmlNewElement( x_modality, "tr");
    mxmlNewReal( x_tr, atof( this->front()->RepetitionTime.c_str() ) );
    
    mxml_node_t *x_te = mxmlNewElement( x_modality, "te");
    mxmlNewReal( x_te, atof( this->front()->EchoTime.c_str() ) );

    if ( this->front()->RawDataType != "unknown" )
      {
      mxml_node_t *x_type = mxmlNewElement( x_modality, "type");
      mxmlNewText( x_type, 0, this->front()->RawDataType.c_str() );
      }
    
    if ( this->front()->IsDWI )
      {
      mxml_node_t *x_dwi = mxmlNewElement( x_modality, "dwi" );
      
      mxml_node_t *x_bval = mxmlNewElement( x_dwi, "bValue");
      mxmlNewInteger( x_bval, this->front()->BValue );
      
      mxml_node_t *x_bvec = mxmlNewElement( x_dwi, "bVector");
      mxmlElementSetAttr( x_bvec, "coordinateSpace", "LPS" );
      for ( size_t idx = 0; idx < 3; ++idx )
	{
	mxmlNewReal( x_bvec, this->front()->BVector[idx] );
	}

      // Determine bVector in image LPS coordinate space:
      // First, create copy of image grid
      cmtk::UniformVolume::SmartPtr gridLPS = volume.CloneGrid();
      // Make sure still in LPS DICOM coordinate space
      gridLPS->ChangeCoordinateSpace( "LPS" );
      // Apply inverse of remaining image-to-space matrix to original bVector
      const cmtk::UniformVolume::CoordinateVectorType bVectorImage = this->front()->BVector * cmtk::Matrix3x3<cmtk::Types::Coordinate>( gridLPS->GetImageToPhysicalMatrix().GetInverse() );
      
      mxml_node_t *x_bvec_image = mxmlNewElement( x_dwi, "bVectorImage");
      mxmlElementSetAttr( x_bvec_image, "imageOrientation", gridLPS->GetMetaInfo( cmtk::META_IMAGE_ORIENTATION ).c_str() );
      for ( size_t idx = 0; idx < 3; ++idx )
	{
	mxmlNewReal( x_bvec_image, bVectorImage[idx] );
	}

      // Determine bVector in image RAS standard coordinate space:
      // First, create copy of image grid
      cmtk::UniformVolume::SmartPtr gridRAS = gridLPS->GetReoriented();
      // Apply inverse of remaining image-to-space matrix to original bVector
      const cmtk::UniformVolume::CoordinateVectorType bVectorStandard = this->front()->BVector * cmtk::Matrix3x3<cmtk::Types::Coordinate>( gridRAS->GetImageToPhysicalMatrix().GetInverse() );
      
      mxml_node_t *x_bvec_std = mxmlNewElement( x_dwi, "bVectorStandard");
      mxmlElementSetAttr( x_bvec_std, "imageOrientation", gridRAS->GetMetaInfo( cmtk::META_IMAGE_ORIENTATION ).c_str() );
      for ( size_t idx = 0; idx < 3; ++idx )
	{
	mxmlNewReal( x_bvec_std, bVectorStandard[idx] );
	}
      }
    }
    
  mxml_node_t *x_dcmpath = mxmlNewElement( x_root, "dcmPath" );
  mxmlNewText( x_dcmpath, 0, this->front()->fpath );

  mxml_node_t *x_stack = mxmlNewElement( x_root, "stack" );

  for ( const_iterator it = this->begin(); it != this->end(); ++it ) 
    {
    mxml_node_t *x_dcmfile = mxmlNewElement( x_stack, "dcmFile" );
    mxmlNewText( x_dcmfile, 0, (*it)->fname );
    }

  FILE *file = fopen( fname.c_str(), "w" );
  if ( file )
    {
    mxmlSaveFile( x_root, file, Self::WhitespaceWriteMiniXML );
    fputs( "\n", file ); // end last line
    fclose( file );
    }
  else
    {
    cmtk::StdErr << "ERROR: could not open file " << fname << " for writing\n";
    }
  
  mxmlDelete( x_root );
}

cmtk::UniformVolume::SmartConstPtr
ImageStackDICOM::WriteImage( const std::string& fname, const Self::EmbedInfoEnum embedInfo ) const
{
  const ImageFileDICOM *first = this->front();
    
  cmtk::UniformVolume::SmartPtr volume;
  if ( !first->IsMultislice )
    {
    cmtk::StudyImageSet studyImageSet;
    
    studyImageSet.SetImageFormat( cmtk::FILEFORMAT_DICOM );
    studyImageSet.SetImageDirectory( first->fpath );
    studyImageSet.SetMultiFile( true );
    
    for ( const_iterator it = this->begin(); it != this->end(); ++it ) 
      {
      studyImageSet.push_back( (*it)->fname );
      }
    
    volume = cmtk::VolumeFromStudy::Read( &studyImageSet );
    }
  else
    {
    char fullPath[PATH_MAX];
#ifdef MSC_VER
    snprintf( fullPath, sizeof( fullPath ), "%s\\%s", first->fpath, first->fname );
#else
    snprintf( fullPath, sizeof( fullPath ), "%s/%s", first->fpath, first->fname );
#endif

    volume = cmtk::VolumeFromFile::ReadDICOM( fullPath );
    }

  if ( volume )
    {
    switch ( embedInfo )
      {
      default:
      case EMBED_NONE:
	break;
      case EMBED_STUDYID_STUDYDATE:
	volume->SetMetaInfo( cmtk::META_IMAGE_DESCRIPTION, first->StudyID + "_" + first->StudyDate );
	break;
	break;
      case EMBED_PATIENTNAME:
	volume->SetMetaInfo( cmtk::META_IMAGE_DESCRIPTION, first->PatientName );
	break;
      case EMBED_SERIESDESCR:
	volume->SetMetaInfo( cmtk::META_IMAGE_DESCRIPTION, first->SeriesDescription );
	break;
      }
    
    cmtk::VolumeIO::Write( *volume, fname.c_str() );
    cmtk::DebugOutput( 1 ).GetStream().printf( "\nOutput file:%s\nImage size: %3dx%3dx%3d pixels\nPixel size: %.4fx%.4fx%.4f mm\n\n", 
					       fname.c_str(), volume->m_Dims[0], volume->m_Dims[1], volume->m_Dims[2], volume->m_Delta[0], volume->m_Delta[1], volume->m_Delta[2] );
    }
  else
    {
    cmtk::StdErr << "WARNING: No valid volume was read.\n";
    }
  
  cmtk::DebugOutput( 1 ) << "DICOM Information: \n"
			 << "  Description:   " << first->SeriesDescription << "\n"
			 << "  Series:        " << first->SeriesUID << "\n"
			 << "  Study:         " << first->StudyUID << "\n"
			 << "  Acquisition:   " << first->AcquisitionNumber << "\n"
			 << "  TR / TE:       " << first->RepetitionTime << "ms /" << first->EchoTime << "ms\n"
			 << "  Position:      " << first->ImagePositionPatient << "\n"
			 << "  Orientation:   " << first->ImageOrientationPatient << "\n"
			 << "  Raw Data Type: " << first->RawDataType << "\n";
    
  cmtk::DebugOutput( 1 ) << "\nImage List:\n";
  for ( const_iterator it = this->begin(); it != this->end(); ++it ) 
    {
    cmtk::DebugOutput( 1 ) << (*it)->fname << " ";
    }
  cmtk::DebugOutput( 1 ) << "\n====================================================\n";

  return volume;
}

} // namespace CMTK
