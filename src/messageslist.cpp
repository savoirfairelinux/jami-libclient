/*
 *  Copyright (C) 2020 Savoir-faire Linux Inc.
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

#include "messageslist.h"
#include "api/interaction.h"

namespace lrc {

using namespace api;

QPair<iterator, bool>
MessagesList::emplace(QString intId, interaction::Info interaction)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == intId) {
            return qMakePair(it, false);
        }
    }
    auto iterator = interactions_.insert(interactions_.end(), qMakePair(intId, interaction));
    return qMakePair(iterator, true);
}
iterator
MessagesList::find(QString intId)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == intId) {
            return it;
        }
    }
    return interactions_.end();
}

constIterator
MessagesList::find(QString intId) const
{
    constIterator it;
    for (it = interactions_.cbegin(); it != interactions_.cend(); ++it) {
        if (it->first == intId) {
            return it;
        }
    }
    return interactions_.cend();
}
QPair<iterator, bool>
MessagesList::insert(std::pair<QString, interaction::Info> interaction)
{
    return emplace(interaction.first, interaction.second);
}
QPair<iterator, bool>
MessagesList::insert(int it, QPair<QString, interaction::Info> interaction) {
    iterator itr;
    for (itr = interactions_.begin(); itr != interactions_.end(); ++itr) {
        if (itr->first == interaction.first) {
            return qMakePair(itr, false);
        }
    }
    if (it >= size()) {
        auto iterator = interactions_.insert(interactions_.end(), interaction);
        return qMakePair(iterator, true);
    }
    interactions_.insert(it, interaction);
    return qMakePair(interactions_.end(), true);
}
int
MessagesList::indexOfMsg(QString intId)
{
    for (int i = 0; i < interactions_.size(); i++) {
        auto iteration = interactions_[i];
        qDebug() << "*** iteration id" << iteration.first;
        qDebug() << "*** iteration body" << iteration.second.body;
        if (iteration.first == intId) {
            return i;
        }
    }
    return -1;
}
int
MessagesList::erase(QString intId)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == intId) {
            interactions_.erase(it);
            return 1;
        }
    }
}
interaction::Info
MessagesList::operator[](QString index)
{
    return at(index);
}

iterator
MessagesList::end()
{
    return interactions_.end();
}

void
MessagesList::clear()
{
    interactions_.clear();
}

bool
MessagesList::empty()
{
    return interactions_.empty();
}
reverseIterator
MessagesList::rbegin()
{
    return interactions_.rbegin();
}
iterator
MessagesList::begin()
{
    return interactions_.begin();
}

constIterator
MessagesList::cend() const
{
    return interactions_.cend();
}
interaction::Info
MessagesList::at(QString intId)
{
    iterator it;
    for (it = interactions_.begin(); it != interactions_.end(); ++it) {
        if (it->first == intId) {
            return it->second;
        }
    }
    return {};
}

constIterator
MessagesList::end() const
{
    return interactions_.cend();
}
constIterator
MessagesList::begin() const
{
    return interactions_.cbegin();
}
int
MessagesList::size() const
{
    return interactions_.size();
}

void
MessagesList::moveMsg(QList<QString> msgIds, QString parentId)
{
    for (auto msgId : msgIds) {
        int currentIndex = indexOfMsg(msgId);
        int newIndex = indexOfMsg(parentId) + 1 < interactions_.size() ? indexOfMsg(parentId) + 1 : interactions_.size() - 1;
        interactions_.move(currentIndex, newIndex);
    }
}

QPair<QString, interaction::Info>
MessagesList::front()
{
    return interactions_.front();
}

QPair<QString, interaction::Info>
MessagesList::last()
{
    return interactions_.last();
}
} // namespace lrc

