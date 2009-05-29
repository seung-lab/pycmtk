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

#include <cmtkSplineWarpMultiChannelRegistrationFunctional.h>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

template<class TMetricFunctional>
CMTK_THREAD_RETURN_TYPE
SplineWarpMultiChannelRegistrationFunctional<TMetricFunctional>
::EvaluateThreadFunction( void* args )
{
  ThreadParameters<Self>* params = static_cast<ThreadParameters<Self>*>( args );

  Self* This = params->thisObject;
  const Self* constThis = This;

  typename Self::MetricData metricData;
  metricData.Init( This );

  const int *dims = constThis->m_ReferenceDims;
  const int dimsX = dims[0], dimsY = dims[1], dimsZ = dims[2];
  std::vector<Vector3D> pFloating( dimsX );

  for ( int pZ = params->ThisThreadIndex; pZ < dimsZ; pZ += params->NumberOfThreads ) 
    {
    for ( int pY = 0; pY < dimsY; ++pY ) 
      {
      constThis->m_Transformation.GetTransformedGridSequenceNonVirtual( &pFloating[0], dimsX, 0, pY, pZ );
      
      size_t r = dimsX * (pY + dimsY * pZ );
      for ( int pX = 0; pX < dimsX; ++pX, ++r ) 
	{
	// Continue metric computation.
	This->ContinueMetricStoreReformatted( metricData, r, pFloating[pX] );
	}
      }
    }

  This->m_MetricDataMutex.Lock();
  This->m_MetricData += metricData;
  This->m_MetricDataMutex.Unlock();

  return CMTK_THREAD_RETURN_VALUE;
}

template<class TMetricFunctional>
CMTK_THREAD_RETURN_TYPE
SplineWarpMultiChannelRegistrationFunctional<TMetricFunctional>
::EvaluateWithGradientThreadFunction( void* args )
{
  EvaluateGradientThreadParameters* params = static_cast<EvaluateGradientThreadParameters*>( args );

  Self* This = params->thisObject;
  const Self* constThis = This;

  const size_t numberOfParameters = This->VariableParamVectorDim();
  const size_t numberOfControlPoints = numberOfParameters / 3;

  const size_t threadIdx = params->ThisThreadIndex;
  const size_t threadCnt = params->NumberOfThreads;

  SplineWarpXform::SmartPtr transformation = constThis->m_ThreadTransformations[threadIdx];
  transformation->SetParamVector( *(params->m_ParameterVector) );

  for ( size_t cp = threadIdx; cp < numberOfControlPoints; cp += threadCnt ) 
    {
    typename Superclass::MetricData localMetricDataCP = constThis->m_MetricData;
    This->BacktraceMetric( localMetricDataCP, constThis->m_VolumeOfInfluenceVector[cp] );

    size_t idx = 3 * cp;
    for ( int dim = 0; dim < 3; ++dim, ++idx )
      {
      if ( constThis->m_StepScaleVector[idx] <= 0 ) 
	{
	params->m_Gradient[idx] = 0;
	} 
      else
	{
	const Types::Coordinate vOld = transformation->GetParameter( idx );
	
	Types::Coordinate thisStep = params->m_Step * constThis->m_StepScaleVector[idx];
	
	typename Superclass::MetricData localMetricData = localMetricDataCP;
	transformation->SetParameter( idx, vOld + thisStep );
	double upper = This->EvaluateIncremental( transformation, localMetricData, constThis->m_VolumeOfInfluenceVector[cp] );
	
	localMetricData = localMetricDataCP;
	transformation->SetParameter( idx, vOld - thisStep );
	double lower = This->EvaluateIncremental( transformation, localMetricData, constThis->m_VolumeOfInfluenceVector[cp] );
	
	transformation->SetParameter( idx, vOld );
	
	if ( constThis->m_JacobianConstraintWeight > 0 )
	  {
	  double lowerConstraint = 0, upperConstraint = 0;
	  transformation->GetJacobianConstraintDerivative( lowerConstraint, upperConstraint, idx, constThis->m_VolumeOfInfluenceVector[cp], thisStep );
	  lower -= constThis->m_JacobianConstraintWeight * lowerConstraint;
	  upper -= constThis->m_JacobianConstraintWeight * upperConstraint;
	  }
	
	if ( finite( upper ) && finite(lower) && ((upper > params->m_MetricBaseValue ) || (lower > params->m_MetricBaseValue)) )
	  {
	  params->m_Gradient[idx] = upper-lower;
	  } 
	else 
	  {
	  params->m_Gradient[idx] = 0;
	  }
	}
      }
    }
  return CMTK_THREAD_RETURN_VALUE;
}

} // namespace cmtk
