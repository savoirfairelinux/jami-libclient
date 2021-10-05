/*
 *  Copyright (C) 2020-2021 Savoir-faire Linux Inc.
 *
 *  Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>
 *  Author: Trevor Tabah <trevor.tabah@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */

#include "messagelistmodel.h"

#include "api/conversationmodel.h"
#include "api/interaction.h"

#include <QAbstractListModel>

namespace lrc {

using namespace api;

using constIterator = MessageListModel::constIterator;
using iterator = MessageListModel::iterator;
using reverseIterator = MessageListModel::reverseIterator;

MessageListModel::MessageListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

QPair<iterator, bool>
MessageListModel::emplace(const QString& msgId, interaction::Info message, bool beginning)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == msgId) {
            return qMakePair(it, false);
        }
    }
    auto iter = beginning ? interactions_.begin() : interactions_.end();
    auto iterator = insertMessage(iter, qMakePair(msgId, message));
    return qMakePair(iterator, true);
}

iterator
MessageListModel::find(const QString& msgId)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == msgId) {
            return it;
        }
    }
    return interactions_.end();
}

constIterator
MessageListModel::find(const QString& msgId) const
{
    constIterator it;
    for (it = interactions_.cbegin(); it != interactions_.cend(); ++it) {
        if (it->first == msgId) {
            return it;
        }
    }
    return interactions_.cend();
}

QPair<iterator, bool>
MessageListModel::insert(std::pair<QString, interaction::Info> message, bool beginning)
{
    return emplace(message.first, message.second, beginning);
}

int
MessageListModel::erase(const QString& msgId)
{
    iterator it;
    int index = 0;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == msgId) {
            removeMessage(index, it);
            return 1;
        }
        index++;
    }
    return 0;
}

interaction::Info&
MessageListModel::operator[](const QString& messageId)
{
    for (auto it = interactions_.cbegin(); it != interactions_.cend(); ++it) {
        if (it->first == messageId) {
            return const_cast<interaction::Info&>(it->second);
        }
    }
    // element not find, add it to the end
    interaction::Info newMessage = {};
    insertMessage(interactions_.end(), qMakePair(messageId, newMessage));
    if (interactions_.last().first == messageId) {
        return const_cast<interaction::Info&>(interactions_.last().second);
    }
    throw std::out_of_range("Cannot find message");
}

iterator
MessageListModel::end()
{
    return interactions_.end();
}

constIterator
MessageListModel::end() const
{
    return interactions_.end();
}

constIterator
MessageListModel::cend() const
{
    return interactions_.cend();
}

iterator
MessageListModel::begin()
{
    return interactions_.begin();
}

constIterator
MessageListModel::begin() const
{
    return interactions_.begin();
}

reverseIterator
MessageListModel::rbegin()
{
    return interactions_.rbegin();
}

int
MessageListModel::size() const
{
    return interactions_.size();
}

void
MessageListModel::clear(int leaveN)
{
    interactions_.erase(interactions_.begin(), std::prev(interactions_.end(), leaveN));
}

bool
MessageListModel::empty() const
{
    return interactions_.empty();
}

interaction::Info
MessageListModel::at(const QString& msgId) const
{
    for (auto it = interactions_.cbegin(); it != interactions_.cend(); ++it) {
        if (it->first == msgId) {
            return it->second;
        }
    }
    return {};
}

QPair<QString, interaction::Info>
MessageListModel::front() const
{
    return interactions_.front();
}

QPair<QString, interaction::Info>
MessageListModel::last() const
{
    return interactions_.last();
}

QPair<QString, interaction::Info>
MessageListModel::atIndex(int index) const
{
    return interactions_.at(index);
}

QPair<iterator, bool>
MessageListModel::insert(int index, QPair<QString, interaction::Info> message)
{
    iterator itr;
    for (itr = interactions_.begin(); itr != interactions_.end(); ++itr) {
        if (itr->first == message.first) {
            return qMakePair(itr, false);
        }
    }
    if (index >= size()) {
        auto iterator = insertMessage(interactions_.end(), message);
        return qMakePair(iterator, true);
    }
    insertMessage(index, message);
    return qMakePair(interactions_.end(), true);
}

int
MessageListModel::indexOfMessage(const QString& msgId, bool reverse) const
{
    auto getIndex = [reverse, &msgId](const auto& start, const auto& end) -> int {
        auto it = std::find_if(start, end, [&msgId](const auto& it) { return it.first == msgId; });
        if (it == end) {
            return -1;
        }
        return reverse ? std::distance(it, end) - 1 : std::distance(start, it);
    };
    return reverse ? getIndex(interactions_.rbegin(), interactions_.rend())
                   : getIndex(interactions_.begin(), interactions_.end());
}

void
MessageListModel::moveMessages(QList<QString> msgIds, const QString& parentId)
{
    for (auto msgId : msgIds) {
        moveMessage(msgId, parentId);
    }
}

void
MessageListModel::moveMessage(const QString& msgId, const QString& parentId)
{
    int currentIndex = indexOfMessage(msgId);

    // if we have a next element check if it is a child interaction
    QString childMessageIdToMove;
    if (currentIndex < (interactions_.size() - 1)) {
        const auto& next = interactions_.at(currentIndex + 1);
        if (next.second.parentId == msgId) {
            childMessageIdToMove = next.first;
        }
    }

    // move a message
    int newIndex = indexOfMessage(parentId) + 1;
    if (newIndex >= interactions_.size()) {
        newIndex = interactions_.size() - 1;
    }

    if (currentIndex == newIndex)
        return;

    moveMessage(currentIndex, newIndex);

    // move a child message
    if (!childMessageIdToMove.isEmpty()) {
        moveMessage(childMessageIdToMove, msgId);
    }
}

void
MessageListModel::insertMessage(int index, item_t& message)
{
    Q_EMIT beginInsertRows(QModelIndex(), index, index);
    interactions_.insert(index, message);
    Q_EMIT endInsertRows();
}

iterator
MessageListModel::insertMessage(iterator it, item_t& message)
{
    auto index = std::distance(begin(), it);
    Q_EMIT beginInsertRows(QModelIndex(), index, index);
    auto insertion = interactions_.insert(it, message);
    Q_EMIT endInsertRows();
    return insertion;
}

void
MessageListModel::removeMessage(int index, iterator it)
{
    Q_EMIT beginRemoveRows(QModelIndex(), index, index);
    interactions_.erase(it);
    Q_EMIT endRemoveRows();
}

void
MessageListModel::moveMessage(int from, int to)
{
    Q_EMIT beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
    interactions_.move(from, to);
    Q_EMIT endMoveRows();
}

bool
MessageListModel::contains(const QString& msgId)
{
    return find(msgId) != interactions_.end();
}

int
MessageListModel::rowCount(const QModelIndex& parent) const
{
    return interactions_.size();
}

QHash<int, QByteArray>
MessageListModel::roleNames() const
{
    using namespace MessageList;
    QHash<int, QByteArray> roles;
#define X(role) roles[role] = #role;
    MSG_ROLES
#undef X
    return roles;
}

QVariant
MessageListModel::dataForItem(item_t item, int indexRow, int role) const
{
    switch (role) {
    case Role::Id:
        return QVariant(item.first);
    case Role::Author:
        return QVariant(item.second.authorUri);
    case Role::Body:
        return QVariant(item.second.body);
    case Role::Timestamp:
        return QVariant::fromValue(item.second.timestamp);
    case Role::Duration:
        return QVariant::fromValue(item.second.duration);
    case Role::Type:
        return QVariant(static_cast<int>(item.second.type));
    case Role::Status:
        return QVariant(static_cast<int>(item.second.status));
    case Role::IsRead:
        return QVariant(item.second.isRead);
    case Role::LinkPreviewInfo:
        return QVariant(item.second.linkPreviewInfo);
    case Role::Linkified:
        return QVariant(item.second.linkified);
    case Role::TransferName:
        return QVariant(item.second.commit["displayName"]);
    default:
        return {};
    }
}

QVariant
MessageListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount()) {
        return {};
    }
    return dataForItem(interactions_.at(index.row()), index.row(), role);
}

int
MessageListModel::getIndexOfMessage(const QString& messageId) const
{
    for (int i = 0; i < interactions_.size(); i++) {
        if (atIndex(i).first == messageId) {
            return i;
        }
    }
    return -1;
}

void
MessageListModel::addHyperlinkInfo(const QString& messageId, const QVariantMap& info)
{
    int index = getIndexOfMessage(messageId);
    if (index == -1) {
        return;
    }
    QModelIndex modelIndex = QAbstractListModel::index(index, 0);

    interactions_[index].second.linkPreviewInfo = info;
    Q_EMIT dataChanged(modelIndex, modelIndex, {Role::LinkPreviewInfo});
}

void
MessageListModel::linkifyMessage(const QString& messageId, const QString& linkified)
{
    int index = getIndexOfMessage(messageId);
    if (index == -1) {
        return;
    }
    QModelIndex modelIndex = QAbstractListModel::index(index, 0);
    interactions_[index].second.body = linkified;
    interactions_[index].second.linkified = true;
    Q_EMIT dataChanged(modelIndex, modelIndex, {Role::Body, Role::Linkified});
}

void
MessageListModel::emitBeginResetModel()
{
    Q_EMIT beginResetModel();
}

void
MessageListModel::emitEndResetModel()
{
    Q_EMIT endResetModel();
}

void
MessageListModel::emitDataChanged(iterator it, VectorInt roles)
{
    auto index = std::distance(begin(), it);
    QModelIndex modelIndex = QAbstractListModel::index(index, 0);
    Q_EMIT dataChanged(modelIndex, modelIndex, roles);
}

void
MessageListModel::emitDataChanged(const QString& msgId, VectorInt roles)
{
    int index = getIndexOfMessage(msgId);
    if (index == -1) {
        return;
    }
    QModelIndex modelIndex = QAbstractListModel::index(index, 0);
    Q_EMIT dataChanged(modelIndex, modelIndex, roles);
}

} // namespace lrc
