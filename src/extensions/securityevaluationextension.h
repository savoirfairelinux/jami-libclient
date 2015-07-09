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
#ifndef SECURITYEVALUATIONEXTENSION_H
#define SECURITYEVALUATIONEXTENSION_H

#include <collectionextensioninterface.h>
#include <collectionextensionmodel.h>
#include <securityevaluationmodel.h>

#include <typedefs.h>

#include <QtCore/QVariant>
#include <QtCore/QModelIndex>

class CollectionInterface;
class ItemBase;

class SecurityEvaluationExtensionPrivate;

class LIB_EXPORT SecurityEvaluationExtension final : public CollectionExtensionInterface
{
   Q_OBJECT

public:
   explicit SecurityEvaluationExtension(QObject* parent);

   virtual QVariant data(int role) const override;

   SecurityEvaluationModel::SecurityLevel securityLevel(const ItemBase* item) const;
   QVariant securityLevelIcon(const ItemBase* item) const;

private:
   SecurityEvaluationExtensionPrivate* d_ptr;
   virtual ~SecurityEvaluationExtension();
   Q_DECLARE_PRIVATE(SecurityEvaluationExtension)
};

#endif
