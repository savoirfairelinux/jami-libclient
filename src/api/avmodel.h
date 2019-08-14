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
#pragma once

// std
#include <memory>
#include <string>
#include <vector>

// Qt
#include <qobject.h>

// LRC
#include "api/newvideo.h"
#include "typedefs.h"

namespace lrc
{

class CallbacksHandler;
class AVModelPimpl;

namespace api
{

class LIB_EXPORT AVModel : public QObject {
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
    bool getHardwareAcceleration() const;
    /**
     * Enable/disable hardware acceleration
     * @param if hardware acceleration enabled
     */
    void setHardwareAcceleration(bool accelerate);

    /**
     * Get video devices
     * @return list of devices
     */
    std::vector<std::string> getDevices() const;
    /**
     * Retrieve current default video device
     * @return current default video device name
     */
    std::string getDefaultDeviceName() const;
    /**
     * Set new default video device
     * @param name of the device
     */
    void setDefaultDevice(const std::string& name);
    /**
     * Retrieve current framerate/resolution/etc of a device
     * @param name of the device
     * @return settings of the device
     */
    video::Settings getDeviceSettings(const std::string& name) const;
    /**
     * Set device settings
     * @param video::Settings
     */
    void setDeviceSettings(video::Settings& settings);
    /**
     * Retrieve all framerate/resolution/etc possibilities of a device
     * @param name of the device
     * @return possibilities of the device
     */
    video::Capabilities getDeviceCapabilities(const std::string& name) const;

    /**
     * Get supported audio managers
     * @return supported audio managers
     */
    std::vector<std::string> getSupportedAudioManagers() const;
    /**
     * Get current audio manager
     * @return current audio manager
     */
    std::string getAudioManager() const;
    /**
     * Get current audio outputs
     * @return audio outputs
     */
    std::vector<std::string> getAudioOutputDevices() const;
    /**
     * Get current audio inputs
     * @return audio inputs
     */
    std::vector<std::string> getAudioInputDevices() const;
    /**
     * Get current ringtone device
     * @return current ringtone device
     */
    std::string getRingtoneDevice() const;
    /**
     * Get current output device
     * @return current output device
     */
    std::string getOutputDevice() const;
    /**
     * Get current input device
     * @return current input device
     */
    std::string getInputDevice() const;
    /**
     * Get current state of the audio meter
     * @return current state of the audio meter
     */
    bool isAudioMeterActive(const std::string& id="") const;
    /**
     * Turn on/off the audio metering feature
     * @param the new state of the meter
     */
    void setAudioMeterState(bool active, const std::string& id="") const;
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
    bool setAudioManager(const std::string& name);
    /**
     * Set current ringtone device
     * @param name of the new ringtone device
     */
    void setRingtoneDevice(const std::string& name);
    /**
     * Set current output device
     * @param name of the new output device
     */
    void setOutputDevice(const std::string& name);
    /**
     * Set current input device
     * @param name of the new input device
     */
    void setInputDevice(const std::string& name);

    /**
     * Stop local record at given path
     * @param path
     */
    void stopLocalRecorder(const std::string& path) const;
    /**
     * Start a local recorder and return it path.
     * @param audioOnly
     */
    std::string startLocalRecorder(const bool& audioOnly) const;
    /**
     * Get the current recording path
     * @return recording path
     */
    std::string getRecordPath() const;
    /**
     * Sets the recording path
     * @param path recording path
     */
    void setRecordPath(const std::string& path) const;
    /**
     * Whether or not to record every call
     * @return always recording
     */
    bool getAlwaysRecord() const;
    /**
     * Sets whether or not to record every call
     * @param rec always recording
     */
    void setAlwaysRecord(const bool& rec) const;
    /**
     * Whether or not local video is recorded
     * @return recording preview
     */
    bool getRecordPreview() const;
    /**
     * Sets whether or not to record local video
     * @param rec recording preview
     */
    void setRecordPreview(const bool& rec) const;
    /**
     * Gets the quality used while recording
     * @return recording quality
     */
    int getRecordQuality() const;
    /**
     * Sets the recording quality
     * @param quality recording quality
     */
    void setRecordQuality(const int& quality) const;

    /**
     * Start preview renderer. This will start the camera
     */
    void startPreview();
    /**
     * Stop preview renderer and the camera.
     */
    void stopPreview();
    /**
     * Get a renderer from a call
     * @param id the callid or "local"
     * @return the linked renderer
     * @throw std::out_of_range if not found
     */
    const video::Renderer& getRenderer(const std::string& id) const;

    /**
     * Render a file
     * @param uri the path of the file
     */
    void setInputFile(const std::string& uri);
    /**
     * Change the current device rendered
     * @param id of the camera
     * @note render a black frame if device not found
     */
    void switchInputTo(const std::string& id);
    /**
     * Render the current display
     * @param idx of the display
     * @param x top left of the area
     * @param y top up of the area
     * @param w width of the area
     * @param h height of the area
     */
    void setDisplay(int idx, int x, int y, int w, int h);
    /**
     * Get informations on the rendered device
     * @param call_id linked call to the renderer
     * @return the device rendered
     */
    video::RenderedDevice getCurrentRenderedDevice(const std::string& call_id) const;

    /**
     * set to true to receive AVFrames from render
     */
    void useAVFrame(bool useAVFrame);

    /**
     * check if camera is used or not
     * @return if camera used
     */
    bool isCameraUsed();

    /**
     * stop camera once needed
     */
    void stopCamera();

Q_SIGNALS:
    /**
     * Emitted when a renderer is started
     * @param id of the renderer
     */
    void rendererStarted(const std::string& id);
    /**
     * Emitted when a renderer is stopped
     * @param id of the renderer
     */
    void rendererStopped(const std::string& id);
    /**
     * Emitted when a new frame is ready
     * @param id
     */
    void frameUpdated(const std::string& id);
    /**
     * Emitted when a device is plugged or unplugged
     */
    void deviceEvent();
    /**
     * Audio volume level
     * @param id Ringbuffer id
     * @param level Volume in range [0, 1]
     */
    void audioMeter(const std::string& id, float level);

private:
    std::unique_ptr<AVModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
