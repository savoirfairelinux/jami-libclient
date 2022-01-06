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
#include "api/newdevicemodel.h"

#include "api/newaccountmodel.h"
#include "callbackshandler.h"
#include "dbus/configurationmanager.h"

#include <account_const.h>

#include <QObject>

#include <list>
#include <mutex>

namespace lrc {

using namespace api;

class NewDeviceModelPimpl : public QObject
{
    Q_OBJECT
public:
    NewDeviceModelPimpl(const NewDeviceModel& linked, const CallbacksHandler& callbacksHandler);
    ~NewDeviceModelPimpl();

    const CallbacksHandler& callbacksHandler;
    const NewDeviceModel& linked;

    std::mutex devicesMtx_;
    QString currentDeviceId_;
    QList<Device> devices_;
public Q_SLOTS:
    /**
     * Listen from CallbacksHandler to get when a device name changed or a device is added
     * @param accountId interaction receiver.
     * @param devices A map of device IDs with corresponding labels.
     */
    void slotKnownDevicesChanged(const QString& accountId, const MapStringString devices);

    /**
     * update devices_ when a device is revoked
     * @param accountId
     * @param deviceId
     * @param status SUCCESS = 0, WRONG_PASSWORD = 1, UNKNOWN_DEVICE = 2
     */
    void slotDeviceRevocationEnded(const QString& accountId,
                                   const QString& deviceId,
                                   const int status);
};

NewDeviceModel::NewDeviceModel(const account::Info& owner, const CallbacksHandler& callbacksHandler)
    : owner(owner)
    , pimpl_(std::make_unique<NewDeviceModelPimpl>(*this, callbacksHandler))
{}

NewDeviceModel::~NewDeviceModel() {}

QList<Device>
NewDeviceModel::getAllDevices() const
{
    return pimpl_->devices_;
}

Device
NewDeviceModel::getDevice(const QString& id) const
{
    std::lock_guard<std::mutex> lock(pimpl_->devicesMtx_);
    auto i = std::find_if(pimpl_->devices_.begin(), pimpl_->devices_.end(), [id](const Device& d) {
        return d.id == id;
    });

    if (i == pimpl_->devices_.end())
        return {};

    return *i;
}

void
NewDeviceModel::revokeDevice(const QString& id, const QString& password)
{
    ConfigurationManager::instance().revokeDevice(owner.id, password, id);
}

void
NewDeviceModel::setCurrentDeviceName(const QString& newName)
{
    // Update deamon config
    auto config = owner.accountModel->getAccountConfig(owner.id);
    config.deviceName = newName;
    owner.accountModel->setAccountConfig(owner.id, config);
    // Update model
    std::lock_guard<std::mutex> lock(pimpl_->devicesMtx_);
    for (auto& device : pimpl_->devices_) {
        if (device.id == config.deviceId) {
            device.name = newName;
            emit deviceUpdated(device.id);

            return;
        }
    }
}

NewDeviceModelPimpl::NewDeviceModelPimpl(const NewDeviceModel& linked,
                                         const CallbacksHandler& callbacksHandler)
    : linked(linked)
    , callbacksHandler(callbacksHandler)
    , devices_({})
{
    const MapStringString aDetails = ConfigurationManager::instance().getAccountDetails(
        linked.owner.id);
    currentDeviceId_ = aDetails.value(DRing::Account::ConfProperties::DEVICE_ID);
    const MapStringString accountDevices = ConfigurationManager::instance().getKnownRingDevices(
        linked.owner.id);
    auto it = accountDevices.begin();
    while (it != accountDevices.end()) {
        {
            std::lock_guard<std::mutex> lock(devicesMtx_);
            auto device = Device {/* id= */ it.key(),
                                  /* name= */ it.value(),
                                  /* isCurrent= */ it.key() == currentDeviceId_};
            if (device.isCurrent) {
                devices_.push_back(device);
            } else {
                devices_.push_back(device);
            }
        }
        ++it;
    }

    connect(&callbacksHandler,
            &CallbacksHandler::knownDevicesChanged,
            this,
            &NewDeviceModelPimpl::slotKnownDevicesChanged);
    connect(&callbacksHandler,
            &CallbacksHandler::deviceRevocationEnded,
            this,
            &NewDeviceModelPimpl::slotDeviceRevocationEnded);
}

NewDeviceModelPimpl::~NewDeviceModelPimpl()
{
    disconnect(&callbacksHandler,
               &CallbacksHandler::knownDevicesChanged,
               this,
               &NewDeviceModelPimpl::slotKnownDevicesChanged);
    disconnect(&callbacksHandler,
               &CallbacksHandler::deviceRevocationEnded,
               this,
               &NewDeviceModelPimpl::slotDeviceRevocationEnded);
}

void
NewDeviceModelPimpl::slotKnownDevicesChanged(const QString& accountId, const MapStringString devices)
{
    if (accountId != linked.owner.id)
        return;
    auto devicesMap = devices;
    // Update current devices
    QStringList updatedDevices;
    {
        std::lock_guard<std::mutex> lock(devicesMtx_);
        for (auto& device : devices_) {
            if (devicesMap.find(device.id) != devicesMap.end()) {
                if (device.name != devicesMap[device.id]) {
                    updatedDevices.push_back(device.id);
                    device.name = devicesMap[device.id];
                }
                devicesMap.remove(device.id);
            }
        }
    }
    for (const auto& device : updatedDevices)
        emit linked.deviceUpdated(device);

    // Add new devices
    QStringList addedDevices;
    {
        std::lock_guard<std::mutex> lock(devicesMtx_);
        auto it = devicesMap.begin();
        while (it != devicesMap.end()) {
            devices_.push_back(Device {/* id= */ it.key(),
                                       /* name= */ it.value(),
                                       /* isCurrent= */ false});
            addedDevices.push_back(it.key());
            ++it;
        }
    }
    for (const auto& device : addedDevices)
        emit linked.deviceAdded(device);
}

void
NewDeviceModelPimpl::slotDeviceRevocationEnded(const QString& accountId,
                                               const QString& deviceId,
                                               const int status)
{
    if (accountId != linked.owner.id)
        return;
    if (status == 0) {
        std::lock_guard<std::mutex> lock(devicesMtx_);
        auto it = std::find_if(devices_.begin(), devices_.end(), [deviceId](const Device& d) {
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
