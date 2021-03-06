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

#include <System/cmtkConsole.h>

template<class TParam> 
void
cmtk::ThreadPoolGCD::Run
( const Self::TaskFunction taskFunction, std::vector<TParam>& taskParameters, const size_t numberOfTasksOverride )
{
  std::vector<void*> taskArgPtr( taskParameters.size() );
  for ( size_t i = 0; i < taskParameters.size(); ++i )
    {
    taskArgPtr[i] = &taskParameters[i];
    }

  this->Dispatch( taskFunction, taskArgPtr, numberOfTasksOverride );
}
