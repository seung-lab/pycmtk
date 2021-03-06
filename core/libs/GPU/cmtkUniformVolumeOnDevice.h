/*
//
//  Copyright 2010 SRI International
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

#ifndef __cmtkUniformVolumeOnDevice_h_included_
#define __cmtkUniformVolumeOnDevice_h_included_

/** \addtogroup GPU */
//@{

namespace
cmtk
{

/// Copy from host to device.
typedef struct 
{
  /// Volume dimensions.
  int m_Dims[3];
  
  /// Pixel sizes.
  float m_Delta[3];
  
  /// Pointer to volume data.
  float* m_Data;
} UniformVolumeOnDevice;

} // namespace cmtk

//@}

#endif // #ifndef __cmtkUniformVolumeOnDevice_h_included_
