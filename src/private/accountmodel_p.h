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
#ifndef ACCOUNTMODELPRIVATE_H
#define ACCOUNTMODELPRIVATE_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <account.h>
class AccountModel;
class AccountListColorVisitor;

class AccountModelPrivate : public QObject
{
   Q_OBJECT
   Q_DECLARE_PUBLIC(AccountModel)

   friend class AccountPrivate;
public:
   //Constructor
   AccountModelPrivate(AccountModel* parent);
   void init();
   void setupRoleName();

   //Helpers
   Account* firstRegisteredAccount() const;

   //Attributes
   AccountModel*            q_ptr             ;
   QVector<Account*>        m_lAccounts       ;
   AccountListColorVisitor* m_pColorVisitor   ;
   QStringList              m_lDeletedAccounts;
   Account*                 m_pIP2IP          ;

   //Future account cache
   static  QHash<QByteArray,AccountPlaceHolder*> m_hsPlaceHolder;

public Q_SLOTS:
   void slotAccountChanged(const QString& account,const QString& state, int code);
   void slotAccountChanged(Account* a);
   void slotVoiceMailNotify( const QString& accountID , int count );
   void slotAccountPresenceEnabledChanged(bool state);
   void slotVolatileAccountDetailsChange(const QString& accountId, const MapStringString& details);
};

#endif
