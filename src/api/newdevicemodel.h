/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
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
#include <memory>
#include <string>
#include <list>

// Qt
#include <qobject.h>
#include <QObject>

// Data
#include "api/account.h"

// LRC
#include "typedefs.h"


namespace lrc
{

class CallbacksHandler;
class NewDeviceModelPimpl;

namespace api
{

namespace account { struct Info; }

struct Device
{
    std::string id = "";
    std::string name = "";
    bool isCurrent = false;
};

/**
  *  @brief Class that manages ring devices for an account
  */
class LIB_EXPORT NewDeviceModel : public QObject {
    Q_OBJECT

public:
    const account::Info& owner;

    NewDeviceModel(const account::Info& owner, const CallbacksHandler& callbacksHandler);
    ~NewDeviceModel();

    std::list<Device> getAllDevices() const;

    Device getDevice(const std::string& id) const;

    void revokeDevice(const std::string& id, const std::string& password);

    void setCurrentDeviceName(const std::string& newName);

Q_SIGNALS:
    void deviceAdded(const std::string& id) const;
    void deviceRevoked(const std::string& id) const;
    void deviceUpdated(const std::string& id) const;

private:
    std::unique_ptr<NewDeviceModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
