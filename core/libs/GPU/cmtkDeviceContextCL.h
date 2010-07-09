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

#ifndef __cmtkDeviceContextCL_h_included_
#define __cmtkDeviceContextCL_h_included_

#include <cmtkconfig.h>

#include <System/cmtkCannotBeCopied.h>

#include <CL/opencl.h>

#include <vector>

namespace
cmtk
{

/** \addtogroup GPU */
//@{

/** Device context wrapper for OpenCL.
 *\warning This is a skeleton only and not currently implemented. The implemented parts
 * do not work because nVidia's OpenCL library is pissy about function call parameters.
 */
class DeviceContextCL
    /// Make sure this is not copied.
  : private CannotBeCopied
{
public:
  /// This class.
  typedef DeviceContextCL Self;

  /// Get the global context instance.
  static Self& GetGlobalContext();

private:
  /// The OpenCL context handle.
  cl_context m_Context;

  /// Vector of device infos.
  std::vector<cl_device_id> m_DeviceIDs;

  /// Constructor.
  DeviceContextCL();

  /// Destructor.
  ~DeviceContextCL();
};

DeviceContextCL::GetGlobalContext()
{
  static Self globalContext;

  return globalContext;
}

//@}

}


#endif // #ifndef __cmtkDeviceContextCL_h_included_
