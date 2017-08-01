/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/





// Parent
#include "smartlistmodel.h"

// Debug
#include <qdebug.h>

// Lrc
#include <contactitem.h>

SmartListModel::SmartListModel()
{
    qDebug() << "{C} SmartListModel\n\n\n\n";
    
    // JN : ce qui suit n'est là que pour rapide exemple.
    // on ne stockera pas des item tels quels, on aura des contacts etc.
    
    items.push_back(new ContactItem(SmartListItemType::CONTACT));
    
}

SmartListModel::~SmartListModel()
{
    qDebug() << "{D} SmartListModel\n\n\n\n";
    // delete the items
    for (auto item : items)
        delete item;
}

std::list<SmartListItem*>
SmartListModel::getItems()
{
    return items;
}


#include <smartlistmodel.moc>
