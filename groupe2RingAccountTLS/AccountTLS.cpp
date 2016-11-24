#include "accountTLS.h"

//Qt
#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMimeData>
#include <QtCore/QItemSelectionModel>


//Ring daemon
#include <account_const.h>


//Ring lib


///Constructors



TlsMethodModel* AccountTLS::tlsMethodModel() const
{
   if (!_account->(m_acc->d_ptr)->m_pTlsMethodModel ) {
      (m_acc->d_ptr)->m_pTlsMethodModel  = new TlsMethodModel(const_cast<Account*>(this));
   }
   return (m_acc->d_ptr)->m_pTlsMethodModel;
}


///Return the account tls password
QString AccountTLS::tlsPassword() const
{
   return (m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::PASSWORD);
}

///Return the account TLS port
int AccountTLS::bootstrapPort() const
{
   return (m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::DHT::PORT).toInt();
}

///Return the account TLS certificate authority list file
Certificate* AccountTLS::tlsCaListCertificate() const
{
   if (!(m_acc->d_ptr)->m_pCaCert) {
      const QString& path = (m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::CA_LIST_FILE);
      if (path.isEmpty())
         return nullptr;
      (m_acc->d_ptr)->m_pCaCert = CertificateModel::instance().getCertificateFromPath(path,Certificate::Type::AUTHORITY);
      connect((m_acc->d_ptr)->m_pCaCert,SIGNAL(changed()),(m_acc->d_ptr).data(),SLOT(slotUpdateCertificate()));
   }
   return (m_acc->d_ptr)->m_pCaCert;
}

///Return the account TLS certificate
Certificate* AccountTLS::tlsCertificate() const
{
   if (!(m_acc->d_ptr)->m_pTlsCert) {
      const QString& path = (m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE);
      if (path.isEmpty())
         return nullptr;
      (m_acc->d_ptr)->m_pTlsCert = CertificateModel::instance().getCertificateFromPath(path,Certificate::Type::USER);
      connect((m_acc->d_ptr)->m_pTlsCert,SIGNAL(changed()),(m_acc->d_ptr).data(),SLOT(slotUpdateCertificate()));
   }
   return (m_acc->d_ptr)->m_pTlsCert;
}

///Return the account private key
QString AccountTLS::tlsPrivateKey() const
{
   return tlsCertificate() ? tlsCertificate()->privateKeyPath() : QString();
}

///Return the account TLS server name
QString AccountTLS::tlsServerName() const
{
   return (m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::SERVER_NAME);
}

///Return the account negotiation timeout in seconds
int AccountTLS::tlsNegotiationTimeoutSec() const
{
   return (m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC).toInt();
}

///Return the account TLS verify server
bool AccountTLS::isTlsVerifyServer() const
{
   return ((m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::VERIFY_SERVER) IS_TRUE);
}

///Return the account TLS verify client
bool AccountTLS::isTlsVerifyClient() const
{
   return ((m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::VERIFY_CLIENT) IS_TRUE);
}

///Return if it is required for the peer to have a certificate
bool AccountTLS::isTlsRequireClientCertificate() const
{
   return ((m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE) IS_TRUE);
}

///Return the account TLS security is enabled
bool AccountTLS::isTlsEnabled() const
{
   return _account->protocol() == Account::Protocol::RING || ((m_acc->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::ENABLED) IS_TRUE);
}


///Set the TLS (encryption) password
void AccountTLS::setTlsPassword(const QString& detail)
{
   auto cert = tlsCertificate();
   if (!cert)
      return;
   cert->setPrivateKeyPassword(detail);
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::PASSWORD, detail);
   (m_acc->d_ptr)->regenSecurityValidation();
}

///Set the certificate authority list file
void AccountTLS::setTlsCaListCertificate(const QString& path)
{
   Certificate* cert = CertificateModel::instance().getCertificateFromPath(path);
   setTlsCaListCertificate(cert);
}

///Set the certificate
void AccountTLS::setTlsCertificate(const QString& path)
{
   Certificate* cert = CertificateModel::instance().getCertificateFromPath(path);
   setTlsCertificate(cert);
}

///Set the private key
void AccountTLS::setTlsPrivateKey(const QString& path)
{
    auto cert = tlsCertificate();
    if (!cert)
        return;

    cert->setPrivateKeyPath(path);
    (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::PRIVATE_KEY_FILE, cert?path:QString());
    (m_acc->d_ptr)->regenSecurityValidation();
}

///Set the certificate authority list file
void AccountTLS::setTlsCaListCertificate(Certificate* cert)
{
   //FIXME it can be a list of multiple certificates
   //this code currently only handle the case where is there is exactly one

   cert->setRequireStrictPermission(false);

   //All calls from the same top level CA are always accepted
   _account->allowCertificate(cert);

   (m_acc->d_ptr)->m_pCaCert = cert;
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::CA_LIST_FILE, cert?cert->path():QString());
   (m_acc->d_ptr)->regenSecurityValidation();

   if ((m_acc->d_ptr)->m_cTlsCaCert)
      disconnect((m_acc->d_ptr)->m_cTlsCaCert);

   if (cert) {
      (m_acc->d_ptr)->m_cTlsCaCert = connect(cert, &Certificate::changed,[this]() {
         (m_acc->d_ptr)->regenSecurityValidation();
      });
   }

}

///Set the certificate
void AccountTLS::setTlsCertificate(Certificate* cert)
{
   //The private key will be required for this certificate
   cert->setRequirePrivateKey(true);

   (m_acc->d_ptr)->m_pTlsCert = cert;
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE, cert?cert->path():QString());
   (m_acc->d_ptr)->regenSecurityValidation();
}

///Set the TLS server
void AccountTLS::setTlsServerName(const QString& detail)
{
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::SERVER_NAME, detail);
   (m_acc->d_ptr)->regenSecurityValidation();
}


///Set the TLS verification server
void AccountTLS::setTlsVerifyServer(bool detail)
{
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::VERIFY_SERVER, (detail)TO_BOOL);
   (m_acc->d_ptr)->regenSecurityValidation();
}

///Set the TLS verification client
void AccountTLS::setTlsVerifyClient(bool detail)
{
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::VERIFY_CLIENT, (detail)TO_BOOL);
   (m_acc->d_ptr)->regenSecurityValidation();
}

///Set if the peer need to be providing a certificate
void AccountTLS::setTlsRequireClientCertificate(bool detail)
{
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE ,(detail)TO_BOOL);
   (m_acc->d_ptr)->regenSecurityValidation();
}

///Set if the security settings are enabled
void AccountTLS::setTlsEnabled(bool detail)
{
   (m_acc->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::ENABLED ,(detail)TO_BOOL);
   (m_acc->d_ptr)->regenSecurityValidation();
}


///Destructor
AccountTLS::~AccountTLS()
{

}
