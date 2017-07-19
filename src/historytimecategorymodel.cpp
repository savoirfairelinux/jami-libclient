/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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
#include <time.h>

class HistoryTimeCategoryModelPrivate
{
public:
   QVector<QString> m_lCategories;
   static HistoryTimeCategoryModel& instance();
};

HistoryTimeCategoryModel& HistoryTimeCategoryModelPrivate::instance()
{
   static auto instance = new HistoryTimeCategoryModel();
   return *instance;
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
   d_ptr->m_lCategories << tr("A week ago")                            ;//7
   d_ptr->m_lCategories << tr("Two weeks ago")                         ;//8
   d_ptr->m_lCategories << tr("Three weeks ago")                       ;//9
   d_ptr->m_lCategories << tr("A month ago")                           ;//10
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
   d_ptr->m_lCategories << tr("A year ago")                            ;//22
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
   static int categoriesSize = HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories.size();
   int period = (int)HistoryTimeCategoryModel::timeToHistoryConst(time);
   if (period >= 0 && period < categoriesSize)
      return HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories[period];
   else
      return HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories[categoriesSize - 1];
}

HistoryTimeCategoryModel::HistoryConst HistoryTimeCategoryModel::timeToHistoryConst(const time_t time)
{
   if (!time || time < 0)
      return HistoryTimeCategoryModel::HistoryConst::Never;

   time_t currentTime;
   ::time(&currentTime);

   /*
   * Struct tm description of fields used below:
   *  tm_yday   int   days since January 1  0-365
   *  tm_mon    int   months since January  0-11
   *  tm_year   int   years since 1900
   *  tm_wday   int   days since Sunday     0-6
   */
   struct tm localCurrentTime;
   struct tm localPastTime;

   ::localtime_r(&currentTime, &localCurrentTime);
   ::localtime_r(&time, &localPastTime);

   int diffYears = localCurrentTime.tm_year - localPastTime.tm_year;
   int diffMonths = localCurrentTime.tm_mon - localPastTime.tm_mon;
   int diffDays = localCurrentTime.tm_yday - localPastTime.tm_yday;

   if (diffYears == 1 && diffMonths < 0) {
      diffMonths += 12;
      diffDays += 365;
      diffYears = 0;
   }

   //Sanity check for future dates
   if (diffYears < 0 || (diffYears == 0 && (diffDays < 0 || diffMonths < 0))) {
       return HistoryTimeCategoryModel::HistoryConst::Never;
   }
   //Check for past 6 days
   if (diffYears == 0 && diffDays < 7) {
      return (HistoryTimeCategoryModel::HistoryConst)(diffDays); //Today to Six_days_ago
   }
   //Check for last month
   else if (diffYears == 0 && diffMonths <= 1 && (diffDays / 7 <= 4)) {
      return (HistoryTimeCategoryModel::HistoryConst)(diffDays / 7 + ((int)HistoryTimeCategoryModel::HistoryConst::A_week_ago) - 1); //A_week_ago to Three_weeks_ago
   }
   //Check for last year
   else if (diffYears == 0 && diffMonths > 0) {
      return (HistoryTimeCategoryModel::HistoryConst)(diffMonths + ((int)HistoryTimeCategoryModel::HistoryConst::A_month_ago) - 1); //A_month_ago to Twelve_months ago
   }
   else if (diffYears == 1)
      return HistoryConst::A_year_ago;

   //Every other senario
   return HistoryTimeCategoryModel::HistoryConst::Very_long_time_ago;
}

QString HistoryTimeCategoryModel::indexToName(int idx)
{
   static int size = HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories.size();
   if (idx < 0 || idx >= size)
      return HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories.last();
   return HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories[idx];
}
