/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                             *
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
#include "daemonproxy.h"

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

    std::list<Codec> videoCodecs;
    std::mutex audioCodecsMtx;
    std::list<Codec> audioCodecs;
    std::mutex videoCodecsMtx;

    const CallbacksHandler& callbacksHandler;
    const NewCodecModel& linked;

    void setActiveCodecs();
    void setCodecDetails(const Codec& codec, bool isAudio);

private:
    void addCodec(const unsigned int& id, const std::vector<unsigned int>& activeCodecs);
};

NewCodecModel::NewCodecModel(const account::Info& owner, const CallbacksHandler& callbacksHandler)
: owner(owner)
, pimpl_(std::make_unique<NewCodecModelPimpl>(*this, callbacksHandler))
{ }

NewCodecModel::~NewCodecModel() {}

std::list<Codec>
NewCodecModel::getAudioCodecs() const
{
    return pimpl_->audioCodecs;
}

std::list<Codec>
NewCodecModel::getVideoCodecs() const
{
    return pimpl_->videoCodecs;
}

void
NewCodecModel::increasePriority(const unsigned int& codecId, bool isVideo)
{
    auto& codecs = isVideo? pimpl_->videoCodecs : pimpl_->audioCodecs;
    auto& mutex = isVideo? pimpl_->videoCodecsMtx : pimpl_->audioCodecsMtx;
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto it = codecs.begin();
        if (codecs.begin()->id == codecId) {
            // Already at top, abort
            return;
        }
        while (it != codecs.end()) {
            if (it->id == codecId) {
                std::iter_swap(it, std::prev(it));
                break;
            }
            it++;
        }
    }
    pimpl_->setActiveCodecs();
}

void
NewCodecModel::decreasePriority(const unsigned int& codecId, bool isVideo)
{
    auto& codecs = isVideo? pimpl_->videoCodecs : pimpl_->audioCodecs;
    auto& mutex = isVideo? pimpl_->videoCodecsMtx : pimpl_->audioCodecsMtx;
    {
        std::unique_lock<std::mutex> lock(mutex);
        auto it = codecs.begin();
        if (codecs.rbegin()->id == codecId) {
            // Already at bottom, abort
            return;
        }
        while (it != codecs.end()) {
            if (it->id == codecId) {
                std::iter_swap(it, std::next(it));
                break;
            }
            it++;
        }
    }
    pimpl_->setActiveCodecs();
}

bool
NewCodecModel::enable(const unsigned int& codecId, bool enabled)
{
    auto redraw = false;
    auto isAudio = true;
    {
        std::unique_lock<std::mutex> lock(pimpl_->videoCodecsMtx);
        auto allDisabled = true;
        for (auto& codec : pimpl_->videoCodecs) {
            if (codec.id == codecId) {
                if (codec.enabled == enabled) return redraw;
                codec.enabled = enabled;
                isAudio = false;
            }
            if (codec.enabled) {
                allDisabled = false;
            }
        }
        if (allDisabled) {
            redraw = true;
            // Disabling all codecs is not possible and the daemon set enabled all codecs here
            // So, set all codecs enabled
            for (auto& codec : pimpl_->videoCodecs) {
                codec.enabled = true;
            }
        }
    }
    if (isAudio) {
        std::unique_lock<std::mutex> lock(pimpl_->audioCodecsMtx);
        auto allDisabled = true;
        for (auto& codec : pimpl_->audioCodecs) {
            if (codec.id == codecId) {
                if (codec.enabled == enabled) return redraw;
                codec.enabled = enabled;
            }
            if (codec.enabled) {
                allDisabled = false;
            }
        }
        if (allDisabled) {
            redraw = true;
            // Disabling all codecs is not possible and the daemon set enabled all codecs here
            // So, set all codecs enabled
            for (auto& codec : pimpl_->audioCodecs) {
                codec.enabled = true;
            }
        }
    }
    pimpl_->setActiveCodecs();
    return redraw;
}

void
NewCodecModel::autoQuality(const unsigned int& codecId, bool on)
{
    auto isAudio = true;
    Codec finalCodec;
    {
        std::unique_lock<std::mutex> lock(pimpl_->videoCodecsMtx);
        for (auto& codec : pimpl_->videoCodecs) {
            if (codec.id == codecId) {
                if (codec.auto_quality_enabled == on) return;
                codec.auto_quality_enabled = on;
                isAudio = false;
                finalCodec = codec;
                break;
            }
        }
    }
    if (isAudio) {
        std::unique_lock<std::mutex> lock(pimpl_->audioCodecsMtx);
        for (auto& codec : pimpl_->audioCodecs) {
            if (codec.id == codecId) {
                if (codec.auto_quality_enabled == on) return;
                codec.auto_quality_enabled = on;
                finalCodec = codec;
                break;
            }
        }
    }
    pimpl_->setCodecDetails(finalCodec, isAudio);
}

void
NewCodecModel::quality(const unsigned int& codecId, double quality)
{
    auto isAudio = true;
    auto qualityStr = std::to_string(static_cast<int>(quality));
    Codec finalCodec;
    {
        std::unique_lock<std::mutex> lock(pimpl_->videoCodecsMtx);
        for (auto& codec : pimpl_->videoCodecs) {
            if (codec.id == codecId) {
                if (codec.quality == qualityStr) return;
                codec.quality = qualityStr;
                isAudio = false;
                finalCodec = codec;
                break;
            }
        }
    }
    if (isAudio) {
        std::unique_lock<std::mutex> lock(pimpl_->audioCodecsMtx);
        for (auto& codec : pimpl_->audioCodecs) {
            if (codec.id == codecId) {
                if (codec.quality == qualityStr) return;
                codec.quality = qualityStr;
                finalCodec = codec;
                break;
            }
        }
    }
    pimpl_->setCodecDetails(finalCodec, isAudio);
}

void
NewCodecModel::bitrate(const unsigned int& codecId, double bitrate)
{
    auto isAudio = true;
    auto bitrateStr = std::to_string(static_cast<int>(bitrate));
    Codec finalCodec;
    {
        std::unique_lock<std::mutex> lock(pimpl_->videoCodecsMtx);
        for (auto& codec : pimpl_->videoCodecs) {
            if (codec.id == codecId) {
                if (codec.bitrate == bitrateStr) return;
                codec.bitrate = bitrateStr;
                isAudio = false;
                finalCodec = codec;
                break;
            }
        }
    }
    if (isAudio) {
        std::unique_lock<std::mutex> lock(pimpl_->audioCodecsMtx);
        for (auto& codec : pimpl_->audioCodecs) {
            if (codec.id == codecId) {
                if (codec.bitrate == bitrateStr) return;
                codec.bitrate = bitrateStr;
                finalCodec = codec;
                break;
            }
        }
    }
    pimpl_->setCodecDetails(finalCodec, isAudio);
}

NewCodecModelPimpl::NewCodecModelPimpl(const NewCodecModel& linked, const CallbacksHandler& callbacksHandler)
: linked(linked)
, callbacksHandler(callbacksHandler)
{
    std::vector<unsigned int> codecsList = DaemonProxy::instance().getCodecList();
    std::vector<unsigned int> activeCodecs = DaemonProxy::instance().getActiveCodecList(linked.owner.id);
    for (const auto& id : activeCodecs) {
        addCodec(id, activeCodecs);
    }
    for (const auto& id : codecsList) {
        bool found = false;
        for (const auto& i: activeCodecs)
            if (id == i) {
                found = true;
                break;
            }
        if (!found)
            addCodec(id, activeCodecs);
    }
}

NewCodecModelPimpl::~NewCodecModelPimpl()
{

}

void
NewCodecModelPimpl::setActiveCodecs()
{
    std::vector<unsigned int> enabledCodecs;
    {
        std::unique_lock<std::mutex> lock(videoCodecsMtx);
        for (auto& codec : videoCodecs) {
            if (codec.enabled) {
                enabledCodecs.push_back(codec.id);
            }
        }
    }
    {
        std::unique_lock<std::mutex> lock(audioCodecsMtx);
        for (auto& codec : audioCodecs) {
            if (codec.enabled) {
                enabledCodecs.push_back(codec.id);
            }
        }
    }
    DaemonProxy::instance().setActiveCodecList(linked.owner.id, enabledCodecs);
}

void
NewCodecModelPimpl::addCodec(const unsigned int& id, const std::vector<unsigned int>& activeCodecs)
{
    std::map<std::string, std::string> details = DaemonProxy::instance().getCodecDetails(linked.owner.id, id);
    Codec codec;
    codec.id = id;
    codec.enabled = false;
    for (auto i: activeCodecs) {
        if (i == id) { codec.enabled = true; break; }
    }
    codec.name = details[DRing::Account::ConfProperties::CodecInfo::NAME];
    codec.samplerate = details[DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE];
    codec.bitrate = details[DRing::Account::ConfProperties::CodecInfo::BITRATE];
    codec.min_bitrate = details[DRing::Account::ConfProperties::CodecInfo::MIN_BITRATE];
    codec.max_bitrate = details[DRing::Account::ConfProperties::CodecInfo::MAX_BITRATE];
    codec.type = details[DRing::Account::ConfProperties::CodecInfo::TYPE];
    codec.quality = details[DRing::Account::ConfProperties::CodecInfo::QUALITY];
    codec.min_quality = details[DRing::Account::ConfProperties::CodecInfo::MIN_QUALITY];
    codec.max_quality = details[DRing::Account::ConfProperties::CodecInfo::MAX_QUALITY];
    codec.auto_quality_enabled = details[DRing::Account::ConfProperties::CodecInfo::AUTO_QUALITY_ENABLED] == "true";
    if (codec.type == "AUDIO") {
        std::unique_lock<std::mutex> lock(audioCodecsMtx);
        audioCodecs.emplace_back(codec);
    } else {
        std::unique_lock<std::mutex> lock(videoCodecsMtx);
        videoCodecs.emplace_back(codec);
    }
}

void
NewCodecModelPimpl::setCodecDetails(const Codec& codec, bool isAudio)
{
    std::map<std::string, std::string> details;
    details[ DRing::Account::ConfProperties::CodecInfo::NAME        ] = codec.name.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE ] = codec.samplerate.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::BITRATE     ] = codec.bitrate.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::MIN_BITRATE ] = codec.min_bitrate.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::MAX_BITRATE ] = codec.max_bitrate.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::TYPE        ] = isAudio? "AUDIO" : "VIDEO";
    details[ DRing::Account::ConfProperties::CodecInfo::QUALITY     ] = codec.quality.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::MIN_QUALITY ] = codec.min_quality.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::MAX_QUALITY ] = codec.max_quality.c_str();
    details[ DRing::Account::ConfProperties::CodecInfo::AUTO_QUALITY_ENABLED] = codec.auto_quality_enabled? "true" : "false";
    DaemonProxy::instance().setCodecDetails(linked.owner.id, codec.id, details);
}

} // namespace lrc

#include "newcodecmodel.moc"
//#include "api/moc_newcodecmodel.cpp"
