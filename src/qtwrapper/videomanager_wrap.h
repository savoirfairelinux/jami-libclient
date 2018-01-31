/******************************************************************************
 *   Copyright (C) 2014-2018 Savoir-faire Linux                                 *
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
#pragma once

// libstdc++
#include <functional>

// Qt
#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QByteArray>
#include <QtCore/QThread>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QTimer>

// Ring
#include <videomanager_interface.h>

#include "typedefs.h"
#include "conversions_wrap.hpp"

class VideoManagerInterface;

class VideoManagerSignalProxy : public QObject
{
   Q_OBJECT
public:
   VideoManagerSignalProxy(VideoManagerInterface* parent);

public Q_SLOTS:
   void slotDeviceEvent();
   void slotStartedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer);
   void slotStoppedDecoding(const QString &id, const QString &shmPath, bool isMixer);

private:
   VideoManagerInterface* m_pParent;
};

class VideoManagerProxySender : public QObject
{
    Q_OBJECT
    friend class VideoManagerInterface;
public:

Q_SIGNALS:
    void deviceEvent();
    void startedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer);
    void stoppedDecoding(const QString &id, const QString &shmPath, bool isMixer);
};



/*
 * Proxy class for interface org.ring.Ring.VideoManager
 */
class VideoManagerInterface: public QObject
{
    Q_OBJECT

friend class VideoManagerSignalProxy;

public:

    VideoManagerInterface();
    ~VideoManagerInterface();

#ifdef ENABLE_VIDEO
     std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> videoHandlers;
#endif

private:
    VideoManagerSignalProxy* proxy;
    VideoManagerProxySender* sender;

public Q_SLOTS: // METHODS
    void applySettings(const QString &name, MapStringString settings)
    {
#ifdef ENABLE_VIDEO
        DRing::applySettings(
            name.toStdString(), convertMap(settings));
#else
        Q_UNUSED(name)
        Q_UNUSED(settings)
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
#else
        Q_UNUSED(name)
#endif
        return ret;
    }

    QString getDefaultDevice()
    {
#ifdef ENABLE_VIDEO
        return QString::fromStdString(DRing::getDefaultDevice().c_str());
#else
        return QString();
#endif
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
        Q_UNUSED(device)
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
#else
        Q_UNUSED(name)
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
        Q_UNUSED(resource)
        return false;
#endif
    }

    void registerSinkTarget(const QString &sinkID,
                            const DRing::SinkTarget& target)
    {
#ifdef ENABLE_VIDEO
        DRing::registerSinkTarget(sinkID.toStdString(), target);
#else
    Q_UNUSED(sinkID)
    Q_UNUSED(target)
#endif
    }

    bool getDecodingAccelerated()
    {
        return DRing::getDecodingAccelerated();
    }

    void setDecodingAccelerated(bool state)
    {
        DRing::setDecodingAccelerated(state);
    }

Q_SIGNALS: // SIGNALS
    void deviceEvent();
    void startedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer);
    void stoppedDecoding(const QString &id, const QString &shmPath, bool isMixer);
};

namespace org { namespace ring { namespace Ring {
      typedef ::VideoManagerInterface VideoManager;
}}} // namesapce org::ring::Ring
