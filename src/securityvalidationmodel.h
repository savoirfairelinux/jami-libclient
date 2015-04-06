/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#ifndef SECURITYVALIDATIONMODEL_H
#define SECURITYVALIDATIONMODEL_H

#include <QtCore/QSortFilterProxyModel>

//Ring
#include "certificate.h"
#include "typedefs.h"


//Ring
class Account;
class SecurityFlaw;

class SecurityValidationModelPrivate;

/**
 * This model provide a real time look at elements security. It aggregate data
 * from various security related sources, assign a severity and sort them.
 *
 * It can also generate a summarized report on the asset security.
 *
 * This use a best effort approach. Not all known or possible attacks are handled
 * by this system. Result should be taken with a grain of salt, but at least some
 * common problems can be detected.
 */
class LIB_EXPORT SecurityValidationModel : public QSortFilterProxyModel {
   Q_OBJECT
   friend class SecurityFlaw;
   friend class AccountPrivate;
public:
   /*
    * This class evaluate the overall security of an account.
    * It does so by checking various potential flaws, then create
    * a metric called SecurityLevel. This model should be used to:
    * 
    * 1) List all potential flaws
    * 2) Decide if an account can be considered secure
    * 3) Decide if a call can be considered secure
    * 
    * End users should not have to be security gurus to setup Ring. It is our
    * job to do as much as we can to make security configuration as transparent as
    * possible.
    * 
    * The SecurityLevel is computed by checking all possible flaw. The level cannot be
    * higher than a flaw maximum security level. If there is 2 (or more) flaw in the same
    * maximum level, the maximum level will be decreased by one (recursively).
    * 
    * A flaw severity is used by the client to display the right icon ( (i), /!\, [x] ).
    */

   ///Give the user an overview of the current security state
   enum class SecurityLevel {
      NONE        = 0, /* Security is not functional or severely defective              */
      WEAK        = 1, /* There is some security, but way too many flaws                */
      MEDIUM      = 2, /* The security is probably good enough, but there is issues     */
      ACCEPTABLE  = 3, /* The security is most probably good enough, only minor issues  */
      STRONG      = 4, /* All the non-information items are correct                     */
      COMPLETE    = 5, /* Everything, even the recommendations, are correct             */
   };

   ///The severity of a given flaw
   enum class Severity {
      UNSUPPORTED  , /*!< This severity is unsupported, to be ignored                     */
      INFORMATION  , /*!< Tip and tricks to have better security                          */
      WARNING      , /*!< It is a problem, but it wont have other side effects            */
      ISSUE        , /*!< The security is compromised                                     */
      ERROR        , /*!< It simply wont work (REGISTER)                                  */
      FATAL_WARNING, /*!< Registration may work, but it render everything else useless    */
   };
   Q_ENUMS(Severity)

   ///Every supported flaws
   enum class AccountSecurityFlaw {
      SRTP_DISABLED                  ,
      TLS_DISABLED                   ,
      CERTIFICATE_EXPIRED            ,
      CERTIFICATE_SELF_SIGNED        ,
      CA_CERTIFICATE_MISSING         ,
      END_CERTIFICATE_MISSING        ,
      PRIVATE_KEY_MISSING            ,
      CERTIFICATE_MISMATCH           ,
      CERTIFICATE_STORAGE_PERMISSION ,
      CERTIFICATE_STORAGE_FOLDER     ,
      CERTIFICATE_STORAGE_LOCATION   ,
      OUTGOING_SERVER_MISMATCH       ,
      VERIFY_INCOMING_DISABLED       ,
      VERIFY_ANSWER_DISABLED         ,
      REQUIRE_CERTIFICATE_DISABLED   ,
      MISSING_CERTIFICATE            ,
      MISSING_AUTHORITY              ,
      COUNT__
   };

   ///Role for the model
   enum class Role {
      Severity = 100
   };

   //Constructor
   explicit SecurityValidationModel(Account* account);
   virtual ~SecurityValidationModel();

   //Model functions
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   QList<SecurityFlaw*> currentFlaws();
   QModelIndex getIndex(const SecurityFlaw* flaw);

   //Setters
   void setTlsCaListCertificate    ( Certificate* cert );
   void setTlsCertificate          ( Certificate* cert );
   void setTlsPrivateKeyCertificate( Certificate* cert );

private:
   SecurityValidationModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(SecurityValidationModel)
};
Q_DECLARE_METATYPE(SecurityValidationModel*)
Q_DECLARE_METATYPE(SecurityValidationModel::Severity)

#endif
