/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2004-2014 SRI International
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

#ifndef __cmtkStrUtility_h_included_
#define __cmtkStrUtility_h_included_

#include <cmtkconfig.h>

#include <map>
#include <string>
#include <vector>

namespace
cmtk
{

/** \addtogroup System */
//@{

/** Safe search for a string within a non-null-terminated string.
 */
const char*
StrNStr( const char* haystack, const size_t nBytes, const char* needle );

/** Safe string comparison.
 * This function is a wrapper for the standard library's "strcmp" function.
 * This wrapper is safe in that it accepts any combination of NULL and non-NULL
 * pointers to both arguments.
 *\return 0 if the strings pointed to by "s1" and "s2" are equal, a value
 * greater than zero if s1 is greater than s2, and a value smaller than zero
 * if s1 is smaller than s2. NULL pointers are considered smaller than any
 * other string except the NULL pointer itself.
 */
int StrCmp( const char *s1, const char* s2 );

/** Replace string components.
 *\todo This is highly unsafe, since we're not checking for infinite loops and
 * the likes. We should really work on this sometimes... or be REALLY carefull.
 */
std::string StrReplaceByRules( const std::string& str, const std::map<std::string,std::string>& rules, const bool multiple = false );

/// Replace a search string with a replacement string.
std::string StrReplace( const std::string& str /*!< The string to replace in */, const std::string& search /*!< Substring to replace */, const std::string& replace /*!< String to replace search string with */ );
  
/// Make a string legal in a path by replacing spaces and colons with "_".
std::string StrMakeLegalInPath( const std::string& s );

/// Split a string into a vector of strings.
std::vector<std::string> StrSplit( const std::string& s /*<! The string to split. */, const std::string separators = "," /*!< String of separator characters. Any of these will separate two parts in the input string. */ );

//@}

} // namespace cmtk

#endif // #ifndef __cmtkStrUtility_h_included_
