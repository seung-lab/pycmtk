/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011 SRI International
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

#include <IO/cmtkClassStreamAffineXform.h>
#include <IO/cmtkVolumeIO.h>
#include <System/cmtkConsole.h>

namespace
cmtk
{

/** \addtogroup IO */
//@{

template<class TMetricFunctionalType>
ClassStream& operator << 
  ( ClassStream& stream, const AffineMultiChannelRegistrationFunctional<TMetricFunctionalType>& functional )
{
  stream.Begin( "registration" );

  stream.WriteInt( "reference_channel_count", functional.GetNumberOfReferenceChannels() );
  for ( size_t idx = 0; idx < functional.GetNumberOfReferenceChannels(); ++idx )
    {
    stream.WriteString( "reference_channel", functional.GetReferenceChannel( idx )->GetMetaInfo( META_FS_PATH ).c_str() );
    }

  stream.WriteInt( "floating_channel_count", functional.GetNumberOfFloatingChannels() );
  for ( size_t idx = 0; idx < functional.GetNumberOfFloatingChannels(); ++idx )
    {
    stream.WriteString( "floating_channel", functional.GetFloatingChannel( idx )->GetMetaInfo( META_FS_PATH ).c_str() );
    }
  
  stream << functional.GetTransformation();
  stream.End();

  return stream;
}

template<class TMetricFunctionalType>
ClassStream& operator >>
  ( ClassStream& stream, AffineMultiChannelRegistrationFunctional<TMetricFunctionalType>& functional )
{
  stream.Seek( "registration" );
  
  const size_t referenceChannelCount = stream.ReadInt( "reference_channel_count", 0 );
  for ( size_t idx = 0; idx < referenceChannelCount; ++idx )
    {
    const char* channel = stream.ReadString( "reference_channel", NULL, true /*forward*/ );
    if ( channel )
      {
      UniformVolume::SmartPtr volume( VolumeIO::ReadOriented( channel ) );
      if ( !volume || !volume->GetData() )
	{
	StdErr << "ERROR: Cannot read image " << channel << "\n";
	exit( 1 );
	}
      functional.AddReferenceChannel( volume );
      }
    }

  const size_t floatingChannelCount = stream.ReadInt( "floating_channel_count", 0 );
  for ( size_t idx = 0; idx < floatingChannelCount; ++idx )
    {
    const char* channel = stream.ReadString( "floating_channel", NULL, true /*forward*/ );
    if ( channel )
      {
      UniformVolume::SmartPtr volume( VolumeIO::ReadOriented( channel ) );
      if ( !volume || !volume->GetData() )
	{
	StdErr << "ERROR: Cannot read image " << channel << "\n";
	exit( 1 );
	}
      functional.AddFloatingChannel( volume );
      }
    }
  
  stream >> functional.GetTransformation();
  stream.End();
  
  return stream;
}

template<class TMetricFunctionalType>
ClassStream& operator << 
  ( ClassStream& stream, const SplineWarpMultiChannelRegistrationFunctional<TMetricFunctionalType>& functional )
{
  stream.Begin( "registration" );
  
  stream.WriteInt( "reference_channel_count", functional.GetNumberOfReferenceChannels() );
  for ( size_t idx = 0; idx < functional.GetNumberOfReferenceChannels(); ++idx )
    {
    stream.WriteString( "reference_channel", functional.GetReferenceChannel( idx )->GetMetaInfo( META_FS_PATH ).c_str() );
    }

  stream.WriteInt( "floating_channel_count", functional.GetNumberOfFloatingChannels() );
  for ( size_t idx = 0; idx < functional.GetNumberOfFloatingChannels(); ++idx )
    {
    stream.WriteString( "floating_channel", functional.GetFloatingChannel( idx )->GetMetaInfo( META_FS_PATH ).c_str() );
    }
  
  stream << functional.GetTransformation();
  stream.End();

  return stream;
}

} // namespace cmtk
