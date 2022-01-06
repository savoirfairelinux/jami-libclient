/****************************************************************************
 *    Copyright (C) 2017-2022 Savoir-faire Linux Inc.                       *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#pragma once

#include <QHash>
#include <QByteArray>

namespace lrc {
namespace vCard {

constexpr static const char* PROFILE_VCF = "x-ring/ring.profile.vcard";

struct Delimiter
{
    constexpr static const char* SEPARATOR_TOKEN = ";";
    constexpr static const char* END_LINE_TOKEN = "\n";
    constexpr static const char* BEGIN_TOKEN = "BEGIN:VCARD";
    constexpr static const char* END_TOKEN = "END:VCARD";
};

struct Property
{
    constexpr static const char* UID = "UID";
    constexpr static const char* VERSION = "VERSION";
    constexpr static const char* ADDRESS = "ADR";
    constexpr static const char* AGENT = "AGENT";
    constexpr static const char* BIRTHDAY = "BDAY";
    constexpr static const char* CATEGORIES = "CATEGORIES";
    constexpr static const char* CLASS = "CLASS";
    constexpr static const char* DELIVERY_LABEL = "LABEL";
    constexpr static const char* EMAIL = "EMAIL";
    constexpr static const char* FORMATTED_NAME = "FN";
    constexpr static const char* GEOGRAPHIC_POSITION = "GEO";
    constexpr static const char* KEY = "KEY";
    constexpr static const char* LOGO = "LOGO";
    constexpr static const char* MAILER = "MAILER";
    constexpr static const char* NAME = "N";
    constexpr static const char* NICKNAME = "NICKNAME";
    constexpr static const char* NOTE = "NOTE";
    constexpr static const char* ORGANIZATION = "ORG";
    constexpr static const char* PHOTO = "PHOTO";
    constexpr static const char* PRODUCT_IDENTIFIER = "PRODID";
    constexpr static const char* REVISION = "REV";
    constexpr static const char* ROLE = "ROLE";
    constexpr static const char* SORT_STRING = "SORT-STRING";
    constexpr static const char* SOUND = "SOUND";
    constexpr static const char* TELEPHONE = "TEL";
    constexpr static const char* TIME_ZONE = "TZ";
    constexpr static const char* TITLE = "TITLE";
    constexpr static const char* URL = "URL";
    constexpr static const char* BASE64 = "ENCODING=BASE64";
    constexpr static const char* TYPE_PNG = "TYPE=PNG";
    constexpr static const char* TYPE_JPEG = "TYPE=JPEG";
    constexpr static const char* PHOTO_PNG = "PHOTO;ENCODING=BASE64;TYPE=PNG";
    constexpr static const char* PHOTO_JPEG = "PHOTO;ENCODING=BASE64;TYPE=JPEG";

    constexpr static const char* X_RINGACCOUNT = "X-RINGACCOUNTID";
};

namespace utils {
/**
 * Payload to vCard
 * @param content payload
 * @return the vCard representation
 */
static QHash<QByteArray, QByteArray>
toHashMap(const QByteArray& content)
{
    // TODO without Qt
    QHash<QByteArray, QByteArray> vCard;
    QByteArray previousKey, previousValue;
    const QList<QByteArray> lines = content.split('\n');

    foreach (const QByteArray& property, lines) {
        // Ignore empty lines
        if (property.size()) {
            // Some properties are over multiple lines
            if (property[0] == ' ' && previousKey.size()) {
                previousValue += property.right(property.size() - 1);
            }

            // Do not use split, URIs can have : in them
            const int dblptPos = property.indexOf(':');
            const QByteArray k(property.left(dblptPos)),
                v(property.right(property.size() - dblptPos - 1));
            vCard[k] = v;
        }
    }
    return vCard;
}
} // namespace utils
} // namespace vCard
} // namespace lrc
