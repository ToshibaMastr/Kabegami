/*
 * File name: XWPWindow.cpp
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

XWPWindow::XWPWindow() : display(nullptr), win(0) {}

XWPWindow::~XWPWindow() {
    closeWindow();
}

bool XWPWindow::createWindow(int x, int y, int width, int height) {
    display = XOpenDisplay(NULL);
    if (!display) {
        return false;
    }

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    Window root = RootWindow(display, 0);

    XSetWindowAttributes attrs;
    attrs.event_mask = ExposureMask | KeyPressMask;

    win = XCreateWindow(display, root,
                        x, y, width, height,
                        0, CopyFromParent, InputOutput,
                        CopyFromParent, CWEventMask, &attrs);

    Atom atom_below = XInternAtom(display, "_NET_WM_STATE_BELOW", False);
    Atom atom_state = XInternAtom(display, "_NET_WM_STATE", False);
    XChangeProperty(display, win, atom_state, XA_ATOM, 32, PropModeReplace, (unsigned char *)&atom_below, 1);

    Atom atom_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom atom_desktop = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    XChangeProperty(display, win, atom_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)&atom_desktop, 1);

    XMapWindow(display, win);
    XLowerWindow(display, win);

    XSync(display, False);

    return true;
}

void XWPWindow::closeWindow() {
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
}

Window XWPWindow::getWindow() const {
    return win;
}
