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
//  $Revision$
//
//  $LastChangedDate$
//
//  $LastChangedBy$
//
*/

#include <cmtkAffineRegistration.h>

#include <cmtkVector.h>

#include <cmtkXform.h>
#include <cmtkAffineXform.h>

#include <cmtkVolume.h>
#include <cmtkUniformVolume.h>
#include <cmtkFunctional.h>

#include <cmtkVoxelMatchingAffineFunctional.h>

#include <cmtkOptimizer.h>
#include <cmtkBestNeighbourOptimizer.h>

#include <cmtkTimers.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

AffineRegistration::AffineRegistration () 
{ 
  this->m_InitialAlignCenters = false;
  this->m_NoSwitch = false;
}

AffineRegistration::~AffineRegistration () 
{
}

CallbackResult 
AffineRegistration::InitRegistration ()
{
  CallbackResult result = this->Superclass::InitRegistration();
  if ( result != CALLBACK_OK ) return result;

  UniformVolume::SmartPtr refVolume;
  UniformVolume::SmartPtr fltVolume;

  if ( this->m_NoSwitch || (this->m_Volume_1->AverageVoxelVolume() >= this->m_Volume_2->AverageVoxelVolume()) ) 
    {
    refVolume = this->m_Volume_1;
    fltVolume = this->m_Volume_2;
    SwitchVolumes = false;
    } 
  else
    {
    refVolume = this->m_Volume_2;
    fltVolume = this->m_Volume_1;
    SwitchVolumes = true;
    }
  
  AffineXform::SmartPtr affineXform;

  if ( this->m_InitialXform )
    {
    if ( SwitchVolumes ^ this->m_InitialXformIsInverse )
      {
      affineXform = AffineXform::SmartPtr( this->m_InitialXform->MakeInverse() );
      } 
    else
      {
      affineXform = AffineXform::SmartPtr( this->m_InitialXform );
      }
  } 
  else 
    {
    affineXform = AffineXform::SmartPtr( new AffineXform );
    }

  if ( this->m_InitialAlignCenters ) 
    {
    Vector3D deltaCenter = ( refVolume->GetCenterCropRegion() - fltVolume->GetCenterCropRegion() );
    affineXform->SetXlate( deltaCenter.XYZ );
    }
  
  // explicit cast needed for MIPSpro comp.
  this->m_Xform = Xform::SmartPtr::DynamicCastFrom( affineXform );
  
  Vector3D center = fltVolume->GetCenterCropRegion();
  affineXform->ChangeCenter( center.XYZ );

  if ( this->m_UseOriginalData ) 
    {  
    VoxelMatchingAffineFunctional *newFunctional = CreateAffineFunctional( this->m_Metric, refVolume, fltVolume, affineXform );
    FunctionalStack.push( Functional::SmartPtr( newFunctional ) );
    }
  
  Types::Coordinate currSampling = std::max( this->m_Sampling, 2 * std::min( refVolume->GetMinDelta(), fltVolume->GetMinDelta()));
  CallbackResult irq = CALLBACK_OK;
  
  double coarsest = CoarsestResolution;
  if ( coarsest <= 0 ) coarsest = this->m_Exploration;
  
  for ( ; ( irq == CALLBACK_OK ) && (currSampling<=coarsest); currSampling *= 2 ) 
    {
    UniformVolume::SmartPtr nextRef( NULL );
    UniformVolume::SmartPtr nextFlt( NULL );
    try 
      {
      this->ReportProgress( "resampling", 0 );
      nextRef = UniformVolume::SmartPtr( new UniformVolume( *refVolume, currSampling ) );
      irq = this->ReportProgress(  "resampling", 50 );
      nextFlt = UniformVolume::SmartPtr( new UniformVolume( *fltVolume, currSampling ) );
      }
    catch (...) 
      {
      }
    
    VoxelMatchingAffineFunctional *newFunctional = CreateAffineFunctional( this->m_Metric, nextRef, nextFlt, affineXform );
    FunctionalStack.push( Functional::SmartPtr( newFunctional ) );
    
    refVolume = nextRef;
    fltVolume = nextFlt;
    }

  this->m_Optimizer = Optimizer::SmartPtr( new BestNeighbourOptimizer( OptimizerStepFactor ) );   
  this->m_Optimizer->SetCallback( this->m_Callback );
  
  // default to rigid transformation
  if ( NumberDOFs.empty() )
    NumberDOFs.push_back( 6 );
  
  // push guard elements
  NumberDOFs.push_back( -1 );
  NumberDOFsFinal.push_back( -1 );
  // intialize iterator.
  NumberDOFsIterator = NumberDOFs.begin();

  return irq;
}

void
AffineRegistration::EnterResolution
( CoordinateVector::SmartPtr& v, Functional::SmartPtr& f, const int level, const int total ) 
{
  if ( *NumberDOFsIterator < 0 )
    {
    if ( (level == total) && (NumberDOFsFinal.size()>1) )
      NumberDOFsIterator = NumberDOFsFinal.begin();
    else
      NumberDOFsIterator = NumberDOFs.begin();
    }

  AffineXform::SmartPtr affineXform = AffineXform::SmartPtr::DynamicCastFrom( this->m_Xform );
  if ( affineXform ) 
    {
    int numberDOFs = *NumberDOFsIterator;
    affineXform->SetNumberDOFs( numberDOFs );
    if ( this->m_Callback ) 
      {
      char buffer[64];
      snprintf( buffer, sizeof( buffer ), "Setting Number DOFs to %d.", numberDOFs );
      this->m_Callback->Comment( buffer );
      }
    }
  this->Superclass::EnterResolution( v, f, level, total );
}

int 
AffineRegistration::DoneResolution
( CoordinateVector::SmartPtr& v, Functional::SmartPtr& f,
  const int level, const int total )
{
  this->Superclass::DoneResolution( v, f, level, total );
  
  NumberDOFsIterator++;
  return (*NumberDOFsIterator < 0);
}

AffineXform::SmartPtr
AffineRegistration::GetTransformation() const
{
  AffineXform::SmartPtr affineXform = AffineXform::SmartPtr::DynamicCastFrom( this->m_Xform );
  if ( affineXform && SwitchVolumes ) 
    {
    return affineXform->GetInverse();
    } 
  else 
    {
    return affineXform;
    }
}

} // namespace cmtk
