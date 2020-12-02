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
    QPair<iterator, bool> emplace(QString intId, interaction::Info interaction);
    iterator find(QString intId);
    constIterator find(QString intId) const;
    QPair<iterator, bool> insert(std::pair<QString, interaction::Info> interaction);
    int erase(QString intId);
    interaction::Info operator[](QString);
    iterator end();
    constIterator end() const;
    constIterator cend() const;
    iterator begin();
    constIterator begin() const;
    reverseIterator rbegin();
    int size() const;
    void clear();
    bool empty();
    interaction::Info at(QString intId);
    QPair<QString, interaction::Info> front();
    QPair<QString, interaction::Info> last();
    // jami functions
    QPair<iterator, bool> insert(int it, QPair<QString, interaction::Info> interaction);
    int indexOfMsg(QString intId, bool searchBackward = true);
    void moveMsg(QList<QString> msgIds, QString parentId);
    
private:
    QList<QPair<QString, interaction::Info>> interactions_;
};
} // namespace api
} // namespace lrc
