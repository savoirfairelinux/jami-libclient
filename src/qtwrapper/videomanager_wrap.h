/******************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                 *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>        *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#ifndef VIDEO_DBUS_INTERFACE_H
#define VIDEO_DBUS_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QTimer>

#include <videomanager_interface.h>

#include "typedefs.h"
#include "conversions_wrap.hpp"

/*
 * Proxy class for interface org.ring.Ring.VideoManager
 */
class VideoManagerInterface: public QObject
{
    Q_OBJECT

public:
    VideoManagerInterface()
    {
#ifdef ENABLE_VIDEO
        using DRing::exportable_callback;
        using DRing::VideoSignal;
        videoHandlers = {
            exportable_callback<VideoSignal::DeviceEvent>(
                [this] () {
                   QTimer::singleShot(0, [this] {
                       emit this->deviceEvent();
                   });
            }),
            exportable_callback<VideoSignal::DecodingStarted>(
                [this] (const std::string &id, const std::string &shmPath, int width, int height, bool isMixer) {
                   QTimer::singleShot(0, [this,id, shmPath, width, height, isMixer] {
                      emit this->startedDecoding(QString(id.c_str()), QString(shmPath.c_str()), width, height, isMixer);
                   });
            }),
            exportable_callback<VideoSignal::DecodingStopped>(
                [this] (const std::string &id, const std::string &shmPath, bool isMixer) {
                   QTimer::singleShot(0, [this,id, shmPath, isMixer] {
                      emit this->stoppedDecoding(QString(id.c_str()), QString(shmPath.c_str()), isMixer);
                   });
            })
        };
#endif
    }

    ~VideoManagerInterface() {}

#ifdef ENABLE_VIDEO
    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> videoHandlers;
#endif

public Q_SLOTS: // METHODS
    void applySettings(const QString &name, MapStringString settings)
    {
#ifdef ENABLE_VIDEO
        DRing::applySettings(
            name.toStdString(), convertMap(settings));
#endif
    }

// TODO: test!!!!!!!!!!!!!!!
    MapStringMapStringVectorString getCapabilities(const QString &name)
    {
        MapStringMapStringVectorString ret;
#ifdef ENABLE_VIDEO
        std::map<std::string, std::map<std::string, std::vector<std::string>>> temp;
        temp = DRing::getCapabilities(name.toStdString());

        for (auto& x : temp) {
                QMap<QString, VectorString> ytemp;
            for (auto& y : x.second) {
                ytemp[QString(y.first.c_str())] = convertVectorString(y.second);
            }
            ret[QString(x.first.c_str())] = ytemp;
        }
#endif
        return ret;
    }

    QString getDefaultDevice()
    {
#ifdef ENABLE_VIDEO
        QString temp(
            DRing::getDefaultDevice().c_str());
#else
        QString temp;
#endif
        return temp;
    }

    QStringList getDeviceList()
    {
#ifdef ENABLE_VIDEO
        QStringList temp =
            convertStringList(DRing::getDeviceList());
#else
        QStringList temp;
#endif
        return temp;
    }

    MapStringString getSettings(const QString &device)
    {
#ifdef ENABLE_VIDEO
        MapStringString temp =
            convertMap(DRing::getSettings(device.toStdString()));
#else
        MapStringString temp;
#endif
        return temp;
    }

    bool hasCameraStarted()
    {
#ifdef ENABLE_VIDEO
        return DRing::hasCameraStarted();
#else
        return false;
#endif
    }

    void setDefaultDevice(const QString &name)
    {
#ifdef ENABLE_VIDEO
        DRing::setDefaultDevice(name.toStdString());
#endif
    }

    void startCamera()
    {
#ifdef ENABLE_VIDEO
        DRing::startCamera();
#endif
    }

    void stopCamera()
    {
#ifdef ENABLE_VIDEO
        DRing::stopCamera();
#endif
    }

    bool switchInput(const QString &resource)
    {
#ifdef ENABLE_VIDEO
        return DRing::switchInput(resource.toStdString());
#else
        return false;
#endif
    }

Q_SIGNALS: // SIGNALS
    void deviceEvent();
    void startedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer);
    void stoppedDecoding(const QString &id, const QString &shmPath, bool isMixer);
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::VideoManagerInterface VideoManager;
    }
  }
}
#endif
