/**
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

#include "mediahandleradapter.h"

#include "lrcinstance.h"

MediaHandlerAdapter::MediaHandlerAdapter(QObject* parent)
    : QmlAdapterBase(parent)
{}

MediaHandlerAdapter::~MediaHandlerAdapter() {}

QVariant
MediaHandlerAdapter::getMediaHandlerSelectableModel()
{
    /*
     * Called from qml every time contact picker refreshes.
     */
    mediaHandlerListModel_.reset(new MediaHandlerItemListModel(this));

    return QVariant::fromValue(mediaHandlerListModel_.get());
}

QVariant
MediaHandlerAdapter::getMediaHandlerPreferencesModel(QString pluginId, QString mediaHandlerName)
{
    /*
     * Called from qml every time contact picker refreshes.
     */
    mediaHandlerPreferenceItemListModel_.reset(new PreferenceItemListModel(this));
    mediaHandlerPreferenceItemListModel_->setMediaHandlerName(mediaHandlerName);
    mediaHandlerPreferenceItemListModel_->setPluginId(pluginId);

    return QVariant::fromValue(mediaHandlerPreferenceItemListModel_.get());
}

QVariant
MediaHandlerAdapter::getMediaHandlerPreferencesSelectableModel(QString pluginId)
{
    /*
     * Called from qml every time contact picker refreshes.
     */
    mediaHandlerListPreferenceModel_.reset(new MediaHandlerListPreferenceModel(this));
    mediaHandlerListPreferenceModel_->setPluginId(pluginId);

    return QVariant::fromValue(mediaHandlerListPreferenceModel_.get());
}

void
MediaHandlerAdapter::initQmlObject()
{}
