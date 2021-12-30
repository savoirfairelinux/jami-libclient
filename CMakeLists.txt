CMAKE_MINIMUM_REQUIRED(VERSION 3.16)

IF(POLICY CMP0022)
   CMAKE_POLICY(SET CMP0022 NEW)
ENDIF(POLICY CMP0022)

IF(POLICY CMP0043)
   CMAKE_POLICY(SET CMP0043 NEW)
ENDIF(POLICY CMP0043)

IF(POLICY CMP0020)
   CMAKE_POLICY(SET CMP0020 NEW)
ENDIF(POLICY CMP0020)

IF(POLICY CMP0063)
   CMAKE_POLICY(SET CMP0063 NEW)
ENDIF(POLICY CMP0063)

INCLUDE(GNUInstallDirs)
INCLUDE(CMakePackageConfigHelpers)
INCLUDE(GenerateExportHeader)

option (ENABLE_SHARED "Build shared library" ON)

string(SUBSTRING ${CMAKE_GENERATOR} 0 14 CMAKE_GENERATOR_SHORT)
if(CMAKE_GENERATOR_SHORT MATCHES "Visual Studio ")
    message(STATUS "Generating VS project")
    set(CMAKE_CXX_COMPILER_ID "MSVC")
    set(ENABLE_TEST false)
    set(ENABLE_SHARED false)
    set(ENABLE_STATIC true)
    add_definitions(-DUNICODE -D_UNICODE)
    remove_definitions(-D_MBCS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi /W0 /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zi /W0 /MP")
endif()

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
    ADD_DEFINITIONS(
       ${QT_DEFINITIONS}
       -fexceptions
       -O2
    )
endif()

# Options
set(CHK_FREEABLE_BEFORE_ERASE_ACCOUNT_DESCRIPTION
"Check that an account has been marked freeable by the client
before freeing the structures. This avoids various race
conditions while removing accounts but may not be supported by
all clients. ON by default on Linux systems, otherwise OFF.")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  option (CHK_FREEABLE_BEFORE_ERASE_ACCOUNT CHK_FREEABLE_BEFORE_ERASE_ACCOUNT_DESCRIPTION ON)
ELSE()
  option (CHK_FREEABLE_BEFORE_ERASE_ACCOUNT CHK_FREEABLE_BEFORE_ERASE_ACCOUNT_DESCRIPTION OFF)
ENDIF()

PROJECT(ringclient)

# First, check is the compiler is new enough, most version of Clang are fine
# until problems arise, checking for GCC is enough
IF (CMAKE_COMPILER_IS_GNUCC)
   EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
   IF (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
      MESSAGE(STATUS "Found GCC version >= 4.8: " ${GCC_VERSION})
   ELSE()
      MESSAGE(FATAL_ERROR "Your version of GCC is too old, please install GCC 4.8 or later")
   ENDIF()
ENDIF()

SET(CMAKE_CXX_STANDARD 17)
SET(CXX_STANDARD_REQUIRED TRUE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Here we let find_package(<PackageName>...) try to find Qt 6,
# If it is found, find_package will succeed, and the CMake variable
# QT_VERSION_MAJOR will be defined 6.
find_package(QT NAMES Qt6 COMPONENTS Core REQUIRED)
if (${QT_VERSION_MAJOR} STRLESS 6)
   message(FATAL_ERROR "We currently only support Qt6" )
endif()

if(QT6_VER AND QT6_PATH)
   message(STATUS "Using custom Qt version")
   find_package(Qt${QT_VERSION_MAJOR}
                REQUIRED COMPONENTS Core DBus Sql Gui PATHS ${QT6_PATH} NO_DEFAULT_PATH)
   # Linguist tools is not required.
   find_package(Qt${QT_VERSION_MAJOR}
                COMPONENTS LinguistTools PATHS ${QT6_PATH} NO_DEFAULT_PATH)
else()
   find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus Sql Gui)
   # Linguist tools is not required.
   find_package(Qt${QT_VERSION_MAJOR}LinguistTools)
endif()

SET(LOCAL_CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LOCAL_CMAKE_MODULE_PATH})

FIND_PACKAGE(Ring REQUIRED)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux" AND NOT ENABLE_LIBWRAP)
   FIND_PACKAGE(Qt${QT_VERSION_MAJOR}DBus)
ELSE()
   SET(ENABLE_LIBWRAP true)
ENDIF()


IF(NOT ENABLE_TEST)
  SET(ENABLE_TEST false)
ENDIF()


IF((${ENABLE_TEST} MATCHES true))
  SET(ENABLE_LIBWRAP true) # because mocks use the same interface present in src/qtwrapper
ENDIF()

if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
# Enable some useful warnings
    ADD_DEFINITIONS(
       -Wall
       -pedantic
       -Wextra
       -Wmissing-declarations
       -Wmissing-noreturn
       -Wpointer-arith
       -Wcast-align
       -Wwrite-strings
       -Wformat-nonliteral
       -Wformat-security
       -Wmissing-include-dirs
       -Wundef
       -Wmissing-format-attribute
       -Wno-reorder
       -Wunused
       -Woverloaded-virtual
       -Wvarargs
    )
endif()

#Add more warnings for compilers that support it. I used this command:
#curl https://gcc.gnu.org/onlinedocs/gcc-4.9.2/gcc/Warning-Options.html | \
#grep -E "^[\t ]+<br><dt><code>-W[a-zA-Z=-]*" -o | grep -E "\-W[a-zA-Z=-]*" -o >
#cat /tmp/48 /tmp/49 | sort | uniq -u
# IF (CMAKE_COMPILER_IS_GNUCC)

IF (CMAKE_COMPILER_IS_GNUCC)
   IF (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
      ADD_DEFINITIONS(
         -Wconditionally-supported
         -Wno-cpp
         -Wdouble-promotion
         -Wdate-time
         -Wfloat-conversion
      )
   ENDIF()

   if (GCC_VERSION VERSION_GREATER 5.1 OR GCC_VERSION VERSION_EQUAL 5.1)
      ADD_DEFINITIONS(
         -Wformat-signedness
         -Wnormalized
         -Wshift-count-negative
         -Wshift-count-overflow
         -Wsized-deallocation
         -Wsizeof-array-argument
      )
   ENDIF()

   IF (GCC_VERSION VERSION_GREATER 6.0 OR GCC_VERSION VERSION_EQUAL 6.0)
      ADD_DEFINITIONS(
         -Wnull-dereference
         -Wshift-negative-value
         -Wshift-overflow
         -Wduplicated-cond
      )
   ENDIF()
ENDIF()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
   ADD_DEFINITIONS(
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
      #-Weverything
   )
endif()

# OS X
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

   SET(CMAKE_MACOSX_RPATH ON)
   SET(CMAKE_SKIP_BUILD_RPATH FALSE)
   SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
   SET(CMAKE_INSTALL_RPATH "${CMAKE_CURRENT_SOURCE_DIR}")
   SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${ENABLE_LIBWRAP} MATCHES true)
   # The daemon is only required for when using direct linking
   IF (${RING_FOUND} MATCHES "false")
      MESSAGE(FATAL_ERROR "The daemon is required")
   ENDIF()

   ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/src/qtwrapper)
   ADD_DEFINITIONS(-DENABLE_LIBWRAP=true) # Use native calls (no dbus)
   if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
      ADD_DEFINITIONS(-Wno-unknown-pragmas)
   endif()
   SET(ENABLE_QT${QT_VERSION_MAJOR} true)
   MESSAGE(STATUS "Compiling with qtwrapper enabled, Qt${QT_VERSION_MAJOR} enabled.")

   INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/src/qtwrapper/)

   IF(${VERBOSE_IPC} MATCHES true)
      MESSAGE(STATUS "Adding more debug output")
      ADD_DEFINITIONS(-DVERBOSE_IPC=true)
   ENDIF()
ENDIF()

IF (${RING_FOUND} MATCHES "true")
   INCLUDE_DIRECTORIES(${ring_INCLUDE_DIRS})
ENDIF()

IF(NOT (${ENABLE_VIDEO} MATCHES false))
   MESSAGE(STATUS "VIDEO enabled")
   SET(ENABLE_VIDEO 1 CACHE BOOL "Enable video")
   add_definitions( -DENABLE_VIDEO=true )
ENDIF(NOT (${ENABLE_VIDEO} MATCHES false))

IF(NOT (${ENABLE_PLUGIN} MATCHES false))
   MESSAGE(STATUS "PLUGIN enabled")
   SET(ENABLE_PLUGIN 1 CACHE BOOL "Enable plugin")
   add_definitions( -DENABLE_PLUGIN=true )
ENDIF(NOT (${ENABLE_PLUGIN} MATCHES false))

# ADD_DEFINITIONS(-DQT_DISABLE_DEPRECATED_BEFORE=1)

if (CHK_FREEABLE_BEFORE_ERASE_ACCOUNT)
   ADD_DEFINITIONS(
      -DCHK_FREEABLE_BEFORE_ERASE_ACCOUNT
   )
endif ()

SET(GENERIC_LIB_VERSION "1.0.0")

INCLUDE_DIRECTORIES(SYSTEM ${QT_INCLUDES} )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES ( ${CMAKE_CURRENT_BINARY_DIR})

IF(NOT (CMAKE_CXX_COMPILER_ID MATCHES "MSVC"))
    # Build dbus api
    IF(DEFINED RING_XML_INTERFACES_DIR)
        SET (dbus_xml_introspecs_path ${RING_XML_INTERFACES_DIR})
    ELSEIF(EXISTS "${RING_BUILD_DIR}/../bin/dbus")
        SET (dbus_xml_introspecs_path ${RING_BUILD_DIR}/../bin/dbus)
    ELSE()
        SET (dbus_xml_introspecs_path ${CMAKE_INSTALL_PREFIX}/share/dbus-1/interfaces)
    ENDIF()

    MESSAGE(STATUS "Using Ring DBus-XML interfaces in ${dbus_xml_introspecs_path}")
ENDIF()

#File to compile
SET( libringclient_LIB_SRCS
  #Data objects
  src/uri.cpp
  src/video/renderer.cpp

  #Models
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
  src/messagelistmodel.cpp

  #Communication
  src/dbus/configurationmanager.cpp
  src/dbus/callmanager.cpp
  src/dbus/instancemanager.cpp
  src/dbus/videomanager.cpp
  src/dbus/presencemanager.cpp
  src/dbus/pluginmanager.cpp

  #Default interface implementations
  src/globalinstances.cpp
  src/pixmapmanipulatordefault.cpp
  src/dbuserrorhandlerdefault.cpp

  #Other
  src/avmodel.cpp
  src/pluginmodel.cpp
  src/namedirectory.cpp
  src/smartinfohub.cpp
  src/chatview.cpp
)

# Public API
SET( libringclient_LIB_HDRS
  src/uri.h
  src/globalinstances.h
  src/pixmapmanipulatordefault.h
  src/dbuserrorhandlerdefault.h
  src/smartinfohub.h
  src/vcard.h
  src/namedirectory.h
  src/shmrenderer.h
  src/directrenderer.h
  src/messagelistmodel.h
)

SET(libringclient_api_LIB_HDRS
  src/api/interaction.h
  src/api/conversation.h
  src/api/contact.h
  src/api/call.h
  src/api/account.h
  src/api/chatview.h
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
  src/api/newvideo.h
)

SET(libringclient_WEB_chatview
   src/web-chatview/.eslintrc.json
   src/web-chatview/chatview-gnome.css
   src/web-chatview/chatview-qt.css
   src/web-chatview/chatview.css
   src/web-chatview/fa.css
   src/web-chatview/chatview.html
   src/web-chatview/chatview.js
   src/web-chatview/jed.js
   src/web-chatview/emoji.js
   src/web-chatview/emoji.css
   src/web-chatview/linkify-html.js
   src/web-chatview/linkify-string.js
   src/web-chatview/linkify.js
   src/web-chatview/previewInfo.js
   src/web-chatview/qwebchannel.js
   src/web-chatview/web.gresource.xml
)

SET(libringclient_video_LIB_HDRS
  src/video/renderer.h
  #The renderer implementations are not exported on purpose
)

SET(libringclient_interface_LIB_HDRS
  src/interfaces/pixmapmanipulatori.h
  src/interfaces/dbuserrorhandleri.h
)

SET( libringclient_extra_LIB_HDRS
  src/typedefs.h
  src/containerview.h
)

IF(${ENABLE_LIBWRAP} MATCHES true OR ${ENABLE_TEST} MATCHES true)
   # done this way because of bug in cmake 2.8
   # (not necessary in 3.0+)
ELSE()
   # presence manager interface
   SET ( presencemanager_xml  ${dbus_xml_introspecs_path}/cx.ring.Ring.PresenceManager.xml )

   SET( dbus_metatype_path "${CMAKE_CURRENT_SOURCE_DIR}/src/dbus/metatypes.h")

   SET_SOURCE_FILES_PROPERTIES(
      ${presencemanager_xml}
      PROPERTIES
      CLASSNAME PresenceManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   # configuration manager interface
   SET ( configurationmanager_xml  ${dbus_xml_introspecs_path}/cx.ring.Ring.ConfigurationManager.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${configurationmanager_xml}
      PROPERTIES
      CLASSNAME ConfigurationManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   # call manager interface
   SET ( callmanager_xml  ${dbus_xml_introspecs_path}/cx.ring.Ring.CallManager.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${callmanager_xml}
      PROPERTIES
      CLASSNAME CallManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   # video manager interface
   SET ( video_xml  ${dbus_xml_introspecs_path}/cx.ring.Ring.VideoManager.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${video_xml}
      PROPERTIES
      CLASSNAME VideoManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   # plugin manager interface
   SET ( plugin_xml  ${dbus_xml_introspecs_path}/cx.ring.Ring.PluginManagerInterface.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${plugin_xml}
      PROPERTIES
      CLASSNAME PluginManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   # instance interface
   SET ( instance_xml  ${dbus_xml_introspecs_path}/cx.ring.Ring.Instance.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${instance_xml}
      PROPERTIES
      CLASSNAME InstanceManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   QT_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${presencemanager_xml}
      presencemanager_dbus_interface
   )

   QT_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${configurationmanager_xml}
      configurationmanager_dbus_interface
   )

   QT_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${callmanager_xml}
      callmanager_dbus_interface
   )

   QT_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${video_xml}
      video_dbus_interface
   )

   QT_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${plugin_xml}
      plugin_dbus_interface
   )

   QT_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${instance_xml}
      instance_dbus_interface
   )
ENDIF()

# Manually wrap private files and interfaces
SET(libringclient_PRIVATE_HDRS
   src/private/namedirectory_p.h
   src/private/smartInfoHub_p.h
   src/private/videorenderer_p.h
)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)


IF(${ENABLE_LIBWRAP} MATCHES true AND ${ENABLE_TEST} MATCHES false)
   SET(libringclient_PRIVATE_HDRS
      ${libringclient_PRIVATE_HDRS}

      src/qtwrapper/callmanager_wrap.h
      src/qtwrapper/configurationmanager_wrap.h
      src/qtwrapper/instancemanager_wrap.h
      src/qtwrapper/presencemanager_wrap.h
      src/qtwrapper/videomanager_wrap.h
      src/qtwrapper/pluginmanager_wrap.h
   )
ENDIF()

IF(NOT ${ENABLE_TEST} MATCHES true)
   QT_WRAP_CPP(LIB_HEADER_MOC ${libringclient_PRIVATE_HDRS})
ENDIF()

IF(NOT ${ENABLE_STATIC} MATCHES false)
    message(STATUS "Configuring as static lib")
    add_library( ringclient_static  STATIC ${libringclient_LIB_SRCS} ${libringclient_api_LIB_HDRS} ${LIB_HEADER_MOC} )
    target_link_libraries(ringclient_static Qt::Core)
    target_link_libraries(ringclient_static Qt::Gui)
    target_link_libraries( ringclient_static
       -lpthread
       ${QT_QTDBUS_LIBRARY}
       ${QT_QTCORE_LIBRARY}
       ${QT_QTGUI_LIBRARY}
       Qt::Sql
       )
    SET_TARGET_PROPERTIES( ringclient_static
       PROPERTIES VERSION ${GENERIC_LIB_VERSION}
    )
ENDIF()

if (ENABLE_SHARED)
    message(STATUS "Configuring as shared lib")
    add_library(ringclient SHARED ${libringclient_LIB_SRCS} ${libringclient_api_LIB_HDRS} ${LIB_HEADER_MOC} )
    target_link_libraries(ringclient Qt::Core Qt::Gui)
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if (ENABLE_SHARED)
        target_link_libraries(ringclient Qt::DBus)
    endif()
  IF(${ENABLE_STATIC} MATCHES true)
      target_link_libraries(ringclient_static Qt::DBus)
  ENDIF()
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    FIND_PACKAGE(Qt${QT_VERSION_MAJOR}Concurrent)

    #For some reason, QSemaphore wont compile on Windows without QtConcurrent
    if(${ENABLE_STATIC} MATCHES true)
        target_link_libraries(ringclient_static Qt::Concurrent)
    endif()

    if(ENABLE_SHARED)
        target_link_libraries(ringclient Qt::Concurrent)
    endif()
ENDIF()


IF (NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    TARGET_LINK_LIBRARIES( ringclient
        -lpthread
        ${QT_QTCORE_LIBRARY}
        Qt::Gui
        Qt::Sql
        )
ENDIF()

IF(${ENABLE_LIBWRAP} MATCHES true)
   IF((NOT ${ring_BIN} MATCHES "ring_BIN-NOTFOUND") AND ENABLE_SHARED)
      TARGET_LINK_LIBRARIES( ringclient
         qtwrapper
         ${ring_BIN}
      )
   ELSE()
      # Allow building with undefined symbols when only the daemon headers are provided
      # It speeds up our CI builds
      IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
         SET_TARGET_PROPERTIES( ringclient PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" )
      ENDIF()
   ENDIF()
   IF(NOT ${ENABLE_STATIC} MATCHES false)
      TARGET_LINK_LIBRARIES( ringclient_static
         qtwrapper
         ${ring_BIN}
      )
   ENDIF()

ELSE()
   TARGET_LINK_LIBRARIES( ringclient
      ${QT_QTDBUS_LIBRARY}
   )
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   TARGET_LINK_LIBRARIES( ringclient
      -lrt
   )

   IF(NOT ${ENABLE_STATIC} MATCHES false)
      TARGET_LINK_LIBRARIES( ringclient_static
         -lrt
      )
   ENDIF()
ENDIF()

if(ENABLE_SHARED)
    SET_TARGET_PROPERTIES( ringclient
      PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_VERSION}
    )
endif()

SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)

INSTALL( FILES ${libringclient_LIB_HDRS} ${libringclient_extra_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_video_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/video
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_plugin_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/plugin
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_WEB_chatview}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/web-chatview
  COMPONENT Devel
)

INSTALL( FILES src/webresource.qrc
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_extensions_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/extensions
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_interface_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/interfaces
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_api_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/api
  COMPONENT Devel
)


#This hack force Debian based system to return a non multi-arch path
#this is required to prevent the .deb libringclient.so from having an
#higher priority than the prefixed one.
STRING(REPLACE "${CMAKE_LIBRARY_ARCHITECTURE}" "" SANE_LIBRARY_PATH "${CMAKE_INSTALL_FULL_LIBDIR}" )

IF(NOT ${ENABLE_STATIC} MATCHES false)
    INSTALL( TARGETS ringclient_static
        ARCHIVE DESTINATION ${SANE_LIBRARY_PATH}
        LIBRARY DESTINATION ${SANE_LIBRARY_PATH}
        DESTINATION ${SANE_LIBRARY_PATH}
    )
ENDIF()

if(ENABLE_SHARED)
    INSTALL( TARGETS ringclient
        ARCHIVE DESTINATION ${SANE_LIBRARY_PATH}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${SANE_LIBRARY_PATH}
        DESTINATION ${SANE_LIBRARY_PATH}
    )
endif()

SET(LIB_INSTALL_DIR ${SANE_LIBRARY_PATH})

# Create a CMake config file

# TARGET_INCLUDE_DIRECTORIES(ringclient PUBLIC
# "$<INSTALL_INTERFACE:$<CMAKE_INSTALL_PREFIX>/${INCLUDE_INSTALL_DIR}/libringclient>"
# )
SET(libringclient_CONFIG_PATH "${CMAKE_CURRENT_BINARY_DIR}/LibRingClientConfig.cmake")

CONFIGURE_PACKAGE_CONFIG_FILE(
   "${CMAKE_SOURCE_DIR}/cmake/LibRingClientConfig.cmake.in" ${libringclient_CONFIG_PATH}
   INSTALL_DESTINATION ${LIB_INSTALL_DIR}/libringclient/cmake
   PATH_VARS INCLUDE_INSTALL_DIR
)

INSTALL( FILES ${libringclient_CONFIG_PATH}
  DESTINATION ${SANE_LIBRARY_PATH}/cmake/LibRingClient
  COMPONENT Devel
)

# WRITE_BASIC_PACKAGE_VERSION_FILE(
#    ${libringclient_CONFIG_PATH}
#    VERSION ${GENERIC_LIB_VERSION}
#    COMPATIBILITY SameMajorVersion
# )

# translations
IF(Qt${QT_VERSION_MAJOR}LinguistTools_FOUND)
   # translation template file
   SET(TS_TEMPLATE ${CMAKE_SOURCE_DIR}/translations/lrc_en.ts)

   # get all translation files except the template
   FILE(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/translations/*.ts)
   LIST(REMOVE_ITEM TS_FILES ${TS_TEMPLATE})

   # compiled .qm files
   QT_ADD_TRANSLATION(QM_FILES ${TS_FILES})

   ADD_CUSTOM_TARGET(translations ALL DEPENDS ${QM_FILES})

   INSTALL(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/libringclient/translations")
ENDIF()

# uninstall target
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
