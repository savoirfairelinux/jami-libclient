cmake_minimum_required (VERSION 2.8)

if (POLICY CMP0022)
   cmake_policy (SET CMP0022 NEW)
endif (POLICY CMP0022)

project (qtwrapper)

message ("Compiling with qtwrapper")

set (ENABLE_LIBWRAP true)
add_definitions (-DENABLE_LIBWRAP=true)

set (LOCAL_CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../cmake/)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LOCAL_CMAKE_MODULE_PATH})

if (${MUTE_DRING} MATCHES true)
   # It makes clients debugging easier
   message ("Dring logs are disabled")
   add_definitions (-DMUTE_DRING=true)
endif ()

find_package (Qt5Core REQUIRED)
find_package (Ring REQUIRED)

set (libqtwrapper_LIB_SRCS instancemanager.cpp videomanager_wrap.cpp)

if (NOT (${ENABLE_VIDEO} MATCHES "false"))
   message ("VIDEO enabled")
   add_definitions (-DENABLE_VIDEO=true)
   set (
      ENABLE_VIDEO
      1
      CACHE STRING "Enable video")
endif (NOT (${ENABLE_VIDEO} MATCHES "false"))

if (NOT (${ENABLE_PLUGIN} MATCHES "false"))
   message ("PLUGIN enabled")
   add_definitions (-DENABLE_PLUGIN=true)
   set (
      ENABLE_PLUGIN
      1
      CACHE STRING "Enable plugin")
endif (NOT (${ENABLE_PLUGIN} MATCHES "false"))

if (ENABLE_PLUGIN)
   message ("Adding pluginmanager.cpp")
   set (plugin_SRC pluginmanager.cpp)
else ()
   message ("Adding pluginmanagerMock.cpp")
   set (plugin_SRC pluginmanagerMock.cpp)
endif ()

include_directories (SYSTEM ${Qt5Core_INCLUDE_DIRS})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${ring_INCLUDE_DIRS})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../dbus)

add_library (qtwrapper STATIC ${libqtwrapper_LIB_SRCS} ${plugin_SRC})

if (NOT ${ring_BIN} MATCHES "ring_BIN-NOTFOUND")
   target_link_libraries (qtwrapper ${QT_QTCORE_LIBRARY} ${ring_BIN})
endif ()

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
   add_definitions ("-std=c++0x")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
   add_definitions (-DUNICODE -D_UNICODE)
   remove_definitions (-D_MBCS)
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()
