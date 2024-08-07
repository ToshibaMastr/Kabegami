/*
 * File name: XWPWindow.h
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

#include <X11/Xatom.h>
#include "XrandrManager.h"

class XWPWindow {
public:
    XWPWindow();
    ~XWPWindow();

    bool createWindow(const MonitorInfo& monitorInfo);
    Window getWindow() const;

private:
    Window win;

    void destroyWindow();
};
