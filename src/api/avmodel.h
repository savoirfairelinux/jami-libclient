/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
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

private:
    std::unique_ptr<AVModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
