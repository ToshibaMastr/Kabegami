/*
 * File name: GStreamer.cpp
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

#include <cstdint>
#include <gst/gst.h>
#include "GStreamer.h"
#include "KLoggeg.h"

GMainLoop* GStreamer::loop = nullptr;

void gst_log(GstDebugCategory * category,
                    GstDebugLevel      level,
                    const gchar      * file,
                    const gchar      * function,
                    gint               line,
                    GObject          * object,
                    GstDebugMessage  * message,
                    gpointer           data)
{
    if (level > gst_debug_category_get_threshold(category)) {
        return;
    }

    char* object_info = gst_info_strdup_printf("%" GST_PTR_FORMAT, static_cast<void*>(object));

    switch (level) {
    default:
    case GST_LEVEL_ERROR:
        error("GStreamer") << file << ":" << line << " " << function << " [CRITICAL] " << object_info << " " << gst_debug_message_get(message);
        break;
    case GST_LEVEL_WARNING:
        warning("GStreamer") << file << ":" << line << " " << function << " [WARNING] " << object_info << " " << gst_debug_message_get(message);
        break;
    case GST_LEVEL_FIXME:
    case GST_LEVEL_INFO:
        info("GStreamer") << file << ":" << line << " " << function << " [INFO] " << object_info << " " << gst_debug_message_get(message);
        break;
    case GST_LEVEL_DEBUG:
    case GST_LEVEL_LOG:
    case GST_LEVEL_TRACE:
    case GST_LEVEL_MEMDUMP:
        info("GStreamer") << file << ":" << line << " " << function << " [DEBUG] " << object_info << " " << gst_debug_message_get(message);
        break;
    }

    std::free(object_info);
    object_info = nullptr;
}

bool GStreamer::initialize(int argc, char* argv[]) {
    GError* gerror = nullptr;
    if (!gst_init_check(&argc, &argv, &gerror)) {
        fatal("GStreamer") << "Init check failed: " << gerror->message;
        g_error_free(gerror);
        return false;
    }
    gst_init(&argc, &argv);
    return true;
}

void GStreamer::enableDebug(int debuglevel) {
    if (debuglevel > 0) {
        gst_debug_set_default_threshold(static_cast<GstDebugLevel>(debuglevel));
        gst_debug_remove_log_function(gst_debug_log_default);
        gst_debug_add_log_function(gst_log, nullptr, nullptr);
    }
}

bool GStreamer::blacklist(DecoderType option) {
    GstRegistry* registry = gst_registry_get();

    if (registry == nullptr) {
        error("GStreamer") << "Failed to get registry";
        return false;
    }

    auto changeRank = [registry](const char* featureName, uint16_t rank) {
        GstPluginFeature* feature = gst_registry_lookup_feature(registry, featureName);
        if (feature == nullptr) {
            error("GStreamer") << "Failed to change ranking of feature. Featuer does not exist: " << featureName;
            return false;
        }

        gst_plugin_feature_set_rank(feature, rank);
        gst_registry_add_feature(registry, feature);
        gst_object_unref(feature);
        return true;
    };

    bool black = false;
    switch (option) {
    case Default:
        break;
    case Software:
        for (auto name : { "avdec_h264", "avdec_h265" }) {
            black |= changeRank(name, GST_RANK_PRIMARY + 1);
        }
        break;
    case VAAPI:
        for (auto name : { "vajpegdec", "vampeg2dec", "vah264dec", "vah265dec", "vaav1dec" }) {
            black |= changeRank(name, GST_RANK_PRIMARY + 1);
        }
        break;
    case NVIDIA:
        for (auto name : { "nvh265dec", "nvh265sldec", "nvh264dec", "nvh264sldec" }) {
            black |= changeRank(name, GST_RANK_PRIMARY + 1);
        }
        break;
    case DirectX3D:
        for (auto name : { "d3d11vp9dec", "d3d11h265dec", "d3d11h264dec" }) {
            black |= changeRank(name, GST_RANK_PRIMARY + 1);
        }
        break;
    default:
        error("GStreamer") << "Can't handle decode option: " << option;
        return false;
    }
    return black;
}

bool GStreamer::createMainLoop() {
    loop = g_main_loop_new(NULL, FALSE);
    if (!loop) {
        fatal("GStreamer") << "Can't create loop";
        return false;
    }
    return true;
}

void GStreamer::runMainLoop() {
    g_main_loop_run(loop);
}

void GStreamer::cleanup() {
    if (loop) {
        g_main_loop_quit(loop);
        g_main_loop_unref(loop);
        loop = nullptr;
    }
    gst_deinit();
}

GMainLoop* GStreamer::getMainLoop() {
    return loop;
}
