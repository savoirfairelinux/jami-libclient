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

#pragma once

#include "qmladapterbase.h"
#include "pluginitemlistmodel.h"
#include "mediahandleritemlistmodel.h"
#include "pluginlistpreferencemodel.h"
#include "preferenceitemlistmodel.h"

#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>

class PluginAdapter final : public QmlAdapterBase
{
    Q_OBJECT
public:
    explicit PluginAdapter(QObject* parent = nullptr);
    ~PluginAdapter() = default;

protected:
    void safeInit() override {};

    Q_INVOKABLE QVariant getMediaHandlerSelectableModel(const QString& callId = QString(""));
    Q_INVOKABLE QVariant getPluginSelectableModel();
    Q_INVOKABLE QVariant getPluginPreferencesModel(const QString& pluginId,
                                                   const QString& mediaHandlerName = "");

private:
    std::unique_ptr<MediaHandlerItemListModel> mediaHandlerListModel_;
    std::unique_ptr<PreferenceItemListModel> preferenceItemListModel_;
    std::unique_ptr<PluginItemListModel> pluginItemListModel_;
};
