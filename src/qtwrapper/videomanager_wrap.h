/******************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                 *
 *   Author : Philippe Groarke <philippe.groarke@savoirfairelinux.com>        *
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
#include <QtDBus/QtDBus>

#include <sflphone.h>
#include "../dbus/metatypes.h"
#include "conversions_wrap.hpp"

/*
 * Proxy class for interface org.sflphone.SFLphone.VideoManager
 */
class VideoManagerInterface: public QObject
{
    Q_OBJECT

public:
    VideoManagerInterface()
    {
#ifdef ENABLE_VIDEO
        video_ev_handlers = {
            .on_device_event = [this] () { emit this->deviceEvent(); }
            .on_start_decoding = [this] (const std::string &id, const std::string &shmPath, int width, int height, bool isMixer) { emit this->startedDecoding(QString(id.c_str()), QString(shmPath.c_str()), width, height, isMixer); }
            .on_stop_decoding = [this] (const std::string &id, const std::string &shmPath, bool isMixer) { emit this->stoppedDecoding(QString(id.c_str()), QString(shmPath.c_str()), isMixer); }
        };
#endif
    }

    ~VideoManagerInterface() {}

#ifdef ENABLE_VIDEO
    sflph_video_ev_handlers video_ev_handlers;
#endif

public Q_SLOTS: // METHODS
    void applySettings(const QString &name, MapStringString settings)
    {
#ifdef ENABLE_VIDEO
        sflph_video_apply_settings(
            name.toStdString(), convertMap(settings));
#endif
    }

// TODO: test!!!!!!!!!!!!!!!
    MapStringMapStringVectorString getCapabilities(const QString &name)
    {
        MapStringMapStringVectorString ret;
#ifdef ENABLE_VIDEO
        std::map<std::string, std::map<std::string, std::vector<std::string>>> temp;
        temp = sflph_video_get_capabilities(name.toStdString());

        for (auto& x : temp) {
                map<QString, QStringList> ytemp;
            for (auto& y : x) {
                ytemp[QString(y.first.c_str())] = convertStringList(y.second);
            }
            ret[QString(x.first.c_str())] = ytemp;
        }
#endif
        return ret;
    }

    VectorMapStringString getCodecs(const QString &accountID)
    {
        VectorMapStringString temp;
#ifdef ENABLE_VIDEO
        for (auto x : sflph_video_get_codecs(accountID.toStdString())) {
            temp.push_back(convertMap(x));
        }
#endif
        return temp;
    }

    Q_DECL_DEPRECATED QString getCurrentCodecName(const QString &callID)
    {
#ifdef ENABLE_VIDEO
        QString temp(
            sflph_video_get_current_codec_name(callID.toStdString()).c_str());
#else
        QString temp;
#endif
        return temp;
    }

    QString getDefaultDevice()
    {
#ifdef ENABLE_VIDEO
        QString temp(
            sflph_video_get_default_device().c_str());
#else
        QString temp;
#endif
        return temp;
    }

    QStringList getDeviceList()
    {
#ifdef ENABLE_VIDEO
        QStringList temp =
            convertStringList(sflph_video_get_device_list());
#else
        QStringList temp;
#endif
        return temp;
    }

    MapStringString getSettings(const QString &device)
    {
#ifdef ENABLE_VIDEO
        MapStringString temp =
            convertMap(sflph_video_get_settings(device.toStdString()));
#else
        MapStringString temp;
#endif
        return temp;
    }

    bool hasCameraStarted()
    {
#ifdef ENABLE_VIDEO
        return sflph_video_has_camera_started();
#else
        return false;
#endif
    }

    void setCodecs(const QString &accountID, VectorMapStringString details)
    {
#ifdef ENABLE_VIDEO
        std::vector<std::map<std::string, std::string> > temp;
        for (auto x : details) {
            temp.push_back(convertMap(x));
        }
        sflph_video_set_codecs(accountID.toStdString(), temp);
#endif
    }

    void setDefaultDevice(const QString &name)
    {
#ifdef ENABLE_VIDEO
        sflph_video_set_default_device(name.toStdString());
#endif
    }

    void startCamera()
    {
#ifdef ENABLE_VIDEO
        sflph_video_start_camera();
#endif
    }

    void stopCamera()
    {
#ifdef ENABLE_VIDEO
        sflph_video_stop_camera();
#endif
    }

    bool switchInput(const QString &resource)
    {
#ifdef ENABLE_VIDEO
        return sflph_video_switch_input(resource.toStdString());
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
  namespace sflphone {
    namespace SFLphone {
      typedef ::VideoManagerInterface VideoManager;
    }
  }
}
#endif
