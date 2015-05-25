/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                         ***
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
#ifndef PIXMAPMANIPULATIONDELEGATE_H
#define PIXMAPMANIPULATIONDELEGATE_H
#include <typedefs.h>

//Qt
#include <QtCore/QVariant>
#include <QtCore/QModelIndex>

//Ring
#include <securityevaluationmodel.h>
class  Person             ;
class  ContactMethod      ;
class  Call               ;
class  CollectionInterface;
struct UserActionElement;

/**
 * Different clients can have multiple way of displaying images. Some may
 * add borders, other add corner radius (see Ubuntu-SDK HIG). This
 * abstract class define many operations that can be defined by each clients.
 *
 * Most methods return QVariants as this library doesn't link against QtGui
 * 
 * This interface is not frozen, more methods may be added later. To implement,
 * just create an object somewhere, be sure to call PixmapManipulationDelegate()
 */
class LIB_EXPORT PixmapManipulationDelegate {
public:
   //Implementation can use random values to extend this
   enum CollectionIconHint {
      NONE,
      HISTORY,
      CONTACT,
      BOOKMARK,
      PHONE_NUMBER,
      RINGTONE,
      PROFILE,
      CERTIFICATE,
      ACCOUNT,
      RECORDING,
   };

   PixmapManipulationDelegate();
   virtual ~PixmapManipulationDelegate() {}
   virtual QVariant   contactPhoto(Person* c, const QSize& size, bool displayPresence = true);
   virtual QVariant   callPhoto(Call* c, const QSize& size, bool displayPresence = true);
   virtual QVariant   callPhoto(const ContactMethod* n, const QSize& size, bool displayPresence = true);
   virtual QVariant   numberCategoryIcon(const QVariant& p, const QSize& size, bool displayPresence = false, bool isPresent = false);
   virtual QVariant   securityIssueIcon(const QModelIndex& index);
   virtual QByteArray toByteArray(const QVariant& pxm);
   virtual QVariant   personPhoto(const QByteArray& data, const QString& type = "PNG");
   virtual QVariant   collectionIcon(const CollectionInterface* interface, PixmapManipulationDelegate::CollectionIconHint hint = PixmapManipulationDelegate::CollectionIconHint::NONE) const;
   virtual QVariant   securityLevelIcon(const SecurityEvaluationModel::SecurityLevel level) const;

   /**
    * Return the icons associated with the action and its state
    */
   virtual QVariant userActionIcon(const UserActionElement& state) const;

   //Singleton
   static PixmapManipulationDelegate* instance();
protected:
   static PixmapManipulationDelegate* m_spInstance;
};

#endif //PIXMAPMANIPULATIONVISITOR_H
