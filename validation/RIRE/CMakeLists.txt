##
##  Copyright 2009-2011 SRI International
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

SET(CMTK_LIBRARIES "cmtkSegmentation;cmtkRegistration;cmtkIO;cmtkBase;cmtkNumerics;cmtkSystem")

SET(APPS register_rire register_rire_experimental register_rire_experimental_cubic)

FOREACH(APPLICATION ${APPS})
  ADD_EXECUTABLE(${APPLICATION} ${CMTK_LIB_INIT_SRCS} ${APPLICATION}.cxx)
  TARGET_LINK_LIBRARIES(${APPLICATION} ${CMTK_LIBRARIES} ${CMTK_SYSTEM_LIBRARIES})
  INSTALL(TARGETS ${APPLICATION} RUNTIME DESTINATION bin COMPONENT tools)
ENDFOREACH(APPLICATION ${APPS})
