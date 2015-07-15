/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#ifndef IMCONVERSATIONMANAGERPRIVATE_H
#define IMCONVERSATIONMANAGERPRIVATE_H

#include <QtCore/QObject>
#include <QtCore/QHash>

class Account;
class Call;
class ContactMethod;

namespace Media {
   class Text;
   class TextRecording;
}

class IMConversationManagerPrivate final : public QObject
{
   Q_OBJECT
public:
   friend class Call;
   IMConversationManagerPrivate(QObject* parent);

   static IMConversationManagerPrivate* instance();

private Q_SLOTS:
   void newMessage       (const QString& callId   , const QString& from, const QMap<QString,QString>& message);
   void newAccountMessage(const QString& accountId, const QString& from, const QString& message);

};

#endif