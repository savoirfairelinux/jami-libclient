#ifndef SECURITY_FLAW_H
#define SECURITY_FLAW_H

#include <QtCore/QObject>

#include <typedefs.h>

#include <securityvalidationmodel.h>

class SecurityFlawPrivate;

///A flaw representation
class LIB_EXPORT SecurityFlaw : public QObject
{
   Q_OBJECT
   friend class SecurityValidationModel;
   friend class SecurityValidationModelPrivate;
public:

   //Operators
   bool operator < ( const SecurityFlaw &r ) const;
   bool operator > ( const SecurityFlaw &r ) const;

   //Getter
   Certificate::Type type() const;
   SecurityValidationModel::AccountSecurityFlaw flaw() const;
   SecurityValidationModel::Severity severity() const;

private:
   explicit SecurityFlaw(SecurityValidationModel::AccountSecurityFlaw f,Certificate::Type type = Certificate::Type::NONE);

   SecurityFlawPrivate* d_ptr;
   Q_DECLARE_PRIVATE(SecurityFlaw)

public Q_SLOTS:
   void requestHighlight();

Q_SIGNALS:
   void solved();
   void highlight();
};

#endif