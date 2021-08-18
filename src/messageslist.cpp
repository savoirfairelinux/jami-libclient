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

#include "messageslist.h"
#include "api/interaction.h"

namespace lrc {

using namespace api;

QPair<iterator, bool>
MessagesList::emplace(QString msgId, interaction::Info message, bool beginning)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == msgId) {
            return qMakePair(it, false);
        }
    }
    auto iter = beginning ? interactions_.begin() : interactions_.end();
    auto iterator = interactions_.insert(iter, qMakePair(msgId, message));
    return qMakePair(iterator, true);
}
iterator
MessagesList::find(QString msgId)
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
MessagesList::find(QString msgId) const
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
MessagesList::insert(std::pair<QString, interaction::Info> message, bool beginning)
{
    return emplace(message.first, message.second, beginning);
}

int
MessagesList::erase(QString msgId)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == msgId) {
            interactions_.erase(it);
            return 1;
        }
    }
    return 0;
}

interaction::Info&
MessagesList::operator[](QString messageId)
{
    for (auto it = interactions_.cbegin(); it != interactions_.cend(); ++it) {
        if (it->first == messageId) {
            return const_cast<interaction::Info&>(it->second);
        }
    }
    // element not find, add it to the end
    interaction::Info newMessage = {};
    interactions_.insert(interactions_.end(), qMakePair(messageId, newMessage));
    if (interactions_.last().first == messageId) {
        return const_cast<interaction::Info&>(interactions_.last().second);
    }
    throw std::out_of_range("Cannot find message");
}

iterator
MessagesList::end()
{
    return interactions_.end();
}

constIterator
MessagesList::end() const
{
    return interactions_.end();
}

constIterator
MessagesList::cend() const
{
    return interactions_.cend();
}

iterator
MessagesList::begin()
{
    return interactions_.begin();
}

constIterator
MessagesList::begin() const
{
    return interactions_.begin();
}

reverseIterator
MessagesList::rbegin()
{
    return interactions_.rbegin();
}

int
MessagesList::size() const
{
    return interactions_.size();
}

void
MessagesList::clear()
{
    interactions_.clear();
}

bool
MessagesList::empty() const
{
    return interactions_.empty();
}

interaction::Info
MessagesList::at(QString msgId) const
{
    for (auto it = interactions_.cbegin(); it != interactions_.cend(); ++it) {
        if (it->first == msgId) {
            return it->second;
        }
    }
    return {};
}

QPair<QString, interaction::Info>
MessagesList::front() const
{
    return interactions_.front();
}

QPair<QString, interaction::Info>
MessagesList::last() const
{
    return interactions_.last();
}

QPair<QString, interaction::Info>
MessagesList::atIndex(int index)
{
    return interactions_.at(index);
}

QPair<iterator, bool>
MessagesList::insert(int it, QPair<QString, interaction::Info> message)
{
    iterator itr;
    for (itr = interactions_.begin(); itr != interactions_.end(); ++itr) {
        if (itr->first == message.first) {
            return qMakePair(itr, false);
        }
    }
    if (it >= size() - 1) {
        auto iterator = interactions_.insert(interactions_.end(), message);
        return qMakePair(iterator, true);
    } else if (it < 0) {
        auto iterator = interactions_.insert(interactions_.begin(), message);
        return qMakePair(iterator, true);
    }
    interactions_.insert(it, message);
    return qMakePair(interactions_.end(), true);
}

int
MessagesList::indexOfMessage(QString msgId, bool reverse) const
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

int
MessagesList::indexOfChildForMessage(QString msgId, bool reverse) const
{
    auto getIndex = [reverse, &msgId](const auto& start, const auto& end) -> int {
        auto it = std::find_if(start, end, [&msgId](const auto& it) {
            return it.second.parentId == msgId;
        });
        if (it == end) {
            return -1;
        }
        return reverse ? std::distance(it, end) - 1 : std::distance(start, it);
    };
    return reverse ? getIndex(interactions_.rbegin(), interactions_.rend())
                   : getIndex(interactions_.begin(), interactions_.end());
}

void
MessagesList::moveMessages(QList<QString> msgIds, QString parentId)
{
    for (auto msgId : msgIds) {
        moveMessage(msgId, parentId);
    }
}

void
MessagesList::moveMessage(QString msgId, QString parentId)
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
} // namespace lrc
