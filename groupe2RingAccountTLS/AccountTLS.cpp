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


AccountTLS::AccountTLS(){


}
AccountTLS::AccountTLS(Account* account){
   _account = account;
}

TlsMethodModel* Account::tlsMethodModel() const
{
   if (!_account->(_account->d_ptr)->m_pTlsMethodModel ) {
      (_account->d_ptr)->m_pTlsMethodModel  = new TlsMethodModel(const_cast<Account*>(this));
   }
   return (_account->d_ptr)->m_pTlsMethodModel;
}


///Return the account tls password
QString Account::tlsPassword() const
{
   return (_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::PASSWORD);
}

///Return the account TLS port
int Account::bootstrapPort() const
{
   return (_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::DHT::PORT).toInt();
}

///Return the account TLS certificate authority list file
Certificate* Account::tlsCaListCertificate() const
{
   if (!(_account->d_ptr)->m_pCaCert) {
      const QString& path = (_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::CA_LIST_FILE);
      if (path.isEmpty())
         return nullptr;
      (_account->d_ptr)->m_pCaCert = CertificateModel::instance().getCertificateFromPath(path,Certificate::Type::AUTHORITY);
      connect((_account->d_ptr)->m_pCaCert,SIGNAL(changed()),(_account->d_ptr).data(),SLOT(slotUpdateCertificate()));
   }
   return (_account->d_ptr)->m_pCaCert;
}

///Return the account TLS certificate
Certificate* Account::tlsCertificate() const
{
   if (!(_account->d_ptr)->m_pTlsCert) {
      const QString& path = (_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE);
      if (path.isEmpty())
         return nullptr;
      (_account->d_ptr)->m_pTlsCert = CertificateModel::instance().getCertificateFromPath(path,Certificate::Type::USER);
      connect((_account->d_ptr)->m_pTlsCert,SIGNAL(changed()),(_account->d_ptr).data(),SLOT(slotUpdateCertificate()));
   }
   return (_account->d_ptr)->m_pTlsCert;
}

///Return the account private key
QString Account::tlsPrivateKey() const
{
   return tlsCertificate() ? tlsCertificate()->privateKeyPath() : QString();
}

///Return the account TLS server name
QString Account::tlsServerName() const
{
   return (_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::SERVER_NAME);
}

///Return the account negotiation timeout in seconds
int Account::tlsNegotiationTimeoutSec() const
{
   return (_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::NEGOTIATION_TIMEOUT_SEC).toInt();
}

///Return the account TLS verify server
bool Account::isTlsVerifyServer() const
{
   return ((_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::VERIFY_SERVER) IS_TRUE);
}

///Return the account TLS verify client
bool Account::isTlsVerifyClient() const
{
   return ((_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::VERIFY_CLIENT) IS_TRUE);
}

///Return if it is required for the peer to have a certificate
bool Account::isTlsRequireClientCertificate() const
{
   return ((_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE) IS_TRUE);
}

///Return the account TLS security is enabled
bool Account::isTlsEnabled() const
{
   return _account->protocol() == Account::Protocol::RING || ((_account->d_ptr)->accountDetail(DRing::Account::ConfProperties::TLS::ENABLED) IS_TRUE);
}


///Set the TLS (encryption) password
void Account::setTlsPassword(const QString& detail)
{
   auto cert = tlsCertificate();
   if (!cert)
      return;
   cert->setPrivateKeyPassword(detail);
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::PASSWORD, detail);
   (_account->d_ptr)->regenSecurityValidation();
}

///Set the certificate authority list file
void Account::setTlsCaListCertificate(const QString& path)
{
   Certificate* cert = CertificateModel::instance().getCertificateFromPath(path);
   setTlsCaListCertificate(cert);
}

///Set the certificate
void Account::setTlsCertificate(const QString& path)
{
   Certificate* cert = CertificateModel::instance().getCertificateFromPath(path);
   setTlsCertificate(cert);
}

///Set the private key
void Account::setTlsPrivateKey(const QString& path)
{
    auto cert = tlsCertificate();
    if (!cert)
        return;

    cert->setPrivateKeyPath(path);
    (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::PRIVATE_KEY_FILE, cert?path:QString());
    (_account->d_ptr)->regenSecurityValidation();
}

///Set the certificate authority list file
void Account::setTlsCaListCertificate(Certificate* cert)
{
   //FIXME it can be a list of multiple certificates
   //this code currently only handle the case where is there is exactly one

   cert->setRequireStrictPermission(false);

   //All calls from the same top level CA are always accepted
   _account->allowCertificate(cert);

   (_account->d_ptr)->m_pCaCert = cert;
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::CA_LIST_FILE, cert?cert->path():QString());
   (_account->d_ptr)->regenSecurityValidation();

   if ((_account->d_ptr)->m_cTlsCaCert)
      disconnect((_account->d_ptr)->m_cTlsCaCert);

   if (cert) {
      (_account->d_ptr)->m_cTlsCaCert = connect(cert, &Certificate::changed,[this]() {
         (_account->d_ptr)->regenSecurityValidation();
      });
   }

}

///Set the certificate
void Account::setTlsCertificate(Certificate* cert)
{
   //The private key will be required for this certificate
   cert->setRequirePrivateKey(true);

   (_account->d_ptr)->m_pTlsCert = cert;
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::CERTIFICATE_FILE, cert?cert->path():QString());
   (_account->d_ptr)->regenSecurityValidation();
}

///Set the TLS server
void Account::setTlsServerName(const QString& detail)
{
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::SERVER_NAME, detail);
   (_account->d_ptr)->regenSecurityValidation();
}


///Set the TLS verification server
void Account::setTlsVerifyServer(bool detail)
{
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::VERIFY_SERVER, (detail)TO_BOOL);
   (_account->d_ptr)->regenSecurityValidation();
}

///Set the TLS verification client
void Account::setTlsVerifyClient(bool detail)
{
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::VERIFY_CLIENT, (detail)TO_BOOL);
   (_account->d_ptr)->regenSecurityValidation();
}

///Set if the peer need to be providing a certificate
void Account::setTlsRequireClientCertificate(bool detail)
{
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::REQUIRE_CLIENT_CERTIFICATE ,(detail)TO_BOOL);
   (_account->d_ptr)->regenSecurityValidation();
}

///Set if the security settings are enabled
void Account::setTlsEnabled(bool detail)
{
   (_account->d_ptr)->setAccountProperty(DRing::Account::ConfProperties::TLS::ENABLED ,(detail)TO_BOOL);
   (_account->d_ptr)->regenSecurityValidation();
}



///Destructor
AccountTLS::~AccountTLS()
{

}
