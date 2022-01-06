/**
 * Copyright (C) 2020-2022 Savoir-faire Linux Inc.
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

#include "abstractlistmodelbase.h"

class LRCInstance;

class PreferenceItemListModel : public AbstractListModelBase
{
    Q_OBJECT

    Q_PROPERTY(QString pluginId READ pluginId WRITE setPluginId)
    Q_PROPERTY(QString mediaHandlerName READ mediaHandlerName WRITE setMediaHandlerName)
    Q_PROPERTY(int preferencesCount READ preferencesCount)
public:
    enum Role {
        PreferenceKey = Qt::UserRole + 1,
        PreferenceName,
        PreferenceSummary,
        PreferenceType,
        PluginId,
        PreferenceCurrentValue,
        CurrentPath,
        FileFilters,
        IsImage,
        Enabled
    };

    typedef enum {
        LIST,
        PATH,
        EDITTEXT,
        SWITCH,
        DEFAULT,
    } Type;

    Q_ENUM(Role)

    explicit PreferenceItemListModel(QObject* parent = nullptr, LRCInstance* instance = nullptr);
    ~PreferenceItemListModel();

    /*
     * QAbstractListModel override.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    /*
     * Override role name as access point in qml.
     */
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& child) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

    /*
     * This function is to reset the model when there's new plugin added or modified.
     */
    Q_INVOKABLE void reset();

    QString pluginId() const;
    void setPluginId(const QString& pluginId);
    QString mediaHandlerName() const;
    void setMediaHandlerName(const QString mediaHandlerName);
    QString category() const;
    void setCategory(const QString category);
    int preferencesCount();

private:
    QString pluginId_;
    QString mediaHandlerName_ = "";
    VectorMapStringString preferenceList_;
    QString category_ = "all";
};
