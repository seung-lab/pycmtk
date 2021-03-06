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

#ifndef __cmtkSymmetryPlaneFunctional_h_included_
#define __cmtkSymmetryPlaneFunctional_h_included_

#include <cmtkconfig.h>

#include <Base/cmtkFunctional.h>
#include <Base/cmtkUniformVolume.h>
#include <Base/cmtkParametricPlane.h>

#include <Registration/cmtkVoxelMatchingNormMutInf.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

/** Functional for finding a symmetry plane in 3-D volumes.
 */
class SymmetryPlaneFunctional :
  /// Inherit functional interface.
  public Functional 
{
protected:
  /// Volume image.
  UniformVolume::SmartPtr m_Volume;

public:
  /// This class.
  typedef SymmetryPlaneFunctional Self;

  /// Smart pointer to this class.
  typedef SmartPointer<Self> SmartPtr;

  /// Superclass
  typedef Functional Superclass;

  /// Constructor.
  SymmetryPlaneFunctional( UniformVolume::SmartPtr& volume );

  /// Constructor with value range limits.
  SymmetryPlaneFunctional( UniformVolume::SmartPtr& volume, const Types::DataItemRange& valueRange );

  /// Destructor.
  virtual ~SymmetryPlaneFunctional() {}

  /// Set volume.
  void SetVolume( UniformVolume::SmartPtr& volume ) 
  {
    m_Volume = volume;
  }

  virtual void GetParamVector ( CoordinateVector& v )  
  {
    this->m_ParametricPlane.GetParameters( v );
  }

  /// Compute functional value.
  virtual Self::ReturnType Evaluate();

  /// Compute functional value.
  virtual Self::ReturnType EvaluateAt( CoordinateVector& v ) 
  {
    this->m_ParametricPlane.SetParameters( v );
    return this->Evaluate();
  }

  /// Return the symmetry plane's parameter vector dimension.
  virtual size_t ParamVectorDim() const { return 6; }

  /// Return the number of variable parameters of the transformation.
  virtual size_t VariableParamVectorDim() const { return 3; }

  /// Return the parameter stepping for 1 mm optimization steps.
  virtual Types::Coordinate GetParamStep( const size_t idx, const Types::Coordinate mmStep = 1 ) const;

private:
  /// Image similarity measure.
  VoxelMatchingNormMutInf<>* m_Metric;

  /// The symmetry plane.
  ParametricPlane m_ParametricPlane;
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkSymmetryPlaneFunctional_h_included_
