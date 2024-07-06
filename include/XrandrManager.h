/*
 * File name: XrandrManager.h
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

#pragma once
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <vector>
#include <string>

struct MonitorInfo {
    std::string name;
    int width;
    int height;
    int x;
    int y;
    bool primary;
};

class XrandrManager {
public:
    static bool initialize();
    static void cleanup();
    static Display* getDisplay() { return display; }
    static Window getRoot() { return root; }
    static std::vector<MonitorInfo> getMonitors();
    static MonitorInfo getPrimaryMonitor();

private:
    static Display* display;
    static Window root;

    static void updateMonitorInfo();
    static std::vector<MonitorInfo> monitors;
};
