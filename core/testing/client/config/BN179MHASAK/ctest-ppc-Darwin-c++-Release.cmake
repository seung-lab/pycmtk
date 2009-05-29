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
SET (TEST_NAME ppc-Darwin-c++-Release)
SET(CTEST_SITE "BN179MHASAK")
SET(CTEST_BUILD_NAME ${TEST_NAME})
SET(DART_TESTING_TIMEOUT 1800)

SET (CTEST_SOURCE_DIRECTORY "/Users/torsten/source/${TEST_NAME}")
SET (CTEST_BINARY_DIRECTORY "/Users/torsten/builds/${TEST_NAME}")
SET(CTEST_UPDATE_COMMAND                "/usr/local/bin/svn")
SET(CTEST_BUILD_CONFIGURATION           "Release")
##SET(CTEST_CHECKOUT_COMMAND  "${CTEST_UPDATE_COMMAND} co https://neuro.sri.com/repos/igs/trunk ${CTEST_SOURCE_DIRECTORY}")
SET(CTEST_CMAKE_GENERATOR "Unix Makefiles")

CTEST_EMPTY_BINARY_DIRECTORY(${CTEST_BINARY_DIRECTORY})

FILE(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
BUILDNAME:STRING=Darwin-c++

BUILD_FUSION:BOOL=OFF
BUILD_GUI:BOOL=OFF
CMAKE_BUILD_TYPE:STRING=Release
CMTK_USE_DCMTK:BOOL=OFF
CMTK_USE_QT:BOOL=OFF
CMTK_USE_OPENMP:BOOL=OFF
CMTK_USE_PTHREADS:BOOL=ON
CMTK_BUILD_NRRD:BOOL=ON
")

CTEST_START(Continuous)
CTEST_UPDATE(SOURCE "${CTEST_SOURCE_DIRECTORY}" RETURN_VALUE res)
IF(${res} GREATER 0)
	CTEST_CONFIGURE(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
	CTEST_BUILD(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
	CTEST_TEST(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
	CTEST_MEMCHECK(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
	CTEST_COVERAGE(BUILD "${CTEST_BINARY_DIRECTORY}" RETURN_VALUE res)
	CTEST_SUBMIT(RETURN_VALUE res)
ENDIF(${res} GREATER 0)
