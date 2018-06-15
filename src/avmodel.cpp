/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
 *   Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>             *
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
#include "api/avmodel.h"

// Std
#include <chrono>
#include <iomanip> // for std::put_time
#include <fstream>
#include <string>
#include <sstream>

// Qt
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

// LRC
#include "dbus/videomanager.h"

namespace lrc
{

using namespace api;

class AVModelPimpl: public QObject
{
    Q_OBJECT
public:
    AVModelPimpl(AVModel& linked);
    std::string getRecordingPath() const;
    static const std::string recorderSavesSubdir;
    AVModel& linked_;
};

const std::string AVModelPimpl::recorderSavesSubdir = "sent_data";

AVModel::AVModel()
: QObject()
, pimpl_(std::make_unique<AVModelPimpl>(*this))
{
}

AVModel::~AVModel()
{
}

AVModelPimpl::AVModelPimpl(AVModel& linked)
: linked_(linked)
{
    std::srand(std::time(nullptr));
}

std::string
AVModelPimpl::getRecordingPath() const
{
    const QDir dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + recorderSavesSubdir.c_str();
    dir.mkpath(".");

    std::chrono::time_point<std::chrono::system_clock> time_now = std::chrono::system_clock::now();
    std::time_t time_now_t = std::chrono::system_clock::to_time_t(time_now);
    std::tm now_tm = *std::localtime(&time_now_t);

    std::stringstream ss;
    ss << dir.path().toStdString();
    ss << "/";
    ss << std::put_time(&now_tm, "%Y%m%d-%H%M%S");
    ss << "-";
    ss << std::rand();

    QDir file_path(ss.str().c_str());

    return file_path.path().toStdString();
}

void AVModel::stopLocalRecorder(const std::string& path) const
{
   if (path.empty()) {
      qDebug("stopLocalRecorder: can't stop non existing recording");
      return;
   }

   VideoManager::instance().stopLocalRecorder(QString::fromStdString(path));
}

std::string AVModel::startLocalRecorder(const bool& audioOnly) const
{
   const QString path = QString::fromStdString(pimpl_->getRecordingPath());
   const QString finalPath = VideoManager::instance().startLocalRecorder(audioOnly, path);
   return finalPath.toStdString();
}
} // namespace lrc

#include "api/moc_avmodel.cpp"
#include "avmodel.moc"
