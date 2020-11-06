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

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(QML_RESOURCES ${CMAKE_SOURCE_DIR}/resources.qrc)
set(QML_RESOURCES_QML ${CMAKE_SOURCE_DIR}/qml.qrc)

# todo: check LRC exists?
set(LRC ${CMAKE_CURRENT_SOURCE_DIR}/../install/lrc)

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
    ${SRC_DIR}/mediahandleritemlistmodel.cpp
    ${SRC_DIR}/preferenceitemlistmodel.cpp
    ${SRC_DIR}/mediacodeclistmodel.cpp
    ${SRC_DIR}/accountstomigratelistmodel.cpp
    ${SRC_DIR}/audioinputdevicemodel.cpp
    ${SRC_DIR}/videoinputdevicemodel.cpp
    ${SRC_DIR}/audiooutputdevicemodel.cpp
    ${SRC_DIR}/pluginlistpreferencemodel.cpp
    ${SRC_DIR}/videoformatfpsmodel.cpp
    ${SRC_DIR}/videoformatresolutionmodel.cpp
    ${SRC_DIR}/audiomanagerlistmodel.cpp
    ${SRC_DIR}/qmlregister.cpp
    ${SRC_DIR}/utilsadapter.cpp)

set(COMMON_HEADERS
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
    ${SRC_DIR}/mediahandleritemlistmodel.h
    ${SRC_DIR}/preferenceitemlistmodel.h
    ${SRC_DIR}/mediacodeclistmodel.h
    ${SRC_DIR}/accountstomigratelistmodel.h
    ${SRC_DIR}/audioinputdevicemodel.h
    ${SRC_DIR}/videoinputdevicemodel.h
    ${SRC_DIR}/audiooutputdevicemodel.h
    ${SRC_DIR}/pluginlistpreferencemodel.h
    ${SRC_DIR}/videoformatfpsmodel.h
    ${SRC_DIR}/videoformatresolutionmodel.h
    ${SRC_DIR}/audiomanagerlistmodel.h
    ${SRC_DIR}/qmlregister.h
    ${SRC_DIR}/qtutils.h
    ${SRC_DIR}/utilsadapter.h)

find_package(Qt5 CONFIG REQUIRED
    Core
    Quick
    QuickWidgets
    Network
    Svg
    QuickControls2
    WebEngine
    DBus)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${SRC_DIR}
    ${LRC}/include/libringclient
    ${LRC}/include)

add_executable(${PROJECT_NAME}
    ${SRC_DIR}/main.cpp
    ${COMMON_HEADERS}
    ${COMMON_SOURCES}
    ${QML_RESOURCES}
    ${QML_RESOURCES_QML})

# alternative: link_directories(${LRC}/lib)
find_library(ringclient ringclient ${LRC}/lib NO_DEFAULT_PATH)
find_library(qrencode qrencode)

target_link_libraries(jami-qt
    ${QML_LIBS}
    ${ringclient}
    ${qrencode})

add_subdirectory(tests)
