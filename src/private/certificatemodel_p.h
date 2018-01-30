/****************************************************************************
 *   Copyright (C) 2015-2018 Savoir-faire Linux                               *
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
#pragma once

#include "matrixutils.h"
#include "certificate.h"

#include <QtCore/QMutex>
#include <QtCore/QObject>

struct CertificateNode;
class Account;
class CertificateModel;
class DaemonCertificateCollection;
class QAbstractItemModel;

class CertificateModelPrivate final : public QObject
{
   Q_OBJECT
public:
   CertificateModelPrivate(CertificateModel* parent);
   virtual ~CertificateModelPrivate();

   //Helper
   CertificateNode* defaultCategory();
   CertificateNode* createCategory(const QString& name, const QString& col2, const QString& tooltip);
   CertificateNode* getCategory(const Account* a);
   CertificateNode* addToTree(Certificate* cert, CertificateNode* category = nullptr);
   CertificateNode* addToTree(Certificate* cert, Account* a);
   void             removeFromTree(CertificateNode* node);
   void             removeFromTree(Certificate* c, CertificateNode* category);
   QModelIndex      createIndex(int r ,int c , void* p);
   QAbstractItemModel* getModelCommon(CertificateNode* node);
   bool allowCertificate(Certificate* c, Account* a);
   bool banCertificate(Certificate* c, Account* a);
   void loadChecks(CertificateNode* checks, Certificate* cert);
   void regenChecks(Certificate* cert);
   bool isPartOf(CertificateNode* sibling, CertificateNode* list);

   //Attributes
   QVector<CertificateNode*>        m_lTopLevelNodes    ;
   QHash<QString,Certificate*>      m_hCertificates     ;
   CertificateNode*                 m_pDefaultCategory  ;
   QMutex                           m_CertLoader        ;
   QMutex                           m_CertInsertion     ;
   int                              m_GroupCounter      ;
   QHash<const Account*,CertificateNode*> m_hAccToCat   ;
   QHash<const QString&,CertificateNode*> m_hStrToCat   ;
   QHash<const Certificate*,CertificateNode*> m_hNodes  ;
   static const Matrix1D<Certificate::Status, const char*> m_StatusMap;
   mutable QHash<const Account*,QAbstractItemModel*> m_hAccAllow;
   mutable QHash<const Account*,QAbstractItemModel*> m_hAccBan  ;
   mutable QHash<const Account*,CertificateNode*> m_hAccAllowCat;
   mutable QHash<const Account*,CertificateNode*> m_hAccBanCat  ;

   //Getters
   QAbstractItemModel* model             (const Certificate* cert) const;
   QAbstractItemModel* checksModel       (const Certificate* cert) const;
   QAbstractItemModel* createKnownList   (const Account* a       ) const;
   QAbstractItemModel* createBannedList  (const Account* a       ) const;
   QAbstractItemModel* createAllowedList (const Account* a       ) const;

private:
   CertificateModel* q_ptr;

private Q_SLOTS:
   void slotCertificateStateChanged(const QString& accountId, const QString& certId, const QString& state);
};
