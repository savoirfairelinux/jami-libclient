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

class SecurityEvaluationModelPrivate;

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
class LIB_EXPORT SecurityEvaluationModel : public QSortFilterProxyModel {
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

   //Properties
   Q_PROPERTY(int           informationCount  READ informationCount  NOTIFY informationCountChanged  )
   Q_PROPERTY(int           warningCount      READ warningCount      NOTIFY warningCountChanged      )
   Q_PROPERTY(int           issueCount        READ issueCount        NOTIFY issueCountChanged        )
   Q_PROPERTY(int           errorCount        READ errorCount        NOTIFY errorCountChanged        )
   Q_PROPERTY(int           fatalWarningCount READ fatalWarningCount NOTIFY fatalWarningCountChanged )
   Q_PROPERTY(SecurityLevel securityLevel     READ securityLevel     NOTIFY securityLevelChanged     )

   ///Give the user an overview of the current security state
   enum class SecurityLevel {
      NONE        = 0, /*!< Security is not functional or severely defective               */
      WEAK        = 1, /*!< There is some security, but way too many flaws                 */
      MEDIUM      = 2, /*!< The security is probably good enough, but there is issues      */
      ACCEPTABLE  = 3, /*!< The security is most probably good enough, only minor issues   */
      STRONG      = 4, /*!< All the non-information items are correct                      */
      COMPLETE    = 5, /*!< Everything, even the recommendations, are correct              */
      COUNT__,
   };
   Q_ENUMS(SecurityLevel)

   ///The severity of a given flaw
   enum class Severity {
      UNSUPPORTED   = 0, /*!< This severity is unsupported, to be ignored                  */
      INFORMATION   = 1, /*!< Tip and tricks to have better security                       */
      WARNING       = 2, /*!< It is a problem, but it wont have other side effects         */
      ISSUE         = 3, /*!< The security is compromised                                  */
      ERROR         = 4, /*!< It simply wont work (REGISTER)                               */
      FATAL_WARNING = 5, /*!< Registration may work, but it render everything else useless */
      COUNT__,
   };
   Q_ENUMS(Severity)

   ///Every supported flaws
   enum class AccountSecurityChecks {
      SRTP_ENABLED                , /*!< The account use secure media streams                    */
      TLS_ENABLED                 , /*!< The account use secure negotiation                      */
      CERTIFICATE_MATCH           , /*!< The certificate an authority are related                */
      OUTGOING_SERVER_MATCH       , /*!< The outgoing server match the certificate hostname      */
      VERIFY_INCOMING_ENABLED     , /*!< The incoming certificates are validated                 */
      VERIFY_ANSWER_ENABLED       , /*!< The answer certificates are validated                   */
      REQUIRE_CERTIFICATE_ENABLED , /*!< The account require certificates to operate in TLS mode */
      NOT_MISSING_CERTIFICATE     , /*!< The certificate is set                                  */
      NOT_MISSING_AUTHORITY       , /*!< The certificate authority is set                        */
      COUNT__
   };
   Q_ENUMS(AccountSecurityChecks)

   ///Role for the model
   enum class Role {
      Severity      = 100,
      SecurityLevel = 101,
   };

   ///Source of a security flaw
   enum class ChecksSource {
      ACCOUNT_CERTIFICATE  = 0, /*!< The flaw is induced by the account certificate                  */
      ACCOUNT_AUTHORITY    = 1, /*!< The flaw is induced by the account authority certificate        */
      ACCOUNT_SETTINGS     = 2, /*!< The flaw is induced by an account misconfiguration              */
      ACCOUNT_REGISTRATION = 3, /*!< The flaw has been detected by runtime registration checks       */
      CALL_DETAILS         = 4, /*!< The flaw has been detected by runtime communication negotiation */
   };

   //Constructor
   explicit SecurityEvaluationModel(Account* account);
   virtual ~SecurityEvaluationModel();

   //Model functions
   virtual QHash<int,QByteArray> roleNames() const override;
   virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

   //Getter
   QList<SecurityFlaw*> currentFlaws();
   QModelIndex getIndex(const SecurityFlaw* flaw);
   int           informationCount () const;
   int           warningCount     () const;
   int           issueCount       () const;
   int           errorCount       () const;
   int           fatalWarningCount() const;
   SecurityLevel securityLevel    () const;

Q_SIGNALS:
   void informationCountChanged ();
   void warningCountChanged     ();
   void issueCountChanged       ();
   void errorCountChanged       ();
   void fatalWarningCountChanged();
   void securityLevelChanged    ();

private:
   SecurityEvaluationModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(SecurityEvaluationModel)
};
Q_DECLARE_METATYPE(SecurityEvaluationModel*)
Q_DECLARE_METATYPE(SecurityEvaluationModel::Severity)
Q_DECLARE_METATYPE(SecurityEvaluationModel::SecurityLevel)

#endif
