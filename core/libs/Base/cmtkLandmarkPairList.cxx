/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2012 SRI International
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

#include "cmtkLandmarkPairList.h"

namespace
cmtk
{

/** \addtogroup Base */
//@{

void
LandmarkPairList::AddLandmarkLists
( const LandmarkList& sourceList, const LandmarkList& targetList )
{
  LandmarkList::ConstIterator it = sourceList.begin();
  while ( it != sourceList.end() ) 
    {
    const LandmarkList::ConstIterator targetLM = targetList.FindByName( it->m_Name );
    if ( targetLM != targetList.end() ) 
      {
      this->push_back( LandmarkPair( *it, targetLM->m_Location ) );
      }
    ++it;
    }
}

LandmarkPairList::ConstIterator
LandmarkPairList::FindByName( const std::string& name ) const
{
  for ( const_iterator it = this->begin(); it != this->end(); ++it )
    {
    if ( it->m_Name == name )
      {
      return it;
      }
    }
  
  return this->end();
}

LandmarkPairList::Iterator
LandmarkPairList::FindByName( const std::string& name )
{
  for ( Self::Iterator it = this->begin(); it != this->end(); ++it )
    {
    if ( it->m_Name == name )
      {
      return it;
      }
    }
  
  return this->end();
}

std::ostream& 
operator<<( std::ostream& stream, const LandmarkPairList& pairList )
{
  for ( LandmarkPairList::ConstIterator it = pairList.begin(); it != pairList.end(); ++it )
    stream << it->m_Location << "\t" << it->m_TargetLocation << "\t" << it->m_Name << std::endl;
  return stream;
}

} // namespace cmtk
