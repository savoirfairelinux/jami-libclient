#pragma once

#include <QtCore/QAbstractTableModel>

#include <typedefs.h>

class Account;
class DaemonContactModelPrivate;
class ContactMethod;
class ContactRequest;

class LIB_EXPORT DaemonContactModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    DaemonContactModel();

    enum Columns {
        PEER_ID,
        COUNT__
    };

    // Model functions
    virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
    virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
    virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;

    // Helper
    void add(ContactMethod* cm);
    void remove(ContactMethod* cm);
    QVector<ContactMethod*>& getContacts() const;

Q_SIGNALS:
   void daemonContactRemoved(const ContactMethod* c);
   void newDaemonContactAdded(const ContactMethod* c);

private:
    virtual ~DaemonContactModel();

    DaemonContactModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(DaemonContactModel)

};
