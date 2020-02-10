/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                                  *
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
#pragma once

// Std
#include <memory>
#include <string>
#include <list>

// Qt
#include <qobject.h>
#include <QObject>

// Lrc
#include "api/account.h"
#include "typedefs.h"

namespace lrc
{

class CallbacksHandler;
class NewCodecModelPimpl;

namespace api
{

namespace account { struct Info; }

struct Codec
{
    unsigned int id;
    bool enabled;
    std::string name;
    std::string samplerate;
    std::string bitrate;
    std::string min_bitrate;
    std::string max_bitrate;
    std::string type;
    std::string quality;
    std::string min_quality;
    std::string max_quality;
    bool auto_quality_enabled;
};

/**
  *  @brief Class that manages ring devices for an account
  */
class LIB_EXPORT NewCodecModel : public QObject {
    Q_OBJECT

public:
    const account::Info& owner;

    NewCodecModel(const account::Info& owner, const CallbacksHandler& callbacksHandler);
    ~NewCodecModel();

    /**
     * @return audio codecs for the account
     */
    std::list<Codec> getAudioCodecs() const;
    /**
     * @return video codecs for the account
     */
    std::list<Codec> getVideoCodecs() const;
    /**
     * Set a higher priority to a codec
     * @param codecId
     * @param isVideo
     */
    void increasePriority(const unsigned int& codecid, bool isVideo);
    /**
     * Set a lower priority to a codec
     * @param codecId
     * @param isVideo
     */
    void decreasePriority(const unsigned int& codecid, bool isVideo);
    /**
     * Enable a codec
     * @param codecId
     * @param enabled true if enabled else false
     * @return if codecId is the only codec impacted
     */
    bool enable(const unsigned int& codecid, bool enabled);
    /**
     * Enable/Disable auto quality for this codec
     * @param codecId
     * @param on true if enabled else false
     * @return
     */
    void autoQuality(const unsigned int& codecid, bool on);
    /**
     * Change wanted quality
     * @param codecId
     * @param quality
     * @return
     */
    void quality(const unsigned int& codecid, double quality);
    /**
     * Change wanted bitrate
     * @param codecId
     * @param bitrate
     * @return
     */
    void bitrate(const unsigned int& codecid, double bitrate);

Q_SIGNALS:
    /**
     * Emit codecListChanged
     * @param accountId
     * @param activated_codecs
     */
    void activeCodecListChanged(const std::string& accountId, std::vector<unsigned> activated_codec);

private:
    std::unique_ptr<NewCodecModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
