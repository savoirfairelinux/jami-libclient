/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Yang Wang   <yang.wang@savoirfairelinux.com>
 * Author: Aline Gondim Santos   <aline.gondimsantos@savoirfairelinux.com>
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

#pragma once

#include "accountadapter.h"
#include "accountlistmodel.h"
#include "audiocodeclistmodel.h"
#include "avadapter.h"
#include "bannedlistmodel.h"
#include "calladapter.h"
#include "contactadapter.h"
#include "pluginadapter.h"
#include "conversationsadapter.h"
#include "deviceitemlistmodel.h"
#include "pluginitemlistmodel.h"
#include "mediahandleritemlistmodel.h"
#include "preferenceitemlistmodel.h"
#include "distantrenderer.h"
#include "globalinstances.h"
#include "globalsystemtray.h"
#include "messagesadapter.h"
#include "namedirectory.h"
#include "pixbufmanipulator.h"
#include "previewrenderer.h"
#include "qrimageprovider.h"
#include "settingsadapter.h"
#include "utils.h"
#include "version.h"
#include "videocodeclistmodel.h"

#include <QObject>

class ClientWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(UtilsAdapter *utilsAdaptor READ getUtilsAdapter NOTIFY utilsAdaptorChanged)
    Q_PROPERTY(SettingsAdapter *SettingsAdapter READ getSettingsAdapter NOTIFY SettingsAdapterChanged)
    Q_PROPERTY(NameDirectory *nameDirectory READ getNameDirectory NOTIFY nameDirectoryChanged)
    Q_PROPERTY(LRCInstance *lrcInstance READ getLRCInstance NOTIFY lrcInstanceChanged)
    Q_PROPERTY(AccountAdapter *accountAdaptor READ getAccountAdapter NOTIFY accountAdaptorChanged)
    Q_PROPERTY(RenderManager *renderManager READ getRenderManager NOTIFY renderManagerChanged)
    Q_PROPERTY(lrc::api::NewAccountModel *accountModel READ getAccountModel NOTIFY accountModelChanged)
    Q_PROPERTY(lrc::api::AVModel *avmodel READ getAvModel NOTIFY avmodelChanged)
    Q_PROPERTY(lrc::api::DataTransferModel *dataTransferModel READ getDataTransferModel NOTIFY dataTransferModelChanged)
    Q_PROPERTY(lrc::api::ContactModel *contactModel READ getContactModel NOTIFY contactModelChanged)
    Q_PROPERTY(lrc::api::NewDeviceModel *deviceModel READ getDeviceModel NOTIFY deviceModelChanged)
    Q_PROPERTY(lrc::api::PluginModel *pluginModel READ getPluginModel)
public:
    explicit ClientWrapper(QObject *parent = nullptr);

    NameDirectory *getNameDirectory();
    UtilsAdapter *getUtilsAdapter();
    SettingsAdapter *getSettingsAdapter();
    LRCInstance *getLRCInstance();
    AccountAdapter *getAccountAdapter();

    RenderManager *getRenderManager();
    lrc::api::NewAccountModel *getAccountModel();
    lrc::api::AVModel *getAvModel();
    lrc::api::DataTransferModel *getDataTransferModel();

    lrc::api::ContactModel *getContactModel();
    lrc::api::NewDeviceModel *getDeviceModel();
    lrc::api::PluginModel *getPluginModel();

signals:
    void utilsAdaptorChanged();
    void SettingsAdapterChanged();
    void nameDirectoryChanged();
    void lrcInstanceChanged();
    void accountAdaptorChanged();
    void renderManagerChanged();
    void accountModelChanged();
    void avmodelChanged();
    void dataTransferModelChanged();
    void contactModelChanged();
    void deviceModelChanged();
};
Q_DECLARE_METATYPE(ClientWrapper *)
