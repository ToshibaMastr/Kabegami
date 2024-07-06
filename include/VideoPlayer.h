/*
 * File name: VideoPlayer.h
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
#include <gst/gst.h>
#include "GStreamer.h"

enum OverlayType {
    xvimagesink = 0,
    glimagesink,
    waylandsink,
    d3dvideosink
};

enum QualityType {
    high = 0,
    medium,
    low
};

struct VideoSettings {
    OverlayType overlay = xvimagesink;
    DecoderType decoder = xvimagesink;
    QualityType quality = medium;
    bool loop = false;
    const char *filename;
};

class VideoPlayer {
public:
    VideoPlayer(GMainLoop *loop, const VideoSettings& settings);
    ~VideoPlayer();

    bool init();
    bool start();
    void stop();

    bool addWindow(guintptr wid);

private:
    static void onNewPad(GstElement *element, GstPad *pad, GstElement *data);
    static gboolean onBusMessage(GstBus *bus, GstMessage *msg, gpointer data);

private:
    VideoSettings settings;

    GMainLoop  *loop;

    GstElement *pipeline;
};
