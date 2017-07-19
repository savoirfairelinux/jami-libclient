/****************************************************************************
 *   Copyright (C) 2015-2017 Savoir-faire Linux                               *
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
#include "accountstatusmodel.h"

//System
#include <errno.h>

#ifdef Q_OS_WIN
#include <winerror.h>
#define ESHUTDOWN WSAESHUTDOWN
#define ENODATA WSANO_DATA
#define ETIME WSAETIMEDOUT
#define EPFNOSUPPORT WSAEPROTONOSUPPORT
#define EHOSTDOWN WSAEHOSTDOWN
#define ESTALE WSAESTALE
#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#define ETOOMANYREFS WSAETOOMANYREFS
#define EUSERS WSAEUSERS
#define EBADMSG 9905
#define ENOLINK 9918
#define ENOSR 9922
#define ENOSTR 9924
#define EMULTIHOP 2004
#endif

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>

//Ring daemon
#include <account_const.h>

//Ring
#include "dbus/configurationmanager.h"
#include "account.h"

struct AccountStatusRow {
   AccountStatusRow(const QString& ,int, AccountStatusModel::Type);
   QString                  description;
   int                      code       ;
   QDateTime                time       ;
   time_t                   timestamp  ;
   uint                     counter    ;
   AccountStatusModel::Type type       ;
};

class AccountStatusModelPrivate {
public:
   AccountStatusModelPrivate(Account* parent);
   ~AccountStatusModelPrivate();

   //Attributes
   Account* m_pAccount;
   QVector<AccountStatusRow*> m_lRows;
   time_t m_FallbackTime_t;
};


AccountStatusRow::AccountStatusRow(const QString& _description, int _code, AccountStatusModel::Type _type):
code(_code),counter(0),
time(QDateTime::currentDateTime()),type(_type)
{
    description = _description;
    timestamp   = time.toTime_t();
}

AccountStatusModelPrivate::AccountStatusModelPrivate(Account* parent) : m_pAccount(parent),
m_FallbackTime_t(QDateTime::currentDateTime().toTime_t())
{
}

AccountStatusModelPrivate::~AccountStatusModelPrivate()
{
   for (int i=0;i<m_lRows.size();i++)
      delete m_lRows[i];
   m_lRows.clear();
}

AccountStatusModel::AccountStatusModel(Account* parent) : QAbstractTableModel(parent),
d_ptr(new AccountStatusModelPrivate(parent))
{}

AccountStatusModel::~AccountStatusModel()
{
   delete d_ptr;
}

QHash<int,QByteArray> AccountStatusModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

//Model functions
QVariant AccountStatusModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   switch(static_cast<Columns>(index.column())) {
      case Columns::DESCRIPTION:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->description;
         };
         break;
      case Columns::CODE:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->code;
         };
         break;
      case Columns::TIME:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->time;
         };
         break;
      case Columns::COUNTER:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->counter;
         };
         break;
   };
   return QVariant();
}

int AccountStatusModel::rowCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return d_ptr->m_lRows.size();
}

int AccountStatusModel::columnCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return 4;
}

Qt::ItemFlags AccountStatusModel::flags( const QModelIndex& index ) const
{
   return (index.isValid()) ? (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsUserCheckable) : Qt::NoItemFlags;
}

bool AccountStatusModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

QVariant AccountStatusModel::headerData( int section, Qt::Orientation o, int role) const
{
   if (o == Qt::Horizontal && role == Qt::DisplayRole) {
      switch(section) {
         case 0:
            return QObject::tr("Message");
         case 1:
            return QObject::tr("Code");
         case 2:
            return QObject::tr("Time");
         case 3:
            return QObject::tr("Counter");
      }
   }
   return QVariant();
}

void AccountStatusModel::addSipRegistrationEvent(const QString& fallbackMessage, int errorCode)
{
   if (errorCode != d_ptr->m_pAccount->lastErrorCode()) {
      beginInsertRows(QModelIndex(), d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
      d_ptr->m_lRows << new AccountStatusRow(fallbackMessage, errorCode, Type::SIP);
      endInsertRows();
   }
   else
      d_ptr->m_lRows.last()->counter++;
}

void AccountStatusModel::addTransportEvent(const QString& fallbackMessage, int errorCode)
{
   if ((!d_ptr->m_lRows.size()) || errorCode != d_ptr->m_pAccount->lastTransportErrorCode()) {
      beginInsertRows(QModelIndex(), d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
      d_ptr->m_lRows << new AccountStatusRow(fallbackMessage, errorCode, Type::TRANSPORT);
      endInsertRows();
   }
   else
      d_ptr->m_lRows.last()->counter++;
}

QString AccountStatusModel::lastErrorMessage() const
{
   if (d_ptr->m_lRows.isEmpty())
      return QString();

   return d_ptr->m_lRows.last()->description;
}

int AccountStatusModel::lastErrorCode() const
{
   if (d_ptr->m_lRows.isEmpty())
      return -1;

   return d_ptr->m_lRows.last()->code;
}

time_t AccountStatusModel::lastTimeStamp() const
{
   if (d_ptr->m_lRows.isEmpty())
      return d_ptr->m_FallbackTime_t;

   return d_ptr->m_lRows.last()->timestamp;
}
