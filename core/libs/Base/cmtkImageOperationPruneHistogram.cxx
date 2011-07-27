/*
//
//  Copyright 2009-2011 SRI International
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
//  $Revision: 2022 $
//
//  $LastChangedDate: 2010-07-21 15:26:03 -0700 (Wed, 21 Jul 2010) $
//
//  $LastChangedBy: torstenrohlfing $
//
*/

#include "cmtkImageOperationPruneHistogram.h"

cmtk::UniformVolume::SmartPtr
cmtk::ImageOperationPruneHistogram::Apply( cmtk::UniformVolume::SmartPtr& volume )
{
  volume->GetData()->PruneHistogram( this->m_High, this->m_Low, this->m_NumberOfBins );
  return volume;
}