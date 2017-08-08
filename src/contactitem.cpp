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
#include "smartlistmodel.h"
#include "database.h"

// Qt
#include <qstring.h>

// Debug
#include <qdebug.h>


ContactItem::ContactItem(ContactMethod* cm)
: SmartListItem()
{
    this->contact.uri = cm->uri().toStdString();
    this->contact.avatar = DataBase::instance().getAvatar(QString(this->contact.uri.c_str()));
    this->contact.id = cm->bestId().toStdString();
    this->contact.registeredName = cm->registeredName().toStdString();
    this->contact.displayName = cm->bestName().toStdString();
    this->contact.isPresent = false;
    this->contact.unreadMessages = 0;
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
ContactItem::getTitle() const
{
    return _title->data();
}

void
ContactItem::action()
{
    emit SmartListModel::instance().showConversationView(this);
}

const std::string
ContactItem::getAlias()
{
    return this->contact.displayName;
}

const std::string
ContactItem::getAvatar()
{
    return this->contact.avatar;
}

#include <contactitem.moc>
