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
//  $Revision$
//
//  $LastChangedDate$
//
//  $LastChangedBy$
//
*/
/*************************************************************************
Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1987, 1995, 2000 by Stephen L. Moshier

Contributors:
    * Sergey Bochkanov (ALGLIB project). Translation from C to
      pseudocode.

See subroutines comments for additional copyrights.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer listed
  in this license in the documentation and/or other materials
  provided with the distribution.

- Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*************************************************************************/

#ifndef _studenttdistr_h
#define _studenttdistr_h

#include "ap.h"

#include "gammaf.h"
#include "normaldistr.h"
#include "ibetaf.h"

namespace
alglib
{

/*************************************************************************
Student's t distribution

Computes the integral from minus infinity to t of the Student
t distribution with integer k > 0 degrees of freedom:

                                     t
                                     -
                                    | |
             -                      |         2   -(k+1)/2
            | ( (k+1)/2 )           |  (     x   )
      ----------------------        |  ( 1 + --- )        dx
                    -               |  (      k  )
      sqrt( k pi ) | ( k/2 )        |
                                  | |
                                   -
                                  -inf.

Relation to incomplete beta integral:

       1 - stdtr(k,t) = 0.5 * incbet( k/2, 1/2, z )
where
       z = k/(k + t**2).

For t < -2, this is the method of computation.  For higher t,
a direct method is derived from integration by parts.
Since the function is symmetric about t=0, the area under the
right tail of the density is found by calling the function
with -t instead of t.

ACCURACY:

Tested at random 1 <= k <= 25.  The "domain" refers to t.
                     Relative error:
arithmetic   domain     # trials      peak         rms
   IEEE     -100,-2      50000       5.9e-15     1.4e-15
   IEEE     -2,100      500000       2.7e-15     4.9e-17

Cephes Math Library Release 2.8:  June, 2000
Copyright 1984, 1987, 1995, 2000 by Stephen L. Moshier
*************************************************************************/
ap::real_value_type studenttdistribution(int k, ap::real_value_type t);


} // namespace alglib

#endif
