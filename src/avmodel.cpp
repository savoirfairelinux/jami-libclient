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
#include <csignal>
#include <iomanip> // for std::put_time
#include <fstream>
#include <mutex>
#include <thread>
#include <string>
#include <sstream>
#include <iostream>

// Qt
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QUrl>

// Ring daemon
#include <media_const.h>

// LRC
#include "api/call.h"
#include "callbackshandler.h"
#include "dbus/callmanager.h"
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

    std::mutex renderers_mtx_;
    std::map<std::string, std::unique_ptr<video::Renderer>> renderers_;
    // store if a renderers is for a finished call
    std::map<std::string, bool> finishedRenders_;

#ifndef ENABLE_LIBWRAP
    // TODO: Init Video Renderers from daemon (see: https://git.ring.cx/savoirfairelinux/ring-daemon/issues/59)
    static void stopCameraAndQuit(int);
    static uint32_t SIZE_RENDERER;
#endif

public Q_SLOTS:
    void startedDecoding(const std::string& id, const std::string& shmPath, int width, int height);
    void stoppedDecoding(const std::string& id, const std::string& shmPath);
    /**
     * Listen from CallbacksHandler when a call got a new state
     * @param id
     * @param state the new state
     * @param code unused
     */
    void slotCallStateChanged(const std::string& id, const std::string &state, int code);

};

const std::string AVModelPimpl::recorderSavesSubdir = "sent_data";
#ifndef ENABLE_LIBWRAP
uint32_t AVModelPimpl::SIZE_RENDERER = 0;
#endif

AVModel::AVModel(const CallbacksHandler& callbacksHandler)
: QObject()
, pimpl_(std::make_unique<AVModelPimpl>(*this, callbacksHandler))
{
#ifndef ENABLE_LIBWRAP
    std::signal(SIGSEGV, AVModelPimpl::stopCameraAndQuit);
    std::signal(SIGINT, AVModelPimpl::stopCameraAndQuit);
#endif
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

    //If the preview is running, reload it
    //doing this during a call will cause re-invite, this is unwanted
    if (pimpl_->renderers_[video::PREVIEW_RENDERER_ID]) {
        if (pimpl_->renderers_[video::PREVIEW_RENDERER_ID]->isRendering() &&
            pimpl_->renderers_.size() == 1) {
            stopPreview();
            startPreview();
        }
    }
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

void
AVModel::stopLocalRecorder(const std::string& path) const
{
   if (path.empty()) {
      qWarning("stopLocalRecorder: can't stop non existing recording");
      return;
   }

   VideoManager::instance().stopLocalRecorder(QString::fromStdString(path));
}

std::string
AVModel::startLocalRecorder(const bool& audioOnly) const
{
   const QString path = QString::fromStdString(pimpl_->getRecordingPath());
   const QString finalPath = VideoManager::instance().startLocalRecorder(audioOnly, path);
   return finalPath.toStdString();
}

void
AVModel::startPreview()
{
    std::lock_guard<std::mutex> lk(pimpl_->renderers_mtx_);
    auto search = pimpl_->renderers_.find(video::PREVIEW_RENDERER_ID);
    if (search == pimpl_->renderers_.end()
        || !pimpl_->renderers_[video::PREVIEW_RENDERER_ID]) {
        qWarning() << "Can't find preview renderer!";
        return;
    }
    VideoManager::instance().startCamera();
    pimpl_->renderers_[video::PREVIEW_RENDERER_ID]->startRendering();
}

void
AVModel::stopPreview()
{
    std::lock_guard<std::mutex> lk(pimpl_->renderers_mtx_);
    auto search = pimpl_->renderers_.find(video::PREVIEW_RENDERER_ID);
    if (search == pimpl_->renderers_.end()
        || !pimpl_->renderers_[video::PREVIEW_RENDERER_ID]) {
        qWarning() << "Can't find preview renderer!";
        return;
    }
    VideoManager::instance().stopCamera();
    pimpl_->renderers_[video::PREVIEW_RENDERER_ID]->stopRendering();
}

const video::Renderer&
AVModel::getRenderer(const std::string& id) const
{
    std::lock_guard<std::mutex> lk(pimpl_->renderers_mtx_);
    auto search = pimpl_->renderers_.find(id);
    if (search == pimpl_->renderers_.end()
        || !pimpl_->renderers_[id]) {
        throw std::out_of_range("Can't find renderer " + id);
    }
    return *pimpl_->renderers_[id];
}

void
AVModel::setInputFile(const std::string& uri)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    VideoManager::instance().switchInput(
        !uri.empty() ? QString("%1%2%3")
                       .arg(DRing::Media::VideoProtocolPrefix::FILE)
                       .arg(sep)
                       .arg(QUrl(uri.c_str()).toLocalFile())
        : DRing::Media::VideoProtocolPrefix::NONE);
}

void
AVModel::setDisplay(int idx, int x, int y, int w, int h)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    VideoManager::instance().switchInput(QString("%1%2:%3+%4,%5 %6x%7")
       .arg(DRing::Media::VideoProtocolPrefix::DISPLAY)
       .arg(sep)
       .arg(idx)
       .arg(x)
       .arg(y)
       .arg(w)
       .arg(h));
}

void
AVModel::switchInputTo(const std::string& id)
{
    auto devices = getDevices();
    auto deviceAvailable = std::find(
        std::begin(devices), std::end(devices), id);
    if (deviceAvailable != devices.end()) {
        QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
        VideoManager::instance().switchInput(QString("%1%2%3")
            .arg(DRing::Media::VideoProtocolPrefix::CAMERA)
            .arg(sep)
            .arg(id.c_str()));
    } else {
        VideoManager::instance()
            .switchInput(DRing::Media::VideoProtocolPrefix::NONE);
    }
}

video::RenderedDevice
AVModel::getCurrentRenderedDevice(const std::string& call_id) const
{
    video::RenderedDevice result;
    MapStringString callDetails = CallManager::instance()
        .getCallDetails(call_id.c_str());
    if (!callDetails.contains("VIDEO_SOURCE")) {
        return result;
    }
    auto source = callDetails["VIDEO_SOURCE"];
    auto sourceSize = source.size();
    if (source.startsWith("camera://")) {
        result.type = video::DeviceType::CAMERA;
        result.name = source
            .right(sourceSize - std::string("camera://").size()).toStdString();
    } else if (source.startsWith("file://")) {
        result.type = video::DeviceType::FILE;
        result.name = source
            .right(sourceSize -std::string("file://").size()).toStdString();
    } else if (source.startsWith("display://")) {
        result.type = video::DeviceType::DISPLAY;
        result.name = source
            .right(sourceSize - std::string("display://").size()).toStdString();
    }
    return result;
}

AVModelPimpl::AVModelPimpl(AVModel& linked, const CallbacksHandler& callbacksHandler)
: linked_(linked)
, callbacksHandler(callbacksHandler)
{
    std::srand(std::time(nullptr));
    // add preview renderer
    renderers_.insert(std::make_pair(video::PREVIEW_RENDERER_ID,
        std::make_unique<video::Renderer>(video::PREVIEW_RENDERER_ID,
        linked_.getDeviceSettings(linked_.getDefaultDeviceName()))));
#ifndef ENABLE_LIBWRAP
    SIZE_RENDERER = renderers_.size();
#endif
    connect(&callbacksHandler, &CallbacksHandler::startedDecoding,
            this, &AVModelPimpl::startedDecoding);
    connect(&callbacksHandler, &CallbacksHandler::stoppedDecoding,
            this, &AVModelPimpl::stoppedDecoding);
    connect(&callbacksHandler, &CallbacksHandler::callStateChanged,
            this, &AVModelPimpl::slotCallStateChanged);
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

void
AVModelPimpl::startedDecoding(const std::string& id, const std::string& shmPath, int width, int height)
{
    const std::string res = std::to_string(width) + "x" + std::to_string(height);
    qDebug() << "startedDecoding for sink id: " << id.c_str();
    {
        std::lock_guard<std::mutex> lk(renderers_mtx_);
        auto search = renderers_.find(id);

        if (search == renderers_.end()) {
            video::Settings settings;
            settings.size = res;
            renderers_.insert(std::make_pair(id,
                std::make_unique<video::Renderer>(id.c_str(), settings)));
                finishedRenders_.insert(std::make_pair(id, false));
#ifndef ENABLE_LIBWRAP
            SIZE_RENDERER = renderers_.size();
#endif
            renderers_.at(id)->initThread();
        } else {
            (*search).second->update(res, shmPath);
        }
        renderers_.at(id)->startRendering();
    }
    emit linked_.rendererStarted(id);
}

void
AVModelPimpl::stoppedDecoding(const std::string& id, const std::string& shmPath)
{
    Q_UNUSED(shmPath)
    {
        std::lock_guard<std::mutex> lk(renderers_mtx_);
        auto search = renderers_.find(id);
        if (search == renderers_.end()) {
            qWarning() << "Cannot stop decoding, renderer " << id.c_str() << "not found";
            return; // nothing to do
        }

        (*search).second->stopRendering();
        qDebug() << "Video stopped for call" << id.c_str();
        (*search).second->quit();
        if (id != video::PREVIEW_RENDERER_ID) {
            auto searchFinished = finishedRenders_.find(id);
            if (searchFinished == finishedRenders_.end()) {
                qWarning() << "Finished flag: " << id.c_str() << " not found";
                return; // nothing to do
            }
            if (searchFinished->second) {
                renderers_.erase(id);
#ifndef ENABLE_LIBWRAP
                SIZE_RENDERER = renderers_.size();
#endif
                finishedRenders_.erase(id);
            }
        }
    }
    emit linked_.rendererStopped(id);
}

void
AVModelPimpl::slotCallStateChanged(const std::string& id, const std::string &state, int code)
{
    Q_UNUSED(code)
    if (call::to_status(state) != call::Status::ENDED)
        return;
    std::lock_guard<std::mutex> lk(renderers_mtx_);
    auto search = renderers_.find(id);
    auto searchFinished = finishedRenders_.find(id);
    if (search == renderers_.end()
        || searchFinished == finishedRenders_.end()) {
        qWarning() << "Renderer " << id.c_str() << "not found";
        return; // nothing to do
    }
    if (!(*search).second->isRendering()) {
        renderers_.erase(id);
#ifndef ENABLE_LIBWRAP
        SIZE_RENDERER = renderers_.size();
#endif
        finishedRenders_.erase(id);
    } else {
        finishedRenders_.at(id) = true;
    }
}

#ifndef ENABLE_LIBWRAP

void
AVModelPimpl::stopCameraAndQuit(int)
{
    if (SIZE_RENDERER == 1) {
        // This will stop the preview if needed (not in a call).
        VideoManager::instance().stopCamera();
        // HACK: this sleep is just here to let the camera stop and
        // avoid immediate raise
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::raise(SIGTERM);
}

#endif

} // namespace lrc

#include "api/moc_avmodel.cpp"
#include "avmodel.moc"
