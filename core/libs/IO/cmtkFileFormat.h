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

#ifndef __cmtkFileFormat_h_included_
#define __cmtkFileFormat_h_included_

#include <cmtkconfig.h>

namespace 
cmtk
{

/** \addtogroup IO */
//@{

/// ID codes for known file formats.
typedef enum {
  /// File of this name does not exist.
  FILEFORMAT_NEXIST = 0,
  /// Path is a typedstream study directory.
  FILEFORMAT_STUDY = 1,
  /// Path is a typedstream studylist directory.
  FILEFORMAT_STUDYLIST = 2,
  /// Path is a typedstream archive file.
  FILEFORMAT_TYPEDSTREAM = 3,
  /// Path is a PGM image.
  FILEFORMAT_PGM = 4,
  /// Path is a DICOM image.
  FILEFORMAT_DICOM = 5,
  /// Path is a Vanderbilt image description file.
  FILEFORMAT_VANDERBILT = 6,
  /// Path is a GIPL (Guy's Hospital Image Processing Lab) image file.
  FILEFORMAT_GIPL = 7,
  /// Path is a Amira image file.
  FILEFORMAT_AMIRA = 8,
  /// Path is some raw binary file (2-D).
  FILEFORMAT_RAW = 9,
  /// Path is some raw binary file (3-D).
  FILEFORMAT_RAW3D = 10,
  /// Path is an Accuray image file
  FILEFORMAT_ACCURAY = 11,
  /// Path is a BioRad .PIC image file.
  FILEFORMAT_BIORAD = 12,
  /// Path is a NRRD file.
  FILEFORMAT_NIFTI_DETACHED = 13,
  /// Path is a NRRD file.
  FILEFORMAT_NIFTI_SINGLEFILE = 14,
  /// Path is an Analyze AVW file.
  FILEFORMAT_ANALYZE_AVW = 15,
  /// Path is a MetaImage file.
  FILEFORMAT_METAIMAGE = 16,
  /// Path is a NRRD file.
  FILEFORMAT_NRRD = 17,
  /// Path is an Analyze header file in little endian.
  FILEFORMAT_ANALYZE_HDR = 18,
  /// Path is an Analyze header file in little endian.
  FILEFORMAT_ANALYZE_HDR_BIGENDIAN = 19,
  /// Path is an ITK transformation file.
  FILEFORMAT_ITK_TFM = 20,
  /** File type cannot be determined.
   * This ID always has to be the last one!
   */
  FILEFORMAT_UNKNOWN
} FileFormatID;

/// Table of file format ID names.
extern const char* FileFormatName[];

/** Identify file (and directory) formats.
 */
class FileFormat 
{
public:
  /** Identify file or directory with given path.
   * Compressed files as supported by CompressedStream are handled. If both
   * an uncompressed and a compressed file exist for the same path prefix, then
   * the uncompressed file has precedence.
   */
  static FileFormatID Identify( const char* path );

  /** Return ID for given file format name.
   */
  static FileFormatID GetID( const char* name );

  /** Return textual description of identified file format.
   */
  static const char* Describe( const FileFormatID id );

  /** Return textual description of a file's format.
   */
  static const char* Describe( const char* path ) 
  {
    return Describe( Identify( path ) );
  }

private:
  /** Identify directory with given path.
   */
  static FileFormatID IdentifyDirectory( const char* path );

  /** Identify regular file with given path.
   */
  static FileFormatID IdentifyFile( const char* path );
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkFileFormat_h_included_
