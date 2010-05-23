/*
//
//  Copyright 2010 Torsten Rohlfing
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

#ifndef __cmtkImageOperationConnectedComponents_h_included_
#define __cmtkImageOperationConnectedComponents_h_included_

#include <cmtkconfig.h>

#include <cmtkImageOperation.h>
#include <cmtkDataGridMorphologicalOperators.h>

namespace
cmtk
{

/// Image operation: create connected components map.
class ImageOperationConnectedComponents
/// Inherit from image operation base class.
  : public ImageOperation
{
public:
  /// Constructor:
  ImageOperationConnectedComponents( const bool sortBySize ) : m_SortBySize( sortBySize ) {}
  
  /// Apply this operation to an image in place.
  virtual cmtk::UniformVolume::SmartPtr  Apply( cmtk::UniformVolume::SmartPtr& volume )
  {
    cmtk::DataGridMorphologicalOperators ops( volume );
    volume->SetData( ops.GetConnectedComponents( this->m_SortBySize ) );
    return volume;
  }
  
  /// Create new connected components operation.
  static void New()
  {
    ImageOperation::m_ImageOperationList.push_back( SmartPtr( new ImageOperationConnectedComponents( false ) ) );
  }

  /// Create new sorted-by-size connected components operation.
  static void NewSorted()
  {
    ImageOperation::m_ImageOperationList.push_back( SmartPtr( new ImageOperationConnectedComponents( true ) ) );
  }

private:
  /// Multi-valued flag: if this is set, a multi-valued boundary map will be created, otherwise a binary map.
  bool m_SortBySize;
};

} // namespace cmtk

#endif // #ifndef __cmtkImageOperationConnectedComponents_h_included_
