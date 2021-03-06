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

#include "cmtkThreadPoolThreads.h"

#include <System/cmtkThreads.h>
#include <System/cmtkMutexLock.h>
#include <System/cmtkConsole.h>

/// This is the actual low-level thread function. It calls ThreadFunction() for the cmtk::ThreadPoolThreads instance given as the function parameter.
extern "C"
CMTK_THREAD_RETURN_TYPE
cmtkThreadPoolThreadFunction( CMTK_THREAD_ARG_TYPE arg )
{
  static_cast<cmtk::ThreadPoolThreads::ThreadPoolThreadsArg*>( arg )->m_Pool->ThreadFunction( static_cast<cmtk::ThreadPoolThreads::ThreadPoolThreadsArg*>( arg )->m_Index );
  return CMTK_THREAD_RETURN_VALUE;
}

namespace
cmtk
{

ThreadPoolThreads::ThreadPoolThreads( const size_t nThreads )
  : m_NumberOfTasks( 0 ),
    m_NextTaskIndex( 0 ),
    m_TaskFunction( NULL ),
    m_ThreadsRunning( false ),
    m_ContinueThreads( true )
{
  if ( ! nThreads )
    this->m_NumberOfThreads = cmtk::Threads::GetNumberOfThreads();
  else
    this->m_NumberOfThreads = nThreads;

#ifdef CMTK_USE_SMP  
  this->m_ThreadID.resize( this->m_NumberOfThreads, 0 );
#ifdef _MSC_VER
  this->m_ThreadHandles.resize( this->m_NumberOfThreads, 0 );
#endif
#endif
  this->m_ThreadArgs.resize( this->m_NumberOfThreads );
}

void
ThreadPoolThreads::StartThreads()
{
  if ( !this->m_ThreadsRunning )
    {
#ifdef CMTK_USE_SMP  
#ifdef CMTK_USE_PTHREADS
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    
    for ( size_t idx = 0; idx < this->m_NumberOfThreads; ++idx ) 
      {
      // set thread function arguments
      this->m_ThreadArgs[idx].m_Pool = this;
      this->m_ThreadArgs[idx].m_Index = idx;

      // start thread
      const int status = pthread_create( &this->m_ThreadID[idx], &attr, cmtkThreadPoolThreadFunction, static_cast<CMTK_THREAD_ARG_TYPE>( &this->m_ThreadArgs[idx] ) );
      
      if ( status ) 
	{
	StdErr.printf( "Creation of pooled thread #%u failed with status %d.\n", idx, status );
	exit( 1 );
	}
      }
    
    pthread_attr_destroy(&attr);
#elif defined(_MSC_VER)
    for ( size_t idx = 0; idx < this->m_NumberOfThreads; ++idx ) 
      {
      // set thread function arguments
      this->m_ThreadArgs[idx].m_Pool = this;
      this->m_ThreadArgs[idx].m_Index = idx;

      // nothing happened yet, so set status to OK
      int status = 0;

      // start thread
      this->m_ThreadHandles[idx] = CreateThread( NULL /*default security attributes*/, 0/*use default stack size*/, (LPTHREAD_START_ROUTINE) cmtkThreadPoolThreadFunction, 
						 static_cast<CMTK_THREAD_ARG_TYPE>(  &this->m_ThreadArgs[idx] ),  0/*use default creation flags*/, &this->m_ThreadID[idx] );
      if ( this->m_ThreadHandles[idx] == NULL ) 
	{
	status = -1;
	}
      
      if ( status ) 
	{
	StdErr.printf( "Creation of pooled thread #%u failed with status %d.\n", idx, status );
	exit( 1 );
	}
      }
#endif // #ifdef CMTK_USE_PTHREADS
#endif // #ifdef CMTK_USE_SMP
    this->m_ThreadsRunning = true;
    }
}

ThreadPoolThreads::~ThreadPoolThreads()
{
  this->EndThreads();
}


void
ThreadPoolThreads::EndThreads()
{
  if ( this->m_ThreadsRunning )
    {
#ifdef CMTK_USE_PTHREADS
    // set flag to terminate threads and post one semaphore per actual thread
    this->m_ContinueThreads = false;
    this->m_TaskWaitingSemaphore.Post( this->m_NumberOfThreads );
    for ( size_t idx = 0; idx < this->m_NumberOfThreads; ++idx ) 
      {
      if ( this->m_ThreadID[idx] ) 
	{
	pthread_join( this->m_ThreadID[idx], NULL );
	this->m_ThreadID[idx] = 0;
	}
      }
#elif defined(_MSC_VER)
    for ( size_t idx = 0; idx < this->m_NumberOfThreads; ++idx ) 
      {
      DWORD resultThread = 0;
      TerminateThread( this->m_ThreadHandles[idx], resultThread );
      }
#endif
    this->m_ThreadsRunning = false;
    }
}

void
ThreadPoolThreads::ThreadFunction( const size_t threadIdx )
{
#ifdef _OPENMP
  // Disable OpenMP inside thread
  omp_set_num_threads( 1 );
#endif

#ifdef CMTK_USE_SMP
  // wait for task waiting
  this->m_TaskWaitingSemaphore.Wait();
  while ( this->m_ContinueThreads )
    {
    // lock, get, increment next task index
    this->m_NextTaskIndexLock.Lock();
    const size_t taskIdx = this->m_NextTaskIndex;
    ++this->m_NextTaskIndex;
    this->m_NextTaskIndexLock.Unlock();
    
    // call task function
    this->m_TaskFunction( this->m_TaskParameters[taskIdx], taskIdx, this->m_NumberOfTasks, threadIdx, this->m_NumberOfThreads ); 

    // post "task done, thread waiting"
    this->m_ThreadWaitingSemaphore.Post();

    // wait for task waiting
    this->m_TaskWaitingSemaphore.Wait();
    }
#endif // #ifdef CMTK_USE_SMP
}

ThreadPoolThreads& 
ThreadPoolThreads::GetGlobalThreadPool()
{
  static ThreadPoolThreads globalThreadPoolThreads;
  return globalThreadPoolThreads;
}

}
