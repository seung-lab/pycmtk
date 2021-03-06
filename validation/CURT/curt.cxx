/*
//
//  Copyright 2010-2011 SRI International
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
#include <System/cmtkFileUtils.h>

#include <Base/cmtkUniformVolume.h>
#include <Base/cmtkValueSequence.h>

#include <IO/cmtkVolumeIO.h>

#include <vector>
#include <algorithm>

#include <stdio.h>

class
IndexValue
{
public:
  /// Constructor.
  IndexValue( const size_t index = 0, const double value = 0 ) : m_Index( index ), m_Value( value ) {};

  /// Pixel index.
  size_t m_Index;

  /// Pixel value.
  double m_Value;
};

bool
operator<( const IndexValue& x, const IndexValue& y )
{
  return x.m_Value < y.m_Value;
}

int
main( const int argc, const char* argv[] )
{
  const char* pathFix = NULL;
  const char* pathMov = NULL;

  std::vector<std::string> pathsLbls;

  bool padZero = false;

  const char* ic_error_path = NULL;

  try
    {
    cmtk::CommandLine cl( cmtk::CommandLine::PROPS_XML );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Completely Useless Registration Tool" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "For experimental demonstration ONLY! This program coregisters two intensity images by simple permutation and reformats one or more matching label maps accordingly." );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_CATEG, "CMTK.Validation" );

    typedef cmtk::CommandLine::Key Key;
    cl.BeginGroup( "Preprocessing", "Input Image Preprocessing" );
    cl.AddSwitch( Key( "pad" ), &padZero, true, "Pad (ignore) zero-filled areas in both images" );
    cl.EndGroup();

    cl.AddOption( Key( "ic-error-file" ), &ic_error_path, "If given, inverse consistency error statistics are written to this file." );

    cl.AddParameter( &pathFix, "FixedImage", "Fixed image path" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameter( &pathMov, "MovingImage", "Moving image path" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    cl.AddParameterVector( &pathsLbls, "LabelImages", "Label image paths" )->SetProperties( cmtk::CommandLine::PROPS_IMAGE );
    
    cl.Parse( argc, argv );
    }
  catch ( const cmtk::CommandLine::Exception& e )
    {
    cmtk::StdErr << e << "\n";
    exit( 1 );
    }

  cmtk::UniformVolume::SmartPtr refImage( cmtk::VolumeIO::ReadOriented( pathFix ) );
  cmtk::UniformVolume::SmartPtr fltImage( cmtk::VolumeIO::ReadOriented( pathMov ) );

  // Get sorted list of all reference pixels
  const size_t nPixelsRef = refImage->GetNumberOfPixels();
  std::vector<IndexValue> refIndexValue( nPixelsRef );
  size_t nRef = 0;
  for ( size_t i = 0; i < nPixelsRef; ++i )
    {
    const cmtk::Types::DataItem value = refImage->GetDataAt( i );
    if ( value || !padZero )
      {
      refIndexValue[nRef++] = IndexValue( i, value );
      }
    }
  refIndexValue.resize( nRef );
  std::sort( refIndexValue.begin(), refIndexValue.end() );
  
  // Get sorted list of all floating pixels
  const size_t nPixelsFlt = fltImage->GetNumberOfPixels();
  std::vector<IndexValue> fltIndexValue( nPixelsFlt );
  size_t nFlt = 0;
  for ( size_t i = 0; i < nPixelsFlt; ++i )
    {
    const cmtk::Types::DataItem value = fltImage->GetDataAt( i );
    if ( value || !padZero )
      {
      fltIndexValue[nFlt++] = IndexValue( i, value );
      }
    }
  fltIndexValue.resize( nFlt );
  std::sort( fltIndexValue.begin(), fltIndexValue.end() );
  
  const double factor = double( nFlt ) / double( nRef );
  
  for ( size_t i = 0; i < nRef; ++i )
    {
    refImage->SetDataAt( fltImage->GetDataAt( fltIndexValue[static_cast<size_t>(0.5+i*factor)].m_Index ), refIndexValue[i].m_Index );
    }
  
  cmtk::VolumeIO::Write( *refImage, "reformat.nii" );

  for ( size_t l = 0; l < pathsLbls.size(); ++l )
    {
    cmtk::UniformVolume::SmartPtr lblImage( cmtk::VolumeIO::ReadOriented( pathsLbls[l].c_str() ) );
    
    for ( size_t i = 0; i < nRef; ++i )
      {
      refImage->SetDataAt( lblImage->GetDataAt( fltIndexValue[static_cast<size_t>(0.5+i*factor)].m_Index ), refIndexValue[i].m_Index );
      }
    
    char output[PATH_MAX];
    snprintf( output, PATH_MAX, "labels%d.nii", static_cast<int>( l ) );
    cmtk::VolumeIO::Write( *refImage, output );    
    }

  if ( ic_error_path )
    {
    cmtk::ValueSequence<double> sequence;

    for ( size_t i = 0; i < nRef; ++i )
      {
      const cmtk::Vector3D v0 = refImage->GetGridLocation( refIndexValue[i].m_Index );
      const cmtk::Vector3D v1 = refImage->GetGridLocation( refIndexValue[static_cast<size_t>( 0.5+static_cast<size_t>(0.5+i*factor)/factor)].m_Index );
      
      const double error = sqrt( (v0-v1).SumOfSquares() );
      sequence.Proceed( error );
      }

    if ( ! cmtk::FileUtils::RecursiveMkPrefixDir( ic_error_path ) )
      {
      FILE* fp = fopen( ic_error_path, "w" );
      if ( fp )
	{
	fprintf( fp, "MIN\t%lf\nMAX\t%lf\nMEAN\t%lf\nSDEV\t%lf\n", sequence.GetMinimum(), sequence.GetMaximum(), sequence.GetAverage(), sqrt( sequence.GetVariance() ) );
	fclose( fp );
	}
      }
    else
      {
      cmtk::StdErr << "Could not open file '" << ic_error_path << "' for writing\n";
      exit( 1 );
      }
    }
}
