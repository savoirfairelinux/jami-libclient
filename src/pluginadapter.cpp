/*!
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

#include "pluginadapter.h"

#include "lrcinstance.h"

PluginAdapter::PluginAdapter(LRCInstance* instance, QObject* parent)
    : QmlAdapterBase(instance, parent)
{
    updateHandlersListCount();
    connect(&lrcInstance_->pluginModel(),
            &lrc::api::PluginModel::modelUpdated,
            this,
            &PluginAdapter::updateHandlersListCount);
}

QVariant
PluginAdapter::getMediaHandlerSelectableModel(const QString& callId)
{
    pluginHandlerListModel_.reset(
        new PluginHandlerItemListModel(this, callId, QString(""), lrcInstance_));
    return QVariant::fromValue(pluginHandlerListModel_.get());
}

QVariant
PluginAdapter::getChatHandlerSelectableModel(const QString& accountId, const QString& peerId)
{
    pluginHandlerListModel_.reset(
        new PluginHandlerItemListModel(this, accountId, peerId, lrcInstance_));
    return QVariant::fromValue(pluginHandlerListModel_.get());
}

QVariant
PluginAdapter::getPluginSelectableModel()
{
    pluginItemListModel_.reset(new PluginItemListModel(this, lrcInstance_));
    return QVariant::fromValue(pluginItemListModel_.get());
}

QVariant
PluginAdapter::getPluginPreferencesModel(const QString& pluginId, const QString& category)
{
    preferenceItemListModel_.reset(new PreferenceItemListModel(this, lrcInstance_));
    preferenceItemListModel_->setCategory(category);
    preferenceItemListModel_->setPluginId(pluginId);

    return QVariant::fromValue(preferenceItemListModel_.get());
}

QVariant
PluginAdapter::getHandlerPreferencesModel(const QString& pluginId, const QString& mediaHandlerName)
{
    preferenceItemListModel_.reset(new PreferenceItemListModel(this, lrcInstance_));
    preferenceItemListModel_->setMediaHandlerName(mediaHandlerName);
    preferenceItemListModel_->setPluginId(pluginId);

    return QVariant::fromValue(preferenceItemListModel_.get());
}

QVariant
PluginAdapter::getPluginPreferencesCategories(const QString& pluginId, bool removeLast)
{
    QStringList categories;
    auto preferences = lrcInstance_->pluginModel().getPluginPreferences(pluginId);
    for (auto& preference : preferences) {
        if (!preference["category"].isEmpty())
            categories.push_back(preference["category"]);
    }
    categories.removeDuplicates();
    if (removeLast)
        categories.pop_back();
    return categories;
}

void
PluginAdapter::updateHandlersListCount()
{
    if (lrcInstance_->pluginModel().getPluginsEnabled()) {
        set_callMediaHandlersListCount(lrcInstance_->pluginModel().getCallMediaHandlers().size());
        set_chatHandlersListCount(lrcInstance_->pluginModel().getChatHandlers().size());
    } else {
        set_callMediaHandlersListCount(0);
        set_chatHandlersListCount(0);
    }
}