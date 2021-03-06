/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2013 SRI International
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

#include <System/cmtkException.h>

#include <Base/cmtkTypes.h>
#include <Base/cmtkEigenValuesSymmetricMatrix.h>

#include <algorithm>
#include <vector>

namespace
cmtk
{

/** \addtogroup Registration */
//@{

template<class TRealType,class TDataType,class TInterpolator>
void
MultiChannelRMIRegistrationFunctional<TRealType,TDataType,TInterpolator>
::ContinueMetric( MetricData& metricData, const size_t rindex, const Vector3D& fvector )
{
  std::vector<Types::DataItem> values( this->m_NumberOfChannels );
  
  size_t idx = 0;
  for ( size_t ref = 0; ref < this->m_ReferenceChannels.size(); ++ref )
    {
    if ( !this->m_ReferenceChannels[ref]->GetDataAt( values[idx++], rindex ) ) return;
    }
  
  for ( size_t flt = 0; flt < this->m_FloatingChannels.size(); ++flt )
    {
    if ( !this->m_FloatingInterpolators[flt]->GetDataAt( fvector, values[idx++] ) ) return;
    }
  
  metricData += values;
}

template<class TRealType,class TDataType,class TInterpolator>
TRealType
MultiChannelRMIRegistrationFunctional<TRealType,TDataType,TInterpolator>
::GetMetric( MetricData& metricData ) const
{
  const size_t nRefs = this->m_ReferenceChannels.size();
  const size_t nFlts = this->m_FloatingChannels.size();

  size_t idx = 0;
  for ( size_t j = 0; j < this->m_NumberOfChannels; ++j )
    {
    const RealType muj = metricData.m_Sums[j] / metricData.m_TotalNumberOfSamples;

    for ( size_t i = 0; i <= j; ++i, ++idx )
      {
      const RealType mui = metricData.m_Sums[i] / metricData.m_TotalNumberOfSamples;
      metricData.m_CovarianceMatrix(i,j) = (metricData.m_Products[idx] / metricData.m_TotalNumberOfSamples) - mui * muj;
      }
    }

  for ( size_t j = 0; j < nRefs; ++j )
    {
    for ( size_t i = 0; i <= j; ++i )
      {
      metricData.m_CovarianceMatrixRef(i,j) = metricData.m_CovarianceMatrix(i,j);
      }
    }
  
  for ( size_t j = 0; j < nFlts; ++j )
    {
    for ( size_t i = 0; i <= j; ++i )
      {
      metricData.m_CovarianceMatrixFlt(i,j) = metricData.m_CovarianceMatrix(nRefs+i,nRefs+j);
      }
    }
  
  const std::vector<RealType> eigenvalues = EigenValuesSymmetricMatrix<RealType>( metricData.m_CovarianceMatrix ).GetEigenvalues();
  const std::vector<RealType> eigenvaluesRef = EigenValuesSymmetricMatrix<RealType>( metricData.m_CovarianceMatrixRef ).GetEigenvalues();
  const std::vector<RealType> eigenvaluesFlt = EigenValuesSymmetricMatrix<RealType>( metricData.m_CovarianceMatrixFlt ).GetEigenvalues();

  const double EIGENVALUE_THRESHOLD = 1e-6;
  double determinant = 1.0, determinantRef = 1.0, determinantFlt = 1.0;
  for ( size_t i = 0; i < this->m_NumberOfChannels; ++i )
    {
    if ( eigenvalues[i] > EIGENVALUE_THRESHOLD )
      determinant *= eigenvalues[i];
    }

  for ( size_t i = 0; i < nRefs; ++i )
    {
    if ( eigenvaluesRef[i] > EIGENVALUE_THRESHOLD )
      determinantRef *= eigenvaluesRef[i];
    }
  
  for ( size_t i = 0; i < nFlts; ++i )
    {
    if ( eigenvaluesFlt[i] > EIGENVALUE_THRESHOLD )
      determinantFlt *= eigenvaluesFlt[i];
    }

  if ( (determinant > 0) && (determinantRef > 0) && (determinantFlt > 0) )
    {
    const static double alpha = 1.41893853320467;
    const double hxy = this->m_NumberOfChannels*alpha + .5*log( determinant );
    const double hx = nRefs*alpha + .5*log( determinantRef );
    const double hy = nFlts*alpha + .5*log( determinantFlt );
    
    if ( this->m_NormalizedMI )
      return static_cast<RealType>( (hx+hy) / hxy );
    else
      return static_cast<RealType>( hx+hy-hxy );
    }
  return -FLT_MAX;
}

} // namespace cmtk
