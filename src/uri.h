/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#ifndef URI_H
#define URI_H

#include "typedefs.h"

#include <QStringList>

class URIPrivate;
class QDataStream;

/**
    * @class URI A specialized string with multiple attributes
    *
    * Most of LibRingClient handle uri as strings, but more
    * advanced algorithms need to access the various sections.
    *
    * Here is some example of common numbers/URIs:
    *  * 123
    *  * 123@192.168.123.123
    *  * 123@asterisk-server
    *  * <sip:123@192.168.123.123>
    *  * <sips:123@192.168.123.123>
    *  * <sips:888@192.168.48.213;transport=TLS>
    *  * <sip:c8oqz84zk7z@privacy.org>;tag=hyh8
    *  * 1 800 123-4567
    *  * 18001234567
    *  * iax:example.com/alice
    *  * iax:johnQ@example.com/12022561414
    *  * iax:example.com:4570/alice?friends
    *
    * @ref http://tools.ietf.org/html/rfc5456#page-8
    * @ref http://tools.ietf.org/html/rfc3986
    * @ref http://tools.ietf.org/html/rfc3261
    * @ref http://tools.ietf.org/html/rfc5630
    *
    * From the RFC:
    *    foo://example.com:8042/over/there?name=ferret#nose
    *    \_/   \______________/\_________/ \_________/ \__/
    *     |           |            |            |        |
    *  scheme     authority       path        query   fragment
    *     |   _____________________|__
    *    / \ /                        \
    *    urn:example:animal:ferret:nose
    *
    *    authority   = [ userinfo "@" ] host [ ":" port ]
    *
    *    "For example, the semicolon (";") and equals ("=") reserved characters are
    *    often used to delimit parameters and parameter values applicable to
    *    that segment.  The comma (",") reserved character is often used for
    *    similar purposes.  For example, one URI producer might use a segment
    *    such as "name;v=1.1" to indicate a reference to version 1.1 of
    *    "name", whereas another might use a segment such as "name,1.1" to
    *    indicate the same. "
    */
class LIB_EXPORT URI : public QString {
public:

   /**
    * Default constructor
    * @param other an URI string
    */
   URI(const QString& other);
   URI(const URI&     other);
   virtual ~URI();

   ///@enum SchemeType The very first part of the URI followed by a ':'
   enum class SchemeType {
      NONE , //Implicit SIP or IAX, use account type as reference
      SIP  ,
      SIPS ,
      IAX  ,
      IAX2 ,
      RING ,
   };
   Q_ENUMS(URI::SchemeType)

   /**
    * @enum Transport each known valid transport types
    * Defined at http://tools.ietf.org/html/rfc3261#page-222
    */
   enum class Transport {
      NOT_SET, /*!<  The transport have not been set directly in the URI  */
      TLS    , /*!<  Encrypted calls (capital)                            */
      tls    , /*!<  Encrypted calls                                      */
      TCP    , /*!<  TCP (the default) (capital)                          */
      tcp    , /*!<  TCP (the default)                                    */
      UDP    , /*!<  Without a connection (capital)                       */
      udp    , /*!<  Without a connection                                 */
      SCTP   , /*!<                                                       */
      sctp   , /*!<                                                       */
   };
   Q_ENUMS(URI::Transport)

   /**
    * @enum ProtocolHint Expanded version of Account::Protocol
    *
    * This is used to make better choice when it come to choose an account or
    * guess if the URI can be used with the current set et configured accounts.
    *
    * @warning This is an approximation. Those values are guessed using partial
    * parsing (for performance) and are not definitive.
    */
   enum class ProtocolHint {
      SIP_OTHER = 0, /*!< Anything non empty that doesn't fit in other categories */
      IAX       = 1, /*!< Start with "iax:" or "iax2:"                            */
      RING      = 2, /*!< Start with "ring:" and 45 ASCII chars OR 40 ASCII chars */
      IP        = 3, /*!< Match an IPv4 address                                   */
      SIP_HOST  = 4, /*!< Has an @ and no "ring:" prefix                          */
   };
   Q_ENUMS(URI::ProtocolHint)

   QString    hostname      () const;
   QString    fullUri       () const;
   QString    userinfo      () const;
   bool       hasHostname   () const;
   SchemeType schemeType    () const;
   ProtocolHint protocolHint() const;

   URI& operator=(const URI&);

private:
   const QScopedPointer<URIPrivate> d_ptr;
};

Q_DECLARE_METATYPE(URI::ProtocolHint)

QDataStream& operator<< ( QDataStream& stream, const URI::ProtocolHint& ph );

#endif //URI_H
