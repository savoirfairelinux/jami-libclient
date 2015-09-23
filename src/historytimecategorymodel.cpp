/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#include "historytimecategorymodel.h"

#include <QtCore/QDate>

class HistoryTimeCategoryModelPrivate
{
public:
   QVector<QString> m_lCategories;
   static HistoryTimeCategoryModel* instance();
};

HistoryTimeCategoryModel* HistoryTimeCategoryModelPrivate::instance()
{
   static auto m_spInstance = new HistoryTimeCategoryModel();
   return m_spInstance;
}

HistoryTimeCategoryModel::HistoryTimeCategoryModel(QObject* parent) : QAbstractListModel(parent),
d_ptr(new HistoryTimeCategoryModelPrivate)
{
   d_ptr->m_lCategories << tr("Today")                                 ;//0
   d_ptr->m_lCategories << tr("Yesterday")                             ;//1
   d_ptr->m_lCategories << QDate::currentDate().addDays(-2).toString("dddd");//2
   d_ptr->m_lCategories << QDate::currentDate().addDays(-3).toString("dddd");//3
   d_ptr->m_lCategories << QDate::currentDate().addDays(-4).toString("dddd");//4
   d_ptr->m_lCategories << QDate::currentDate().addDays(-5).toString("dddd");//5
   d_ptr->m_lCategories << QDate::currentDate().addDays(-6).toString("dddd");//6
   d_ptr->m_lCategories << tr("Last week")                             ;//7
   d_ptr->m_lCategories << tr("Two weeks ago")                         ;//8
   d_ptr->m_lCategories << tr("Three weeks ago")                       ;//9
   d_ptr->m_lCategories << tr("Last month")                            ;//10
   d_ptr->m_lCategories << tr("Two months ago")                        ;//11
   d_ptr->m_lCategories << tr("Three months ago")                      ;//12
   d_ptr->m_lCategories << tr("Four months ago")                       ;//13
   d_ptr->m_lCategories << tr("Five months ago")                       ;//14
   d_ptr->m_lCategories << tr("Six months ago")                        ;//15
   d_ptr->m_lCategories << tr("Seven months ago")                      ;//16
   d_ptr->m_lCategories << tr("Eight months ago")                      ;//17
   d_ptr->m_lCategories << tr("Nine months ago")                       ;//18
   d_ptr->m_lCategories << tr("Ten months ago")                        ;//19
   d_ptr->m_lCategories << tr("Eleven months ago")                     ;//20
   d_ptr->m_lCategories << tr("Twelve months ago")                     ;//21
   d_ptr->m_lCategories << tr("Last year")                             ;//22
   d_ptr->m_lCategories << tr("Very long time ago")                    ;//23
   d_ptr->m_lCategories << tr("Never")                                 ;//24
}

HistoryTimeCategoryModel::~HistoryTimeCategoryModel()
{
   delete d_ptr;
}

QHash<int,QByteArray> HistoryTimeCategoryModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

//Abstract model member
QVariant HistoryTimeCategoryModel::data(const QModelIndex& index, int role ) const
{
   if (!index.isValid()) return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         return d_ptr->m_lCategories[index.row()];
   }
   return QVariant();
}

int HistoryTimeCategoryModel::rowCount(const QModelIndex& parent ) const
{
   if (parent.isValid()) return 0;
   return d_ptr->m_lCategories.size();
}

Qt::ItemFlags HistoryTimeCategoryModel::flags(const QModelIndex& index ) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool HistoryTimeCategoryModel::setData(const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}


QString HistoryTimeCategoryModel::timeToHistoryCategory(const time_t time)
{
   int period = (int)HistoryTimeCategoryModel::timeToHistoryConst(time);
   if (period >= 0 && period <= 24)
      return HistoryTimeCategoryModelPrivate::instance()->d_ptr->m_lCategories[period];
   else
      return HistoryTimeCategoryModelPrivate::instance()->d_ptr->m_lCategories[24];
}

HistoryTimeCategoryModel::HistoryConst HistoryTimeCategoryModel::timeToHistoryConst(const time_t time)
{
   if (!time || time < 0)
      return HistoryTimeCategoryModel::HistoryConst::Never;

   time_t currentTime;
   ::time(&currentTime);
   struct tm localCurrentTime = {};
   struct tm localPastTime = {};

   ::localtime_r(&currentTime, &localCurrentTime);
   ::gmtime_r(&time, &localPastTime);

   int diffYears = localCurrentTime.tm_year - localPastTime.tm_year;
   int diffMonths = localCurrentTime.tm_mon - localPastTime.tm_mon;
   int diffDays = localCurrentTime.tm_mday - localPastTime.tm_mday;

   printf ("Current local time and date: %s", ::asctime(&localCurrentTime));
   printf ("past local time and date: %s", ::asctime(&localPastTime));

   qDebug() << "diffYears" << diffYears << "diffMonths" << diffMonths << "diffDays" << diffDays;

   //Check for last week
   if (diffYears == 0 && diffMonths == 0 && diffDays < 7) {
      return (HistoryTimeCategoryModel::HistoryConst)(diffDays); //Today to Six_days_ago
   }
   //Check for last month
   else if (diffYears == 0 && diffMonths == 0) {
      return (HistoryTimeCategoryModel::HistoryConst)(diffDays / 7 + ((int)HistoryTimeCategoryModel::HistoryConst::Last_week)); //Last_week to Three_weeks_ago
   }
   //Check for last year
   else if (diffYears == 0 && diffMonths > 0) {
      return (HistoryTimeCategoryModel::HistoryConst)(diffMonths + ((int)HistoryTimeCategoryModel::HistoryConst::Last_month)); //Last_month to Twelve_months ago
   }
   else if (diffYears == 1)
      return HistoryConst::Last_year;

   //Every other senario
   return HistoryTimeCategoryModel::HistoryConst::Very_long_time_ago;
}

QString HistoryTimeCategoryModel::indexToName(int idx)
{
   if (idx > 24) return HistoryTimeCategoryModelPrivate::instance()->d_ptr->m_lCategories[24];
   return HistoryTimeCategoryModelPrivate::instance()->d_ptr->m_lCategories[idx];
}
