/*
 *  Copyright (C) 2020-2021 Savoir-faire Linux Inc.
 *
 *  Author: Kateryna Kostiuk <kateryna.kostiuk@savoirfairelinux.com>
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

#include <QCoreApplication>
#include <stdexcept>

#include "messagelistmodel.h"
#include "api/conversationmodel.h"
#include "api/interaction.h"
#include <QAbstractListModel>

namespace lrc {

using namespace api;

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
    beginInsertRows(QModelIndex(), interactions_.size(), interactions_.size());
    auto iterator = interactions_.insert(iter, qMakePair(msgId, message));
    endInsertRows();
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
    int indexCounter = 0;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == msgId) {
            beginRemoveRows(QModelIndex(), indexCounter, indexCounter);
            interactions_.erase(it);
            endRemoveRows();
            return 1;
        }
        indexCounter++;
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
    beginInsertRows(QModelIndex(), interactions_.size(), interactions_.size());
    interactions_.insert(interactions_.end(), qMakePair(messageId, newMessage));
    endInsertRows();
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
MessageListModel::clear()
{
    interactions_.clear();
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
MessageListModel::atIndex(int index)
{
    return interactions_.at(index);
}

QPair<iterator, bool>
MessageListModel::insert(int it, QPair<QString, interaction::Info> message)
{
    iterator itr;
    for (itr = interactions_.begin(); itr != interactions_.end(); ++itr) {
        if (itr->first == message.first) {
            return qMakePair(itr, false);
        }
    }
    if (it >= size()) {
        beginInsertRows(QModelIndex(), interactions_.size(), interactions_.size());
        auto iterator = interactions_.insert(interactions_.end(), message);
        endInsertRows();
        return qMakePair(iterator, true);
    }
    beginInsertRows(QModelIndex(), interactions_.size(), interactions_.size());
    interactions_.insert(it, message);
    endInsertRows();
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

    interactions_.move(currentIndex, newIndex);

    // move a child message
    if (!childMessageIdToMove.isEmpty()) {
        moveMessage(childMessageIdToMove, msgId);
    }
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
    MESS_ROLES
#undef X
    return roles;
}

void
MessageListModel::clearModel()
{
    interactions_.clear();
}

QString
MessageListModel::formatTimeString(const std::time_t& timeStamp) const
{
    auto currentTimeStamp = QDateTime::fromSecsSinceEpoch(timeStamp);
    auto now = QDateTime::currentDateTime();
    auto timeStampDMY = currentTimeStamp.toString("dd/MM/yy");
    if (timeStampDMY == now.toString("dd/MM/yy")) {
        return currentTimeStamp.toString("hh:mm");
    }
    return timeStampDMY;
}

QVariant
MessageListModel::dataForItem(item_t item, int indexRow, int role) const
{
    switch (role) {
    case Role::MessageId:
        return QVariant(item.first);
    case Role::Author:
        return QVariant(item.second.authorUri);
    case Role::MessageBody:
        return QVariant(item.second.body);
        //    case Role::ParentId:
        //        return QVariant(item.second.parentId);
    case Role::Timestamp:
        return QVariant(formatTimeString(item.second.timestamp));
    case Role::Duration:
        return QVariant(formatTimeString(item.second.duration));
    case Role::Type:
        return QVariant(static_cast<int>(item.second.type));
    case Role::Status:
        return QVariant(static_cast<int>(item.second.status));
        //    case Role::IsRead:
        //        return QVariant(item.second.isRead);
        //    case Role::Commit:
        //        return QVariant(item.second.commit);

    default:
        break;
    }
}

QVariant
MessageListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return {};
    return dataForItem(interactions_.at(index.row()), index.row(), role);
}

} // namespace lrc
