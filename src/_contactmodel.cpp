#include "_contactmodel.h"

ContactModel::ContactModel(QObject* parent)
: QObject(parent)
{
    
}

ContactModel::~ContactModel()
{
    
}

const ContactInfo&
ContactModel::addContact(const std::string& uri)
{
    return ContactInfo();
}

void
ContactModel::removeContact(const std::string& uri)
{
    
}

void
ContactModel::sendMessage(const std::string& uri, const std::string& body) const
{
    
}

const ContactInfo&
ContactModel::getContact(const std::string& uri/*TODO: add type*/) const
{
    return ContactInfo();
}

const ContactsInfo&
ContactModel::getContacts() const
{
    return ContactsInfo();
}

void
ContactModel::nameLookup(const std::string& uri) const
{
    
}

void
ContactModel::addressLookup(const std::string& name) const
{
    
}
