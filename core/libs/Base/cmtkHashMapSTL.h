/*
//
//  Copyright 1997-2009 Torsten Rohlfing
//
//  Copyright 2009, 2012 SRI International
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

#ifndef __cmtkHashMapSTL_h_included_
#define __cmtkHashMapSTL_h_included_

#include <cmtkconfig.h>

#define HAVE_STL_HASH_MAP

#if defined(HAVE_UNORDERED_MAP)
#  include <unordered_map>
#elif defined(HAVE_HASH_MAP_H)
#  include <hash_map.h>
#elif defined(HAVE_HASH_MAP)
#  include <hash_map>
#else
#  undef HAVE_STL_HASH_MAP
#endif

#ifdef HAVE_STL_HASH_MAP
namespace
cmtk
{

/// Generic hash function for all integer types.
template<typename TKey>
struct HashFunctionInteger
{
  /// Simply cast integer key to size_t
  size_t operator()( const TKey x ) const
  { 
    return static_cast<size_t>( x ); 
  }
};

/** Wrapper class for STL hash_map or unordered_map classes.
 * This class will use whatever hash map implementation is provided by the
 * currently used STL. If both unordered_map and hash_map are provided, the
 * former is used as it is the future standard class.
 */
template<
  class TKey, 
  class TValue, 
  class THashFunc = HashFunctionInteger<TKey>
  >
class HashMapSTL : 
    /// Inherit STL hash/unordered map.
#if defined(_MSC_VER)
    public std::tr1::unordered_map<TKey,TValue,THashFunc>
#elif defined(HAVE_UNORDERED_MAP)
    public std::unordered_map<TKey,TValue,THashFunc>
#elif defined(__GNUC__)
    public __gnu_cxx::hash_map<TKey,TValue,THashFunc>
#else
    public std::hash_map<TKey,TValue,THashFunc>
#endif
{
};

} // namespace cmtk

#endif // #ifdef HAVE_STL_HASH_MAP

#endif // #ifndef __cmtkHashMapSTL_h_included_
