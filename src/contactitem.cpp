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
#include "contactitem.h"



// Debug
#include <qdebug.h>


ContactItem::ContactItem()
: SmartListItem()
{
    qDebug() << "{C} ContactItem";
    _title = new std::string("_Xtitle (fallback)");
    //~ std::unique_ptr<int> v1 = std::make_unique<int>();
    
    qDebug() << "TITLE : " << _title->data();

    qDebug() << "\n\n\n\n";
}

ContactItem::~ContactItem()
{
    qDebug() << "{D} ContactItem\n\n\n\n";
    delete _title;
}

void
ContactItem::setTitle(const std::string title)
{
    qDebug() << ":G: " << title.data();
    _title->assign(title);
}

const std::string
ContactItem::getTitle()
{
    return _title->data();
}


#include <contactitem.moc>
