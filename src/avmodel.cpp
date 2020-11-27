/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                       *
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
#include <algorithm> // std::sort
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
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/videomanager.h"
#include "authority/storagehelper.h"

namespace lrc {

using namespace api;

class AVModelPimpl : public QObject
{
    Q_OBJECT
public:
    AVModelPimpl(AVModel& linked, const CallbacksHandler& callbacksHandler);

    const CallbacksHandler& callbacksHandler;
    QString getRecordingPath() const;
    static const QString recorderSavesSubdir;
    AVModel& linked_;

    std::mutex renderers_mtx_;
    std::map<QString, std::unique_ptr<video::Renderer>> renderers_;
    // store if a renderers is for a finished call
    std::map<QString, bool> finishedRenderers_;
    bool useAVFrame_ = false;
    QString currentVideoCaptureDevice_ {};

#ifndef ENABLE_LIBWRAP
    // TODO: Init Video Renderers from daemon (see:
    // https://git.jami.net/savoirfairelinux/ring-daemon/issues/59)
    static void stopCameraAndQuit(int);
    static uint32_t SIZE_RENDERER;
#endif

    /**
     * Get device via its type
     * @param type
     * @return the device name
     */
    QString getDevice(int type) const;

public Q_SLOTS:
    /**
     * Listen from CallbacksHandler when a renderer starts
     * @param id
     * @param shmPath
     * @param width
     * @param height
     */
    void startedDecoding(const QString& id, const QString& shmPath, int width, int height);
    /**
     * Listen from CallbacksHandler when a renderer stops
     * @param id
     * @param shmPath
     */
    void stoppedDecoding(const QString& id, const QString& shmPath);
    /**
     * Listen from CallbacksHandler when a call got a new state
     * @param id
     * @param state the new state
     * @param code unused
     */
    void slotCallStateChanged(const QString& id, const QString& state, int code);
    /**
     * Detect when the current frame is updated
     * @param id
     */
    void slotFrameUpdated(const QString& id);
    /**
     * Detect when a device is plugged or unplugged
     */
    void slotDeviceEvent();
    /**
     * Audio volume level
     * @param id Ringbuffer id
     * @param level Volume in range [0, 1]
     */
    void slotAudioMeter(const QString& id, float level);
    /**
     * Listen from CallbacksHandler when a recorder stopped notice is incoming
     * @param filePath
     */
    void slotRecordPlaybackStopped(const QString& filePath);
};

const QString AVModelPimpl::recorderSavesSubdir = "sent_data";
#ifndef ENABLE_LIBWRAP
uint32_t AVModelPimpl::SIZE_RENDERER = 0;
#endif

AVModel::AVModel(const CallbacksHandler& callbacksHandler)
    : QObject(nullptr)
    , pimpl_(std::make_unique<AVModelPimpl>(*this, callbacksHandler))
{
#ifndef ENABLE_LIBWRAP
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
        if ((*r).second)
            (*r).second->quit();
    }
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

bool
AVModel::getEncodingAccelerated() const
{
    bool result = VideoManager::instance().getEncodingAccelerated();
    return result;
}

void
AVModel::setEncodingAccelerated(bool accelerate)
{
    VideoManager::instance().setEncodingAccelerated(accelerate);
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

QVector<QString>
AVModel::getDevices() const
{
    QStringList devices = VideoManager::instance().getDeviceList();
    VectorString result;
    for (const auto& device : devices) {
        result.push_back(device);
    }
    return (QVector<QString>) result;
}

QString
AVModel::getDefaultDevice() const
{
    return VideoManager::instance().getDefaultDevice();
}

void
AVModel::setDefaultDevice(const QString& deviceId)
{
    VideoManager::instance().setDefaultDevice(deviceId);
}

video::Settings
AVModel::getDeviceSettings(const QString& deviceId) const
{
    if (deviceId.isEmpty()) {
        return video::Settings();
    }
    MapStringString settings = VideoManager::instance().getSettings(deviceId);
    if (settings["id"] != deviceId) {
        throw std::out_of_range("Device '" + deviceId.toStdString() + "' not found");
    }
    video::Settings result;
    result.name = settings["name"];
    result.id = settings["id"];
    result.channel = settings["channel"];
    result.size = settings["size"];
    result.rate = settings["rate"].toFloat();
    return result;
}

video::Capabilities
AVModel::getDeviceCapabilities(const QString& deviceId) const
{
    // Channel x Resolution x Framerate
    QMap<QString, QMap<QString, QVector<QString>>> capabilites = VideoManager::instance()
                                                                     .getCapabilities(deviceId);
    video::Capabilities result;
    for (auto& channel : capabilites.toStdMap()) {
        video::ResRateList channelCapabilities;
        for (auto& resToRates : channel.second.toStdMap()) {
            video::FrameratesList rates;
            QVectorIterator<QString> itRates(resToRates.second);
            while (itRates.hasNext()) {
                rates.push_back(itRates.next().toFloat());
            }
            std::sort(rates.begin(), rates.end(), std::greater<int>());
            channelCapabilities.push_back(qMakePair(resToRates.first, rates));
        }
        // sort by resolution widths
        std::sort(channelCapabilities.begin(),
                  channelCapabilities.end(),
                  [](const QPair<video::Resolution, video::FrameratesList>& lhs,
                     const QPair<video::Resolution, video::FrameratesList>& rhs) {
                      auto lhsWidth = lhs.first.left(lhs.first.indexOf("x")).toLongLong();
                      auto rhsWidth = rhs.first.left(rhs.first.indexOf("x")).toLongLong();
                      return lhsWidth > rhsWidth;
                  });
        result.insert(channel.first, channelCapabilities);
    }
    return result;
}

void
AVModel::setDeviceSettings(video::Settings& settings)
{
    MapStringString newSettings;
    auto rate = QString::number(settings.rate, 'f', 7);
    rate = rate.left(rate.length() - 1);
    newSettings["channel"] = settings.channel;
    newSettings["name"] = settings.name;
    newSettings["id"] = settings.id;
    newSettings["rate"] = rate;
    newSettings["size"] = settings.size;
    VideoManager::instance().applySettings(settings.id, newSettings);

    // If the preview is running, reload it
    // doing this during a call will cause re-invite, this is unwanted
    if (pimpl_->renderers_[video::PREVIEW_RENDERER_ID]) {
        if (pimpl_->renderers_[video::PREVIEW_RENDERER_ID]->isRendering()
            && pimpl_->renderers_.size() == 1) {
            stopPreview();
            startPreview();
        }
    }
}

QString
AVModel::getDeviceIdFromName(const QString& deviceName) const
{
    auto devices = getDevices();
    auto iter = std::find_if(devices.begin(), devices.end(), [this, deviceName](const QString& d) {
        auto settings = getDeviceSettings(d);
        return settings.name == deviceName;
    });
    if (iter == devices.end()) {
        qWarning() << "Couldn't find device: " << deviceName;
        return {};
    }
    return *iter;
}

VectorString
AVModel::getSupportedAudioManagers() const
{
    QStringList managers = ConfigurationManager::instance().getSupportedAudioManagers();
    VectorString result;
    for (const auto& manager : managers) {
        result.push_back(manager);
    }
    return result;
}

QString
AVModel::getAudioManager() const
{
    return ConfigurationManager::instance().getAudioManager();
}

QVector<QString>
AVModel::getAudioOutputDevices() const
{
    QStringList devices = ConfigurationManager::instance().getAudioOutputDeviceList();

    // A fix for ring-daemon#43
    if (ConfigurationManager::instance().getAudioManager() == QStringLiteral("pulseaudio")) {
        if (devices.at(0) == QStringLiteral("default")) {
            devices[0] = QObject::tr("default");
        }
    }

    VectorString result;
    for (const auto& device : devices) {
        result.push_back(device);
    }
    return (QVector<QString>) result;
}

QVector<QString>
AVModel::getAudioInputDevices() const
{
    QStringList devices = ConfigurationManager::instance().getAudioInputDeviceList();

    // A fix for ring-daemon#43
    if (ConfigurationManager::instance().getAudioManager() == QStringLiteral("pulseaudio")) {
        if (devices.at(0) == QStringLiteral("default")) {
            devices[0] = QObject::tr("default");
        }
    }

    VectorString result;
    for (const auto& device : devices) {
        result.push_back(device);
    }
    return (QVector<QString>) result;
}

QString
AVModel::getRingtoneDevice() const
{
    const int RINGTONE_IDX = 2;
    return pimpl_->getDevice(RINGTONE_IDX);
}

QString
AVModel::getOutputDevice() const
{
    const int OUTPUT_IDX = 0;
    return pimpl_->getDevice(OUTPUT_IDX);
}

QString
AVModel::getInputDevice() const
{
    const int INPUT_IDX = 1;
    return pimpl_->getDevice(INPUT_IDX);
}

bool
AVModel::isAudioMeterActive(const QString& id) const
{
    return ConfigurationManager::instance().isAudioMeterActive(id);
}

void
AVModel::setAudioMeterState(bool active, const QString& id) const
{
    ConfigurationManager::instance().setAudioMeterState(id, active);
}

void
AVModel::startAudioDevice() const
{
    VideoManager::instance().startAudioDevice();
}

void
AVModel::stopAudioDevice() const
{
    VideoManager::instance().stopAudioDevice();
}

bool
AVModel::setAudioManager(const QString& name)
{
    return ConfigurationManager::instance().setAudioManager(name);
}

void
AVModel::setRingtoneDevice(const QString& name)
{
    int idx = ConfigurationManager::instance().getAudioOutputDeviceIndex(name);
    ConfigurationManager::instance().setAudioRingtoneDevice(idx);
}

void
AVModel::setOutputDevice(const QString& name)
{
    int idx = ConfigurationManager::instance().getAudioOutputDeviceIndex(name);
    ConfigurationManager::instance().setAudioOutputDevice(idx);
}

void
AVModel::setInputDevice(const QString& name)
{
    int idx = ConfigurationManager::instance().getAudioInputDeviceIndex(name);
    ConfigurationManager::instance().setAudioInputDevice(idx);
}

void
AVModel::stopLocalRecorder(const QString& path) const
{
    if (path.isEmpty()) {
        qWarning("stopLocalRecorder: can't stop non existing recording");
        return;
    }

    VideoManager::instance().stopLocalRecorder(path);
}

QString
AVModel::startLocalRecorder(const bool& audioOnly) const
{
    const QString path = pimpl_->getRecordingPath();
    const QString finalPath = VideoManager::instance().startLocalRecorder(audioOnly, path);
    return finalPath;
}

QString
AVModel::getRecordPath() const
{
    return ConfigurationManager::instance().getRecordPath();
}

void
AVModel::setRecordPath(const QString& path) const
{
    ConfigurationManager::instance().setRecordPath(path.toUtf8());
}

bool
AVModel::getAlwaysRecord() const
{
    return ConfigurationManager::instance().getIsAlwaysRecording();
}

void
AVModel::setAlwaysRecord(const bool& rec) const
{
    ConfigurationManager::instance().setIsAlwaysRecording(rec);
}

bool
AVModel::getRecordPreview() const
{
    return ConfigurationManager::instance().getRecordPreview();
}

void
AVModel::setRecordPreview(const bool& rec) const
{
    ConfigurationManager::instance().setRecordPreview(rec);
}

int
AVModel::getRecordQuality() const
{
    return ConfigurationManager::instance().getRecordQuality();
}

void
AVModel::setRecordQuality(const int& rec) const
{
    ConfigurationManager::instance().setRecordQuality(rec);
}

void
AVModel::useAVFrame(bool useAVFrame)
{
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
    if (search == pimpl_->renderers_.end() || !pimpl_->renderers_[video::PREVIEW_RENDERER_ID]) {
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
    if (search == pimpl_->renderers_.end() || !pimpl_->renderers_[video::PREVIEW_RENDERER_ID]) {
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
        VideoManager::instance().stopCamera();
    pimpl_->renderers_[video::PREVIEW_RENDERER_ID]->stopRendering();
}

const video::Renderer&
AVModel::getRenderer(const QString& id) const
{
    std::lock_guard<std::mutex> lk(pimpl_->renderers_mtx_);
    auto search = pimpl_->renderers_.find(id);
    if (search == pimpl_->renderers_.end() || !pimpl_->renderers_[id]) {
        throw std::out_of_range("Can't find renderer " + id.toStdString());
    }
    return *pimpl_->renderers_[id];
}

void
AVModel::setInputFile(const QString& uri, const QString& callId)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    auto resource = !uri.isEmpty() ? QString("%1%2%3")
                                         .arg(DRing::Media::VideoProtocolPrefix::FILE)
                                         .arg(sep)
                                         .arg(QUrl(uri).toLocalFile())
                                   : DRing::Media::VideoProtocolPrefix::NONE;
    if (callId.isEmpty()) {
        VideoManager::instance().switchInput(resource);
    } else {
        CallManager::instance().switchInput(callId, resource);
    }
}

void
AVModel::setDisplay(int idx, int x, int y, int w, int h, const QString& callId)
{
    QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
    auto resource = QString("%1%2:%3+%4,%5 %6x%7")
                        .arg(DRing::Media::VideoProtocolPrefix::DISPLAY)
                        .arg(sep)
                        .arg(idx)
                        .arg(x)
                        .arg(y)
                        .arg(w)
                        .arg(h);
    if (callId.isEmpty()) {
        VideoManager::instance().switchInput(resource);
    } else {
        CallManager::instance().switchInput(callId, resource);
    }
}

void
AVModel::switchInputTo(const QString& id, const QString& callId)
{
    QString resource;
    auto devices = getDevices();
    auto deviceAvailable = std::find(std::begin(devices), std::end(devices), id);
    if (deviceAvailable != devices.end()) {
        QString sep = DRing::Media::VideoProtocolPrefix::SEPARATOR;
        resource = QString("%1%2%3").arg(DRing::Media::VideoProtocolPrefix::CAMERA).arg(sep).arg(id);
    } else {
        resource = QString(DRing::Media::VideoProtocolPrefix::NONE);
    }
    if (callId.isEmpty()) {
        VideoManager::instance().switchInput(resource);
    } else {
        CallManager::instance().switchInput(callId, resource);
    }
}

video::RenderedDevice
AVModel::getCurrentRenderedDevice(const QString& call_id) const
{
    video::RenderedDevice result;
    MapStringString callDetails;
    QStringList conferences = CallManager::instance().getConferenceList();
    if (conferences.indexOf(call_id) != -1) {
        callDetails = CallManager::instance().getConferenceDetails(call_id);
    } else {
        callDetails = CallManager::instance().getCallDetails(call_id);
    }
    if (!callDetails.contains("VIDEO_SOURCE")) {
        return result;
    }
    auto source = callDetails["VIDEO_SOURCE"];
    auto sourceSize = source.size();
    if (source.startsWith("camera://")) {
        result.type = video::DeviceType::CAMERA;
        result.name = source.right(sourceSize - QString("camera://").size());
    } else if (source.startsWith("file://")) {
        result.type = video::DeviceType::FILE;
        result.name = source.right(sourceSize - QString("file://").size());
    } else if (source.startsWith("display://")) {
        result.type = video::DeviceType::DISPLAY;
        result.name = source.right(sourceSize - QString("display://").size());
    }
    return result;
}

void
AVModel::setCurrentVideoCaptureDevice(const QString& currentVideoCaptureDevice)
{
    pimpl_->currentVideoCaptureDevice_ = currentVideoCaptureDevice;
}

QString
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
    try {
        renderers_.insert(
            std::make_pair(video::PREVIEW_RENDERER_ID,
                           std::make_unique<video::Renderer>(video::PREVIEW_RENDERER_ID,
                                                             linked_.getDeviceSettings(
                                                                 linked_.getDefaultDevice()),
                                                             "",
                                                             useAVFrame_)));
    } catch (const std::out_of_range& e) {
        qWarning() << "Couldn't setup video input renderer: " << e.what();
    }
#ifndef ENABLE_LIBWRAP
    SIZE_RENDERER = renderers_.size();
#endif
    connect(&callbacksHandler, &CallbacksHandler::deviceEvent, this, &AVModelPimpl::slotDeviceEvent);
    connect(&callbacksHandler, &CallbacksHandler::audioMeter, this, &AVModelPimpl::slotAudioMeter);
    connect(&callbacksHandler,
            &CallbacksHandler::startedDecoding,
            this,
            &AVModelPimpl::startedDecoding);
    connect(&callbacksHandler,
            &CallbacksHandler::stoppedDecoding,
            this,
            &AVModelPimpl::stoppedDecoding);
    connect(&callbacksHandler,
            &CallbacksHandler::callStateChanged,
            this,
            &AVModelPimpl::slotCallStateChanged);
    connect(&*renderers_[video::PREVIEW_RENDERER_ID],
            &api::video::Renderer::frameUpdated,
            this,
            &AVModelPimpl::slotFrameUpdated);
    connect(&callbacksHandler,
            &CallbacksHandler::recordPlaybackStopped,
            this,
            &AVModelPimpl::slotRecordPlaybackStopped);

    auto startedPreview = false;
    auto restartRenderers = [&](const QStringList& callList) {
        for (const auto& callId : callList) {
            MapStringString rendererInfos = VideoManager::instance().getRenderer(callId);
            auto shmPath = rendererInfos[DRing::Media::Details::SHM_PATH];
            auto width = rendererInfos[DRing::Media::Details::WIDTH].toInt();
            auto height = rendererInfos[DRing::Media::Details::HEIGHT].toInt();
            if (width > 0 && height > 0) {
                startedPreview = true;
                startedDecoding(callId, shmPath, width, height);
            }
        }
    };
    restartRenderers(CallManager::instance().getCallList());
    restartRenderers(CallManager::instance().getConferenceList());
    if (startedPreview)
        restartRenderers({"local"});
}

QString
AVModelPimpl::getRecordingPath() const
{
#if defined(_WIN32) || defined(__APPLE__)
    const QDir dir = linked_.getRecordPath() + "/" + recorderSavesSubdir;
#else
    const QDir dir = authority::storage::getPath() + "/" + recorderSavesSubdir;
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

    return file_path.path();
}

void
AVModelPimpl::startedDecoding(const QString& id, const QString& shmPath, int width, int height)
{
    const QString res = toQString(width) + "x" + toQString(height);
    {
        std::lock_guard<std::mutex> lk(renderers_mtx_);
        auto search = renderers_.find(id);

        if (search == renderers_.end()) {
            video::Settings settings;
            settings.size = res;
            renderers_.insert(std::make_pair(id,
                                             std::make_unique<video::Renderer>(id,
                                                                               settings,
                                                                               shmPath,
                                                                               useAVFrame_)));
            finishedRenderers_.insert(std::make_pair(id, false));
#ifndef ENABLE_LIBWRAP
            SIZE_RENDERER = renderers_.size();
#endif
            renderers_.at(id)->initThread();
            connect(&*renderers_[id],
                    &api::video::Renderer::frameUpdated,
                    this,
                    &AVModelPimpl::slotFrameUpdated);
        } else {
            (*search).second->update(res, shmPath);
        }
        renderers_.at(id)->startRendering();
    }
    emit linked_.rendererStarted(id);
}

void
AVModelPimpl::stoppedDecoding(const QString& id, const QString& shmPath)
{
    Q_UNUSED(shmPath)
    {
        std::lock_guard<std::mutex> lk(renderers_mtx_);
        auto search = renderers_.find(id);
        if (search == renderers_.end()) {
            qWarning() << "Cannot stop decoding, renderer " << id << "not found";
            return; // nothing to do
        }

        (*search).second->stopRendering();
        qDebug() << "Video stopped for call" << id;
        (*search).second->quit();
        if (id != video::PREVIEW_RENDERER_ID) {
            auto searchFinished = finishedRenderers_.find(id);
            if (searchFinished == finishedRenderers_.end()) {
                qWarning() << "Finished flag: " << id << " not found";
                return; // nothing to do
            }
            if (searchFinished->second) {
                disconnect(&*search->second,
                           &api::video::Renderer::frameUpdated,
                           this,
                           &AVModelPimpl::slotFrameUpdated);
                renderers_.erase(search);
#ifndef ENABLE_LIBWRAP
                SIZE_RENDERER = renderers_.size();
#endif
                finishedRenderers_.erase(id);
            }
        }
    }
    emit linked_.rendererStopped(id);
}

void
AVModelPimpl::slotCallStateChanged(const QString& id, const QString& state, int code)
{
    Q_UNUSED(code)
    if (call::to_status(state) != call::Status::ENDED)
        return;
    std::lock_guard<std::mutex> lk(renderers_mtx_);
    auto search = renderers_.find(id);
    auto searchFinished = finishedRenderers_.find(id);
    if (search == renderers_.end() || searchFinished == finishedRenderers_.end()) {
        qWarning() << "Renderer " << id << "not found";
        return; // nothing to do
    }
    if (!search->second->isRendering()) {
        disconnect(&*search->second,
                   &api::video::Renderer::frameUpdated,
                   this,
                   &AVModelPimpl::slotFrameUpdated);
        renderers_.erase(search);
#ifndef ENABLE_LIBWRAP
        SIZE_RENDERER = renderers_.size();
#endif
        finishedRenderers_.erase(id);
    } else {
        finishedRenderers_.at(id) = true;
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

QString
AVModelPimpl::getDevice(int type) const
{
    if (type < 0 || type > 2)
        return {}; // No device
    QString result = "";
    VectorString devices;
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
    QStringList currentDevicesIdx = ConfigurationManager::instance().getCurrentAudioDevicesIndex();
    try {
        if (currentDevicesIdx.size() < 3) {
            // Should not happen, but cannot retrieve current ringtone device
            return "";
        }
        auto deviceIdx = currentDevicesIdx[type].toUInt();
        for (const auto& dev : devices) {
            uint32_t idx;
            switch (type) {
            case 1: // INPUT
                idx = ConfigurationManager::instance().getAudioInputDeviceIndex(dev);
                break;
            case 0: // OUTPUT
            case 2: // RINGTONE
                idx = ConfigurationManager::instance().getAudioOutputDeviceIndex(dev);
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
AVModelPimpl::slotFrameUpdated(const QString& id)
{
    emit linked_.frameUpdated(id);
}

void
AVModelPimpl::slotDeviceEvent()
{
    emit linked_.deviceEvent();
}

void
AVModelPimpl::slotAudioMeter(const QString& id, float level)
{
    emit linked_.audioMeter(id, level);
}

void
AVModelPimpl::slotRecordPlaybackStopped(const QString& filePath)
{
    emit linked_.recordPlaybackStopped(filePath);
}

} // namespace lrc

#include "api/moc_avmodel.cpp"
#include "avmodel.moc"
