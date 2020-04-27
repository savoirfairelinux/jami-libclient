/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                       *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#include "api/collections/trustRequestList.h"

#include "api/element.h"

// Daemon
#include <account_const.h>

// Lrc
#include "vcard.h"
#include "authority/storagehelper.h"

// Dbus
#include "dbus/configurationmanager.h"
namespace lrc
{

namespace api
{

using namespace authority;

class TrustRequestListPimpl
{
public:
    TrustRequestListPimpl(const QString& owner);

    QMap<QString, Element> elements;
};

TrustRequestListPimpl::TrustRequestListPimpl(const QString& owner)
{
    // Add pending contacts
    const VectorMapStringString& pending_tr {ConfigurationManager::instance().getTrustRequests(owner)};
    for (const auto& tr_info : pending_tr) {
        // Get pending requests.
        auto payload = tr_info[DRing::Account::TrustRequest::PAYLOAD].toUtf8();

        auto contactUri = tr_info[DRing::Account::TrustRequest::FROM];

        auto contactInfo = storage::buildContactFromProfile(owner, contactUri, profile::Type::PENDING);

        const auto vCard = lrc::vCard::utils::toHashMap(payload);
        const auto alias = vCard["FN"];
        QByteArray photo;
        for (const auto& key: vCard.keys()) {
            if (key.contains("PHOTO"))
                photo = vCard[key];
        }

        elements.insert(contactUri, {
            contactUri,
            photo.constData(),
            alias,
            {},
            {},
            0
        });

        if (!alias.isEmpty()) contactInfo.profileInfo.alias = alias.constData();
        if (!photo.isEmpty()) contactInfo.profileInfo.avatar = photo.constData();

        // create profile vcard for contact
        storage::createOrUpdateProfile(owner, contactInfo.profileInfo, true);
    }
}

TrustRequestList::TrustRequestList(const QString &owner) : Collection(owner)
{

}

TrustRequestList::~TrustRequestList()
{

}

QVector<Element>
TrustRequestList::filter(const QString& search)
{
    return {};
}

} // namespace api
} // namespace lrc

#include "api/collections/moc_trustRequestList.cpp"
#include "trustRequestList.moc"
