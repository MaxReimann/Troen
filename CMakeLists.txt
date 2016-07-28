cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(troen)

#################################################
# PROJECT DESCRIPTION
#################################################

set(META_PROJECT_NAME        "Troen")
set(META_VERSION_MAJOR       "0")
set(META_VERSION_MINOR       "2")
set(META_VERSION_PATCH       "0")
set(META_VERSION             "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
set(META_AUTHOR_ORGANIZATION "GameProgramming")
set(META_AUTHOR_DOMAIN       "https://github.com/philippotto/GP2013")

#################################################
# OPTIONS
#################################################

option(OPTION_LIMIT_CONFIGS  "Generate limited configs (Release; Debug)" ON)
option(OPTION_LOCAL_INSTALL  "Install to a local directory instead of the system" OFF)
option(OPTION_ERRORS_AS_EXCEPTION "Throw exceptions" OFF)
option(OPTION_PORTABLE_INSTALL    "Install to a local directory instead of the system" ON)


#################################################
# CMAKE CONFIGURATION
#################################################

set(TROEN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Include cmake modules from ./cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find the WinSDK libs
if (WIN32)
	set(CMAKE_PREFIX_PATH "C:\\Program Files (x86)\\Windows Kits\\8.0\\Lib\\win8\\um\\x64" "C:\\Program Files (x86)\\Windows Kits\\8.1\\Lib\\winv6.3\\um\\x64")
	cmake_policy(SET CMP0020 NEW)
endif (WIN32)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set configuration types
# if(OPTION_LIMIT_CONFIGS)
# 	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited Configs" FORCE)
# endif()

# Project
project(${META_PROJECT_NAME} C CXX)

# Generate folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Include custom cmake functions
include(cmake/Custom.cmake)
include(cmake/GitRevision.cmake)

#just for temporary convenience
set(CMAKE_INSTALL_PREFIX /local/Troen/bin)

#################################################
# PLATFORM AND ARCHITECTURE
#################################################

# Architecture (32/64 bit)
set(X64 OFF)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(X64 ON)
endif()

# Check for linux
if(UNIX AND NOT APPLE)
	set(LINUX 1)
endif()

# Setup platform specifics (compile flags, etc., ...)
if(MSVC)
	message(STATUS "Configuring for platform Windows/MSVC.")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformWindowsMSVC.cmake)
elseif(WIN32 AND CMAKE_COMPILER_IS_GNUCXX)
	message(STATUS "Configuring for platform Windows/GCC.")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformWindowsGCC.cmake)
elseif(LINUX AND CMAKE_COMPILER_IS_GNUCXX)
	message(STATUS "Configuring for platform Linux/GCC.")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformLinuxGCC.cmake)
elseif(APPLE)
	message(STATUS "Configuring for platform MacOS.")
	include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PlatformMacOS.cmake)
else()
	# Unsupported system/compiler
	message(WARNING "Unsupported platform/compiler combination")
endif()


#################################################
# DEPENDENCIES
#################################################

# Qt5 - taken from cg2sandbox
set(CMAKE_AUTOMOC ON)
set(AUTOMOC_MOC_OPTIONS PROPERTIES FOLDER CMakeAutomocTargets)
# Probably works in the next cmake release -> http://www.cmake.org/Bug/view.php?id=13788
# What we do not want is automocs beside the project -> http://www.cmake.org/Bug/view.php?id=13688
set_property(GLOBAL PROPERTY AUTOMOC_FOLDER CMakeAutomocTargets)
# try to find either qt4 or qt5, favoring qt5
# good resource: http://www.kdab.com/using-cmake-with-qt-5/
# http://qt-project.org/forums/viewthread/30006/
if (MSVC)
	cmake_policy(SET CMP0020 NEW)
endif()

set(QT_PACKAGES
	Qt5Core
	Qt5Gui
	Qt5Widgets
)
foreach(package ${QT_PACKAGES})
	find_package(${package} 5.1 REQUIRED)
endforeach(package)



# OpenGL
find_package(OpenGL REQUIRED)




#set(OMEGA_BINARY_DIR /local/omegalib/build/bin )
set(Omegalib_DIR /local/omegalib/build)
set(OmegaLib_SOURCE_DIR /local/omegalib/)
find_package(Omegalib)

# # OpenSceneGraph
# set(OSG_DIR ${Omegalib_DIR}/3rdparty/osg/install)
# find_package(OpenSceneGraph REQUIRED)


# # Bullet
set(BULLET_ROOT "${Omegalib_DIR}/3rdparty/bullet/install")
# set(BULLET_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/bullet/install")
message(STATUS "bullet root: ${BULLET_ROOT}")
find_package(Bullet REQUIRED)

# MESSAGE(STATUS "osg bullet inc: ${OSGBULLET_INCLUDES}")
# MESSAGE(STATUS "bullet libs: ${BULLET_LIBS}, ${OSGBULLET_LIBS}")

# FMOD
set(FMOD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/fmod/)
find_package(FMOD REQUIRED)

# HIDAPI
# find_package(HIDAPI REQUIRED)

#RakNet
set(RAKNET_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/RakNet)
find_package(RakNet REQUIRED)

# V8
# find_package(V8 REQUIRED)

# Scriptzeug
find_package(Libzeug REQUIRED)

find_package(PythonLibs 2.7)
if(PYTHONLIBS_FOUND)
	include(${OmegaLib_SOURCE_DIR}/external/UsePython.cmake)
endif()

set(CYCLOPS_INCLUDE "${OmegaLib_SOURCE_DIR}/modules/cyclops")

#this is set to use from withiin the code to dynamically register the correct libraries
add_definitions(-DOMEGALIB_BIN_DIR="${Omegalib_DIR}/bin")


#################################################
# SOURCES
#################################################

set(sources
	source/main.cpp
	source/forwarddeclarations.h
	source/mainwindow.h
	source/mainwindow.cpp
	source/troengame.h
	source/troengame.cpp
	source/troengamebuilder.h
	source/troengamebuilder.cpp
	source/sampleosgviewer.h
	source/sampleosgviewer.cpp
	source/gameeventhandler.h
	source/gameeventhandler.cpp
	source/globals.h
	source/constants.h
	source/constants.cpp
	source/gamelogic.h
	source/gamelogic.cpp
	source/player.h
	source/player.cpp
	source/interpolate.h
	source/omegascene.h
	source/omegascene.cpp
	source/gamethread.h
	source/gamethread.cpp

	source/resourcepool.h
	source/resourcepool.cpp

	source/input/keyboard.h
	source/input/keyboard.cpp
	source/input/gamepad.h
	source/input/gamepad.cpp
	source/input/gamepadps4.h
	source/input/gamepadps4.cpp
	source/input/bikeinputstate.h
	source/input/bikeinputstate.cpp
	source/input/ai.h
	source/input/ai.cpp
	source/input/pollingdevice.h
	source/input/pollingdevice.cpp

	source/sound/audiomanager.h
	source/sound/audiomanager.cpp

	source/model/bikemodel.h
	source/model/bikemodel.cpp
	source/model/levelmodel.h
	source/model/levelmodel.cpp
	source/model/fencemodel.h
	source/model/fencemodel.cpp
	source/model/abstractmodel.h
	source/model/abstractmodel.cpp
	source/model/itemmodel.h
	source/model/itemmodel.cpp
	source/model/physicsworld.h
	source/model/physicsworld.cpp
	source/model/bikemotionstate.h
	source/model/objectinfo.h

	source/controller/bikecontroller.h
	source/controller/bikecontroller.cpp
	source/controller/levelcontroller.h
	source/controller/levelcontroller.cpp
	source/controller/fencecontroller.h
	source/controller/fencecontroller.cpp
	source/controller/abstractcontroller.h
	source/controller/abstractcontroller.cpp
	source/controller/hudcontroller.h
	source/controller/hudcontroller.cpp
	source/controller/itemcontroller.h
	source/controller/itemcontroller.cpp


	source/view/bikeview.h
	source/view/bikeview.cpp
	source/view/levelview.h
	source/view/levelview.cpp
	source/view/fenceview.h
	source/view/fenceview.cpp
	source/view/abstractview.h
	source/view/abstractview.cpp
	source/view/shaders.h
	source/view/shaders.cpp
	source/view/hudview.h
	source/view/hudview.cpp
	source/view/itemview.h
	source/view/itemview.cpp
	source/view/skydome.h
	source/view/skydome.cpp
	source/view/postprocessing.cpp
	source/view/postprocessing.h
	source/view/nearfarcallback.h
	source/view/timeupdate.h
	source/view/nodefollowcameramanipulator.h
	source/view/nodefollowcameramanipulator.cpp
	source/view/playermarker.h
	source/view/playermarker.cpp
	source/view/easemotion.h
	source/view/reflection.h
	source/view/reflection.cpp

	source/util/chronotimer.h
	source/util/chronotimer.cpp
	source/util/gldebugdrawer.h
	source/util/gldebugdrawer.cpp
	source/util/scriptwatcher.h
	source/util/filteredrayresultcallback.h


	source/network/networkmanager.h
	source/network/networkmanager.cpp
	source/network/clientmanager.cpp
	source/network/clientmanager.h
	source/network/servermanager.cpp
	source/network/servermanager.h
	source/input/remoteplayer.cpp
	source/input/remoteplayer.h


	BendedViews/src/DeformationRendering.cpp
	BendedViews/src/DeformationRendering.h
	BendedViews/src/GlobalDeformationsExample.cpp
	BendedViews/src/GlobalDeformationsExample.h
	BendedViews/src/PreRenderCallback.cpp
	BendedViews/src/PreRenderCallback.h
	BendedViews/src/SplineDeformationRendering.cpp
	BendedViews/src/SplineDeformationRendering.h

	BendedViews/src/deformentities/bsplinedeformationfunction.cpp
	BendedViews/src/deformentities/bsplinedeformationfunction.h
	BendedViews/src/deformentities/curve.cpp
	BendedViews/src/deformentities/curve.h
	BendedViews/src/deformentities/curvestylingpreset.cpp
	BendedViews/src/deformentities/curvestylingpreset.h
	BendedViews/src/deformentities/preset.cpp
	BendedViews/src/deformentities/preset.h
	BendedViews/src/deformentities/tagpoint.cpp
	BendedViews/src/deformentities/tagpoint.h

	##################################################
	# Bended Views GUI
	##################################################
	BendedViews/src/ui/DeformationConfigWidget.ui
	BendedViews/src/ui/DeformationConfigWidget.h
	BendedViews/src/ui/DeformationConfigWidget.cpp
	BendedViews/src/ui/ControlPointItem.h
	BendedViews/src/ui/ControlPointItem.cpp
	BendedViews/src/ui/SplineItem.h
	BendedViews/src/ui/SplineItem.cpp
	BendedViews/src/ui/SplineSceneItem.h
	BendedViews/src/ui/SplineSceneItem.cpp

)


source_group_by_path("${CMAKE_CURRENT_SOURCE_DIR}/source"
	"\\\\.h$|\\\\.hpp$|\\\\.cpp$|\\\\.c$|\\\\.ui$|\\\\.qrc$" "Source Files" ${sources})

#################################################
# TARGET
#################################################

# Set target name
set(target ${META_PROJECT_NAME})

add_executable(${target} ${sources})

qt5_use_modules(${target} Core Gui Widgets)
qt5_use_modules(${target} Core OpenGL Gui Widgets)
qt5_wrap_ui(${target} BendedViews/src/ui/DeformationConfigWidget.ui)

include_directories(
	${CMAKE_SOURCE_DIR}
	# ${OPENSCENEGRAPH_INCLUDE_DIRS}
	${BULLET_INCLUDE_DIRS}
	${OMEGA_INCLUDE_DIRS}
	${BULLET_INCLUDES} #from omega
	${OMEGA_TOOLKIT_INCLUDE_DIRS}
	${OMEGA_OSG_INCLUDE_DIRS}
	${OSG_INCLUDES}
	${FMOD_INCLUDE_DIRS}
	# ${HIDAPI_INCLUDE_DIRS}
	${RakNet_INCLUDES}
	${LIBZEUG_INCLUDE_DIRS}
	${PROJECT_BINARY_DIR} #for the generated files
	${PYTHON_INCLUDES}
	# ${CYCLOPS_INCLUDE}

	# ${Omegalib_DIR}/3rdparty/osg/install/include
)


MESSAGE(STATUS ${OMEGA_INCLUDE_DIRS})
MESSAGE(STATUS "libs:")
MESSAGE(STATUS ${OMEGA_LIB})
MESSAGE(STATUS "omegosg libs:")
MESSAGE(STATUS ${OMEGA_OSG_LIB})

target_link_libraries( ${target}
    ${OPENGL_LIBRARIES}
    # ${OPENSCENEGRAPH_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${OMEGA_LIB}
    # ${BULLET_LIBS}
	${OMEGA_OSG_LIB}
    ${FMOD_LIBRARIES}
	# ${HIDAPI_LIBRARIES}
	${RakNet_LIBRARY}
	${OMEGA_TOOLKIT_LIB}
	${LIBZEUG_LIBRARIES}
	${PYTHON_LIBRARIES}
	# ${CYCLOPS_LIB}	

)
# message(STATUS ${CYCLOPS_LIB})
# message(STATUS ${OMEGA_LIB_DEPENDS})

if (WIN32)
target_link_libraries( ${target}
	${CMAKE_PREFIX_PATH}/Xinput9_1_0.lib
)
endif (WIN32)

#################################################
# INSTALL
#################################################


# Installation paths
set(project ${META_PROJECT_NAME})
if(WIN32) 
	set(INSTALL_ROOT      ".")                         # C:\Programme\<project>
    set(INSTALL_EXAMPLES  "bin")                       # C:\Programme\<project>
    set(INSTALL_DATA      ".")	                       # C:\Programme\<project>
    set(INSTALL_BIN       "bin")                       # C:\Programme\<project>
    set(INSTALL_SHARED    ".")                         # C:\Programme\<project>
    set(INSTALL_LIB       "3rdParty")                  # C:\Programme\<project>\lib
    set(INSTALL_INCLUDE   "include")                   # C:\Programme\<project>\include
    set(INSTALL_DOC       "doc")                       # C:\Programme\<project>\doc
    set(INSTALL_SHORTCUTS ".")                         # Not available under Windows
    set(INSTALL_ICONS     ".")                         # Not available under Windows
    set(INSTALL_INIT      ".")                         # Not available under Windows
else()
    set(INSTALL_ROOT      "share/${project}")          # /usr/[local]/share/<project>
    set(INSTALL_EXAMPLES  "share/${project}/examples") # /usr/[local]/share/<project>/examples
    set(INSTALL_DATA      "share/${project}/data") # /usr/[local]/share/<project>/data
    set(INSTALL_BIN       "bin")                       # /usr/[local]/bin
    set(INSTALL_SHARED    "lib")                       # /usr/[local]/lib
    set(INSTALL_LIB       "lib")                       # /usr/[local]/lib
    set(INSTALL_INCLUDE   "include")                   # /usr/[local]/include
    set(INSTALL_DOC       "share/doc/${project}")      # /usr/[local]/share/doc/<project>
    set(INSTALL_SHORTCUTS "share/applications")        # /usr/[local]/share/applications
    set(INSTALL_ICONS     "share/pixmaps")             # /usr/[local]/share/pixmaps
    set(INSTALL_INIT      "/etc/init")                 # /etc/init (upstart init scripts)

    # Adjust target paths for portable installs
    if(OPTION_PORTABLE_INSTALL)
        # Put binaries in root directory and keep data directory name
        set(INSTALL_ROOT     ".") # <INSTALL_PREFIX>/
        set(INSTALL_EXAMPLES "examples") # <INSTALL_PREFIX>/
        set(INSTALL_DATA     "data") # <INSTALL_PREFIX>/
        set(INSTALL_BIN      "bin") # <INSTALL_PREFIX>/

        # We have to change the RPATH of binaries to achieve a usable local install.
        # [TODO] For binaries, "$ORIGIN/lib" is right, so that libraries are found in ./lib.
        #        However, I have not yet tested what happens when libraries use other libraries.
        #        In that case, they might need the rpath $ORIGIN instead ...
        set(CMAKE_SKIP_BUILD_RPATH            FALSE)         # Use automatic rpath for build
        set(CMAKE_BUILD_WITH_INSTALL_RPATH    FALSE)         # Use specific rpath for INSTALL
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)         # NO automatic rpath for INSTALL
        set(CMAKE_INSTALL_RPATH               "$ORIGIN/3rdParty") # Libraries are relative to binary
    endif()
endif()

if (MSVC)
	set(LIBENDING dll)
elseif(LINUX)
	set(LIBENDING so)
endif()

#
# Global deployment
#

# Project meta files
# install(FILES glow-config.cmake DESTINATION ${INSTALL_ROOT})
install(FILES AUTHORS DESTINATION ${INSTALL_ROOT} COMPONENT ${META_PROJECT_NAME})
# install(FILES LICENSE           DESTINATION ${INSTALL_ROOT})

# Add a revision file containing the git-head tag for cpack and install
create_revision_file(${CMAKE_BINARY_DIR}/revision ${INSTALL_ROOT})
# data
# install(
# 	DIRECTORY ${CMAKE_SOURCE_DIR}/data
# 	DESTINATION ${INSTALL_DATA}
# 	COMPONENT ${META_PROJECT_NAME})
# shaders
install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
	DESTINATION ${INSTALL_ROOT}
	COMPONENT ${META_PROJECT_NAME})
# bended views
install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/BendedViews/shaders
	DESTINATION ${INSTALL_ROOT}/BendedViews
	COMPONENT ${META_PROJECT_NAME})
#scripts
install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/scripts
	DESTINATION ${INSTALL_ROOT}
	COMPONENT ${META_PROJECT_NAME})

file( GLOB installlibs ${CMAKE_SOURCE_DIR}/3rdParty/RakNet/Lib/*.${LIBENDING}
   ${CMAKE_SOURCE_DIR}/3rdParty/fmod/api/lib/*.${LIBENDING}
   ${CMAKE_SOURCE_DIR}/3rdParty/fmod/fmoddesignerapi/api/lib/*.${LIBENDING}
   ${CMAKE_SOURCE_DIR}/3rdParty/libzeug/lib/*.${LIBENDING}*
	)

# # hidapi, fmod, raknet
install(
	FILES ${installlibs}
    DESTINATION ${INSTALL_ROOT}
	COMPONENT ${META_PROJECT_NAME}
)

# Qt5
get_target_property(QtCore_location Qt5::Core LOCATION)
get_target_property(QtGui_location Qt5::Gui LOCATION)
get_target_property(QtWidgets_location Qt5::Widgets LOCATION)

install(
	FILES
		${QtCore_location}
		${QtGui_location}
		${QtWidgets_location}
	DESTINATION ${INSTALL_ROOT}
	COMPONENT ${META_PROJECT_NAME})

# # OSG
# string(REGEX REPLACE "/include" "/bin" OPENSCENEGRAPH_DLL_DIR ${OPENSCENEGRAPH_INCLUDE_DIRS})
# #file(GLOB osgDLLs RELATIVE "${OPENSCENEGRAPH_DLL_DIR}" "*.dll")

# install(
# 	FILES
# 		${OSG_LIBS}
# 	DESTINATION ${INSTALL_ROOT}
# 	COMPONENT ${META_PROJECT_NAME})

# install(
# 	DIRECTORY ${OMEGA_BINARY_DIR}/3rdParty/osg/lib
# 	DESTINATION ${INSTALL_ROOT}
# 	COMPONENT ${META_PROJECT_NAME})

# executable
INSTALL(
	TARGETS ${META_PROJECT_NAME}
	DESTINATION ${INSTALL_ROOT}
	COMPONENT ${META_PROJECT_NAME})


#################################################
# CPACK DEPLOY
#################################################

set(CPACK_GENERATOR "NSIS")
set(CPACK_SOURCE_GENERATOR "ZIP")

set(CPACK_PACKAGE_VERSION_MAJOR ${META_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${META_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${GIT_REV})

set(CPACK_INSTALL_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
set(CPACK_IGNORE_FILES "/.pdb/;/.ilk/;/.svn/;/.hg/;/.git/;.swp$;.#;/#")
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_BINARY_DIR}/install)

SET(CPACK_NSIS_MODIFY_PATH OFF)
SET(CPACK_NSIS_DISPLAY_NAME "${META_PROJECT_NAME}.${META_VERSION}")
SET(CPACK_NSIS_HELP_LINK "https://github.com/philippotto/GP2013")
set(CPACK_NSIS_MENU_LINKS "https://github.com/philippotto/GP2013" "Troen Repository")
set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/philippotto/GP2013")
#set(CPACK_NSIS_MUI_FINISHPAGE_RUN ${META_PROJECT_NAME})
set(CPACK_NSIS_PACKAGE_NAME ${META_PROJECT_NAME})

include(CPack)
