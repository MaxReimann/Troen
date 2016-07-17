# - try to find Scriptzeug library
#
# Cache Variables: (probably not for direct use in your scripts)
#  LIBZEUG_INCLUDE_DIR
#  LIBZEUG_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  LIBZEUG_FOUND
#  LIBZEUG_INCLUDE_DIRS
#  LIBZEUG_LIBRARIES


set(LIBZEUG_INCLUDE_DIR
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/include
)

# RELEASE

  FIND_LIBRARY(LIBZEUG_SCRIPTZEUG_LIBRARY_RELEASE
    NAMES scriptzeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_PROPERTYGUIZEUG_LIBRARY_RELEASE
    NAMES propertyguizeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )


  FIND_LIBRARY(LIBZEUG_STRINGZEUG_LIBRARY_RELEASE
    NAMES stringzeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_IOZEUG_LIBRARY_RELEASE
    NAMES iozeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )


  FIND_LIBRARY(LIBZEUG_WIDGETZEUG_LIBRARY_RELEASE
    NAMES widgetzeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )


  FIND_LIBRARY(LIBZEUG_THREADINGZEUG_LIBRARY_RELEASE
    NAMES threadingzeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_LOGGINGZEUG_LIBRARY_RELEASE
    NAMES loggingzeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_SIGNALZEUG_LIBRARY_RELEASE
    NAMES signalzeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_REFLECTIONZEUG_LIBRARY_RELEASE
    NAMES reflectionzeug
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

# DEBUG

  FIND_LIBRARY(LIBZEUG_SCRIPTZEUG_LIBRARY_DEBUG
    NAMES scriptzeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_PROPERTYGUIZEUG_LIBRARY_DEBUG
    NAMES propertyguizeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_STRINGZEUG_LIBRARY_DEBUG
    NAMES stringzeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_IOZEUG_LIBRARY_DEBUG
    NAMES iozeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )


  FIND_LIBRARY(LIBZEUG_WIDGETZEUG_LIBRARY_DEBUG
    NAMES widgetzeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )


  FIND_LIBRARY(LIBZEUG_THREADINGZEUG_LIBRARY_DEBUG
    NAMES threadingzeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_LOGGINGZEUG_LIBRARY_DEBUG
    NAMES loggingzeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )


  FIND_LIBRARY(LIBZEUG_SIGNALZEUG_LIBRARY_DEBUG
    NAMES signalzeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )

  FIND_LIBRARY(LIBZEUG_REFLECTIONZEUG_LIBRARY_DEBUG
    NAMES reflectionzeugd
    PATHS
      ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/lib
      /usr/local/lib
  )


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Libzeug
	DEFAULT_MSG
	LIBZEUG_SCRIPTZEUG_LIBRARY_RELEASE
  # LIBZEUG_PROPERTYGUIZEUG_LIBRARY_RELEASE
  # LIBZEUG_SIGNALZEUG_LIBRARY_RELEASE
	LIBZEUG_INCLUDE_DIR)

if(LIBZEUG_FOUND)
	set(LIBZEUG_LIBRARIES
    optimized ${LIBZEUG_SCRIPTZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_PROPERTYGUIZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_SIGNALZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_REFLECTIONZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_STRINGZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_IOZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_WIDGETZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_LOGGINGZEUG_LIBRARY_RELEASE}
    optimized ${LIBZEUG_THREADINGZEUG_LIBRARY_RELEASE}
    debug ${LIBZEUG_SCRIPTZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_PROPERTYGUIZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_SIGNALZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_REFLECTIONZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_STRINGZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_IOZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_WIDGETZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_LOGGINGZEUG_LIBRARY_DEBUG}
    debug ${LIBZEUG_THREADINGZEUG_LIBRARY_DEBUG}
  )

  set(LIBZEUG_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/libzeug/include
  )

endif()