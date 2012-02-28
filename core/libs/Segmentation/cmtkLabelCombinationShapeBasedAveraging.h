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

#ifndef __cmtkLabelCombinationShapeBasedAveraging_h_included_
#define __cmtkLabelCombinationShapeBasedAveraging_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkUniformVolume.h>

#include <System/cmtkSmartPtr.h>
#include <System/cmtkSmartConstPtr.h>

#include <vector>

namespace
cmtk
{

/** \addtogroup Segmentation */
//@{

/** Label image combination by Shape Based Averaging.
 *\see T. Rohlfing and C. R. Maurer, Jr., “Shape-based averaging,” IEEE Transactions on Image Processing, vol. 16, no. 1, pp. 153-161, 2007. http://dx.doi.org/10.1109/TIP.2006.884936
 */
class
LabelCombinationShapeBasedAveraging
{
public:
  /// This class.
  typedef LabelCombinationShapeBasedAveraging Self;

  /// Smart pointer to this class.
  typedef SmartPointer<Self> SmartPtr;

  /// Smart pointer to const this class.
  typedef SmartConstPointer<Self> SmartConstPtr;

  /// Label index type.
  typedef unsigned short LabelIndexType;

  /// Constructor: compute label combination.
  LabelCombinationShapeBasedAveraging( const std::vector<UniformVolume::SmartConstPtr>& labelImages, const LabelIndexType numberOfLabels = 0 /*!< Number of labels. If zero, the highest label index is determined from the data */ );

  /// Get result.
  TypedArray::SmartPtr GetResult() const;

private:
  /// Number of labels.
  LabelIndexType m_NumberOfLabels;

  /// Vector of label images.
  const std::vector<UniformVolume::SmartConstPtr>& m_LabelImages;

  /// Number of pixels per image.
  size_t m_NumberOfPixels;

  /// Flags for which labels actually exist in the data.
  std::vector<bool> m_LabelFlags;
};

} // namespace cmtk

#endif // #ifndef __cmtkLabelCombinationShapeBasedAveraging_h_included_
