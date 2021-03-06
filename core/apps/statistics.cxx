/*
//
//  Copyright 1997-2011 Torsten Rohlfing
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

#include <IO/cmtkVolumeIO.h>
#include <IO/cmtkTypedStreamStudylist.h>

#include <Base/cmtkSplineWarpXform.h>
#include <Base/cmtkHistogram.h>
#include <Base/cmtkValueSequence.h>
#include <Base/cmtkMathFunctionWrappers.h>

#include <Registration/cmtkReformatVolume.h>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <list>

#include <sys/types.h>
#include <sys/stat.h>

bool Label = false;
bool LogData = false;
bool ExpData = false;

bool WriteAsColumn = false;
bool OutputExpNotation = false;

const char *MaskFileName = NULL;
bool MaskIsBinary = false;
int MaskOutputAllUpTo = 0;
std::list<const char*> ImageFileNames;

int NumberOfHistogramBins = 256;

std::list<cmtk::Types::DataItem> percentiles;

void
CallbackAddPercentile( const double arg )
{
  percentiles.push_back( static_cast<cmtk::Types::DataItem>( arg ) );
}

void
AnalyzeLabels( const cmtk::UniformVolume& volume, const cmtk::TypedArray* maskData ) 
{
  const cmtk::TypedArray* data = volume.GetData();
  cmtk::Types::DataItemRange range = data->GetRange();
  
  if ( MaskOutputAllUpTo )
    range.m_UpperBound = MaskOutputAllUpTo;

  const unsigned int numberOfLabels = static_cast<unsigned int>( range.m_UpperBound - range.m_LowerBound + 1 );

  // Number of label voxels.
  std::vector<unsigned int> count( numberOfLabels );
  std::fill( count.begin(), count.end(), 0 );

  // Number of label surface voxels.
  std::vector<unsigned int> countSurface( numberOfLabels );
  std::fill( countSurface.begin(), countSurface.end(), 0 );

  // Centers-of-mass for each label
  std::vector<cmtk::UniformVolume::CoordinateVectorType> centerOfMass( numberOfLabels );
  std::fill( centerOfMass.begin(), centerOfMass.end(), cmtk::UniformVolume::CoordinateVectorType( 0.0 ) );

  int index = 0;
  cmtk::Types::DataItem value, neighbor, maskValue;
  for ( int z = 0; z < volume.GetDims()[cmtk::AXIS_Z]; ++z ) 
    {
    for ( int y = 0; y < volume.GetDims()[cmtk::AXIS_Y]; ++y ) 
      {
      for ( int x = 0; x < volume.GetDims()[cmtk::AXIS_X]; ++x, ++index ) 
	{
	if ( maskData && !(maskData->Get( maskValue, index ) && (maskValue != 0) ) )
	  continue;
	
	if ( data->Get( value, index ) && range.InRange( value ) ) 
	  {
	  const int labelIdx = static_cast<int>( value - range.m_LowerBound );

	  ++count[labelIdx];
	  centerOfMass[labelIdx] += volume.GetGridLocation( x, y, z );
	  
	  bool isSurface = false;
	  for ( int dz = -1; (dz < 2) && !isSurface; ++dz )
	    for ( int dy = -1; (dy < 2) && !isSurface; ++dy )
	      for ( int dx = -1; (dx < 2) && !isSurface; ++dx )
		if ( dx || dy || dz )
		  if ( (dx+x)>=0 && (dx+x)<volume.GetDims()[cmtk::AXIS_X] && (dy+y)>=0 && (dy+y)<volume.GetDims()[cmtk::AXIS_Y] && (dz+z)>=0 && (dz+z)<volume.GetDims()[cmtk::AXIS_Z] ) 
		    {
		    const int offset = (x+dx) + volume.GetDims()[cmtk::AXIS_X] * ( ( y+dy ) + volume.GetDims()[cmtk::AXIS_Y] * (z+dz) );
		    if ( data->Get( neighbor, offset ) && ( neighbor != value ) )
		      isSurface = true;
		    }
	  
	  if ( isSurface )
	    ++countSurface[labelIdx];	  
	  }
	}
      }
    }
  
  cmtk::DebugOutput( 1 ) << "idx\t\tcount\t\tsurface\t\tvolume\tCenterOfMass\n";

  const cmtk::Types::Coordinate voxelVolume = volume.m_Delta[0] * volume.m_Delta[1] * volume.m_Delta[2];

  size_t totalCount = 0;
  for ( unsigned int idx = 0; idx < numberOfLabels; ++idx ) 
    {
    if ( count[idx] || MaskOutputAllUpTo ) 
      {
      if ( count[idx] )
	centerOfMass[idx] *= (1.0 / count[idx]);
      
      if ( OutputExpNotation )
	fprintf( stdout, "%03d\t%14d\t%14d\t%.4e\t(%e,%e,%e)\n", 
		 (int)(idx+range.m_LowerBound), count[idx], countSurface[idx], count[idx] * voxelVolume, centerOfMass[idx][0], centerOfMass[idx][1], centerOfMass[idx][2] );
      else
	fprintf( stdout, "%03d\t%14d\t%14d\t%.4f\t(%f,%f,%f)\n", 
		 (int)(idx+range.m_LowerBound), count[idx], countSurface[idx], count[idx] * voxelVolume, centerOfMass[idx][0], centerOfMass[idx][1], centerOfMass[idx][2] );
      totalCount += count[idx];
      }
    }
  
  cmtk::Types::DataItem entropy = 0;
  if ( totalCount )
    {
    for ( unsigned int idx=0; idx < numberOfLabels; ++idx ) 
      {
      if ( count[idx] ) 
	{
	cmtk::Types::DataItem p = ((cmtk::Types::DataItem) count[idx]) / totalCount;
	entropy += p * log( p );
	}
      }
    }
  
  if ( OutputExpNotation )
    fprintf( stdout, "\nEntropy:\t%.5e\n", entropy );
  else
    fprintf( stdout, "\nEntropy:\t%.5f\n", entropy );
}

void
AnalyzeGrey( const cmtk::UniformVolume& volume, const cmtk::TypedArray& maskData ) 
{
  const cmtk::TypedArray* data = volume.GetData();

  cmtk::Histogram<unsigned int> histogram( NumberOfHistogramBins );
  histogram.SetRange( data->GetRange() );
  
  int maxLabel = static_cast<int>( maskData.GetRange().m_UpperBound );
  if ( MaskOutputAllUpTo )
    maxLabel = MaskOutputAllUpTo;
    
  std::vector<bool> maskFlags( maxLabel+1 );
  std::fill( maskFlags.begin(), maskFlags.end(), false );
  
  for ( size_t i = 0; i < maskData.GetDataSize(); ++i )
    {
    cmtk::Types::DataItem l;
    if ( maskData.Get( l, i ) && (l <= maxLabel) )
      maskFlags[static_cast<int>( l )] = true;
    }
  
  if ( ! WriteAsColumn )
    fprintf( stdout, "#M\tmin\tmax\tmean\tsdev\tn\tEntropy\tsum\n" );
  
  for ( int maskSelect = 0; maskSelect <= maxLabel; ++maskSelect )
    {
    cmtk::ValueSequence<cmtk::Types::DataItem> seq;

    if ( maskFlags[maskSelect] )
      {
      histogram.Reset();
      cmtk::Types::DataItem value, maskValue;
      
      size_t index = 0;
      for ( int z = 0; z < volume.GetDims()[cmtk::AXIS_Z]; ++z ) 
	{
	for ( int y = 0; y < volume.GetDims()[cmtk::AXIS_Y]; ++y ) 
	  {
	  for ( int x = 0; x < volume.GetDims()[cmtk::AXIS_X]; ++x, ++index ) 
	    {
	    if ( maskData.Get( maskValue, index ) && (maskValue == maskSelect) )
	      {
	      if ( data->Get( value, index ) ) 
		{
		seq.Proceed( value );
		histogram.Increment( histogram.ValueToBin( value ) );
		}
	      }
	    }
	  }
	}
      }

    if ( seq.GetNValues() || MaskOutputAllUpTo )
      {
      if ( ! WriteAsColumn )
	{
	if ( OutputExpNotation )
	  fprintf( stdout, "%03d\t%.5f\t%.5e\t%.5e\t%.5e\t%d\t%.5e\t%f\n",
		   maskSelect, seq.GetMinimum(), seq.GetMaximum(), seq.GetAverage(), sqrt( seq.GetVariance() ), seq.GetNValues(), histogram.GetEntropy(), seq.GetSum() );
	else
	  fprintf( stdout, "%03d\t%.5f\t%.5f\t%.5f\t%.5f\t%d\t%.5f\t%f\n",
		   maskSelect, seq.GetMinimum(), seq.GetMaximum(), seq.GetAverage(), sqrt( seq.GetVariance() ), seq.GetNValues(), histogram.GetEntropy(), seq.GetSum() );
	}
      }
    }
}

void
AnalyzeGrey( const cmtk::UniformVolume& volume ) 
{
  const cmtk::TypedArray* data = volume.GetData();

  if ( ! WriteAsColumn )
    fprintf( stdout, "min\tmax\tmean\tsdev\tn\tEntropy\tsum\n" );
  
  cmtk::Types::DataItem value;
  cmtk::ValueSequence<cmtk::Types::DataItem> seq;
  
  size_t index = 0;
  for ( int z = 0; z < volume.GetDims()[cmtk::AXIS_Z]; ++z ) 
    {
    for ( int y = 0; y < volume.GetDims()[cmtk::AXIS_Y]; ++y ) 
      {
      for ( int x = 0; x < volume.GetDims()[cmtk::AXIS_X]; ++x, ++index ) 
	{
	if ( data->Get( value, index ) ) 
	  {
	  seq.Proceed( value );
	  }
	}
      }
    }

  if ( seq.GetNValues() )
    {
    if ( WriteAsColumn )
      {
      if ( OutputExpNotation )
	fprintf( stdout, "min\t%.5e\nmax\t%.5e\nmean\t%.5e\nsdev\t%.5e\nn\t%d\nEntropy\t%.5e\nsum\t%.5e\n",
		 seq.GetMinimum(), seq.GetMaximum(), seq.GetAverage(), 
		 sqrt( seq.GetVariance() ), seq.GetNValues(), data->GetEntropy( true /*fractional*/, NumberOfHistogramBins ), seq.GetSum());
      else
	fprintf( stdout, "min\t%.5f\nmax\t%.5f\nmean\t%.5f\nsdev\t%.5f\nn\t%d\nEntropy\t%.5f\nsum\t%f\n",
		 seq.GetMinimum(), seq.GetMaximum(), seq.GetAverage(), 
		 sqrt( seq.GetVariance() ), seq.GetNValues(), data->GetEntropy( true /*fractional*/, NumberOfHistogramBins ), seq.GetSum());
      }
    else
      {
      if ( OutputExpNotation )
	fprintf( stdout, "%.5e\t%.5e\t%.5e\t%.5e\t%d\t%.5e\t%e\n",
		 seq.GetMinimum(), seq.GetMaximum(), seq.GetAverage(), 
		 sqrt( seq.GetVariance() ), seq.GetNValues(), data->GetEntropy( true /*fractional*/, NumberOfHistogramBins ), seq.GetSum() );   
      else
	fprintf( stdout, "%.5f\t%.5f\t%.5f\t%.5f\t%d\t%.5f\t%f\n",
		 seq.GetMinimum(), seq.GetMaximum(), seq.GetAverage(), 
		 sqrt( seq.GetVariance() ), seq.GetNValues(), data->GetEntropy( true /*fractional*/, NumberOfHistogramBins ), seq.GetSum() );   
      }
    }
    
  if ( !percentiles.empty() )
    {
    fprintf( stdout, "PERC\tVALUE\n" );
    for ( std::list<cmtk::Types::DataItem>::const_iterator it = percentiles.begin(); it != percentiles.end(); ++it )
      {
      fprintf( stdout, "%.5f\t%.5f\n", (cmtk::Types::DataItem)(*it), (cmtk::Types::DataItem)data->GetPercentile( *it, NumberOfHistogramBins ) );
      }
    }
}

int
doMain ( const int argc, const char* argv[] ) 
{
  try 
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Image statistics" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "Statistical computations on image pixel intensities, i.e., means and standard deviations" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "statistics [options] ImageFile0 [ImageFile1 ...]" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_CATEG, "CMTK.Statistics and Modeling" );

    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( 'l', "label" ), &Label, true, "Interpret voxel values as labels" );
    cl.AddSwitch( Key( 'g', "gray" ), &Label, false, "Interpret voxel values as gray values (default)" );
    
    cl.AddSwitch( Key( 'L', "log" ), &LogData, true, "Apply log() function to data" );
    cl.AddSwitch( Key( 'e', "exp" ), &ExpData, true, "Apply exp() function to data" );
    
    cl.AddOption( Key( 'n', "num-bins" ), &NumberOfHistogramBins, "Number of histogram bins." );
    
    cl.AddSwitch( Key( 'C', "column" ), &WriteAsColumn, true, "Write statistics in column format rather than line format." );
    cl.AddSwitch( Key( 'E', "e-notation" ), &OutputExpNotation, true, "Write floating point numbers in #e# notation (i.e., 1e-3 instead of 0.001)" ); 
    
    cl.AddOption( Key( 'm', "mask" ), &MaskFileName, "Analyze region based on binary mask file", &MaskIsBinary );
    cl.AddOption( Key( 'M', "Mask" ), &MaskFileName, "Analyze regions separately based on mask file (label field)." );
    cl.AddOption( Key( "mask-output-all-up-to" ), &MaskOutputAllUpTo, "Output results for all mask values up to given value, even such that do not actually appear in the mask." );
    
    cl.AddCallback( Key( 'p', "percentile" ), CallbackAddPercentile, "Add value to list of percentile to compute." );
    
    cl.Parse( argc, argv );
        
    const char* next = cl.GetNext();
    while ( next )
      {
      ImageFileNames.push_back( next );
      next = cl.GetNextOptional();
      }
    }
  catch ( const cmtk::CommandLine::Exception& e ) 
    {
    cmtk::StdErr << e;
    throw cmtk::ExitException( 1 );
    }
  
  cmtk::UniformVolume::SmartPtr maskVolume( NULL );
  cmtk::TypedArray::SmartPtr maskData( NULL );
  if ( MaskFileName ) 
    {
    maskVolume = cmtk::UniformVolume::SmartPtr( cmtk::VolumeIO::ReadOriented( MaskFileName ) );
    if ( ! maskVolume ) 
      {
      cmtk::StdErr << "ERROR: could not read mask file " << MaskFileName << "\n";
      throw cmtk::ExitException( 1 );
      }
    maskData = maskVolume->GetData();
    if ( ! maskData ) 
      {
      cmtk::StdErr << "ERROR: could not read data from mask file " << MaskFileName << "\n";
      throw cmtk::ExitException( 1 );
      }
    
    if ( MaskIsBinary )
      {
      maskData->Binarize();
      }
    }
  
  std::list<const char*>::const_iterator it = ImageFileNames.begin();
  for ( ; it != ImageFileNames.end(); ++it )
    {
    const char* imageFileName = *it;
    cmtk::UniformVolume::SmartPtr volume( cmtk::VolumeIO::ReadOriented( imageFileName ) );
    if ( ! volume ) 
      {
      cmtk::StdErr << "ERROR: could not read image file " << imageFileName << "\n";
      continue;
      }
    
    cmtk::TypedArray::SmartPtr volumeData = volume->GetData();
    if ( ! volumeData ) 
      {
      cmtk::StdErr << "ERROR: could not read data from image file " << imageFileName << "\n";
      continue;
      }

    if ( maskVolume && !volume->GridMatches( *maskVolume ) )
      {
      cmtk::StdErr << "ERROR: mask grid does not match grid of image " << imageFileName << "\n";
      continue;
      }
    cmtk::StdOut.printf( "Statistics for image %s\n", imageFileName );
    
    if ( Label ) 
      {
      AnalyzeLabels( *volume, maskData );
      } 
    else 
      {
      if ( LogData )
	volumeData->ApplyFunctionDouble( cmtk::Wrappers::Log );
      if ( ExpData )
	volumeData->ApplyFunctionDouble( cmtk::Wrappers::Exp );
      if ( maskData )
	AnalyzeGrey( *volume, *maskData );
      else
	AnalyzeGrey( *volume );
      }
    }
  return 0;
}

#include "cmtkSafeMain"
