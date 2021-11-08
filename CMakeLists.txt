cmake_minimum_required(VERSION 3.11)

if (APPLE)
    project(Jami)
else()
    project(jami-qt)
endif()

if(MSVC)
    cmake_minimum_required(VERSION 3.15)
endif()

# todo: show error if Qt version < 5.14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH true)

set(QML_RESOURCES ${PROJECT_SOURCE_DIR}/resources.qrc)
set(QML_RESOURCES_QML ${PROJECT_SOURCE_DIR}/qml.qrc)

if (APPLE)
    include(FetchContent)
    FetchContent_Declare(
        libqrencode
        GIT_REPOSITORY https://github.com/fukuchi/libqrencode.git
        GIT_TAG v4.1.1
    )
    FetchContent_Populate(libqrencode)
    add_subdirectory(${libqrencode_SOURCE_DIR} ${libqrencode_BINARY_DIR})
    include_directories(${libqrencode_SOURCE_DIR})
endif()

# Resource auto-gen
# check files in the resource directory and force a reconfigure if it changes
file(GLOB_RECURSE
    RES_FILES CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/resources/*
)
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.12.0")
    include(FindPython3)
    find_package (Python3 COMPONENTS Interpreter)
    set(PYTHON_EXEC ${Python3_EXECUTABLE})
else()
    include(FindPythonInterp)
    set(PYTHON_EXEC ${PYTHON_EXECUTABLE})
endif()
execute_process(
    COMMAND ${PYTHON_EXEC} ${PROJECT_SOURCE_DIR}/gen-resources.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# library compatibility (boost, libnotify, etc.)
add_definitions(-DQT_NO_KEYWORDS)

set(COMMON_SOURCES
    ${SRC_DIR}/bannedlistmodel.cpp
    ${SRC_DIR}/accountlistmodel.cpp
    ${SRC_DIR}/networkmanager.cpp
    ${SRC_DIR}/runguard.cpp
    ${SRC_DIR}/updatemanager.cpp
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/smartlistmodel.cpp
    ${SRC_DIR}/utils.cpp
    ${SRC_DIR}/rendermanager.cpp
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
    ${SRC_DIR}/deviceitemlistmodel.cpp
    ${SRC_DIR}/pluginitemlistmodel.cpp
    ${SRC_DIR}/pluginhandleritemlistmodel.cpp
    ${SRC_DIR}/preferenceitemlistmodel.cpp
    ${SRC_DIR}/mediacodeclistmodel.cpp
    ${SRC_DIR}/currentaccounttomigrate.cpp
    ${SRC_DIR}/audiodevicemodel.cpp
    ${SRC_DIR}/pluginlistpreferencemodel.cpp
    ${SRC_DIR}/audiomanagerlistmodel.cpp
    ${SRC_DIR}/qmlregister.cpp
    ${SRC_DIR}/utilsadapter.cpp
    ${SRC_DIR}/moderatorlistmodel.cpp
    ${SRC_DIR}/screensaver.cpp
    ${SRC_DIR}/systemtray.cpp
    ${SRC_DIR}/appsettingsmanager.cpp
    ${SRC_DIR}/lrcinstance.cpp
    ${SRC_DIR}/selectablelistproxymodel.cpp
    ${SRC_DIR}/conversationlistmodelbase.cpp
    ${SRC_DIR}/conversationlistmodel.cpp
    ${SRC_DIR}/searchresultslistmodel.cpp
    ${SRC_DIR}/calloverlaymodel.cpp
    ${SRC_DIR}/filestosendlistmodel.cpp
    ${SRC_DIR}/wizardviewstepmodel.cpp
    ${SRC_DIR}/avatarregistry.cpp
    ${SRC_DIR}/currentconversation.cpp
    ${SRC_DIR}/currentaccount.cpp
    ${SRC_DIR}/videodevices.cpp
    ${SRC_DIR}/previewengine.cpp)

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
    ${SRC_DIR}/rendermanager.h
    ${SRC_DIR}/connectivitymonitor.h
    ${SRC_DIR}/jamiavatartheme.h
    ${SRC_DIR}/mainapplication.h
    ${SRC_DIR}/qrimageprovider.h
    ${SRC_DIR}/messagesadapter.h
    ${SRC_DIR}/accountadapter.h
    ${SRC_DIR}/calladapter.h
    ${SRC_DIR}/conversationsadapter.h
    ${SRC_DIR}/distantrenderer.h
    ${SRC_DIR}/previewrenderer.h
    ${SRC_DIR}/qmladapterbase.h
    ${SRC_DIR}/avadapter.h
    ${SRC_DIR}/contactadapter.h
    ${SRC_DIR}/pluginadapter.h
    ${SRC_DIR}/deviceitemlistmodel.h
    ${SRC_DIR}/pluginitemlistmodel.h
    ${SRC_DIR}/pluginhandleritemlistmodel.h
    ${SRC_DIR}/preferenceitemlistmodel.h
    ${SRC_DIR}/mediacodeclistmodel.h
    ${SRC_DIR}/currentaccounttomigrate.h
    ${SRC_DIR}/audiodevicemodel.h
    ${SRC_DIR}/pluginlistpreferencemodel.h
    ${SRC_DIR}/audiomanagerlistmodel.h
    ${SRC_DIR}/qmlregister.h
    ${SRC_DIR}/abstractlistmodelbase.h
    ${SRC_DIR}/quickimageproviderbase.h
    ${SRC_DIR}/qtutils.h
    ${SRC_DIR}/utilsadapter.h
    ${SRC_DIR}/moderatorlistmodel.h
    ${SRC_DIR}/screensaver.h
    ${SRC_DIR}/systemtray.h
    ${SRC_DIR}/appsettingsmanager.h
    ${SRC_DIR}/lrcinstance.h
    ${SRC_DIR}/selectablelistproxymodel.h
    ${SRC_DIR}/conversationlistmodelbase.h
    ${SRC_DIR}/conversationlistmodel.h
    ${SRC_DIR}/searchresultslistmodel.h
    ${SRC_DIR}/calloverlaymodel.h
    ${SRC_DIR}/filestosendlistmodel.h
    ${SRC_DIR}/wizardviewstepmodel.h
    ${SRC_DIR}/avatarregistry.h
    ${SRC_DIR}/currentconversation.h
    ${SRC_DIR}/currentaccount.h
    ${SRC_DIR}/videodevices.h
    ${SRC_DIR}/previewengine.h)

set(QML_LIBS
    Qt::Quick
    Qt::Network
    Qt::NetworkAuth
    Qt::Svg
    Qt::Gui
    Qt::Qml
    Qt::QmlModels
    Qt::Positioning
    Qt::Sql
    Qt::Concurrent
    Qt::Core
    Qt::Core5Compat
    Qt::WebEngineCore
    Qt::WebEngineQuick
    Qt::WebChannel
    Qt::WebEngineWidgets)

set(QML_LIBS_LIST
    Quick
    Network
    NetworkAuth
    Svg
    Gui
    Qml
    QmlModels
    Positioning
    Sql
    Concurrent
    Core
    Core5Compat
    WebEngineCore
    WebEngineQuick
    WebChannel
    WebEngineWidgets)

set(WINDOWS_SYS_LIBS Shell32.lib
                     Ole32.lib
                     Advapi32.lib
                     Shlwapi.lib
                     User32.lib
                     Gdi32.lib
                     Crypt32.lib
                     Strmiids.lib)

# Here we let find_package(<PackageName>...) try to find Qt 6,
# If it is found, find_package will succeed, and the CMake variable
# QT_VERSION_MAJOR will be defined 6.
find_package(QT NAMES Qt6 COMPONENTS ${QML_LIBS_LIST} REQUIRED)
if (${QT_VERSION_MAJOR} STRLESS 6)
   message(FATAL_ERROR "We currently only support Qt6" )
endif()

if(MSVC)
    list(APPEND COMMON_SOURCES
                ${SRC_DIR}/connectivitymonitor.cpp)
    # preprocessor defines
    add_definitions(-DUNICODE -DQT_NO_DEBUG -DNDEBUG)

    # dependencies
    set(LRC ${PROJECT_SOURCE_DIR}/../lrc)
    set(DRING ${PROJECT_SOURCE_DIR}/../daemon)
    set(QRENCODE ${PROJECT_SOURCE_DIR}/qrencode-win32/qrencode-win32)

    # compiler options
    add_compile_options(/wd4068 /wd4099 /wd4189 /wd4267 /wd4577 /wd4467 /wd4715 /wd4828)
    add_compile_options(/MP /GS /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /Zc:inline /fp:precise /errorReport:prompt)
    add_compile_options(/Gd /Oi /MD /std:c++17 /FC /EHsc /nologo /sdl)

    # linker options
    add_link_options("/IGNORE:4006,4049,4078,4098" "/FORCE:MULTIPLE" "/INCREMENTAL:NO" "/DEBUG" "/LTCG" "/NODEFAULTLIB:LIBCMT")

    # client deps
    set(QRENCODE_LIB ${QRENCODE}/vc8/qrcodelib/x64/Release-Lib/qrcodelib.lib)

    # lrc
    set(LRC_SRC_PATH ${LRC}/src)
    set(RINGCLIENT_STATIC_LIB ${LRC}/build/release/ringclient_static.lib)
    set(QTWRAPPER_LIB ${LRC}/build/src/qtwrapper/Release/qtwrapper.lib)

    # daemon
    set(DRING_SRC_PATH ${DRING}/contrib/msvc/include)
    set(DRING_LIB ${DRING}/build/x64/ReleaseLib_win32/bin/jami.lib)
    set(GNUTLS_LIB ${DRING}/contrib/msvc/lib/x64/libgnutls.lib)

    # ReleaseCompile config
    if(ReleaseCompile)
        message("ReleaseCompile config enabled")
        add_definitions(-DCOMPILE_ONLY)
    else()
        # Beta config
        if(BETA)
            message("Beta config enabled")
            add_definitions(-DBETA)
            set(JAMI_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/x64/Beta)
        else()
            set(JAMI_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/x64/Release)
        endif()
    endif()

    include_directories(${DRING_SRC_PATH}
                        ${LRC_SRC_PATH}
                        ${QRENCODE})
elseif (NOT APPLE)
    list(APPEND COMMON_SOURCES
                ${SRC_DIR}/xrectsel.c
                ${SRC_DIR}/dbuserrorhandler.cpp
                ${SRC_DIR}/connectivitymonitor.cpp)
    list(APPEND COMMON_HEADERS
                ${SRC_DIR}/dbuserrorhandler.h
                ${SRC_DIR}/xrectsel.h)
    list(APPEND QML_LIBS Qt::DBus)
    list(APPEND QML_LIBS_LIST DBus)

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(GLIB REQUIRED glib-2.0)
    if(GLIB_FOUND)
        add_definitions(${GLIB_CFLAGS_OTHER})
    endif()

    pkg_check_modules(GIO REQUIRED gio-2.0)
    if(GIO_FOUND)
        add_definitions(${GIO_CFLAGS})
    endif()

    pkg_check_modules(LIBNM libnm)
    if(LIBNM_FOUND)
        add_definitions(-DUSE_LIBNM)
    endif()

    pkg_check_modules(LIBNOTIFY libnotify>=0.7.6)
    if(LIBNOTIFY_FOUND)
        add_definitions(-DUSE_LIBNOTIFY)
        add_definitions(${LIBNOTIFY_CFLAGS})
        pkg_check_modules(LIBGDKPIXBUF gdk-pixbuf-2.0>=2.40.0)
        add_definitions(${LIBGDKPIXBUF_CFLAGS})
    endif()

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

    include_directories(${LRC_SRC_PATH}
                        ${LIBNM_INCLUDE_DIRS}
                        ${LIBNOTIFY_INCLUDE_DIRS}
                        ${LIBGDKPIXBUF_INCLUDE_DIRS}
                        ${GLIB_INCLUDE_DIRS})

    set(JAMI_DATA_PREFIX "${CMAKE_INSTALL_PREFIX}/share")

    find_library(ringclient ringclient ${LRCLIBDIR} NO_DEFAULT_PATH)
    find_library(qrencode qrencode)
    find_library(X11 X11)
else()
    list(APPEND COMMON_SOURCES
                ${SRC_DIR}/connectivitymonitor.mm)
    find_package(PkgConfig REQUIRED)
    if(NOT DEFINED LRC)
        if(EXISTS ${PROJECT_SOURCE_DIR}/../install/lrc)
            set(LRC ${PROJECT_SOURCE_DIR}/../install/lrc)
        endif()
    endif()
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

    include_directories(${LRC_SRC_PATH})

    set(JAMI_DATA_PREFIX "${CMAKE_INSTALL_PREFIX}/share")

    find_library(ringclient ringclient ${LRCLIBDIR} NO_DEFAULT_PATH)
    find_library(SYSTEM_CONFIGURATUION SystemConfiguration)
    SET(myApp_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/images/jami.icns)
    SET_SOURCE_FILES_PROPERTIES(${myApp_ICON} PROPERTIES
           MACOSX_PACKAGE_LOCATION Resources)
endif()

# Qt find package
if(QT6_VER AND QT6_PATH)
    message(STATUS "Using custom Qt version")
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QML_LIBS_LIST}
                 PATHS ${QT6_PATH} NO_DEFAULT_PATH)
    # Linguist tools is not required.
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools
                 PATHS ${QT6_PATH} NO_DEFAULT_PATH)
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QML_LIBS_LIST})
    find_package(Qt${QT_VERSION_MAJOR}LinguistTools)
endif()

# common include
include_directories(${PROJECT_SOURCE_DIR}
                    ${SRC_DIR})

# common executable sources
qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION
                  ${SRC_DIR}/main.cpp
                  ${COMMON_HEADERS}
                  ${COMMON_SOURCES}
                  ${QML_RESOURCES}
                  ${QML_RESOURCES_QML}
                  ${LRC_SRC_PATH}/webresource.qrc)

if(MSVC)
    # Makes it a GUI executable instead of a console application
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

    target_link_libraries(${PROJECT_NAME} PRIVATE
                          ${QML_LIBS}
                          ${QRENCODE_LIB}
                          ${WINDOWS_SYS_LIBS})

    # specify output executable files
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${JAMI_OUTPUT_DIRECTORY_RELEASE}"
    )

    if(NOT DEFINED ReleaseCompile)
        # executable icon
        target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/ico.rc)

        target_link_libraries(${PROJECT_NAME} PRIVATE
                              ${RINGCLIENT_STATIC_LIB}
                              ${QTWRAPPER_LIB}
                              ${DRING_LIB}
                              ${GNUTLS_LIB})

        # translations
        if(Qt${QT_VERSION_MAJOR}LinguistTools_FOUND)
            message("Releasing and copying translation files")
            file(MAKE_DIRECTORY "${JAMI_OUTPUT_DIRECTORY_RELEASE}/share/ring/translations/")
            file(MAKE_DIRECTORY "${JAMI_OUTPUT_DIRECTORY_RELEASE}/share/libringclient/translations/")
            file(GLOB TS_CLIENT_FILES ${PROJECT_SOURCE_DIR}/translations/*.ts)
            file(GLOB TS_LRC_FILES ${LRC}/translations/*.ts)
            set_source_files_properties(${TS_CLIENT_FILES} PROPERTIES OUTPUT_LOCATION
                                        "${JAMI_OUTPUT_DIRECTORY_RELEASE}/share/ring/translations")
            set_source_files_properties(${TS_LRC_FILES} PROPERTIES OUTPUT_LOCATION
                                        "${JAMI_OUTPUT_DIRECTORY_RELEASE}/share/libringclient/translations")

            qt_add_translation(QM_CLIENT_FILES ${TS_CLIENT_FILES})
            qt_add_translation(QM_LRC_FILES ${TS_LRC_FILES})
            target_sources(${PROJECT_NAME} PRIVATE ${QM_CLIENT_FILES})
            target_sources(${PROJECT_NAME} PRIVATE ${QM_LRC_FILES})
        endif()

        # POST_BUILD steps

        # check time stamp
        set(TIME_STAMP_FILE ".deploy.stamp")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                           WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                                   -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/time_stamp_check.cmake)

        # copy runtime files and run windeployqt on target and deploy Qt libs
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                           WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                                   -DCOPY_TO_PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>
                                   -DDRING_PATH=${DRING}
                                   -DPROJECT_ROOT_DIR=${PROJECT_SOURCE_DIR}
                                   -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_daemon_deploy.cmake)

        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                           WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                                   -DWIN_DEPLOY_QT_PATH=${CMAKE_PREFIX_PATH}/bin
                                   -DQML_SRC_DIR=${SRC_DIR}
                                   -DEXE_NAME=$<TARGET_FILE:${PROJECT_NAME}>
                                   -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_qt_deploy.cmake)

        # create time stamp
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                           WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
                           COMMAND ${CMAKE_COMMAND} -DTIME_STAMP_FILE=${TIME_STAMP_FILE}
                                   -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/time_stamp_create.cmake)
    endif()

    # executable name
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "Jami")
elseif (NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
                          ${QML_LIBS}
                          ${LRC_LIB_NAME}
                          ${qrencode}
                          ${X11}
                          ${LIBNM_LIBRARIES}
                          ${LIBNOTIFY_LIBRARIES}
                          ${LIBGDKPIXBUF_LIBRARIES}
                          ${GLIB_LIBRARIES}
                          ${GIO_LIBRARIES})

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
    install(FILES resources/images/jami.svg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps)

    install(FILES resources/images/jami-48px.png
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/48x48/apps
        RENAME jami.png)

    install(FILES resources/images/jami-32px.xpm
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
    if(Qt${QT_VERSION_MAJOR}LinguistTools_FOUND)
        message("Releasing and copying translation files")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/share/ring/translations/")
        file(GLOB TS_FILES ${PROJECT_SOURCE_DIR}/translations/*.ts)
        set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "share/ring/translations")

        qt_add_translation(QM_FILES ${TS_FILES})
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
else()
    target_sources(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources/images/jami.icns)
    target_link_libraries(${PROJECT_NAME} PRIVATE
                          ${QML_LIBS}
                          ${LRC_LIB_NAME}
                          ${SYSTEM_CONFIGURATUION}
                          qrencode)

     # translations
     if(Qt${QT_VERSION_MAJOR}LinguistTools_FOUND)
         message("Releasing and copying translation files")
         file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/share/ring/translations/")
         file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/share/libringclient/translations/")
         file(GLOB TS_CLIENT_FILES ${PROJECT_SOURCE_DIR}/translations/*.ts)
         file(GLOB TS_LRC_FILES ${LRC}/translations/*.ts)
         set_source_files_properties(${TS_CLIENT_FILES} PROPERTIES OUTPUT_LOCATION
                                     "${CMAKE_BINARY_DIR}/share/ring/translations")
         set_source_files_properties(${TS_LRC_FILES} PROPERTIES OUTPUT_LOCATION
                                     "${CMAKE_BINARY_DIR}/share/libringclient/translations")

         qt_add_translation(QM_CLIENT_FILES ${TS_CLIENT_FILES})
         qt_add_translation(QM_LRC_FILES ${TS_LRC_FILES})
         target_sources(${PROJECT_NAME} PRIVATE ${QM_CLIENT_FILES})
         target_sources(${PROJECT_NAME} PRIVATE ${QM_LRC_FILES})
         install(DIRECTORY "${CMAKE_BINARY_DIR}/share/ring/translations/"
                            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ring/translations)
         install(DIRECTORY "${CMAKE_BINARY_DIR}/share/libringclient/translations/"
                            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/libringclient/translations)
      endif()
      set_target_properties(${PROJECT_NAME} PROPERTIES
                            MACOSX_BUNDLE TRUE
                            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist"
                            MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJ_NAME}"
                            MACOSX_BUNDLE_ICON_FILE "jami.icns"
                            MACOSX_BUNDLE_GUI_IDENTIFIER "${BUNDLE_ID}"
                            MACOSX_BUNDLE_BUNDLE_NAME "${PROJ_NAME}"
                            MACOSX_BUNDLE_SHORT_VERSION_STRING "${JAMI_VERSION}"
                            MACOSX_BUNDLE_BUNDLE_VERSION "${JAMI_BUILD}"
                            MACOSX_BUNDLE_COPYRIGHT "${PROJ_COPYRIGHT}"
                            XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/entitlements/Jami.entitlements"
                            XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME TRUE)
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                         COMMAND ${CMAKE_COMMAND} -DQML_SRC_DIR=${SRC_DIR}
                         -DMAC_DEPLOY_QT_PATH=${CMAKE_PREFIX_PATH}/bin
                         -DEXE_NAME="${CMAKE_BINARY_DIR}/${PROJECT_NAME}.app"
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos_qt_deploy.cmake)
endif()

qt_import_qml_plugins(${PROJECT_NAME})
qt_finalize_executable(${PROJECT_NAME})

# test
if(ENABLE_TESTS)
    message("Add Jami tests")
    add_subdirectory(tests)
endif()
