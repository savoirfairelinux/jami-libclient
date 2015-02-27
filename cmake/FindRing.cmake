# Once done this will define
#  ring_INCLUDE_DIRS - include directories
#  ring_BIN - Path to Ring binary

SET(RING_FOUND true)

IF(EXISTS ${CMAKE_INSTALL_PREFIX}/include/dring/dring.h)
   SET(ring_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include/dring)
ELSEIF(EXISTS ${RING_BUILD_DIR}/dring/dring.h)
   SET(ring_INCLUDE_DIRS ${RING_BUILD_DIR}/dring)
ELSE()
   MESSAGE("Daemon header not found!
   Add -DRING_BUILD_DIR or -DCMAKE_INSTALL_PREFIX")
   SET(RING_FOUND false)
ENDIF()

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib;.so;.dll")
FIND_LIBRARY(ring_BIN NAMES ring 
             PATHS ${RING_BUILD_DIR}/.libs 
             PATHS ${CMAKE_INSTALL_PREFIX}/libexec )

MESSAGE("Ring daemon header is in " ${ring_INCLUDE_DIRS})
MESSAGE("Ring library path is " ${ring_BIN})
