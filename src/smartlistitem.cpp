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

SmartListItem::SmartListItem()
: _title(new std::string())
{
}

SmartListItem::~SmartListItem()
{
}

void
SmartListItem::setTitle(const std::string& newTitle)
{
    if (not _title)
        _title = std::unique_ptr<std::string>(new std::string());

    _title->assign(newTitle);
    emit changed(this);
}


const std::string
SmartListItem::getTitle() const
{
    return _title->data();
}

#include <smartlistitem.moc>
