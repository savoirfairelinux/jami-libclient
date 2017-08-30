/******************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                                 *
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
#include "qtwrapper/conversions_wrap.hpp"

class VideoManagerInterface;

class VideoManagerSignalProxy : public QObject
{
   Q_OBJECT
public:
    VideoManagerSignalProxy(VideoManagerInterface* parent) {
        Q_UNUSED(parent)
    }

public Q_SLOTS:
    void slotDeviceEvent() {}
    void slotStartedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer) {
        Q_UNUSED(id)
        Q_UNUSED(shmPath)
        Q_UNUSED(width)
        Q_UNUSED(height)
        Q_UNUSED(isMixer)

    }

    void slotStoppedDecoding(const QString &id, const QString &shmPath, bool isMixer) {
        Q_UNUSED(id)
        Q_UNUSED(shmPath)
        Q_UNUSED(isMixer)

    }

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

    VideoManagerInterface() {}
    ~VideoManagerInterface() {}

private:
    VideoManagerSignalProxy* proxy;
    VideoManagerProxySender* sender;

public Q_SLOTS: // METHODS
    void applySettings(const QString &name, MapStringString settings)
    {
        Q_UNUSED(name)
        Q_UNUSED(settings)

    }

// TODO: test!!!!!!!!!!!!!!!
    MapStringMapStringVectorString getCapabilities(const QString &name)
    {
        Q_UNUSED(name)
        MapStringMapStringVectorString ret;
        return ret;
    }

    QString getDefaultDevice()
    {
        return QString();
    }

    QStringList getDeviceList()
    {
        QStringList temp;
        return temp;
    }

    MapStringString getSettings(const QString &device)
    {
        Q_UNUSED(device)
        MapStringString temp;
        return temp;
    }

    bool hasCameraStarted()
    {
        return false;
    }

    void setDefaultDevice(const QString &name)
    {
        Q_UNUSED(name)
    }

    void startCamera()
    {
    }

    void stopCamera()
    {
    }

    bool switchInput(const QString &resource)
    {
        Q_UNUSED(resource)
        return false;
    }

    void registerSinkTarget(const QString &sinkID,
                            const DRing::SinkTarget& target)
    {
        Q_UNUSED(sinkID)
        Q_UNUSED(target)
    }

    bool getDecodingAccelerated()
    {
        return false;
    }

    void setDecodingAccelerated(bool state)
    {

        Q_UNUSED(state)
    }

Q_SIGNALS: // SIGNALS
    void deviceEvent();
    void startedDecoding(const QString &id, const QString &shmPath, int width, int height, bool isMixer);
    void stoppedDecoding(const QString &id, const QString &shmPath, bool isMixer);
};

namespace org { namespace ring { namespace Ring {
      typedef ::VideoManagerInterface VideoManager;
}}} // namesapce org::ring::Ring
