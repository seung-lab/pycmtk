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

#include "cmtkCommandLine.h"

mxml_node_t*
cmtk::CommandLine::KeyToActionSingle
::MakeXML( mxml_node_t *const parent ) const
{
  if ( ! (this->m_Properties & PROPS_NOXML) )
    {
    return this->Superclass::MakeXML( this->m_Action->MakeXML( parent ) );
    }
  return NULL;
}

void
cmtk::CommandLine::KeyToActionSingle
::PrintHelp( const size_t globalIndent, const bool advanced ) const
{
  std::ostringstream fmt;
  this->Superclass::FormatHelp( fmt );

  if ( ((this->m_Action->GetProperties() & Self::PROPS_ADVANCED)==0) || advanced )
    {
    this->m_Action->PrintHelp( fmt );
    StdOut.FormatText( fmt.str(), CommandLine::HelpTextIndent + globalIndent, StdErr.GetLineWidth(), -CommandLine::HelpTextIndent ) << "\n";  
    }
}

void
cmtk::CommandLine::KeyToActionSingle
::PrintWikiWithPrefix( const std::string& prefix ) const
{
  this->Superclass::PrintWikiWithPrefix( prefix );
  
  this->m_Action->PrintWiki();
  StdOut << "\n";
}

void
cmtk::CommandLine::KeyToActionSingle
::PrintManWithPrefix( const std::string& prefix ) const
{
  this->Superclass::PrintManWithPrefix( prefix );
  
  this->m_Action->PrintMan();
}

bool
cmtk::CommandLine::KeyToActionSingle
::MatchAndExecute( const std::string& key, const size_t argc, const char* argv[], size_t& index )
{
  if ( this->MatchLongOption( std::string( key ) ) )
    {
    this->m_Action->Evaluate( argc, argv, index );
    return true;
    }
  return false;
}

bool
cmtk::CommandLine::KeyToActionSingle
::MatchAndExecute( const char keyChar, const size_t argc, const char* argv[], size_t& index )
{
  if ( this->m_Key.m_KeyChar == keyChar )
    {
    this->m_Action->Evaluate( argc, argv, index );
    return true;
    }

  return false;
}
