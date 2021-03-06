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

#include <System/cmtkConsole.h>
#include <System/cmtkCommandLine.h>
#include <System/cmtkExitException.h>

#include <Base/cmtkAffineXform.h>
#include <IO/cmtkXformIO.h>
#include <IO/cmtkClassStreamOutput.h>
#include <IO/cmtkClassStreamAffineXform.h>

#include <list>

const char* OutputName = "average.xform";
bool AppendToOutput = false;
bool InvertOutput = false;
bool IncludeReference = false;

int 
doMain( const int argc, const char* argv[] )
{
  std::list<cmtk::AffineXform::SmartPtr> xformList;
  try 
    {
    cmtk::CommandLine cl;
    cl.SetProgramInfo( cmtk::CommandLine::PRG_TITLE, "Average affine transformations" );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_DESCR, "This tool computes the average of a sequence of user-provided affine coordinate transformations." );
    cl.SetProgramInfo( cmtk::CommandLine::PRG_SYNTX, "average_affine [options] x0 [x1 ...] \n WHERE x0 ... xN is [{-i,--inverse}] affine transformation #. "
		       "(If the first transformation in the sequence is inverted, then '--inverse' must be preceded by '--', i.e., use '-- --inverse xform.path')." );

    typedef cmtk::CommandLine::Key Key;
    cl.AddSwitch( Key( 'r', "include-reference" ), &IncludeReference, true, "Include reference coordinate system in averaging." );
    cl.AddOption( Key( 'o', "outfile" ), &OutputName, "Output transformation." );
    cl.AddSwitch( Key( 'a', "append" ), &AppendToOutput, true, "Append to output file [default: overwrite]." );
    cl.AddSwitch( Key( 'I', "invert-output" ), &InvertOutput, true, "Invert averaged transformation before output [default: no]." );

    cl.Parse( argc, argv );

    const cmtk::AffineXform* firstXform = NULL;
    const char* next = cl.GetNextOptional();
    while ( next ) 
      {
      const bool inverse = ! strcmp( next, "-i" ) || ! strcmp( next, "--inverse" );
      if ( inverse ) 
	next = cl.GetNext();
      
      cmtk::Xform::SmartPtr xform( cmtk::XformIO::Read( next ) );
      if ( ! xform ) 
	{
	cmtk::StdErr << "ERROR: could not read transformation from " << next << "\n";
	throw cmtk::ExitException( 1 );
	}

      cmtk::AffineXform::SmartPtr affine( cmtk::AffineXform::SmartPtr::DynamicCastFrom( xform ) );
      if ( ! affine )
	{
	cmtk::StdErr << "ERROR: transformation " << next << " is not affine.\n";
	throw cmtk::ExitException( 1 );
	}

      if ( inverse )
	{
	try
	  {
	  affine = affine->GetInverse();
	  }
	catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
	  {
	  cmtk::StdErr << "ERROR: singular matrix encountered in cmtk::AffineXform::GetInverse()\n";
	  throw cmtk::ExitException( 1 );
	  }
	}

      if ( firstXform )
	{
	affine->ChangeCenter( cmtk::FixedVector<3,cmtk::Types::Coordinate>::FromPointer( firstXform->RetCenter() ) );
	}
      else
	{
	firstXform = affine;
	}
      
      affine->SetUseLogScaleFactors( true );

      xformList.push_back( affine );
      next = cl.GetNextOptional();
      }
    }
  catch ( const cmtk::CommandLine::Exception& e ) 
    {
    cmtk::StdErr << e << "\n";
    throw cmtk::ExitException( 1 );
    }

  cmtk::AffineXform average;
  average.SetUseLogScaleFactors( true );

  const size_t numberOfXforms = xformList.size();
  if ( numberOfXforms )
    {
    cmtk::CoordinateVector v, vx;
    for ( std::list<cmtk::AffineXform::SmartPtr>::const_iterator xit = xformList.begin(); xit != xformList.end(); ++xit )
      {
      if ( xit == xformList.begin() )
	{
	(*xit)->GetParamVector( v );
	}
      else
	{
	(*xit)->GetParamVector( vx );
	for ( size_t p = 0; p < 12; ++p )
	  {
	  v[p] += vx[p];
	  }
	}
      }
    
    for ( size_t p = 0; p < 12; ++p )
      {
      if ( IncludeReference )
	v[p] /= numberOfXforms+1;
      else
	v[p] /= numberOfXforms;
      }
    
    average.SetParamVector( v );
    }

  cmtk::ClassStreamOutput outStream;
  if ( AppendToOutput )
    outStream.Open( OutputName, cmtk::ClassStreamOutput::MODE_APPEND );
  else
    outStream.Open( OutputName, cmtk::ClassStreamOutput::MODE_WRITE );
  
  if ( InvertOutput )
    {
    try
      {
      outStream << (*average.GetInverse());
      }
    catch ( const cmtk::AffineXform::MatrixType::SingularMatrixException& )
      {
      cmtk::StdErr << "ERROR: singular matrix encountered in cmtk::AffineXform::GetInverse()\n";
      throw cmtk::ExitException( 1 );
      }
    }
  else
    {
    outStream << average;
    }

  return 0;
}

#include "cmtkSafeMain"
