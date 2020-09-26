cmake_minimum_required (VERSION 3.5.1)

if (POLICY CMP0022)
   cmake_policy (SET CMP0022 NEW)
endif (POLICY CMP0022)

if (POLICY CMP0043)
   cmake_policy (SET CMP0043 NEW)
endif (POLICY CMP0043)

if (POLICY CMP0020)
   cmake_policy (SET CMP0020 NEW)
endif (POLICY CMP0020)

if (POLICY CMP0063)
   cmake_policy (SET CMP0063 NEW)
endif (POLICY CMP0063)

include (GNUInstallDirs)
include (CMakePackageConfigHelpers)
include (GenerateExportHeader)

option (ENABLE_SHARED "Build shared library" ON)

string (SUBSTRING ${CMAKE_GENERATOR} 0 14 CMAKE_GENERATOR_SHORT)
if (CMAKE_GENERATOR_SHORT MATCHES "Visual Studio ")
   message (STATUS "Generating VS project")
   set (CMAKE_CXX_COMPILER_ID "MSVC")
   set (ENABLE_TEST false)
   set (ENABLE_STATIC true)
   set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /W0 /MP")
endif ()

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
   add_definitions (${QT_DEFINITIONS} -fexceptions -O2)
endif ()

# Options
set (
   CHK_FREEABLE_BEFORE_ERASE_ACCOUNT_DESCRIPTION
   "Check that an account has been marked freeable by the client
before freeing the structures. This avoids various race
conditions while removing accounts but may not be supported by
all clients. ON by default on Linux systems, otherwise OFF.")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   option (CHK_FREEABLE_BEFORE_ERASE_ACCOUNT CHK_FREEABLE_BEFORE_ERASE_ACCOUNT_DESCRIPTION ON)
else ()
   option (CHK_FREEABLE_BEFORE_ERASE_ACCOUNT CHK_FREEABLE_BEFORE_ERASE_ACCOUNT_DESCRIPTION OFF)
endif ()

project (ringclient)

# First, check is the compiler is new enough, most version of Clang are fine until problems arise,
# checking for GCC is enough
if (CMAKE_COMPILER_IS_GNUCC)
   execute_process (COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
      message (STATUS "Found GCC version >= 4.8: " ${GCC_VERSION})
   else ()
      message (FATAL_ERROR "Your version of GCC is too old, please install GCC 4.8 or later")
   endif ()
endif ()

set (CMAKE_CXX_STANDARD 17)
set (CXX_STANDARD_REQUIRED TRUE)

set (CMAKE_AUTOMOC TRUE)

set (LOCAL_CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LOCAL_CMAKE_MODULE_PATH})

find_package (Ring REQUIRED)

if (${QT_MIN_VER} MATCHES "5.15")
   message (STATUS "Using custom Qt version")
   find_package (
      Qt5 ${QT_MIN_VER} REQUIRED
      COMPONENTS Core
                 DBus
                 Sql
                 Gui
                 PATHS
                 ${QT5_PATH}
                 NO_DEFAULT_PATH)
   # Linguist tools is not required.
   find_package (Qt5 ${QT_MIN_VER} COMPONENTS LinguistTools PATHS ${QT5_PATH} NO_DEFAULT_PATH)
   set (CMAKE_INSTALL_RPATH_USE_LINK_PATH true)
else ()
   find_package (Qt5 REQUIRED COMPONENTS Core DBus Sql Gui)
   # Linguist tools is not required.
   find_package (Qt5LinguistTools)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT ENABLE_LIBWRAP)
   find_package (Qt5DBus)
else ()
   set (ENABLE_LIBWRAP true)
endif ()

if (NOT ENABLE_TEST)
   set (ENABLE_TEST false)
endif ()

if ((${ENABLE_TEST} MATCHES true))
   set (ENABLE_LIBWRAP true) # because mocks use the same interface present in src/qtwrapper
endif ()

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
   # Enable some useful warnings
   add_definitions (
      -Wall
      -Wextra
      -Wmissing-declarations
      -Wmissing-noreturn
      -Wpointer-arith
      -Wcast-align
      -Wwrite-strings
      -Wformat-nonliteral
      -Wformat-security
      -Wswitch-enum
      -Winit-self
      -Wmissing-include-dirs
      -Wundef
      -Wmissing-format-attribute
      -Wno-reorder
      -Wunused
      -Wuninitialized
      -Woverloaded-virtual
      -Wunused-value
      -pedantic
      -Wnonnull
      -Wsequence-point
      # -Wsystem-headers
      -Wsizeof-pointer-memaccess
      # -Wuseless-cast
      -Wvarargs
      # See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55578
      -Wno-unused-function
      -Wno-attributes)
endif ()

# Add more warnings for compilers that support it. I used this command: curl
# https://gcc.gnu.org/onlinedocs/gcc-4.9.2/gcc/Warning-Options.html | \ grep -E "^[\t
# ]+<br><dt><code>-W[a-zA-Z=-]*" -o | grep -E "\-W[a-zA-Z=-]*" -o > cat /tmp/48 /tmp/49 | sort |
# uniq -u IF (CMAKE_COMPILER_IS_GNUCC)

if (CMAKE_COMPILER_IS_GNUCC)
   if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
      add_definitions (
         -Wunused-but-set-parameter
         -Wconditionally-supported
         # -Wsuggest-attribute=const
         -Wno-cpp
         -Wdouble-promotion
         -Wdate-time
         -Wdelete-incomplete
         -Wfloat-conversion)
   endif ()

   if (GCC_VERSION VERSION_GREATER 5.1 OR GCC_VERSION VERSION_EQUAL 5.1)
      add_definitions (
         # -Wsuggest-override -Wsuggest-final-types -Wsuggest-final-methods
         -Wbool-compare
         -Wformat-signedness
         -Wlogical-not-parentheses
         -Wnormalized
         -Wshift-count-negative
         -Wshift-count-overflow
         -Wsized-deallocation
         -Wsizeof-array-argument)
   endif ()

   if (GCC_VERSION VERSION_GREATER 6.0 OR GCC_VERSION VERSION_EQUAL 6.0)
      add_definitions (-Wnull-dereference -Wshift-negative-value -Wshift-overflow -Wduplicated-cond
                       -Wmisleading-indentation)
   endif ()
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   add_definitions (
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-unknown-pragmas
      -Wno-documentation-unknown-command
      -Wno-padded
      -Wno-old-style-cast
      -Wno-sign-conversion
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-shorten-64-to-32
      # -Weverything
   )
endif ()

# OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   set (CMAKE_MACOSX_RPATH ON)
   set (CMAKE_SKIP_BUILD_RPATH FALSE)
   set (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
   set (CMAKE_INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}")
   set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if (${ENABLE_LIBWRAP} MATCHES true)
   # The daemon is only required for when using direct linking
   if (${RING_FOUND} MATCHES "false")
      message (FATAL_ERROR "The daemon is required")
   endif ()

   add_subdirectory (${CMAKE_SOURCE_DIR}/src/qtwrapper)
   add_definitions (-DENABLE_LIBWRAP=true) # Use native calls (no dbus)
   if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
      add_definitions (-Wno-unknown-pragmas)
   endif ()
   set (ENABLE_QT5 true) # Use Qt5
   message (STATUS "Compiling with qtwrapper enabled, Qt5 enabled.")

   include_directories (${CMAKE_SOURCE_DIR}/src/qtwrapper/)

   if (${VERBOSE_IPC} MATCHES true)
      message (STATUS "Adding more debug output")
      add_definitions (-DVERBOSE_IPC=true)
   endif ()
endif ()

if (${RING_FOUND} MATCHES "true")
   include_directories (${ring_INCLUDE_DIRS})
endif ()

if (NOT (${ENABLE_VIDEO} MATCHES false))
   message (STATUS "VIDEO enabled")
   set (
      ENABLE_VIDEO
      1
      CACHE STRING "Enable video")
   add_definitions (-DENABLE_VIDEO=true)
endif (NOT (${ENABLE_VIDEO} MATCHES false))

if (NOT (${ENABLE_PLUGIN} MATCHES false))
   message (STATUS "PLUGIN enabled")
   set (
      ENABLE_PLUGIN
      1
      CACHE STRING "Enable plugin")
   add_definitions (-DENABLE_PLUGIN=true)
endif (NOT (${ENABLE_PLUGIN} MATCHES false))

# ADD_DEFINITIONS(-DQT_DISABLE_DEPRECATED_BEFORE=1)

if (CHK_FREEABLE_BEFORE_ERASE_ACCOUNT)
   add_definitions (-DCHK_FREEABLE_BEFORE_ERASE_ACCOUNT)
endif ()

set (GENERIC_LIB_VERSION "1.0.0")

include_directories (SYSTEM ${QT_INCLUDES})
include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
   # Build dbus api
   if (DEFINED RING_XML_INTERFACES_DIR)
      set (dbus_xml_introspecs_path ${RING_XML_INTERFACES_DIR})
   elseif (EXISTS "${RING_BUILD_DIR}/../bin/dbus")
      set (dbus_xml_introspecs_path ${RING_BUILD_DIR}/../bin/dbus)
   else ()
      set (dbus_xml_introspecs_path ${CMAKE_INSTALL_PREFIX}/share/dbus-1/interfaces)
   endif ()

   message (STATUS "Using Ring DBus-XML interfaces in ${dbus_xml_introspecs_path}")
endif ()

# File to compile
set (
   libringclient_LIB_SRCS
   # Data objects
   src/uri.cpp
   src/video/renderer.cpp
   # Models
   src/contactmodel.cpp
   src/newcallmodel.cpp
   src/newdevicemodel.cpp
   src/newcodecmodel.cpp
   src/conversationmodel.cpp
   src/database.cpp
   src/authority/daemon.cpp
   src/authority/storagehelper.cpp
   src/lrc.cpp
   src/newaccountmodel.cpp
   src/peerdiscoverymodel.cpp
   src/callbackshandler.cpp
   src/behaviorcontroller.cpp
   src/datatransfermodel.cpp
   src/newvideo.cpp
   src/shmrenderer.cpp
   src/directrenderer.cpp
   # Communication
   src/dbus/configurationmanager.cpp
   src/dbus/callmanager.cpp
   src/dbus/instancemanager.cpp
   src/dbus/videomanager.cpp
   src/dbus/presencemanager.cpp
   src/dbus/pluginmanager.cpp
   # Default interface implementations
   src/globalinstances.cpp
   src/pixmapmanipulatordefault.cpp
   src/dbuserrorhandlerdefault.cpp
   # Other
   src/avmodel.cpp
   src/pluginmodel.cpp
   src/namedirectory.cpp
   src/smartinfohub.cpp)

# Public API
set (
   libringclient_LIB_HDRS
   src/uri.h
   src/globalinstances.h
   src/pixmapmanipulatordefault.h
   src/dbuserrorhandlerdefault.h
   src/smartinfohub.h
   src/vcard.h
   src/namedirectory.h
   src/shmrenderer.h
   src/directrenderer.h)

set (
   libringclient_api_LIB_HDRS
   src/api/interaction.h
   src/api/conversation.h
   src/api/contact.h
   src/api/call.h
   src/api/account.h
   src/api/lrc.h
   src/api/avmodel.h
   src/api/pluginmodel.h
   src/api/newaccountmodel.h
   src/api/peerdiscoverymodel.h
   src/api/newcallmodel.h
   src/api/newcodecmodel.h
   src/api/newdevicemodel.h
   src/api/contactmodel.h
   src/api/conversationmodel.h
   src/api/profile.h
   src/api/behaviorcontroller.h
   src/api/datatransfermodel.h
   src/api/datatransfer.h
   src/api/newvideo.h)

set (
   libringclient_WEB_chatview
   src/web-chatview/.eslintrc.json
   src/web-chatview/chatview-gnome.css
   src/web-chatview/chatview.css
   src/web-chatview/chatview.html
   src/web-chatview/chatview.js
   src/web-chatview/jed.js
   src/web-chatview/linkify-html.js
   src/web-chatview/linkify-string.js
   src/web-chatview/linkify.js
   src/web-chatview/qwebchannel.js
   src/web-chatview/web.gresource.xml)

set (libringclient_video_LIB_HDRS src/video/renderer.h
                                  # The renderer implementations are not exported on purpose
)

set (libringclient_interface_LIB_HDRS src/interfaces/pixmapmanipulatori.h
                                      src/interfaces/dbuserrorhandleri.h)

set (libringclient_extra_LIB_HDRS src/typedefs.h)

if (${ENABLE_LIBWRAP} MATCHES true OR ${ENABLE_TEST} MATCHES true)
   # done this way because of bug in cmake 2.8 (not necessary in 3.0+)
else ()
   # presence manager interface
   set (presencemanager_xml ${dbus_xml_introspecs_path}/cx.ring.Ring.PresenceManager.xml)

   set (dbus_metatype_path "${CMAKE_CURRENT_SOURCE_DIR}/src/dbus/metatypes.h")

   set_source_files_properties (${presencemanager_xml} PROPERTIES CLASSNAME PresenceManagerInterface
                                                                  INCLUDE ${dbus_metatype_path})

   qt5_add_dbus_interface (libringclient_LIB_SRCS ${presencemanager_xml}
                           presencemanager_dbus_interface)

   # configuration manager interface
   set (configurationmanager_xml ${dbus_xml_introspecs_path}/cx.ring.Ring.ConfigurationManager.xml)

   set_source_files_properties (
      ${configurationmanager_xml} PROPERTIES CLASSNAME ConfigurationManagerInterface
                                             INCLUDE ${dbus_metatype_path})

   qt5_add_dbus_interface (libringclient_LIB_SRCS ${configurationmanager_xml}
                           configurationmanager_dbus_interface)

   # call manager interface
   set (callmanager_xml ${dbus_xml_introspecs_path}/cx.ring.Ring.CallManager.xml)

   set_source_files_properties (${callmanager_xml} PROPERTIES CLASSNAME CallManagerInterface
                                                              INCLUDE ${dbus_metatype_path})

   qt5_add_dbus_interface (libringclient_LIB_SRCS ${callmanager_xml} callmanager_dbus_interface)

   # video manager interface
   set (video_xml ${dbus_xml_introspecs_path}/cx.ring.Ring.VideoManager.xml)

   set_source_files_properties (${video_xml} PROPERTIES CLASSNAME VideoManagerInterface
                                                        INCLUDE ${dbus_metatype_path})

   qt5_add_dbus_interface (libringclient_LIB_SRCS ${video_xml} video_dbus_interface)

   # plugin manager interface
   set (plugin_xml ${dbus_xml_introspecs_path}/cx.ring.Ring.PluginManagerInterface.xml)

   set_source_files_properties (${plugin_xml} PROPERTIES CLASSNAME PluginManagerInterface
                                                         INCLUDE ${dbus_metatype_path})

   qt5_add_dbus_interface (libringclient_LIB_SRCS ${plugin_xml} plugin_dbus_interface)

   # instance interface
   set (instance_xml ${dbus_xml_introspecs_path}/cx.ring.Ring.Instance.xml)

   set_source_files_properties (${instance_xml} PROPERTIES CLASSNAME InstanceManagerInterface
                                                           INCLUDE ${dbus_metatype_path})

   qt5_add_dbus_interface (libringclient_LIB_SRCS ${instance_xml} instance_dbus_interface)
endif ()

# Manually wrap private files and interfaces
set (libringclient_PRIVATE_HDRS src/private/namedirectory_p.h src/private/smartInfoHub_p.h
                                src/private/videorenderer_p.h)

if (${ENABLE_LIBWRAP} MATCHES true AND ${ENABLE_TEST} MATCHES false)
   set (
      libringclient_PRIVATE_HDRS
      ${libringclient_PRIVATE_HDRS}
      src/qtwrapper/callmanager_wrap.h
      src/qtwrapper/configurationmanager_wrap.h
      src/qtwrapper/instancemanager_wrap.h
      src/qtwrapper/presencemanager_wrap.h
      src/qtwrapper/videomanager_wrap.h
      src/qtwrapper/pluginmanager_wrap.h)
endif ()

if (NOT ${ENABLE_TEST} MATCHES true)
   qt5_wrap_cpp (LIB_HEADER_MOC ${libringclient_PRIVATE_HDRS})
endif ()

if (NOT ${ENABLE_STATIC} MATCHES false)
   message (STATUS "Configuring as static lib")
   add_library (ringclient_static STATIC ${libringclient_LIB_SRCS} ${libringclient_api_LIB_HDRS}
                                         ${LIB_HEADER_MOC})
   target_link_libraries (ringclient_static Qt5::Core)
   target_link_libraries (ringclient_static Qt5::Gui)
   target_link_libraries (ringclient_static -lpthread ${QT_QTDBUS_LIBRARY} ${QT_QTCORE_LIBRARY}
                          ${QT_QTGUI_LIBRARY} Qt5::Sql)
   set_target_properties (ringclient_static PROPERTIES VERSION ${GENERIC_LIB_VERSION})
endif ()

if (ENABLE_SHARED)
   message (STATUS "Configuring as shared lib")
   add_library (ringclient SHARED ${libringclient_LIB_SRCS} ${libringclient_api_LIB_HDRS}
                                  ${LIB_HEADER_MOC})
   target_link_libraries (ringclient Qt5::Core Qt5::Gui)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   if (ENABLE_SHARED)
      target_link_libraries (ringclient Qt5::DBus)
   endif ()
   if (${ENABLE_STATIC} MATCHES true)
      target_link_libraries (ringclient_static Qt5::DBus)
   endif ()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   find_package (Qt5Concurrent)

   # For some reason, QSemaphore wont compile on Windows without QtConcurrent
   if (${ENABLE_STATIC} MATCHES true)
      target_link_libraries (ringclient_static Qt5::Concurrent)
   endif ()

   if (ENABLE_SHARED)
      target_link_libraries (ringclient Qt5::Concurrent)
   endif ()
endif ()

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
   target_link_libraries (ringclient -lpthread ${QT_QTCORE_LIBRARY} Qt5::Gui Qt5::Sql)
endif ()

if (${ENABLE_LIBWRAP} MATCHES true)
   if ((NOT ${ring_BIN} MATCHES "ring_BIN-NOTFOUND") AND ENABLE_SHARED)
      target_link_libraries (ringclient qtwrapper ${ring_BIN})
   else ()
      # Allow building with undefined symbols when only the daemon headers are provided It speeds up
      # our CI builds
      if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
         set_target_properties (ringclient PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
      endif ()
   endif ()
   if (NOT ${ENABLE_STATIC} MATCHES false)
      target_link_libraries (ringclient_static qtwrapper ${ring_BIN})
   endif ()

else ()
   target_link_libraries (ringclient ${QT_QTDBUS_LIBRARY})
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   target_link_libraries (ringclient -lrt)

   if (NOT ${ENABLE_STATIC} MATCHES false)
      target_link_libraries (ringclient_static -lrt)
   endif ()
endif ()

if (ENABLE_SHARED)
   set_target_properties (ringclient PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                                SOVERSION ${GENERIC_LIB_VERSION})
endif ()

set (INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)

install (
   FILES ${libringclient_LIB_HDRS} ${libringclient_extra_LIB_HDRS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient
   COMPONENT Devel)

install (
   FILES ${libringclient_video_LIB_HDRS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/video
   COMPONENT Devel)

install (
   FILES ${libringclient_plugin_LIB_HDRS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/plugin
   COMPONENT Devel)

install (
   FILES ${libringclient_WEB_chatview}
   DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/web-chatview
   COMPONENT Devel)

install (
   FILES ${libringclient_extensions_LIB_HDRS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/extensions
   COMPONENT Devel)

install (
   FILES ${libringclient_interface_LIB_HDRS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/interfaces
   COMPONENT Devel)

install (
   FILES ${libringclient_api_LIB_HDRS}
   DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/api
   COMPONENT Devel)

# This hack force Debian based system to return a non multi-arch path this is required to prevent
# the .deb libringclient.so from having an higher priority than the prefixed one.
string (REPLACE "${CMAKE_LIBRARY_ARCHITECTURE}" "" SANE_LIBRARY_PATH "${CMAKE_INSTALL_FULL_LIBDIR}")

if (NOT ${ENABLE_STATIC} MATCHES false)
   install (
      TARGETS ringclient_static
      ARCHIVE DESTINATION ${SANE_LIBRARY_PATH}
      LIBRARY DESTINATION ${SANE_LIBRARY_PATH} DESTINATION ${SANE_LIBRARY_PATH})
endif ()

if (ENABLE_SHARED)
   install (
      TARGETS ringclient
      ARCHIVE DESTINATION ${SANE_LIBRARY_PATH}
      RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
      LIBRARY DESTINATION ${SANE_LIBRARY_PATH} DESTINATION ${SANE_LIBRARY_PATH})
endif ()

set (LIB_INSTALL_DIR ${SANE_LIBRARY_PATH})

# Create a CMake config file

# TARGET_INCLUDE_DIRECTORIES(ringclient PUBLIC
# "$<INSTALL_INTERFACE:$<CMAKE_INSTALL_PREFIX>/${INCLUDE_INSTALL_DIR}/libringclient>" )
set (libringclient_CONFIG_PATH "${CMAKE_CURRENT_BINARY_DIR}/LibRingClientConfig.cmake")

configure_package_config_file (
   "${CMAKE_SOURCE_DIR}/cmake/LibRingClientConfig.cmake.in" ${libringclient_CONFIG_PATH}
   INSTALL_DESTINATION ${LIB_INSTALL_DIR}/libringclient/cmake
   PATH_VARS INCLUDE_INSTALL_DIR)

install (
   FILES ${libringclient_CONFIG_PATH}
   DESTINATION ${SANE_LIBRARY_PATH}/cmake/LibRingClient
   COMPONENT Devel)

# WRITE_BASIC_PACKAGE_VERSION_FILE( ${libringclient_CONFIG_PATH} VERSION ${GENERIC_LIB_VERSION}
# COMPATIBILITY SameMajorVersion )

# translations
if (Qt5LinguistTools_FOUND)
   # translation template file
   set (TS_TEMPLATE ${CMAKE_SOURCE_DIR}/translations/lrc_en.ts)

   # get all translation files except the template
   file (GLOB TS_FILES ${CMAKE_SOURCE_DIR}/translations/*.ts)
   list (REMOVE_ITEM TS_FILES ${TS_TEMPLATE})

   # compiled .qm files
   qt5_add_translation (QM_FILES ${TS_FILES})

   add_custom_target (translations ALL DEPENDS ${QM_FILES})

   install (FILES ${QM_FILES}
            DESTINATION "${CMAKE_INSTALL_PREFIX}/share/libringclient/translations")
endif ()

# uninstall target
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
                "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

add_custom_target (uninstall COMMAND ${CMAKE_COMMAND} -P
                                     ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

if ((${ENABLE_TEST} MATCHES true))
   add_definitions (-DENABLE_LIBWRAP=true) # Avoid dbus models
   add_definitions (-DENABLE_TEST=true) # Use mocks
   include_directories (SYSTEM ${Qt5Core_INCLUDE_DIRS})
   include_directories (SYSTEM ${Qt5Gui_INCLUDE_DIRS})
   find_package (Cppunit REQUIRED)
   find_package (Qt5Widgets)
   message (STATUS "Is cppunit present? " ${CPPUNIT_FOUND})
   # unit testing
   if (CPPUNIT_FOUND)
      add_executable (
         LRCTester
         test/mocks/callmanager_mock.h
         test/mocks/configurationmanager_mock.h
         test/mocks/instancemanager_mock.h
         test/mocks/presencemanager_mock.h
         test/mocks/videomanager_mock.h
         test/test_runner.cpp
         test/databasetester.h
         test/databasetester.cpp
         test/utils/waitforsignalhelper.h
         test/utils/waitforsignalhelper.cpp
         test/conversationmodeltester.h
         test/conversationmodeltester.cpp
         test/contactmodeltester.h
         test/contactmodeltester.cpp
         test/datatransfertester.h
         test/datatransfertester.cpp
         test/newcallmodeltester.h
         test/newcallmodeltester.cpp
         test/newaccountmodeltester.h
         test/newaccountmodeltester.cpp
         ${libringclient_LIB_SRCS}
         ${LIB_HEADER_MOC}
         ${libringclient_PRIVATE_HDRS})
      target_link_libraries (LRCTester -lcppunit -lpthread ${QT_QTCORE_LIBRARY} Qt5::Gui Qt5::Sql)

      if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
         target_link_libraries (LRCTester -lrt)
      endif ()
      enable_testing ()
      add_test (TEST LRCTester)
   endif ()
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
   add_definitions (-DUNICODE -D_UNICODE)
   remove_definitions (-D_MBCS)
   set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif ()
