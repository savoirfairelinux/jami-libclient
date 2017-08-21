#include "conversationmodel.h"

ConversationModel::ConversationModel(QObject* parent)
:QObject(parent)
{

}

ConversationModel::~ConversationModel()
{

}

const Conversations&
ConversationModel::getConversations() const
{
    return conversations_;
}

const Conversation&
ConversationModel::getConversation(const unsigned int row) const
{
    return Conversation();
}

const Conversation&
ConversationModel::addConversation(const std::string& uri)
{
    return Conversation();
}

void
ConversationModel::selectConversation(const std::string& uid)
{

}

void
ConversationModel::removeConversation(const std::string& uid)
{

}

void
ConversationModel::placeCall(const std::string& uid) const
{

}

void
ConversationModel::sendMessage(const std::string& uid, const std::string& body) const
{

}

void
ConversationModel::setFilter(const std::string&)
{

}

void
ConversationModel::addParticipant(const std::string& uid, const::std::string& uri)
{

}

void
ConversationModel::cleanHistory(const std::string& uid)
{

}
