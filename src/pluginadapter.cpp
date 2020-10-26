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

PluginAdapter::PluginAdapter(QObject* parent)
    : QmlAdapterBase(parent)
{}

QVariant
PluginAdapter::getMediaHandlerSelectableModel(const QString& callId)
{
    mediaHandlerListModel_.reset(new MediaHandlerItemListModel(this, callId));
    return QVariant::fromValue(mediaHandlerListModel_.get());
}

QVariant
PluginAdapter::getPluginSelectableModel()
{
    pluginItemListModel_.reset(new PluginItemListModel(this));
    return QVariant::fromValue(pluginItemListModel_.get());
}

QVariant
PluginAdapter::getPluginPreferencesModel(const QString& pluginId, const QString& mediaHandlerName)
{
    preferenceItemListModel_.reset(new PreferenceItemListModel(this));
    preferenceItemListModel_->setMediaHandlerName(mediaHandlerName);
    preferenceItemListModel_->setPluginId(pluginId);

    return QVariant::fromValue(preferenceItemListModel_.get());
}
