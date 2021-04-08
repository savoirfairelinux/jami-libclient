/**
 * Copyright (C) 2020 by Savoir-faire Linux
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

#include "abstractitemmodelbase.h"

#include "api/pluginmodel.h"

class PluginListPreferenceModel : public AbstractListModelBase
{
    Q_OBJECT
    Q_PROPERTY(QString pluginId READ pluginId WRITE setPluginId)
    Q_PROPERTY(QString preferenceKey READ preferenceKey WRITE setPreferenceKey)
    Q_PROPERTY(QString preferenceNewValue READ preferenceNewValue WRITE setPreferenceNewValue)
    Q_PROPERTY(int idx READ idx WRITE setIdx)
    Q_PROPERTY(int optSize READ optSize)
public:
    enum Role { PreferenceValue = Qt::UserRole + 1, PreferenceEntryValue };
    Q_ENUM(Role)

    explicit PluginListPreferenceModel(QObject* parent = nullptr);
    ~PluginListPreferenceModel();

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
     * This function is to reset the model when there's new account added.
     */
    Q_INVOKABLE void reset();
    /*
     * This function is to get the current preference value
     */
    Q_INVOKABLE int getCurrentSettingIndex();

    Q_INVOKABLE void populateLists();

    void setPreferenceNewValue(const QString preferenceNewValue)
    {
        preferenceNewValue_ = preferenceNewValue;
    }
    void setPreferenceKey(const QString preferenceKey)
    {
        preferenceKey_ = preferenceKey;
    }
    void setPluginId(const QString pluginId)
    {
        pluginId_ = pluginId;
        populateLists();
    }

    void setIdx(const int index)
    {
        idx_ = index;
    }

    int idx()
    {
        return idx_;
    }
    QString preferenceCurrentValue()
    {
        return lrcInstance_->pluginModel().getPluginPreferencesValues(pluginId_)[preferenceKey_];
    }

    QString preferenceNewValue()
    {
        preferenceNewValue_ = preferenceValuesList_[idx_];
        return preferenceNewValue_;
    }
    QString preferenceKey()
    {
        return preferenceKey_;
    }
    QString pluginId()
    {
        return pluginId_;
    }
    int optSize()
    {
        return preferenceValuesList_.size();
    }

private:
    QString pluginId_ = "";
    QString preferenceKey_ = "";
    QString preferenceNewValue_ = "";
    QStringList preferenceValuesList_;
    QStringList preferenceList_;
    int idx_ = 0;
};
