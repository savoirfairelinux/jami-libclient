/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#include "securityflaw.h"

//Ring
#include "private/securityevaluationmodel_p.h"
#include "private/securityflaw_p.h"

SecurityFlawPrivate::SecurityFlawPrivate(SecurityFlaw* parent, SecurityEvaluationModel::AccountSecurityFlaw f,Certificate::Type type):m_flaw(f),m_certType(type),m_Row(-1)
,m_severity(SecurityEvaluationModelPrivate::flawSeverity[f]),q_ptr(parent)
{

}

SecurityFlaw::SecurityFlaw(SecurityEvaluationModel::AccountSecurityFlaw f,Certificate::Type type)
   : QObject(), d_ptr(new SecurityFlawPrivate(this, f, type))
{
}

Certificate::Type SecurityFlaw::type() const
{
   return d_ptr->m_certType;
}

SecurityEvaluationModel::AccountSecurityFlaw SecurityFlaw::flaw() const
{
   return d_ptr->m_flaw;
}

SecurityEvaluationModel::Severity SecurityFlaw::severity() const
{
   return d_ptr->m_severity;
}

void SecurityFlaw::requestHighlight()
{
   emit highlight();
}

bool SecurityFlaw::operator < ( const SecurityFlaw &r ) const {
   return ( (int)d_ptr->m_severity > (int)r.d_ptr->m_severity );
}

bool SecurityFlaw::operator > ( const SecurityFlaw &r ) const {
   return ( (int)d_ptr->m_severity < (int)r.d_ptr->m_severity );
}

#include <securityflaw.moc>
