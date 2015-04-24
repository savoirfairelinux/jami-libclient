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
#include "uri.h"


class URIPrivate
{
public:
   ///Strings associated with SchemeType
   constexpr static const char* schemeNames[] = {
      /*NONE = */ ""     ,
      /*SIP  = */ "sip:" ,
      /*SIPS = */ "sips:",
      /*IAX  = */ "iax:" ,
      /*IAX2 = */ "iax2:",
      /*RING = */ "ring:",
   };

   URIPrivate(QString* uri);
   //Attributes
   QString           m_Hostname    ;
   QString           m_Userinfo    ;
   QStringList       m_lAttributes ;
   QString           m_Stripped    ;
   URI::SchemeType   m_HeaderType  ;
   bool              m_hasChevrons ;
   bool              m_Parsed      ;
   bool              m_HasAt       ;
   URI::ProtocolHint m_ProtocolHint;
   bool              m_HintParsed  ;

   //Helper
   static QString strip(const QString& uri, URI::SchemeType& scheme);
   void parse();
   static bool checkIp(const QString& str, bool &isHash, const URI::SchemeType& scheme);
private:
   QString* q_ptr;
};

constexpr const char* URIPrivate::schemeNames[];

URIPrivate::URIPrivate(QString* uri) : m_Parsed(false),m_HeaderType(URI::SchemeType::NONE),q_ptr(uri),
m_hasChevrons(false),m_HasAt(false),m_ProtocolHint(URI::ProtocolHint::SIP_OTHER),m_HintParsed(false)
{
}

///Constructor
URI::URI(const QString& other):QString(), d_ptr(new URIPrivate(this))
{
   d_ptr->m_Stripped              = URIPrivate::strip(other,d_ptr->m_HeaderType);
   (*static_cast<QString*>(this)) = d_ptr->m_Stripped                           ;
}

///Copy constructor
URI::URI(const URI& o):QString(), d_ptr(new URIPrivate(this))
{
   //TODO see if a copy on write kind of algo could be used for this
   d_ptr->m_Parsed       = o.d_ptr->m_Parsed      ;
   d_ptr->m_HintParsed   = o.d_ptr->m_HintParsed  ;
   d_ptr->m_Hostname     = o.d_ptr->m_Hostname    ;
   d_ptr->m_HasAt        = o.d_ptr->m_HasAt       ;
   d_ptr->m_ProtocolHint = o.d_ptr->m_ProtocolHint;
   d_ptr->m_HeaderType   = o.d_ptr->m_HeaderType  ;
   d_ptr->m_Userinfo     = o.d_ptr->m_Userinfo    ;
   d_ptr->m_Stripped     = o.d_ptr->m_Stripped    ;

   (*static_cast<QString*>(this)) = o.d_ptr->m_Stripped;
}

///Destructor
URI::~URI()
{
   (*static_cast<QString*>(this)) = QString();
   d_ptr->m_Stripped = QString();
//    delete d_ptr;
}

/// Copy operator, make sure the cache is also copied
URI& URI::operator=(const URI& o)
{
   d_ptr->m_Parsed       = o.d_ptr->m_Parsed      ;
   d_ptr->m_HintParsed   = o.d_ptr->m_HintParsed  ;
   d_ptr->m_Hostname     = o.d_ptr->m_Hostname    ;
   d_ptr->m_HasAt        = o.d_ptr->m_HasAt       ;
   d_ptr->m_ProtocolHint = o.d_ptr->m_ProtocolHint;
   d_ptr->m_HeaderType   = o.d_ptr->m_HeaderType  ;
   d_ptr->m_Userinfo     = o.d_ptr->m_Userinfo    ;
   d_ptr->m_Stripped     = o.d_ptr->m_Stripped    ;

   (*static_cast<QString*>(this)) = o.d_ptr->m_Stripped;
   return (*this);
}

///Strip out <sip:****> from the URI
QString URIPrivate::strip(const QString& uri, URI::SchemeType& scheme)
{
   if (uri.isEmpty())
      return {};

   int start(uri[0] == '<'?1:0),end(uri.size()-1); //Other type of comparisons were too slow

   if (start == end+1)
      return {};

   const uchar c = uri[start].toLatin1();

   //Assume the scheme is either iax, sip or ring using the first letter and length, this
   //is dangerous and can cause undefined behaviour that will cause the call to fail
   //later on, but this is not really a problem for now
   if (end > start+3 && uri[start+3] == ':') {
      switch (c) {
         case 'i':
            scheme = URI::SchemeType::IAX;
            break;
         case 's':
            scheme = URI::SchemeType::SIP;
            break;
      }
      start = start +4;
   }
   else if (end > start+4 && uri[start+4] == ':') {
      switch (c) {
         case 'i':
            scheme = URI::SchemeType::IAX2;
            break;
         case 'r':
            scheme = URI::SchemeType::RING;
            break;
         case 's':
            scheme = URI::SchemeType::SIPS;
            break;
      }
      start = start +5;
   }

   if (end && uri[end] == '>')
      end--;
   else if (start) {
      //TODO there may be a ';' section with arguments, check
   }

   return uri.mid(start,end-start+1);
}

/**
 * Return the domaine of an URI
 *
 * For example, example.com in <sip:12345@example.com>
 */
QString URI::hostname() const
{
   if (!d_ptr->m_Parsed)
      const_cast<URI*>(this)->d_ptr->parse();
   return d_ptr->m_Hostname;
}

/**
 * Check if the URI has an hostname
 * 
 * This will return true if there is something between '@' and ';' (or an end of line)
 */
bool URI::hasHostname() const
{
   if (!d_ptr->m_Parsed)
      const_cast<URI*>(this)->d_ptr->parse();
   return !d_ptr->m_Hostname.isEmpty();
}

/**
 * Return the URI SchemeType
 */
URI::SchemeType URI::schemeType() const
{
   if (!d_ptr->m_Parsed)
      const_cast<URI*>(this)->d_ptr->parse();
   return d_ptr->m_HeaderType;
}

/**
 * "Fast" Ipv4 and Ipv6 check, accept 999.999.999.999, :::::::::FF and other
 * atrocities, but at least perform a O(N) ish check and validate the hash
 *
 * @param str an uservalue (faster the scheme and before the "at" sign)
 * @param [out] isHash if the content is pure hexadecimal ASCII
 */
bool URIPrivate::checkIp(const QString& str, bool &isHash, const URI::SchemeType& scheme)
{
   char* raw = str.toLatin1().data();
   ushort max = str.size();

   if (max < 3 || max > 45 || (!isHash && scheme == URI::SchemeType::RING))
      return false;

   uchar dc(0),sc(0),i(0),d(0),hx(1);

   while (i < max) {
      switch(raw[i]) {
         case '.':
            isHash = false;
            d = 0;
            dc++;
            break;
         case '0': case '1': case '2':
         case '3': case '4': case '5':
         case '6': case '7': case '8':
         case '9':
            if (++d > 3 && dc)
               return false;
            break;
         case ':':
            isHash = false;
            sc++;
            //No break
         case 'A': case 'B': case 'C':
         case 'D': case 'E': case 'F':
         case 'a': case 'b': case 'c':
         case 'd': case 'e': case 'f':
            hx = 0;
            break;
         default:
            isHash = false;
            return false;
      };
      i++;
   }
   return (hx && dc == 3 && d < 4) ^ (sc > 1 && dc==0);
}

/**
 * This method return an hint to guess the protocol that could be used to call
 * this URI. It is a quick guess, not something that should be trusted
 *
 * @warning, this method is O(N) when called for the first time on an URI
 */
URI::ProtocolHint URI::protocolHint() const
{
   if (!d_ptr->m_Parsed)
      const_cast<URI*>(this)->d_ptr->parse();

   if (!d_ptr->m_HintParsed) {
      bool isHash = d_ptr->m_Userinfo.size() == 40;
      d_ptr->m_ProtocolHint = \
        (
         //Step one    : Check IAX protocol, is has already been detected at this point
         d_ptr->m_HeaderType == URI::SchemeType::IAX2 || d_ptr->m_HeaderType == URI::SchemeType::IAX
            ? URI::ProtocolHint::IAX

      : (
         //Step two  : check IP
         URIPrivate::checkIp(d_ptr->m_Userinfo,isHash,d_ptr->m_HeaderType) ? URI::ProtocolHint::IP

      : (
         //Step three    : Check RING protocol, is has already been detected at this point
         d_ptr->m_HeaderType == URI::SchemeType::RING && isHash ? URI::ProtocolHint::RING

      : (
         //Step four   : Differentiate between ***@*** and *** type URIs
         d_ptr->m_HasAt ? URI::ProtocolHint::SIP_HOST : URI::ProtocolHint::SIP_OTHER

        ))));

        d_ptr->m_HintParsed = true;
   }
   return d_ptr->m_ProtocolHint;
}

///Keep a cache of the values to avoid re-parsing them
void URIPrivate::parse()
{
   //FIXME the indexOf is done twice, the second time could be avoided
   if (q_ptr->indexOf('@') != -1) {
      const QStringList split = q_ptr->split('@');
      m_HasAt    = true;
      m_Hostname = split[1];
      m_Userinfo = split[0];
      m_Parsed   = true;
   }
   else
      m_Userinfo = (*q_ptr);
}

/**
 * Extract the user info field from the URI
 *
 * For example, "123" in sip:123@myserver.net
 */
QString URI::userinfo() const
{
   if (!d_ptr->m_Parsed)
      const_cast<URI*>(this)->d_ptr->parse();
   return d_ptr->m_Userinfo;
}

/**
 * Some feature, like SIP presence, require a properly formatted URI
 */
QString URI::fullUri() const
{
   return QString("<%1%2>")
      .arg(URIPrivate::schemeNames[static_cast<int>(d_ptr->m_HeaderType == SchemeType::NONE?SchemeType::SIP:d_ptr->m_HeaderType)])
      .arg(*this);
}

QDataStream& operator<<( QDataStream& stream, const URI::ProtocolHint& ph )
{
   switch(ph) {
      case URI::ProtocolHint::SIP_OTHER:
         stream << "SIP_OTHER";
         break;
      case URI::ProtocolHint::IAX      :
         stream << "IAX";
         break;
      case URI::ProtocolHint::RING     :
         stream << "RING";
         break;
      case URI::ProtocolHint::IP       :
         stream << "IP";
         break;
      case URI::ProtocolHint::SIP_HOST :
         stream << "SIP_HOST";
         break;
   }
   return stream;
}
