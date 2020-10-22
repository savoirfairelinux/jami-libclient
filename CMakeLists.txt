cmake_minimum_required(VERSION 3.7)

project(jami-qt)

# todo: show error if Qt version < 5.14
# todo: move to common.cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-Og -ggdb")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(QML_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)
set(QML_RESOURCES_QML ${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc)

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
    src/bannedlistmodel.cpp
    src/accountlistmodel.cpp
    src/networkmanager.cpp
    src/runguard.cpp
    src/updatemanager.cpp
    src/webchathelpers.cpp
    src/main.cpp
    src/smartlistmodel.cpp
    src/utils.cpp
    src/pixbufmanipulator.cpp
    src/rendermanager.cpp
    src/connectivitymonitor.cpp
    src/mainapplication.cpp
    src/messagesadapter.cpp
    src/accountadapter.cpp
    src/calladapter.cpp
    src/conversationsadapter.cpp
    src/distantrenderer.cpp
    src/previewrenderer.cpp
    src/avadapter.cpp
    src/contactadapter.cpp
    src/pluginadapter.cpp
    src/settingsadapter.cpp
    src/deviceitemlistmodel.cpp
    src/pluginitemlistmodel.cpp
    src/mediahandleritemlistmodel.cpp
    src/preferenceitemlistmodel.cpp
    src/mediacodeclistmodel.cpp
    src/accountstomigratelistmodel.cpp
    src/audioinputdevicemodel.cpp
    src/videoinputdevicemodel.cpp
    src/audiooutputdevicemodel.cpp
    src/pluginlistpreferencemodel.cpp
    src/videoformatfpsmodel.cpp
    src/videoformatresolutionmodel.cpp
    src/audiomanagerlistmodel.cpp
    src/qmlregister.cpp
    src/utilsadapter.cpp)

set(COMMON_HEADERS
    src/networkmanager.h
    src/smartlistmodel.h
    src/updatemanager.h
    src/utils.h
    src/bannedlistmodel.h
    src/version.h
    src/accountlistmodel.h
    src/runguard.h
    src/lrcinstance.h
    src/globalsystemtray.h
    src/appsettingsmanager.h
    src/webchathelpers.h
    src/pixbufmanipulator.h
    src/rendermanager.h
    src/connectivitymonitor.h
    src/jamiavatartheme.h
    src/mainapplication.h
    src/qrimageprovider.h
    src/messagesadapter.h
    src/accountadapter.h
    src/tintedbuttonimageprovider.h
    src/calladapter.h
    src/conversationsadapter.h
    src/distantrenderer.h
    src/previewrenderer.h
    src/qmladapterbase.h
    src/avadapter.h
    src/contactadapter.h
    src/pluginadapter.h
    src/settingsadapter.h
    src/deviceitemlistmodel.h
    src/pluginitemlistmodel.h
    src/mediahandleritemlistmodel.h
    src/preferenceitemlistmodel.h
    src/mediacodeclistmodel.h
    src/accountstomigratelistmodel.h
    src/audioinputdevicemodel.h
    src/videoinputdevicemodel.h
    src/audiooutputdevicemodel.h
    src/pluginlistpreferencemodel.h
    src/videoformatfpsmodel.h
    src/videoformatresolutionmodel.h
    src/audiomanagerlistmodel.h
    src/qmlregister.h
    src/qtutils.h
    src/utilsadapter.h)

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
