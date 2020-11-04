/*
 * Copyright (C) 2020 by Savoir-faire Linux
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

// JamiQmlUtils as a singleton is to provide global property entry
pragma Singleton

import QtQuick 2.14

Item {
    readonly property string mainViewLoadPath: "qrc:/src/mainview/MainView.qml"
    readonly property string wizardViewLoadPath: "qrc:/src/wizardview/WizardView.qml"

    property bool callIsFullscreen: false

    TextMetrics {
        id: globalTextMetrics
    }

    function getTextBoundingRect(font, text) {
        globalTextMetrics.font = font
        globalTextMetrics.text = text

        return globalTextMetrics.boundingRect
    }
}
