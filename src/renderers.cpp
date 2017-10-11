/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
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
#include "renderers.h"

// Models and database
#include "api/lrc.h"
#include "newshmrenderer.h"

// legacy
#include <dbus/videomanager.h>

namespace lrc
{

using namespace api;

class RenderersPimpl: public QObject
{
    Q_OBJECT
public:
    RenderersPimpl(Renderers& linked);
    ~RenderersPimpl();

    std::map<std::string, std::shared_ptr<RENDERER_TYPE>> renderers_;
    bool isPreviewing_ = false;
    Renderers& linked;

public Q_SLOTS:
    void slotStoppedDecoding(const QString& id, const QString& shmPath);
    void slotStartedDecoding(const QString& id, const QString& shmPath, int width, int height);

};

RenderersPimpl::RenderersPimpl(Renderers& linked)
: linked(linked)
{
    connect( &VideoManager::instance() , &VideoManagerInterface::startedDecoding, this, &RenderersPimpl::slotStartedDecoding);
    connect( &VideoManager::instance() , &VideoManagerInterface::stoppedDecoding, this, &RenderersPimpl::slotStoppedDecoding);
}

RenderersPimpl::~RenderersPimpl()
{

}

Renderers::Renderers()
: QObject()
, pimpl_(std::make_unique<RenderersPimpl>(*this))
{
}

Renderers::~Renderers()
{
}

std::shared_ptr<RENDERER_TYPE>
Renderers::getRenderer(const std::string& callId)
{
    auto it = pimpl_->renderers_.find(callId);
    
    if (it != pimpl_->renderers_.end())
        return it->second;
    else
        throw std::out_of_range("No render for callId: " + callId);
}

std::shared_ptr<RENDERER_TYPE>
Renderers::getPreviewRenderer()
{
    return getRenderer("local");
}

void
RenderersPimpl::slotStoppedDecoding(const QString& rendererId, const QString& shmPath)
{
    if (rendererId == "local") {
        qDebug() << "emit for local video";
        emit linked.renderLocalStopped();
    } else {
        emit linked.renderRemoteStopped(rendererId.toStdString());
    }
    
    return;
}


// [jn] il faut déplacer le renderer dans un thread ?? rendererId -> almost equal to callId actually excepted local 
void
RenderersPimpl::slotStartedDecoding(const QString& rendererId, const QString& shmPath, int width, int height)
{
    //~ if (rendererId == "local") {
        //~ qDebug() << "  X@X ajouter la preview (local) pour l'instant on l'ignore";
        //~ return;
    //~ }

    auto renderer = std::make_shared<RENDERER_TYPE>(shmPath.toStdString(), width, height, true);
    renderer->startRendering();

    renderers_.emplace(std::make_pair(rendererId.toStdString(), renderer));

    if (rendererId == "local") {
        qDebug() << "emit for local video";
        emit linked.renderLocalStarted();
    } else {
        emit linked.renderRemoteStarted(rendererId.toStdString());
    }

    return;
}

bool
Renderers::hasRenderer(const std::string& rendererId)
{
    pimpl_->renderers_.count(rendererId) == 1;
}

} // namespace lrc

#include "renderers.moc"
