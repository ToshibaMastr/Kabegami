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
#include <cstdlib>
#include <memory>
#include <vector>

std::vector<std::unique_ptr<XWPWindow>> windows;
VideoPlayer videoPlayer;

void cleanup() {
    windows.clear();
    videoPlayer.stop();

    XrandrManager::cleanup();
    GStreamer::cleanup();
}

void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            cleanup();
            exit(0);
    }
}

int ProjectMain(int argc, char *argv[]) {
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

    GStreamer::createMainLoop();

    videoPlayer.setSettings(settings);
    if (!videoPlayer.init()) {
        return -1;
    }

    auto monitors = XrandrManager::getMonitors();

    for (const auto& monitor : monitors) {
        auto window = std::make_unique<XWPWindow>();
        if (!window->createWindow(monitor) && !videoPlayer.addWindow(window->getWindow())) {
            fatal("XrandrManager") << "name: " << monitor.name << ", resolution: " << monitor.width << "x" << monitor.height;
            continue;
        }

        windows.push_back(std::move(window));

        info("XrandrManager") << "name: " << monitor.name << ", resolution: " << monitor.width << "x" << monitor.height;
    }

    if (!videoPlayer.start()) {
        return -1;
    }

    GStreamer::runMainLoop();

    return 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    int ret = ProjectMain(argc, argv);
    cleanup();
    return ret;
}
