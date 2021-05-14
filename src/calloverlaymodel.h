/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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

#include "lrcinstance.h"
#include "qtutils.h"

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <QDebug>

#define CC_ROLES \
    X(QObject*, ItemAction) \
    X(int, BadgeCount) \
    X(bool, HasBackground) \
    X(QObject*, MenuAction) \
    X(QString, Name)

namespace CallControl {
Q_NAMESPACE
enum Role {
    DummyRole = Qt::UserRole + 1,
#define X(t, role) role,
    CC_ROLES
#undef X
};
Q_ENUM_NS(Role)

struct Item
{
#define X(t, role) t role;
    CC_ROLES
#undef X
};
} // namespace CallControl

class CallControlListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    CallControlListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setBadgeCount(int row, int count);
    void addItem(const CallControl::Item& item);

private:
    QList<CallControl::Item> data_;
};

class IndexRangeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit IndexRangeFilterProxyModel(QAbstractListModel* parent = nullptr);

    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

    void setRange(int min, int max);

private:
    int min_ {-1};
    int max_ {-1};
};

class CallOverlayModel : public QObject
{
    Q_OBJECT
    QML_PROPERTY(int, overflowIndex)

public:
    CallOverlayModel(LRCInstance* instance, QObject* parent = nullptr);

    Q_INVOKABLE void addPrimaryControl(const QVariantMap& props);
    Q_INVOKABLE void addSecondaryControl(const QVariantMap& props);
    Q_INVOKABLE void setBadgeCount(int row, int count);

    Q_INVOKABLE QVariant primaryModel();
    Q_INVOKABLE QVariant secondaryModel();
    Q_INVOKABLE QVariant overflowModel();
    Q_INVOKABLE QVariant overflowVisibleModel();
    Q_INVOKABLE QVariant overflowHiddenModel();

private Q_SLOTS:
    void setControlRanges();

private:
    LRCInstance* lrcInstance_ {nullptr};

    CallControlListModel* primaryModel_;
    CallControlListModel* secondaryModel_;
    IndexRangeFilterProxyModel* overflowModel_;
    IndexRangeFilterProxyModel* overflowVisibleModel_;
    IndexRangeFilterProxyModel* overflowHiddenModel_;
};
