#include "daemoncontactmodel.h"

// LRC
#include "account.h"
#include "contactmethod.h"
#include "dbus/configurationmanager.h"
#include "phonedirectorymodel.h"
#include "private/sortproxies.h"
#include "availableaccountmodel.h"

#include <QtCore/QSortFilterProxyModel>
#include <iostream>


class DaemonContactModelPrivate : public QObject
{
    Q_OBJECT
public:
    //Constructor
    explicit DaemonContactModelPrivate(DaemonContactModel* parent);

    //Attributes
    QVector<ContactMethod*> m_lContacts;
    Account* m_pAccount;


private:
    DaemonContactModel* q_ptr;
};

/**
 * constructor of DaemonContactModelPrivate.
 */
DaemonContactModelPrivate::DaemonContactModelPrivate(DaemonContactModel* p) : QObject(), q_ptr(p)
{
//    connect(AvailableAccountModel::instance().selectionModel(),
//      &QItemSelectionModel::currentChanged, this, &DaemonContactModelPrivate::reload);
}


/**
 * constructor of DaemonContactModel.
 */
DaemonContactModel::DaemonContactModel(Account* a) : QAbstractTableModel(a),
d_ptr(new DaemonContactModelPrivate(this))
{
    std::cout << "hello ±±±±±±±±±±±±±±±±±±±±±±±±±±±11" << std::endl;
    d_ptr->m_pAccount = a;

    // Load the contacts associated from the daemon and create the cms.
    const auto account_contacts
        = static_cast<QVector<QMap<QString, QString>>>(ConfigurationManager::instance().getContacts(a->id().data()));

    if (a->protocol() == Account::Protocol::RING) {
        for (auto contact_info : account_contacts) {
            if (contact_info["banned"] == "false") {
                auto cm = PhoneDirectoryModel::instance().getNumber(contact_info["id"], a);
                add(cm);
            }
        }
    }
}

/**
 * destructor of DaemonContactModel.
 */
DaemonContactModel::~DaemonContactModel()
{
    delete d_ptr;
}

QVector<ContactMethod*>&
DaemonContactModel::getContacts() const
{
    return d_ptr->m_lContacts;
}

/**
 * QAbstractTableModel function used to return the data.
 */
QVariant
DaemonContactModel::data( const QModelIndex& index, int role ) const
{
    std::cout << "###DisplayRole " << role << std::endl;
    if (!index.isValid())
        return QVariant();

    switch(index.column()) {
    case Columns::PEER_ID:
        return d_ptr->m_lContacts[index.row()]->roleData(role);
    break;
    case Columns::COUNT__:
        switch(role) {
        case Qt::DisplayRole:
            return static_cast<int>(DaemonContactModel::Columns::COUNT__);
        }
    break;
    }

   return QVariant();
}

/**
 * return the number of rows from the model.
 */
int
DaemonContactModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid()? 0 : d_ptr->m_lContacts.size();
}

/**
 * return the number of columns from the model.
 */
int
DaemonContactModel::columnCount( const QModelIndex& parent ) const
{
    return parent.isValid()? 0 : static_cast<int>(DaemonContactModel::Columns::COUNT__);
}

/**
 * this function add a ContactMethod to the contact list.
 * @param cm, the ContactMethod to add to the list.
 */
void
DaemonContactModel::add(ContactMethod* cm)
{
    std::cout << "###add contact: " << cm->roleData(static_cast<int>(Ring::Role::Number)).toString().toUtf8().constData() << std::endl;
    if (d_ptr->m_lContacts.contains(cm))
        return;
    beginInsertRows(QModelIndex(),d_ptr->m_lContacts.size(),d_ptr->m_lContacts.size());
    d_ptr->m_lContacts << cm;
    endInsertRows();
    emit newDaemonContactAdded(cm);
}

/**
 * this function removes a ContactMethod from the contact list.
 * @param cm, the ContactMethod to remove from the list.
 */
void
DaemonContactModel::remove(ContactMethod* cm)
{
    std::cout << "###rm contact: " << cm->roleData(static_cast<int>(Ring::Role::Number)).toString().toUtf8().constData() << std::endl;
    auto rowIndex = d_ptr->m_lContacts.indexOf(cm);
    if (rowIndex < 0)
        return;

    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    d_ptr->m_lContacts.removeAt(rowIndex);
    endRemoveRows();

    if (!cm->account()) {
        qWarning() << "DaemonContactModel, cannot remove. cm->account is nullptr";
        return;
    }

    emit daemonContactRemoved(cm);
}

#include <daemoncontactmodel.moc>
