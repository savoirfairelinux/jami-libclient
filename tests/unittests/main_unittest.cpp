/*
 * Copyright (C) 2021 by Savoir-faire Linux
 * Author: Albert Babí Oller <albert.babi@savoirfairelinux.com>
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

#include "globaltestenvironment.h"

#include <QApplication>
#include <QStandardPaths>

#include <gtest/gtest.h>

bool muteDring;

int
main(int argc, char* argv[])
{
    // Remove "-mutedring" from argv, as quick_test_main_with_setup() will
    // fail if given an invalid command-line argument.
    auto end = std::remove_if(argv + 1, argv + argc, [](char* argv) {
        return (strcmp(argv, "-mutedring") == 0);
    });

    if (end != argv + argc) {
        muteDring = true;

        // Adjust the argument count.
        argc = std::distance(argv, end);
    }

    QStandardPaths::setTestModeEnabled(true);

    QApplication a(argc, argv);
    a.processEvents();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
