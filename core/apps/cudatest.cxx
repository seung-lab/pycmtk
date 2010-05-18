/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2010 SRI International
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
//  $Revision: 1652 $
//
//  $LastChangedDate: 2010-05-14 14:45:52 -0700 (Fri, 14 May 2010) $
//
//  $LastChangedBy: torstenrohlfing $
//
*/

#include <cmtkconfig.h>

#include <cuda.h>

#include <iostream>

int
main( const int argc, const char*[] )
{
  if ( cuInit(0) != CUDA_SUCCESS )
    {
    std::cerr << "Call to cuInit() failed." << std::endl;
    return 1;
    }

  int version;
  if ( cuDriverGetVersion( &version ) != CUDA_SUCCESS )
    {
    std::cerr << "Call to cuDriverGetVersion() failed." << std::endl;
    return 1;
    }
  
  std::cerr << "Found CUDA driver version " << version << std::endl;
  
  // if we got here, the program probably ran
  return 0;
}

