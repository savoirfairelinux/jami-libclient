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
#ifndef SORTPROXIES_H
#define SORTPROXIES_H

class QAbstractListModel;
class QItemSelectionModel;
class QSortFilterProxyModel;

/*
 * This file is dedicated to store the various sorting possibilities
 *
 * As they can share some helper code in common and would only add noise
 * to their "real" source model, They are placed here.
 */

namespace SortingCategory {
   struct ModelTuple {
      QAbstractListModel*     categories    ;
      QSortFilterProxyModel*  model         ;
      QItemSelectionModel*    selectionModel;
   };

   ModelTuple* getContactProxy();
   ModelTuple* getHistoryProxy();
}

#endif