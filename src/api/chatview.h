/****************************************************************************
 *    Copyright (C) 2020 Savoir-faire Linux Inc.                            *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#pragma once

// Qt
#include <QObject>
#include <QVariantMap>
#include <QString>

namespace lrc {

namespace api {

namespace chatview {

QVariantMap
getTranslatedStrings()
{
    return {
        {"Hide chat view", QObject::tr("Hide chat view")},
        {"Place video call", QObject::tr("Place video call")},
        {"Place audio call", QObject::tr("Place audio call")},
        {"Add to conversations", QObject::tr("Add to conversations")},
        {"Unban contact", QObject::tr("Unban contact")},
        {"Send", QObject::tr("Send")},
        {"Options", QObject::tr("Options")},
        {"Jump to latest", QObject::tr("Jump to latest")},
        {"Send file", QObject::tr("Send file")},
        {"Leave video message", QObject::tr("Leave video message")},
        {"Leave audio message", QObject::tr("Leave audio message")},
        {"Accept", QObject::tr("Accept")},
        {"Refuse", QObject::tr("Refuse")},
        {"Block", QObject::tr("Block")},
        {"Type a message", QObject::tr("Type a message")},
        {"Note: an interaction will create a new contact.",
         QObject::tr("Note: an interaction will create a new contact.")},
        {"is not in your contacts", QObject::tr("is not in your contacts")},
        {"Note: you can automatically accept this invitation by sending a message.",
         QObject::tr("Note: you can automatically accept this invitation by sending a message.")},
        {"%d days ago", QObject::tr("%d days ago")},
        {"one day ago", QObject::tr("one day ago")},
        {"%d hours ago", QObject::tr("%d hours ago")},
        {"one hour ago", QObject::tr("one hour ago")},
        {"%d minutes ago", QObject::tr("%d minutes ago")},
        {"just now", QObject::tr("just now")},
        {"Failure", QObject::tr("Failure")},
        {"Accept", QObject::tr("Accept")},
        {"Refuse", QObject::tr("Refuse")},
        {"Delete", QObject::tr("Delete")},
        {"Retry", QObject::tr("Retry")},
    };
}

} // namespace chatview
} // namespace api
} // namespace lrc
