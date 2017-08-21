#include "_newcallmodel.h"


NewCallModel::NewCallModel(QObject* parent)
:QObject(parent)
{
    
}


NewCallModel::~NewCallModel()
{
    
}

const CallInfo&
NewCallModel::createCall()
{
    
}

void
NewCallModel::sendMessage(const std::string& callId, const std::string& body) const
{
    
}

void
NewCallModel::hangUp(const std::string& callId) const
{
    
}

void
NewCallModel::togglePause(const std::string& callId) const
{
    
}

void
NewCallModel::toggleMuteaUdio(const std::string& callId) const
{
    
}

void
NewCallModel::toggleMuteVideo(const std::string& callId) const
{
    
}

void
NewCallModel::toggleRecoringdAudio(const std::string& callId) const
{
    
}

void
NewCallModel::setQuality(const std::string& callId /*TODO: add parameter for quality*/) const
{
    
}

void
NewCallModel::transfer(const std::string& callId, const std::string& to) const
{
    
}

void
NewCallModel::addParticipant(const std::string& callId, const std::string& participant)
{
    
}

void
NewCallModel::removeParticipant(const std::string& callId, const std::string& participant)
{
    
}
