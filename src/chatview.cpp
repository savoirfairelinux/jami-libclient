/****************************************************************************
 *    Copyright (C) 2020-2022 Savoir-faire Linux Inc.                       *
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
#include "api/chatview.h"

namespace lrc {

namespace api {

namespace chatview {

QVariantMap
getTranslatedStrings(bool qwebview)
{
    return {
        {"Hide chat view", QObject::tr("Hide chat view")},
        {"Place video call", QObject::tr("Place video call")},
        {"Show available plugins", QObject::tr("Show available plugins")},
        {"Place audio call", QObject::tr("Place audio call")},
        {"Add to conversations", QObject::tr("Add to conversations")},
        {"Unban contact", QObject::tr("Unban contact")},
        {"Send", QObject::tr("Send")},
        {"Options", QObject::tr("Options")},
        {"Jump to latest", QObject::tr("Jump to latest")},
        {"Send file", QObject::tr("Send file")},
        {"Add emoji", QObject::tr("Add emoji")},
        {"Leave video message", QObject::tr("Leave video message")},
        {"Leave audio message", QObject::tr("Leave audio message")},
        {"Block", QObject::tr("Block")},
        {"Copy to downloads", QObject::tr("Copy to downloads")},
        {"Write to {0}", QObject::tr("Write to {0}")},
        {"Note: an interaction will create a new contact.",
         QObject::tr("Note: an interaction will create a new contact.")},
        {"is not in your contacts", QObject::tr("is not in your contacts")},
        {"has sent you a conversation request.",
         QObject::tr("has sent you a conversation request.")},
        {"Hello, do you want to join the conversation?",
         QObject::tr("Hello, do you want to join the conversation?")},
        {"You have accepted the conversation request.",
         QObject::tr("You have accepted the conversation request.")},
        {"We are waiting for another device to synchronize the conversation.",
         QObject::tr("We are waiting for another device to synchronize the conversation.")},
        {"Note: you can automatically accept this invitation by sending a message.",
         QObject::tr("Note: you can automatically accept this invitation by sending a message.")},
        {"%d days ago", qwebview ? QObject::tr("{0} days ago") : QObject::tr("%d days ago")},
        {"%d hours ago", qwebview ? QObject::tr("{0} hours ago") : QObject::tr("%d hours ago")},
        {"%d minutes ago",
         qwebview ? QObject::tr("{0} minutes ago") : QObject::tr("%d minutes ago")},
        {"one day ago", QObject::tr("one day ago")},
        {"one hour ago", QObject::tr("one hour ago")},
        {"just now", QObject::tr("just now")},
        {"Failure", QObject::tr("Failure")},
        {"Accept", QObject::tr("Accept")},
        {"Refuse", QObject::tr("Refuse")},
        {"Delete", QObject::tr("Delete")},
        {"Retry", QObject::tr("Retry")},
        {"unjoinable peer", QObject::tr("Unable to make contact")},
        {"connecting", QObject::tr("Connecting")},
        {"accepted", QObject::tr("Accepted")},
        {"canceled", QObject::tr("Canceled")},
        {"ongoing", QObject::tr("Ongoing")},
        {"awaiting peer", QObject::tr("Waiting for contact")},
        {"awaiting host", QObject::tr("Incoming transfer")},
        {"awaiting peer timeout", QObject::tr("Timed out waiting for contact")},
        {"finished", QObject::tr("Finished")},
    };
}

} // namespace chatview
} // namespace api
} // namespace lrc
