/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//  Copyright 2004-2009 SRI International
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
//  $Revision: 5806 $
//
//  $LastChangedDate: 2009-05-29 13:36:00 -0700 (Fri, 29 May 2009) $
//
//  $LastChangedBy: torsten $
//
*/

#ifndef __cmtkIsolineFilter_h_included_
#define __cmtkIsolineFilter_h_included_

#include <cmtkFilter.h>

#include <cmtkImage.h>

namespace
cmtk
{

/** \addtogroup Pipeline */
//@{

/** Filter class for computation of isolines.
 * This filter generates isolines from a given scalar image. The resulting 
 * lines are rendered into an image of the same dimensions. Isolevels are
 * equally spaced in a user-defined range. The number of levels (contours) can
 * also be user-defined.
 */
class IsolineFilter : 
  public Filter<Image,Image> 
{
public:
  /// Create new object.
  static IsolineFilter* New() { return new IsolineFilter; }

  /// Return virtual class name.
  virtual const char* GetClassName() const { return "IsolineFilter"; }

  /// Execute isoline operator.
  virtual void Execute();
  
  /// Lower bound of value range for contour generation.
  igsClassParameter(float,RangeFrom);

  /// Upper bound of value range for contour generation.
  igsClassParameter(float,RangeTo);

  /// Number of contours to generate.
  igsClassParameter(unsigned int,NumberOfLevels);

protected:
  /// Default constructor.
  IsolineFilter() { RangeFrom = 0; RangeTo = 1; NumberOfLevels = 0; }

  /// Destructor.
  virtual ~IsolineFilter() {}
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkIsolineFilter_h_included_
