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

//Qt
#include <QtCore/QObject>
#include <QtCore/QStringList>

//Ring
#include <account.h>
#include <accountmodel.h>
#include "matrixutils.h"
class AccountModel;
class AccountListColorDelegate;
class ProtocolModel;
class QItemSelectionModel;

class AccountModelPrivate final : public QObject
{
   Q_OBJECT
   Q_DECLARE_PUBLIC(AccountModel)

   friend class AccountPrivate;
   friend class AvailableAccountModel;
public:
   //Constructor
   explicit AccountModelPrivate(AccountModel* parent);
   void init();

   //Helpers
   static Account::RegistrationState fromDaemonName(const QString& st);
   void enableProtocol(Account::Protocol proto);
   AccountModel::EditState convertAccountEditState(const Account::EditState s);
   void insertAccount(Account* a, int idx);

   //Attributes
   AccountModel*                     q_ptr                ;
   QVector<Account*>                 m_lAccounts          ;
   AccountListColorDelegate*         m_pColorDelegate     ;
   QStringList                       m_lDeletedAccounts   ;
   Account*                          m_pIP2IP             ;
   QList<Account*>                   m_pRemovedAccounts   ;
   static AccountModel*              m_spAccountList      ;
   ProtocolModel*                    m_pProtocolModel     ;
   QItemSelectionModel*              m_pSelectionModel    ;
   QStringList                       m_lMimes             ;
   QList<Account*>                   m_lSipAccounts       ;
   QList<Account*>                   m_lIAXAccounts       ;
   QList<Account*>                   m_lRingAccounts      ;
   Matrix1D<Account::Protocol, bool> m_lSupportedProtocols;

   //Future account cache
   static QHash<QByteArray,AccountPlaceHolder*> m_hsPlaceHolder;

public Q_SLOTS:
   void slotDaemonAccountChanged(const QString& account, const QString&  registration_state, unsigned code, const QString& status);
   void slotAccountChanged(Account* a);
   void slotSupportedProtocolsChanged();
   void slotVoiceMailNotify( const QString& accountID , int count );
   void slotAccountPresenceEnabledChanged(bool state);
   void slotVolatileAccountDetailsChange(const QString& accountId, const MapStringString& details);
   void slotIncomingTrustRequest(const QString& accountId, const QString& hash, const QByteArray& payload, time_t time);
};

#endif
