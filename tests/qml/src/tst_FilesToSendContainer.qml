/*
 * Copyright (C) 2021 by Savoir-faire Linux
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14

import QtTest 1.2

import net.jami.Models 1.1
import net.jami.Constants 1.1

import "qrc:/src/mainview/components"

ColumnLayout {
    id: root

    spacing: 0

    width: 300
    height: 300

    FilesToSendContainer {
        id: uut

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: root.width
        Layout.maximumWidth: JamiTheme.chatViewMaximumWidth
        Layout.preferredHeight: filesToSendCount ?
                                    JamiTheme.chatViewFooterFileContainerPreferredHeight : 0

        TestCase {
            name: "FilesToSendContainer add/remove file test"
            when: windowShown

            function test_add_remove_file_test() {
                // Add animated image file
                uut.filesToSendListModel.addToPending(":/src/resources/gif_test.gif")
                compare(uut.filesToSendCount, 1)

                // Add image file
                uut.filesToSendListModel.addToPending(":/src/resources/png_test.png")
                compare(uut.filesToSendCount, 2)

                // Add normal file
                uut.filesToSendListModel.addToPending(":/src/resources/gz_test.gz")
                compare(uut.filesToSendCount, 3)

                // Flush
                uut.filesToSendListModel.flush()
                compare(uut.filesToSendCount, 0)
            }
        }
    }
}
