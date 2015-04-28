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
#include "accountstatusmodel.h"

//System
#include <errno.h>

#ifdef Q_OS_WIN
#include <winerror.h>
#define ESHUTDOWN WSAESHUTDOWN
#define ENODATA WSANO_DATA
#define ETIME WSAETIMEDOUT
#define EPFNOSUPPORT WSAEPROTONOSUPPORT
#define EHOSTDOWN WSAEHOSTDOWN
#define ESTALE WSAESTALE
#define ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#define ETOOMANYREFS WSAETOOMANYREFS
#define EUSERS WSAEUSERS
#define EBADMSG 9905
#define ENOLINK 9918
#define ENOSR 9922
#define ENOSTR 9924
#define EMULTIHOP 2004
#endif

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>

//Ring daemon
#include <account_const.h>

//Ring
#include "dbus/configurationmanager.h"
#include "account.h"
#include "private/account_p.h"


struct AccountStatusRow {
   AccountStatusRow(const QString& ,int, AccountStatusModel::Type);
   QString                  description;
   int                      code       ;
   QDateTime                time       ;
   uint                     counter    ;
   AccountStatusModel::Type type       ;
};

class AccountStatusModelPrivate {
public:
   AccountStatusModelPrivate(Account* parent);
   ~AccountStatusModelPrivate();

   //Attributes
   Account* m_pAccount;
   QVector<AccountStatusRow*> m_lRows;
   static QHash<int,QString> m_shKnownErrors;
   static const QString DEFAULT_PJ_MESSAGE;
   static const QString DEFAULT_USER_MESSAGE;
};

QHash<int,QString> AccountStatusModelPrivate::m_shKnownErrors;
const QString AccountStatusModelPrivate::DEFAULT_USER_MESSAGE = QObject::tr("Unhandled error");
const QString AccountStatusModelPrivate::DEFAULT_PJ_MESSAGE   = "Default status message";

#define PJ_SYS_ERR  12000
#define PJ_SELF_ERR 17100
#define SET_MESSAGE(code) AccountStatusModelPrivate::m_shKnownErrors[code]=
static void init_statuscode()
{
   if (AccountStatusModelPrivate::m_shKnownErrors.size())
      return;

   //The content of this list is mostly copy pasted from various pjproject
   //header. This is done for translation reasons and eventually to associate
   //error with account fields/properties

   //This set represent the standard ietf error code used for SIP[S] and HTTP[S]
   SET_MESSAGE(100) QObject::tr("Trying"                                     );
   SET_MESSAGE(180) QObject::tr("Ringing"                                    );
   SET_MESSAGE(181) QObject::tr("Call Is Being Forwarded"                    );
   SET_MESSAGE(182) QObject::tr("Queued"                                     );
   SET_MESSAGE(183) QObject::tr("Session Progress"                           );

   SET_MESSAGE(200) QObject::tr("OK"                                         );
   SET_MESSAGE(202) QObject::tr("Accepted"                                   );

   SET_MESSAGE(300) QObject::tr("Multiple Choices"                           );
   SET_MESSAGE(301) QObject::tr("Moved Permanently"                          );
   SET_MESSAGE(302) QObject::tr("Moved Temporarily"                          );
   SET_MESSAGE(305) QObject::tr("Use Proxy"                                  );
   SET_MESSAGE(380) QObject::tr("Alternative Service"                        );

   SET_MESSAGE(400) QObject::tr("Bad Request"                                );
   SET_MESSAGE(401) QObject::tr("Unauthorized"                               );
   SET_MESSAGE(402) QObject::tr("Payment Required"                           );
   SET_MESSAGE(403) QObject::tr("Forbidden"                                  );
   SET_MESSAGE(404) QObject::tr("Not Found"                                  );
   SET_MESSAGE(405) QObject::tr("Method Not Allowed"                         );
   SET_MESSAGE(406) QObject::tr("Not Acceptable"                             );
   SET_MESSAGE(407) QObject::tr("Proxy Authentication Required"              );
   SET_MESSAGE(408) QObject::tr("Request Timeout"                            );
   SET_MESSAGE(410) QObject::tr("Gone"                                       );
   SET_MESSAGE(413) QObject::tr("Request Entity Too Large"                   );
   SET_MESSAGE(414) QObject::tr("Request URI Too Long"                       );
   SET_MESSAGE(415) QObject::tr("Unsupported Media Type"                     );
   SET_MESSAGE(416) QObject::tr("Unsupported URI Scheme"                     );
   SET_MESSAGE(420) QObject::tr("Bad Extension"                              );
   SET_MESSAGE(421) QObject::tr("Extension Required"                         );
   SET_MESSAGE(422) QObject::tr("Session Timer Too Small"                    );
   SET_MESSAGE(423) QObject::tr("Interval Too Brief"                         );
   SET_MESSAGE(480) QObject::tr("Temporarily Unavailable"                    );
   SET_MESSAGE(481) QObject::tr("Call/Transaction Does Not Exist"            );
   SET_MESSAGE(482) QObject::tr("Loop Detected"                              );
   SET_MESSAGE(483) QObject::tr("Too Many Hops"                              );
   SET_MESSAGE(484) QObject::tr("Address Incompleted"                        );
   SET_MESSAGE(485) QObject::tr("Ambiguous"                                  );
   SET_MESSAGE(486) QObject::tr("Busy Here"                                  );
   SET_MESSAGE(487) QObject::tr("Request Terminated"                         );
   SET_MESSAGE(488) QObject::tr("Not Acceptable Here"                        );
   SET_MESSAGE(489) QObject::tr("Bad Event"                                  );
   SET_MESSAGE(490) QObject::tr("Request Updated"                            );
   SET_MESSAGE(491) QObject::tr("Request Pending"                            );
   SET_MESSAGE(493) QObject::tr("Undecipherable"                             );

   SET_MESSAGE(500) QObject::tr("Internal Server Error"                      );
   SET_MESSAGE(501) QObject::tr("Not Implemented"                            );
   SET_MESSAGE(502) QObject::tr("Bad Gateway"                                );
   SET_MESSAGE(503) QObject::tr("Service Unavailable"                        );
   SET_MESSAGE(504) QObject::tr("Server Timeout"                             );
   SET_MESSAGE(505) QObject::tr("Version Not Supported"                      );
   SET_MESSAGE(513) QObject::tr("Message Too Large"                          );
   SET_MESSAGE(580) QObject::tr("Precondition Failure"                       );

   SET_MESSAGE(600) QObject::tr("Busy Everywhere"                            );
   SET_MESSAGE(603) QObject::tr("Decline"                                    );
   SET_MESSAGE(604) QObject::tr("Does Not Exist Anywhere"                    );
   SET_MESSAGE(606) QObject::tr("Not Acceptable"                             );

   SET_MESSAGE(701) QObject::tr("No response from destination server"        );
   SET_MESSAGE(702) QObject::tr("Unable to resolve destination server"       );
   SET_MESSAGE(703) QObject::tr("Error sending message to destination server");

   //This section match to POSIX error codes
#ifdef Q_OS_LINUX
   SET_MESSAGE(PJ_SYS_ERR + EBFONT          ) QObject::tr("Bad font file format"                             );
   SET_MESSAGE(PJ_SYS_ERR + ENONET          ) QObject::tr("Machine is not on the network"                    );
   SET_MESSAGE(PJ_SYS_ERR + EADV            ) QObject::tr("Advertise error"                                  );
   SET_MESSAGE(PJ_SYS_ERR + ESRMNT          ) QObject::tr("Srmount error"                                    );
   SET_MESSAGE(PJ_SYS_ERR + ECOMM           ) QObject::tr("Communication error on send"                      );
   SET_MESSAGE(PJ_SYS_ERR + EDOTDOT         ) QObject::tr("RFS specific error"                               );
   SET_MESSAGE(PJ_SYS_ERR + ENOTUNIQ        ) QObject::tr("Name not unique on network"                       );
   SET_MESSAGE(PJ_SYS_ERR + EBADFD          ) QObject::tr("File descriptor in bad state"                     );
   SET_MESSAGE(PJ_SYS_ERR + EREMCHG         ) QObject::tr("Remote address changed"                           );
   SET_MESSAGE(PJ_SYS_ERR + ELIBACC         ) QObject::tr("Can not access a needed shared library"           );
   SET_MESSAGE(PJ_SYS_ERR + ELIBBAD         ) QObject::tr("Accessing a corrupted shared library"             );
   SET_MESSAGE(PJ_SYS_ERR + ELIBMAX         ) QObject::tr("Attempting to link in too many shared libraries"  );
   SET_MESSAGE(PJ_SYS_ERR + ELIBEXEC        ) QObject::tr("Cannot exec a shared library directly"            );
   SET_MESSAGE(PJ_SYS_ERR + ERESTART        ) QObject::tr("Interrupted system call should be restarted"      );
   SET_MESSAGE(PJ_SYS_ERR + ESTRPIPE        ) QObject::tr("Streams pipe error"                               );
   SET_MESSAGE(PJ_SYS_ERR + EUCLEAN         ) QObject::tr("Structure needs cleaning"                         );
   SET_MESSAGE(PJ_SYS_ERR + ENOTNAM         ) QObject::tr("Not a XENIX named type file"                      );
   SET_MESSAGE(PJ_SYS_ERR + ENAVAIL         ) QObject::tr("No XENIX semaphores available"                    );
   SET_MESSAGE(PJ_SYS_ERR + EISNAM          ) QObject::tr("Is a named type file"                             );
   SET_MESSAGE(PJ_SYS_ERR + EREMOTEIO       ) QObject::tr("Remote I/O error"                                 );
   SET_MESSAGE(PJ_SYS_ERR + ENOMEDIUM       ) QObject::tr("No medium found"                                  );
   SET_MESSAGE(PJ_SYS_ERR + EMEDIUMTYPE     ) QObject::tr("Wrong medium type"                                );
   SET_MESSAGE(PJ_SYS_ERR + ENOKEY          ) QObject::tr("Required key not available"                       );
   SET_MESSAGE(PJ_SYS_ERR + EKEYEXPIRED     ) QObject::tr("Key has expired"                                  );
   SET_MESSAGE(PJ_SYS_ERR + EKEYREVOKED     ) QObject::tr("Key has been revoked"                             );
   SET_MESSAGE(PJ_SYS_ERR + EKEYREJECTED    ) QObject::tr("Key was rejected by service"                      );
   SET_MESSAGE(PJ_SYS_ERR + EDQUOT          ) QObject::tr("Quota exceeded"                                   );
   SET_MESSAGE(PJ_SYS_ERR + ECANCELED       ) QObject::tr("Operation Canceled"                               );
#endif
   SET_MESSAGE(PJ_SYS_ERR + ENOSTR          ) QObject::tr("Device not a stream"                              );
   SET_MESSAGE(PJ_SYS_ERR + ENODATA         ) QObject::tr("No data available"                                );
   SET_MESSAGE(PJ_SYS_ERR + ETIME           ) QObject::tr("Timer expired"                                    );
   SET_MESSAGE(PJ_SYS_ERR + ENOSR           ) QObject::tr("Out of streams resources"                         );
   SET_MESSAGE(PJ_SYS_ERR + ENOLINK         ) QObject::tr("Link has been severed"                            );
   SET_MESSAGE(PJ_SYS_ERR + EPROTO          ) QObject::tr("Protocol error"                                   );
   SET_MESSAGE(PJ_SYS_ERR + EMULTIHOP       ) QObject::tr("Multihop attempted"                               );
   SET_MESSAGE(PJ_SYS_ERR + EBADMSG         ) QObject::tr("Not a data message"                               );
   SET_MESSAGE(PJ_SYS_ERR + EOVERFLOW       ) QObject::tr("Value too large for defined data type"            );
   SET_MESSAGE(PJ_SYS_ERR + EILSEQ          ) QObject::tr("Illegal byte sequence"                            );
   SET_MESSAGE(PJ_SYS_ERR + EUSERS          ) QObject::tr("Too many users"                                   );
   SET_MESSAGE(PJ_SYS_ERR + ENOTSOCK        ) QObject::tr("Socket operation on non-socket"                   );
   SET_MESSAGE(PJ_SYS_ERR + EDESTADDRREQ    ) QObject::tr("Destination address required"                     );
   SET_MESSAGE(PJ_SYS_ERR + EMSGSIZE        ) QObject::tr("Message too long"                                 );
   SET_MESSAGE(PJ_SYS_ERR + EPROTOTYPE      ) QObject::tr("Protocol wrong type for socket"                   );
   SET_MESSAGE(PJ_SYS_ERR + ENOPROTOOPT     ) QObject::tr("Protocol not available"                           );
   SET_MESSAGE(PJ_SYS_ERR + EPROTONOSUPPORT ) QObject::tr("Protocol not supported"                           );
   SET_MESSAGE(PJ_SYS_ERR + ESOCKTNOSUPPORT ) QObject::tr("Socket type not supported"                        );
   SET_MESSAGE(PJ_SYS_ERR + EOPNOTSUPP      ) QObject::tr("Operation not supported on transport endpoint"    );
   SET_MESSAGE(PJ_SYS_ERR + EPFNOSUPPORT    ) QObject::tr("Protocol family not supported"                    );
   SET_MESSAGE(PJ_SYS_ERR + EAFNOSUPPORT    ) QObject::tr("Address family not supported by protocol"         );
   SET_MESSAGE(PJ_SYS_ERR + EADDRINUSE      ) QObject::tr("Address already in use"                           );
   SET_MESSAGE(PJ_SYS_ERR + EADDRNOTAVAIL   ) QObject::tr("Cannot assign requested address"                  );
   SET_MESSAGE(PJ_SYS_ERR + ENETDOWN        ) QObject::tr("Network is down"                                  );
   SET_MESSAGE(PJ_SYS_ERR + ENETUNREACH     ) QObject::tr("Network is unreachable"                           );
   SET_MESSAGE(PJ_SYS_ERR + ENETRESET       ) QObject::tr("Network dropped connection because of reset"      );
   SET_MESSAGE(PJ_SYS_ERR + ECONNABORTED    ) QObject::tr("Software caused connection abort"                 );
   SET_MESSAGE(PJ_SYS_ERR + ECONNRESET      ) QObject::tr("Connection reset by peer"                         );
   SET_MESSAGE(PJ_SYS_ERR + ENOBUFS         ) QObject::tr("No buffer space available"                        );
   SET_MESSAGE(PJ_SYS_ERR + EISCONN         ) QObject::tr("Transport endpoint is already connected"          );
   SET_MESSAGE(PJ_SYS_ERR + ENOTCONN        ) QObject::tr("Transport endpoint is not connected"              );
   SET_MESSAGE(PJ_SYS_ERR + ESHUTDOWN       ) QObject::tr("Cannot send after transport endpoint shutdown"    );
   SET_MESSAGE(PJ_SYS_ERR + ETOOMANYREFS    ) QObject::tr("Too many references: cannot splice"               );
   SET_MESSAGE(PJ_SYS_ERR + ETIMEDOUT       ) QObject::tr("Connection timed out"                             );
   SET_MESSAGE(PJ_SYS_ERR + ECONNREFUSED    ) QObject::tr("Connection refused"                               );
   SET_MESSAGE(PJ_SYS_ERR + EHOSTDOWN       ) QObject::tr("Host is down"                                     );
   SET_MESSAGE(PJ_SYS_ERR + EHOSTUNREACH    ) QObject::tr("No route to host"                                 );
   SET_MESSAGE(PJ_SYS_ERR + EALREADY        ) QObject::tr("Operation already in progress"                    );
   SET_MESSAGE(PJ_SYS_ERR + EINPROGRESS     ) QObject::tr("Operation now in progress"                        );
   SET_MESSAGE(PJ_SYS_ERR + ESTALE          ) QObject::tr("Stale file handle"                                );

   //The next sections represent pjproject specific errors

   /* Generic SIP errors */
   SET_MESSAGE(PJ_SELF_ERR + 01 ) QObject::tr("Object is busy"                                     );/* PJSIP_EBUSY               */
   SET_MESSAGE(PJ_SELF_ERR + 02 ) QObject::tr("Object with the same type exists"                   );/* PJSIP_ETYPEEXISTS         */
   SET_MESSAGE(PJ_SELF_ERR + 03 ) QObject::tr("SIP stack shutting down"                            );/* PJSIP_ESHUTDOWN           */
   SET_MESSAGE(PJ_SELF_ERR + 04 ) QObject::tr("SIP object is not initialized."                     );/* PJSIP_ENOTINITIALIZED     */
   SET_MESSAGE(PJ_SELF_ERR + 05 ) QObject::tr("Missing route set (for tel: URI)"                   );/* PJSIP_ENOROUTESET         */

   /* Messaging errors */
   SET_MESSAGE(PJ_SELF_ERR + 20 ) QObject::tr("Invalid message/syntax error"                       );/* PJSIP_EINVALIDMSG         */
   SET_MESSAGE(PJ_SELF_ERR + 21 ) QObject::tr("Expecting request message"                          );/* PJSIP_ENOTREQUESTMSG      */
   SET_MESSAGE(PJ_SELF_ERR + 22 ) QObject::tr("Expecting response message"                         );/* PJSIP_ENOTRESPONSEMSG     */
   SET_MESSAGE(PJ_SELF_ERR + 23 ) QObject::tr("Message too long"                                   );/* PJSIP_EMSGTOOLONG         */
   SET_MESSAGE(PJ_SELF_ERR + 24 ) QObject::tr("Partial message"                                    );/* PJSIP_EPARTIALMSG         */

   SET_MESSAGE(PJ_SELF_ERR + 30 ) QObject::tr("Invalid/unexpected SIP status code"                 );/* PJSIP_EINVALIDSTATUS      */

   SET_MESSAGE(PJ_SELF_ERR + 39 ) QObject::tr("Invalid URI"                                        );/* PJSIP_EINVALIDURI         */
   SET_MESSAGE(PJ_SELF_ERR + 40 ) QObject::tr("Invalid URI scheme"                                 );/* PJSIP_EINVALIDSCHEME      */
   SET_MESSAGE(PJ_SELF_ERR + 41 ) QObject::tr("Missing Request-URI"                                );/* PJSIP_EMISSINGREQURI      */
   SET_MESSAGE(PJ_SELF_ERR + 42 ) QObject::tr("Invalid Request URI"                                );/* PJSIP_EINVALIDREQURI      */
   SET_MESSAGE(PJ_SELF_ERR + 43 ) QObject::tr("URI is too long"                                    );/* PJSIP_EURITOOLONG         */

   SET_MESSAGE(PJ_SELF_ERR + 50 ) QObject::tr("Missing required header(s)"                         );/* PJSIP_EMISSINGHDR         */
   SET_MESSAGE(PJ_SELF_ERR + 51 ) QObject::tr("Invalid header field"                               );/* PJSIP_EINVALIDHDR         */
   SET_MESSAGE(PJ_SELF_ERR + 52 ) QObject::tr("Invalid Via header"                                 );/* PJSIP_EINVALIDVIA         */
   SET_MESSAGE(PJ_SELF_ERR + 53 ) QObject::tr("Multiple Via headers in response"                   );/* PJSIP_EMULTIPLEVIA        */

   SET_MESSAGE(PJ_SELF_ERR + 54 ) QObject::tr("Missing message body"                               );/* PJSIP_EMISSINGBODY        */
   SET_MESSAGE(PJ_SELF_ERR + 55 ) QObject::tr("Invalid/unexpected method"                          );/* PJSIP_EINVALIDMETHOD      */

   /* Transport errors */
   SET_MESSAGE(PJ_SELF_ERR + 60 ) QObject::tr("Unsupported transport"                              );/* PJSIP_EUNSUPTRANSPORT     */
   SET_MESSAGE(PJ_SELF_ERR + 61 ) QObject::tr("Transmit buffer already pending"                    );/* PJSIP_EPENDINGTX          */
   SET_MESSAGE(PJ_SELF_ERR + 62 ) QObject::tr("Rx buffer overflow"                                 );/* PJSIP_ERXOVERFLOW         */
   SET_MESSAGE(PJ_SELF_ERR + 63 ) QObject::tr("Buffer destroyed"                                   );/* PJSIP_EBUFDESTROYED       */
   SET_MESSAGE(PJ_SELF_ERR + 64 ) QObject::tr("Unsuitable transport selected"                      );/* PJSIP_ETPNOTSUITABLE      */
   SET_MESSAGE(PJ_SELF_ERR + 65 ) QObject::tr("Transport not available for use"                    );/* PJSIP_ETPNOTAVAIL         */

   /* Transaction errors */
   SET_MESSAGE(PJ_SELF_ERR + 70 ) QObject::tr("Transaction has been destroyed"                     );/* PJSIP_ETSXDESTROYED       */
   SET_MESSAGE(PJ_SELF_ERR + 71 ) QObject::tr("No transaction is associated with the object "
                                   "(expecting stateful processing)"                    );/* PJSIP_ENOTSX              */

   /* URI comparison status */
   SET_MESSAGE(PJ_SELF_ERR + 80 ) QObject::tr("URI scheme mismatch"                                );/* PJSIP_ECMPSCHEME          */
   SET_MESSAGE(PJ_SELF_ERR + 81 ) QObject::tr("URI user part mismatch"                             );/* PJSIP_ECMPUSER            */
   SET_MESSAGE(PJ_SELF_ERR + 82 ) QObject::tr("URI password part mismatch"                         );/* PJSIP_ECMPPASSWD          */
   SET_MESSAGE(PJ_SELF_ERR + 83 ) QObject::tr("URI host part mismatch"                             );/* PJSIP_ECMPHOST            */
   SET_MESSAGE(PJ_SELF_ERR + 84 ) QObject::tr("URI port mismatch"                                  );/* PJSIP_ECMPPORT            */
   SET_MESSAGE(PJ_SELF_ERR + 85 ) QObject::tr("URI transport param mismatch"                       );/* PJSIP_ECMPTRANSPORTPRM    */
   SET_MESSAGE(PJ_SELF_ERR + 86 ) QObject::tr("URI ttl param mismatch"                             );/* PJSIP_ECMPTTLPARAM        */
   SET_MESSAGE(PJ_SELF_ERR + 87 ) QObject::tr("URI user param mismatch"                            );/* PJSIP_ECMPUSERPARAM       */
   SET_MESSAGE(PJ_SELF_ERR + 88 ) QObject::tr("URI method param mismatch"                          );/* PJSIP_ECMPMETHODPARAM     */
   SET_MESSAGE(PJ_SELF_ERR + 89 ) QObject::tr("URI maddr param mismatch"                           );/* PJSIP_ECMPMADDRPARAM      */
   SET_MESSAGE(PJ_SELF_ERR + 90 ) QObject::tr("URI other param mismatch"                           );/* PJSIP_ECMPOTHERPARAM      */
   SET_MESSAGE(PJ_SELF_ERR + 91 ) QObject::tr("URI header parameter mismatch"                      );/* PJSIP_ECMPHEADERPARAM     */

   /* Authentication. */
   SET_MESSAGE(PJ_SELF_ERR + 100) QObject::tr("Credential failed to authenticate"                  );/* PJSIP_EFAILEDCREDENTIAL   */
   SET_MESSAGE(PJ_SELF_ERR + 101) QObject::tr("No suitable credential"                             );/* PJSIP_ENOCREDENTIAL       */
   SET_MESSAGE(PJ_SELF_ERR + 102) QObject::tr("Invalid/unsupported digest algorithm"               );/* PJSIP_EINVALIDALGORITHM   */
   SET_MESSAGE(PJ_SELF_ERR + 103) QObject::tr("Invalid/unsupported digest qop"                     );/* PJSIP_EINVALIDQOP         */
   SET_MESSAGE(PJ_SELF_ERR + 104) QObject::tr("Unsupported authentication scheme"                  );/* PJSIP_EINVALIDAUTHSCHEME  */
   SET_MESSAGE(PJ_SELF_ERR + 105) QObject::tr("No previous challenge"                              );/* PJSIP_EAUTHNOPREVCHAL     */
   SET_MESSAGE(PJ_SELF_ERR + 106) QObject::tr("No suitable authorization header"                   );/* PJSIP_EAUTHNOAUTH         */
   SET_MESSAGE(PJ_SELF_ERR + 107) QObject::tr("Account or credential not found"                    );/* PJSIP_EAUTHACCNOTFOUND    */
   SET_MESSAGE(PJ_SELF_ERR + 108) QObject::tr("Account or credential is disabled"                  );/* PJSIP_EAUTHACCDISABLED    */
   SET_MESSAGE(PJ_SELF_ERR + 109) QObject::tr("Invalid authorization realm"                        );/* PJSIP_EAUTHINVALIDREALM   */
   SET_MESSAGE(PJ_SELF_ERR + 110) QObject::tr("Invalid authorization digest"                       );/* PJSIP_EAUTHINVALIDDIGEST  */
   SET_MESSAGE(PJ_SELF_ERR + 111) QObject::tr("Maximum number of stale retries exceeded"           );/* PJSIP_EAUTHSTALECOUNT     */
   SET_MESSAGE(PJ_SELF_ERR + 112) QObject::tr("Invalid nonce value in authentication challenge"    );/* PJSIP_EAUTHINNONCE        */
   SET_MESSAGE(PJ_SELF_ERR + 113) QObject::tr("Invalid AKA credential"                             );/* PJSIP_EAUTHINAKACRED      */
   SET_MESSAGE(PJ_SELF_ERR + 114) QObject::tr("No challenge is found"                              );/* PJSIP_EAUTHNOCHAL         */

   /* UA/dialog layer. */
   SET_MESSAGE(PJ_SELF_ERR + 120) QObject::tr("Missing From/To tag parameter"                      );/* PJSIP_EMISSINGTAG         */
   SET_MESSAGE(PJ_SELF_ERR + 121) QObject::tr("Expecting REFER request"                            );/* PJSIP_ENOTREFER           */
   SET_MESSAGE(PJ_SELF_ERR + 122) QObject::tr("Not associated with REFER subscription"             );/* PJSIP_ENOREFERSESSION     */

   /* Invite session. */
   SET_MESSAGE(PJ_SELF_ERR + 140) QObject::tr("INVITE session already terminated"                  );/*  PJSIP_ESESSIONTERMINATED */
   SET_MESSAGE(PJ_SELF_ERR + 141) QObject::tr("Invalid INVITE session state"                       );/*  PJSIP_ESESSIONSTATE      */
   SET_MESSAGE(PJ_SELF_ERR + 142) QObject::tr("Require secure session/transport"                   );/*  PJSIP_ESESSIONINSECURE   */

   /* SSL errors */
   SET_MESSAGE(PJ_SELF_ERR + 160) QObject::tr("Unknown TLS error"                                  );/* PJSIP_TLS_EUNKNOWN         */
   SET_MESSAGE(PJ_SELF_ERR + 161) QObject::tr("Invalid SSL protocol method"                        );/* PJSIP_TLS_EINVMETHOD       */
   SET_MESSAGE(PJ_SELF_ERR + 162) QObject::tr("Error loading/verifying SSL CA list file"           );/* PJSIP_TLS_ECACERT          */
   SET_MESSAGE(PJ_SELF_ERR + 163) QObject::tr("Error loading SSL certificate chain file"           );/* PJSIP_TLS_ECERTFILE        */
   SET_MESSAGE(PJ_SELF_ERR + 164) QObject::tr("Error adding private key from SSL certificate file" );/* PJSIP_TLS_EKEYFILE         */
   SET_MESSAGE(PJ_SELF_ERR + 165) QObject::tr("Error setting SSL cipher list"                      );/* PJSIP_TLS_ECIPHER          */
   SET_MESSAGE(PJ_SELF_ERR + 166) QObject::tr("Error creating SSL context"                         );/* PJSIP_TLS_ECTX             */
   SET_MESSAGE(PJ_SELF_ERR + 167) QObject::tr("Error creating SSL connection object"               );/* PJSIP_TLS_ESSLCONN         */
   SET_MESSAGE(PJ_SELF_ERR + 168) QObject::tr("Unknown error when performing SSL connect()"        );/* PJSIP_TLS_ECONNECT         */
   SET_MESSAGE(PJ_SELF_ERR + 169) QObject::tr("Unknown error when performing SSL accept()"         );/* PJSIP_TLS_EACCEPT          */
   SET_MESSAGE(PJ_SELF_ERR + 170) QObject::tr("Unknown error when sending SSL data"                );/* PJSIP_TLS_ESEND            */
   SET_MESSAGE(PJ_SELF_ERR + 171) QObject::tr("Unknown error when reading SSL data"                );/* PJSIP_TLS_EREAD            */
   SET_MESSAGE(PJ_SELF_ERR + 172) QObject::tr("SSL negotiation has timed out"                      );/* PJSIP_TLS_ETIMEDOUT        */
   SET_MESSAGE(PJ_SELF_ERR + 173) QObject::tr("SSL certificate verification error"                 );/* PJSIP_TLS_ECERTVERIF       */

}
#undef SET_MESSAGE
#undef PJ_SELF_ERR
#undef PJ_SYS_ERR


AccountStatusRow::AccountStatusRow(const QString& _description, int _code, AccountStatusModel::Type _type):
code(_code),counter(0),
time(QDateTime::currentDateTime()),type(_type)
{
   description = AccountStatusModelPrivate::m_shKnownErrors[code];

   if (AccountStatusModelPrivate::m_shKnownErrors[code].isEmpty())
      description = _description==AccountStatusModelPrivate::DEFAULT_PJ_MESSAGE?
         AccountStatusModelPrivate::DEFAULT_USER_MESSAGE : _description;
}

AccountStatusModelPrivate::AccountStatusModelPrivate(Account* parent) : m_pAccount(parent)
{
   init_statuscode();
}

AccountStatusModelPrivate::~AccountStatusModelPrivate()
{
   for (int i=0;i<m_lRows.size();i++)
      delete m_lRows[i];
   m_lRows.clear();
}

AccountStatusModel::AccountStatusModel(Account* parent) : QAbstractTableModel(parent),
d_ptr(new AccountStatusModelPrivate(parent))
{}

AccountStatusModel::~AccountStatusModel()
{}

QHash<int,QByteArray> AccountStatusModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

//Model functions
QVariant AccountStatusModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   switch(static_cast<Columns>(index.column())) {
      case Columns::DESCRIPTION:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->description;
         };
         break;
      case Columns::CODE:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->code;
         };
         break;
      case Columns::TIME:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->time;
         };
         break;
      case Columns::COUNTER:
         switch (role) {
            case Qt::DisplayRole:
               return d_ptr->m_lRows[index.row()]->counter;
         };
         break;
   };
   return QVariant();
}

int AccountStatusModel::rowCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return d_ptr->m_lRows.size();
}

int AccountStatusModel::columnCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return 4;
}

Qt::ItemFlags AccountStatusModel::flags( const QModelIndex& index ) const
{
   return (index.isValid()) ? (Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsUserCheckable) : Qt::NoItemFlags;
}

bool AccountStatusModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}


void AccountStatusModel::addSipRegistrationEvent(const QString& fallbackMessage, int errorCode)
{
   if (errorCode != d_ptr->m_pAccount->lastErrorCode()) {
      beginInsertRows(QModelIndex(), d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
      d_ptr->m_lRows << new AccountStatusRow(fallbackMessage, errorCode, Type::SIP);
      endInsertRows();
   }
   else
      d_ptr->m_lRows.last()->counter++;
}

void AccountStatusModel::addTransportEvent(const QString& fallbackMessage, int errorCode)
{
   if ((!d_ptr->m_lRows.size()) || errorCode != d_ptr->m_pAccount->lastTransportErrorCode()) {
      beginInsertRows(QModelIndex(), d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
      d_ptr->m_lRows << new AccountStatusRow(fallbackMessage, errorCode, Type::TRANSPORT);
      endInsertRows();
   }
   else
      d_ptr->m_lRows.last()->counter++;
}
