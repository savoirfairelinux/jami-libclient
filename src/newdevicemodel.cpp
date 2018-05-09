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
#include "api/newdevicemodel.h"

// std
#include <list>
#include <mutex>

// LRC
#include "callbackshandler.h"
#include "dbus/configurationmanager.h"

// Daemon
#include <account_const.h>

// Qt
#include <QObject>

namespace lrc
{


using namespace api;

class NewDeviceModelPimpl: public QObject
{
    Q_OBJECT
public:
    NewDeviceModelPimpl(const NewDeviceModel& linked, const CallbacksHandler& callbacksHandler);
    ~NewDeviceModelPimpl();

    const CallbacksHandler& callbacksHandler;
    const NewDeviceModel& linked;

    std::mutex devicesMtx_;
    std::string currentDeviceId_;
    std::list<Device> devices_;
public Q_SLOTS:
    /**
     * Listen from CallbacksHandler to get when a device name changed or a device is added
     * @param accountId interaction receiver.
     * @param devices A map of device IDs with corresponding labels.
     */
    void slotKnownDevicesChanged(const std::string& accountId,
                                 const std::map<std::string, std::string> devices);

    /**
     * update devices_ when a device is revoked
     * @param accountId
     * @param deviceId
     * @param status SUCCESS = 0, WRONG_PASSWORD = 1, UNKNOWN_DEVICE = 2
     */
    void slotDeviceRevocationEnded(const std::string& accountId,
                                   const std::string& deviceId,
                                   const int status);
};

NewDeviceModel::NewDeviceModel(const account::Info& owner, const CallbacksHandler& callbacksHandler)
: owner(owner)
, pimpl_(std::make_unique<NewDeviceModelPimpl>(*this, callbacksHandler))
{ }

NewDeviceModel::~NewDeviceModel() {}

std::list<Device>
NewDeviceModel::getAllDevices() const
{
    return pimpl_->devices_;
}

Device
NewDeviceModel::getDevice(const std::string& id) const
{
    std::lock_guard<std::mutex> lock(pimpl_->devicesMtx_);
    auto i = std::find_if(
        pimpl_->devices_.begin(), pimpl_->devices_.end(),
        [id](const Device& d) {
            return d.id == id;
        });

    if (i == pimpl_->devices_.end()) return {};

    return *i;
}

void
NewDeviceModel::revokeDevice(const std::string& id, const std::string& password)
{
    ConfigurationManager::instance().revokeDevice(owner.id.c_str(), password.c_str(), id.c_str());
}

void
NewDeviceModel::setCurrentDeviceName(const std::string& newName)
{
    MapStringString details = {};
    details[DRing::Account::ConfProperties::RING_DEVICE_NAME] = newName.c_str();
    ConfigurationManager::instance().setAccountDetails(owner.id.c_str(), details);
}

NewDeviceModelPimpl::NewDeviceModelPimpl(const NewDeviceModel& linked, const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
, devices_({})
{
    const MapStringString aDetails = ConfigurationManager::instance().getAccountDetails(linked.owner.id.c_str());
    currentDeviceId_ = aDetails.value(DRing::Account::ConfProperties::RING_DEVICE_ID).toStdString();
    const MapStringString accountDevices = ConfigurationManager::instance().getKnownRingDevices(linked.owner.id.c_str());
    auto it = accountDevices.begin();
    while (it != accountDevices.end()) {
        {
            std::lock_guard<std::mutex> lock(devicesMtx_);
            devices_.emplace_back(Device {
                /* id= */it.key().toStdString(),
                /* name= */it.value().toStdString(),
                /* isCurrent= */it.key().toStdString() == currentDeviceId_
            });
        }
        ++it;
    }

    connect(&callbacksHandler, &CallbacksHandler::knownDevicesChanged, this,
            &NewDeviceModelPimpl::slotKnownDevicesChanged);
    connect(&callbacksHandler, &CallbacksHandler::deviceRevocationEnded, this,
            &NewDeviceModelPimpl::slotDeviceRevocationEnded);
}

NewDeviceModelPimpl::~NewDeviceModelPimpl()
{
    disconnect(&callbacksHandler, &CallbacksHandler::knownDevicesChanged, this,
            &NewDeviceModelPimpl::slotKnownDevicesChanged);
    disconnect(&callbacksHandler, &CallbacksHandler::deviceRevocationEnded, this,
            &NewDeviceModelPimpl::slotDeviceRevocationEnded);
}

void
NewDeviceModelPimpl::slotKnownDevicesChanged(const std::string& accountId,
                                             const std::map<std::string, std::string> devices)
{
    if (accountId != linked.owner.id) return;
    auto devicesMap = devices;
    // Update current devices
    std::list<std::string> updatedDevices;
    {
        std::lock_guard<std::mutex> lock(devicesMtx_);
        for (auto& device : devices_) {
            if (devicesMap.find(device.id) != devicesMap.end()) {
                if (device.name != devicesMap[device.id]) {
                    updatedDevices.emplace_back(device.id);
                    device.name = devicesMap[device.id];
                }
                devicesMap.erase(device.id);
            }
        }
    }
    for (const auto& device : updatedDevices)
        emit linked.deviceUpdated(device);

    // Add new devices
    std::list<std::string> addedDevices;
    {
        std::lock_guard<std::mutex> lock(devicesMtx_);
        auto it = devicesMap.begin();
        while (it != devicesMap.end()) {
            devices_.emplace_back(Device {
                /* id= */it->first,
                /* name= */it->second,
                /* isCurrent= */false
            });
            addedDevices.emplace_back(it->first);
            ++it;
        }
    }
    for (const auto& device : addedDevices)
        emit linked.deviceAdded(device);
}


void
NewDeviceModelPimpl::slotDeviceRevocationEnded(const std::string& accountId,
                                               const std::string& deviceId,
                                               const int status)
{
    if (accountId != linked.owner.id) return;
    if (status == 0) {
        auto it = std::find_if(
            devices_.begin(), devices_.end(),
            [deviceId](const Device& d) {
                return d.id == deviceId;
            });

        if (it != devices_.end())
            devices_.erase(it);
    }

    switch (status) {
    case 0:
        emit linked.deviceRevoked(deviceId, NewDeviceModel::Status::SUCCESS);
        break;
    case 1:
        emit linked.deviceRevoked(deviceId, NewDeviceModel::Status::WRONG_PASSWORD);
        break;
    case 2:
        emit linked.deviceRevoked(deviceId, NewDeviceModel::Status::UNKNOWN_DEVICE);
        break;
    default:
        break;
    }
}

} // namespace lrc

#include "newdevicemodel.moc"
#include "api/moc_newdevicemodel.cpp"
