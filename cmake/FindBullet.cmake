# - Try to find the Bullet physics engine
#
#  This module defines the following variables
#
#  BULLET_FOUND - Was bullet found
#  BULLET_INCLUDE_DIRS - the Bullet include directories
#  BULLET_LIBRARIES - Link to this, by default it includes
#                     all bullet components (Dynamics,
#                     Collision, LinearMath, & SoftBody)
#
#  This module accepts the following variables
#
#  BULLET_ROOT - Can be set to bullet install path or Windows build path
#

# Copyright (c) 2009, Philip Lowman <philip at yhbt.com>
#
# Redistribution AND use is allowed according to the terms of the New
# BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(!BULLET_ROOT)
  set(BULLET_ROOT $ENV{BULLET_ROOT})
endif(!BULLET_ROOT)

macro(_FIND_BULLET_LIBRARY _var)
  find_library(${_var}
     NAMES
        ${ARGN}
     PATHS
        ${BULLET_ROOT}
                ${BULLET_ROOT}/lib/Debug
                ${BULLET_ROOT}/lib/Release
        ${BULLET_ROOT}/out/release8/libs
        ${BULLET_ROOT}/out/debug8/libs
     PATH_SUFFIXES lib
  )
  mark_as_advanced(${_var})
endmacro()

macro(_BULLET_APPEND_LIBRARIES _list _release)
   set(_debug ${_release}_DEBUG)
   if(${_debug})
      set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
   else()
      set(${_list} ${${_list}} ${${_release}})
   endif()
endmacro()

find_path(BULLET_INCLUDE_DIR NAMES btBulletCollisionCommon.h
  PATHS
    ${BULLET_ROOT}/include
    ${BULLET_ROOT}/src
  PATH_SUFFIXES bullet
)

# Find the libraries

_FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIBRARY         BulletDynamics)
_FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIBRARY_DEBUG   BulletDynamics_Debug BulletDynamics_d)
_FIND_BULLET_LIBRARY(BULLET_COLLISION_LIBRARY        BulletCollision)
_FIND_BULLET_LIBRARY(BULLET_COLLISION_LIBRARY_DEBUG  BulletCollision_Debug BulletCollision_d)
_FIND_BULLET_LIBRARY(BULLET_MATH_LIBRARY             BulletMath LinearMath)
_FIND_BULLET_LIBRARY(BULLET_MATH_LIBRARY_DEBUG       BulletMath_Debug BulletMath_d LinearMath_debug LinearMath_d)
_FIND_BULLET_LIBRARY(BULLET_SOFTBODY_LIBRARY         BulletSoftBody)
_FIND_BULLET_LIBRARY(BULLET_SOFTBODY_LIBRARY_DEBUG   BulletSoftBody_Debug BulletSoftBody_d)

_FIND_BULLET_LIBRARY(BULLET_FILELOADER_LIBRARY       BulletFileLoader)
_FIND_BULLET_LIBRARY(BULLET_FILELOADER_LIBRARY_DEBUG BulletFileLoader_Debug BulletFileLoader_d)
# _FIND_BULLET_LIBRARY(BULLET_MULTITHREADED_LIBRARY       BulletMultiThreaded)
# # _FIND_BULLET_LIBRARY(BULLET_MULTITHREADED_LIBRARY_DEBUG BulletMultiThreaded_Debug BulletMultiThreaded_d)
# _FIND_BULLET_LIBRARY(BULLET_SOFTBODYSOLVERS_LIBRARY       BulletSoftBodySolvers_OpenCL_Mini)
# _FIND_BULLET_LIBRARY(BULLET_SOFTBODYSOLVERS_LIBRARY_DEBUG BulletSoftBodySolvers_OpenCL_Mini_Debug BulletSoftBodySolvers_OpenCL_Mini_d)
_FIND_BULLET_LIBRARY(BULLET_WORLDIMPORTER_LIBRARY       BulletWorldImporter)
_FIND_BULLET_LIBRARY(BULLET_WORLDIMPORTER_LIBRARY_DEBUG BulletWorldImporter_Debug BulletWorldImporter_d)
_FIND_BULLET_LIBRARY(BULLET_XMLWORLDIMPORTER_LIBRARY       BulletXmlWorldImporter)
_FIND_BULLET_LIBRARY(BULLET_XMLWORLDIMPORTER_LIBRARY_DEBUG BulletXmlWorldImporter_Debug BulletXmlWorldImporter_d)

_FIND_BULLET_LIBRARY(BULLET_CONVEXDECOMPOSITION_LIBRARY       ConvexDecomposition)
_FIND_BULLET_LIBRARY(BULLET_CONVEXDECOMPOSITION_LIBRARY_DEBUG ConvexDecomposition_Debug ConvexDecomposition_d)

_FIND_BULLET_LIBRARY(BULLET_GIMPACTUTILS_LIBRARY       GIMPACTUtils)
_FIND_BULLET_LIBRARY(BULLET_GIMPACTUTILS_LIBRARY_DEBUG GIMPACTUtils_Debug GIMPACTUtils_d)




# handle the QUIETLY and REQUIRED arguments and set BULLET_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Bullet DEFAULT_MSG
    BULLET_DYNAMICS_LIBRARY BULLET_COLLISION_LIBRARY BULLET_MATH_LIBRARY
    BULLET_SOFTBODY_LIBRARY BULLET_FILELOADER_LIBRARY #BULLET_MULTITHREADED_LIBRARY
    BULLET_WORLDIMPORTER_LIBRARY
    BULLET_XMLWORLDIMPORTER_LIBRARY BULLET_CONVEXDECOMPOSITION_LIBRARY
    BULLET_GIMPACTUTILS_LIBRARY BULLET_INCLUDE_DIR)

set(BULLET_INCLUDE_DIRS ${BULLET_INCLUDE_DIR})
if(BULLET_FOUND)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_DYNAMICS_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_COLLISION_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_MATH_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_SOFTBODY_LIBRARY)

   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_FILELOADER_LIBRARY)
   #_BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_MULTITHREADED_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_WORLDIMPORTER_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_XMLWORLDIMPORTER_LIBRARY)

   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_CONVEXDECOMPOSITION_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_GIMPACTUTILS_LIBRARY)


endif()