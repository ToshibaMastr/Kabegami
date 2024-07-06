/*
 * File name: CLIHandler.cpp
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

 #include "CLIHandler.h"
 #include <getopt.h>
 #include <map>

 int getParam(const std::map<std::string, int>& map, const char* arg) {
     auto it = map.find(arg);
     return (it != map.end()) ? it->second : -1;
 }

bool CLIHandler::splitArgs(const int argc, char *argv[], VideoSettings& settings) {
    static const std::map<std::string, int> overlayMap = {
        {"X11", OverlayType::X11},
        {"OpenGL", OverlayType::OpenGL},
        {"Wayland", OverlayType::Wayland},
        {"DirectX", OverlayType::DirectX}
    };
    static const std::map<std::string, int> decoderMap = {
        {"Default", DecoderType::Default},
        {"Software", DecoderType::Software},
        {"NVIDIA", DecoderType::NVIDIA},
        {"VAAPI", DecoderType::VAAPI},
        {"DirectX3D", DecoderType::DirectX3D}
    };
    static const std::map<std::string, int> qualityMap = {
        {"high", QualityType::high},
        {"medium", QualityType::medium},
        {"low", QualityType::low}
    };

    static struct option long_options[] = {
        {"overlay", required_argument, 0, 'o'},
        {"force-decoder", required_argument, 0, 'f'},
        {"loop", no_argument, 0, 'l'},
        {"quality", required_argument, 0, 'q'},
        {"gst-debug-level", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    while ((opt = getopt_long(argc, argv, "o:f:lq:d:h", long_options, &option_index)) != -1) {
        int ret;
        switch (opt) {
        case 'o':
            if ((ret = getParam(overlayMap, optarg)) == -1) {
                std::cerr << "Missing option for -o: " << optarg << "\n\n";
                return false;
            }
            settings.overlay = (OverlayType)ret;
            break;
        case 'f':
            if ((ret = getParam(decoderMap, optarg)) == -1) {
                std::cerr << "Missing option for -f: " << optarg << "\n\n";
                return false;
            }
            settings.decoder = (DecoderType)ret;
            break;
        case 'q':
            if ((ret = getParam(qualityMap, optarg)) == -1) {
                std::cerr << "Missing option for -q: " << optarg << "\n\n";
                return false;
            }
            settings.quality = (QualityType)ret;
            break;

        case 'l':
            settings.loop = true;
            break;
        case 'd':
            GStreamer::enableDebug(atoi(optarg));
            break;
        case 'h':
            printHelp(EXECUTABLE_NAME);
            return true;
        default:
            printHelp(argv[0]);
            return false;
        }

    }

    if (optind >= argc) {
        std::cerr << "Expected .mp4 filename\n";
        printHelp(argv[0]);
        return false;
    }

    settings.filename = argv[optind];
    return true;
}

void CLIHandler::printHelp(std::string& prog_name) {
    printHelp(prog_name.data());
}

void CLIHandler::printHelp(const char* prog_name) {
    std::cout << "Usage:\n"
              << "  " << prog_name << " [options] <video file>\n\n"
              << "Options:\n"
              << "  -o, --overlay <sink>               Set video overlay sink\n"
              << "                                     Supported sinks: X11, OpenGL, Wayland, DirectX\n"
              << "  -f, --force-decoder <decoder>      Force video decoder\n"
              << "                                     Supported decoders: Default, Software, NVIDIA, VAAPI, DirectX3D\n"
              << "  -l, --loop                         Enable video looping\n"
              << "  -d, --gst-debug-level <level>      Set GStreamer debug level (0-7)\n"
              << "  -h, --help                         Print this help message\n\n"
              << "Example:\n"
              << "  " << prog_name << " -o glimagesink -f NVIDIA -q high -l video.mp4\n\n";
}
