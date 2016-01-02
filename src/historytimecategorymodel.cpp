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
#include <QtCore/QTimeZone>
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

   // Months are not 28 days, but using the right number of days would
   // not change the algoritm output, so 28 is correct
   // Years are also assumed at 365, the impact wont be visible

   constexpr static const int day   = 3600*24;
   constexpr static const int week  = day*7  ;
   constexpr static const int month = week*4 ;
   constexpr static const int year  = 365*day;

   // Please note that this code ignore daylight saving time change
   // There will be a +/- 1 hour offset for calls made during the
   // other timezone used. This will introduce mis categorizing of
   // older calls.
   //
   // However, it can be ignored as the older calls will be grouped
   // in larger categories, reducing the impact of the issue as time
   // goes by. Trying to detect it is pointless.

   static QTimeZone timeZone = QTimeZone::systemTimeZone();
   static int offset = timeZone.offsetFromUtc(QDateTime::currentDateTime());

   // Get working values
   time_t time2 = time;
   time_t currentTime;
   ::time(&currentTime);

   // Align the time to midnight instead of using Nychthemeron based categories
   currentTime -= currentTime % (day); //Reset to midnight
   time2       -= time        % (day); //Reset to midnight

   // The time_t are aligned to UTC, bring them back to the right timezones
   time2       -= offset;
   currentTime -= offset;

   //Check for last week
   if ((currentTime-time2) / day < 7) {
      return (HistoryTimeCategoryModel::HistoryConst)(
         ((currentTime-time2) / day)
      ); //Yesterday to Six_days_ago
   }
   //Check for last month
   else if (currentTime - month < time2) {
      return (HistoryTimeCategoryModel::HistoryConst)(
         ((int)HistoryTimeCategoryModel::HistoryConst::A_week_ago)
            +
         ((currentTime-time2-week) / week)
      ); //A week ago to 3 weeks ago
   }
   //Check for last year
   else if (currentTime-(12)*30.4f*day < time2) {
      return (HistoryTimeCategoryModel::HistoryConst)(
         ((int)HistoryTimeCategoryModel::HistoryConst::A_month_ago)
            +
         ((currentTime-time2-month) / (30.4f*day))
      ); //A week ago to 3 weeks ago
   }
   //if (QDate::currentDate().addYears(-1)  >= date && QDate::currentDate().addYears(-2)  < date)
   else if (currentTime - year < time2)
      return HistoryConst::A_year_ago;

   //Every other senario
   return HistoryTimeCategoryModel::HistoryConst::Very_long_time_ago;
}

QString HistoryTimeCategoryModel::indexToName(int idx)
{
   static int size = HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories.size();
   if (idx >= size)
      return HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories.last();
   return HistoryTimeCategoryModelPrivate::instance().d_ptr->m_lCategories[idx];
}
