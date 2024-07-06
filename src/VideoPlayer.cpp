/*
 * File name: VideoPlayer.cpp
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

#include "VideoPlayer.h"
#include <gst/video/videooverlay.h>
#include "GStreamer.h"
#include "KLoggeg.h"

VideoPlayer::VideoPlayer(const VideoSettings& settings) : settings(settings), loop(GStreamer::getMainLoop()) {}

VideoPlayer::~VideoPlayer() {
    stop();
}

bool VideoPlayer::init() {
    pipeline = gst_pipeline_new("video-player");
    GstElement *source = gst_element_factory_make("filesrc", nullptr);
    GstElement *demuxer = gst_element_factory_make("qtdemux", nullptr);
    GstElement *decoder = gst_element_factory_make("decodebin3", nullptr);
    GstElement *converter = gst_element_factory_make("videoconvert", nullptr);
    GstElement *tee = gst_element_factory_make("tee", "t");

    if (!pipeline || !source || !demuxer || !decoder || !converter || !tee) {
        fatal("VideoPlayer") << "One element could not be created"; // 𓆏
        return false;
    }

    gst_bin_add_many(GST_BIN(pipeline), source, demuxer, decoder, converter, tee, NULL);

    if (!gst_element_link(source, demuxer)) {
        fatal("VideoPlayer") << "Source and Demuxer could not be linked";
        return false;
    }

    g_signal_connect(demuxer, "pad-added", G_CALLBACK(onNewPad), decoder);
    g_signal_connect(decoder, "pad-added", G_CALLBACK(onNewPad), converter);

    if (!gst_element_link_many(converter, tee, NULL)) {
        fatal("VideoPlayer") << "Elements could not be linked";
        return false;
    }

    g_object_set(G_OBJECT(source), "location", settings.filename.data(), NULL);

    GstBus *bus;
    if ((bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline))) != nullptr) {
        gst_bus_add_watch(bus, onBusMessage, this);
        gst_object_unref(bus);
        bus = nullptr;
    }

    return true;
}

bool VideoPlayer::start() {
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        error("VideoPlayer") << "Pipeline failed to start";
        gst_object_unref(pipeline);
        pipeline = nullptr;
        return false;
    }
    return true;
}

void VideoPlayer::stop() {
    if (pipeline){
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(pipeline));
        pipeline = nullptr;
    }
}

bool VideoPlayer::addWindow(guintptr wid) {
    GstElement *tee = gst_bin_get_by_name(GST_BIN(pipeline), "t");
    if (tee == nullptr) {
        fatal("VideoPlayer") << "Tee not found in pipeline";
        return false;
    }

    GstElement *queue;
    GstElement *sink;
    std::string sink_name;

    switch (settings.overlay) {
        case X11:
            sink_name = "xvimagesink";
            sink = gst_element_factory_make("xvimagesink", nullptr);
            break;
        case OpenGL:
            sink_name = "glimagesink";
            sink = gst_element_factory_make("glimagesink", nullptr);
            break;
        case Wayland:
            sink_name = "waylandsink";
            sink = gst_element_factory_make("waylandsink", nullptr);
            break;
        case DirectX:
            sink_name = "d3dvideosink";
            break;
        default:
            fatal("VideoPlayer") << "Invalid video overlay option";
            return false;
    }

    queue = gst_element_factory_make("queue", nullptr);
    sink = gst_element_factory_make(sink_name.data(), nullptr);

    if (!sink) {
        fatal("VideoPlayer") << "Failed to create a " << sink_name << " element";
        return false;
    }

    if (!queue) {
        fatal("VideoPlayer") << "Failed to create a queue element";
        return false;
    }

    g_object_set(G_OBJECT(queue), "max-size-buffers", 2, "max-size-time", 0, "max-size-bytes", 0, NULL);

    gst_bin_add_many(GST_BIN(pipeline), queue, sink, NULL);
    gst_element_link_many(tee, queue, sink, NULL);

    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(sink), wid);

    tee = queue = sink = nullptr;

    return true;
}

void VideoPlayer::onNewPad(GstElement *element, GstPad *pad, GstElement *data) {
    GstPad *sink_pad = gst_element_get_static_pad(data, "sink");
    if (gst_pad_is_linked(sink_pad)) {
        g_object_unref(sink_pad);
        return;
    }
    if (gst_pad_link(pad, sink_pad) != GST_PAD_LINK_OK) {
        error("VideoPlayer") << "Failed to link pads";
    }
    g_object_unref(sink_pad);
}

gboolean VideoPlayer::onBusMessage(GstBus *bus, GstMessage *msg, gpointer data) {
    VideoPlayer *player = static_cast<VideoPlayer*>(data);
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: {
            if (player->settings.loop) {
                GstElement *pipeline = GST_ELEMENT(msg->src);
                gst_element_seek_simple(pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, 0);
            } else {
                g_main_loop_quit(player->loop);
            }
            break;
        }
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(msg, &err, &debug);
            g_free(debug);
            error("VideoPlayer") << "Error: " << err->message;
            g_error_free(err);
            g_main_loop_quit(player->loop);
            player->stop();
            break;
        }
        default:
            break;
    }

    return TRUE;
}
