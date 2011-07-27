##
##  Copyright 1997-2009 Torsten Rohlfing
##
##  Copyright 2004-2011 SRI International
##
##  This file is part of the Computational Morphometry Toolkit.
##
##  http://www.nitrc.org/projects/cmtk/
##
##  The Computational Morphometry Toolkit is free software: you can
##  redistribute it and/or modify it under the terms of the GNU General Public
##  License as published by the Free Software Foundation, either version 3 of
##  the License, or (at your option) any later version.
##
##  The Computational Morphometry Toolkit is distributed in the hope that it
##  will be useful, but WITHOUT ANY WARRANTY; without even the implied
##  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License along
##  with the Computational Morphometry Toolkit.  If not, see
##  <http://www.gnu.org/licenses/>.
##
##  $Revision: 92 $
##
##  $LastChangedDate: 2009-06-23 09:56:05 -0700 (Tue, 23 Jun 2009) $
##
##  $LastChangedBy: torstenrohlfing $
##

SET (TEST_NAME solstudio-x86_64-Debug)
SET(CTEST_SITE "neuro-test0")
SET(CTEST_BUILD_NAME ${TEST_NAME})
SET(DART_TESTING_TIMEOUT 1800)

SET(CTEST_PROJECT_NAME "CMTK")
SET(CMTK_CTEST_ROOT "/home/testrunner/nitrc/${TEST_NAME}")
SET(CTEST_SOURCE_DIRECTORY "${CMTK_CTEST_ROOT}/core")
SET(CTEST_BINARY_DIRECTORY "${CMTK_CTEST_ROOT}/build")
SET(CTEST_UPDATE_COMMAND                "/usr/bin/svn")
SET(CTEST_CMAKE_GENERATOR "Unix Makefiles")

IF(NOT EXISTS ${CTEST_SOURCE_DIRECTORY})
  SET(CTEST_CHECKOUT_COMMAND  "${CTEST_UPDATE_COMMAND} co https://www.nitrc.org:443/svn/cmtk/trunk/core/  ${CMTK_SOURCE_DIRECTORY}")
ENDIF(NOT EXISTS ${CTEST_SOURCE_DIRECTORY})
CTEST_EMPTY_BINARY_DIRECTORY(${CTEST_BINARY_DIRECTORY})

FILE(WRITE "${CTEST_BINARY_DIRECTORY}/CMakeCache.txt" "
CMAKE_C_COMPILER:FILEPATH=/opt/sun/solstudio12.2/bin/cc
CMAKE_CXX_COMPILER:FILEPATH=/opt/sun/solstudio12.2/bin/CC

BUILDNAME:STRING=Linux-solstudio-x86_64-Debug
BUILD_TESTING:BOOL=ON
CMTK_DATA_ROOT:PATH=/home/testrunner/nitrc/data
CMAKE_BUILD_TYPE:STRING=Debug
CMAKE_CXX_FLAGS:STRING=
CMAKE_CXX_FLAGS_DEBUG:STRING=-g -DDEBUG
CMAKE_C_FLAGS:STRING=

CMTK_BUILD_NRRD:BOOL=ON
CMTK_USE_DCMTK:BOOL=ON
CMTK_USE_SQLITE:BOOL=ON
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
