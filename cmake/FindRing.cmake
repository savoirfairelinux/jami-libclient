# Once done this will define
#  ring_INCLUDE_DIRS - include directories
#  ring_BIN - Path to Ring binary

IF(EXISTS ${RING_BUILD_DIR}/public/ring.h)
   SET(ring_INCLUDE_DIRS ${RING_BUILD_DIR}/public)
ELSEIF(EXISTS ${CMAKE_INSTALL_PREFIX}/include/ring.h)
   SET(ring_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)
ELSE()
   MESSAGE(FATAL_ERROR "Daemon header not found!
   Add -DRING_BUILD_DIR or -DCMAKE_INSTALL_PREFIX")
ENDIF()

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".dylib;.so;.dll")
FIND_LIBRARY(ring_BIN NAMES ring 
             PATHS ${RING_BUILD_DIR}/.libs 
             PATHS ${CMAKE_INSTALL_PREFIX}/libexec )

MESSAGE("Ring daemon header is " ${ring_INCLUDE_DIRS}/ring.h)
MESSAGE("Ring library path is " ${ring_BIN})
