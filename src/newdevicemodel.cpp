#include "api/newdevicemodel.h"

// Dbus
#include "dbus/configurationmanager.h"

// Daemon
#include <account_const.h>

// Qt
#include <QObject>

// std
#include <list>

#include <iostream>

namespace lrc
{

using namespace api;

class NewDeviceModelPimpl: public QObject
{
public:
    NewDeviceModelPimpl(const NewDeviceModel& linked, const CallbacksHandler& callbacksHandler);
    ~NewDeviceModelPimpl();

    const CallbacksHandler& callbacksHandler;
    const NewDeviceModel& linked;

    std::list<Device> devices_;
    std::string currentDeviceId_;
};

NewDeviceModel::NewDeviceModel(const account::Info& owner, const CallbacksHandler& callbacksHandler)
: owner(owner)
, pimpl_(std::make_unique<NewDeviceModelPimpl>(*this, callbacksHandler))
{

}

NewDeviceModel::~NewDeviceModel() {}

std::list<Device>
NewDeviceModel::getAllDevices() const
{
    return pimpl_->devices_;
}

Device
NewDeviceModel::getDevice(const std::string& id) const
{
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
    ConfigurationManager::instance().revokeDevice(owner.id.c_str(), id.c_str(), password.c_str());
}

void
NewDeviceModel::setCurrentDeviceName(const std::string& newName)
{

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
        Device device {
            /* id= */it.key().toStdString(),
            /* name= */it.value().toStdString(),
            /* isCurrent= */it.key().toStdString() == currentDeviceId_
        };
        devices_.emplace_back(device);
        ++it;
    }
}

NewDeviceModelPimpl::~NewDeviceModelPimpl()
{

}

} // namespace lrc

#include "api/moc_newdevicemodel.cpp"
