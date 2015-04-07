CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(POLICY CMP0022)
   CMAKE_POLICY(SET CMP0022 NEW)
ENDIF(POLICY CMP0022)

IF(POLICY CMP0043)
   CMAKE_POLICY(SET CMP0043 NEW)
ENDIF(POLICY CMP0043)

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

INCLUDE(GNUInstallDirs)
INCLUDE(CMakePackageConfigHelpers)
INCLUDE(GenerateExportHeader)

ADD_DEFINITIONS("-std=c++1y")

ADD_DEFINITIONS(
   ${QT_DEFINITIONS}
   -fexceptions
)

PROJECT(ringclient)

SET(CMAKE_AUTOMOC TRUE)

SET(LOCAL_CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LOCAL_CMAKE_MODULE_PATH})

SET(QT5_MODULE_PATH ${QT5_PATH}/lib/cmake)

FIND_PACKAGE(Ring REQUIRED)


# Enable some useful warnings
ADD_DEFINITIONS(
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
   #-Wsystem-headers
   -Wsizeof-pointer-memaccess
   #-Wuseless-cast
   -Wvarargs
)

#Add more warnings for compilers that support it. I used this command:
#curl https://gcc.gnu.org/onlinedocs/gcc-4.9.2/gcc/Warning-Options.html | \
#grep -E "^[\t ]+<br><dt><code>-W[a-zA-Z=-]*" -o | grep -E "\-W[a-zA-Z=-]*" -o >
#cat /tmp/48 /tmp/49 | sort | uniq -u
# IF (CMAKE_COMPILER_IS_GNUCC)

IF (CMAKE_COMPILER_IS_GNUCC)
   IF (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
      ADD_DEFINITIONS(
         -Wunused-but-set-parameter
         -Wconditionally-supported
         -Wsuggest-attribute=const
         -Wno-cpp
         -Wdouble-promotion
         -Wdate-time
         -Wdelete-incomplete
         -Wfloat-conversion
      )
   ENDIF()

   IF (GCC_VERSION VERSION_GREATER 5.0 OR GCC_VERSION VERSION_EQUAL 5.0)
      ADD_DEFINITIONS(
         -Wsuggest-override
         -Wsuggest-final-types
         -Wsuggest-final-methods
         -Wbool-compare
         -Wformat-signedness
         -Wlogical-not-parentheses
         -Wnormalized
         -Wshift-count-negative
         -Wshift-count-overflow
         -Wsized-deallocation
         -Wsizeof-array-argument
      )
   ENDIF()

ENDIF()

# OS X
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   SET(ENABLE_LIBWRAP true)

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
   ADD_DEFINITIONS(-Wno-unknown-pragmas)
   SET(ENABLE_QT5 true) # Use Qt5
   MESSAGE("Compiling for OS X: LibWrap enabled, Qt5 enabled.")

   INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR}/src/qtwrapper/)

   IF(${VERBOSE_IPC} MATCHES true)
      MESSAGE("Adding more debug output")
      ADD_DEFINITIONS(-DVERBOSE_IPC=true)
   ENDIF()
ENDIF()

IF (${RING_FOUND} MATCHES "true")
   INCLUDE_DIRECTORIES(${ring_INCLUDE_DIRS})
ENDIF()

IF(NOT (${ENABLE_VIDEO} MATCHES false))
   MESSAGE("VIDEO enabled")
   SET(ENABLE_VIDEO 1 CACHE BOOLEAN "Enable video")
   add_definitions( -DENABLE_VIDEO=true )
ENDIF(NOT (${ENABLE_VIDEO} MATCHES false))

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5DBus)
# ADD_DEFINITIONS(-DQT_DISABLE_DEPRECATED_BEFORE=1)

SET(GENERIC_LIB_VERSION "0.2.0")

INCLUDE_DIRECTORIES(SYSTEM ${QT_INCLUDES} )
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES ( ${CMAKE_CURRENT_BINARY_DIR})

# Build dbus interfaces
SET ( dbus_xml_introspecs_path ${CMAKE_CURRENT_SOURCE_DIR}/xml/)

#File to compile
SET( libringclient_LIB_SRCS
  #Data objects
  src/call.cpp
  src/uri.cpp
  src/account.cpp
  src/person.cpp
  src/contactmethod.cpp
  src/numbercategory.cpp
  src/collectionextensioninterface.cpp
  src/video/rate.cpp
  src/video/device.cpp
  src/video/renderer.cpp
  src/certificate.cpp
  src/securityflaw.cpp

  #Models
  src/bootstrapmodel.cpp
  src/accountmodel.cpp
  src/availableaccountmodel.cpp
  src/callmodel.cpp
  src/categorizedhistorymodel.cpp
  src/categorizedbookmarkmodel.cpp
  src/credentialmodel.cpp
  src/instantmessagingmodel.cpp
  src/imconversationmanager.cpp
  src/categorizedcontactmodel.cpp
  src/useractionmodel.cpp
  src/presencestatusmodel.cpp
  src/phonedirectorymodel.cpp
  src/historytimecategorymodel.cpp
  src/numbercategorymodel.cpp
  src/keyexchangemodel.cpp
  src/tlsmethodmodel.cpp
  src/protocolmodel.cpp
  src/numbercompletionmodel.cpp
  src/profilemodel.cpp
  src/ringtonemodel.cpp
  src/lastusednumbermodel.cpp
  src/securityvalidationmodel.cpp
  src/personmodel.cpp
  src/collectionmodel.cpp
  src/collectionmanagerinterface.cpp
  src/networkinterfacemodel.cpp
  src/certificatemodel.cpp
  src/ciphermodel.cpp
  src/accountstatusmodel.cpp
  src/codecmodel.cpp
  src/video/devicemodel.cpp
  src/video/sourcemodel.cpp
  src/video/channel.cpp
  src/video/resolution.cpp
  src/video/configurationproxy.cpp
  src/audio/alsapluginmodel.cpp
  src/audio/inputdevicemodel.cpp
  src/audio/managermodel.cpp
  src/audio/outputdevicemodel.cpp
  src/audio/ringtonedevicemodel.cpp
  src/audio/settings.cpp


  #Data collections
  src/transitionalpersonbackend.cpp
  src/collectioninterface.cpp
  src/collectioneditor.cpp
  src/fallbackpersoncollection.cpp

  #Communication
  src/dbus/configurationmanager.cpp
  src/dbus/callmanager.cpp
  src/dbus/instancemanager.cpp
  src/dbus/videomanager.cpp
  src/dbus/presencemanager.cpp

  #Delegates
  src/delegates/accountlistcolordelegate.cpp
  src/delegates/phonenumberselectordelegate.cpp
  src/delegates/pixmapmanipulationdelegate.cpp
  src/delegates/presenceserializationdelegate.cpp
  src/delegates/itemmodelstateserializationdelegate.cpp
  src/delegates/profilepersisterdelegate.cpp
  src/delegates/certificateserializationdelegate.cpp

  #Other
  src/categorizedcompositenode.cpp
  src/hookmanager.cpp
  src/private/vcardutils.cpp
  src/private/videorenderermanager.cpp
  src/video/previewmanager.cpp

  #Extension
  src/extensions/presencecollectionextension.cpp
)

IF(${ENABLE_LIBWRAP} MATCHES true)
SET(libringclient_LIB_SRCS ${libringclient_LIB_SRCS}
  src/private/directrenderer.cpp
)
ELSE()
SET(libringclient_LIB_SRCS ${libringclient_LIB_SRCS}
  src/private/shmrenderer.cpp
)
ENDIF(${ENABLE_LIBWRAP} MATCHES true)

# Public API
SET( libringclient_LIB_HDRS
  src/account.h
  src/accountmodel.h
  src/availableaccountmodel.h
  src/call.h
  src/callmodel.h
  src/categorizedhistorymodel.h
  src/person.h
  src/bootstrapmodel.h
  src/collectioninterface.h
  src/collectioninterface.hpp
  src/categorizedbookmarkmodel.h
  src/credentialmodel.h
  src/instantmessagingmodel.h
  src/imconversationmanager.h
  src/categorizedcontactmodel.h
  src/useractionmodel.h
  src/presencestatusmodel.h
  src/contactmethod.h
  src/phonedirectorymodel.h
  src/historytimecategorymodel.h
  src/numbercategorymodel.h
  src/keyexchangemodel.h
  src/tlsmethodmodel.h
  src/protocolmodel.h
  src/numbercompletionmodel.h
  src/profilemodel.h
  src/numbercategory.h
  src/ringtonemodel.h
  src/lastusednumbermodel.h
  src/securityvalidationmodel.h
  src/certificate.h
  src/personmodel.h
  src/transitionalpersonbackend.h
  src/collectionmodel.h
  src/hookmanager.h
  src/uri.h
  src/itembase.h
  src/itembase.hpp
  src/mime.h
  src/categorizedcompositenode.h
  src/collectionextensioninterface.h
  src/collectionmanagerinterface.h
  src/collectionmanagerinterface.hpp
  src/networkinterfacemodel.h
  src/certificatemodel.h
  src/ciphermodel.h
  src/accountstatusmodel.h
  src/collectionmediator.h
  src/collectionmediator.hpp
  src/collectioneditor.h
  src/collectioneditor.hpp
  src/fallbackpersoncollection.h
  src/securityflaw.h
  src/collectioncreationinterface.h
  src/collectionconfigurationinterface.h

)

SET(libringclient_video_LIB_HDRS
  src/video/device.h
  src/video/devicemodel.h
  src/video/sourcemodel.h
  src/video/renderer.h
  src/video/resolution.h
  src/video/channel.h
  src/video/rate.h
  src/video/previewmanager.h
  src/video/configurationproxy.h
  #The renderer implementations are not exported on purpose
)

SET(libringclient_audio_LIB_HDRS
  src/audio/alsapluginmodel.h
  src/codecmodel.h
  src/audio/inputdevicemodel.h
  src/audio/managermodel.h
  src/audio/outputdevicemodel.h
  src/audio/ringtonedevicemodel.h
  src/audio/settings.h
)

SET(libringclient_extensions_LIB_HDRS
  src/extensions/presencecollectionextension.h
)

SET(libringclient_delegates_LIB_HDRS
  src/delegates/accountlistcolordelegate.h
  src/delegates/phonenumberselectordelegate.h
  src/delegates/presenceserializationdelegate.h
  src/delegates/itemmodelstateserializationdelegate.h
  src/delegates/pixmapmanipulationdelegate.h
  src/delegates/profilepersisterdelegate.h
  src/delegates/certificateserializationdelegate.h
)

SET( libringclient_extra_LIB_HDRS
  src/typedefs.h
)

IF(${ENABLE_LIBWRAP} MATCHES true)

ELSE()
   # presence manager interface
   SET ( presencemanager_xml  ${dbus_xml_introspecs_path}/presencemanager-introspec.xml )

   SET( dbus_metatype_path "${CMAKE_CURRENT_SOURCE_DIR}/src/dbus/metatypes.h")

   SET_SOURCE_FILES_PROPERTIES(
      ${presencemanager_xml}
      PROPERTIES
      CLASSNAME PresenceManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   QT5_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${presencemanager_xml}
      presencemanager_dbus_interface
   )

   # configuration manager interface
   SET ( configurationmanager_xml  ${dbus_xml_introspecs_path}/configurationmanager-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${configurationmanager_xml}
      PROPERTIES
      CLASSNAME ConfigurationManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   QT5_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${configurationmanager_xml}
      configurationmanager_dbus_interface
   )

   # call manager interface
   SET ( callmanager_xml  ${dbus_xml_introspecs_path}/callmanager-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${callmanager_xml}
      PROPERTIES
      CLASSNAME CallManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   QT5_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${callmanager_xml}
      callmanager_dbus_interface
   )

   # video manager interface
   SET ( video_xml  ${dbus_xml_introspecs_path}/videomanager-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${video_xml}
      PROPERTIES
      CLASSNAME VideoManagerInterface
      INCLUDE ${dbus_metatype_path}
   )

   QT5_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${video_xml}
      video_dbus_interface
   )


   # instance interface
   SET ( instance_xml  ${dbus_xml_introspecs_path}/instance-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${instance_xml}
      PROPERTIES
      CLASSNAME InstanceInterface
      INCLUDE ${dbus_metatype_path}
   )

   QT5_ADD_DBUS_INTERFACE(
      libringclient_LIB_SRCS
      ${instance_xml}
      instance_dbus_interface
   )

ENDIF(${ENABLE_LIBWRAP} MATCHES true)

# Manually wrap private files and interfaces
SET(libringclient_PRIVATE_HDRS
   src/private/call_p.h
   src/private/account_p.h
   src/private/accountmodel_p.h
   src/private/phonedirectorymodel_p.h
   src/private/instantmessagingmodel_p.h
   src/private/videorenderer_p.h
   src/private/videodevice_p.h
   src/private/collectionmodel_p.h
   src/private/securityflaw_p.h
   src/collectioncreationinterface.h
   src/collectionconfigurationinterface.h
)

IF(${ENABLE_LIBWRAP} MATCHES true)
   SET(libringclient_PRIVATE_HDRS
      ${libringclient_PRIVATE_HDRS}

      src/qtwrapper/callmanager_wrap.h
      src/qtwrapper/configurationmanager_wrap.h
      src/qtwrapper/instancemanager_wrap.h
      src/qtwrapper/presencemanager_wrap.h
      src/qtwrapper/videomanager_wrap.h
   )
ENDIF()

QT5_WRAP_CPP(LIB_HEADER_MOC ${libringclient_PRIVATE_HDRS})


ADD_LIBRARY( ringclient SHARED ${libringclient_LIB_SRCS} ${LIB_HEADER_MOC} )

IF(NOT ${ENABLE_STATIC} MATCHES false)
   ADD_LIBRARY( ringclient_static  STATIC ${libringclient_LIB_SRCS} ${LIB_HEADER_MOC} )
   QT5_USE_MODULES(ringclient_static Core)
   TARGET_LINK_LIBRARIES( ringclient_static
      -lpthread
      ${QT_QTDBUS_LIBRARY}
      ${QT_QTCORE_LIBRARY}
      )
   SET_TARGET_PROPERTIES( ringclient_static
      PROPERTIES VERSION ${GENERIC_LIB_VERSION}
   )
ENDIF()

QT5_USE_MODULES(ringclient Core)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  QT5_USE_MODULES(ringclient DBus)
  IF(NOT ${ENABLE_STATIC} MATCHES false)
      QT5_USE_MODULES(ringclient_static DBus)
  ENDIF()
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")


TARGET_LINK_LIBRARIES( ringclient
  -lpthread
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTCORE_LIBRARY}
)

IF(${ENABLE_LIBWRAP} MATCHES true)
   TARGET_LINK_LIBRARIES( ringclient
    qtwrapper
    ${ring_BIN}
   )
  IF(NOT ${ENABLE_STATIC} MATCHES false)
      TARGET_LINK_LIBRARIES( ringclient_static
      qtwrapper
      ${ring_BIN})
  ENDIF()
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  TARGET_LINK_LIBRARIES( ringclient
    -lrt)
  IF(NOT ${ENABLE_STATIC} MATCHES false)
      TARGET_LINK_LIBRARIES( ringclient_static
      -lrt)
  ENDIF()
ENDIF()

SET_TARGET_PROPERTIES( ringclient
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_VERSION}
)

SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)

INSTALL( FILES ${libringclient_LIB_HDRS} ${libringclient_extra_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_video_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/video
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_audio_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/audio
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_extensions_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/extensions
  COMPONENT Devel
)

INSTALL( FILES ${libringclient_delegates_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/delegates
  COMPONENT Devel
)

#This hack force Debian based system to return a non multi-arch path
#this is required to prevent the .deb libringclient.so from having an
#higher priority than the prefixed one.
STRING(REPLACE "${CMAKE_LIBRARY_ARCHITECTURE}" "" SANE_LIBRARY_PATH "${CMAKE_INSTALL_FULL_LIBDIR}" )

INSTALL( TARGETS ringclient
  ARCHIVE DESTINATION ${SANE_LIBRARY_PATH}
  LIBRARY DESTINATION ${SANE_LIBRARY_PATH}
  DESTINATION ${SANE_LIBRARY_PATH}
)

IF(NOT ${ENABLE_STATIC} MATCHES false)
   INSTALL( TARGETS ringclient_static
      ARCHIVE DESTINATION ${SANE_LIBRARY_PATH}
      LIBRARY DESTINATION ${SANE_LIBRARY_PATH}
      DESTINATION ${SANE_LIBRARY_PATH}
   )
ENDIF()


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
