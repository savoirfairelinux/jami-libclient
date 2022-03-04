/*
 *  Copyright (C) 2018-2022 Savoir-faire Linux Inc.
 *  Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "api/video.h"
#include "typedefs.h"

#include <QObject>

#include <memory>
#include <string>
#include <vector>

namespace lrc {

class CallbacksHandler;
class AVModelPimpl;

namespace api {

class LIB_EXPORT AVModel : public QObject
{
    Q_OBJECT
public:
    AVModel(const CallbacksHandler& callbacksHandler);
    ~AVModel();

    /**
     * Get if hardware decoding is enabled
     * @return hardware decoding enabled
     */
    bool getDecodingAccelerated() const;
    /**
     * Enable/disable hardware decoding
     * @param if hardware decoding enabled
     */
    void setDecodingAccelerated(bool accelerate);
    /**
     * Get if hardware encoding is enabled
     * @return hardware encoding enabled
     */
    bool getEncodingAccelerated() const;
    /**
     * Enable/disable hardware encoding
     * @param if hardware encoding enabled
     */
    void setEncodingAccelerated(bool accelerate);
    /**
     * Get if hardware acceleration is enabled
     * @return hardware acceleration enabled
     */
    Q_INVOKABLE bool getHardwareAcceleration() const;
    /**
     * Enable/disable hardware acceleration
     * @param if hardware acceleration enabled
     */
    Q_INVOKABLE void setHardwareAcceleration(bool accelerate);
    /**
     * Get video devices
     * @return list of devices
     */
    QVector<QString> getDevices() const;
    /**
     * Retrieve current default video device
     * @return current default video device id
     */
    Q_INVOKABLE QString getDefaultDevice() const;
    /**
     * Set new default video device
     * @param id of the device
     */
    Q_INVOKABLE void setDefaultDevice(const QString& deviceId);
    /**
     * Retrieve current framerate/resolution/etc of a device
     * @param id of the device
     * @return settings of the device
     */
    video::Settings getDeviceSettings(const QString& deviceId) const;
    /**
     * Set device settings
     * @param video::Settings
     */
    void setDeviceSettings(video::Settings& settings);
    /**
     * Retrieve all framerate/resolution/etc possibilities of a device
     * @param id of the device
     * @return possibilities of the device
     */
    video::Capabilities getDeviceCapabilities(const QString& deviceId) const;
    /**
     * Get the deviceId corresponding to a given device friendly name
     * @return deviceId
     */
    QString getDeviceIdFromName(const QString& deviceName) const;
    /**
     * Get supported audio managers
     * @return supported audio managers
     */
    VectorString getSupportedAudioManagers() const;
    /**
     * Get current audio manager
     * @return current audio manager
     */
    QString getAudioManager() const;
    /**
     * Get current audio outputs
     * @return audio outputs
     */
    QVector<QString> getAudioOutputDevices() const;
    /**
     * Get current audio inputs
     * @return audio inputs
     */
    QVector<QString> getAudioInputDevices() const;
    /**
     * Get current ringtone device
     * @return current ringtone device
     */
    QString getRingtoneDevice() const;
    /**
     * Get current output device
     * @return current output device
     */
    QString getOutputDevice() const;
    /**
     * Get current input device
     * @return current input device
     */
    QString getInputDevice() const;
    /**
     * Get current state of the audio meter
     * @return current state of the audio meter
     */
    bool isAudioMeterActive(const QString& id = "") const;
    /**
     * Turn on/off the audio metering feature
     * @param the new state of the meter
     */
    void setAudioMeterState(bool active, const QString& id = "") const;
    /**
     * Starts audio device. Should only be invoked when outside of a call.
     */
    void startAudioDevice() const;
    /**
     * Stops audio device. Should only be invoked when outside of a call.
     */
    void stopAudioDevice() const;
    /**
     * Set current audio manager
     * @param name of the new audio manager
     * @return if the operation is successful
     */
    Q_INVOKABLE bool setAudioManager(const QString& name);
    /**
     * Set current ringtone device
     * @param name of the new ringtone device
     */
    Q_INVOKABLE void setRingtoneDevice(const QString& name);
    /**
     * Set current output device
     * @param name of the new output device
     */
    Q_INVOKABLE void setOutputDevice(const QString& name);
    /**
     * Set current input device
     * @param name of the new input device
     */
    Q_INVOKABLE void setInputDevice(const QString& name);
    /**
     * Stop local record at given path
     * @param path
     */
    Q_INVOKABLE void stopLocalRecorder(const QString& path) const;
    /**
     * Start a local recorder and return it path.
     * @param audioOnly
     */
    Q_INVOKABLE QString startLocalMediaRecorder(const QString& videoInputId) const;
    /**
     * Get the current recording path
     * @return recording path
     */
    Q_INVOKABLE QString getRecordPath() const;
    /**
     * Sets the recording path
     * @param path recording path
     */
    Q_INVOKABLE void setRecordPath(const QString& path) const;
    /**
     * Whether or not to record every call
     * @return always recording
     */
    Q_INVOKABLE bool getAlwaysRecord() const;
    /**
     * Sets whether or not to record every call
     * @param rec always recording
     */
    Q_INVOKABLE void setAlwaysRecord(const bool& rec) const;
    /**
     * Whether or not local video is recorded
     * @return recording preview
     */
    Q_INVOKABLE bool getRecordPreview() const;
    /**
     * Sets whether or not to record local video
     * @param rec recording preview
     */
    Q_INVOKABLE void setRecordPreview(const bool& rec) const;
    /**
     * Gets the quality used while recording
     * @return recording quality
     */
    Q_INVOKABLE int getRecordQuality() const;
    /**
     * Sets the recording quality
     * @param quality recording quality
     */
    Q_INVOKABLE void setRecordQuality(const int& quality) const;
    /**
     * Start preview renderer. This will start the camera
     * @param resource
     * @return sinkId
     */
    Q_INVOKABLE QString startPreview(const QString& resource);
    /**
     * Stop preview renderer and the camera.
     * @param resource
     */
    Q_INVOKABLE void stopPreview(const QString& resource);
    /**
     * Get the list of available windows ids
     * X11: a id is of the form 0x0000000
     * @return map with windows names and ids
     */
    const QVariantMap getListWindows() const;
    /**
     * set current using device
     * @param device name
     */
    Q_INVOKABLE void setCurrentVideoCaptureDevice(const QString& currentVideoCaptureDevice);
    /**
     * set current using device
     * @return current using device name
     */
    Q_INVOKABLE QString getCurrentVideoCaptureDevice() const;
    /**
     * clear current using device
     */
    void clearCurrentVideoCaptureDevice();
    /**
     * Add Renderer to renderers_ and start it
     * @param id
     * @param settings
     * @param shmPath
     */
    void addRenderer(const QString& id, const QSize& res, const QString& shmPath = {});

    bool hasRenderer(const QString& id);
    QSize getRendererSize(const QString& id);
    video::Frame getRendererFrame(const QString& id);
    bool useDirectRenderer() const;

Q_SIGNALS:
    /**
     * Emitted when a renderer is started
     * @param id of the renderer
     */
    void rendererStarted(const QString& id);
    /**
     * Emitted when a renderer is stopped
     * @param id of the renderer
     */
    void rendererStopped(const QString& id);
    /**
     * Emitted when a new frame is requested
     * @param id
     */
    void frameBufferRequested(const QString& id, AVFrame* frame);
    /**
     * Emitted when a new frame is ready
     * @param id
     */
    void frameUpdated(const QString& id);
    /**
     * Emitted when a video device is plugged or unplugged
     */
    void deviceEvent();
    /**
     * Emitted when an audio device is plugged or unplugged
     */
    void audioDeviceEvent();
    /**
     * Audio volume level
     * @param id Ringbuffer id
     * @param level Volume in range [0, 1]
     */
    void audioMeter(const QString& id, float level);
    /**
     * local recorder stopped
     * @param filePath
     */
    void recordPlaybackStopped(const QString& filePath);

private:
    std::unique_ptr<AVModelPimpl> pimpl_;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::AVModel*)
