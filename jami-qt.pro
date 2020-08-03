win32-msvc {
    TARGET = Jami
    TEMPLATE = vcapp

    QT += core winextras qml quickcontrols2 quick xml multimedia network webengine svg sql

    CONFIG += suppress_vcproj_warnings c++17 qtquickcompiler

    QTQUICK_COMPILER_SKIPPED_RESOURCES += ./ressources.qrc

    # compiler options
    QMAKE_CXXFLAGS += /wd"4068" /wd"4099" /wd"4189" /wd"4267" /wd"4577" /wd"4467" /wd"4715" /wd"4828"
    QMAKE_CXXFLAGS += /MP /GS /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /Zc:inline /fp:precise /errorReport:prompt
    QMAKE_CXXFLAGS += /Gd /Oi /MD /std:c++17 /FC /EHsc /nologo /sdl

    # linker options
    QMAKE_LFLAGS+= /ignore:4006,4049,4078,4098 /FORCE:MULTIPLE /INCREMENTAL:NO /Debug /LTCG /NODEFAULTLIB:LIBCMT

    # preprocessor defines
    DEFINES += UNICODE QT_NO_DEBUG NDEBUG

    # dependencies
    LRC= ../lrc
    DRING= ../daemon
    QRENCODE= qrencode-win32/qrencode-win32

    # client deps
    INCLUDEPATH += $${QRENCODE}
    LIBS += $${QRENCODE}/vc8/qrcodelib/x64/Release-Lib/qrcodelib.lib

    # lrc
    INCLUDEPATH += $${LRC}/src/
    LIBS += $${LRC}/msvc/release/ringclient_static.lib
    LIBS += $${LRC}/msvc/src/qtwrapper/Release/qtwrapper.lib

    # daemon
    INCLUDEPATH += ../daemon/contrib/msvc/include/
    LIBS += $${DRING}/build-local/x64/ReleaseLib_win32/bin/dring.lib
    LIBS += $${DRING}/contrib/msvc/lib/x64/libgnutls.lib

    # windows system libs
    LIBS += Shell32.lib Ole32.lib Advapi32.lib Shlwapi.lib User32.lib Gdi32.lib Crypt32.lib Strmiids.lib

    # output paths
    OBJECTS_DIR = obj/.obj
    MOC_DIR = obj/.moc
    RCC_DIR = obj/.rcc
    UI_DIR = obj/.ui

    # ReleaseCompile config
    contains(CONFIG, ReleaseCompile) {
        CONFIG(ReleaseCompile) {
            message(ReleaseCompile config enabled)
            Release: DEFINES += COMPILE_ONLY
        }
    }

    # beta config
    contains(CONFIG, Beta) {
        CONFIG(Beta) {
            message(Beta config enabled)
            Release: DESTDIR = x64/Beta
            Release: DEFINES += BETA
        }
    } else {
        Release: DESTDIR = x64/Release
    }
    Debug: DESTDIR = x64/Debug

    # qt dir
    QMAKE_INCDIR_QT=$(QTDIR)\include
    QMAKE_LIBDIR=$(QTDIR)\lib
    QMAKE_MOC=$(QTDIR)\bin\moc.exe
    QMAKE_QMAKE=$(QTDIR)\bin\qmake.exe

    # exe icons
    Release: RC_FILE = ico.rc
}

unix {
    TARGET = jami-qt
    TEMPLATE = app

    QT += quick quickwidgets widgets xml multimedia multimediawidgets network webenginewidgets svg quickcontrols2 webengine webenginecore sql dbus

    #check Qt version
    QT_VERSION = $$[QT_VERSION]
    QT_VERSION = $$split(QT_VERSION, ".")
    QT_VER_MAJ = $$member(QT_VERSION, 0)
    QT_VER_MIN = $$member(QT_VERSION, 1)

    lessThan(QT_VER_MIN, 12) {
        QMAKE_CXXFLAGS += -std=c++17
    }
    greaterThan(QT_VER_MIN, 12) | equals(QT_VER_MIN, 12) {
        CONFIG += c++17
    }

    isEmpty(LRC) { LRC=../../install/lrc/ }

    INCLUDEPATH += $${LRC}/include/libringclient
    INCLUDEPATH += $${LRC}/include
    INCLUDEPATH += ../src

    LIBS += -L$${LRC}/lib -lringclient
    LIBS += -lqrencode
}

# Input
HEADERS += ./src/smartlistmodel.h \
        ./src/utils.h \
        ./src/bannedlistmodel.h \
        ./src/version.h \
        ./src/accountlistmodel.h \
        ./src/runguard.h \
        ./src/lrcinstance.h \
        ./src/globalsystemtray.h \
        ./src/settingskey.h \
        ./src/webchathelpers.h \
        ./src/pixbufmanipulator.h \
        ./src/rendermanager.h \
        ./src/connectivitymonitor.h \
        ./src/jamiavatartheme.h \
        ./src/mainapplication.h \
        ./src/qrimageprovider.h \
        ./src/messagesadapter.h \
        ./src/accountadapter.h \
        ./src/tintedbuttonimageprovider.h \
        ./src/calladapter.h \
        ./src/conversationsadapter.h \
        ./src/distantrenderer.h \
        ./src/previewrenderer.h \
        ./src/qmladapterbase.h \
        ./src/avadapter.h \
        ./src/contactadapter.h \
        ./src/settingsadaptor.h \
        ./src/deviceitemlistmodel.h \
        ./src/pluginitemlistmodel.h \
        ./src/preferenceitemlistmodel.h \
        ./src/audiocodeclistmodel.h \
        ./src/videocodeclistmodel.h \
        ./src/accountstomigratelistmodel.h \
        ./src/clientwrapper.h \
        ./src/audioinputdevicemodel.h \
        ./src/videoinputdevicemodel.h \
        ./src/audiooutputdevicemodel.h \
        ./src/videoformatfpsmodel.h \
        ./src/videoformatresolutionmodel.h \
        ./src/audiomanagerlistmodel.h
SOURCES += ./src/bannedlistmodel.cpp \
        ./src/accountlistmodel.cpp \
        ./src/runguard.cpp \
        ./src/webchathelpers.cpp \
        ./src/main.cpp \
        ./src/globalsystemtray.cpp \
        ./src/smartlistmodel.cpp \
        ./src/utils.cpp \
        ./src/pixbufmanipulator.cpp \
        ./src/rendermanager.cpp \
        ./src/connectivitymonitor.cpp \
        ./src/mainapplication.cpp \
        ./src/messagesadapter.cpp \
        ./src/accountadapter.cpp \
        ./src/calladapter.cpp \
        ./src/conversationsadapter.cpp \
        ./src/distantrenderer.cpp \
        ./src/previewrenderer.cpp \
        ./src/qmladapterbase.cpp \
        ./src/avadapter.cpp \
        ./src/contactadapter.cpp \
        ./src/settingsadaptor.cpp \
        ./src/deviceitemlistmodel.cpp \
        ./src/pluginitemlistmodel.cpp \
        ./src/preferenceitemlistmodel.cpp \
        ./src/audiocodeclistmodel.cpp \
        ./src/videocodeclistmodel.cpp \
        ./src/accountstomigratelistmodel.cpp \
        ./src/clientwrapper.cpp \
        ./src/audioinputdevicemodel.cpp \
        ./src/videoinputdevicemodel.cpp \
        ./src/audiooutputdevicemodel.cpp \
        ./src/videoformatfpsmodel.cpp \
        ./src/videoformatresolutionmodel.cpp \
        ./src/audiomanagerlistmodel.cpp
RESOURCES += ./ressources.qrc \
             ./qml.qrc
