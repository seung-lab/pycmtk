/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011, 2014 SRI International
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

#ifdef _OPENMP
#  include <omp.h>
#endif

namespace
cmtk
{

/** \addtogroup Registration */
//@{

template<class TInterpolator, class Fct> 
TypedArray::SmartPtr
ReformatVolume::ReformatMasked
( const UniformVolume* target, const cmtk::XformList& targetToRef, const cmtk::XformList& refToFloat, Fct& fct, const UniformVolume* floating, TInterpolator& interpolator )
{
  const DataGrid::IndexType& dims = target->GetDims();

  TypedArray::SmartPtr result = TypedArray::Create( fct.GetDataType( *floating ), target->GetNumberOfPixels() );
  if ( fct.UsePaddingValue )
    result->SetPaddingValue( fct.PaddingValue );

  // If there is floating image data (may not be, for Jacobian mapping etc.), then copy its data class
  if ( floating )
    result->SetDataClass( floating->GetData()->GetDataClass() );

  const TypedArray* targetData = target->GetData();
  
  Progress::Begin( 0, dims[2], 1, "Volume reformatting" );
  
#pragma omp parallel for
  for ( int z = 0; z < dims[2]; z++ ) 
    {
    Vector3D vRef;
    Types::DataItem value, targetValue;
    size_t offset = z * dims[0] * dims[1];

#ifdef _OPENMP
    if ( ! omp_get_thread_num() )
#endif
      Progress::SetProgress( z );
    
    for ( int y = 0; y < dims[1]; y++ ) 
      {
      for ( int x = 0; x < dims[0]; x++, offset++ ) 
	{
	if ( !targetData || (targetData->Get( targetValue, offset ) && (targetValue != 0))) 
	  {
	  vRef = target->GetGridLocation( x, y, z );
	  if ( targetToRef.ApplyInPlace( vRef ) && fct( value, vRef, refToFloat, interpolator ) ) 
	    {
	    result->Set( value, offset );
	    } 
	  else
	    {
	    result->SetPaddingAt( offset );
	    }
	  } 
	else
	  {
	  result->SetPaddingAt( offset );
	  }
	}
      }
    }
  
  Progress::Done();
  return result;
}

template<class TInterpolator, class Fct> 
TypedArray::SmartPtr
ReformatVolume::ReformatUnmasked
( const UniformVolume* target, const cmtk::XformList& targetToRef, const cmtk::XformList& refToFloat, Fct& fct, const UniformVolume* floating, TInterpolator& interpolator )
{
  const DataGrid::IndexType& dims = target->GetDims();

  TypedArray::SmartPtr result = TypedArray::Create( fct.GetDataType( *floating ), target->GetNumberOfPixels() );
  if ( fct.UsePaddingValue )
    result->SetPaddingValue( fct.PaddingValue );

  // If there is floating image data (may not be, for Jacobian mapping etc.), then copy its data class
  if ( floating )
    result->SetDataClass( floating->GetData()->GetDataClass() );

  Progress::Begin( 0, dims[2], 1, "Volume reformatting" );
  
#pragma omp parallel for
  for ( int z = 0; z < dims[2]; z++ ) 
    {
    Vector3D vRef;
    Types::DataItem value;
    size_t offset = z * dims[0] * dims[1];

#ifdef _OPENMP
    if ( ! omp_get_thread_num() )
#endif
      Progress::SetProgress( z );
    
    for ( int y = 0; y < dims[1]; y++ ) 
      {
      for ( int x = 0; x < dims[0]; x++, offset++ ) 
	{
	vRef = target->GetGridLocation( x, y, z );
	if ( targetToRef.ApplyInPlace( vRef ) && fct( value, vRef, refToFloat, interpolator ) ) 
	  {
	  result->Set( value, offset );
	  } 
	else
	  {
	  result->SetPaddingAt( offset );
	  }
	} 
      }
    }
  
  Progress::Done();
  return result;
}

} // namespace cmtk
