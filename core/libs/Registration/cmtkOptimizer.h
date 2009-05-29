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

#ifndef __cmtkOptimizer_h_included_
#define __cmtkOptimizer_h_included_

#include <cmtkconfig.h>

#include <cmtkOptimizerBase.h>
#include <cmtkMacros.h>

#include <cmtkFunctional.h>
#include <cmtkVector.h>

#include <cmtkRegistrationCallback.h>

#include <vector>

#include <cmtkSmartPtr.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

/// Abstract optimizer.
class Optimizer : 
  /// Inherit from optimizer base class.
  public OptimizerBase
{
public:
  /// This class.
  typedef Optimizer Self;

  /// Superclass.
  typedef OptimizerBase Superclass;

  /// Smart pointer to Optimizer.
  typedef SmartPointer<Self> SmartPtr;

  /** This flag determines whether the vector of step sizes is updated.
   * For some optimization problems, such as 2-D projection to 3-D image
   * registration, the current parameter vector determines the optimum step
   * sizes for the transformation parameters. In such cases, this flag can be
   * set to "true" so that after each optimization step the functional is
   * queried for updated steppings.
   */
  igsGetSetMacro(bool,UpdateStepScaleVector);

  /** External callback object.
   * This object is called during the optimization, reporting optimization
   * progress to the user and checking for user interrupts.
   */
  igsGetSetMacro(RegistrationCallback::SmartPtr,Callback);

  /** Optimization functional.
   */
  igsGetSetMacro(Functional::SmartPtr,Functional);

  /// Execute callback if one was set.
  virtual CallbackResult CallbackExecute( const CoordinateVector &v, const Self::ReturnType metric, const int percentDone ) 
  {
    if ( Callback ) return Callback->Execute( v, metric, percentDone );
    return CALLBACK_OK;
  }

  /// Execute callback if one was set.
  virtual CallbackResult CallbackExecute( const int percentDone )
  {
    if ( Callback ) Callback->Execute( percentDone );
    return CALLBACK_OK;
  }

  /// Notify callback of an annotation if one exists.
  virtual void CallbackComment ( const char* comment = NULL ) 
  {
    if ( Callback ) Callback->Comment( comment );
  }

  /// Return dimension of search space.
  virtual unsigned int GetSearchSpaceDimension() const 
  {
    return Functional->VariableParamVectorDim();
  }

  /// Return parameter stepping.
  virtual Self::ParameterType GetParamStep( unsigned int idx, const Self::ParameterType mmStep = 1.0 ) const 
  {
    return Functional->GetParamStep( idx, mmStep );
  }

  /// Return functional value.
  virtual Self::ReturnType Evaluate ( CoordinateVector& v ) 
  {
    return Functional->EvaluateAt( v );
  }

  /// Evaluate functional and also return its gradient.
  virtual Self::ReturnType EvaluateWithGradient( CoordinateVector& v, CoordinateVector& directionVector, const Self::ParameterType step = 1 ) 
  {
    return Functional->EvaluateWithGradient( v, directionVector, step );
  }

  /// Default constructor.
  Optimizer()
    : Callback( NULL ),
      Functional( NULL )
  {
    UpdateStepScaleVector = false;
  }

  /// Virtual destructor.
  virtual ~Optimizer () {}

  /// Interface: Optimize functional.
  virtual CallbackResult Optimize( CoordinateVector&, const Self::ParameterType = 1, const Self::ParameterType = 0 ) = 0;

  /// Get flag to check whether previous call to Optimize() changed parameters.
  bool GetLastOptimizeChangedParameters() const
  {
    return this->m_LastOptimizeChangedParameters;
  }

protected:
  /// Flag whether the last call to Optimize() made any changes to functional parameters.
  bool m_LastOptimizeChangedParameters;
};

//@}

} // namespace cmtk

#endif // #ifdef __cmtkOptimizer_h_included_
