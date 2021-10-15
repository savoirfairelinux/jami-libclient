/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "api/avmodel.h"
#include "api/lrc.h"

#include <QImage>
#include <QMutex>
#include <QObject>

using namespace lrc::api;

/*
 * This class acts as a QImage rendering sink and manages
 * signal/slot connections to it's underlying (AVModel) renderer
 * corresponding to the object's renderer id.
 * A QImage pointer is provisioned and updated once rendering
 * starts.
 */

struct RenderConnections
{
    QMetaObject::Connection started, stopped, updated;
};

class FrameWrapper final : public QObject
{
    Q_OBJECT;

public:
    FrameWrapper(AVModel& avModel, const QString& id);
    ~FrameWrapper();

    /*
     * Reconnect the started rendering connection for this object.
     */
    void connectStartRendering();

    /*
     * Get a pointer to the renderer and reconnect the update/stopped
     * rendering connections for this object.
     * @return whether the start succeeded or not
     */
    bool startRendering();

    /*
     * Locally disable frame access to this FrameWrapper
     */
    void stopRendering();

    /*
     * Get the most recently rendered frame as a QImage.
     * @return the rendered image of this object's id
     */
    QImage* getFrame();

    /*
     * Check if the object is updating actively.
     */
    bool isRendering();

    bool frameMutexTryLock();

    void frameMutexUnlock();

    const QString getId()
    {
        return id_;
    }

Q_SIGNALS:
    /*
     * Emitted once in slotRenderingStarted.
     * @param id of the renderer
     */
    void renderingStarted(const QString& id);
    /*
     * Emitted each time a frame is ready to be displayed.
     * @param id of the renderer
     */
    void frameUpdated(const QString& id);
    /*
     * Emitted once in slotRenderingStopped.
     * @param id of the renderer
     */
    void renderingStopped(const QString& id);

public Q_SLOTS:
    /*
     * Used to listen to AVModel::rendererStarted.
     * @param id of the renderer
     */
    void slotRenderingStarted(const QString& id);
    /*
     * Used to listen to AVModel::frameUpdated.
     * @param id of the renderer
     */
    void slotFrameUpdated(const QString& id);
    /*
     * Used to listen to AVModel::renderingStopped.
     * @param id of the renderer
     */
    void slotRenderingStopped(const QString& id);

private:
    /*
     * The id of the renderer.
     */
    QString id_;

    /*
     * A pointer to the lrc renderer object.
     */
    video::Renderer* renderer_;

    /*
     * A local copy of the renderer's current frame.
     */
    video::Frame frame_;

    /*
     * A the frame's storage data used to set the image.
     */
    std::vector<uint8_t> buffer_;

    /*
     * The frame's paint ready QImage.
     */
    std::unique_ptr<QImage> image_;

    /*
     * Used to protect the buffer during QImage creation routine.
     */
    QMutex mutex_;

    /*
     * True if the object is rendering
     */
    std::atomic_bool isRendering_;

    /*
     * Convenience ref to avmodel
     */
    AVModel& avModel_;

    /*
     * Connections to the underlying renderer signals in avmodel
     */
    RenderConnections renderConnections_;
};

/**
 * RenderManager filters signals and ecapsulates preview and distant
 * frame wrappers, providing access to QImages for each and simplified
 * start/stop mechanisms for renderers. It should contain as much
 * renderer control logic as possible and prevent ui widgets from directly
 * interfacing the rendering logic.
 */
class RenderManager final : public QObject
{
    Q_OBJECT;

public:
    explicit RenderManager(AVModel& avModel);
    ~RenderManager();

    using DrawFrameCallback = std::function<void(QImage*)>;

    /*
     * Start capturing and rendering preview frames.
     * @param force if the capture device should be started
     */
    const QString startPreviewing(const QString& id, bool force = false);
    /*
     * Stop capturing.
     */
    void stopPreviewing(const QString& id);
    /*
     * Add and connect a distant renderer for a given id
     * to a FrameWrapper object
     * @param id
     */
    void addDistantRenderer(const QString& id);
    /*
     * Disconnect and remove a FrameWrapper object connected to a
     * distant renderer for a given id
     * @param id
     */
    void removeDistantRenderer(const QString& id);
    /*
     * Frame will be provided in the callback thread safely
     * @param id
     * @param cb
     */
    void drawFrame(const QString& id, DrawFrameCallback cb);

    /*
     * Get the most recently rendered preview frame as a QImage (none thread safe).
     * @return the rendered preview image
     */
    QImage* getPreviewFrame(const QString& id = "");

Q_SIGNALS:
    /*
     * Emitted when a distant renderer has a new frame ready for a given id.
     */
    void distantFrameUpdated(const QString& id);

    /*
     * Emitted when a distant renderer is stopped for a given id.
     */
    void distantRenderingStopped(const QString& id);

private:
    /*
     * Distant for each call/conf/conversation.
     */
    std::map<QString, std::unique_ptr<FrameWrapper>> distantFrameWrapperMap_;
    std::map<QString, RenderConnections> distantConnectionMap_;

    /*
     * Convenience ref to avmodel.
     */
    AVModel& avModel_;
};
