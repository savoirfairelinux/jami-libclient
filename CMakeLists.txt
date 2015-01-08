CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(POLICY CMP0022)
   CMAKE_POLICY(SET CMP0022 NEW)
ENDIF(POLICY CMP0022)

INCLUDE(GNUInstallDirs)

ADD_DEFINITIONS("-std=c++0x")

ADD_DEFINITIONS(
   ${QT_DEFINITIONS}
   -fexceptions
)

PROJECT(ringclient)

SET(CMAKE_AUTOMOC TRUE)

SET(LOCAL_CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
SET(CMAKE_MODULE_PATH "${LOCAL_CMAKE_MODULE_PATH}")

IF(NOT ${ENABLE_VIDEO} MATCHES false)
   MESSAGE("VIDEO enabled")
   SET(ENABLE_VIDEO 1 CACHE BOOLEAN "Enable video")
   ADD_DEFINITIONS( -DENABLE_VIDEO=true )
ENDIF(NOT ${ENABLE_VIDEO} MATCHES false)

IF(${ENABLE_QT5} MATCHES true)
   FIND_PACKAGE(Qt5Core)
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
  src/video/device.cpp
  src/video/renderer.cpp
  src/video/codec.cpp

  #Models
  src/accountmodel.cpp
  src/callmodel.cpp
  src/historymodel.cpp
  src/abstractbookmarkmodel.cpp
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
  src/categorizedaccountmodel.cpp
  src/ringtonemodel.cpp
  src/lastusednumbermodel.cpp
  src/securityvalidationmodel.cpp
  src/certificate.cpp
  src/contactmodel.cpp
  src/itembackendmodel.cpp
  src/video/devicemodel.cpp
  src/video/sourcesmodel.cpp
  src/video/codecmodel2.cpp
  src/video/channel.cpp
  src/video/resolution.cpp
  src/video/rate.cpp
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
  src/legacyhistorybackend.cpp
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

  #Other
  src/categorizedcompositenode.cpp
  src/hookmanager.cpp

  #Extension
  src/extensions/presenceitembackendmodelextension.cpp
)

SET( libringclient_LIB_HDRS
  src/account.h
  src/accountmodel.h
  src/call.h
  src/callmodel.h
  src/historymodel.h
  src/contact.h
  src/abstractitembackend.h
  src/abstractbookmarkmodel.h
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
  src/categorizedaccountmodel.h
  src/numbercategory.h
  src/ringtonemodel.h
  src/lastusednumbermodel.h
  src/securityvalidationmodel.h
  src/certificate.h
  src/contactmodel.h
  src/transitionalcontactbackend.h
  src/abstractitembackend.h
  src/itembackendmodel.h
  src/hookmanager.h
  src/uri.h
  src/mime.h
  src/abstractitembackendmodelextension.h
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
  src/audio/alsapluginmodel.h
  src/audio/codecmodel.h
  src/audio/inputdevicemodel.h
  src/audio/managermodel.h
  src/audio/outputdevicemodel.h
  src/audio/ringtonedevicemodel.h
  src/audio/settings.h

  extensions/presenceitembackendmodelextension.h
  #commonbackendmanagerinterface.h
)

SET( libringclient_extra_LIB_HDRS
  src/typedefs.h
  src/visitors/accountlistcolorvisitor.h
  src/visitors/phonenumberselector.h
  src/visitors/presenceserializationvisitor.h
  src/visitors/itemmodelstateserializationvisitor.h
)

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

# Manually wrap private files
SET(libringclient_PRIVATE_HDRS
   src/private/call_p.h
   src/private/phonedirectorymodel_p.h
   src/private/instantmessagingmodel_p.h
)

IF(${ENABLE_QT5} MATCHES true)
   QT5_WRAP_CPP(LIB_HEADER_MOC ${libringclient_PRIVATE_HDRS})
ELSE()
   QT4_WRAP_CPP(LIB_HEADER_MOC ${libringclient_PRIVATE_HDRS})
ENDIF(${ENABLE_QT5} MATCHES true)


ADD_LIBRARY( ringclient  SHARED ${libringclient_LIB_SRCS} ${LIB_HEADER_MOC} )
ADD_LIBRARY( ringclient_static  STATIC ${libringclient_LIB_SRCS} ${LIB_HEADER_MOC} )
# REMOVE_DEFINITIONS("-w")

IF(${ENABLE_QT5} MATCHES true)
   QT5_USE_MODULES(ringclient Core DBus)
ENDIF(${ENABLE_QT5} MATCHES true)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   SET(LINK_LIBRT "-lrt")
ENDIF()

TARGET_LINK_LIBRARIES( ringclient
  ${LINK_LIBRT}
  -lpthread
  ${QT_QTDBUS_LIBRARY}
  ${QT_QTCORE_LIBRARY}
)

SET_TARGET_PROPERTIES( ringclient
  PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_VERSION}
)

SET(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)

INSTALL( FILES ${libringclient_LIB_HDRS} ${libringclient_extra_LIB_HDRS}
  DESTINATION ${INCLUDE_INSTALL_DIR}/libringclient
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
