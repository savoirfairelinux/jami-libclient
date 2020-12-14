win32-msvc {
    TARGET = Jami
    TEMPLATE = app

    QT += core winextras qml quickcontrols2 quick xml multimedia network webengine svg sql

    CONFIG += suppress_vcproj_warnings c++17 qtquickcompiler

    QTQUICK_COMPILER_SKIPPED_RESOURCES += resources.qrc

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
    QRENCODE= $$PWD/qrencode-win32/qrencode-win32

    # client deps
    INCLUDEPATH += $${QRENCODE}
    LIBS += $${QRENCODE}/vc8/qrcodelib/x64/Release-Lib/qrcodelib.lib

    # lrc
    INCLUDEPATH += $${LRC}/src/
    LIBS += $${LRC}/build/release/ringclient_static.lib
    LIBS += $${LRC}/build/src/qtwrapper/Release/qtwrapper.lib

    # daemon
    INCLUDEPATH += ../daemon/contrib/msvc/include/
    LIBS += $${DRING}/build/x64/ReleaseLib_win32/bin/dring.lib
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

    # run the deployment script(run windeployqt)
    QMAKE_POST_LINK += $$quote(python .\copy-runtime-files.py -o $${DESTDIR})
}

unix {
    TARGET = jami-qt
    TEMPLATE = app

    QT += quick quickwidgets widgets xml multimedia multimediawidgets network \
          webenginewidgets svg quickcontrols2 webengine webenginecore sql dbus

    # Maj/min versions can be checked(if needed) using:
    # equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 12) {}
    versionAtLeast(QT_VERSION, 5.12.0) {
        CONFIG += c++17
    } else {
        QMAKE_CXXFLAGS += -std=c++17
    }

    INCLUDEPATH += ../src

    isEmpty(LRC) {
        LRC=$$PWD/../install/lrc
        INCLUDEPATH += $${LRC}/include/libringclient
        LIBDIR = $${LRC}/lib
    } else {
        INCLUDEPATH += $${LRC}/src
        isEmpty(LRCBUILD) {
            LIBDIR = $${LRC}/build
        } else {
            LIBDIR = $${LRCBUILD}
        }
    }
    QMAKE_RPATHDIR += $${LIBDIR}

    LIBS += -L$${LIBDIR} -lringclient
    LIBS += -lqrencode
    LIBS += -lX11

    CONFIG += link_pkgconfig
    PKGCONFIG += libnm

    isEmpty(PREFIX) { PREFIX = /tmp/$${TARGET}/bin }
    target.path = $$PREFIX/bin
    INSTALLS += target

    packagesExist(libnm) {
        DEFINES += USE_LIBNM
    }

    # unix specific
    HEADERS += \
            src/dbuserrorhandler.h \
            src/xrectsel.h
    SOURCES += \
            src/dbuserrorhandler.cpp \
            src/xrectsel.c
}

# Input
HEADERS += \
        src/avatarimageprovider.h \
        src/moderatorlistmodel.h \
        src/networkmanager.h \
        src/smartlistmodel.h \
        src/updatemanager.h \
        src/utils.h \
        src/bannedlistmodel.h \
        src/version.h \
        src/accountlistmodel.h \
        src/runguard.h \
        src/lrcinstance.h \
        src/globalsystemtray.h \
        src/appsettingsmanager.h \
        src/webchathelpers.h \
        src/rendermanager.h \
        src/connectivitymonitor.h \
        src/jamiavatartheme.h \
        src/mainapplication.h \
        src/qrimageprovider.h \
        src/messagesadapter.h \
        src/accountadapter.h \
        src/tintedbuttonimageprovider.h \
        src/calladapter.h \
        src/conversationsadapter.h \
        src/distantrenderer.h \
        src/previewrenderer.h \
        src/qmladapterbase.h \
        src/avadapter.h \
        src/contactadapter.h \
        src/pluginadapter.h \
        src/settingsadapter.h \
        src/deviceitemlistmodel.h \
        src/pluginitemlistmodel.h \
        src/mediahandleritemlistmodel.h \
        src/preferenceitemlistmodel.h \
        src/mediacodeclistmodel.h \
        src/accountstomigratelistmodel.h \
        src/audioinputdevicemodel.h \
        src/videoinputdevicemodel.h \
        src/audiooutputdevicemodel.h \
        src/pluginlistpreferencemodel.h \
        src/videoformatfpsmodel.h \
        src/videoformatresolutionmodel.h \
        src/audiomanagerlistmodel.h \
        src/qmlregister.h \
        src/qtutils.h \
        src/utilsadapter.h

SOURCES += \
        src/bannedlistmodel.cpp \
        src/accountlistmodel.cpp \
        src/moderatorlistmodel.cpp \
        src/networkmanager.cpp \
        src/runguard.cpp \
        src/updatemanager.cpp \
        src/webchathelpers.cpp \
        src/main.cpp \
        src/smartlistmodel.cpp \
        src/utils.cpp \
        src/rendermanager.cpp \
        src/connectivitymonitor.cpp \
        src/mainapplication.cpp \
        src/messagesadapter.cpp \
        src/accountadapter.cpp \
        src/calladapter.cpp \
        src/conversationsadapter.cpp \
        src/distantrenderer.cpp \
        src/previewrenderer.cpp \
        src/avadapter.cpp \
        src/contactadapter.cpp \
        src/pluginadapter.cpp \
        src/settingsadapter.cpp \
        src/deviceitemlistmodel.cpp \
        src/pluginitemlistmodel.cpp \
        src/mediahandleritemlistmodel.cpp \
        src/preferenceitemlistmodel.cpp \
        src/mediacodeclistmodel.cpp \
        src/accountstomigratelistmodel.cpp \
        src/audioinputdevicemodel.cpp \
        src/videoinputdevicemodel.cpp \
        src/audiooutputdevicemodel.cpp \
        src/pluginlistpreferencemodel.cpp \
        src/videoformatfpsmodel.cpp \
        src/videoformatresolutionmodel.cpp \
        src/audiomanagerlistmodel.cpp \
        src/qmlregister.cpp \
        src/utilsadapter.cpp

RESOURCES += \
        resources.qrc \
        qml.qrc
