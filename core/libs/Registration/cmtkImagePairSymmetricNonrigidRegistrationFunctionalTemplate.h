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

#ifndef __cmtkImagePairSymmetricNonrigidRegistrationFunctionalTemplate_h_included_
#define __cmtkImagePairSymmetricNonrigidRegistrationFunctionalTemplate_h_included_

#include <cmtkconfig.h>

#include "cmtkImagePairSymmetricNonrigidRegistrationFunctionalTemplate.h"

#include <Base/cmtkFunctional.h>
#include <Base/cmtkMacros.h>

#include <Registration/cmtkImagePairNonrigidRegistrationFunctionalTemplate.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

/// Template for symmtric-consistent elastic registration functional.
template<class VM, class W>
class ImagePairSymmetricNonrigidRegistrationFunctionalTemplate :
  /** Inherit from non-template base functional class. */
  public ImagePairSymmetricNonrigidRegistrationFunctional
{
public:
  /// This class.
  typedef ImagePairSymmetricNonrigidRegistrationFunctionalTemplate<VM,W> Self;

  /// Smart pointer to this class.
  typedef SmartPointer<Self> SmartPtr;

  /// Superclass.
  typedef ImagePairSymmetricNonrigidRegistrationFunctional Superclass;

  /// The forward functional.
  ImagePairNonrigidRegistrationFunctionalTemplate<VM> FwdFunctional;

  /// The backward functional.
  ImagePairNonrigidRegistrationFunctionalTemplate<VM> BwdFunctional;

  /// Constructor.
  ImagePairSymmetricNonrigidRegistrationFunctionalTemplate( UniformVolume::SmartPtr& reference, UniformVolume::SmartPtr& floating, const Interpolators::InterpolationEnum interpolation )
    : FwdFunctional( reference, floating, interpolation ),
      BwdFunctional( floating, reference, interpolation )
  {}

  /// Set inverse consistency weight.
  virtual void SetInverseConsistencyWeight( const typename Self::ReturnType inverseConsistencyWeight ) 
  {
    this->FwdFunctional.SetInverseConsistencyWeight( inverseConsistencyWeight );
    this->BwdFunctional.SetInverseConsistencyWeight( inverseConsistencyWeight );
  }
  
  /// Set adaptive parameter fixing flag.
  virtual void SetAdaptiveFixParameters( const bool adaptiveFixParameters ) 
  {
    this->FwdFunctional.SetAdaptiveFixParameters( adaptiveFixParameters );
    this->BwdFunctional.SetAdaptiveFixParameters( adaptiveFixParameters );
  }

  /// Set adaptive parameter fixing threshold.
  virtual void SetAdaptiveFixThreshFactor( const typename Self::ReturnType threshFactor ) 
  {
    this->FwdFunctional.SetAdaptiveFixThreshFactor( threshFactor );
    this->BwdFunctional.SetAdaptiveFixThreshFactor( threshFactor );
  }

  /// Set Jacobian constraint weight.
  virtual void SetJacobianConstraintWeight( const typename Self::ReturnType jacobianConstraintWeight ) 
  {
    this->FwdFunctional.SetJacobianConstraintWeight( jacobianConstraintWeight );
    this->BwdFunctional.SetJacobianConstraintWeight( jacobianConstraintWeight );
  }
  
  /// Set smoothness constraint weight.
  virtual void SetGridEnergyWeight( const typename Self::ReturnType gridEnergyWeight ) 
  {
    this->FwdFunctional.SetGridEnergyWeight( gridEnergyWeight );
    this->BwdFunctional.SetGridEnergyWeight( gridEnergyWeight );
  }
  
  /// Set warp for forward and backward functional.
  virtual void SetWarpXform( SplineWarpXform::SmartPtr& warpFwd, SplineWarpXform::SmartPtr& warpBwd );

  /// Return parameter vector.
  virtual void GetParamVector ( CoordinateVector& v )  
  {
    CoordinateVector vFwd, vBwd;
    this->FwdFunctional.GetParamVector( vFwd );
    this->BwdFunctional.GetParamVector( vBwd );

    v.SetDim( vFwd.Dim + vBwd.Dim );
    v.CopyToOffset( vFwd );
    v.CopyToOffset( vBwd, vFwd.Dim );
  }

  /// Evaluate functional value and gradient.
  virtual typename Self::ReturnType EvaluateWithGradient( CoordinateVector& v, CoordinateVector& g, const Types::Coordinate step = 1 );
    
  /// Evaluate functional value.
  virtual typename Self::ReturnType EvaluateAt ( CoordinateVector& v ) 
  {
    CoordinateVector vFwd( this->FwdFunctional.ParamVectorDim(), v.Elements, false /*freeElements*/ );
    CoordinateVector vBwd( this->BwdFunctional.ParamVectorDim(), v.Elements+this->FwdFunctional.ParamVectorDim(), false /*freeElements*/ );
    return this->FwdFunctional.EvaluateAt( vFwd ) + this->BwdFunctional.EvaluateAt( vBwd );
  }
  
  virtual typename Self::ReturnType Evaluate () 
  {
    return this->FwdFunctional.Evaluate() + this->BwdFunctional.Evaluate();
  }
  
  /// Get parameter stepping in milimeters.
  virtual Types::Coordinate GetParamStep( const size_t idx, const Types::Coordinate mmStep = 1 ) const 
  {
    if ( idx < this->FwdFunctional.ParamVectorDim() )
      return this->FwdFunctional.GetParamStep( idx, mmStep );
    else
      return this->BwdFunctional.GetParamStep( idx - this->FwdFunctional.ParamVectorDim(), mmStep );
  }
  
  /// Return the transformation's parameter vector dimension.
  virtual size_t ParamVectorDim() const
  {
    return this->FwdFunctional.ParamVectorDim() + this->BwdFunctional.ParamVectorDim();
  }
  
  /// Return the number of variable parameters of the transformation.
  virtual size_t VariableParamVectorDim() const 
  {
    return this->FwdFunctional.VariableParamVectorDim() + this->BwdFunctional.VariableParamVectorDim();
  }
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkImagePairSymmetricNonrigidRegistrationFunctionalTemplate_h_included_
