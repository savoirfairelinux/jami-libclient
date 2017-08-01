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
#include "smartlistitem.h"



// Debug
#include <qdebug.h>


SmartListItem::SmartListItem(SmartListItemType smartListItemType)
{
    qDebug() << "{C} SmartListItem";
    _title = new std::string("_title (fallback)");
    //~ std::unique_ptr<int> v1 = std::make_unique<int>();
    
    qDebug() << "TITLE : " << _title->data();

    qDebug() << "\n\n\n\n";
    
    _smartListItemType = smartListItemType;
    
}

SmartListItem::~SmartListItem()
{
    qDebug() << "{D} SmartListItem\n\n\n\n";
    delete _title;
}

//~ void
//~ SmartListItem::setTitle(const std::string)
//~ {
    
//~ }

//~ const std::string
//~ SmartListItem::getTitle()
//~ {
    //~ return _title->data();
//~ }


#include <smartlistitem.moc>
