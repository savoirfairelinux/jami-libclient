/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clientwrapper.h"

ClientWrapper::ClientWrapper(QObject *parent)
    : QObject(parent)
{
    connect(getAccountAdapter(), &AccountAdapter::accountSignalsReconnect, [this]() {
        emit accountModelChanged();
        emit avmodelChanged();
        emit dataTransferModelChanged();
        emit contactModelChanged();
        emit deviceModelChanged();
    });
}

NameDirectory *
ClientWrapper::getNameDirectory()
{
    return &(NameDirectory::instance());
}

UtilsAdapter *
ClientWrapper::getUtilsAdapter()
{
    return &(UtilsAdapter::instance());
}

SettingsAdapter *
ClientWrapper::getSettingsAdapter()
{
    return &(SettingsAdapter::instance());
}

LRCInstance *
ClientWrapper::getLRCInstance()
{
    return &(LRCInstance::instance());
}

AccountAdapter *
ClientWrapper::getAccountAdapter()
{
    return &(AccountAdapter::instance());
}

RenderManager *
ClientWrapper::getRenderManager()
{
    return LRCInstance::renderer();
}

lrc::api::NewAccountModel *
ClientWrapper::getAccountModel()
{
    return &(LRCInstance::accountModel());
}

lrc::api::AVModel *
ClientWrapper::getAvModel()
{
    return &(LRCInstance::avModel());
}

lrc::api::PluginModel *
ClientWrapper::getPluginModel()
{
    return &(LRCInstance::pluginModel());
}

lrc::api::DataTransferModel *
ClientWrapper::getDataTransferModel()
{
    return &(LRCInstance::dataTransferModel());
}

lrc::api::ContactModel *
ClientWrapper::getContactModel()
{
    return getSettingsAdapter()->getCurrentAccountInfo().contactModel.get();
}

lrc::api::NewDeviceModel *
ClientWrapper::getDeviceModel()
{
    return getSettingsAdapter()->getCurrentAccountInfo().deviceModel.get();
}
