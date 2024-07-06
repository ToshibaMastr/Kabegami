/*
 * File name: main.cpp
 * Author: ToshibaMastru
 * Copyright (c) 2024 ToshibaMastru
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "XWPWindow.h"
#include "VideoPlayer.h"
#include "GStreamer.h"
#include "CLIHandler.h"
#include "KLoggeg.h"
#include <memory>
#include <vector>

int main(int argc, char *argv[]) {
    if (!GStreamer::initialize(argc, argv)){
        fatal("GStreamer") << "Failed to initialize";
        return -1;
    }
    if (!XrandrManager::initialize()) {
        fatal("XrandrManager") << "Failed to initialize";
        return -1;
    }

    VideoSettings settings;
    if(!CLIHandler::splitArgs(argc, argv, settings)){
        return -1;
    }
    if (settings.filename.empty()) {
        return 0;
    }

    gst_init(&argc, &argv);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    VideoPlayer videoPlayer(loop, settings);
    if (!videoPlayer.init()) {
        return -1;
    }

    std::vector<MonitorInfo> monitors = XrandrManager::getMonitors();
    std::vector<std::unique_ptr<XWPWindow>> windows;

    for (const auto& monitor : monitors) {
        auto window = std::make_unique<XWPWindow>();
        if (!window->createWindow(monitor)) {
            return -1;
        }

        if (!videoPlayer.addWindow(window->getWindow())) {
            return -1;
        }

        windows.push_back(std::move(window));

        info("XrandrManager") << "name: " << monitor.name << ", resolution: " << monitor.width << "x" << monitor.height;
    }

    if (!videoPlayer.start()) {
        return -1;
    }

    g_main_loop_run(loop);

    videoPlayer.stop();

    windows.clear();

    XrandrManager::cleanup();
    gst_deinit();
    return 0;
}
