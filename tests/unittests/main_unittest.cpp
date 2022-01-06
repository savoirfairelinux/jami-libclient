/*
 * Copyright (C) 2021-2022 Savoir-faire Linux Inc.
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

TestEnvironment globalEnv;

int
main(int argc, char* argv[])
{
    QDir tempDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));

    auto jamiDataDir = tempDir.absolutePath() + "\\jami_test\\jami";
    auto jamiConfigDir = tempDir.absolutePath() + "\\jami_test\\.config";
    auto jamiCacheDir = tempDir.absolutePath() + "\\jami_test\\.cache";

    bool envSet = qputenv("JAMI_DATA_HOME", jamiDataDir.toLocal8Bit());
    envSet &= qputenv("JAMI_CONFIG_HOME", jamiConfigDir.toLocal8Bit());
    envSet &= qputenv("JAMI_CACHE_HOME", jamiCacheDir.toLocal8Bit());
    if (!envSet)
        return 1;

    // Remove "-mutejamid" from argv, as quick_test_main_with_setup() will
    // fail if given an invalid command-line argument.
    auto end = std::remove_if(argv + 1, argv + argc, [](char* argv) {
        return (strcmp(argv, "-mutejamid") == 0);
    });

    if (end != argv + argc) {
        globalEnv.muteDring = true;

        // Adjust the argument count.
        argc = std::distance(argv, end);
    }

    QApplication a(argc, argv);
    a.processEvents();

    ::testing::InitGoogleTest(&argc, argv);
    globalEnv.SetUp();
    auto result = RUN_ALL_TESTS();
    globalEnv.TearDown();

    return result;
}
