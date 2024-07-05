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

VideoPlayer::VideoPlayer(GMainLoop *loop, const VideoSettings& settings) : settings(settings) , loop(loop) {}

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
        gst_printerrln("One element could not be created. Exiting."); // 𓆏
        return false;
    }

    gst_bin_add_many(GST_BIN(pipeline), source, demuxer, decoder, converter, tee, NULL);

    if (!gst_element_link(source, demuxer)) {
        gst_printerrln("Source and Demuxer could not be linked.");
        return false;
    }

    g_signal_connect(demuxer, "pad-added", G_CALLBACK(onNewPad), decoder);
    g_signal_connect(decoder, "pad-added", G_CALLBACK(onNewPad), converter);

    if (!gst_element_link_many(converter, tee, NULL)) {
        gst_printerrln("Elements could not be linked.");
        return false;
    }

    g_object_set(G_OBJECT(source), "location", settings.filename, NULL);

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
        gst_printerrln("Pipeline failed to start.");
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
        gst_printerrln("Tee not found in pipeline.");
        return false;
    }

    GstElement *queue = gst_element_factory_make("queue", nullptr);
    GstElement *sink;

    switch (settings.overlay) {
        case xvimagesink:
            sink = gst_element_factory_make("xvimagesink", nullptr);
            break;
        case glimagesink:
            sink = gst_element_factory_make("glimagesink", nullptr);
            break;
        case waylandsink:
            sink = gst_element_factory_make("waylandsink", nullptr);
            break;
        case d3dvideosink:
            sink = gst_element_factory_make("d3dvideosink", nullptr);
            break;
        default:
            gst_printerrln("Invalid video overlay option.");
            return false;
    }

    if (!queue || !sink) {
        gst_printerrln("One element could not be created. Exiting.");
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
        gst_printerrln("Failed to link pads.");
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
            gst_printerrln("Error: %s", err->message);
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
