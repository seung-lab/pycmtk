/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011, 2013 SRI International
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

#include "cmtkProgressConsole.h"

#include <System/cmtkConsole.h>
#include <System/cmtkDebugOutput.h>
#include <System/cmtkTimers.h>

#include <stdlib.h>

namespace
cmtk
{

/** \addtogroup System */
//@{

ProgressConsole::ProgressConsole( const std::string& programName )
  : m_ProgramName( programName ),
    m_TimeAtStart( 0.0 )
{
  this->m_InsideSlicer3 = ( getenv( "Slicer3_HOME" ) != NULL );
    
  if ( this->m_InsideSlicer3 )
    {
    std::cout << "<filter-start>\n"
	      << "<filter-name>" << this->m_ProgramName << "</filter-name>\n"
	      << "<filter-comment> \"" << this->m_ProgramName << "\" </filter-comment>\n"
	      << "</filter-start>\n";
    std::cout.flush();
    }
}

ProgressConsole::~ProgressConsole()
{
  if ( this->m_InsideSlicer3 )
    {
    std::cout << "<filter-end>\n"
	      << "<filter-name>" << this->m_ProgramName << "</filter-name>\n"
	      << "<filter-time>" << Timers::GetTimeProcess() - this->m_TimeAtStart << "</filter-time>\n"
	      << "</filter-end>\n";
    std::cout.flush();
    }
}

Progress::ResultEnum
ProgressConsole::UpdateProgress()
{
  const double fraction = this->GetFractionComplete();

  if ( this->m_InsideSlicer3 )
    {
    std::cout << "<filter-progress>" << fraction << "</filter-progress>\n";
    std::cout.flush();
    }
  else
    {
    const std::string& currentTaskName = this->GetCurrentTaskName();
    if ( currentTaskName.length() )
      {
      DebugOutput( 2 ).GetStream().printf( "%s: %d %%\r", currentTaskName.c_str(), static_cast<int>( 100.0 * fraction ) );
      }
    else
      {
      DebugOutput( 2 ).GetStream().printf( "%d %%\r", static_cast<int>( 100.0 * fraction ) );
      }
    }

  return Self::OK;
}

void
ProgressConsole:: BeginVirtual
( const double start, const double end, const double increment, const std::string& taskName )
{
  this->Superclass::BeginVirtual( start, end, increment, taskName );

  if ( this->IsTopLevel() )
    {
    this->m_TimeAtStart = Timers::GetTimeProcess();
    }
}

} // namespace cmtk
