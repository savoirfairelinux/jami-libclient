/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

#include "api/mediamodel.h"

// Lrc
#include "callbackshandler.h"
#include "dbus/videomanager.h"

namespace lrc
{

using namespace api;

class MediaModelPimpl: public QObject
{
public:
    MediaModelPimpl(const MediaModel& linked,
        const CallbacksHandler& callbacksHandler);
    ~MediaModelPimpl();

    const CallbacksHandler& callbacksHandler;
    const MediaModel& linked;

public Q_SLOTS:
};

MediaModel::MediaModel(const account::Info& owner,
    const CallbacksHandler& callbacksHandler)
: owner(owner)
, pimpl_(std::make_unique<MediaModelPimpl>(*this, callbacksHandler))
{
}

MediaModel::~MediaModel()
{
}

bool
MediaModel::getDecodingAccelerated() const
{
    bool result = VideoManager::instance().getDecodingAccelerated();
    return result;
}

void
MediaModel::setDecodingAccelerated(bool accelerate)
{
    VideoManager::instance().setDecodingAccelerated(accelerate);
}

std::string
MediaModel::getDefaultDeviceName() const
{
    QString name = VideoManager::instance().getDefaultDevice();
    return name.toStdString();
}

void
MediaModel::setDefaultDevice(const std::string& name)
{
    VideoManager::instance().setDefaultDevice(name.c_str());
}

video::Settings
MediaModel::getDeviceSettings(const std::string& name) const
{
    MapStringString settings = VideoManager::instance()
        .getSettings(name.c_str());
    if (settings["name"].toStdString() != name) {
        throw std::out_of_range("Device " + name + " not found");
    }
    video::Settings result;
    result.name = settings["name"].toStdString();
    result.channel = settings["channel"].toStdString();
    result.size = settings["size"].toStdString();
    result.rate = settings["rate"].toUInt();
    return result;
}

video::Capabilities
MediaModel::getDeviceCapabilities(const std::string& name) const
{
    QMap<QString, QMap<QString, QVector<QString>>> capabilites =
        VideoManager::instance().getCapabilities(name.c_str());
    video::Capabilities result;
    for (auto& channel : capabilites.toStdMap()) {
        std::map<video::Resolution, video::FrameratesList> channelCapabilities;
        for (auto& resToRates : channel.second.toStdMap()) {
            video::FrameratesList rates;
            QVectorIterator<QString> itRates(resToRates.second);
            while (itRates.hasNext()) {
                rates.emplace_back(itRates.next().toUInt());
            }
            channelCapabilities.insert(
                std::make_pair(resToRates.first.toStdString(), rates));
        }
        result.insert(
            std::make_pair(channel.first.toStdString(), channelCapabilities));
    }
    return result;
}

MediaModelPimpl::MediaModelPimpl(const MediaModel& linked,
    const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
{ }

MediaModelPimpl::~MediaModelPimpl()
{ }

} // namespace lrc

#include "api/moc_mediamodel.cpp"
