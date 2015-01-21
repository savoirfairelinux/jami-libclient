CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(POLICY CMP0022)
   CMAKE_POLICY(SET CMP0022 NEW)
ENDIF(POLICY CMP0022)

INCLUDE(GNUInstallDirs)
INCLUDE(CMakePackageConfigHelpers)
INCLUDE(GenerateExportHeader)

ADD_DEFINITIONS("-std=c++0x")

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
   INCLUDE_DIRECTORIES(${ring_INCLUDE_DIRS})
ENDIF()


IF(${ENABLE_VIDEO} MATCHES true)
   MESSAGE("VIDEO enabled")
   SET(ENABLE_VIDEO 1 CACHE BOOLEAN "Enable video")
   add_definitions( -DENABLE_VIDEO=true )
ENDIF(${ENABLE_VIDEO} MATCHES true)

IF(${ENABLE_QT5} MATCHES true)
   FIND_PACKAGE(Qt5Core REQUIRED)
   FIND_PACKAGE(Qt5DBus)
   ADD_DEFINITIONS(-DQT_DISABLE_DEPRECATED_BEFORE=0)
ELSE()
   FIND_PACKAGE ( Qt4  REQUIRED )
ENDIF(${ENABLE_QT5} MATCHES true)

SET(GENERIC_LIB_VERSION "1.4.1")

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
  src/contact.cpp
  src/phonenumber.cpp
  src/numbercategory.cpp
  src/abstractitembackendmodelextension.cpp
  src/video/rate.cpp
  src/video/device.cpp
  src/video/renderer.cpp
  src/video/codec.cpp

  #Models
  src/accountmodel.cpp
  src/callmodel.cpp
  src/historymodel.cpp
  src/bookmarkmodel.cpp
  src/credentialmodel.cpp
  src/instantmessagingmodel.cpp
  src/imconversationmanager.cpp
  src/contactproxymodel.cpp
  src/useractionmodel.cpp
  src/presencestatusmodel.cpp
  src/phonedirectorymodel.cpp
  src/historytimecategorymodel.cpp
  src/numbercategorymodel.cpp
  src/keyexchangemodel.cpp
  src/tlsmethodmodel.cpp
  src/numbercompletionmodel.cpp
  src/profilemodel.cpp
  src/ringtonemodel.cpp
  src/lastusednumbermodel.cpp
  src/securityvalidationmodel.cpp
  src/certificate.cpp
  src/contactmodel.cpp
  src/itembackendmodel.cpp
  src/networkinterfacemodel.cpp
  src/video/devicemodel.cpp
  src/video/sourcesmodel.cpp
  src/video/codecmodel2.cpp
  src/video/channel.cpp
  src/video/resolution.cpp
  src/video/manager.cpp
  src/audio/alsapluginmodel.cpp
  src/audio/codecmodel.cpp
  src/audio/inputdevicemodel.cpp
  src/audio/managermodel.cpp
  src/audio/outputdevicemodel.cpp
  src/audio/ringtonedevicemodel.cpp
  src/audio/settings.cpp


  #Data backends
  src/transitionalcontactbackend.cpp
  src/abstractitembackend.cpp

  #Communication
  src/dbus/configurationmanager.cpp
  src/dbus/callmanager.cpp
  src/dbus/instancemanager.cpp
  src/dbus/videomanager.cpp
  src/dbus/presencemanager.cpp

  #Visitors
  src/visitors/accountlistcolorvisitor.cpp
  src/visitors/phonenumberselector.cpp
  src/visitors/numbercategoryvisitor.cpp
  src/visitors/pixmapmanipulationvisitor.cpp
  src/visitors/presenceserializationvisitor.cpp
  src/visitors/itemmodelstateserializationvisitor.cpp
  src/visitors/profilepersistervisitor.cpp

  #Other
  src/categorizedcompositenode.cpp
  src/hookmanager.cpp

  #Extension
  src/extensions/presenceitembackendmodelextension.cpp
)

# Public API
SET( libringclient_LIB_HDRS
  src/account.h
  src/accountmodel.h
  src/call.h
  src/callmodel.h
  src/historymodel.h
  src/contact.h
  src/abstractitembackend.h
  src/bookmarkmodel.h
  src/credentialmodel.h
  src/instantmessagingmodel.h
  src/imconversationmanager.h
  src/contactproxymodel.h
  src/useractionmodel.h
  src/presencestatusmodel.h
  src/phonenumber.h
  src/phonedirectorymodel.h
  src/historytimecategorymodel.h
  src/numbercategorymodel.h
  src/keyexchangemodel.h
  src/tlsmethodmodel.h
  src/numbercompletionmodel.h
  src/profilemodel.h
  src/numbercategory.h
  src/ringtonemodel.h
  src/lastusednumbermodel.h
  src/securityvalidationmodel.h
  src/certificate.h
  src/contactmodel.h
  src/transitionalcontactbackend.h
  src/itembackendmodel.h
  src/hookmanager.h
  src/uri.h
  src/mime.h
  src/categorizedcompositenode.h
  src/abstractitembackendmodelextension.h
  src/commonbackendmanagerinterface.h
  src/networkinterfacemodel.h
)

SET(libringclient_video_LIB_HDRS
  src/video/device.h
  src/video/devicemodel.h
  src/video/sourcesmodel.h
  src/video/codec.h
  src/video/codecmodel2.h
  src/video/manager.h
  src/video/renderer.h
  src/video/resolution.h
  src/video/channel.h
  src/video/rate.h
)

SET(libringclient_audio_LIB_HDRS
  src/audio/alsapluginmodel.h
  src/audio/codecmodel.h
  src/audio/inputdevicemodel.h
  src/audio/managermodel.h
  src/audio/outputdevicemodel.h
  src/audio/ringtonedevicemodel.h
  src/audio/settings.h
)

SET(libringclient_extensions_LIB_HDRS
  src/extensions/presenceitembackendmodelextension.h
)

SET(libringclient_visitors_LIB_HDRS
  src/visitors/accountlistcolorvisitor.h
  src/visitors/phonenumberselector.h
  src/visitors/presenceserializationvisitor.h
  src/visitors/itemmodelstateserializationvisitor.h
  src/visitors/pixmapmanipulationvisitor.h
  src/visitors/numbercategoryvisitor.h
  src/visitors/profilepersistervisitor.h
)

SET( libringclient_extra_LIB_HDRS
  src/typedefs.h
)

# Include the headers for linking directly with libDring (used on platforms
# that don't have/use dbus.
IF(${ENABLE_LIBWRAP} MATCHES true)
   SET_SOURCE_FILES_PROPERTIES(
      src/qtwrapper/presencemanager_wrap.h
      PROPERTIES
      CLASSNAME PresenceManagerInterface
      INCLUDE "src/qtwrapper/metatypes.h")

   SET_SOURCE_FILES_PROPERTIES(
      src/qtwrapper/configurationmanager_wrap.h
      PROPERTIES
      CLASSNAME PresenceManagerInterface
      INCLUDE "src/qtwrapper/metatypes.h")
ELSE() # Use dbus to communicate with libDring

   # presence manager interface
   SET ( presencemanager_xml  ${dbus_xml_introspecs_path}/presencemanager-introspec.xml )

   SET( dbus_metatype_path "${CMAKE_CURRENT_SOURCE_DIR}/src/dbus/metatypes.h")

   SET_SOURCE_FILES_PROPERTIES(
      ${presencemanager_xml}
      PROPERTIES
      CLASSNAME PresenceManagerInterface
      INCLUDE ${dbus_metatype_path})

   IF(${ENABLE_QT5} MATCHES true)
      QT5_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${presencemanager_xml}
         presencemanager_dbus_interface
      )
   ELSE()
      QT4_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${presencemanager_xml}
         presencemanager_dbus_interface
      )

   ENDIF(${ENABLE_QT5} MATCHES true)

   # configuration manager interface
   SET ( configurationmanager_xml  ${dbus_xml_introspecs_path}/configurationmanager-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${configurationmanager_xml}
      PROPERTIES
      CLASSNAME ConfigurationManagerInterface
      INCLUDE ${dbus_metatype_path})

   IF(${ENABLE_QT5} MATCHES true)
      QT5_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${configurationmanager_xml}
         configurationmanager_dbus_interface
      )
   ELSE()
      QT4_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${configurationmanager_xml}
         configurationmanager_dbus_interface
      )

   ENDIF(${ENABLE_QT5} MATCHES true)

   # call manager interface
   SET ( callmanager_xml  ${dbus_xml_introspecs_path}/callmanager-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${callmanager_xml}
      PROPERTIES
      CLASSNAME CallManagerInterface
      INCLUDE ${dbus_metatype_path})

   IF(${ENABLE_QT5} MATCHES true)
      QT5_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${callmanager_xml}
         callmanager_dbus_interface
      )
   ELSE()
      QT4_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${callmanager_xml}
         callmanager_dbus_interface
      )
   ENDIF(${ENABLE_QT5} MATCHES true)


   # video manager interface
   SET ( video_xml  ${dbus_xml_introspecs_path}/videomanager-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${video_xml}
      PROPERTIES
      CLASSNAME VideoManagerInterface
      INCLUDE ${dbus_metatype_path})

   IF(${ENABLE_QT5} MATCHES true)
      QT5_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${video_xml}
         video_dbus_interface
      )
   ELSE()
      QT4_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${video_xml}
         video_dbus_interface
      )
   ENDIF(${ENABLE_QT5} MATCHES true)


   # instance interface
   SET ( instance_xml  ${dbus_xml_introspecs_path}/instance-introspec.xml )

   SET_SOURCE_FILES_PROPERTIES(
      ${instance_xml}
      PROPERTIES
      CLASSNAME InstanceInterface
      INCLUDE ${dbus_metatype_path})

   IF(${ENABLE_QT5} MATCHES true)
      QT5_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${instance_xml}
         instance_dbus_interface
      )
   ELSE()
      QT4_ADD_DBUS_INTERFACE(
         libringclient_LIB_SRCS
         ${instance_xml}
         instance_dbus_interface
      )
   ENDIF(${ENABLE_QT5} MATCHES true)

ENDIF(${ENABLE_LIBWRAP} MATCHES true)

# Manually wrap private files
SET(libringclient_PRIVATE_HDRS
   src/private/call_p.h
   src/private/phonedirectorymodel_p.h
   src/private/instantmessagingmodel_p.h
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

IF(${ENABLE_QT5} MATCHES true)
   QT5_WRAP_CPP(LIB_HEADER_MOC ${libringclient_PRIVATE_HDRS})
ELSE()
   QT4_WRAP_CPP(LIB_HEADER_MOC ${libringclient_PRIVATE_HDRS})
ENDIF(${ENABLE_QT5} MATCHES true)


ADD_LIBRARY( ringclient  SHARED ${libringclient_LIB_SRCS} ${LIB_HEADER_MOC} )

IF(NOT ${ENABLE_STATIC} MATCHES false)
   ADD_LIBRARY( ringclient_static  STATIC ${libringclient_LIB_SRCS} ${LIB_HEADER_MOC} )
ENDIF()

IF(${ENABLE_QT5} MATCHES true)
   QT5_USE_MODULES(ringclient Core)
   IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      QT5_USE_MODULES(ringclient DBus)
   ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
ENDIF(${ENABLE_QT5} MATCHES true)


TARGET_LINK_LIBRARIES( ringclient
  -lpthread
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTCORE_LIBRARY}
)

IF(${ENABLE_LIBWRAP} MATCHES true)
   TARGET_LINK_LIBRARIES( ringclient
    qtwrapper
    ${ring_BIN})
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   TARGET_LINK_LIBRARIES( ringclient
    -lrt)
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

INSTALL( FILES ${libringclient_visitors_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient/visitors
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
