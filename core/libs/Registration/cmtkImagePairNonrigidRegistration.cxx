/*
//
//  Copyright 2004-2012 SRI International
//
//  Copyright 1997-2009 Torsten Rohlfing
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

#include "cmtkImagePairNonrigidRegistration.h"

#include <Base/cmtkUniformVolume.h>
#include <Base/cmtkSplineWarpXform.h>
#include <Base/cmtkTypedArrayFunctionHistogramMatching.h>

#include <Registration/cmtkImagePairNonrigidRegistrationFunctional.h>
#include <Registration/cmtkImagePairSymmetricNonrigidRegistrationFunctional.h>
#include <Registration/cmtkOptimizer.h>
#include <Registration/cmtkBestNeighbourOptimizer.h>
#include <Registration/cmtkBestDirectionOptimizer.h>
#include <Registration/cmtkReformatVolume.h>

#include <algorithm>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

ImagePairNonrigidRegistration::ImagePairNonrigidRegistration () 
  : InitialWarpXform( NULL ),
    InverseWarpXform( NULL ),
    m_MatchFltToRefHistogram( false ),
    m_RepeatMatchFltToRefHistogram( false ),
    m_InverseConsistencyWeight( 0.0 ),
    m_RelaxToUnfold( false )
{
  this->m_Metric = 0;
  this->m_Algorithm = 3;

  this->m_GridSpacing = 15;
  this->m_ExactGridSpacing = 0;
  this->m_GridSpacing = 10;
  RestrictToAxes = NULL;
  this->m_RefineGrid = 0;
  RefinedGridAtLevel = -1;
  RefineGridCount = 0;
  this->m_DelayRefineGrid = 0;
  RefineDelayed = false;
  IgnoreEdge = 0;
  this->m_FastMode = false;
  this->m_AdaptiveFixParameters = 1;
  this->m_AdaptiveFixThreshFactor = 0.5;
  this->m_JacobianConstraintWeight = 0;
  this->m_GridEnergyWeight = 0;
  this->m_RelaxWeight = -1;
  this->m_InverseConsistencyWeight = 0.0;
  RelaxationStep = false;
}

CallbackResult 
ImagePairNonrigidRegistration::InitRegistration ()
{
  this->m_ReferenceVolume = this->m_Volume_1;
  this->m_FloatingVolume = this->m_Volume_2;

  Vector3D center = this->m_FloatingVolume->GetCenterCropRegion();
  this->m_InitialTransformation->ChangeCenter( center );

  Types::Coordinate currSampling = std::max( this->m_Sampling, 2 * std::min( this->m_ReferenceVolume->GetMinDelta(), this->m_FloatingVolume->GetMinDelta()));

  // If no initial transformation exists, create one from the defined
  // parameters.
  if ( InitialWarpXform ) 
    {
    // If we have an initial transformation from somewhere, use that.
    // This will override all registration parameters otherwise defined,
    // for example grid spacing and deformation type.
    InitialWarpXform->SetIgnoreEdge( IgnoreEdge );
    InitialWarpXform->SetFastMode( this->m_FastMode );
    // MIPSpro needs explicit.
    this->m_Xform = Xform::SmartPtr::DynamicCastFrom( InitialWarpXform );
    } 
  else
    {
    SplineWarpXform::SmartPtr warpXform( this->MakeWarpXform( this->m_ReferenceVolume->m_Size, this->m_InitialTransformation ) );
    
    if ( this->m_InverseConsistencyWeight > 0 ) 
      InverseWarpXform = SplineWarpXform::SmartPtr( this->MakeWarpXform( this->m_FloatingVolume->m_Size, this->m_InitialTransformation->GetInverse() ) );

    // MIPSpro needs explicit:
    this->m_Xform = Xform::SmartPtr::DynamicCastFrom( warpXform ); 
    }
  
  if ( this->m_MaxStepSize <= 0 )
    {
    const SplineWarpXform* warp = SplineWarpXform::SmartPtr::DynamicCastFrom( this->m_Xform ); 
    this->m_MaxStepSize = 0.25 * std::max( warp->m_Spacing[0], std::max( warp->m_Spacing[1], warp->m_Spacing[2] ) );
    }

  if ( this->m_CoarsestResolution <= 0 ) 
    this->m_CoarsestResolution = this->m_MaxStepSize;
  
  if ( this->m_UseOriginalData )
    {
    this->m_ParameterStack.push( Self::LevelParameters::SmartPtr( new Self::LevelParameters( -1 ) ) );
    }
  
  for ( ;(currSampling<=this->m_CoarsestResolution); currSampling *= 2 ) 
    {
    this->m_ParameterStack.push( Self::LevelParameters::SmartPtr( new Self::LevelParameters( currSampling ) ) );
    }
  
  switch ( this->m_Algorithm ) 
    {
    case 0:
      this->m_Optimizer = Optimizer::SmartPtr( new BestNeighbourOptimizer( this->m_OptimizerStepFactor ) ); 
      break;
    case 1:
    case 2:
      this->m_Optimizer = Optimizer::SmartPtr( NULL );
      break;
    case 3: 
    {
    BestDirectionOptimizer *optimizer = new BestDirectionOptimizer( this->m_OptimizerStepFactor );
    optimizer->SetUseMaxNorm( this->m_UseMaxNorm );
    this->m_Optimizer = Optimizer::SmartPtr( optimizer );
    break;
    }
    }
  
  this->m_Optimizer->SetCallback( this->m_Callback );

  return this->Superclass::InitRegistration();
}

SplineWarpXform::SmartPtr
ImagePairNonrigidRegistration::MakeWarpXform
( const UniformVolume::CoordinateVectorType& size, const AffineXform* initialAffine ) const
{
  SplineWarpXform::SmartPtr warpXform( new SplineWarpXform( size, this->m_GridSpacing, initialAffine, this->m_ExactGridSpacing ) );
  
  warpXform->SetIgnoreEdge( this->IgnoreEdge );
  warpXform->SetFastMode( this->m_FastMode );
 
  return warpXform;
}

Functional* 
ImagePairNonrigidRegistration::MakeFunctional
( const int level, const Superclass::LevelParameters* parameters )
{
  const Self::LevelParameters* levelParameters = dynamic_cast<const Self::LevelParameters*>( parameters );
  if ( ! levelParameters )
    {
    StdErr << "CODING ERROR: wrong RTTI for 'parameters'\n";
    exit( 1 );
    }

  WarpXform::SmartPtr warpXform = WarpXform::SmartPtr::DynamicCastFrom( this->m_Xform );
  if ( ! warpXform )
    {
    StdErr << "CODING ERROR: wrong RTTI for 'this->m_Xform'\n";
    exit( 1 );
    }  

  UniformVolume::SmartPtr referenceVolume( this->m_ReferenceVolume );
  UniformVolume::SmartPtr floatingVolume( this->m_FloatingVolume );
  if ( !level && this->m_MatchFltToRefHistogram )
    {
    floatingVolume = UniformVolume::SmartPtr( floatingVolume->Clone( true /*copyData*/ ) );
    floatingVolume->GetData()->ApplyFunctionObject( TypedArrayFunctionHistogramMatching( *(floatingVolume->GetData()), *(referenceVolume->GetData()) ) );
    }
  else
    {
    if ( this->m_RepeatMatchFltToRefHistogram )
      {
      floatingVolume = UniformVolume::SmartPtr( floatingVolume->Clone( true /*copyData*/ ) );
      UniformVolume::SmartPtr reformat( this->GetReformattedFloatingImage( Interpolators::NEAREST_NEIGHBOR ) );
      floatingVolume->GetData()->ApplyFunctionObject( TypedArrayFunctionHistogramMatching( *(reformat->GetData()), *(referenceVolume->GetData()) ) );
      }
    }
  
  if ( levelParameters->m_Resolution > 0 )
    {
    // for resample if not final, original resolution
    referenceVolume = UniformVolume::SmartPtr( referenceVolume->GetResampled( levelParameters->m_Resolution ) );
    floatingVolume = UniformVolume::SmartPtr( floatingVolume->GetResampled( levelParameters->m_Resolution ) );
    }

  if ( this->m_InverseConsistencyWeight > 0 ) 
    {
    ImagePairSymmetricNonrigidRegistrationFunctional *newFunctional = 
      ImagePairSymmetricNonrigidRegistrationFunctional::Create( this->m_Metric, referenceVolume, floatingVolume, this->m_FloatingImageInterpolation );
    newFunctional->SetInverseConsistencyWeight( this->m_InverseConsistencyWeight );
    newFunctional->SetAdaptiveFixParameters( this->m_AdaptiveFixParameters );
    newFunctional->SetAdaptiveFixThreshFactor( this->m_AdaptiveFixThreshFactor );
    newFunctional->SetJacobianConstraintWeight( this->m_JacobianConstraintWeight );
    newFunctional->SetGridEnergyWeight( this->m_GridEnergyWeight );

    return newFunctional;
    } 
  else
    {
    ImagePairNonrigidRegistrationFunctional *newFunctional = 
      ImagePairNonrigidRegistrationFunctional::Create( this->m_Metric, referenceVolume, floatingVolume, this->m_FloatingImageInterpolation );
    newFunctional->SetActiveCoordinates( this->RestrictToAxes );
    newFunctional->SetAdaptiveFixParameters( this->m_AdaptiveFixParameters );
    newFunctional->SetAdaptiveFixThreshFactor( this->m_AdaptiveFixThreshFactor );
    newFunctional->SetJacobianConstraintWeight( this->m_JacobianConstraintWeight );
    newFunctional->SetForceOutside( this->m_ForceOutsideFlag, this->m_ForceOutsideValue );
    newFunctional->SetGridEnergyWeight( this->m_GridEnergyWeight );
    return newFunctional;
  }
}

void
ImagePairNonrigidRegistration::EnterResolution
( CoordinateVector::SmartPtr& v, Functional::SmartPtr& functional,
  const int idx, const int total ) 
{
  float effGridEnergyWeight = this->m_GridEnergyWeight;
  float effJacobianConstraintWeight = this->m_JacobianConstraintWeight;
  float effInverseConsistencyWeight = this->m_InverseConsistencyWeight;

  if ( (this->m_RelaxWeight > 0) && !this->RelaxationStep ) 
    {
    effGridEnergyWeight *= this->m_RelaxWeight;
    effJacobianConstraintWeight *= this->m_RelaxWeight;
    effInverseConsistencyWeight *= this->m_RelaxWeight;
    }

  SplineWarpXform::SmartPtr warpXform = SplineWarpXform::SmartPtr::DynamicCastFrom( this->m_Xform );
  
  // handle simple nonrigid functional
  SmartPointer<ImagePairNonrigidRegistrationFunctional> nonrigidFunctional = ImagePairNonrigidRegistrationFunctional::SmartPtr::DynamicCastFrom( functional );
  if ( nonrigidFunctional ) 
    {
    nonrigidFunctional->SetWarpXform( warpXform );

    if ( this->m_RelaxToUnfold ) // has to come after functional->SetWarpXform so reference volume is registered with warp grid
      warpXform->RelaxToUnfold();

    nonrigidFunctional->SetGridEnergyWeight( effGridEnergyWeight );
    nonrigidFunctional->SetJacobianConstraintWeight( effJacobianConstraintWeight );
    } 
  else 
    {
    // handle inverse-consistent nonrigid functional
    SmartPointer<ImagePairSymmetricNonrigidRegistrationFunctional> symmetricFunctional = ImagePairSymmetricNonrigidRegistrationFunctional::SmartPtr::DynamicCastFrom( functional );
    if ( symmetricFunctional ) 
      {
      symmetricFunctional->SetWarpXform( warpXform, this->InverseWarpXform );

      if ( this->m_RelaxToUnfold ) // has to come after functional->SetWarpXform so reference volume is registered with warp grid
	{
	warpXform->RelaxToUnfold();
	this->InverseWarpXform->RelaxToUnfold();
	}

      symmetricFunctional->SetGridEnergyWeight( effGridEnergyWeight );
      symmetricFunctional->SetJacobianConstraintWeight( effJacobianConstraintWeight );
      symmetricFunctional->SetInverseConsistencyWeight( effInverseConsistencyWeight );
      } 
    else 
      {
      // neither simple nor inverse-consistent functional: something went
      // badly wrong.
      StdErr << "Fatal coding error: Non-nonrigid functional in ImagePairNonrigidRegistration::EnterResolution.\n";
      abort();
      }
    }
  
  Superclass::EnterResolution( v, functional, idx, total );
}

int 
ImagePairNonrigidRegistration::DoneResolution
( CoordinateVector::SmartPtr& v, Functional::SmartPtr& functional,
  const int idx, const int total ) 
{
  if ( ( this->m_RelaxWeight > 0 ) && ! RelaxationStep ) 
    {
    RelaxationStep = true;
    this->Superclass::DoneResolution( v, functional, idx, total );
    return false; // repeat with a relaxation step.
    } 
  else 
    {
    RelaxationStep = false;
    }
  
  bool repeat = ( ( idx == total ) && ( RefineGridCount < this->m_RefineGrid ) );
  
  if ( (RefinedGridAtLevel != idx) || (idx==total) ) 
    {    
    if ( RefineGridCount < this->m_RefineGrid ) 
      {      
      if ( (!this->m_DelayRefineGrid) || RefineDelayed || ( idx == total ) ) 
	{
	WarpXform::SmartPtr warpXform = WarpXform::SmartPtr::DynamicCastFrom( this->m_Xform );
	if ( warpXform ) 
	  {
	  warpXform->Refine();
	  if ( InverseWarpXform )
	    InverseWarpXform->Refine();
	  ++RefineGridCount;
	  functional->GetParamVector( *v );    
	  if ( this->m_Callback ) 
	    this->m_Callback->Comment( "Refined control point grid." );
	  RefinedGridAtLevel = idx;
	  } 	  
	if ( this->m_DelayRefineGrid && ( idx > 1 ) ) repeat = true;
	RefineDelayed = false;
	} 
      else 
	{
	RefineDelayed = true;
	}
      }
    }
  else 
    {
    RefineDelayed = true;
    }
  
  return this->Superclass::DoneResolution( v, functional, idx, total ) && !repeat;
}

const UniformVolume::SmartPtr
ImagePairNonrigidRegistration::GetReformattedFloatingImage( Interpolators::InterpolationEnum interpolator ) const
{
  ReformatVolume reformat;
  reformat.SetInterpolation( interpolator );
  reformat.SetReferenceVolume( this->m_Volume_1 );
  reformat.SetFloatingVolume( this->m_Volume_2 );

  WarpXform::SmartPtr warpXform( this->GetTransformation() );
  reformat.SetWarpXform( warpXform );

  if ( this->m_ForceOutsideFlag )
    {
    reformat.SetPaddingValue( this->m_ForceOutsideValue );
    }
  
  UniformVolume::SmartPtr result = reformat.PlainReformat();

  if ( this->m_ForceOutsideFlag )
    {
    result->GetData()->ClearPaddingFlag();
    }
  return result;
}


} // namespace cmtk
