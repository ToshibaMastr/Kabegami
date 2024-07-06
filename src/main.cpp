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
#include <memory>
#include <vector>
#include <iostream>
#include <getopt.h>

void print_help(const char *prog_name) {
    std::cout << "Usage:\n"
              << "  " << prog_name << " [options] <video file>\n\n"
              << "Options:\n"
              << "  -o, --overlay <sink>               Set video overlay sink\n"
              << "                                     Supported sinks: xvimagesink, glimagesink, waylandsink, d3dvideosink\n"
              << "  -f, --force-decoder <decoder>      Force video decoder\n"
              << "                                     Supported decoders: Default, Software, NVIDIA, VAAPI, DirectX3D\n"
              << "  -l, --loop                         Enable video looping\n"
              << "  -d, --gst-debug-level <level>      Set GStreamer debug level (0-7)\n"
              << "  -h, --help                         Print this help message\n\n"
              << "Example:\n"
              << "  " << prog_name << " -o glimagesink -f NVIDIA -q high -l video.mp4\n\n";
}

bool splitArgs(int argc, char *argv[], VideoSettings& settings) {
    int opt;
    int option_index = 0;

    static struct option long_options[] = {
        {"overlay", required_argument, 0, 'o'},
        {"force-decoder", required_argument, 0, 'f'},
        {"loop", no_argument, 0, 'l'},
        {"quality", required_argument, 0, 'q'},
        {"gst-debug-level", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "o:f:lq:d:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                if (optarg == nullptr) {
                    std::cerr << "Missing argument for -o option\n\n";
                    print_help(argv[0]);
                    return -1;
                }
                if (!strcmp(optarg, "xvimagesink")) {
                    settings.overlay = xvimagesink;
                } else if (!strcmp(optarg, "glimagesink")) {
                    settings.overlay = glimagesink;
                } else if (!strcmp(optarg, "waylandsink")) {
                    settings.overlay = waylandsink;
                } else if (!strcmp(optarg, "d3dvideosink")) {
                    settings.overlay = d3dvideosink;
                } else {
                    std::cerr << "Invalid overlay option: " << optarg << "\n\n";
                    print_help(argv[0]);
                    return -1;
                }
                break;
            case 'f':
                if (optarg == nullptr) {
                    std::cerr << "Missing argument for -f option\n\n";
                    print_help(argv[0]);
                    return -1;
                }
                bool ret;
                if (!strcmp(optarg, "Default")) {
                    ret = GStreamer::blacklist(ForceVideoDecoderDefault);
                } else if (!strcmp(optarg, "Software")) {
                    ret = GStreamer::blacklist(ForceVideoDecoderSoftware);
                } else if (!strcmp(optarg, "NVIDIA")) {
                    ret = GStreamer::blacklist(ForceVideoDecoderNVIDIA);
                } else if (!strcmp(optarg, "VAAPI")) {
                    ret = GStreamer::blacklist(ForceVideoDecoderVAAPI);
                } else if (!strcmp(optarg, "DirectX3D")) {
                    ret = GStreamer::blacklist(ForceVideoDecoderDirectX3D);
                } else {
                    std::cerr << "Invalid overlay option: " << optarg << "\n\n";
                    print_help(argv[0]);
                    return -1;
                }
                if (!ret){
                    std::cerr << optarg << " decoder is not supported, using Default\n";
                }
                break;
            case 'q':
                if (optarg == nullptr) {
                    std::cerr << "Missing argument for -q option\n\n";
                    print_help(argv[0]);
                    return -1;
                }
                if (!strcmp(optarg, "high")) {
                    settings.quality = high;
                } else if (!strcmp(optarg, "medium")) {
                    settings.quality = medium;
                } else if (!strcmp(optarg, "low")) {
                    settings.quality = low;
                } else {
                    std::cerr << "Invalid quality option: " << optarg << "\n\n";
                    print_help(argv[0]);
                    return -1;
                }
                break;
            case 'l':
                settings.loop = true;
                break;
            case 'd':
                GStreamer::enableDebug(atoi(optarg));
                break;
            case 'h':
                print_help(EXECUTABLE_NAME);
                return -1;
            default:
                print_help(argv[0]);
                return -1;
        }
    }

    if (optind >= argc) {
        std::cerr << "Expected .mp4 filename after options\n\n";
        print_help(argv[0]);
        return -1;
    }

    settings.filename = argv[optind];
    return 0;
}

int main(int argc, char *argv[]) {
    if (!GStreamer::initialize(argc, argv)){
        std::cerr << "Failed to initialize Gstreamer\n\n";
        return -1;
    }
    if (!XrandrManager::initialize()) {
        std::cerr << "Failed to initialize Xrandr\n\n";
        return -1;
    }

    VideoSettings settings;
    if(splitArgs(argc, argv, settings)){
        return -1; // -1 even with --help
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
            std::cerr << "Failed to create window for screen " << monitor.name << "\n\n";
            return -1;
        }

        videoPlayer.addWindow(window->getWindow());
        windows.push_back(std::move(window));

        std::cout << "name: " << monitor.name << ", resolution: " << monitor.width << "x" << monitor.height << "\n";
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
