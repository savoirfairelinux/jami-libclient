/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#include "api/newcodecmodel.h"

// std
#include <list>
#include <mutex>

// LRC
#include "callbackshandler.h"
#include "dbus/configurationmanager.h"

// Daemon
#include <account_const.h>

// Qt
#include <QObject>

namespace lrc
{


using namespace api;

class NewCodecModelPimpl: public QObject
{
    Q_OBJECT
public:
    NewCodecModelPimpl(const NewCodecModel& linked, const CallbacksHandler& callbacksHandler);
    ~NewCodecModelPimpl();

    std::list<Codec> codecs;

    const CallbacksHandler& callbacksHandler;
    const NewCodecModel& linked;

    void setActiveCodecs();
};

NewCodecModel::NewCodecModel(const account::Info& owner, const CallbacksHandler& callbacksHandler)
: owner(owner)
, pimpl_(std::make_unique<NewCodecModelPimpl>(*this, callbacksHandler))
{ }

NewCodecModel::~NewCodecModel() {}

std::list<Codec>
NewCodecModel::getAudioCodecs() const
{
    std::list<Codec> audioCodecs;
    for (auto codec : pimpl_->codecs) {
        if (codec.type == "AUDIO") {
            audioCodecs.emplace_back(codec);
        }
    }
    return audioCodecs;
}

std::list<Codec>
NewCodecModel::getVideoCodecs() const
{
    std::list<Codec> videoCodecs;
    for (auto codec : pimpl_->codecs) {
        if (codec.type == "VIDEO") {
            videoCodecs.emplace_back(codec);
        }
    }
    return videoCodecs;
}

void
NewCodecModel::increasePriority(const unsigned int& codecId)
{
    auto it = pimpl_->codecs.begin();
    while (it != pimpl_->codecs.end()) {
        if (it->id == codecId) {
            if (it == pimpl_->codecs.begin()) {
                // Already at top, abort
                return;
            }
            std::swap(*it, *it--);
            break;
        }
        it++;
    }
    pimpl_->setActiveCodecs();
}

void
NewCodecModel::decreasePriority(const unsigned int& codecId)
{
    auto it = pimpl_->codecs.begin();
    while (it != pimpl_->codecs.end()) {
        if (it->id == codecId) {
            if (it == pimpl_->codecs.end()) {
                // Already at bottom, abort
                return;
            }
            std::swap(*it, *it++);
            break;
        }
        it++;
    }
    pimpl_->setActiveCodecs();
}

void
NewCodecModel::enable(const unsigned int& codecId, bool enabled)
{
    for (auto& codec : pimpl_->codecs) {
        if (codec.id == codecId) {
            if (codec.enabled == enabled) return;
            codec.enabled = enabled;
        }
    }
    pimpl_->setActiveCodecs();
}

NewCodecModelPimpl::NewCodecModelPimpl(const NewCodecModel& linked, const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
{
    QVector<unsigned int> codecsList = ConfigurationManager::instance().getCodecList();
    QVector<unsigned int> activeCodecs = ConfigurationManager::instance().getActiveCodecList(linked.owner.id.c_str());
    for (const auto& id : codecsList) {
        MapStringString details = ConfigurationManager::instance().getCodecDetails(linked.owner.id.c_str(), id);
        Codec codec;
        codec.id = id;
        codec.enabled = activeCodecs.indexOf(id) != -1;
        codec.name = details[DRing::Account::ConfProperties::CodecInfo::NAME].toStdString();
        codec.samplerate = details[DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE].toStdString();
        codec.bitrate = details[DRing::Account::ConfProperties::CodecInfo::BITRATE].toStdString();
        codec.min_bitrate = details[DRing::Account::ConfProperties::CodecInfo::MIN_BITRATE].toStdString();
        codec.max_bitrate = details[DRing::Account::ConfProperties::CodecInfo::MAX_BITRATE].toStdString();
        codec.type = details[DRing::Account::ConfProperties::CodecInfo::TYPE].toStdString();
        codec.quality = details[DRing::Account::ConfProperties::CodecInfo::QUALITY].toStdString();
        codec.min_quality = details[DRing::Account::ConfProperties::CodecInfo::MIN_QUALITY].toStdString();
        codec.max_quality = details[DRing::Account::ConfProperties::CodecInfo::MAX_QUALITY].toStdString();
        codec.auto_quality_enabled = details[DRing::Account::ConfProperties::CodecInfo::AUTO_QUALITY_ENABLED].toStdString() == "true";
        codecs.emplace_back(codec);
    }
}

NewCodecModelPimpl::~NewCodecModelPimpl()
{

}

void
NewCodecModelPimpl::setActiveCodecs()
{
    QVector<unsigned int> enabledCodecs;
    for (auto& codec : pimpl_->codecs) {
        if (codec.enabled) {
            enabledCodecs.push_back(codec.id);
        }
    }
    ConfigurationManager::instance().setActiveCodecList(owner.id.c_str(), enabledCodecs);
}

} // namespace lrc

#include "newcodecmodel.moc"
#include "api/moc_newcodecmodel.cpp"
