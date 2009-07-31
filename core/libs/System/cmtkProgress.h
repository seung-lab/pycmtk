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

#ifndef __cmtkProgress_h_included_
#define __cmtkProgress_h_included_

#include <cmtkconfig.h>

#include <string>

namespace
cmtk
{

/** \addtogroup System */
//@{

/// Status code returned by SetPercentDone() method.
typedef enum {
  /// Everything okay; continue as usual.
  PROGRESS_OK,
  /// User requests interrupt of operation.
  PROGRESS_INTERRUPT,
  /// Interrupt generated by timeout.
  PROGRESS_TIMEOUT,
  /// Something went wrong.
  PROGRESS_FAILED
} ProgressResult;

/** Generic class for progress indication.
 */
class Progress 
{
public:
  /// This class.
  typedef Progress Self;

  /// Virtual (dummy) destructor.
  virtual ~Progress() {};
  
  /// Set total number of steps to complete.
  static void SetTotalSteps( const unsigned int totalSteps, const std::string& taskName = std::string("") );

  /// Set number of tasks completed.
  static ProgressResult SetProgress( const unsigned int progress );

  /// Done with progress indicator.
  static void Done();

  /// Set number of tasks completed.
  virtual ProgressResult SetProgressVirtual( const unsigned int progress ) = 0;

  /// Set progress handler instance.
  static void SetProgressInstance( Self *const progressInstance ) 
  {
    ProgressInstance = progressInstance;
  }
  
private:
  /// Instance of a derived class that handles GUI interaction etc.
  static Self* ProgressInstance;

protected:
  /// Name of the current task.
  static std::string m_CurrentTaskName;

  /// Total number of steps in current task.
  static unsigned int TotalSteps;

  /** Set total number of steps to complete.
   * This member function can be overriden by derived classes.
   */
  virtual void SetTotalStepsVirtual( const unsigned int ) {};

  /** Clean up progress output.
   * This member function can be overriden by derived classes.
   */
  virtual void DoneVirtual() {};
};

//@}

} // namespace cmtk

#endif // #ifndef __cmtkProgress_h_included_
