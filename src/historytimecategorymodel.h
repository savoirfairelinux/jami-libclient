/****************************************************************************
 *   Copyright (C) 2012-2018 Savoir-faire Linux                          *
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
#pragma once

#include "typedefs.h"
#include <time.h>

#include <QtCore/QAbstractListModel>
#include <QtCore/QVector>
#include <QtCore/QString>

class HistoryTimeCategoryModelPrivate;

class LIB_EXPORT HistoryTimeCategoryModel : public QAbstractListModel {
   Q_OBJECT
public:
   ///@enum HistoryConst: History categories
   enum class HistoryConst : int {
      Today             = 0  ,
      Yesterday         = 1  ,
      Two_days_ago      = 2  ,
      Three_days_ago    = 3  ,
      Four_days_ago     = 4  ,
      Five_days_ago     = 5  ,
      Six_days_ago      = 6  ,
      A_week_ago        = 7  ,
      Two_weeks_ago     = 8  ,
      Three_weeks_ago   = 9  ,
      A_month_ago       = 10 ,
      Two_months_ago    = 11 ,
      Three_months_ago  = 12 ,
      Four_months_ago   = 13 ,
      Five_months_ago   = 14 ,
      Six_months_ago    = 15 ,
      Seven_months_ago  = 16 ,
      Eight_months_ago  = 17 ,
      Nine_months_ago   = 18 ,
      Ten_months_ago    = 19 ,
      Eleven_months_ago = 20 ,
      Twelve_months_ago = 21 ,
      A_year_ago        = 22 ,
      Very_long_time_ago= 23 ,
      Never             = 24 ,
   };
   Q_ENUMS(HistoryConst)

   //Constructor
   explicit HistoryTimeCategoryModel(QObject* parent = nullptr);
   virtual ~HistoryTimeCategoryModel();

   //Abstract model member
   virtual QVariant      data    (const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount(const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags   (const QModelIndex& index                                 ) const override;
   virtual bool          setData (const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getters
   static QString indexToName(int idx);

   //Helpers
   static HistoryConst timeToHistoryConst   (const time_t time);
   static QString      timeToHistoryCategory(const time_t time);

private:
   HistoryTimeCategoryModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(HistoryTimeCategoryModel)
};
Q_DECLARE_METATYPE(HistoryTimeCategoryModel::HistoryConst)
Q_DECLARE_METATYPE(HistoryTimeCategoryModel*)
