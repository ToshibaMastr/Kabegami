/*
 * File name: XrandrManager.cpp
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

#include "XrandrManager.h"
#include "KLoggeg.h"

Display* XrandrManager::display = nullptr;
Window XrandrManager::root = None;
std::vector<MonitorInfo> XrandrManager::monitors;

int x_log(Display *display, XErrorEvent *xerror) {
    char errorText[256];
    XGetErrorText(display, xerror->error_code, errorText, sizeof(errorText));
    error("X11") << errorText;
    return 0;
}

bool XrandrManager::initialize() {
    XSetErrorHandler(x_log);
    display = XOpenDisplay(nullptr);
    if (!display) {
        fatal("XrandrManager") << "Failed to initialize";
        return false;
    }
    root = DefaultRootWindow(display);
    updateMonitorInfo();
    return true;
}

void XrandrManager::cleanup() {
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
}

std::vector<MonitorInfo> XrandrManager::getMonitors() {
    return monitors;
}

MonitorInfo XrandrManager::getPrimaryMonitor() {
    for (const auto& monitor : monitors) {
        if (monitor.primary) {
            return monitor;
        }
    }
    return MonitorInfo();
}

void XrandrManager::updateMonitorInfo() {
    monitors.clear();

    XRRScreenResources* res = XRRGetScreenResources(display, root);
    if (!res) return;

    for (int i = 0; i < res->noutput; i++) {
        XRROutputInfo* output_info = XRRGetOutputInfo(display, res, res->outputs[i]);
        if (!output_info || output_info->connection != RR_Connected) {
            XRRFreeOutputInfo(output_info);
            continue;
        }

        XRRCrtcInfo* crtc_info = XRRGetCrtcInfo(display, res, output_info->crtc);
        if (crtc_info) {
            MonitorInfo info;
            info.name = output_info->name;
            info.width = crtc_info->width;
            info.height = crtc_info->height;
            info.x = crtc_info->x;
            info.y = crtc_info->y;
            info.primary = (XRRGetOutputPrimary(display, root) == res->outputs[i]);

            monitors.push_back(info);

            XRRFreeCrtcInfo(crtc_info);
        }
        XRRFreeOutputInfo(output_info);
    }
    XRRFreeScreenResources(res);
}
