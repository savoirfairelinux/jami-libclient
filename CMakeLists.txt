cmake_minimum_required(VERSION 3.7)

project(jami-qt)

# todo: show error if Qt version < 5.14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}
    ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS})

set(QML_RESOURCES ${PROJECT_SOURCE_DIR}/resources.qrc)
set(QML_RESOURCES_QML ${PROJECT_SOURCE_DIR}/qml.qrc)

set(QML_LIBS
    Qt5::Quick
    Qt5::QuickWidgets
    Qt5::Network
    Qt5::Svg
    Qt5::QuickControls2
    Qt5::WebEngine
    Qt5::DBus
    Qt5::Core)

set(COMMON_SOURCES
    ${SRC_DIR}/bannedlistmodel.cpp
    ${SRC_DIR}/accountlistmodel.cpp
    ${SRC_DIR}/networkmanager.cpp
    ${SRC_DIR}/runguard.cpp
    ${SRC_DIR}/updatemanager.cpp
    ${SRC_DIR}/webchathelpers.cpp
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/smartlistmodel.cpp
    ${SRC_DIR}/utils.cpp
    ${SRC_DIR}/rendermanager.cpp
    ${SRC_DIR}/connectivitymonitor.cpp
    ${SRC_DIR}/mainapplication.cpp
    ${SRC_DIR}/messagesadapter.cpp
    ${SRC_DIR}/accountadapter.cpp
    ${SRC_DIR}/calladapter.cpp
    ${SRC_DIR}/conversationsadapter.cpp
    ${SRC_DIR}/distantrenderer.cpp
    ${SRC_DIR}/previewrenderer.cpp
    ${SRC_DIR}/avadapter.cpp
    ${SRC_DIR}/contactadapter.cpp
    ${SRC_DIR}/pluginadapter.cpp
    ${SRC_DIR}/settingsadapter.cpp
    ${SRC_DIR}/deviceitemlistmodel.cpp
    ${SRC_DIR}/pluginitemlistmodel.cpp
    ${SRC_DIR}/pluginhandleritemlistmodel.cpp
    ${SRC_DIR}/preferenceitemlistmodel.cpp
    ${SRC_DIR}/mediacodeclistmodel.cpp
    ${SRC_DIR}/accountstomigratelistmodel.cpp
    ${SRC_DIR}/audiodevicemodel.cpp
    ${SRC_DIR}/videoinputdevicemodel.cpp
    ${SRC_DIR}/pluginlistpreferencemodel.cpp
    ${SRC_DIR}/videoformatfpsmodel.cpp
    ${SRC_DIR}/videoformatresolutionmodel.cpp
    ${SRC_DIR}/audiomanagerlistmodel.cpp
    ${SRC_DIR}/qmlregister.cpp
    ${SRC_DIR}/utilsadapter.cpp
    ${SRC_DIR}/dbuserrorhandler.cpp
    ${SRC_DIR}/xrectsel.c
    ${SRC_DIR}/moderatorlistmodel.cpp
    ${SRC_DIR}/screensaver.cpp)

set(COMMON_HEADERS
    ${SRC_DIR}/avatarimageprovider.h
    ${SRC_DIR}/networkmanager.h
    ${SRC_DIR}/smartlistmodel.h
    ${SRC_DIR}/updatemanager.h
    ${SRC_DIR}/utils.h
    ${SRC_DIR}/bannedlistmodel.h
    ${SRC_DIR}/version.h
    ${SRC_DIR}/accountlistmodel.h
    ${SRC_DIR}/runguard.h
    ${SRC_DIR}/lrcinstance.h
    ${SRC_DIR}/globalsystemtray.h
    ${SRC_DIR}/appsettingsmanager.h
    ${SRC_DIR}/webchathelpers.h
    ${SRC_DIR}/rendermanager.h
    ${SRC_DIR}/connectivitymonitor.h
    ${SRC_DIR}/jamiavatartheme.h
    ${SRC_DIR}/mainapplication.h
    ${SRC_DIR}/qrimageprovider.h
    ${SRC_DIR}/messagesadapter.h
    ${SRC_DIR}/accountadapter.h
    ${SRC_DIR}/tintedbuttonimageprovider.h
    ${SRC_DIR}/calladapter.h
    ${SRC_DIR}/conversationsadapter.h
    ${SRC_DIR}/distantrenderer.h
    ${SRC_DIR}/previewrenderer.h
    ${SRC_DIR}/qmladapterbase.h
    ${SRC_DIR}/avadapter.h
    ${SRC_DIR}/contactadapter.h
    ${SRC_DIR}/pluginadapter.h
    ${SRC_DIR}/settingsadapter.h
    ${SRC_DIR}/deviceitemlistmodel.h
    ${SRC_DIR}/pluginitemlistmodel.h
    ${SRC_DIR}/pluginhandleritemlistmodel.h
    ${SRC_DIR}/preferenceitemlistmodel.h
    ${SRC_DIR}/mediacodeclistmodel.h
    ${SRC_DIR}/accountstomigratelistmodel.h
    ${SRC_DIR}/audiodevicemodel.h
    ${SRC_DIR}/videoinputdevicemodel.h
    ${SRC_DIR}/pluginlistpreferencemodel.h
    ${SRC_DIR}/videoformatfpsmodel.h
    ${SRC_DIR}/videoformatresolutionmodel.h
    ${SRC_DIR}/audiomanagerlistmodel.h
    ${SRC_DIR}/qmlregister.h
    ${SRC_DIR}/abstractitemmodelbase.h
    ${SRC_DIR}/quickimageproviderbase.h
    ${SRC_DIR}/qtutils.h
    ${SRC_DIR}/utilsadapter.h
    ${SRC_DIR}/dbuserrorhandler.h
    ${SRC_DIR}/xrectsel.h
    ${SRC_DIR}/moderatorlistmodel.h
    ${SRC_DIR}/screensaver.h)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM libnm)

if(LIBNM_FOUND)
    add_definitions(-DUSE_LIBNM)
endif()

if(QT5_VER AND QT5_PATH)
    string(REPLACE "." ";" VERSION_LIST ${QT5_VER})
    list(GET VERSION_LIST 0 QT5_VER_MAJOR)
    list(GET VERSION_LIST 1 QT5_VER_MINOR)
    list(GET VERSION_LIST 2 QT5_VER_PATCH)

    if((${QT5_VER_MAJOR} GREATER_EQUAL 5) AND (${QT5_VER_MINOR} GREATER_EQUAL 14))
        message(STATUS "Using custom Qt version")
        find_package(Qt5 ${QT5_VER} REQUIRED COMPONENTS
            Core
            Quick
            QuickWidgets
            Network
            Svg
            QuickControls2
            WebEngine
            DBus
            PATHS ${QT5_PATH} NO_DEFAULT_PATH)
        find_package(Qt5LinguistTools ${QT5_VER}
            PATHS ${QT5_PATH} NO_DEFAULT_PATH)
    endif()
else()
    find_package(Qt5 REQUIRED COMPONENTS
        Core
        Quick
        QuickWidgets
        Network
        Svg
        QuickControls2
        WebEngine
        DBus)
    find_package(Qt5LinguistTools)
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)

if(NOT DEFINED LRC)
    if(EXISTS ${PROJECT_SOURCE_DIR}/../install/lrc)
        set(LRC ${PROJECT_SOURCE_DIR}/../install/lrc)
    endif()
endif()

# If LRC is not globally installed, it's path should be specified
# by setting variable LRC. If library is not inside LRC + suffixes
# lib, build or build-local, it's path should be set with LRCLIB.
if(DEFINED LRC)
    if(EXISTS ${LRC}/include/libringclient)
        set(LRC_SRC_PATH ${LRC}/include/libringclient)
    else()
        set(LRC_SRC_PATH ${LRC}/src)
    endif()
    if(NOT DEFINED LRCLIB)
        set(LRCLIB ${LRC})
    endif()
    find_library(ringclient ringclient
        PATHS ${LRCLIB}
        PATH_SUFFIXES lib build build-local NO_DEFAULT_PATH)
    message("Will expect lrc library in ${LRCLIB} (including \
subdirs /lib, /build and /build-local)")
    set(LRC_LIB_NAME ${ringclient})
else()
    find_package(LibRingClient REQUIRED)
    if (LibRingClient_FOUND)
        set(LRC_SRC_PATH ${LIB_RING_CLIENT_INCLUDE_DIR})
        set(LRC_LIB_NAME ${LIB_RING_CLIENT_LIBRARY})
    else()
        message("lrc not found!")
    endif()
endif()

message("Will expect lrc headers in ${LRC_SRC_PATH}")

include_directories(${PROJECT_SOURCE_DIR}
    ${SRC_DIR}
    ${LRC_SRC_PATH}
    ${LIBNM_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
    ${SRC_DIR}/main.cpp
    ${COMMON_HEADERS}
    ${COMMON_SOURCES}
    ${QML_RESOURCES}
    ${QML_RESOURCES_QML}
    ${LRC_SRC_PATH}/webresource.qrc)

set(JAMI_DATA_PREFIX "${CMAKE_INSTALL_PREFIX}/share")

find_library(ringclient ringclient ${LRCLIBDIR} NO_DEFAULT_PATH)
find_library(qrencode qrencode)
find_library(X11 X11)

target_link_libraries(jami-qt
    ${QML_LIBS}
    ${LRC_LIB_NAME}
    ${qrencode}
    ${X11}
    ${LIBNM_LIBRARIES})

if(ENABLE_TESTS)
    message("Add Jami tests")
    add_subdirectory(tests)
endif()

# Installation rules
install(TARGETS jami-qt
    RUNTIME DESTINATION bin)

# install .desktop in XDG desktop dir so that it is recognized by the system
install(FILES ${PROJECT_SOURCE_DIR}/jami-qt.desktop
    DESTINATION ${JAMI_DATA_PREFIX}/applications)

# install .desktop in the jami-qt data dir, so that it can be copied to the
# autostart dir by the client
install(FILES ${PROJECT_SOURCE_DIR}/jami-qt.desktop
   DESTINATION
      "${JAMI_DATA_PREFIX}/${PROJECT_NAME}"
   PERMISSIONS
      WORLD_READ
      OWNER_WRITE
      OWNER_READ
      GROUP_READ
)

# adjust JAMI_DATA_PREFIX for snap package
#   (this must come after all 'install' commands that refer to
#    JAMI_DATA_PREFIX; the following value is not meant to be used for
#    any install destinations)
if(DEFINED ENV{SNAPCRAFT_PROJECT_NAME})
    set(JAMI_DATA_PREFIX "/snap/$ENV{SNAPCRAFT_PROJECT_NAME}/current/usr/share")
endif()

# (this must come after the above adjustment to JAMI_DATA_PREFIX)
target_compile_definitions(jami-qt PRIVATE JAMI_INSTALL_PREFIX="${JAMI_DATA_PREFIX}")

# logos
install(FILES images/jami.svg
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)

install(FILES images/jami-48px.png
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps
    RENAME jami.png)

install(FILES images/jami-32px.xpm
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps
    RENAME jami.xpm)

install(FILES jami-qt.appdata.xml
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/metainfo)

# install jami launcher that selects between clients
install(
    FILES
        "${PROJECT_SOURCE_DIR}/src/jami"
    DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
    PERMISSIONS
        WORLD_READ
        OWNER_WRITE
        OWNER_READ
        GROUP_READ
        OWNER_EXECUTE
        GROUP_EXECUTE
        WORLD_EXECUTE)

# add a target to generate API documentation with Doxygen and graphviz-dot
find_package(Doxygen)
find_program(dot "dot")
if(Doxygen_FOUND AND dot)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile @ONLY)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
        COMMENT "Generating API documentation with Doxygen" VERBATIM)

    # create doc/README.md symlink to README since Doxygen doesn't understand file with no extension
    add_custom_command(
        TARGET doc
        PRE_BUILD
        COMMAND ln -sf ${CMAKE_CURRENT_SOURCE_DIR}/README ${CMAKE_CURRENT_SOURCE_DIR}/doc/README.md)
endif()


# translations
if(Qt5LinguistTools_FOUND)
    message("Releasing and copying translation files")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/share/ring/translations/")
    file(GLOB TS_FILES ${PROJECT_SOURCE_DIR}/translations/*.ts)
    set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "share/ring/translations")
    qt5_add_translation(QM_FILES ${TS_FILES})
    add_custom_target(translations ALL DEPENDS ${QM_FILES})
    install(DIRECTORY "${CMAKE_BINARY_DIR}/share/ring/translations/"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ring/translations)
endif()

# uninstall
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
