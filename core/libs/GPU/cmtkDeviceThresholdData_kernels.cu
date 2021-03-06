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

#include "cmtkDeviceThresholdData_kernels.h"

#include "GPU/cmtkCUDA.h"

#include <cuda_runtime_api.h>

__device__
void
cmtkDeviceThresholdDataKernel( float* dest, const int n, const float lowerThreshold, const float upperThreshold )
{
  for ( int i = threadIdx.x; i < n; i += blockDim.x )
    {
      dest[idx] = fmaxf( lowerThreshold, fminf( upperThreshold, dest[idx] ) );
    }
}

void
cmtk::DeviceThresholdData( float* dest, const int n, const float lowerThreshold, const float upperThreshold )
{
  cmtkDeviceThresholdDataKernel<<<1,512>>>( dest, n, lowerThreshold, upperThreshold );
  cmtkCheckLastErrorCUDA;
}
