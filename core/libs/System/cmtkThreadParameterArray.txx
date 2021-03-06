/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011, 2013 SRI International
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

#include <System/cmtkConsole.h>

#ifdef _OPENMP
#  include <omp.h>
#endif

namespace
cmtk
{

/** \addtogroup System */
//@{

template<class TClass,class TParam>
void
ThreadParameterArray<TClass,TParam>
::RunInParallelFIFO
( ThreadFunction threadCall, const size_t numberOfThreadsTotal, const size_t firstThreadIdx )
{
#ifdef _OPENMP
  const int nThreadsOMP = std::max<int>( 1, 1+GetNumberOfThreads()-this->m_NumberOfThreads );
  omp_set_num_threads( nThreadsOMP );
#endif

#ifndef CMTK_USE_PTHREADS
  // we're not actually using threads, so simply run everything "by hand".
  for ( size_t threadIdx = 0; threadIdx < numberOfThreadsTotal; ++threadIdx ) 
    {
    this->m_Ptr[0].ThisThreadIndex = firstThreadIdx + threadIdx;
    threadCall( this->m_Ptr );
    }
#else
  if ( this->m_NumberOfThreads == 1 )
    {
    for ( size_t threadIdx = 0; threadIdx < numberOfThreadsTotal; ++threadIdx ) 
      {
      this->m_Ptr[0].ThisThreadIndex = firstThreadIdx + threadIdx;
      threadCall( this->m_Ptr );
      }
    }
  else
    {
#ifdef _MSC_VER
#else
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#endif // #ifdef _MSC_VER
      
    /* Initialization phase -- start first batch of parallel threads. */
    size_t threadIdx = 0;
    for ( ; (threadIdx < this->m_NumberOfThreads) && (threadIdx < numberOfThreadsTotal); ++threadIdx ) 
      {
      this->m_Ptr[threadIdx].ThisThreadIndex = firstThreadIdx + threadIdx;
#ifdef _MSC_VER
      this->m_Ptr[threadIdx].m_Handle = CreateThread( NULL /*default security attributes*/, 0 /*use default stack size*/, (LPTHREAD_START_ROUTINE) threadCall, 
						      &this->m_Ptr[threadIdx], 0 /*use default creation flags*/, &this->m_Ptr[threadIdx].m_ThreadID );
      if ( this->m_Ptr[threadIdx].m_Handle == NULL )
	{
	fprintf( stderr, "Creation of thread #%d failed.\n", (int)threadIdx );
	exit( 1 );
	}
#else
      int status = pthread_create( &this->m_Ptr[threadIdx].m_ThreadID, &attr, threadCall, &this->m_Ptr[threadIdx] );	
      if ( status ) 
	{
	fprintf( stderr, "Creation of thread #%d failed with status %d.\n", (int)threadIdx, (int)status );
	exit( 1 );
	}
#endif
      }
      
    /* Sustained phase -- start new thread whenever oldest one completes. */
    size_t nextThreadToJoin = 0;
    while ( threadIdx < numberOfThreadsTotal )
      {
#ifndef _MSC_VER
      void *resultThread;
#endif
      if ( this->m_Ptr[threadIdx].m_ThreadID ) 
	{
#ifdef _MSC_VER
#else
	pthread_join( this->m_Ptr[threadIdx].m_ThreadID, &resultThread );
#endif
	}
	
      this->m_Ptr[nextThreadToJoin].ThisThreadIndex = firstThreadIdx + threadIdx;
	
#ifdef _MSC_VER
      this->m_Ptr[nextThreadToJoin].m_Handle = CreateThread( NULL /*default security attributes*/, 0 /*use default stack size*/, (LPTHREAD_START_ROUTINE) threadCall, 
							     &this->m_Ptr[nextThreadToJoin],0 /*use default creation flags*/, &this->m_Ptr[nextThreadToJoin].m_ThreadID );
      if ( this->m_Ptr[nextThreadToJoin].m_Handle == NULL) 
	{
	fprintf( stderr, "Creation of thread #%d failed.\n", (int)threadIdx );
	exit( 1 );
	}
#else
      int status = pthread_create( &this->m_Ptr[nextThreadToJoin].m_ThreadID, &attr, threadCall, &this->m_Ptr[nextThreadToJoin] );
      if ( status ) 
	{
	fprintf( stderr, "Creation of thread #%d failed with status %d.\n", (int)threadIdx, (int)status );
	exit( 1 );
	}
#endif	

      ++threadIdx;
      nextThreadToJoin = (nextThreadToJoin + 1) % this->m_NumberOfThreads;
      }

    /* Cleanup phase -- Collect remaining thread results. */
    for ( threadIdx = 0; (threadIdx < this->m_NumberOfThreads) && (threadIdx < numberOfThreadsTotal); ++threadIdx ) 
      {
#ifndef _MSC_VER
      void *resultThread;
#endif
      if ( this->m_Ptr[nextThreadToJoin].m_ThreadID ) 
	{
#ifdef _MSC_VER
#else
	pthread_join( this->m_Ptr[nextThreadToJoin].m_ThreadID, &resultThread );
#endif
	}
	
      nextThreadToJoin = (nextThreadToJoin + 1) % this->m_NumberOfThreads;
      }
      
#ifdef _MSC_VER
#else
    pthread_attr_destroy(&attr);
#endif
    }
#endif // #ifndef CMTK_USE_PTHREADS

#ifdef _OPENMP
  omp_set_num_threads( GetNumberOfThreads() );
#endif
}

} // namespace cmtk
