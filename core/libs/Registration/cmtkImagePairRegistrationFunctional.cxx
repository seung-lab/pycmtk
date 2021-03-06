/*
//
//  Copyright 2016 Google, Inc.
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

#include <Registration/cmtkImagePairRegistrationFunctional.h>

#include <Base/cmtkVector.h>

#include <assert.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

void
ImagePairRegistrationFunctional::InitFloating( UniformVolume::SmartConstPtr& floating )
{
  this->m_FloatingGrid = floating;
  
  this->m_FloatingDims = this->m_FloatingGrid->GetDims();
  this->m_FloatingSize = this->m_FloatingGrid->m_Size;
  this->m_FloatingCropRegionCoordinates = this->m_FloatingGrid->GetHighResCropRegion();
  for ( int dim = 0; dim < 3; ++dim ) 
    {
    this->m_FloatingInverseDelta[dim] = 1.0 / this->m_FloatingGrid->m_Delta[dim];
    this->m_FloatingCropRegionFractIndex.From()[dim] = this->m_FloatingCropRegionCoordinates.From()[dim] * this->m_FloatingInverseDelta[dim];
    this->m_FloatingCropRegionFractIndex.To()[dim] = this->m_FloatingCropRegionCoordinates.To()[dim] * this->m_FloatingInverseDelta[dim];
    }
  
  this->m_FloatingDataClass = floating->GetData()->GetDataClass();
}

void
ImagePairRegistrationFunctional::InitReference( UniformVolume::SmartConstPtr& reference )
{
  this->m_ReferenceGrid = reference;

  this->m_ReferenceDims = this->m_ReferenceGrid->GetDims();
  this->m_ReferenceSize = this->m_ReferenceGrid->m_Size;
  this->m_ReferenceCropRegion = this->m_ReferenceGrid->CropRegion();

  for ( int dim = 0; dim < 3; ++dim )
    this->m_ReferenceInverseDelta[dim] = 1.0 / this->m_ReferenceGrid->m_Delta[dim];

  this->m_ReferenceDataClass = reference->GetData()->GetDataClass();
}

const DataGrid::RegionType
ImagePairRegistrationFunctional::GetReferenceGridRange
( const UniformVolume::CoordinateRegionType& region ) const
{
  const DataGrid::IndexType& cropRegionFrom = this->m_ReferenceCropRegion.From();
  const DataGrid::IndexType& cropRegionTo = this->m_ReferenceCropRegion.To();

  DataGrid::IndexType from, to;
  for ( int i = 0; i < 3; ++i )
    {
    from[i] = std::min( cropRegionTo[i]-1, std::max( cropRegionFrom[i], static_cast<Types::GridIndexType>( region.From()[i] * this->m_ReferenceInverseDelta[i] ) ) );
    to[i] = 1+std::max( cropRegionFrom[i], std::min( cropRegionTo[i]-1, 1+static_cast<Types::GridIndexType>( region.To()[i] * this->m_ReferenceInverseDelta[i] ) ) );
    }

  return DataGrid::RegionType( from, to );
}

} // namespace cmtk
