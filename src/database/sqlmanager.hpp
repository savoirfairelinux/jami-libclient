/************************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                                       *
 *   Author : Edric Milaret <edric.ladent-milaret@savoirfairelinux.com              *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/

#include <type_traits>

#include "itembase.h"
#include "call.h"

template<typename T>
bool SqlManager::saveItem(const ItemBase& obj) const {
   if (db.isOpen()) {
      auto serialObj = obj.serialize();
      QString field = serialObj.keys().join(", ");
      QStringList valuesList;
      for (auto v : serialObj.keys()) {
         valuesList << ":" + v;
      }
      QSqlQuery query;
      query.prepare(QString("INSERT INTO %1 (%2) VALUES (%3)").arg(getTableName<T>(), field, valuesList.join(", ")));
      for (auto k : serialObj.keys()) {
         query.bindValue(":"+k, serialObj[k]);
      }
      auto ret = query.exec();
      if (not ret)
         qWarning() << query.lastError();
      return ret;
   }
   return false;
}

template<typename T>
QList<QMap<QString, QVariant>> SqlManager::loadItems() const {
   QList<QMap<QString, QVariant>> objList;
   if (db.isOpen()) {
      auto tbName = getTableName<T>();
      auto record = db.record(tbName);
      QSqlQuery fecthAll;
      fecthAll.exec(QString("SELECT * FROM %1").arg(tbName));
      QMap<QString, int> indexOfFields;
      for (int i = 0; i < record.count(); i++)
         indexOfFields[record.fieldName(i)] = i;
      QMapIterator<QString, int> i(indexOfFields);
      while (fecthAll.next()) {
         QMap<QString, QVariant> obj;
         i.toFront();
         while (i.hasNext()) {
            i.next();
            obj[i.key()] = fecthAll.value(i.value());
         }
         objList.append(obj);
      }
   } else {
      qWarning() << "Database not opened";
   }
   return objList;
}

template<typename T>
QString SqlManager::getTableName() const {
   QString tableName;
   if (std::is_same<T, Call>::value)
      tableName = "call_history";
   return tableName;
}

template<typename T>
bool SqlManager::deleteAll() const {
   if (db.isOpen()) {
      QSqlQuery query(QString("DELETE FROM %1").arg(getTableName<T>()));
      return query.exec();
   }
   return false;
}

template<typename T>
bool SqlManager::deleteItem(const T* obj) const {
   if (db.isOpen()) {
      auto primaryIndex = obj->getPrimaryIndex();
      QSqlQuery query(QString("DELETE FROM %1 WHERE %2=:key").arg(getTableName<T>(), primaryIndex.first));
      query.bindValue(":key", primaryIndex.second);
      return query.exec();
   }
   return false;
}
