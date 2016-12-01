#include "tlsmethodmodel.h"




friend class TlsMethodModelPrivate;
friend class TlsMethodModel;


//Properties
Q_PROPERTY(Certificate*   tlsCaListCertificate         READ tlsCaListCertificate          WRITE setTlsCaListCertificate        )
Q_PROPERTY(Certificate*   tlsCertificate               READ tlsCertificate                WRITE setTlsCertificate              )
Q_PROPERTY(QString        tlsPrivateKey                READ tlsPrivateKey                 WRITE setTlsPrivateKey               )
Q_PROPERTY(QString        tlsServerName                READ tlsServerName                 WRITE setTlsServerName               )

private:
  Account* _account;

public:
  AccountTLS();
  AccountTLS(Account* account);
  //Getters
  TlsMethodModel*           tlsMethodModel             () const;
  QString tlsPassword                  () const;
  QString tlsPassword                  () const;
  int     bootstrapPort                () const;
  Certificate* tlsCaListCertificate    () const;
  Certificate* tlsCertificate          () const;
  QString tlsPrivateKey                () const;
  QString tlsServerName                () const;
  int     tlsNegotiationTimeoutSec     () const;
  bool    isTlsVerifyServer            () const;
  bool    isTlsVerifyClient            () const;
  bool    isTlsRequireClientCertificate() const;
  bool    isTlsEnabled                 () const;


  //Setters
  void setTlsPassword                   (const QString& detail  );
  void setTlsCaListCertificate          (Certificate* cert      );
  void setTlsCertificate                (Certificate* cert      );
  void setTlsCaListCertificate          (const QString& detail  );
  void setTlsCertificate                (const QString& detail  );
  void setTlsPrivateKey                 (const QString& path    );
  void setTlsServerName                 (const QString& detail  );

  void setTlsNegotiationTimeoutSec      (int  detail);
  void setLocalPort                     (unsigned short detail);
  void setBootstrapPort                 (unsigned short detail);
  void setPublishedPort                 (unsigned short detail);
  void setAutoAnswer                    (bool detail);
  void setTlsVerifyServer               (bool detail);
  void setTlsVerifyClient               (bool detail);
  void setTlsRequireClientCertificate   (bool detail);
  void setTlsEnabled                    (bool detail);
