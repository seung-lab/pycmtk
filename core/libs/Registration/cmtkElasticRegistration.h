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

#ifndef __cmtkElasticRegistration_h_included_
#define __cmtkElasticRegistration_h_included_

#include <cmtkconfig.h>

#include <Registration/cmtkVoxelRegistration.h>

#include <Base/cmtkSplineWarpXform.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

/** Generic multiresolution voxel-registration class.
 * By implementing member functions to retrieve parameters and report results
 * in derived classes, registration can be integrated into various 
 * environments.
 *\version $Revision$ $Date$
 */
class ElasticRegistration : 
  /// Inherit basic voxel registration functions.
  public VoxelRegistration 
{
protected:
  /// Initial deformation.
  SplineWarpXform::SmartPtr InitialWarpXform;

  /// Optional inverse warp for inverse-consistent registration.
  SplineWarpXform::SmartPtr InverseWarpXform;

  /** If this flag is set, reference and model volume are exchanged.
   * By default, volume #1 is the reference and volume #2 the model image.
   * However, a studylist for example may contain both volumes in the wrong
   * order. This is caused by the fact that for CT to MRI registration, for
   * instance, it does not make sense to warp the CT and leave MRI fixed.
   */
  bool ForceSwitchVolumes;
  
  /// Flag whether to adjust floating image histogram to match reference image.
  cmtkGetSetMacro(bool,MatchFltToRefHistogram);

  /// This value determines how often the control point grid is refined.
  cmtkGetSetMacro(int,RefineGrid);

  /** Flag whether to delay grid refinement.
   * If this flag is set, a newly entered image resolution level is run with 
   * the previous, coarser deformation grid first before refining.
   */
  cmtkGetSetMacro(bool,DelayRefineGrid);

  /// Initial spacing of the control point grid.
  cmtkGetSetMacro(Types::Coordinate,GridSpacing);

  /** Force exact grid spacing.
   * If this flag is set, then the CPG will be spaced at exactly the distance
   * given in the GridSpacing field. Otherwise, the grid spacing will be 
   * adjusted so that there is an integral number of CPG cells that cover the
   * reference image domain.
   */
  cmtkGetSetMacro(bool,ExactGridSpacing);

  /// This counter determines how many edge control points are fixed.
  unsigned int IgnoreEdge;

  /** Restrict deformation to one or more coordinate axes.
   */
  const char* RestrictToAxes;

  /// Flag for fast mode (less accurate) of spline deformations.
  cmtkGetSetMacro(bool,FastMode);

  /// Flag for adaptive selection of active and passive parameters.
  cmtkGetSetMacro(bool,AdaptiveFixParameters);

  /** Set threshold factor for selecting passive warp parameters adaptively.
   * If the flag AdaptiveFixParameters is set, this value determines the
   * threshold by which active vs. passive parameters are selected. All
   * control points are set to passive for which the local region entropy is
   * below this factor times sum of min and max region entropy. The default
   * value is 0.5.
   */
  cmtkGetSetMacro(float,AdaptiveFixThreshFactor);

  /// Weighting of Jacobian constraint relative to similairy measure.
  cmtkGetSetMacro(float,JacobianConstraintWeight);

  /// Weighting of rigidity constraint relative to similairy measure.
  cmtkGetSetMacro(float,RigidityConstraintWeight);

  /// Pixelwise weight map of rigidity constraint relative to similairy measure.
  cmtkGetSetMacro(UniformVolume::SmartPtr,RigidityConstraintMap);

  /// Weighting of grid bending energy constraint relative to image similarity.
  cmtkGetSetMacro(float,GridEnergyWeight);

  /// Factor by which to relax constraint weights for a relaxation step.
  cmtkGetSetMacro(float,RelaxWeight);

  /** Weight for inverse consistency weight.
   * If this is set to a value greater than 0, inverse consistency of the
   * transformation is enforced. In fact, both forward and backward
   * transformation are optimized simultaneously.
   */
  cmtkGetSetMacro(float,InverseConsistencyWeight);

  /// Flag to turn on deformation unfolding before each level.
  bool m_RelaxToUnfold;

  /// Set flag and value for forcing values outside the floating image.
  virtual void SetForceOutside( const bool flag = true, const Types::DataItem value = 0 )
  {
    this->m_ForceOutsideFlag = flag;
    this->m_ForceOutsideValue = value;
  }

  /** Default constructor.
   * Set initial values for some flags.
   */
  ElasticRegistration();

  /** Destructor.
   * Free local objects for this class.
   */
  virtual ~ElasticRegistration() {};

  /**\name Member functions to be overwritten.
   */
  //@{
  /** Initialize registration.
   * This function is called by Register before any other operations. It can
   * be overloaded to open status dialog windows, etc. Derived implementations
   * should call their base class' InitRegistration first.
   *\return 
   */
  virtual CallbackResult InitRegistration ();

  /** Enter resolution level.
  */
  virtual void EnterResolution( CoordinateVector::SmartPtr&, Functional::SmartPtr&, const int, const int );

  /** Finish resolution level.
   * In addition to operations necessary for general registration, we have
   * to make some additional modifications. In particular the sampling of
   * the warp transformations' control point mesh has to be refined before
   * entering the next resolution. 
  */
  virtual int DoneResolution( CoordinateVector::SmartPtr&, Functional::SmartPtr&, const int, const int );
  //@}

  /// Return final transformation.
  SplineWarpXform::SmartPtr GetTransformation() const
  {
    return SplineWarpXform::SmartPtr::DynamicCastFrom( this->m_Xform );
  }

  /// Get reformatted floating image.
  const UniformVolume::SmartPtr GetReformattedFloatingImage( Interpolators::InterpolationEnum interpolator = Interpolators::LINEAR );

private:
  /// Level on which the last control grid refinement was performend.
  int RefinedGridAtLevel;

  /// Number of refinements so far.
  int RefineGridCount;

  /// Convenience typedef.
  typedef VoxelRegistration Superclass;

  /// Are we currently doing a relaxation step?
  bool RelaxationStep;

  /// Have we already run the current level before refining the grid?
  bool RefineDelayed;

  /// Flag for forcing pixel values outside the floating image.
  bool m_ForceOutsideFlag;

  /// Value for forcing pixel values outside the floating image.
  Types::DataItem m_ForceOutsideValue;

  /** Create warp transformation with current settings.
   * This function is used to create the standard warp, as well as the
   * approximated inverse warp for the optional inverse-consistent
   * registration.
   *\param size Reference volume size.
   *\param initialAffine Initial affine transformation for the warp.
   */
  const SplineWarpXform::SmartPtr MakeWarpXform( const Vector3D& size, const AffineXform* initialAffine ) const;

  /** Create functional with all settings and two given volume objects.
   */
  Functional* MakeFunctional( UniformVolume::SmartPtr& refVolume, UniformVolume::SmartPtr& fltVolume, UniformVolume::SmartPtr& rigidityMap ) const;
};

//@}

} // namespace cmtk

#endif // __cmtkElasticRegistration_h_included_
