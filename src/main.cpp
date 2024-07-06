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
#include <csignal>
#include <cstdlib>
#include <memory>
#include <vector>

void cleanup() {
    info("Main") << "cleanup";
    XrandrManager::cleanup();
    GStreamer::cleanup();
}

void signal_handler(int signal) {
    info("Main") << "Received " << signal << " signal";
    switch (signal) {
        case SIGINT:
        case SIGTERM:
            cleanup();
            std::exit(0);
    }
}

int ProjectMain(int argc, char *argv[]) {
    if (!GStreamer::initialize(argc, argv) ||
        !XrandrManager::initialize()){
        return -1;
    }

    VideoSettings settings;
    if(!CLIHandler::splitArgs(argc, argv, settings)){
        return -1;
    }
    if (settings.filename.empty()) {
        return 0;
    }

    if (!GStreamer::createMainLoop()) {
        return -1;
    }

    VideoPlayer videoPlayer(settings);
    if (!videoPlayer.init()) {
        return -1;
    }

    auto monitors = XrandrManager::getMonitors();
    std::vector<std::unique_ptr<XWPWindow>> windows;

    for (const auto& monitor : monitors) {
        auto window = std::make_unique<XWPWindow>();
        if (!window->createWindow(monitor) ||
            !videoPlayer.addWindow(window->getWindow())) {
            error("Main") << "Failed to create window: "
                          << monitor.name << ", resolution: " << monitor.width << "x" << monitor.height;
            continue;
        }

        windows.push_back(std::move(window));
        info("XrandrManager") << "name: " << monitor.name << ", resolution: " << monitor.width << "x" << monitor.height;
    }

    if (!videoPlayer.start()) {
        return -1;
    }

    GStreamer::runMainLoop();

    if (windows.empty()) {
        windows.clear();
    }

    if (!videoPlayer.start()) {
        videoPlayer.stop();
    }
    return 0;
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    int ret = ProjectMain(argc, argv);
    cleanup();
    return ret;
}
