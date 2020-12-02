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
#pragma once

namespace lrc {
namespace api {

namespace interaction {
struct Info;
}

typedef QList<QPair<QString, interaction::Info>>::ConstIterator constIterator;
typedef QList<QPair<QString, interaction::Info>>::Iterator iterator;
typedef QList<QPair<QString, interaction::Info>>::reverse_iterator reverseIterator;
class MessagesList
{
public:
    // map functions
    QPair<iterator, bool> emplace(QString msgId, interaction::Info message, bool beginning = false);
    iterator find(QString msgId);
    constIterator find(QString msgId) const;
    QPair<iterator, bool> insert(std::pair<QString, interaction::Info> message,
                                 bool beginning = false);
    int erase(QString msgId);
    interaction::Info& operator[](QString);
    iterator end();
    constIterator end() const;
    constIterator cend() const;
    iterator begin();
    constIterator begin() const;
    reverseIterator rbegin();
    int size() const;
    void clear();
    bool empty() const;
    interaction::Info at(QString intId) const;
    QPair<QString, interaction::Info> front() const;
    QPair<QString, interaction::Info> last() const;
    QPair<QString, interaction::Info> atIndex(int index);
    // jami functions
    QPair<iterator, bool> insert(int it, QPair<QString, interaction::Info> message);
    int indexOfMessage(QString msgId, bool reverse = true) const;
    void moveMessages(QList<QString> msgIds, QString parentId);

private:
    QList<QPair<QString, interaction::Info>> interactions_;
    void moveMessage(QString msgId, QString parentId);
};
} // namespace api
} // namespace lrc
