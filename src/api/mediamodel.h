/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
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

// Std
#include <map>
#include <memory>
#include <string>

// Qt
#include <qobject.h>
#include <QObject>

// Lrc
#include "api/account.h"
#include "api/video.h"
#include "typedefs.h"

namespace lrc
{

class CallbacksHandler;
class MediaModelPimpl;

namespace api
{

/**
  *  @brief Class that manages audio, video and renderers for Ring
  *  NOTE: For now, video devices will be reloaded foreach account. But we can
  *  imagine to have different video devices per account one day.
  */
class LIB_EXPORT MediaModel : public QObject {
    Q_OBJECT

public:
    const account::Info& owner;

    MediaModel(const account::Info& owner,
        const CallbacksHandler& callbacksHandler);
    ~MediaModel();

    bool getDecodingAccelerated() const;
    void setDecodingAccelerated(bool accelerate);

    std::string getDefaultDeviceName() const;
    void setDefaultDevice(const std::string& name);
    video::Settings getDeviceSettings(const std::string& name) const;
    video::Capabilities getDeviceCapabilities(const std::string& name) const;

private:
    std::unique_ptr<MediaModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
