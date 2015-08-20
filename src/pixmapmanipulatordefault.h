/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-faire Linux                          *
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
#ifndef PIXMAPMANIPULATORDEFAULT_H
#define PIXMAPMANIPULATORDEFAULT_H

#include "interfaces/pixmapmanipulatori.h"

namespace Interfaces {

///Default implementation of the PixmapManipulator interface which simply returns empty QVariants/QByteArrays
class LIB_EXPORT PixmapManipulatorDefault : public PixmapManipulatorI {
public:
    QVariant   contactPhoto(Person* c, const QSize& size, bool displayPresence = true) override;
    QVariant   callPhoto(Call* c, const QSize& size, bool displayPresence = true) override;
    QVariant   callPhoto(const ContactMethod* n, const QSize& size, bool displayPresence = true) override;
    QVariant   numberCategoryIcon(const QVariant& p, const QSize& size, bool displayPresence = false, bool isPresent = false) override;
    QVariant   securityIssueIcon(const QModelIndex& index) override;
    QByteArray toByteArray(const QVariant& pxm) override;
    QVariant   personPhoto(const QByteArray& data, const QString& type = "PNG") override;
    QVariant   collectionIcon(const CollectionInterface* interface, PixmapManipulatorI::CollectionIconHint hint = PixmapManipulatorI::CollectionIconHint::NONE) const override;
    QVariant   securityLevelIcon(const SecurityEvaluationModel::SecurityLevel level) const override;
    QVariant   historySortingCategoryIcon(const CategorizedHistoryModel::SortedProxy::Categories cat) const override;
    QVariant   contactSortingCategoryIcon(const CategorizedContactModel::SortedProxy::Categories cat) const override;

    /**
     * Return the icons associated with the action and its state
     */
    QVariant userActionIcon(const UserActionElement& state) const override;
};

} // namespace Interfaces

#endif // PIXMAPMANIPULATORDEFAULT_H
