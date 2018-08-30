/****************************************************************************
 *   Copyright (C) 2014-2018 Savoir-faire Linux                             *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *   Author : Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>            *
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
#include <regex>

class URIPimpl
{
public:
    // Strings associated with SchemeType
    static const std::map<URI::SchemeType, const char*> schemeNames;

    // String associated with the transport name
    static const std::map<URI::Transport, const char*> transportNames;

    struct Constants {
        constexpr static const char TRANSPORT[] = "transport";
        constexpr static const char TAG[] = "tag";
    };

    URIPimpl(const URI& uri);
    URIPimpl& operator=(const URIPimpl&);

    // Attributes
    const URI& linked;

    QString m_ExtHostname;
    QString m_Scheme;
    QString m_Userinfo;
    QStringList m_lAttributes;
    QString m_Stripped;
    QString m_Hostname2;
    QByteArray m_Tag;
    int m_Port = -1;

    URI::SchemeType m_HeaderType = URI::SchemeType::NONE;
    URI::Transport m_Transport = URI::Transport::NOT_SET;
    URI::ProtocolHint m_ProtocolHint = URI::ProtocolHint::UNRECOGNIZED;

    bool m_hasChevrons {false};
    bool m_Parsed {false};
    bool m_HasAt {false};
    bool m_HintParsed {false};
    bool m_IsHNParsed {false};

    // Helpers
    URI::Transport nameToTransport(const QByteArray& name);
    static QString strip(const QString& uri, URI::SchemeType& schemeType, QString& scheme);
    static bool checkIp(const QString& str, bool &isHash, const URI::SchemeType& scheme);
    void parseAttribute(const QByteArray& extHn, const int start, const int pos);
    void parseHostname();
    void parse();
};

constexpr const char URIPimpl::Constants::TRANSPORT[];
constexpr const char URIPimpl::Constants::TAG[];

const std::map<URI::Transport, const char*> URIPimpl::transportNames = {
    { URI::Transport::NOT_SET, "NOT_SET" },
    { URI::Transport::TLS, "TLS" },
    { URI::Transport::tls, "tls" },
    { URI::Transport::TCP, "TCP" },
    { URI::Transport::tcp, "tcp" },
    { URI::Transport::UDP, "UDP" },
    { URI::Transport::udp, "udp" },
    { URI::Transport::SCTP, "SCTP" },
    { URI::Transport::sctp, "sctp" },
    { URI::Transport::DTLS, "DTLS" },
    { URI::Transport::dtls, "dtls" }
};

const std::map<URI::SchemeType, const char*> URIPimpl::schemeNames = {
    { URI::SchemeType::SIP, "sip:" },
    { URI::SchemeType::SIPS, "sips:" },
    { URI::SchemeType::RING, "ring:" }
};

URIPimpl::URIPimpl(const URI& uri)
: linked(uri)
{
}

URIPimpl& URIPimpl::operator=(const URIPimpl& other)
{
    m_Parsed = other.m_Parsed;
    m_HintParsed = other.m_HintParsed;
    m_ExtHostname = other.m_ExtHostname;
    m_HasAt = other.m_HasAt;
    m_Scheme = other.m_Scheme;
    m_ProtocolHint = other.m_ProtocolHint;
    m_HeaderType = other.m_HeaderType;
    m_Userinfo = other.m_Userinfo;
    m_Stripped = other.m_Stripped;
    m_IsHNParsed = other.m_IsHNParsed;
    m_Port = other.m_Port;
    m_Transport = other.m_Transport;
    m_Tag = other.m_Tag;
    m_hasChevrons = other.m_hasChevrons;
    m_Hostname2 = other.m_Hostname2;
    m_lAttributes = other.m_lAttributes;
    return *this;
}

URI::URI()
: QString()
, pimpl_(std::make_unique<URIPimpl>(*this))
{
}

URI::URI(const QString& uri)
: URI()
{
    QString simplified = uri.simplified().remove(' ').remove('<').remove('>');
    pimpl_->m_Stripped = URIPimpl::strip(simplified, pimpl_->m_HeaderType, pimpl_->m_Scheme);
    (*static_cast<QString*>(this)) = pimpl_->m_Stripped;
}

URI::URI(const URI& other)
: URI()
{
    *pimpl_ = *other.pimpl_;
    (*static_cast<QString*>(this)) = pimpl_->m_Stripped;
}

URI& URI::operator=(const URI& other)
{
    if (this != &other) {
        *pimpl_ = *other.pimpl_;
        (*static_cast<QString*>(this)) = pimpl_->m_Stripped;
    }
    return *this;
}

URI::~URI()
{
}

/**
 * Strip out scheme from the URI
 */
QString URIPimpl::strip(const QString& uri, URI::SchemeType& schemeType, QString& scheme)
{
    if (uri.isEmpty())
        return {};

    std::regex uri_regex = std::regex("[a-zA-Z][a-zA-Z0-9+.-]*:");
    std::string uri_to_match = uri.toStdString();
    std::smatch match;

    if (std::regex_search(uri_to_match, match, uri_regex)) {
        if (match.ready()) {
            scheme = match.str(0).c_str();
        }
    }

    if (scheme == URIPimpl::schemeNames.at(URI::SchemeType::SIP)) {
        schemeType = URI::SchemeType::SIP;
    } else if (scheme == URIPimpl::schemeNames.at(URI::SchemeType::RING)) {
        schemeType = URI::SchemeType::RING;
    } else if (scheme == URIPimpl::schemeNames.at(URI::SchemeType::SIPS)) {
        schemeType = URI::SchemeType::SIPS;
    } else if (!scheme.isEmpty()) {
        schemeType = URI::SchemeType::UNRECOGNIZED;
    } else {
        schemeType = URI::SchemeType::NONE;
    }

    return uri.mid(scheme.size(), uri.size());
}

/**
 * Return the domain of the URI
 *
 * For example, example.com in <sip:12345@example.com>
 */
QString URI::hostname() const
{
    if (!pimpl_->m_Parsed)
        pimpl_->parse();
    return pimpl_->m_ExtHostname;
}

/**
 * Check if the URI has an hostname
 *
 * This will return true if there is something between '@' and ';' (or an end of line)
 */
bool URI::hasHostname() const
{
    return hostname().isEmpty();
}

/**
 * If hasHostname() is true, this does a second parsing of the hostname to
 * extract the port.
 */
bool URI::hasPort() const
{
    return port() != -1;
}

/**
 * Return the port, -1 is none is set
 */
int URI::port() const
{
    if (!pimpl_->m_IsHNParsed) {
        pimpl_->parseHostname();
    }
    return pimpl_->m_Port;
}

/**
 * Return the URI SchemeType
 */
URI::SchemeType URI::schemeType() const
{
    if (!pimpl_->m_Parsed)
        const_cast<URI*>(this)->pimpl_->parse();
    return pimpl_->m_HeaderType;
}

/**
 * "Fast" Ipv4 and Ipv6 check, accept 999.999.999.999, :::::::::FF and other
 * atrocities, but at least perform a O(N) ish check and validate the hash
 *
 * @param str an uservalue (faster the scheme and before the "at" sign)
 * @param [out] isHash if the content is pure hexadecimal ASCII
 */
bool URIPimpl::checkIp(const QString& str, bool &isHash, const URI::SchemeType& scheme)
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
    if (!pimpl_->m_Parsed)
        pimpl_->parse();

    if (!pimpl_->m_HintParsed) {
        bool isHash = pimpl_->m_Userinfo.size() == 40;

        URI::ProtocolHint hint;

        //Step 1: Check IP
        if (URIPimpl::checkIp(pimpl_->m_Userinfo, isHash, pimpl_->m_HeaderType)) {
            hint = URI::ProtocolHint::IP;
        }
        //Step 2: Check RING hash
        else if (isHash)
        {
            hint = URI::ProtocolHint::RING;
        }
        //Step 3: Not a hash but it begins with ring:. This is a username.
        else if (pimpl_->m_HeaderType == URI::SchemeType::RING){
            hint = URI::ProtocolHint::RING_USERNAME;
        }
        //Step 4: Check for SIP URIs
        else if (pimpl_->m_HeaderType == URI::SchemeType::SIP)
        {
            //Step 4.1: Check for SIP URI with hostname
            if (pimpl_->m_HasAt) {
                hint = URI::ProtocolHint::SIP_HOST;
            }
            //Step 4.2: Assume SIP URI without hostname
            else {
                hint = URI::ProtocolHint::SIP_OTHER;
            }
        }
        //Step 5: UNRECOGNIZED
        else {
            hint = URI::ProtocolHint::UNRECOGNIZED;
        }

        pimpl_->m_ProtocolHint = hint;
        pimpl_->m_HintParsed = true;
    }
    return pimpl_->m_ProtocolHint;
}

// Convert the transport name to a string
URI::Transport URIPimpl::nameToTransport(const QByteArray& name)
{
    if (name == transportNames.at(URI::Transport::NOT_SET))
        return URI::Transport::NOT_SET;
    else if (name == transportNames.at(URI::Transport::TLS))
        return URI::Transport::TLS;
    else if (name == transportNames.at(URI::Transport::tls))
        return URI::Transport::tls;
    else if (name == transportNames.at(URI::Transport::TCP))
        return URI::Transport::TCP;
    else if (name == transportNames.at(URI::Transport::tcp))
        return URI::Transport::tcp;
    else if (name == transportNames.at(URI::Transport::UDP))
        return URI::Transport::UDP;
    else if (name == transportNames.at(URI::Transport::udp))
        return URI::Transport::udp;
    else if (name == transportNames.at(URI::Transport::SCTP))
        return URI::Transport::SCTP;
    else if (name == transportNames.at(URI::Transport::sctp))
        return URI::Transport::sctp;
    else if (name == transportNames.at(URI::Transport::DTLS))
        return URI::Transport::DTLS;
    else if (name == transportNames.at(URI::Transport::dtls))
        return URI::Transport::dtls;
    return URI::Transport::NOT_SET;
}

// Keep a cache of the values to avoid re-parsing them
void URIPimpl::parse()
{
    //FIXME the indexOf is done twice, the second time could be avoided
    if (linked.indexOf('@') != -1) {
        const QStringList split = linked.split('@');
        m_HasAt       = true;
        m_ExtHostname = split[1];
        m_Userinfo    = split[0];
        m_Parsed      = true;
    } else {
        m_Userinfo = (linked);
    }
}

void URIPimpl::parseAttribute(const QByteArray& extHn, const int start, const int pos)
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

// Extract the hostname, port and attributes
void URIPimpl::parseHostname()
{
    if (!m_Parsed)
        parse();

    const QByteArray extHn = linked.hostname().toLatin1();
    int length(extHn.size()), start(0);
    bool inAttributes = false;

    URI::Section section = URI::Section::HOSTNAME;

    // in case no port, attributes, etc are provided
    m_Hostname2 = linked.hostname();

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
        case ';': // Begin attributes
            if (inAttributes) {
                parseAttribute(extHn, start, i);
            } else {
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
        case '#': // Begin fragments
            //TODO handle fragments to comply to the RFC
            break;
        default:
            break;
        }
    }

    // Get the remaining attribute
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
    if (!pimpl_->m_Parsed)
        pimpl_->parse();
    return pimpl_->m_Userinfo;
}

/**
 * Sometime, some metadata can be used to deduce the scheme even if it wasn't
 * originally known.
 */
void URI::setSchemeType(SchemeType t)
{
    pimpl_->m_HeaderType = t;
    pimpl_->m_Scheme = URIPimpl::schemeNames.at(t);
}

/**
 * Generate a new URI formatted with the sections passed in `sections`
 *
 * It is kept as a QString to avoid the URI class to start reformatting
 * it right away.
 */
QString URI::format(FlagPack<URI::Section> sections) const
{
    if (!pimpl_->m_IsHNParsed) {
        pimpl_->parseHostname();
    }

    QString ret;

    if (sections & URI::Section::CHEVRONS)
        ret += '<';

    if (sections & URI::Section::SCHEME)
        ret += pimpl_->m_Scheme;

    if (sections & URI::Section::USER_INFO)
        ret += pimpl_->m_Userinfo;

    if (sections & URI::Section::HOSTNAME && !pimpl_->m_Hostname2.isEmpty())
        ret += '@' + pimpl_->m_Hostname2;

    if (sections & URI::Section::PORT && pimpl_->m_Port != -1)
        ret += ':' + QString::number(pimpl_->m_Port);

    if (sections & URI::Section::CHEVRONS)
        ret += '>';

    if (sections & URI::Section::TRANSPORT && pimpl_->m_Transport != URI::Transport::NOT_SET)
        ret += ";transport=" + QString(URIPimpl::transportNames.at(pimpl_->m_Transport));

    if (sections & URI::Section::TAG && !pimpl_->m_Tag.isEmpty())
        ret += ";tag=" + pimpl_->m_Tag;

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
        case URI::ProtocolHint::IP:
            stream << QStringLiteral("IP");
            break;
        case URI::ProtocolHint::SIP_HOST:
            stream << QStringLiteral("SIP_HOST");
            break;
        case URI::ProtocolHint::UNRECOGNIZED:
            stream << QStringLiteral("UNRECOGNIZED");
            break;
    }

    return stream;
}
