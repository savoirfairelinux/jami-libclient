/****************************************************************************
 *    Copyright (C) 2018-2019 Savoir-faire Linux Inc.                                  *
 *   Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

// Qt
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QUrl>

// Ring daemon
#include <media_const.h>

// LRC
#include "api/call.h"
#include "callbackshandler.h"
#include "daemonproxy.h"
#include "authority/storagehelper.h"

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
    std::map<std::string, bool> finishedRenderers_;
    bool useAVFrame_ = false;
    std::string currentVideoCaptureDevice_ {};

#ifndef DAEMON_INTERFACE_IS_LIBRARY
    // TODO: Init Video Renderers from daemon (see: https://git.jami.net/savoirfairelinux/ring-daemon/issues/59)
    static void stopCameraAndQuit(int);
    static uint32_t SIZE_RENDERER;
#endif

    /**
     * Get device via its type
     * @param type
     * @return the device name
     */
    std::string getDevice(int type) const;

public Q_SLOTS:
    /**
     * Listen from CallbacksHandler when a renderer starts
     * @param id
     * @param shmPath
     * @param width
     * @param height
     */
    void startedDecoding(const std::string& id, const std::string& shmPath, int width, int height);
    /**
     * Listen from CallbacksHandler when a renderer stops
     * @param id
     * @param shmPath
     */
    void stoppedDecoding(const std::string& id, const std::string& shmPath);
    /**
     * Listen from CallbacksHandler when a call got a new state
     * @param id
     * @param state the new state
     * @param code unused
     */
    void slotCallStateChanged(const std::string& id, const std::string &state, int code);
    /**
     * Detect when the current frame is updated
     * @param id
     */
    void slotFrameUpdated(const std::string& id);
    /**
     * Detect when a device is plugged or unplugged
     */
    void slotDeviceEvent();
    /**
     * Audio volume level
     * @param id Ringbuffer id
     * @param level Volume in range [0, 1]
     */
    void slotAudioMeter(const std::string& id, float level);

};

const std::string AVModelPimpl::recorderSavesSubdir = "sent_data";
#ifndef DAEMON_INTERFACE_IS_LIBRARY
uint32_t AVModelPimpl::SIZE_RENDERER = 0;
#endif

AVModel::AVModel(const CallbacksHandler& callbacksHandler)
: QObject()
, pimpl_(std::make_unique<AVModelPimpl>(*this, callbacksHandler))
{
#ifndef DAEMON_INTERFACE_IS_LIBRARY
    // Because the client uses DBUS, if a crash occurs, the daemon will not
    // be able to know it. So, stop the camera if the user was just previewing.
    std::signal(SIGSEGV, AVModelPimpl::stopCameraAndQuit);
    std::signal(SIGINT, AVModelPimpl::stopCameraAndQuit);
#endif
}

AVModel::~AVModel()
{
    std::lock_guard<std::mutex> lk(pimpl_->renderers_mtx_);
    for (auto r = pimpl_->renderers_.cbegin(); r != pimpl_->renderers_.cend(); ++r) {
        (*r).second->quit();
    }
}

bool
AVModel::getDecodingAccelerated() const
{
    return DaemonProxy::instance().getDecodingAccelerated();
}

void
AVModel::setDecodingAccelerated(bool accelerate)
{
    DaemonProxy::instance().setDecodingAccelerated(accelerate);
}

bool
AVModel::getEncodingAccelerated() const
{
    return DaemonProxy::instance().getEncodingAccelerated();
}

void
AVModel::setEncodingAccelerated(bool accelerate)
{
    DaemonProxy::instance().setEncodingAccelerated(accelerate);
}

bool
AVModel::getHardwareAcceleration() const
{
    bool result = getDecodingAccelerated() & getEncodingAccelerated();
    return result;
}
void
AVModel::setHardwareAcceleration(bool accelerate)
{
    setDecodingAccelerated(accelerate);
    setEncodingAccelerated(accelerate);
}

std::vector<std::string>
AVModel::getDevices() const
{
    return DaemonProxy::instance().getDeviceList();
}

std::string
AVModel::getDefaultDeviceName() const
{
    return DaemonProxy::instance().getDefaultDevice();
}

void
AVModel::setDefaultDevice(const std::string& name)
{
    DaemonProxy::instance().setDefaultDevice(name);
}

video::Settings
AVModel::getDeviceSettings(const std::string& name) const
{
    std::map<std::string, std::string> settings = DaemonProxy::instance().getSettings(name);

    if (settings["name"] != name) {
        throw std::out_of_range("Device " + name + " not found");
    }
    video::Settings result;
    result.name = settings["name"];
    result.channel = settings["channel"];
    result.size = settings["size"];
    try {
        result.rate = stof(settings["rate"]);
    } catch (std::invalid_argument& e) {
        result.rate = 0.0;
    }
    return result;
}

video::Capabilities
AVModel::getDeviceCapabilities(const std::string& name) const
{
    // Channel x Resolution x Framerate
    std::map<std::string, std::map<std::string, std::vector<std::string>>> capabilites =
        DaemonProxy::instance().getCapabilities(name);

    video::Capabilities result;
    for (auto& channel : capabilites) {
        video::ResRateList channelCapabilities;
        for (auto& resToRates : channel.second) {
            video::FrameratesList rates;
            for (auto& rate: resToRates.second) {
                rates.emplace_back(std::stof(rate));
            }
            channelCapabilities.emplace_back(
                std::make_pair(resToRates.first, rates));
        }
        // sort by resolution widths
        std::sort(channelCapabilities.begin(), channelCapabilities.end(),
            [](const std::pair<video::Resolution, video::FrameratesList>& lhs,
               const std::pair<video::Resolution, video::FrameratesList>& rhs) {
                auto lhsWidth = stoull(lhs.first.substr(0, lhs.first.find("x")));
                auto rhsWidth = stoull(rhs.first.substr(0, rhs.first.find("x")));
                return lhsWidth > rhsWidth;
            });
        result.insert(std::make_pair(channel.first, channelCapabilities));
    }
    return result;
}

void
AVModel::setDeviceSettings(video::Settings& settings)
{
    std::map<std::string, std::string> newSettings;
    newSettings["channel"] = settings.channel.c_str();
    newSettings["name"] = settings.name.c_str();
    newSettings["rate"] = std::to_string(settings.rate);
    newSettings["size"] = settings.size.c_str();
    DaemonProxy::instance().applySettings(settings.name.c_str(), newSettings);

    // If the preview is running, reload it
    // doing this during a call will cause re-invite, this is unwanted
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
    return DaemonProxy::instance().getSupportedAudioManagers();
}

std::string
AVModel::getAudioManager() const
{
    return DaemonProxy::instance().getAudioManager();
}

std::vector<std::string>
AVModel::getAudioOutputDevices() const
{
    std::vector<std::string> devices = DaemonProxy::instance().getAudioOutputDeviceList();

    // A fix for ring-daemon#43
    if (getAudioManager() == "pulseaudio") {
        if (devices.at(0) == "default") {
            devices[0] = QObject::tr("default").toStdString();
        }
    }

    return devices;
}

std::vector<std::string>
AVModel::getAudioInputDevices() const
{
    std::vector<std::string> devices = DaemonProxy::instance().getAudioInputDeviceList();

    // A fix for ring-daemon#43
    if (getAudioManager() == "pulseaudio") {
        if (devices.at(0) == "default") {
            devices[0] = QObject::tr("default").toStdString();
        }
    }

    return devices;
}

std::string
AVModel::getRingtoneDevice() const
{
    const int RINGTONE_IDX = 2;
    return pimpl_->getDevice(RINGTONE_IDX);
}

std::string
AVModel::getOutputDevice() const
{
    const int OUTPUT_IDX = 0;
    return pimpl_->getDevice(OUTPUT_IDX);
}

std::string
AVModel::getInputDevice() const
{
    const int INPUT_IDX = 1;
    return pimpl_->getDevice(INPUT_IDX);
}

bool
AVModel::isAudioMeterActive(const std::string& id) const
{
    return DaemonProxy::instance().isAudioMeterActive(id);
}

void
AVModel::setAudioMeterState(bool active, const std::string& id) const
{
    DaemonProxy::instance().setAudioMeterState(id, active);
}

void
AVModel::startAudioDevice() const
{
    DaemonProxy::instance().startAudioDevice();
}

void
AVModel::stopAudioDevice() const
{
    DaemonProxy::instance().stopAudioDevice();
}

bool
AVModel::setAudioManager(const std::string& name)
{
    return DaemonProxy::instance().setAudioManager(name);
}

void
AVModel::setRingtoneDevice(const std::string& name)
{
    int idx = DaemonProxy::instance().getAudioOutputDeviceIndex(name);
    DaemonProxy::instance().setAudioRingtoneDevice(idx);
}

void
AVModel::setOutputDevice(const std::string& name)
{
    int idx = DaemonProxy::instance().getAudioOutputDeviceIndex(name);
    DaemonProxy::instance().setAudioOutputDevice(idx);
}

void
AVModel::setInputDevice(const std::string& name)
{
    int idx = DaemonProxy::instance().getAudioInputDeviceIndex(name);
    DaemonProxy::instance().setAudioInputDevice(idx);
}

void
AVModel::stopLocalRecorder(const std::string& path) const
{
   if (path.empty()) {
      qWarning("stopLocalRecorder: can't stop non existing recording");
      return;
   }

   DaemonProxy::instance().stopLocalRecorder(path);
}

std::string
AVModel::startLocalRecorder(const bool& audioOnly) const
{
   const std::string path = pimpl_->getRecordingPath();
   return DaemonProxy::instance().startLocalRecorder(audioOnly, path);
}

std::string
AVModel::getRecordPath() const
{
    return DaemonProxy::instance().getRecordPath();
}

void
AVModel::setRecordPath(const std::string& path) const
{
    DaemonProxy::instance().setRecordPath(path);
}

bool
AVModel::getAlwaysRecord() const
{
    return DaemonProxy::instance().getIsAlwaysRecording();
}

void
AVModel::setAlwaysRecord(const bool& rec) const
{
    DaemonProxy::instance().setIsAlwaysRecording(rec);
}

bool
AVModel::getRecordPreview() const
{
    return DaemonProxy::instance().getRecordPreview();
}

void
AVModel::setRecordPreview(const bool& rec) const
{
    DaemonProxy::instance().setRecordPreview(rec);
}

int
AVModel::getRecordQuality() const
{
    return DaemonProxy::instance().getRecordQuality();
}

void
AVModel::setRecordQuality(const int& rec) const
{
    DaemonProxy::instance().setRecordQuality(rec);
}

void
AVModel::useAVFrame(bool useAVFrame) {
    pimpl_->useAVFrame_ = useAVFrame;
    for (auto it = pimpl_->renderers_.cbegin(); it != pimpl_->renderers_.cend(); ++it) {
        it->second->useAVFrame(pimpl_->useAVFrame_);
    }
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
    DaemonProxy::instance().startCamera();
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
    // If an active call does not have video muted, don't stop the camera
    // stopCamera() calls switchInput(""), which disables the camera
    bool previewShouldBeStopped = true;
    for (auto it = pimpl_->renderers_.cbegin(); it != pimpl_->renderers_.cend(); ++it) {
        if (it->second->getId() != video::PREVIEW_RENDERER_ID)
            // If rendering, don't stop preview
            previewShouldBeStopped &= !it->second->isRendering();
    }
    if (previewShouldBeStopped)
        DaemonProxy::instance().stopCamera();
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
    DaemonProxy::instance().switchInput(
        !uri.empty() ? QString("%1%2%3")
                       .arg(DRing::Media::VideoProtocolPrefix::FILE)
                       .arg(sep)
                       .arg(QUrl(uri.c_str()).toLocalFile()).toStdString()
        : DRing::Media::VideoProtocolPrefix::NONE);
}

void
AVModel::setDisplay(int idx, int x, int y, int w, int h)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    DaemonProxy::instance().switchInput(QString("%1%2:%3+%4,%5 %6x%7")
       .arg(DRing::Media::VideoProtocolPrefix::DISPLAY)
       .arg(sep)
       .arg(idx)
       .arg(x)
       .arg(y)
       .arg(w)
       .arg(h).toStdString());
}

void
AVModel::switchInputTo(const std::string& id)
{
    auto devices = getDevices();
    auto deviceAvailable = std::find(
        std::begin(devices), std::end(devices), id);
    if (deviceAvailable != devices.end()) {
        QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
        DaemonProxy::instance().switchInput(QString("%1%2%3")
            .arg(DRing::Media::VideoProtocolPrefix::CAMERA)
            .arg(sep)
            .arg(id.c_str()).toStdString());
    } else {
        DaemonProxy::instance()
            .switchInput(DRing::Media::VideoProtocolPrefix::NONE);
    }
}

video::RenderedDevice
AVModel::getCurrentRenderedDevice(const std::string& call_id) const
{
    video::RenderedDevice result;
    std::map<std::string, std::string> callDetails = DaemonProxy::instance().getCallDetails(call_id);
    if (callDetails.find("VIDEO_SOURCE") == callDetails.end()) {
        return result;
    }
    auto source = callDetails["VIDEO_SOURCE"];
    if (source.compare(0,9,"camera://") == 0) {
        result.type = video::DeviceType::CAMERA;
        result.name = source.substr(9);

    } else if (source.compare(0,7,"file://") == 0) {
        result.type = video::DeviceType::FILE;
        result.name = source.substr(7);

    } else if (source.compare(0,10,"display://") == 0) {
        result.type = video::DeviceType::DISPLAY;
        result.name = source.substr(10);
    }
    return result;
}

void
AVModel::setCurrentVideoCaptureDevice(std::string &currentVideoCaptureDevice)
{
    pimpl_->currentVideoCaptureDevice_ = currentVideoCaptureDevice;
}

std::string
AVModel::getCurrentVideoCaptureDevice() const
{
    return pimpl_->currentVideoCaptureDevice_;
}

void
AVModel::clearCurrentVideoCaptureDevice()
{
    pimpl_->currentVideoCaptureDevice_.clear();
}

AVModelPimpl::AVModelPimpl(AVModel& linked, const CallbacksHandler& callbacksHandler)
: linked_(linked)
, callbacksHandler(callbacksHandler)
{
    std::srand(std::time(nullptr));
    // add preview renderer
    renderers_.insert(std::make_pair(video::PREVIEW_RENDERER_ID,
                                     std::make_unique<video::Renderer>(video::PREVIEW_RENDERER_ID,
                                                                       linked_.getDeviceSettings(linked_.getDefaultDeviceName()),"", useAVFrame_)));
#ifndef DAEMON_INTERFACE_IS_LIBRARY
    SIZE_RENDERER = renderers_.size();
#endif
    connect(&callbacksHandler, &CallbacksHandler::deviceEvent,
            this, &AVModelPimpl::slotDeviceEvent, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::audioMeter,
            this, &AVModelPimpl::slotAudioMeter, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::startedDecoding,
            this, &AVModelPimpl::startedDecoding, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::stoppedDecoding,
            this, &AVModelPimpl::stoppedDecoding, Qt::QueuedConnection);
    connect(&callbacksHandler, &CallbacksHandler::callStateChanged,
            this, &AVModelPimpl::slotCallStateChanged, Qt::QueuedConnection);
    connect(&*renderers_[video::PREVIEW_RENDERER_ID], &api::video::Renderer::frameUpdated,
        this, &AVModelPimpl::slotFrameUpdated, Qt::QueuedConnection);

    auto startedPreview = false;
    auto restartRenderers = [&](const std::vector<std::string>& callList) {
        for (const auto& callId : callList)
        {
            std::map<std::string, std::string> rendererInfos = DaemonProxy::instance().getRenderer(callId);
            auto shmPath = rendererInfos[DRing::Media::Details::SHM_PATH];
            auto width = std::stoi(rendererInfos[DRing::Media::Details::WIDTH]);
            auto height = std::stoi(rendererInfos[DRing::Media::Details::HEIGHT]);
            if (width > 0 && height > 0) {
                startedPreview = true;
                startedDecoding(callId, shmPath, width, height);
            }
        }
    };
    restartRenderers(DaemonProxy::instance().getCallList());
    restartRenderers(DaemonProxy::instance().getConferenceList());
    if (startedPreview)
        restartRenderers({"local"});
}

std::string
AVModelPimpl::getRecordingPath() const
{
#if defined(_WIN32) || defined(__APPLE__)
    const QDir dir = QString::fromStdString(linked_.getRecordPath()) + "/" + recorderSavesSubdir.c_str();
#else
    const QDir dir = authority::storage::getPath() + "/" + recorderSavesSubdir.c_str();
#endif

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
    {
        std::lock_guard<std::mutex> lk(renderers_mtx_);
        auto search = renderers_.find(id);

        if (search == renderers_.end()) {
            video::Settings settings;
            settings.size = res;
            renderers_.insert(std::make_pair(id,
                std::make_unique<video::Renderer>(id.c_str(), settings, shmPath, useAVFrame_)));
            finishedRenderers_.insert(std::make_pair(id, false));
#ifndef DAEMON_INTERFACE_IS_LIBRARY
            SIZE_RENDERER = renderers_.size();
#endif
            renderers_.at(id)->initThread();
            connect(&*renderers_[id], &api::video::Renderer::frameUpdated,
                this, &AVModelPimpl::slotFrameUpdated);
        } else {
            (*search).second->update(res, shmPath);
        }
        renderers_.at(id)->startRendering();
    }
    Q_EMIT linked_.rendererStarted(id);
}

void
AVModelPimpl::stoppedDecoding(const std::string& id, const std::string& /*shmPath*/)
{
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
            auto searchFinished = finishedRenderers_.find(id);
            if (searchFinished == finishedRenderers_.end()) {
                qWarning() << "Finished flag: " << id.c_str() << " not found";
                return; // nothing to do
            }
            if (searchFinished->second) {
                disconnect(&*renderers_[id], &api::video::Renderer::frameUpdated,
                    this, &AVModelPimpl::slotFrameUpdated);
                renderers_.erase(id);
#ifndef DAEMON_INTERFACE_IS_LIBRARY
                SIZE_RENDERER = renderers_.size();
#endif
                finishedRenderers_.erase(id);
            }
        }
    }
    Q_EMIT linked_.rendererStopped(id);
}

void
AVModelPimpl::slotCallStateChanged(const std::string& id, const std::string &state, int code)
{
    Q_UNUSED(code)
    if (call::to_status(state) != call::Status::ENDED)
        return;
    std::lock_guard<std::mutex> lk(renderers_mtx_);
    auto search = renderers_.find(id);
    auto searchFinished = finishedRenderers_.find(id);
    if (search == renderers_.end()
        || searchFinished == finishedRenderers_.end()) {
        qWarning() << "Renderer " << id.c_str() << "not found";
        return; // nothing to do
    }
    if (!(*search).second->isRendering()) {
        disconnect(&*renderers_[id], &api::video::Renderer::frameUpdated,
            this, &AVModelPimpl::slotFrameUpdated);
        renderers_.erase(id);
#ifndef DAEMON_INTERFACE_IS_LIBRARY
        SIZE_RENDERER = renderers_.size();
#endif
        finishedRenderers_.erase(id);
    } else {
        finishedRenderers_.at(id) = true;
    }
}

#ifndef DAEMON_INTERFACE_IS_LIBRARY

void
AVModelPimpl::stopCameraAndQuit(int)
{
    if (SIZE_RENDERER == 1) {
        // This will stop the preview if needed (not in a call).
        DaemonProxy::instance().stopCamera();
        // HACK: this sleep is just here to let the camera stop and
        // avoid immediate raise
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::raise(SIGTERM);
}

#endif

std::string
AVModelPimpl::getDevice(int type) const
{
    if (type < 0 || type > 2) return {};  // No device
    std::string result = "";
    std::vector<std::string> devices;
    switch (type) {
        case 1: // INPUT
            devices = linked_.getAudioInputDevices();
            break;
        case 0: // OUTPUT
        case 2: // RINGTONE
            devices = linked_.getAudioOutputDevices();
            break;
        default:
            break;
    }
    std::vector<std::string> currentDevicesIdx = DaemonProxy::instance().getCurrentAudioDevicesIndex();
    try {
        if (currentDevicesIdx.size() < 3) {
            // Should not happen, but cannot retrieve current ringtone device
            return "";
        }
        auto deviceIdx = std::stoul(currentDevicesIdx[type]);
        for (const auto& dev : devices) {
            uint32_t idx;
            switch (type) {
            case 1: // INPUT
                idx = DaemonProxy::instance().getAudioInputDeviceIndex(dev);
                break;
            case 0: // OUTPUT
            case 2: // RINGTONE
                idx = DaemonProxy::instance().getAudioOutputDeviceIndex(dev);
                break;
            default:
                break;
            }
            if (idx == deviceIdx) {
                return dev;
            }
        }
        return "";
    } catch (std::bad_alloc& ba) {
        qWarning() << "bad_alloc caught: " << ba.what();
        return "";
    }
    return result;
}

void
AVModelPimpl::slotFrameUpdated(const std::string& id)
{
    Q_EMIT linked_.frameUpdated(id);
}

void
AVModelPimpl::slotDeviceEvent()
{
    Q_EMIT linked_.deviceEvent();
}

void
AVModelPimpl::slotAudioMeter(const std::string& id, float level)
{
    Q_EMIT linked_.audioMeter(id, level);
}

} // namespace lrc

#include "avmodel.moc"
