/****************************************************************************
 *    Copyright (C) 2017-2022 Savoir-faire Linux Inc.                       *
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

#include "api/account.h"
#include "typedefs.h"

#include <QObject>

#include <memory>
#include <string>
#include <list>

namespace lrc {

class CallbacksHandler;
class NewDeviceModelPimpl;

namespace api {

namespace account {
struct Info;
}

struct Device
{
    QString id = "";
    QString name = "";
    bool isCurrent = false;
};

/**
 *  @brief Class that manages ring devices for an account
 */
class LIB_EXPORT NewDeviceModel : public QObject
{
    Q_OBJECT

public:
    /**
     * Used by deviceRevoked's status
     */
    enum class Status { SUCCESS = 0, WRONG_PASSWORD = 1, UNKNOWN_DEVICE = 2 };
    const account::Info& owner;

    NewDeviceModel(const account::Info& owner, const CallbacksHandler& callbacksHandler);
    ~NewDeviceModel();

    /**
     * Get ring devices of an account
     * @return a copy of current devices
     */
    QList<Device> getAllDevices() const;

    /**
     * Retrieve a device by its id
     * @param id of the device
     * @return the device if found else a device with a null id
     */
    Device getDevice(const QString& id) const;

    /**
     * Revoke a ring device
     * @param id of the device to revoke
     * @param password of the account's archive
     * @note will emit deviceRevoked when finished
     */
    Q_INVOKABLE void revokeDevice(const QString& id, const QString& password);

    /**
     * Change the name of the current device
     * @param newName
     * @note will emit deviceUpdated when finished
     * @note ring can't change the name of another device
     */
    void setCurrentDeviceName(const QString& newName);

Q_SIGNALS:
    /**
     * Link to this signal to know when a new device is added
     * @param id added device
     */
    void deviceAdded(const QString& id) const;
    /**
     * Link to this signal to know when a device is removed
     * @param id removed device
     * @param Status (SUCCESS, WRONG_PASSWORD, UNKNOWN_DEVICE)
     */
    void deviceRevoked(const QString& id, const Status status) const;
    /**
     * Link to this signal when a device get a new name
     * @param id
     */
    void deviceUpdated(const QString& id) const;

private:
    std::unique_ptr<NewDeviceModelPimpl> pimpl_;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::NewDeviceModel*)
