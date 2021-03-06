/*
//
//  Copyright 2004-2014 SRI International
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
#include <System/cmtkCoverity.h>

#include <IO/cmtkStudy.h>
#include <IO/cmtkStudyImageSet.h>
#include <IO/cmtkVolumeFromStudy.h>
#include <IO/cmtkVolumeFromFile.h>
#include <IO/cmtkVolumeIO.h>

#include <algorithm>
#include <sstream>

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
      if ( (*it)->GetTagValue( DCM_ImagePositionPatient ) == newImage.GetTagValue( DCM_ImagePositionPatient ) )
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
    if ( newImage->m_InstanceNumber < (*it)->m_InstanceNumber ) break;
  insert( it, newImage );
}

std::vector<double>
ImageStackDICOM::AssembleSliceTimes() const
{
  std::vector<double> sliceTimes;
  for ( const_iterator it = this->begin(); it != this->end(); ++it ) 
    {
    // add this file's slice times to stack slice time vector - this should be safe even if file slice time vector is empty
    sliceTimes.insert( sliceTimes.end(), (*it)->m_SliceTimes.begin(), (*it)->m_SliceTimes.end() );
    }

  return sliceTimes;
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
    { "dicom:Manufacturer",                 { "\t", NULL, NULL, "\n" } },
    { "dicom:ManufacturerModel",            { "\t", NULL, NULL, "\n" } },
    { "dicom:DeviceSerialNumber",           { "\t", NULL, NULL, "\n" } },
    { "dicom:StationName",                  { "\t", NULL, NULL, "\n" } },
    { "dicom:RepetitionTime",               { "\t", NULL, NULL, "\n" } },
    { "dicom:EchoTime",                     { "\t", NULL, NULL, "\n" } },
    { "dicom:InversionTime",                { "\t", NULL, NULL, "\n" } },
    { "dicom:ImagingFrequency",             { "\t", NULL, NULL, "\n" } },
    { "dwellTime",                          { "\t", NULL, NULL, "\n" } },
    { "phaseEncodeDirection",               { "\t", NULL, NULL, "\n" } },
    { "phaseEncodeDirectionSign",           { "\t", NULL, NULL, "\n" } },
    { "dicom:SequenceName",                 { "\t", NULL, NULL, "\n" } },
    { "dicom:GE:PulseSequenceName",         { "\t", NULL, NULL, "\n" } },
    { "dicom:GE:PulseSequenceDate",         { "\t", NULL, NULL, "\n" } },
    { "dicom:GE:InternalPulseSequenceName", { "\t", NULL, NULL, "\n" } },
    { "dicom:GE:EffectiveEchoSpacing",      { "\t", NULL, NULL, "\n" } },
    { "type",                               { "\t", NULL, NULL, "\n" } },
    { "dwi",                                { "\t", "\n", "\t", "\n" } },
    { "bValue",                             { "\t\t", NULL, NULL, "\n" } },
    { "bVector",                            { "\t\t", NULL, NULL, "\n" } },
    { "bVectorImage",                       { "\t\t", NULL, NULL, "\n" } },
    { "bVectorStandard",                    { "\t\t", NULL, NULL, "\n" } },
    { "dcmFileDirectory",                   { "\t", NULL, NULL, "\n" } },
    { "dicom:StudyInstanceUID",             { "\t", NULL, NULL, "\n" } },
    { "dicom:SeriesInstanceUID",            { "\t", NULL, NULL, "\n" } },
    { "dicom:FrameOfReferenceUID",          { "\t", NULL, NULL, "\n" } }, 
    { "dicom:ImageOrientationPatient",      { "\t", NULL, NULL, "\n" } },
    { "sliceTime",                          { "\t", NULL, NULL, "\n" } },
    { "image",                              { "\t", "\n", "\t", "\n" } },
    { "dcmFile",                            { "\t\t", NULL, NULL, "\n" } },
    { "dicom:AcquisitionTime",              { "\t\t", NULL, NULL, "\n" } },
    { "dicom:ImagePositionPatient",         { "\t\t", NULL, NULL, "\n" } },
    { "dicom:RescaleIntercept",             { "\t\t", NULL, NULL, "\n" } },
    { "dicom:RescaleSlope",                 { "\t\t", NULL, NULL, "\n" } },
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
ImageStackDICOM::WriteXML( const std::string& fname, const UniformVolume& volume, const bool includeIdentifiers ) const
{
  mxmlSetWrapMargin( 120 ); // make enough room for indented bVectorStandard
  mxml_node_t *x_root = mxmlNewElement( NULL, "?xml version=\"1.0\" encoding=\"utf-8\"?" );

  // convenience pointer to first image in series
  const ImageFileDICOM* firstImage = this->front();

  if ( includeIdentifiers )
    {
    mxml_node_t *x_device = mxmlNewElement( x_root, "device" );
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_device, "dicom:Manufacturer" ), 0, firstImage->GetTagValue( DCM_Manufacturer ).c_str() ) );
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_device, "dicom:ManufacturerModel" ), 0, firstImage->GetTagValue( DCM_ManufacturerModelName ).c_str() ) );
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_device, "dicom:StationName" ), 0, firstImage->GetTagValue( DCM_StationName ).c_str() ) );
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_device, "dicom:DeviceSerialNumber" ), 0, firstImage->GetTagValue( DCM_DeviceSerialNumber ).c_str() ) );
    }

  std::string modality = firstImage->GetTagValue( DCM_Modality );
  std::transform( modality.begin(), modality.end(), modality.begin(), cmtkWrapToLower );
  
  mxml_node_t *x_modality = mxmlNewElement( x_root, modality.c_str() );
  if ( modality == "mr" )
    {
    mxml_node_t *x_tr = mxmlNewElement( x_modality, "dicom:RepetitionTime");
    mxmlNewReal( x_tr, atof( firstImage->GetTagValue( DCM_RepetitionTime ).c_str() ) );
    mxmlElementSetAttr( x_tr, "units", "ms" );
    
    mxml_node_t *x_te = mxmlNewElement( x_modality, "dicom:EchoTime");
    mxmlNewReal( x_te, atof( firstImage->GetTagValue( DCM_EchoTime ).c_str() ) );
    mxmlElementSetAttr( x_te, "units", "ms" );

    mxml_node_t *x_ti = mxmlNewElement( x_modality, "dicom:InversionTime");
    mxmlNewReal( x_ti, atof( firstImage->GetTagValue( DCM_InversionTime ).c_str() ) );
    mxmlElementSetAttr( x_ti, "units", "ms" );

    Coverity::FakeFree( mxmlNewReal( mxmlNewElement( x_modality, "dicom:ImagingFrequency"), atof( firstImage->GetTagValue( DCM_ImagingFrequency ).c_str() ) ) );

    if ( firstImage->m_DwellTime > 0 )
      {
      mxml_node_t *x_dwell_time = mxmlNewElement( x_modality, "dwellTime");
      mxmlNewReal( x_dwell_time, firstImage->m_DwellTime );
      mxmlElementSetAttr( x_dwell_time, "units", "s" );
      }

    const std::string phaseEncodeDirection = firstImage->GetTagValue( DCM_InPlanePhaseEncodingDirection );
    if ( phaseEncodeDirection != "" )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_modality, "phaseEncodeDirection"), 0, phaseEncodeDirection.c_str() ) );
      }

    if ( firstImage->m_PhaseEncodeDirectionSign != "" )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_modality, "phaseEncodeDirectionSign"), 0, firstImage->m_PhaseEncodeDirectionSign.c_str() ) );
      }

    if ( firstImage->GetTagValue( DCM_GE_EffectiveEchoSpacing ) != "" )
      {
      Coverity::FakeFree( mxmlNewReal( mxmlNewElement( x_modality, "dicom:GE:EffectiveEchoSpacing"), atof( firstImage->GetTagValue( DCM_GE_EffectiveEchoSpacing ).c_str() ) ) );
      }

    if ( firstImage->GetTagValue( DCM_SequenceName ) != "" && includeIdentifiers )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_modality, "dicom:SequenceName"), 0, firstImage->GetTagValue( DCM_SequenceName ).c_str() ) );
      }
    
    if ( firstImage->GetTagValue( DCM_GE_PulseSequenceName ) != "" && includeIdentifiers )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_modality, "dicom:GE:PulseSequenceName"), 0, firstImage->GetTagValue( DCM_GE_PulseSequenceName ).c_str() ) );
      }
    
    if ( firstImage->GetTagValue( DCM_GE_PulseSequenceDate ) != "" && includeIdentifiers )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_modality, "dicom:GE:PulseSequenceDate"), 0, firstImage->GetTagValue( DCM_GE_PulseSequenceDate ).c_str() ) );
      }
    
    if ( firstImage->GetTagValue( DCM_GE_InternalPulseSequenceName ) != "" && includeIdentifiers )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_modality, "dicom:GE:InternalPulseSequenceName"), 0, firstImage->GetTagValue( DCM_GE_InternalPulseSequenceName ).c_str() ) );
      }
    
    if ( firstImage->m_RawDataType != "unknown" )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_modality, "type"), 0, firstImage->m_RawDataType.c_str() ) );
      }
    
    if ( firstImage->m_IsDWI )
      {
      mxml_node_t *x_dwi = mxmlNewElement( x_modality, "dwi" );
      
      Coverity::FakeFree( mxmlNewInteger( mxmlNewElement( x_dwi, "bValue"), firstImage->m_BValue ) );

      if ( firstImage->m_HasBVector )
	{
	mxml_node_t *x_bvec = mxmlNewElement( x_dwi, "bVector");
	mxmlElementSetAttr( x_bvec, "coordinateSpace", "LPS" );
	for ( size_t idx = 0; idx < 3; ++idx )
	  {
	  Coverity::FakeFree( mxmlNewReal( x_bvec, firstImage->m_BVector[idx] ) );
	  }
	
	// Determine bVector in image LPS coordinate space:
	// First, create copy of image grid
	UniformVolume::SmartPtr gridLPS = volume.CloneGrid();
	// Make sure still in LPS DICOM coordinate space
	gridLPS->ChangeCoordinateSpace( "LPS" );
	
	try
	  {
	  // Apply inverse of remaining image-to-space matrix to original bVector
	  const UniformVolume::CoordinateVectorType bVectorImage = firstImage->m_BVector * Matrix3x3<Types::Coordinate>( gridLPS->GetImageToPhysicalMatrix().GetInverse() );
	  
	  mxml_node_t *x_bvec_image = mxmlNewElement( x_dwi, "bVectorImage");
	  mxmlElementSetAttr( x_bvec_image, "imageOrientation", gridLPS->GetMetaInfo( META_IMAGE_ORIENTATION ).c_str() );
	  for ( size_t idx = 0; idx < 3; ++idx )
	    {
	    Coverity::FakeFree( mxmlNewReal( x_bvec_image, bVectorImage[idx] ) );
	    }
	  }
	catch ( const AffineXform::MatrixType::SingularMatrixException& )
	  {
	  StdErr << "WARNING: singular image-to-physical matrix; cannot determine b vector orientation in image space (bVectorImage).\n";
	  }
	
	// Determine bVector in image RAS standard coordinate space:
	// First, create copy of image grid
	UniformVolume::SmartPtr gridRAS = gridLPS->GetReoriented();
	
	try
	  {
	  // Apply inverse of remaining image-to-space matrix to original bVector
	  const UniformVolume::CoordinateVectorType bVectorStandard = firstImage->m_BVector * Matrix3x3<Types::Coordinate>( gridRAS->GetImageToPhysicalMatrix().GetInverse() );
	  
	  mxml_node_t *x_bvec_std = mxmlNewElement( x_dwi, "bVectorStandard" );
	  mxmlElementSetAttr( x_bvec_std, "imageOrientation", gridRAS->GetMetaInfo( META_IMAGE_ORIENTATION ).c_str() );
	  for ( size_t idx = 0; idx < 3; ++idx )
	    {
	    Coverity::FakeFree( mxmlNewReal( x_bvec_std, bVectorStandard[idx] ) );
	    }
	  }
	catch ( const AffineXform::MatrixType::SingularMatrixException& )
	  {
	  StdErr << "WARNING: singular image-to-physical matrix; cannot determine b vector orientation in standard space (bVectorStandard).\n";
	  }
	}
      }
    }
    
  mxml_node_t *x_stack = mxmlNewElement( x_root, "stack" );

  if ( includeIdentifiers )
    {
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_stack, "dcmFileDirectory" ), 0, firstImage->m_FileDir.c_str() ) );
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_stack, "dicom:StudyInstanceUID" ), 0, firstImage->GetTagValue( DCM_StudyInstanceUID ).c_str() ) );
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_stack, "dicom:SeriesInstanceUID" ), 0, firstImage->GetTagValue( DCM_SeriesInstanceUID ).c_str() ) );

    if ( firstImage->GetTagValue( DCM_FrameOfReferenceUID, "missing" ) != "missing" )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_stack, "dicom:FrameOfReferenceUID" ), 0, firstImage->GetTagValue( DCM_FrameOfReferenceUID ).c_str() ) );
      }
    }

  Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_stack, "dicom:ImageOrientationPatient" ), 0, firstImage->GetTagValue( DCM_ImageOrientationPatient ).c_str() ) );

  for ( const_iterator it = this->begin(); it != this->end(); ++it ) 
    {
    mxml_node_t *x_image = mxmlNewElement( x_stack, "image" );

    if ( includeIdentifiers )
      {
      Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_image, "dcmFile" ), 0, (*it)->m_FileName.c_str() ) );
      }

    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_image, "dicom:AcquisitionTime" ), 0, (*it)->GetTagValue( DCM_AcquisitionTime ).c_str() ) );
    Coverity::FakeFree( mxmlNewText( mxmlNewElement( x_image, "dicom:ImagePositionPatient" ), 0, (*it)->GetTagValue( DCM_ImagePositionPatient ).c_str() ) );

    if ( (*it)->GetTagValue( DCM_RescaleIntercept, "missing" ) != "missing" )
      {
      Coverity::FakeFree( mxmlNewReal( mxmlNewElement( x_image, "dicom:RescaleIntercept" ), atof( (*it)->GetTagValue( DCM_RescaleIntercept ).c_str() ) ) );
      }
      
    if ( (*it)->GetTagValue( DCM_RescaleSlope, "missing" ) != "missing" )
      {
      Coverity::FakeFree( mxmlNewReal( mxmlNewElement( x_image, "dicom:RescaleSlope" ), atof( (*it)->GetTagValue( DCM_RescaleSlope ).c_str() ) ) );
      }
    }

  // put slice times into XML (if we have them)
  const std::vector<double> stackSliceTimes = this->AssembleSliceTimes();
  if ( ! stackSliceTimes.empty() )
    {
    const double baseTime = *std::min_element( stackSliceTimes.begin(),stackSliceTimes.end() );
    for ( size_t stackSlice = 0; stackSlice < stackSliceTimes.size(); ++stackSlice )
      {
      mxml_node_t *x_slice_time = mxmlNewElement( x_stack, "sliceTime" );
      Coverity::FakeFree( mxmlNewReal( x_slice_time, stackSliceTimes[stackSlice]-baseTime ) );
      
      char slice_str[10];
      snprintf( slice_str, 9, "%u", static_cast<unsigned int>( stackSlice ) );	
      mxmlElementSetAttr( x_slice_time, "slice", slice_str );
      }
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
    StdErr << "ERROR: could not open file " << fname << " for writing\n";
    }

  Coverity::FakeFree( x_modality );
  
  mxmlDelete( x_root );
}

cmtk::UniformVolume::SmartConstPtr
ImageStackDICOM::WriteImage( const std::string& fname, const Self::EmbedInfoEnum embedInfo ) const
{
  const ImageFileDICOM *first = this->front();
    
  UniformVolume::SmartPtr volume;
  if ( !first->m_IsMultislice )
    {
    StudyImageSet studyImageSet;
    
    studyImageSet.SetImageFormat( FILEFORMAT_DICOM );
    studyImageSet.SetImageDirectory( first->m_FileDir.c_str() );
    studyImageSet.SetMultiFile( true );
    
    for ( const_iterator it = this->begin(); it != this->end(); ++it ) 
      {
      studyImageSet.push_back( (*it)->m_FileName );
      }
    
    volume = VolumeFromStudy::Read( &studyImageSet, this->m_Tolerance );
    }
  else
    {
    char fullPath[PATH_MAX];
#ifdef MSC_VER
    snprintf( fullPath, sizeof( fullPath ), "%s\\%s", first->m_FileDir.c_str(), first->m_FileName.c_str() );
#else
    snprintf( fullPath, sizeof( fullPath ), "%s/%s", first->m_FileDir.c_str(), first->m_FileName.c_str() );
#endif

    volume = VolumeFromFile::ReadDICOM( fullPath );
    }

  if ( volume )
    {
    switch ( embedInfo )
      {
      default:
      case EMBED_NONE:
	break;
      case EMBED_STUDYID_STUDYDATE:
	volume->SetMetaInfo( META_IMAGE_DESCRIPTION, first->GetTagValue( DCM_StudyID ) + "_" + first->GetTagValue( DCM_StudyDate ) );
	break;
      case EMBED_PATIENTNAME:
	volume->SetMetaInfo( META_IMAGE_DESCRIPTION, first->GetTagValue( DCM_PatientsName ) );
	break;
      case EMBED_SERIESDESCR:
	volume->SetMetaInfo( META_IMAGE_DESCRIPTION, first->GetTagValue( DCM_SeriesDescription ) );
	break;
      }

    // see if we have Phase Encode direction and set metadata (for NIFTI only at this time)
    const std::string phaseEncodeDirection = first->GetTagValue( DCM_InPlanePhaseEncodingDirection );
    if ( ! phaseEncodeDirection.empty() )
      {
      volume->SetMetaInfo( META_IMAGE_SLICE_PEDIRECTION, phaseEncodeDirection );
      }

    // see if we have slice times and set metadata accordingly (relevant for NIFTI only at this time)
    const std::vector<double> sliceTimes = this->AssembleSliceTimes();
    if ( sliceTimes.size() > 1 ) // need at least 2 slices for meaningful slice order
      {
      std::vector<double> sliceTimesSorted = sliceTimes;
      std::sort( sliceTimesSorted.begin(), sliceTimesSorted.end() );

      // This next bit inspired by Xiangru Li's Matlab DICOM-to-NIfTI converter, http://www.mathworks.com/matlabcentral/fileexchange/42997-dicom-to-nifti-converter
      double duration = 0;
      for ( size_t i = 1; i < sliceTimes.size(); ++i )
	{
	duration += fabs(sliceTimes[i]-sliceTimes[i-1]);
	}
      duration /= (sliceTimes.size()-1);
      const double difference = (sliceTimesSorted[sliceTimesSorted.size()-1]-sliceTimesSorted[0]) / (sliceTimes.size()-1);

      std::string sliceOrder;
      if ( fabs( difference - duration ) < 1e-6 )
	{
	sliceOrder = META_IMAGE_SLICEORDER_SI;
	}
      else if ( fabs( difference + duration ) < 1e-6 )
	{
	sliceOrder = META_IMAGE_SLICEORDER_SD;
	}
      else if ( difference > 0 )
	{
	if ( sliceTimes[0] < sliceTimes[1] ) 
	  {
	  // EVEN slices acquired first.
	  sliceOrder = META_IMAGE_SLICEORDER_AI;
	  }
	else
	  {
	  // ODD slices acquired first
	  sliceOrder = META_IMAGE_SLICEORDER_AI2;
	  }
	}
      else
	{
	if ( sliceTimes[sliceTimes.size()-1] < sliceTimes[sliceTimes.size()-2] ) 
	  {
	  // EVEN slices acquired first.
	  sliceOrder = META_IMAGE_SLICEORDER_AD;
	  }
	else
	  {
	  // ODD slices acquired first
	  sliceOrder = META_IMAGE_SLICEORDER_AD2;
	  }
	}

      if ( ! sliceOrder.empty() )
	{
	volume->SetMetaInfo( META_IMAGE_SLICEORDER, sliceOrder );

	std::ostringstream strm;
	strm << difference;
	volume->SetMetaInfo( META_IMAGE_SLICEDURATION, strm.str() );
	}
      }
    
    VolumeIO::Write( *volume, fname.c_str() );
    DebugOutput( 1 ).GetStream().printf( "\nOutput file:%s\nImage size: %3dx%3dx%3d pixels\nPixel size: %.4fx%.4fx%.4f mm\n\n", 
					       fname.c_str(), volume->m_Dims[0], volume->m_Dims[1], volume->m_Dims[2], volume->m_Delta[0], volume->m_Delta[1], volume->m_Delta[2] );
    }
  else
    {
    // No longer need to warn - now warn at lower level
    //    StdErr << "WARNING: No valid volume was read.\n";
    }
  
  DebugOutput( 1 ) << "DICOM Information: \n"
			 << "  Description:   " << first->GetTagValue( DCM_SeriesDescription ) << "\n"
			 << "  Series:        " << first->GetTagValue( DCM_SeriesInstanceUID ) << "\n"
			 << "  Study:         " << first->GetTagValue( DCM_StudyInstanceUID ) << "\n"
			 << "  Acquisition:   " << first->m_AcquisitionNumber << "\n"
			 << "  TR / TE:       " << first->GetTagValue( DCM_RepetitionTime ) << "ms / " << first->GetTagValue( DCM_EchoTime ) << "ms\n"
			 << "  Position:      " << first->GetTagValue( DCM_ImagePositionPatient ) << "\n"
			 << "  Orientation:   " << first->GetTagValue( DCM_ImageOrientationPatient ) << "\n"
			 << "  Raw Data Type: " << first->m_RawDataType << "\n";
    
  DebugOutput( 1 ) << "\nImage List:\n";
  for ( const_iterator it = this->begin(); it != this->end(); ++it ) 
    {
    DebugOutput( 1 ) << (*it)->m_FileName << " ";
    }
  DebugOutput( 1 ) << "\n====================================================\n";

  return volume;
}

} // namespace CMTK
