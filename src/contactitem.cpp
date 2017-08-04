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

// Lrc
#include "database.h"
#include "smartlistmodel.h"

// Qt
#include <qstring.h>

// Debug
#include <qdebug.h>


ContactItem::ContactItem()
: SmartListItem()
{
}

ContactItem::~ContactItem()
{
}

void
ContactItem::setTitle(const std::string title)
{
    if (not _title)
        _title = std::unique_ptr<std::string>(new std::string());

    _title->assign(title);
}

const std::string
ContactItem::getTitle()
{
    return _title->data();
}

void
ContactItem::action()
{
    qDebug() << "tof";
    emit SmartListModel::instance().showConversationView(DataBase::instance().getMessages(QString(_title->c_str())));
}

#include <contactitem.moc>
