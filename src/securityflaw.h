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
#ifndef SECURITY_FLAW_H
#define SECURITY_FLAW_H

#include <QtCore/QObject>

#include <typedefs.h>

#include <securityevaluationmodel.h>

class SecurityFlawPrivate;

///A flaw representation
class LIB_EXPORT SecurityFlaw : public QObject
{
   Q_OBJECT
   friend class SecurityEvaluationModel;
   friend class SecurityEvaluationModelPrivate;
public:

   //Operators
   bool operator < ( const SecurityFlaw &r ) const;
   bool operator > ( const SecurityFlaw &r ) const;

   //Getter
   Certificate::Type type() const;
   SecurityEvaluationModel::AccountSecurityChecks flaw() const;
   SecurityEvaluationModel::Severity severity() const;

private:
   explicit SecurityFlaw(SecurityEvaluationModel::AccountSecurityChecks f,Certificate::Type type = Certificate::Type::NONE);

   SecurityFlawPrivate* d_ptr;
   Q_DECLARE_PRIVATE(SecurityFlaw)

public Q_SLOTS:
   void requestHighlight();

Q_SIGNALS:
   void solved();
   void highlight();
};

#endif