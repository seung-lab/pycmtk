##
##  Copyright 1997-2009 Torsten Rohlfing
##
##  Copyright 2004-2013 SRI International
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

# Sources of non-templated classes.
SET(cmtkSegmentation_SRCS
  cmtkAtlasSegmentation.cxx
  cmtkEntropyMinimizationIntensityCorrectionFunctionalBase.cxx
  cmtkEntropyMinimizationIntensityCorrectionFunctional.cxx
  cmtkLabelCombinationLocalBinaryShapeBasedAveraging.cxx
  cmtkLabelCombinationLocalShapeBasedAveraging.cxx
  cmtkLabelCombinationLocalVoting.cxx
  cmtkLabelCombinationLocalWeighting.cxx
  cmtkLabelCombinationMultiClassSTAPLE.cxx
  cmtkLabelCombinationSTAPLE.cxx
  cmtkLabelCombinationVoting.cxx
  cmtkLabelCombinationShapeBasedAveraging.cxx
  cmtkLabelCombinationShapeBasedAveragingInterpolation.cxx
  cmtkLeastSquaresPolynomialIntensityBiasField.cxx
  cmtkOverlapMeasures.cxx
  cmtkSimpleLevelset.cxx
  cmtkSimpleLevelsetCommandLineBase.cxx
)

SET(cmtkSegmentation_LIBS cmtkSegmentation cmtkRegistration cmtkBase cmtkSystem cmtkNumerics)

IF(CMTK_USE_FFTW_FOUND)
  LIST(APPEND cmtkSegmentation_SRCS 
    cmtkDetectPhantomMagphanEMR051.cxx
    cmtkSphereDetectionBipolarMatchedFilterFFT.cxx
    cmtkSphereDetectionNormalizedBipolarMatchedFilterFFT.cxx)
  LIST(APPEND cmtkSegmentation_LIBS ${CMTK_FFTW_LIBRARIES})
ENDIF(CMTK_USE_FFTW_FOUND)

ADD_LIBRARY(cmtkSegmentation ${cmtkSegmentation_SRCS})
TARGET_LINK_LIBRARIES(${cmtkSegmentation_LIBS})

IF(CMTK_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(cmtkSegmentation PROPERTIES ${CMTK_LIBRARY_PROPERTIES})
ENDIF(CMTK_LIBRARY_PROPERTIES)

INSTALL(TARGETS cmtkSegmentation
  RUNTIME DESTINATION ${CMTK_INSTALL_BIN_DIR} COMPONENT runtime
  LIBRARY DESTINATION ${CMTK_INSTALL_LIB_DIR} COMPONENT runtime
  ARCHIVE DESTINATION ${CMTK_INSTALL_LIB_DIR} COMPONENT libraries)

FILE(GLOB files_h "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
FILE(GLOB files_txx "${CMAKE_CURRENT_SOURCE_DIR}/*.txx")
INSTALL(FILES ${files_h} ${files_txx} DESTINATION ${CMTK_INSTALL_INCLUDE_DIR}/Segmentation COMPONENT headers)
