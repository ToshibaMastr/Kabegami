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
#include <iostream>
#include <gst/gst.h>
#include "GStreamer.h"

void GStreamer::gst_log(GstDebugCategory * category,
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
        std::cerr << file << ":" << line << " " << function << " [CRITICAL] " << object_info << " " << gst_debug_message_get(message) << std::endl;
        break;
    case GST_LEVEL_WARNING:
        std::cerr << file << ":" << line << " " << function << " [WARNING] " << object_info << " " << gst_debug_message_get(message) << std::endl;
        break;
    case GST_LEVEL_FIXME:
    case GST_LEVEL_INFO:
        std::cout << file << ":" << line << " " << function << " [INFO] " << object_info << " " << gst_debug_message_get(message) << std::endl;
        break;
    case GST_LEVEL_DEBUG:
    case GST_LEVEL_LOG:
    case GST_LEVEL_TRACE:
    case GST_LEVEL_MEMDUMP:
        std::cout << file << ":" << line << " " << function << " [DEBUG] " << object_info << " " << gst_debug_message_get(message) << std::endl;
        break;
    }

    std::free(object_info);
    object_info = nullptr;
}

bool GStreamer::initialize(int argc, char* argv[]) {
    GError* error = nullptr;
    if (!gst_init_check(&argc, &argv, &error)) {
        gst_printerrln("Gstreamer init check failed: %s", error->message);
        g_error_free(error);
        return false;
    }
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
        gst_printerrln("Failed to get gstreamer registry.");
        return false;
    }

    auto changeRank = [registry](const char* featureName, uint16_t rank) {
        GstPluginFeature* feature = gst_registry_lookup_feature(registry, featureName);
        if (feature == nullptr) {
            gst_printerrln("Failed to change ranking of feature. Featuer does not exist: %s", featureName);
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
        gst_printerrln("Can't handle decode option: %d", option);
        return false;
    }
    return black;
}
