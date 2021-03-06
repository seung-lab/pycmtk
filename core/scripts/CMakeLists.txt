##
##  Copyright 2009-2012, 2014 SRI International
##
##  Copyright 2012 Torsten Rohlfing
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
##  $Revision$
##
##  $LastChangedDate$
##
##  $LastChangedBy$
##

## Configure scripts for the build tree
SET(CMTK_BINARY_DIR_CONFIG ${CMAKE_BINARY_DIR}/bin)
SET(CMTK_LIBRARY_DIR_CONFIG ${CMAKE_BINARY_DIR}/bin)

SET(SCRIPTS cmtk_functions.sh cmtk_locking.sh cmtk_locking_procmail.sh)
LIST(APPEND SCRIPTS iterative_shape_averaging correct_nex_motion groupwise_reformat)
LIST(APPEND SCRIPTS correct_dwi_distortion correct_dwi_distortion_and_motion)

FOREACH(s ${SCRIPTS})
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${s}.in ${CMAKE_BINARY_DIR}/bin/${s} @ONLY)
ENDFOREACH(s ${SCRIPTS})

IF(CMTK_BUILD_WRAPPER)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmtk.in ${CMAKE_BINARY_DIR}/bin/cmtk @ONLY)
ENDIF(CMTK_BUILD_WRAPPER)

## Configure SCRIPTS for the install tree
# Link directories.
IF(CMTK_BUILD_WRAPPER)
  SET(CMTK_BINARY_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/${CMTK_INSTALL_LIB_DIR}/bin)
ELSE(CMTK_BUILD_WRAPPER)
  SET(CMTK_BINARY_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/${CMTK_INSTALL_BIN_DIR})
ENDIF(CMTK_BUILD_WRAPPER)

SET(CMTK_LIBRARY_DIR_CONFIG ${CMAKE_INSTALL_PREFIX}/${CMTK_INSTALL_LIB_DIR})

FOREACH(s ${SCRIPTS})
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${s}.in ${CMAKE_BINARY_DIR}/Install/${s} @ONLY)
  IF(CMTK_BUILD_WRAPPER)
    INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/Install/${s} DESTINATION ${CMTK_INSTALL_LIB_DIR}/bin COMPONENT tools)
  ELSE(CMTK_BUILD_WRAPPER)
    INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/Install/${s} DESTINATION ${CMTK_INSTALL_BIN_DIR} COMPONENT tools)
  ENDIF(CMTK_BUILD_WRAPPER)
ENDFOREACH(s ${SCRIPTS})

IF(CMTK_BUILD_WRAPPER)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/cmtk.in ${CMAKE_BINARY_DIR}/Install/cmtk @ONLY)
  INSTALL(PROGRAMS ${CMAKE_BINARY_DIR}/Install/cmtk DESTINATION ${CMTK_INSTALL_BIN_DIR} COMPONENT tools)
ENDIF(CMTK_BUILD_WRAPPER)

