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
#ifndef SECURITYVALIDATIONMODELPRIVATE_H
#define SECURITYVALIDATIONMODELPRIVATE_H

class SecurityFlaw;
class Account;
class Certificate;
class PrefixAndSeverityProxyModel;
class AccountChecksModel;

#include <certificate.h>
#include "private/matrixutils.h"
#include <securityevaluationmodel.h>

class SecurityEvaluationModelPrivate final : public QObject
{
   Q_OBJECT
public:
   SecurityEvaluationModelPrivate(Account* account, SecurityEvaluationModel* parent);

   //Attributes
   QList<SecurityFlaw*>  m_lCurrentFlaws       ;
   SecurityEvaluationModel::SecurityLevel m_CurrentSecurityLevel;
   Account*      m_pAccount            ;
   QHash< int, QHash< int, SecurityFlaw* > > m_hFlaws;
   bool         m_isScheduled;
   int          m_SeverityCount[enum_class_size<SecurityEvaluationModel::Severity>()];

   AccountChecksModel*          m_pAccChecks;

   //Helper
   static SecurityEvaluationModel::SecurityLevel maxSecurityLevel(QAbstractItemModel* m, int* counter = nullptr);
   static QAbstractItemModel* getCertificateSeverityProxy(Certificate* c);
   static SecurityEvaluationModel::SecurityLevel certificateSecurityLevel(Certificate* c, bool forceIgnorePrivateKey = false);

   ///Messages to show to the end user
   static const QString messages[enum_class_size<SecurityEvaluationModel::AccountSecurityChecks>()];

   //Static mapping
   static const TypedStateMachine< SecurityEvaluationModel::SecurityLevel , SecurityEvaluationModel::AccountSecurityChecks > maximumSecurityLevel;
   static const TypedStateMachine< SecurityEvaluationModel::Severity      , SecurityEvaluationModel::AccountSecurityChecks > flawSeverity        ;

   static const TypedStateMachine< SecurityEvaluationModel::SecurityLevel , Certificate::Checks > maximumCertificateSecurityLevel;
   static const TypedStateMachine< SecurityEvaluationModel::Severity      , Certificate::Checks > certificateFlawSeverity        ;

   SecurityEvaluationModel* q_ptr;

public Q_SLOTS:
   void update();
   void updateReal();

};

#endif
