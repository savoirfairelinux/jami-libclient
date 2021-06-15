# Once done this will define
#  ring_INCLUDE_DIRS - include directories
#  ring_BIN - Path to Ring binary

SET(RING_FOUND true)

IF(EXISTS ${CMAKE_INSTALL_PREFIX}/include/jami/jami.h)
   SET(ring_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include/jami)
ELSEIF(EXISTS ${RING_INCLUDE_DIR}/jami.h)
   SET(ring_INCLUDE_DIRS ${RING_INCLUDE_DIR})
ELSEIF(EXISTS ${RING_BUILD_DIR}/jami/jami.h)
   SET(ring_INCLUDE_DIRS ${RING_BUILD_DIR}/jami)
ELSE()
   MESSAGE(STATUS "Daemon header not found!
   Add -DRING_BUILD_DIR or -DCMAKE_INSTALL_PREFIX")
   SET(RING_FOUND false)
ENDIF()

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib;.so;.dll")

FIND_LIBRARY(ring_BIN NAMES ring
   PATHS ${RING_BUILD_DIR}/.libs
   PATHS ${CMAKE_INSTALL_PREFIX}/lib
   PATHS ${CMAKE_INSTALL_PREFIX}/libexec
   PATHS ${CMAKE_INSTALL_PREFIX}/bin
)

IF(${ring_BIN} MATCHES "")
   FIND_LIBRARY(ring_BIN NAMES jami
      PATHS ${RING_BUILD_DIR}/.libs
      PATHS ${CMAKE_INSTALL_PREFIX}/lib
      PATHS ${CMAKE_INSTALL_PREFIX}/libexec
      PATHS ${CMAKE_INSTALL_PREFIX}/bin
   )
ENDIF()

# Try a static version too
IF(${ring_BIN} MATCHES "")
   SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib")

   FIND_LIBRARY(ring_BIN NAMES ring
      PATHS ${RING_BUILD_DIR}/.libs
      PATHS ${CMAKE_INSTALL_PREFIX}/lib
      PATHS ${CMAKE_INSTALL_PREFIX}/libexec
   )

   IF(${ring_BIN} MATCHES "")
      FIND_LIBRARY(ring_BIN NAMES jami
         PATHS ${RING_BUILD_DIR}/.libs
         PATHS ${CMAKE_INSTALL_PREFIX}/lib
         PATHS ${CMAKE_INSTALL_PREFIX}/libexec
      )
   ENDIF()

IF(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   ADD_DEFINITIONS(-fPIC)
ENDIF()

ENDIF()

MESSAGE(STATUS "Ring daemon header is in " ${ring_INCLUDE_DIRS})
MESSAGE(STATUS "Ring library path is " ${ring_BIN})
