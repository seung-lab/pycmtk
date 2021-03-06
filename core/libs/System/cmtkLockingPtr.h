/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2011 SRI International
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

#ifndef __cmtkLockingPtr_h_included_
#define __cmtkLockingPtr_h_included_

#include <cmtkconfig.h>

#include <System/cmtkMutexLock.h>

namespace
cmtk
{

/** \addtogroup System */
//@{

/** Class for mutually exclusive access to objects.
 * This class is an adapted version of a concept by Andrei Alexandrescu of 
 * RealNetworks Inc.
 *\see http://www.cuj.com/experts/1902/alexandr.htm?topic=experts
 */
template<class T> 
class LockingPtr 
{
public:
  /// Create locking pointer and lock mutex.
  LockingPtr( T& object, MutexLock& mutexLock ) :
    m_Object( &object ), 
    m_MutexLock( &mutexLock )
  { 
    this->m_MutexLock->Lock();
  }

  /// Destroy locking pointer and unlock mutex.
  ~LockingPtr() 
  { 
    this->m_MutexLock->Unlock(); 
  }
  
  /// Dereferencing operator.
  T& operator*() 
  { 
    return *this->m_Object; 
  }

  /// Member access operator.
  T* operator->() 
  { 
    return this->m_Object; 
  }
                                  
private:
  /// Pointer to the accessed object.
  T* m_Object;

  /// The mutex lock.
  MutexLock* m_MutexLock;
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkLockingPtr_h_included_
