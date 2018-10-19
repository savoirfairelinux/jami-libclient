/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
 *   Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>             *
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
#include "api/avmodel.h"

// Std
#include <chrono>
#include <iomanip> // for std::put_time
#include <fstream>
#include <string>
#include <sstream>

// Qt
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

// LRC
#include "dbus/configurationmanager.h"
#include "dbus/videomanager.h"

namespace lrc
{

using namespace api;

class AVModelPimpl: public QObject
{
    Q_OBJECT
public:
    AVModelPimpl(AVModel& linked, const CallbacksHandler& callbacksHandler);

    const CallbacksHandler& callbacksHandler;
    std::string getRecordingPath() const;
    static const std::string recorderSavesSubdir;
    AVModel& linked_;
};

const std::string AVModelPimpl::recorderSavesSubdir = "sent_data";

AVModel::AVModel(const CallbacksHandler& callbacksHandler)
: QObject()
, pimpl_(std::make_unique<AVModelPimpl>(*this, callbacksHandler))
{
}

AVModel::~AVModel()
{
}

bool
AVModel::getDecodingAccelerated() const
{
    bool result = VideoManager::instance().getDecodingAccelerated();
    return result;
}

void
AVModel::setDecodingAccelerated(bool accelerate)
{
    VideoManager::instance().setDecodingAccelerated(accelerate);
}

std::vector<std::string>
AVModel::getDevices() const
{
    QStringList devices = VideoManager::instance()
        .getDeviceList();
    std::vector<std::string> result;
    for (const auto& manager : devices) {
        result.emplace_back(manager.toStdString());
    }
    return result;
}

std::string
AVModel::getDefaultDeviceName() const
{
    QString name = VideoManager::instance().getDefaultDevice();
    return name.toStdString();
}

void
AVModel::setDefaultDevice(const std::string& name)
{
    VideoManager::instance().setDefaultDevice(name.c_str());
}

video::Settings
AVModel::getDeviceSettings(const std::string& name) const
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
AVModel::getDeviceCapabilities(const std::string& name) const
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

void
AVModel::setDeviceSettings(video::Settings& settings)
{
    MapStringString newSettings;
    newSettings["channel"] = settings.channel.c_str();
    newSettings["name"] = settings.name.c_str();
    newSettings["rate"] = QString::number(settings.rate);
    newSettings["size"] = settings.size.c_str();
    VideoManager::instance().applySettings(settings.name.c_str(), newSettings);
}

AVModelPimpl::AVModelPimpl(AVModel& linked, const CallbacksHandler& callbacksHandler)
: linked_(linked)
, callbacksHandler(callbacksHandler)
{
    std::srand(std::time(nullptr));
}

std::string
AVModelPimpl::getRecordingPath() const
{
    const QDir dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + recorderSavesSubdir.c_str();
    dir.mkpath(".");

    std::chrono::time_point<std::chrono::system_clock> time_now = std::chrono::system_clock::now();
    std::time_t time_now_t = std::chrono::system_clock::to_time_t(time_now);
    std::tm now_tm = *std::localtime(&time_now_t);

    std::stringstream ss;
    ss << dir.path().toStdString();
    ss << "/";
    ss << std::put_time(&now_tm, "%Y%m%d-%H%M%S");
    ss << "-";
    ss << std::rand();

    QDir file_path(ss.str().c_str());

    return file_path.path().toStdString();
}

std::vector<std::string>
AVModel::getSupportedAudioManagers() const
{
    QStringList managers = ConfigurationManager::instance()
        .getSupportedAudioManagers();
    std::vector<std::string> result;
    for (const auto& manager : managers) {
        result.emplace_back(manager.toStdString());
    }
    return result;
}

std::string
AVModel::getAudioManager() const
{
    QString manager = ConfigurationManager::instance().getAudioManager();
    return manager.toStdString();
}

std::vector<std::string>
AVModel::getAudioOutputDevices() const
{
    QStringList managers = ConfigurationManager::instance()
        .getAudioOutputDeviceList();
    std::vector<std::string> result;
    for (const auto& manager : managers) {
        result.emplace_back(manager.toStdString());
    }
    return result;
}

std::vector<std::string>
AVModel::getAudioInputDevices() const
{
    QStringList managers = ConfigurationManager::instance()
        .getAudioInputDeviceList();
    std::vector<std::string> result;
    for (const auto& manager : managers) {
        result.emplace_back(manager.toStdString());
    }
    return result;
}

std::string
AVModel::getRingtoneDevice() const
{
    const int RINGTONE_IDX = 2;
    auto outputDevices = getAudioOutputDevices();
    QStringList currentDevicesIdx = ConfigurationManager::instance()
        .getCurrentAudioDevicesIndex();
    if (currentDevicesIdx.size() < 3
    || outputDevices.size() != static_cast<size_t>(currentDevicesIdx.size())) {
        // Should not happen, but cannot retrieve current ringtone device
        return "";
    }
    return outputDevices[currentDevicesIdx[RINGTONE_IDX].toUInt()];
}

std::string
AVModel::getOutputDevice() const
{
    const int OUTPUT_IDX = 0;
    auto outputDevices = getAudioOutputDevices();
    QStringList currentDevicesIdx = ConfigurationManager::instance()
        .getCurrentAudioDevicesIndex();
    if (currentDevicesIdx.size() < 3
    || outputDevices.size() != static_cast<size_t>(currentDevicesIdx.size())) {
        // Should not happen, but cannot retrieve current ringtone device
        return "";
    }
    return outputDevices[currentDevicesIdx[OUTPUT_IDX].toUInt()];
}

std::string
AVModel::getInputDevice() const
{
    const int INPUT_IDX = 1;
    auto outputDevices = getAudioOutputDevices();
    QStringList currentDevicesIdx = ConfigurationManager::instance()
        .getCurrentAudioDevicesIndex();
    if (currentDevicesIdx.size() < 3
    || outputDevices.size() != static_cast<size_t>(currentDevicesIdx.size())) {
        // Should not happen, but cannot retrieve current ringtone device
        return "";
    }
    return outputDevices[currentDevicesIdx[INPUT_IDX].toUInt()];
}

bool
AVModel::setAudioManager(const std::string& name)
{
    return ConfigurationManager::instance().setAudioManager(name.c_str());
}

void
AVModel::setRingtoneDevice(const std::string& name)
{
    int idx = ConfigurationManager::instance()
        .getAudioOutputDeviceIndex(name.c_str());
    ConfigurationManager::instance().setAudioRingtoneDevice(idx);
}

void
AVModel::setOutputDevice(const std::string& name)
{
    int idx = ConfigurationManager::instance()
        .getAudioOutputDeviceIndex(name.c_str());
    ConfigurationManager::instance().setAudioOutputDevice(idx);
}

void
AVModel::setInputDevice(const std::string& name)
{
    int idx = ConfigurationManager::instance()
        .getAudioOutputDeviceIndex(name.c_str());
    ConfigurationManager::instance().setAudioInputDevice(idx);
}

void AVModel::stopLocalRecorder(const std::string& path) const
{
   if (path.empty()) {
      qDebug("stopLocalRecorder: can't stop non existing recording");
      return;
   }

   VideoManager::instance().stopLocalRecorder(QString::fromStdString(path));
}

std::string AVModel::startLocalRecorder(const bool& audioOnly) const
{
   const QString path = QString::fromStdString(pimpl_->getRecordingPath());
   const QString finalPath = VideoManager::instance().startLocalRecorder(audioOnly, path);
   return finalPath.toStdString();
}
} // namespace lrc

#include "api/moc_avmodel.cpp"
#include "avmodel.moc"
