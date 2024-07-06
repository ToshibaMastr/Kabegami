/*
 * File name: GStreamer.h
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
 *
 * This code is inspired by QGroundControl (QGC), which is dual-licensed
 * under Apache 2.0 and GPLv3. For more information about QGC, visit:
 * https://github.com/mavlink/qgroundcontrol
 */

#pragma once
#include <gst/gst.h>

enum DecoderType {
    Default = 0,
    Software,
    NVIDIA,
    VAAPI,
    DirectX3D
};

class GStreamer {
public:
    static bool initialize(int argc, char* argv[]);
    static void enableDebug(int debuglevel);
    static bool blacklist(DecoderType option);

private:
    static void gst_log(GstDebugCategory * category,
                        GstDebugLevel      level,
                        const gchar      * file,
                        const gchar      * function,
                        gint               line,
                        GObject          * object,
                        GstDebugMessage  * message,
                        gpointer           data);
};
