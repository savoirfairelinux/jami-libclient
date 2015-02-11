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
#include "ciphermodel.h"

//Qt
#include <QtCore/QCoreApplication>

//Ring daemon
#include <account_const.h>

//Ring
#include "dbus/configurationmanager.h"
#include "account.h"
#include "private/account_p.h"

class CipherModelPrivate {
public:
   CipherModelPrivate(Account* parent);
   ~CipherModelPrivate();

   //Attributes
   bool*                      m_lChecked          ;
   Account*                   m_pAccount          ;
   static QVector<QByteArray> m_slSupportedCiphers;
   static QHash<QString,int>  m_shMapping         ;
   static bool                m_sIsLoaded         ;

   static void loadCiphers();
};

bool CipherModelPrivate::m_sIsLoaded = false;
QVector<QByteArray> CipherModelPrivate::m_slSupportedCiphers;
QHash<QString,int>  CipherModelPrivate::m_shMapping;

CipherModelPrivate::CipherModelPrivate(Account* parent) : m_pAccount(parent)
{
   if (!CipherModelPrivate::m_sIsLoaded) {
      const QStringList cs = DBus::ConfigurationManager::instance().getSupportedCiphers(Account::ProtocolName::IP2IP);
      foreach(const QString& c, cs)
         m_slSupportedCiphers << c.toLatin1();
      m_sIsLoaded = true;
   }

   m_lChecked = new bool[m_slSupportedCiphers.size()]{};

   foreach(const QString& cipher, parent->d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::CIPHERS).split(' ')) {
      m_lChecked[m_shMapping[cipher]] = true;
   }
}

CipherModelPrivate::~CipherModelPrivate()
{
   delete[] m_lChecked;
}

void CipherModelPrivate::loadCiphers()
{
   const QStringList cs = DBus::ConfigurationManager::instance().getSupportedCiphers("IP2IP");
   foreach(const QString& c, cs)
      m_slSupportedCiphers << c.toLatin1();
   m_sIsLoaded = true;
}

CipherModel::CipherModel(Account* parent) : QAbstractListModel(parent),
d_ptr(new CipherModelPrivate(parent))
{}

CipherModel::~CipherModel()
{}

//Model functions
QVariant CipherModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         return CipherModelPrivate::m_slSupportedCiphers[index.row()];
      case Qt::CheckStateRole:
         return d_ptr->m_lChecked[index.row()]?Qt::Checked:Qt::Unchecked;
   };
   return QVariant();
}

int CipherModel::rowCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return d_ptr->m_slSupportedCiphers.size();
}

Qt::ItemFlags CipherModel::flags( const QModelIndex& index ) const
{
   return (index.isValid()) ? (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsUserCheckable) : Qt::NoItemFlags;
}

bool CipherModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   if (index.isValid() && role == Qt::CheckStateRole) {
      d_ptr->m_lChecked[index.row()] = value == Qt::Checked;
      emit dataChanged(index,index);
      QStringList ciphers;
      for(int i =0; i< d_ptr->m_slSupportedCiphers.size();i++) {
         if (d_ptr->m_lChecked[i])
            ciphers << d_ptr->m_slSupportedCiphers[i];
      }
      d_ptr->m_pAccount->d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::CIPHERS,ciphers.join(QString(' ')));
      return true;
   }
   return false;
}
