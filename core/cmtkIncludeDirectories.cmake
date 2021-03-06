##
##  Copyright 1997-2009 Torsten Rohlfing
##
##  Copyright 2004-2010 SRI International
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

#-----------------------------------------------------------------------------
# Include directories for other projects installed on the system.
SET(CMTK_INCLUDE_DIRS_SYSTEM 
  ${QT_INCLUDE_DIR} 
  ${QT_INCLUDE_PATH}
  )

#-----------------------------------------------------------------------------
# Include directories from the build tree.
SET(CMTK_INCLUDE_DIRS_BUILD_TREE ${CMTK_BINARY_DIR})

# These directories are always needed.
SET(CMTK_INCLUDE_DIRS_BUILD_TREE ${CMTK_INCLUDE_DIRS_BUILD_TREE}
  ${DCMTK_INCLUDE_DIRS}
  ${NRRD_INCLUDE_DIRS}
  ${MXML_INCLUDE_DIR}
  ${SQLITE_INCLUDE_DIR}
  ${ZCONF_INCLUDE_DIR}
  ${ZLIB_INCLUDE_DIR}
  ${CMTK_SOURCE_DIR}/libs
  )
