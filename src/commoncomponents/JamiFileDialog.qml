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

import QtQuick
import Qt.labs.platform

import net.jami.Constants 1.1

FileDialog {
    id: root

    // Use enum to avoid importing Qt.labs.platform when using JamiFileDialog.
    property int mode: JamiFileDialog.Mode.OpenFile

    enum Mode {
        OpenFile = 0,
        OpenFiles,
        SaveFile
    }

    title: JamiStrings.selectFile

    onModeChanged: {
        switch(mode) {
          case JamiFileDialog.Mode.OpenFile:
              root.fileMode = FileDialog.OpenFile
              break
          case JamiFileDialog.Mode.OpenFiles:
              root.fileMode = FileDialog.OpenFiles
              break
          default:
              root.fileMode = FileDialog.SaveFile
        }
    }
}
