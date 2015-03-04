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
#include "certificateserializationdelegate.h"

#include <QtCore/QUrl>

CertificateSerializationDelegate* CertificateSerializationDelegate::m_spInstance = new CertificateSerializationDelegate();

CertificateSerializationDelegate* CertificateSerializationDelegate::instance()
{
   return m_spInstance;
}

void CertificateSerializationDelegate::setInstance(CertificateSerializationDelegate* visitor)
{
   m_spInstance = visitor;
}

QByteArray CertificateSerializationDelegate::loadCertificate(const QByteArray& id)
{
   Q_UNUSED(id)
   return QByteArray();
}

QUrl CertificateSerializationDelegate::saveCertificate(const QByteArray& id, const QByteArray& content)
{
   Q_UNUSED(id)
   Q_UNUSED(content)
   return QUrl();
}

QList<QByteArray> CertificateSerializationDelegate::listCertificates()
{
   return QList<QByteArray>();
}

bool CertificateSerializationDelegate::deleteCertificate(const QByteArray& id)
{
   Q_UNUSED(id)
   return false;
}
