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

#include "calloverlaymodel.h"

#include <QEvent>
#include <QMouseEvent>
#include <QQuickWindow>

CallControlListModel::CallControlListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

int
CallControlListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return data_.size();
}

QVariant
CallControlListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    using namespace CallControl;
    auto item = data_.at(index.row());

    switch (role) {
    case Role::ItemAction:
        return QVariant::fromValue(item.itemAction);
    case Role::BadgeCount:
        return QVariant::fromValue(item.badgeCount);
    }
    return QVariant();
}

QHash<int, QByteArray>
CallControlListModel::roleNames() const
{
    using namespace CallControl;
    QHash<int, QByteArray> roles;
    roles[ItemAction] = "ItemAction";
    roles[BadgeCount] = "BadgeCount";
    return roles;
}

void
CallControlListModel::setBadgeCount(int row, int count)
{
    if (row >= rowCount())
        return;
    data_[row].badgeCount = count;
    auto idx = index(row, 0);
    Q_EMIT dataChanged(idx, idx);
}

void
CallControlListModel::addItem(const CallControl::Item& item)
{
    beginResetModel();
    data_.append(item);
    endResetModel();
}

void
CallControlListModel::clearData()
{
    data_.clear();
}

IndexRangeFilterProxyModel::IndexRangeFilterProxyModel(QAbstractListModel* parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(parent);
    sourceModel()->data(QModelIndex());
}

bool
IndexRangeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    auto index = sourceModel()->index(sourceRow, 0, sourceParent);
    bool predicate = true;
    if (filterRole() != Qt::DisplayRole) {
        predicate = sourceModel()->data(index, filterRole()).toInt() != 0;
    }
    return sourceRow <= max_ && sourceRow >= min_ && predicate;
}

void
IndexRangeFilterProxyModel::setRange(int min, int max)
{
    min_ = min;
    max_ = max;
    invalidateFilter();
}

PendingConferenceesListModel::PendingConferenceesListModel(LRCInstance* instance, QObject* parent)
    : QAbstractListModel(parent)
    , lrcInstance_(instance)
{
    connectSignals();
    connect(lrcInstance_, &LRCInstance::currentAccountIdChanged, [this]() { connectSignals(); });
}

int
PendingConferenceesListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return lrcInstance_->getCurrentCallModel()->getPendingConferencees().size();
}

QVariant
PendingConferenceesListModel::data(const QModelIndex& index, int role) const
{
    using namespace PendingConferences;

    // WARNING: not swarm ready
    QString pendingConferenceeCallId;
    QString pendingConferenceeContactUri;
    ContactModel* contactModel {nullptr};
    lrc::api::call::Status callStatus;
    try {
        auto callModel = lrcInstance_->getCurrentCallModel();
        auto currentPendingConferenceeInfo = callModel->getPendingConferencees().at(index.row());
        pendingConferenceeCallId = currentPendingConferenceeInfo.callId;
        const auto call = callModel->getCall(pendingConferenceeCallId);

        callStatus = call.status;
        pendingConferenceeContactUri = currentPendingConferenceeInfo.uri;
        contactModel = lrcInstance_->getCurrentContactModel();
    } catch (...) {
        return QVariant(false);
    }

    // Since we are using image provider right now, image url representation should be unique to
    // be able to use the image cache, account avatar will only be updated once PictureUid changed
    switch (role) {
    case Role::PrimaryName:
        return QVariant(contactModel->bestNameForContact(pendingConferenceeContactUri));
    case Role::CallStatus:
        return QVariant(lrc::api::call::to_string(callStatus));
    case Role::ContactUri:
        return QVariant(pendingConferenceeContactUri);
    case Role::PendingConferenceeCallId:
        return QVariant(pendingConferenceeCallId);
    }
    return QVariant();
}

QHash<int, QByteArray>
PendingConferenceesListModel::roleNames() const
{
    using namespace PendingConferences;
    QHash<int, QByteArray> roles;
#define X(role) roles[role] = #role;
    PC_ROLES
#undef X
    return roles;
}

void
PendingConferenceesListModel::connectSignals()
{
    beginResetModel();

    disconnect(callsStatusChanged_);
    disconnect(beginInsertPendingConferencesRows_);
    disconnect(endInsertPendingConferencesRows_);
    disconnect(beginRemovePendingConferencesRows_);
    disconnect(endRemovePendingConferencesRows_);

    callsStatusChanged_ = connect(lrcInstance_->getCurrentCallModel(),
                                  &NewCallModel::callStatusChanged,
                                  this,
                                  [this]() {
                                      dataChanged(index(0, 0),
                                                  index(rowCount() - 1),
                                                  {PendingConferences::Role::CallStatus});
                                  });

    beginInsertPendingConferencesRows_ = connect(
        lrcInstance_->getCurrentCallModel(),
        &NewCallModel::beginInsertPendingConferenceesRows,
        this,
        [this](int position, int rows) {
            beginInsertRows(QModelIndex(), position, position + (rows - 1));
        },
        Qt::DirectConnection);

    endInsertPendingConferencesRows_ = connect(
        lrcInstance_->getCurrentCallModel(),
        &NewCallModel::endInsertPendingConferenceesRows,
        this,
        [this]() { endInsertRows(); },
        Qt::DirectConnection);

    beginRemovePendingConferencesRows_ = connect(
        lrcInstance_->getCurrentCallModel(),
        &NewCallModel::beginRemovePendingConferenceesRows,
        this,
        [this](int position, int rows) {
            beginRemoveRows(QModelIndex(), position, position + (rows - 1));
        },
        Qt::DirectConnection);

    endRemovePendingConferencesRows_ = connect(
        lrcInstance_->getCurrentCallModel(),
        &NewCallModel::endRemovePendingConferenceesRows,
        this,
        [this]() { endRemoveRows(); },

        Qt::DirectConnection);

    endResetModel();
}

CallOverlayModel::CallOverlayModel(LRCInstance* instance, QObject* parent)
    : QObject(parent)
    , lrcInstance_(instance)
    , primaryModel_(new CallControlListModel(this))
    , secondaryModel_(new CallControlListModel(this))
    , overflowModel_(new IndexRangeFilterProxyModel(secondaryModel_))
    , overflowVisibleModel_(new IndexRangeFilterProxyModel(secondaryModel_))
    , overflowHiddenModel_(new IndexRangeFilterProxyModel(secondaryModel_))
    , pendingConferenceesModel_(new PendingConferenceesListModel(instance, this))
{
    connect(this,
            &CallOverlayModel::overflowIndexChanged,
            this,
            &CallOverlayModel::setControlRanges);
    overflowVisibleModel_->setFilterRole(CallControl::Role::BadgeCount);
}

void
CallOverlayModel::addPrimaryControl(const QVariant& action)
{
    primaryModel_->addItem(CallControl::Item {action.value<QObject*>()});
}

void
CallOverlayModel::addSecondaryControl(const QVariant& action)
{
    secondaryModel_->addItem(CallControl::Item {action.value<QObject*>()});
    setControlRanges();
}

void
CallOverlayModel::setBadgeCount(int row, int count)
{
    secondaryModel_->setBadgeCount(row, count);
}

QVariant
CallOverlayModel::primaryModel()
{
    return QVariant::fromValue(primaryModel_);
}

QVariant
CallOverlayModel::secondaryModel()
{
    return QVariant::fromValue(secondaryModel_);
}

QVariant
CallOverlayModel::overflowModel()
{
    return QVariant::fromValue(overflowModel_);
}

QVariant
CallOverlayModel::overflowVisibleModel()
{
    return QVariant::fromValue(overflowVisibleModel_);
}

QVariant
CallOverlayModel::overflowHiddenModel()
{
    return QVariant::fromValue(overflowHiddenModel_);
}

QVariant
CallOverlayModel::pendingConferenceesModel()
{
    return QVariant::fromValue(pendingConferenceesModel_);
}

void
CallOverlayModel::clearControls()
{
    primaryModel_->clearData();
    secondaryModel_->clearData();
}

void
CallOverlayModel::registerFilter(QQuickWindow* object, QQuickItem* item)
{
    if (!object || !item || watchedItems_.contains(item))
        return;
    watchedItems_.push_back(item);
    if (watchedItems_.size() == 1)
        object->installEventFilter(this);
}

void
CallOverlayModel::unregisterFilter(QQuickWindow* object, QQuickItem* item)
{
    if (!object || !item || !watchedItems_.contains(item))
        return;
    watchedItems_.removeOne(item);
    if (watchedItems_.size() == 0)
        object->removeEventFilter(this);
}

bool
CallOverlayModel::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::MouseMove) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint eventPos(mouseEvent->x(), mouseEvent->y());
        auto windowItem = static_cast<QQuickWindow*>(object)->contentItem();
        Q_FOREACH (const auto& item, watchedItems_) {
            if (item->contains(windowItem->mapToItem(item, eventPos))) {
                Q_EMIT mouseMoved(item);
            }
        }
    }
    return QObject::eventFilter(object, event);
}

void
CallOverlayModel::setControlRanges()
{
    overflowModel_->setRange(0, overflowIndex_ - 1);
    overflowVisibleModel_->setRange(overflowIndex_, secondaryModel_->rowCount());
    overflowHiddenModel_->setRange(overflowIndex_, secondaryModel_->rowCount());
}
