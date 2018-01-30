/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
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

#include <typedefs.h>

//Qt
class QVariant   ;
class QModelIndex;
class QByteArray ;

//Ring
#include <securityevaluationmodel.h>
#include <categorizedcontactmodel.h>
#include <categorizedhistorymodel.h>

class  Person             ;
class  ContactMethod      ;
class  Call               ;
class  CollectionInterface;
class  Account;
struct UserActionElement  ;

namespace lrc { namespace api {
namespace account { struct Info; }
namespace conversation { struct Info; }
}} // namespace lrc::api

namespace Interfaces {

/**
 * Different clients can have multiple way of displaying images. Some may
 * add borders, other add corner radius (see Ubuntu-SDK HIG). This
 * abstract class define many operations that can be defined by each clients.
 *
 * Most methods return QVariants as this library doesn't link against QtGui
 *
 * This interface is not frozen, more methods may be added later.
 */
class PixmapManipulatorI {
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
        MACRO,
    };

    virtual ~PixmapManipulatorI() = default;

    virtual QVariant   contactPhoto(Person* c, const QSize& size, bool displayPresence = true) = 0;
    virtual QVariant   callPhoto(Call* c, const QSize& size, bool displayPresence = true) = 0;
    virtual QVariant   callPhoto(const ContactMethod* n, const QSize& size, bool displayPresence = true) = 0;
    virtual QVariant   conversationPhoto(const lrc::api::conversation::Info& conversation,
                                         const lrc::api::account::Info& accountInfo,
                                         const QSize& size,
                                         bool displayPresence = true) {
        Q_UNUSED(conversation); Q_UNUSED(accountInfo); Q_UNUSED(size); Q_UNUSED(displayPresence);
        return {};
    }
    virtual QVariant   numberCategoryIcon(const QVariant& p, const QSize& size, bool displayPresence = false, bool isPresent = false) = 0;
    virtual QVariant   securityIssueIcon(const QModelIndex& index) = 0;
    virtual QByteArray toByteArray(const QVariant& pxm) = 0;
    virtual QVariant   personPhoto(const QByteArray& data, const QString& type = "PNG") = 0;
    virtual QVariant   collectionIcon(const CollectionInterface* colItf, PixmapManipulatorI::CollectionIconHint hint = PixmapManipulatorI::CollectionIconHint::NONE) const = 0;
    virtual QVariant   securityLevelIcon(const SecurityEvaluationModel::SecurityLevel level) const = 0;
    virtual QVariant   historySortingCategoryIcon(const CategorizedHistoryModel::SortedProxy::Categories cat) const = 0;
    virtual QVariant   contactSortingCategoryIcon(const CategorizedContactModel::SortedProxy::Categories cat) const = 0;
    virtual QVariant   decorationRole(const QModelIndex&   index) = 0;
    virtual QVariant   decorationRole(const Call*          c    ) = 0;
    virtual QVariant   decorationRole(const ContactMethod* cm   ) = 0;
    virtual QVariant   decorationRole(const Person*        p    ) = 0;
    virtual QVariant   decorationRole(const lrc::api::conversation::Info& conversation,
                                      const lrc::api::account::Info& accountInfo) {
        Q_UNUSED(conversation); Q_UNUSED(accountInfo);
        return {};
    }
    virtual QVariant   decorationRole(const Account*       acc  ) = 0;

    /**
     * Return the icons associated with the action and its state
     */
    virtual QVariant userActionIcon(const UserActionElement& state) const = 0;
};

} // namespace Interfaces
