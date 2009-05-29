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

#include <cmtkSplineWarpXform.h>

#include <cmtkMathUtil.h>

#ifndef CMTK_VAR_AUTO_ARRAYSIZE
#  include <vector>
#endif

namespace
cmtk
{

/** \addtogroup Base */
//@{

CoordinateMatrix3x3
SplineWarpXform::GetJacobian( const Vector3D& v ) const
{
  CoordinateMatrix3x3 J;
  this->GetJacobian( v, J );
  return J;
}

void
SplineWarpXform::GetJacobianSequence
( CoordinateMatrix3x3 *const array, const int x, const int y, const int z, const size_t numberOfPoints ) 
  const
{
  const Types::Coordinate *spX = &splineX[x<<2], *spY = &splineY[y<<2], *spZ = &splineZ[z<<2];
  const Types::Coordinate *dspX = &dsplineX[x<<2], *dspY = &dsplineY[y<<2], *dspZ = &dsplineZ[z<<2];
  const Types::Coordinate *coeff = this->m_Parameters + gX[x] + gY[y] + gZ[z];

  // precompute the products of B_j(v) and B_k(w) for the 4 x 4 neighborhood
  // in y- and z-direction.
  Types::Coordinate smlX[16], smlY[16], smlZ[16];
  for ( int i = 0, m = 0; m < 4; ++m )
    {
    for ( int l = 0; l < 4; ++l, ++i )
      {
      smlX[i] =  spZ[m] *  spY[l];
      smlY[i] =  spZ[m] * dspY[l];
      smlZ[i] = dspZ[m] *  spY[l];
      }
    }
  
  // determine the number of CPG cells on our way along the row
  const int numberOfCells = (gX[x + numberOfPoints - 1] - gX[x]) / nextI + 4;
  
  // pre-compute the contributions of all control points in y- and z-direction
  // along the way
  Types::Coordinate phiCompX, phiCompY, phiCompZ;

#ifdef CMTK_VAR_AUTO_ARRAYSIZE
  Types::Coordinate phiHatX[3*numberOfCells];
  Types::Coordinate phiHatY[3*numberOfCells];
  Types::Coordinate phiHatZ[3*numberOfCells];
#else
  std::vector<Types::Coordinate> phiHatX(3*numberOfCells);
  std::vector<Types::Coordinate> phiHatY(3*numberOfCells);
  std::vector<Types::Coordinate> phiHatZ(3*numberOfCells);
#endif

  const int *gpo;
  int phiIdx = 0;
  for ( int cell = 0; cell < numberOfCells; ++cell, coeff += nextI ) 
    {
    gpo = &GridPointOffset[0];
    for ( int dim = 0; dim < 3; ++dim, ++phiIdx ) 
      {
      phiCompX = phiCompY = phiCompZ = 0;
      for ( int ml = 0; ml < 16; ++ml, ++gpo ) 
	{
	phiCompX += coeff[ *gpo ] * smlX[ml];
	phiCompY += coeff[ *gpo ] * smlY[ml];
	phiCompZ += coeff[ *gpo ] * smlZ[ml];
	}
      phiHatX[phiIdx] = phiCompX;
      phiHatY[phiIdx] = phiCompY;
      phiHatZ[phiIdx] = phiCompZ;
      }
    }
  
  // start at the leftmost precomputed CPG cell
  int cellIdx = 0;

  CoordinateMatrix3x3 J;
  // run over all points we're supposed to transform
  int i = x;
  for ( const int lastPoint = x + numberOfPoints; i < lastPoint; ) 
    {
    // these change only when we enter a new cell
#ifdef CMTK_VAR_AUTO_ARRAYSIZE
    const Types::Coordinate* phiPtrX = phiHatX + 3*cellIdx;
    const Types::Coordinate* phiPtrY = phiHatY + 3*cellIdx;
    const Types::Coordinate* phiPtrZ = phiHatZ + 3*cellIdx;
#else
    const Types::Coordinate* phiPtrX = &phiHatX[3*cellIdx];
    const Types::Coordinate* phiPtrY = &phiHatY[3*cellIdx];
    const Types::Coordinate* phiPtrZ = &phiHatZ[3*cellIdx];
#endif

    // do everything inside one cell
    do 
      {
      // compute transformed voxel by taking precomputed y- and z-contributions
      // and adding x. The loops to do this have been unrolled for increased
      // performance.
      J[0][0] = this->InverseSpacing[0] * 
	( dspX[0] * phiPtrX[0] + dspX[1] * phiPtrX[3] + dspX[2] * phiPtrX[6] + dspX[3] * phiPtrX[9] );
      J[0][1] = this->InverseSpacing[0] * 
	( dspX[0] * phiPtrX[1] + dspX[1] * phiPtrX[4] + dspX[2] * phiPtrX[7] + dspX[3] * phiPtrX[10] );
      J[0][2] = this->InverseSpacing[0] * 
	( dspX[0] * phiPtrX[2] + dspX[1] * phiPtrX[5] + dspX[2] * phiPtrX[8] + dspX[3] * phiPtrX[11] );

      J[1][0] = this->InverseSpacing[1] * 
	( spX[0] * phiPtrY[0] + spX[1] * phiPtrY[3] + spX[2] * phiPtrY[6] + spX[3] * phiPtrY[9] );
      J[1][1] = this->InverseSpacing[1] * 
	( spX[0] * phiPtrY[1] + spX[1] * phiPtrY[4] + spX[2] * phiPtrY[7] + spX[3] * phiPtrY[10] );
      J[1][2] = this->InverseSpacing[1] * 
	( spX[0] * phiPtrY[2] + spX[1] * phiPtrY[5] + spX[2] * phiPtrY[8] + spX[3] * phiPtrY[11] );

      J[2][0] = this->InverseSpacing[2] * 
	( spX[0] * phiPtrZ[0] + spX[1] * phiPtrZ[3] + spX[2] * phiPtrZ[6] + spX[3] * phiPtrZ[9] );
      J[2][1] = this->InverseSpacing[2] * 
	( spX[0] * phiPtrZ[1] + spX[1] * phiPtrZ[4] + spX[2] * phiPtrZ[7] + spX[3] * phiPtrZ[10] );
      J[2][2] = this->InverseSpacing[2] * 
	( spX[0] * phiPtrZ[2] + spX[1] * phiPtrZ[5] + spX[2] * phiPtrZ[8] + spX[3] * phiPtrZ[11] );

      array[i-x].Set( &J[0][0] );

      // go to next voxel
      ++i;
      spX += 4;
      dspX += 4;
      // repeat this until we leave current CPG cell.
    } while ( ( gX[i-1] == gX[i] ) && ( i < lastPoint ) );

    // we've just left a cell -- shift index of precomputed control points
    // to the next one.
    ++cellIdx;
  }
}

void
SplineWarpXform::GetJacobianAtControlPoint
( const Types::Coordinate* cp, CoordinateMatrix3x3& J ) const
{
  J.Fill( 0.0 );
  
  const double  sp[3] = {  1.0/6, 2.0/3, 1.0/6 };
  const double dsp[3] = { -1.0/2,     0, 1.0/2 };

  const Types::Coordinate* coeff = cp - nextI - nextJ - nextK;
  for ( int dim = 0; dim<3; ++dim ) {
    const Types::Coordinate *coeff_mm = coeff;
    for ( int m = 0; m < 3; ++m ) {
      Types::Coordinate ll[3] = { 0, 0, 0 };
      const Types::Coordinate *coeff_ll = coeff_mm;
      for ( int l = 0; l < 3; ++l ) {
	Types::Coordinate kk[3] = { 0, 0, 0 };
	const Types::Coordinate *coeff_kk = coeff_ll;
	for ( int k = 0; k < 3; ++k, coeff_kk += nextI ) {
	  kk[0] += dsp[k] * (*coeff_kk);
	  kk[1] +=  sp[k] * (*coeff_kk);
	  kk[2] +=  sp[k] * (*coeff_kk);
	}
	ll[0] +=  sp[l] * kk[0];
	ll[1] += dsp[l] * kk[1];
	ll[2] +=  sp[l] * kk[2];
	coeff_ll += nextJ;
      }	
      J[0][dim] +=  sp[m] * ll[0];
      J[1][dim] +=  sp[m] * ll[1];
      J[2][dim] += dsp[m] * ll[2];
      coeff_mm += nextK;
    }
    ++coeff;
  }
  for ( int i = 0; i<3; ++i ) 
    {
    for ( int j = 0; j<3; ++j )
      J[i][j] *= this->InverseSpacing[i];
    }
}

void
SplineWarpXform::GetJacobian
( const Vector3D& v, CoordinateMatrix3x3& J ) const
{
  Types::Coordinate r[3], f[3];
  int grid[3];
  
  for ( int dim = 0; dim<3; ++dim ) 
    {
    r[dim] = this->InverseSpacing[dim] * v.XYZ[dim];
    grid[dim] = std::min( static_cast<int>( r[dim] ), this->Dims[dim]-4 );
    f[dim] = r[dim] - grid[dim];
    assert( (f[dim] >= 0.0) && (f[dim] <= 1.0) );
    }

  const Types::Coordinate* coeff = this->m_Parameters + 3 * ( grid[0] + this->Dims[0] * (grid[1] + this->Dims[1] * grid[2]) );
  
  // loop over the three components of the coordinate transformation function,
  // x, y, z.
  for ( int dim = 0; dim<3; ++dim, ++coeff ) 
    {
    const Types::Coordinate *coeff_mm = coeff;
    for ( int m = 0; m < 4; ++m, coeff_mm += nextK ) 
      {
      Types::Coordinate ll[3] = { 0, 0, 0 };
      const Types::Coordinate *coeff_ll = coeff_mm;
      for ( int l = 0; l < 4; ++l, coeff_ll += nextJ ) 
	{
	Types::Coordinate kk[3] = { 0, 0, 0 };
	const Types::Coordinate *coeff_kk = coeff_ll;
	
	for ( int k = 0; k < 4; ++k, coeff_kk+=3 ) 
	  {
	  // dT / dx
	  kk[0] += CubicSpline::DerivApproxSpline( k, f[0] ) * (*coeff_kk);
	  // dT / dy
	  const Types::Coordinate tmp = CubicSpline::ApproxSpline( k, f[0] ) * (*coeff_kk);
	  kk[1] += tmp;
	  // dT / dz
	  kk[2] += tmp;
	  }
	
	// dT / dx
	const Types::Coordinate tmp = CubicSpline::ApproxSpline( l, f[1] );
	ll[0] += tmp * kk[0];
	// dT / dy
	ll[1] += CubicSpline::DerivApproxSpline( l, f[1] ) * kk[1];
	// dT / dz
	ll[2] += tmp * kk[2];
	}

      // dT / dx
      const Types::Coordinate tmp = CubicSpline::ApproxSpline( m, f[2] );
      J[dim][0] += tmp * ll[0];
      // dT / dy
      J[dim][1] += tmp * ll[1];
      // dT / dz
      J[dim][2] += CubicSpline::DerivApproxSpline( m, f[2] ) * ll[2];
      }
    }

  // scale with grid spacing to normalize Jacobian (chain rule of derivation)
  for ( int i = 0; i<3; ++i ) 
    {
    for ( int j = 0; j<3; ++j )
      J[i][j] *= this->InverseSpacing[i];
    }
}

Types::Coordinate
SplineWarpXform::GetJacobianDeterminant
( const int x, const int y, const int z ) const
{
  const Types::Coordinate *spX = &splineX[x<<2], *spY = &splineY[y<<2], *spZ = &splineZ[z<<2];
  const Types::Coordinate *dspX = &dsplineX[x<<2], *dspY = &dsplineY[y<<2], *dspZ = &dsplineZ[z<<2];
  const Types::Coordinate *coeff = this->m_Parameters + gX[x] + gY[y] + gZ[z];
  
  double J[3][3];
  memset( J, 0, sizeof( J ) );
  for ( int dim = 0; dim<3; ++dim ) 
    {
    const Types::Coordinate *coeff_mm = coeff;
    for ( int m = 0; m < 4; ++m ) 
      {
      Types::Coordinate ll[3] = { 0, 0, 0 };
      const Types::Coordinate *coeff_ll = coeff_mm;
      for ( int l = 0; l < 4; ++l ) 
	{
	Types::Coordinate kk[3] = { 0, 0, 0 };
	const Types::Coordinate *coeff_kk = coeff_ll;
	for ( int k = 0; k < 4; ++k, coeff_kk+=3 ) 
	  {
	  kk[0] += dspX[k] * (*coeff_kk);
	  kk[1] += spX[k];
	  kk[2] += spX[k];
	  }
	ll[0] += spY[l] * kk[0];
	ll[1] += dspY[l] * kk[1];
	ll[2] += spY[l] * kk[2];
	coeff_ll += nextJ;
	}	
      J[0][dim] += spZ[m] * ll[0];
      J[1][dim] += spZ[m] * ll[1];
      J[2][dim] += dspZ[m] * ll[2];
      coeff_mm += nextK;
      }
    ++coeff;
    }
  
  return InverseSpacing[0] * InverseSpacing[1] * InverseSpacing[2] * 
    ( J[0][0] * (J[1][1]*J[2][2] - J[1][2]*J[2][1]) + 
      J[0][1] * (J[1][2]*J[2][0] - J[1][0]*J[2][2]) + 
      J[0][2] * (J[1][0]*J[2][1] - J[1][1]*J[2][0]) );
}

Types::Coordinate
SplineWarpXform::GetJacobianDeterminant
( const Vector3D& v ) const
{
  Types::Coordinate r[3], f[3];
  int grid[3];
  
  double J[3][3];
  memset( J, 0, sizeof( J ) );

  for ( int dim = 0; dim<3; ++dim ) 
    {
    r[dim] = InverseSpacing[dim] * v.XYZ[dim];
    grid[dim] = std::min( static_cast<int>( r[dim] ), Dims[dim]-4 );
    f[dim] = r[dim] - grid[dim];
    }
  
  const Types::Coordinate* coeff = this->m_Parameters + 3 * ( grid[0] + Dims[0] * (grid[1] + Dims[1] * grid[2]) );
  
  for ( int dim = 0; dim<3; ++dim ) 
    {
    const Types::Coordinate *coeff_mm = coeff;
    for ( int m = 0; m < 4; ++m ) 
      {
      Types::Coordinate ll[3] = { 0, 0, 0 };
      const Types::Coordinate *coeff_ll = coeff_mm;
      for ( int l = 0; l < 4; ++l ) 
	{
	Types::Coordinate kk[3] = { 0, 0, 0 };
	const Types::Coordinate *coeff_kk = coeff_ll;
	for ( int k = 0; k < 4; ++k, coeff_kk+=3 ) 
	  {
	  kk[0] += CubicSpline::DerivApproxSpline( k, f[0] ) * (*coeff_kk);
	  const Types::Coordinate tmp = CubicSpline::ApproxSpline( k, f[0] ) * (*coeff_kk);
	  kk[1] += tmp;
	  kk[2] += tmp;
	  }
	const Types::Coordinate tmp = CubicSpline::ApproxSpline( l, f[1] );
	ll[0] += tmp * kk[0];
	ll[1] += CubicSpline::DerivApproxSpline( l, f[1] ) * kk[1];
	ll[2] += tmp * kk[2];
	coeff_ll += nextJ;
	}	
      const Types::Coordinate tmp = CubicSpline::ApproxSpline( m, f[2] );
      J[0][dim] += tmp * ll[0];
      J[1][dim] += tmp * ll[1];
      J[2][dim] += CubicSpline::DerivApproxSpline( m, f[2] ) * ll[2];
      coeff_mm += nextK;
      }
    ++coeff;
    }
  
  return InverseSpacing[0] * InverseSpacing[1] * InverseSpacing[2] * 
    ( J[0][0] * (J[1][1]*J[2][2] - J[1][2]*J[2][1]) + 
      J[0][1] * (J[1][2]*J[2][0] - J[1][0]*J[2][2]) + 
      J[0][2] * (J[1][0]*J[2][1] - J[1][1]*J[2][0]) );
}

void
SplineWarpXform::GetJacobianDeterminantSequence
( double *const values, const int x, const int y, const int z, 
  const size_t numberOfPoints ) 
  const
{
  const Types::Coordinate *spX = &splineX[x<<2], *spY = &splineY[y<<2], *spZ = &splineZ[z<<2];
  const Types::Coordinate *dspX = &dsplineX[x<<2], *dspY = &dsplineY[y<<2], *dspZ = &dsplineZ[z<<2];
  const Types::Coordinate *coeff = this->m_Parameters + gX[x] + gY[y] + gZ[z];

  const Types::Coordinate globalInverseSpacing = InverseSpacing[0] * InverseSpacing[1] * InverseSpacing[2];

  // precompute the products of B_j(v) and B_k(w) for the 4 x 4 neighborhood
  // in y- and z-direction.
  Types::Coordinate smlX[16], smlY[16], smlZ[16];
  for ( int i = 0, m = 0; m < 4; ++m )
    {
    for ( int l = 0; l < 4; ++l, ++i )
      {
      smlX[i] =  spZ[m] *  spY[l];
      smlY[i] =  spZ[m] * dspY[l];
      smlZ[i] = dspZ[m] *  spY[l];
      }
    }
  
  // determine the number of CPG cells on our way along the row
  const int numberOfCells = (gX[x + numberOfPoints - 1] - gX[x]) / nextI + 4;
  
  // pre-compute the contributions of all control points in y- and z-direction
  // along the way
  Types::Coordinate phiCompX, phiCompY, phiCompZ;

#ifdef CMTK_VAR_AUTO_ARRAYSIZE
  Types::Coordinate phiHatX[3*numberOfCells];
  Types::Coordinate phiHatY[3*numberOfCells];
  Types::Coordinate phiHatZ[3*numberOfCells];
#else
  std::vector<Types::Coordinate> phiHatX(3*numberOfCells);
  std::vector<Types::Coordinate> phiHatY(3*numberOfCells);
  std::vector<Types::Coordinate> phiHatZ(3*numberOfCells);
#endif

  const int *gpo;
  int phiIdx = 0;
  for ( int cell = 0; cell < numberOfCells; ++cell, coeff += nextI ) 
    {
    gpo = &GridPointOffset[0];
    for ( int dim = 0; dim < 3; ++dim, ++phiIdx ) 
      {
      phiCompX = phiCompY = phiCompZ = 0;
      for ( int ml = 0; ml < 16; ++ml, ++gpo ) 
	{
	phiCompX += coeff[ *gpo ] * smlX[ml];
	phiCompY += coeff[ *gpo ] * smlY[ml];
	phiCompZ += coeff[ *gpo ] * smlZ[ml];
	}
      phiHatX[phiIdx] = phiCompX;
      phiHatY[phiIdx] = phiCompY;
      phiHatZ[phiIdx] = phiCompZ;
      }
    }
  
  // start at the leftmost precomputed CPG cell
  int cellIdx = 0;

  Types::Coordinate JXX, JXY, JXZ, JYX, JYY, JYZ, JZX, JZY, JZZ;
  // run over all points we're supposed to transform
  int i = x;
  for ( const int lastPoint = x + numberOfPoints; i < lastPoint; ) 
    {
    // these change only when we enter a new cell
#ifdef CMTK_VAR_AUTO_ARRAYSIZE
    const Types::Coordinate* phiPtrX = phiHatX + 3*cellIdx;
    const Types::Coordinate* phiPtrY = phiHatY + 3*cellIdx;
    const Types::Coordinate* phiPtrZ = phiHatZ + 3*cellIdx;
#else
    const Types::Coordinate* phiPtrX = &phiHatX[3*cellIdx];
    const Types::Coordinate* phiPtrY = &phiHatY[3*cellIdx];
    const Types::Coordinate* phiPtrZ = &phiHatZ[3*cellIdx];
#endif

    // do everything inside one cell
    do 
      {
      // compute transformed voxel by taking precomputed y- and z-contributions
      // and adding x. The loops to do this have been unrolled for increased
      // performance.
      JXX = dspX[0] * phiPtrX[0] + dspX[1] * phiPtrX[3] + dspX[2] * phiPtrX[6] + dspX[3] * phiPtrX[9];
      JXY = dspX[0] * phiPtrX[1] + dspX[1] * phiPtrX[4] + dspX[2] * phiPtrX[7] + dspX[3] * phiPtrX[10];
      JXZ = dspX[0] * phiPtrX[2] + dspX[1] * phiPtrX[5] + dspX[2] * phiPtrX[8] + dspX[3] * phiPtrX[11];

      JYX = spX[0] * phiPtrY[0] + spX[1] * phiPtrY[3] + spX[2] * phiPtrY[6] + spX[3] * phiPtrY[9];
      JYY = spX[0] * phiPtrY[1] + spX[1] * phiPtrY[4] + spX[2] * phiPtrY[7] + spX[3] * phiPtrY[10];
      JYZ = spX[0] * phiPtrY[2] + spX[1] * phiPtrY[5] + spX[2] * phiPtrY[8] + spX[3] * phiPtrY[11];

      JZX = spX[0] * phiPtrZ[0] + spX[1] * phiPtrZ[3] + spX[2] * phiPtrZ[6] + spX[3] * phiPtrZ[9];
      JZY = spX[0] * phiPtrZ[1] + spX[1] * phiPtrZ[4] + spX[2] * phiPtrZ[7] + spX[3] * phiPtrZ[10];
      JZZ = spX[0] * phiPtrZ[2] + spX[1] * phiPtrZ[5] + spX[2] * phiPtrZ[8] + spX[3] * phiPtrZ[11];

      values[i-x] = globalInverseSpacing * 
	( JXX * (JYY*JZZ - JYZ*JZY) + JXY * (JYZ*JZX - JYX*JZZ) + JXZ * (JYX*JZY - JYY*JZX) );

      // go to next voxel
      ++i;
      spX += 4;
      dspX += 4;
      // repeat this until we leave current CPG cell.
    } while ( ( gX[i-1] == gX[i] ) && ( i < lastPoint ) );

    // we've just left a cell -- shift index of precomputed control points
    // to the next one.
    ++cellIdx;
  }
}

Types::Coordinate
SplineWarpXform::JacobianDeterminant ( const Types::Coordinate *cp ) const
{
  CoordinateMatrix3x3 J;
  this->GetJacobianAtControlPoint( cp, J );
  
  return InverseSpacing[0] * InverseSpacing[1] * InverseSpacing[2] * 
    ( J[0][0] * (J[1][1]*J[2][2] - J[1][2]*J[2][1]) + 
      J[0][1] * (J[1][2]*J[2][0] - J[1][0]*J[2][2]) + 
      J[0][2] * (J[1][0]*J[2][1] - J[1][1]*J[2][0]) );
}

CMTK_THREAD_RETURN_TYPE
SplineWarpXform
::GetJacobianConstraintThreads( void *arg )
{
  Self::JacobianConstraintThreadInfo *info = static_cast<Self::JacobianConstraintThreadInfo*>( arg );

  const SplineWarpXform *me = info->thisObject;

  int pixelsPerRow = me->VolumeDims[0];
  std::vector<double> valuesJ( pixelsPerRow );

  int rowCount = ( me->VolumeDims[1] * me->VolumeDims[2] );
  int rowFrom = ( rowCount / info->NumberOfThreads ) * info->ThisThreadIndex;
  int rowTo = ( info->ThisThreadIndex == (info->NumberOfThreads-1) ) ? rowCount : ( rowCount/info->NumberOfThreads ) * (info->ThisThreadIndex+1);
  int rowsToDo = rowTo - rowFrom;

  int yFrom = rowFrom % me->VolumeDims[1];
  int zFrom = rowFrom / me->VolumeDims[2];

  double constraint = 0;
  if ( me->IncompressibilityMap )
    {
    for ( int z = zFrom; (z < me->VolumeDims[2]) && rowsToDo; ++z ) 
      {
      for ( int y = yFrom; (y < me->VolumeDims[1]) && rowsToDo; yFrom = 0, ++y, --rowsToDo ) 
	{
	me->GetJacobianDeterminantSequence( &(valuesJ[0]), 0, y, z, pixelsPerRow );
	for ( int x = 0; x < pixelsPerRow; ++x ) 
	  {
	  constraint += me->IncompressibilityMap->GetDataAt( x, y, z ) * fabs( log ( valuesJ[x] / me->GlobalScaling ) );
	  }
	}
      }
    }
  else
    { // no voxel-by-voxel constraint
    for ( int z = zFrom; (z < me->VolumeDims[2]) && rowsToDo; ++z ) 
      {
      for ( int y = yFrom; (y < me->VolumeDims[1]) && rowsToDo; yFrom = 0, ++y, --rowsToDo ) 
	{
	me->GetJacobianDeterminantSequence( &(valuesJ[0]), 0, y, z, pixelsPerRow );
	for ( int x = 0; x < pixelsPerRow; ++x ) 
	  {
	  constraint += fabs( log ( valuesJ[x] / me->GlobalScaling ) );
	  }
	}
      }
    }
  
  // Divide by number of control points to normalize with respect to the
  // number of local Jacobians in the computation.
  info->Constraint = constraint;

  return CMTK_THREAD_RETURN_VALUE;
}

CMTK_THREAD_RETURN_TYPE
SplineWarpXform
::GetJacobianFoldingConstraintThreads( void *arg )
{
  Self::JacobianConstraintThreadInfo *info = static_cast<Self::JacobianConstraintThreadInfo*>( arg );

  const SplineWarpXform *me = info->thisObject;

  int pixelsPerRow = me->VolumeDims[0];
  std::vector<double> valuesJ( pixelsPerRow );

  int rowCount = ( me->VolumeDims[1] * me->VolumeDims[2] );
  int rowFrom = ( rowCount / info->NumberOfThreads ) * info->ThisThreadIndex;
  int rowTo = ( info->ThisThreadIndex == (info->NumberOfThreads-1) ) ? rowCount : ( rowCount/info->NumberOfThreads ) * (info->ThisThreadIndex+1);
  int rowsToDo = rowTo - rowFrom;

  int yFrom = rowFrom % me->VolumeDims[1];
  int zFrom = rowFrom / me->VolumeDims[2];

  double constraint = 0;
  for ( int z = zFrom; (z < me->VolumeDims[2]) && rowsToDo; ++z ) 
    {
    for ( int y = yFrom; (y < me->VolumeDims[1]) && rowsToDo; yFrom = 0, ++y, --rowsToDo ) 
      {
      me->GetJacobianDeterminantSequence( &(valuesJ[0]), 0, y, z, pixelsPerRow );
      for ( int x = 0; x < pixelsPerRow; ++x ) 
	{
	constraint += fabs( me->GlobalScaling / valuesJ[x] + valuesJ[x] / me->GlobalScaling - 2 );
	}
      }
    }
  
  // Divide by number of control points to normalize with respect to the
  // number of local Jacobians in the computation.
  info->Constraint = constraint;
  
  return CMTK_THREAD_RETURN_VALUE;
}

Types::Coordinate
SplineWarpXform::GetJacobianConstraint () const
{
  unsigned short numberOfThreads = std::min( Threads::GetNumberOfThreads(), Dims[ 2 ] );

  // Info blocks for parallel threads that evaulate the constraint.
  Self::JacobianConstraintThreadInfo *ConstraintThreadInfo = Memory::AllocateArray<Self::JacobianConstraintThreadInfo>( numberOfThreads );

  for ( int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx ) 
    {
    ConstraintThreadInfo[threadIdx].thisObject = this;
    ConstraintThreadInfo[threadIdx].ThisThreadIndex = threadIdx;
    ConstraintThreadInfo[threadIdx].NumberOfThreads = numberOfThreads;
    }
  
  Threads::RunThreads( SplineWarpXform::GetJacobianConstraintThreads, numberOfThreads, ConstraintThreadInfo );
  
  double constraint = 0;
  for ( size_t threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx ) 
    {
    constraint += ConstraintThreadInfo[threadIdx].Constraint;
    }
  delete[] ConstraintThreadInfo;

  // Divide by number of control points to normalize with respect to the
  // number of local Jacobians in the computation.
  return constraint / ( VolumeDims[0] * VolumeDims[1] * VolumeDims[2] );
}

Types::Coordinate
SplineWarpXform::GetJacobianFoldingConstraint () const
{
  unsigned short numberOfThreads = std::min( Threads::GetNumberOfThreads(), Dims[ 2 ] );

  // Info blocks for parallel threads that evaulate the constraint.
  Self::JacobianConstraintThreadInfo *ConstraintThreadInfo = Memory::AllocateArray<Self::JacobianConstraintThreadInfo>( numberOfThreads );

  for ( int threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx ) 
    {
    ConstraintThreadInfo[threadIdx].thisObject = this;
    ConstraintThreadInfo[threadIdx].ThisThreadIndex = threadIdx;
    ConstraintThreadInfo[threadIdx].NumberOfThreads = numberOfThreads;
    }
  
  Threads::RunThreads( SplineWarpXform::GetJacobianFoldingConstraintThreads, numberOfThreads, ConstraintThreadInfo );
  
  double constraint = 0;
  for ( size_t threadIdx = 0; threadIdx < numberOfThreads; ++threadIdx ) 
    {
    constraint += ConstraintThreadInfo[threadIdx].Constraint;
    }
  delete[] ConstraintThreadInfo;
  
  // Divide by number of control points to normalize with respect to the
  // number of local Jacobians in the computation.
  return constraint / ( VolumeDims[0] * VolumeDims[1] * VolumeDims[2] );
}

Types::Coordinate
SplineWarpXform::GetJacobianConstraintSparse () const
{
  double Constraint = 0;

  const Types::Coordinate* coeff = this->m_Parameters + nextI + nextJ + nextK;
  for ( int z = 1; z<Dims[2]-1; ++z, coeff+=2*nextJ )
    for ( int y = 1; y<Dims[1]-1; ++y, coeff+=2*nextI )
      for ( int x = 1; x<Dims[0]-1; ++x, coeff+=nextI )
	Constraint += fabs( log ( this->GetJacobianDeterminant( coeff ) / GlobalScaling ) );
  
  // Divide by number of control points to normalize with respect to the
  // number of local Jacobians in the computation.
  return (double)(Constraint / this->NumberOfControlPoints);
}

void 
SplineWarpXform::GetJacobianConstraintDerivative
( double& lower, double& upper, const int param, const Rect3D& voi, const Types::Coordinate step ) const
{
  const int pixelsPerRow = voi.endX - voi.startX;
  std::vector<double> valuesJ( pixelsPerRow );
  
  double ground = 0;

  for ( int k = voi.startZ; k < voi.endZ; ++k )
    for ( int j = voi.startY; j < voi.endY; ++j ) 
      {
      this->GetJacobianDeterminantSequence( &(valuesJ[0]), voi.startX, j, k, pixelsPerRow );
      for ( int i = 0; i < pixelsPerRow; ++i )
	ground += fabs( log( valuesJ[i] / GlobalScaling ) );
      }
  
  upper = -ground;
  lower = -ground;
  
  const Types::Coordinate oldCoeff = this->m_Parameters[param];
  this->m_Parameters[param] += step;
  for ( int k = voi.startZ; k < voi.endZ; ++k )
    for ( int j = voi.startY; j < voi.endY; ++j ) 
      {
      this->GetJacobianDeterminantSequence( &(valuesJ[0]), voi.startX, j, k, pixelsPerRow );
      for ( int i = 0; i < pixelsPerRow; ++i )
	{
	upper += fabs( log( valuesJ[i] / GlobalScaling ) );
	}
      }
  
  this->m_Parameters[param] = oldCoeff - step;
  for ( int k = voi.startZ; k < voi.endZ; ++k )
    for ( int j = voi.startY; j < voi.endY; ++j ) 
      {
      this->GetJacobianDeterminantSequence( &(valuesJ[0]), voi.startX, j, k, pixelsPerRow );
      for ( int i = 0; i < pixelsPerRow; ++i )
	{
	lower += fabs( log( valuesJ[i] / GlobalScaling ) );
	}
      }
  this->m_Parameters[param] = oldCoeff;
  
  const double invVolume = 1.0 / ((voi.endX-voi.startX)*(voi.endY-voi.startY)*(voi.endZ-voi.startZ));
  upper *= invVolume;
  lower *= invVolume;
}

void 
SplineWarpXform::GetJacobianFoldingConstraintDerivative
( double& lower, double& upper, const int param, const Rect3D& voi, const Types::Coordinate step ) const
{
  const int pixelsPerRow = voi.endX - voi.startX;
  std::vector<double> valuesJ( pixelsPerRow );
  
  double ground = 0;

  for ( int k = voi.startZ; k < voi.endZ; ++k )
    for ( int j = voi.startY; j < voi.endY; ++j ) 
      {
      this->GetJacobianDeterminantSequence( &(valuesJ[0]), voi.startX, j, k, pixelsPerRow );
      for ( int i = 0; i < pixelsPerRow; ++i )
	ground += fabs( this->GlobalScaling / valuesJ[i] + valuesJ[i] / this->GlobalScaling - 2 );
      }
  
  upper = -ground;
  lower = -ground;
  
  const Types::Coordinate oldCoeff = this->m_Parameters[param];
  this->m_Parameters[param] += step;
  for ( int k = voi.startZ; k < voi.endZ; ++k )
    for ( int j = voi.startY; j < voi.endY; ++j ) 
      {
      this->GetJacobianDeterminantSequence( &(valuesJ[0]), voi.startX, j, k, pixelsPerRow );
      for ( int i = 0; i < pixelsPerRow; ++i )
	{
	upper += fabs( this->GlobalScaling / valuesJ[i] + valuesJ[i] / this->GlobalScaling - 2 );

	}
      }
  
  this->m_Parameters[param] = oldCoeff - step;
  for ( int k = voi.startZ; k < voi.endZ; ++k )
    for ( int j = voi.startY; j < voi.endY; ++j ) 
      {
      this->GetJacobianDeterminantSequence( &(valuesJ[0]), voi.startX, j, k, pixelsPerRow );
      for ( int i = 0; i < pixelsPerRow; ++i )
	{
	lower += fabs( this->GlobalScaling / valuesJ[i] + valuesJ[i] / this->GlobalScaling - 2 );
	}
      }
  this->m_Parameters[param] = oldCoeff;
  
  const double invVolume = 1.0 / ((voi.endX-voi.startX)*(voi.endY-voi.startY)*(voi.endZ-voi.startZ));
  upper *= invVolume;
  lower *= invVolume;
}

void 
SplineWarpXform::GetJacobianConstraintDerivative
( double& lower, double& upper, const int param, const Types::Coordinate step )
  const
{
  const int controlPointIdx = param / nextI;
  const unsigned short x =  ( controlPointIdx %  Dims[0] );
  const unsigned short y = ( (controlPointIdx /  Dims[0]) % Dims[1] );
  const unsigned short z = ( (controlPointIdx /  Dims[0]) / Dims[1] );
  
  const int thisDim = param % nextI;
  const Types::Coordinate* coeff = this->m_Parameters + param - thisDim;
  
  double ground = 0;

  const int iFrom = std::max( -1, 1-x );
  const int jFrom = std::max( -1, 1-y );
  const int kFrom = std::max( -1, 1-z );

  const int iTo = std::min( 1, Dims[0]-2-x );
  const int jTo = std::min( 1, Dims[1]-2-y );
  const int kTo = std::min( 1, Dims[2]-2-z );

  for ( int k = kFrom; k < kTo; ++k )
    for ( int j = jFrom; j < jTo; ++j )
      for ( int i = iFrom; i < iTo; ++i )
	ground += fabs( log( this->GetJacobianDeterminant(coeff + i*nextI + j*nextJ + k*nextK) / GlobalScaling ) );

  upper = -ground;
  lower = -ground;

  const Types::Coordinate oldCoeff = this->m_Parameters[param];
  this->m_Parameters[param] += step;
  for ( int k = kFrom; k < kTo; ++k )
    for ( int j = jFrom; j < jTo; ++j )
      for ( int i = iFrom; i < iTo; ++i )
	upper += fabs( log( this->GetJacobianDeterminant(coeff + i*nextI + j*nextJ + k*nextK) / GlobalScaling ) );

  this->m_Parameters[param] = oldCoeff - step;
  for ( int k = kFrom; k < kTo; ++k )
    for ( int j = jFrom; j < jTo; ++j )
      for ( int i = iFrom; i < iTo; ++i )
	lower += fabs( log( this->GetJacobianDeterminant(coeff + i*nextI + j*nextJ + k*nextK) / GlobalScaling ) );
  this->m_Parameters[param] = oldCoeff;

  upper /= this->NumberOfControlPoints;
  lower /= this->NumberOfControlPoints;
}

} // namespace cmtk
