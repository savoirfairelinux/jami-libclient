/******************************************************************************
 *    Copyright (C) 2014-2022 Savoir-faire Linux Inc.                         *
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

class VideoManagerInterface : public QObject
{
    Q_OBJECT

    friend class VideoManagerSignalProxy;

public:
    VideoManagerInterface();
    ~VideoManagerInterface();

#ifdef ENABLE_VIDEO
    std::map<std::string, std::shared_ptr<DRing::CallbackWrapperBase>> videoHandlers;
#endif

public Q_SLOTS: // METHODS
    void applySettings(const QString& name, MapStringString settings)
    {
#ifdef ENABLE_VIDEO
        DRing::applySettings(name.toStdString(), convertMap(settings));
#else
        Q_UNUSED(name)
        Q_UNUSED(settings)
#endif
    }

    MapStringMapStringVectorString getCapabilities(const QString& name)
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
        QStringList temp = convertStringList(DRing::getDeviceList());
#else
        QStringList temp;
#endif
        return temp;
    }

    MapStringString getSettings(const QString& device)
    {
#ifdef ENABLE_VIDEO
        MapStringString temp = convertMap(DRing::getSettings(device.toStdString()));
#else
        Q_UNUSED(device)
        MapStringString temp;
#endif
        return temp;
    }

    void setDefaultDevice(const QString& name)
    {
#ifdef ENABLE_VIDEO
        DRing::setDefaultDevice(name.toStdString());
#else
        Q_UNUSED(name)
#endif
    }

    QString openVideoInput(const QString& resource)
    {
#ifdef ENABLE_VIDEO
        return DRing::openVideoInput(resource.toStdString()).c_str();
#endif
    }

    void closeVideoInput(const QString& resource)
    {
#ifdef ENABLE_VIDEO
        DRing::closeVideoInput(resource.toStdString());
#endif
    }

    void startAudioDevice() { DRing::startAudioDevice(); }

    void stopAudioDevice() { DRing::stopAudioDevice(); }

    void registerSinkTarget(const QString& sinkID, const DRing::SinkTarget& target)
    {
#ifdef ENABLE_VIDEO
        DRing::registerSinkTarget(sinkID.toStdString(), target);
#else
        Q_UNUSED(sinkID)
        Q_UNUSED(target)
#endif
    }

    bool getDecodingAccelerated() { return DRing::getDecodingAccelerated(); }

    void setDecodingAccelerated(bool state) { DRing::setDecodingAccelerated(state); }

    bool getEncodingAccelerated() { return DRing::getEncodingAccelerated(); }

    void setEncodingAccelerated(bool state) { DRing::setEncodingAccelerated(state); }

    void stopLocalRecorder(const QString& path) { DRing::stopLocalRecorder(path.toStdString()); }

    QString startLocalMediaRecorder(const QString& videoInputId, const QString& path)
    {
        return QString::fromStdString(
            DRing::startLocalMediaRecorder(videoInputId.toStdString(), path.toStdString()));
    }

    MapStringString getRenderer(const QString& id)
    {
        return convertMap(DRing::getRenderer(id.toStdString()));
    }

Q_SIGNALS: // SIGNALS
    void deviceEvent();
    void decodingStarted(
        const QString& id, const QString& shmPath, int width, int height, bool isMixer);
    void decodingStopped(const QString& id, const QString& shmPath, bool isMixer);
};

namespace org {
namespace ring {
namespace Ring {
typedef ::VideoManagerInterface VideoManager;
}
} // namespace ring
} // namespace org
