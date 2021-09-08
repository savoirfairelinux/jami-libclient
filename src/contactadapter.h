/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
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
#include "smartlistmodel.h"
#include "conversationlistmodel.h"

#include <QObject>
#include <QSortFilterProxyModel>
#include <QString>

class LRCInstance;

/*
 * The SelectableProxyModel class
 * provides support for sorting and filtering data passed between another model and a view.
 *
 * User can customize a function pointer to pass to FilterPredicate to ensure which row in
 * the source model can be accepted.
 *
 * Additionally, user need to setFilterRegExp to be able to get input QRegExp from FilterPredicate.
 */
class SelectableProxyModel final : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    using FilterPredicate = std::function<bool(const QModelIndex&, const QRegularExpression&)>;

    explicit SelectableProxyModel(QObject* parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setSortRole(ConversationList::Role::LastInteractionTimeStamp);
        sort(0, Qt::DescendingOrder);
        setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
    }

    void setPredicate(FilterPredicate filterPredicate)
    {
        filterPredicate_ = filterPredicate;
    }

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        // Accept all contacts in conversation list filtered with account type, except those in a call.
        auto index = sourceModel()->index(sourceRow, 0, sourceParent);
        return filterPredicate_ ? filterPredicate_(index, filterRegularExpression()) : false;
    }

    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override
    {
        QVariant leftData = sourceModel()->data(left, sortRole());
        QVariant rightData = sourceModel()->data(right, sortRole());
        // we're assuming the sort role data type here is some integral time
        return leftData.toUInt() < rightData.toUInt();
    };

private:
    FilterPredicate filterPredicate_;
};

class ContactAdapter final : public QmlAdapterBase
{
    Q_OBJECT

public:
    explicit ContactAdapter(LRCInstance* instance, QObject* parent = nullptr);
    ~ContactAdapter() = default;

    using Role = ConversationList::Role;

    void safeInit() override {}

    Q_INVOKABLE QVariant getContactSelectableModel(int type);
    Q_INVOKABLE void setSearchFilter(const QString& filter);
    Q_INVOKABLE void contactSelected(int index);

    void connectSignals();

Q_SIGNALS:
    void bannedStatusChanged(const QString& uri, bool banned);

private:
    SmartListModel::Type listModeltype_;
    QScopedPointer<SmartListModel> smartListModel_;
    QScopedPointer<SelectableProxyModel> selectableProxyModel_;

    QStringList defaultModerators_;

Q_SIGNALS:
    void defaultModeratorsUpdated();
};
