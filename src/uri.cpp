/****************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                          *
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

#include "private/matrixutils.h"

#include <QRegularExpression>

class URIPrivate
{
public:
   ///Strings associated with SchemeType
   static const Matrix1D<URI::SchemeType, const char*> schemeNames;

   ///String associated with the transport name
   static const Matrix1D<URI::Transport, const char*> transportNames;

   static const QString whitespaceCharClass;

   static const QRegularExpression startWhitespaceMatcher;
   static const QRegularExpression endWhitespaceMatcher;

   ///Attributes names
   struct Constants {
      constexpr static const char TRANSPORT[] = "transport";
      constexpr static const char TAG      [] = "tag"      ;
   };

   //Constructor
   URIPrivate(URI* uri);
   void commonCopyConstructor(const URI& o);

   //Attributes
   QString           m_ExtHostname ;
   QString           m_Userinfo    ;
   QStringList       m_lAttributes ;
   QString           m_Stripped    ;
   QString           m_Hostname2   ;
   QByteArray        m_Tag         ;
   URI::SchemeType   m_HeaderType  ;
   URI::Transport    m_Transport   ;
   bool              m_hasChevrons ;
   bool              m_Parsed      ;
   bool              m_HasAt       ;
   URI::ProtocolHint m_ProtocolHint;
   bool              m_HintParsed  ;
   bool              m_IsHNParsed  ;
   int               m_Port        ;

   //Helper
   static QString strip(const QString& uri, URI::SchemeType& scheme);
   void parse();
   void parseHostname();
   static bool checkIp(const QString& str, bool &isHash, const URI::SchemeType& scheme);
   URI::Transport nameToTransport(const QByteArray& name);
   void parseAttribute(const QByteArray& extHn, const int start, const int pos);
private:
   URI* q_ptr;
};

constexpr const char  URIPrivate::Constants::TRANSPORT[];
constexpr const char  URIPrivate::Constants::TAG      [];

const Matrix1D<URI::Transport, const char*> URIPrivate::transportNames = {{
   /*NOT_SET*/ "NOT_SET",
   /*TLS    */ "TLS"    ,
   /*tls    */ "tls"    ,
   /*TCP    */ "TCP"    ,
   /*tcp    */ "tcp"    ,
   /*UDP    */ "UDP"    ,
   /*udp    */ "udp"    ,
   /*SCTP   */ "SCTP"   ,
   /*sctp   */ "sctp"   ,
   /*DTLS   */ "DTLS"   ,
   /*dtls   */ "dtls"   ,
}};

const Matrix1D<URI::SchemeType, const char*> URIPrivate::schemeNames = {{
   /*NONE = */ ""     ,
   /*SIP  = */ "sip:" ,
   /*SIPS = */ "sips:",
   /*RING = */ "ring:",
}};

const QString URIPrivate::whitespaceCharClass = QStringLiteral("[\\h\\x{200B}\\x{200C}\\x{200D}\\x{FEFF}]+");

const QRegularExpression URIPrivate::startWhitespaceMatcher = QRegularExpression(
                                                   "^" + URIPrivate::whitespaceCharClass,
                                                   QRegularExpression::UseUnicodePropertiesOption);
const QRegularExpression URIPrivate::endWhitespaceMatcher = QRegularExpression(
                                                   URIPrivate::whitespaceCharClass + "$",
                                                   QRegularExpression::UseUnicodePropertiesOption);

URIPrivate::URIPrivate(URI* uri) : m_Parsed(false),m_HeaderType(URI::SchemeType::NONE),q_ptr(uri),
m_hasChevrons(false),m_HasAt(false),m_ProtocolHint(URI::ProtocolHint::SIP_OTHER),m_HintParsed(false),
m_IsHNParsed(false),m_Port(-1),m_Transport(URI::Transport::NOT_SET)
{
}

///Default constructor
URI::URI() : QString(), d_ptr(new URIPrivate(this))
{

}

///Constructor
URI::URI(const QString& other) : URI()
{
   d_ptr->m_Stripped              = URIPrivate::strip(other,d_ptr->m_HeaderType);
   (*static_cast<QString*>(this)) = d_ptr->m_Stripped                           ;
}

///Copy constructor
URI::URI(const URI& o) : URI()
{
   d_ptr->commonCopyConstructor(o);
}

///Destructor
URI::~URI()
{
   (*static_cast<QString*>(this)) = QString();
   d_ptr->m_Stripped = QString();
//    delete d_ptr;
}

void URIPrivate::commonCopyConstructor(const URI& o)
{
   //TODO see if a copy on write kind of algo could be used for this
   m_Parsed       = o.d_ptr->m_Parsed      ;
   m_HintParsed   = o.d_ptr->m_HintParsed  ;
   m_ExtHostname  = o.d_ptr->m_ExtHostname ;
   m_HasAt        = o.d_ptr->m_HasAt       ;
   m_ProtocolHint = o.d_ptr->m_ProtocolHint;
   m_HeaderType   = o.d_ptr->m_HeaderType  ;
   m_Userinfo     = o.d_ptr->m_Userinfo    ;
   m_Stripped     = o.d_ptr->m_Stripped    ;
   m_IsHNParsed   = o.d_ptr->m_IsHNParsed  ;
   m_Port         = o.d_ptr->m_Port        ;
   m_Transport    = o.d_ptr->m_Transport   ;
   m_Tag          = o.d_ptr->m_Tag         ;

   (*static_cast<QString*>(q_ptr)) = o.d_ptr->m_Stripped;
}

/// Copy operator, make sure the cache is also copied
URI& URI::operator=(const URI& o)
{
   d_ptr->commonCopyConstructor(o);
   return (*this);
}

///Strip out <sip:****> from the URI
QString URIPrivate::strip(const QString& uri, URI::SchemeType& scheme)
{
   if (uri.isEmpty())
      return {};

   /* remove whitespace at the start and end */
   auto uriTrimmed = uri;
   uriTrimmed.replace(startWhitespaceMatcher, "");
   uriTrimmed.replace(endWhitespaceMatcher, "");

   int start(uriTrimmed[0] == '<'?1:0),end(uriTrimmed.size()-1); //Other type of comparisons were too slow

   if (start == end+1)
      return {};

   const char c = uriTrimmed[start].toLatin1();

   //Assume the scheme is either sip or ring using the first letter and length, this
   //is dangerous and can cause undefined behaviour that will cause the call to fail
   //later on, but this is not really a problem for now
   if (end > start+3 && uriTrimmed[start+3] == ':') {
      switch (c) {
         case 's':
            scheme = URI::SchemeType::SIP;
            break;
      }
      start = start +4;
   }
   else if (end > start+4 && uriTrimmed[start+4] == ':') {
      switch (c) {
         case 'r':
            scheme = URI::SchemeType::RING;
            break;
         case 's':
            scheme = URI::SchemeType::SIPS;
            break;
      }
      start = start +5;
   }

   if (end && uriTrimmed[end] == '>')
      end--;
   else if (start) {
      //TODO there may be a ';' section with arguments, check
   }

   return uriTrimmed.mid(start,end-start+1);
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
   return d_ptr->m_ExtHostname;
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
   return !d_ptr->m_ExtHostname.isEmpty();
}

/**
 * If hasHostname() is true, this does a second parsing of the hostname to
 * extract the port.
 */
bool URI::hasPort() const
{
   if (!d_ptr->m_IsHNParsed) {
      d_ptr->parseHostname();
   }
   return d_ptr->m_Port != -1;
}

/**
 * Return the port, -1 is none is set
 */
int  URI::port() const
{
   if (!d_ptr->m_IsHNParsed) {
      d_ptr->parseHostname();
   }
   return d_ptr->m_Port;
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
   const QByteArray raw = str.toLatin1();
   int max = str.size();

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
            [[clang::fallthrough]];
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

       URI::ProtocolHint hint;

       //Step 1: Check IP
       if (URIPrivate::checkIp(d_ptr->m_Userinfo, isHash, d_ptr->m_HeaderType)) {
           hint = URI::ProtocolHint::IP;
       }
       //Step 2: Check RING hash
       else if (isHash)
       {
           hint = URI::ProtocolHint::RING;
       }
       //Step 3: Not a hash but it begins with ring:. This is a username.
       else if (d_ptr->m_HeaderType == URI::SchemeType::RING){
           hint = URI::ProtocolHint::RING_USERNAME;
       }
       //Step 4: Check for SIP URIs
       else if (d_ptr->m_HeaderType == URI::SchemeType::SIP)
       {
           //Step 4.1: Check for SIP URI with hostname
           if (d_ptr->m_HasAt) {
               hint = URI::ProtocolHint::SIP_HOST;
           }
           //Step 4.2: Assume SIP URI without hostname
           else {
               hint = URI::ProtocolHint::SIP_OTHER;
           }
       }
       //Step 5: Assume SIP
       else {
           hint = URI::ProtocolHint::SIP_OTHER;
       }

       d_ptr->m_ProtocolHint = hint;
       d_ptr->m_HintParsed = true;
    }
    return d_ptr->m_ProtocolHint;
 }

///Convert the transport name to a string
URI::Transport URIPrivate::nameToTransport(const QByteArray& name)
{
   if (name == transportNames[URI::Transport::NOT_SET  ])
      return URI::Transport::NOT_SET;
   else if (name == transportNames[URI::Transport::TLS ])
      return URI::Transport::TLS    ;
   else if (name == transportNames[URI::Transport::tls ])
      return URI::Transport::tls    ;
   else if (name == transportNames[URI::Transport::TCP ])
      return URI::Transport::TCP    ;
   else if (name == transportNames[URI::Transport::tcp ])
      return URI::Transport::tcp    ;
   else if (name == transportNames[URI::Transport::UDP ])
      return URI::Transport::UDP    ;
   else if (name == transportNames[URI::Transport::udp ])
      return URI::Transport::udp    ;
   else if (name == transportNames[URI::Transport::SCTP])
      return URI::Transport::SCTP   ;
   else if (name == transportNames[URI::Transport::sctp])
      return URI::Transport::sctp   ;
   else if (name == transportNames[URI::Transport::DTLS])
      return URI::Transport::DTLS   ;
   else if (name == transportNames[URI::Transport::dtls])
      return URI::Transport::dtls   ;
   return URI::Transport::NOT_SET   ;
}

///Keep a cache of the values to avoid re-parsing them
void URIPrivate::parse()
{
   //FIXME the indexOf is done twice, the second time could be avoided
   if (q_ptr->indexOf('@') != -1) {
      const QStringList split = q_ptr->split('@');
      m_HasAt       = true;
      m_ExtHostname = split[1];
      m_Userinfo    = split[0];
      m_Parsed      = true;
   }
   else
      m_Userinfo = (*q_ptr);
}

void URIPrivate::parseAttribute(const QByteArray& extHn, const int start, const int pos)
{
   const QList<QByteArray> parts = extHn.mid(start+1,pos-start).split('=');

   if (parts.size() == 2) {
      if (parts[0].toLower() == Constants::TRANSPORT) {
         m_Transport = nameToTransport(parts[1]);
      }
      else if (parts[0].toLower() == Constants::TAG) {
         m_Tag = parts[1];
      }
   }
}

///Extract the hostname, port and attributes
void URIPrivate::parseHostname()
{
   if (!m_Parsed)
      parse();

   const QByteArray extHn = q_ptr->hostname().toLatin1();
   int length(extHn.size()), start(0);
   bool inAttributes = false;

   URI::Section section = URI::Section::HOSTNAME;

   // in case no port, attributes, etc are provided
   m_Hostname2 = q_ptr->hostname();

   for (int i = 0; i < length; i++) {
      const char c = extHn[i];
      switch (c) {
         case ':': //Begin port
            switch(section) {
               case URI::Section::HOSTNAME:
                  m_Hostname2 = extHn.mid(start,i);
                  start = i;
                  section = URI::Section::PORT;
                  break;
               case URI::Section::USER_INFO:
               case URI::Section::CHEVRONS :
               case URI::Section::SCHEME   :
               case URI::Section::TRANSPORT:
               case URI::Section::TAG      :
               case URI::Section::PORT     :
                  break;
            }
            break;
         case ';': //Begin attributes

            if (inAttributes) {
               parseAttribute(extHn, start, i);
            }
            else {
               switch(section) {
                  case URI::Section::HOSTNAME:
                     m_Hostname2 = extHn.mid(start+1,i-start);
                     break;
                  case URI::Section::PORT:
                     m_Port = extHn.mid(start+1,i-start-1).toInt();
                     break;
                  case URI::Section::USER_INFO:
                  case URI::Section::CHEVRONS :
                  case URI::Section::SCHEME   :
                  case URI::Section::TRANSPORT:
                  case URI::Section::TAG      :
                     break;
               }
               inAttributes = true;
            }

            start = i;
            break;
         case '#': //Begin fragments
            //TODO handle fragments to comply to the RFC
            break;
         default:
            break;
      }
   }

   ///Get the remaining attribute
   parseAttribute(extHn, start, length-1);

   m_IsHNParsed = true;
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
 * Sometime, some metadata can be used to deduce the scheme even if it wasn't
 * originally known. This will improve the result of ::format.
 */
void URI::setSchemeType(SchemeType t)
{
    d_ptr->m_HeaderType = t;
}

/**
 * Generate a new URI formatted with the sections passed in `sections`
 *
 * It is kept as a QString to avoid the URI class to start reformatting
 * it right away.
 */
QString URI::format(FlagPack<URI::Section> sections) const
{
   if (!d_ptr->m_IsHNParsed) {
      d_ptr->parseHostname();
   }

   QString ret;

   if (sections & URI::Section::CHEVRONS)
      ret += '<';

   if (sections & URI::Section::SCHEME) {
       auto header_type = d_ptr->m_HeaderType;

       // Try to use the protocol hint on undeterminated header type.
       // Use SIP scheme type on last resort
       if (header_type == SchemeType::NONE) {
           switch (protocolHint()) {
               case ProtocolHint::RING:
               case ProtocolHint::RING_USERNAME:
                   header_type = SchemeType::RING;
                   break;
               case ProtocolHint::SIP_HOST:
               case ProtocolHint::SIP_OTHER:
               case ProtocolHint::IP:
               default:
                   header_type = SchemeType::SIP;
                   break;
           }
       }

      ret += URIPrivate::schemeNames[header_type];
   }

   if (sections & URI::Section::USER_INFO)
      ret += d_ptr->m_Userinfo;

   if (sections & URI::Section::HOSTNAME && !d_ptr->m_Hostname2.isEmpty())
      ret += '@' + d_ptr->m_Hostname2;

   if (sections & URI::Section::PORT && d_ptr->m_Port != -1)
      ret += ':' + QString::number(d_ptr->m_Port);

   if (sections & URI::Section::CHEVRONS)
      ret += '>';

   if (sections & URI::Section::TRANSPORT && d_ptr->m_Transport != URI::Transport::NOT_SET)
      ret += ";transport=" + QString(URIPrivate::transportNames[d_ptr->m_Transport]);

   if (sections & URI::Section::TAG && !d_ptr->m_Tag.isEmpty())
      ret += ";tag=" + d_ptr->m_Tag;

   return ret;
}

/**
 * Helper function which returns a QString containing a uri formatted to include at minimum the
 * SCHEME and USER_INFO, and also the HOSTNAME and PORT, if available.
 */
QString URI::full() const
{
    return format(URI::Section::SCHEME | URI::Section::USER_INFO | URI::Section::HOSTNAME | URI::Section::PORT);
}

QDataStream& operator<<( QDataStream& stream, const URI::ProtocolHint& ph )
{
   switch(ph) {
      case URI::ProtocolHint::SIP_OTHER:
         stream << QStringLiteral("SIP_OTHER");
         break;
      case URI::ProtocolHint::RING:
      case URI::ProtocolHint::RING_USERNAME:
         stream << QStringLiteral("RING");
         break;
      case URI::ProtocolHint::IP       :
         stream << QStringLiteral("IP");
         break;
      case URI::ProtocolHint::SIP_HOST :
         stream << QStringLiteral("SIP_HOST");
         break;
   }
   return stream;
}
