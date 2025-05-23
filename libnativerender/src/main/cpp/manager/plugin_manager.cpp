/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugin_manager.h"
#include "../common/common.h"
#include <ace/xcomponent/native_interface_xcomponent.h>
#include <cstdint>
#include <hilog/log.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_window/external_window.h>
#include <string>

namespace NativeXComponentSample {

// 定义日志标签常量
static constexpr char *PM_LOG_TAG = "PLUGIN_MANAGER";

namespace {
int64_t ParseId(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ParseId env or info is null");
        return -1;
    }
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    if (napi_ok != napi_get_cb_info(env, info, &argc, args, nullptr, nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ParseId GetContext napi_get_cb_info failed");
        return -1;
    }
    int64_t value = 0;
    bool lossless = true;
    if (napi_ok != napi_get_value_bigint_int64(env, args[0], &value, &lossless)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ParseId Get value failed");
        return -1;
    }
    return value;
}
} // namespace

std::unordered_map<int64_t, PluginRender *> PluginManager::pluginRenderMap_;
std::unordered_map<int64_t, OHNativeWindow *> PluginManager::windowMap_;

PluginManager::~PluginManager() {
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "~PluginManager");
    for (auto iter = pluginRenderMap_.begin(); iter != pluginRenderMap_.end(); ++iter) {
        if (iter->second != nullptr) {
            delete iter->second;
            iter->second = nullptr;
        }
    }
    pluginRenderMap_.clear();
    for (auto iter = windowMap_.begin(); iter != windowMap_.end(); ++iter) {
        if (iter->second != nullptr) {
            delete iter->second;
            iter->second = nullptr;
        }
    }
    windowMap_.clear();
}

PluginRender *PluginManager::GetPluginRender(int64_t &id) {
    if (pluginRenderMap_.find(id) != pluginRenderMap_.end()) {
        return pluginRenderMap_[id];
    }
    return nullptr;
}

napi_value PluginManager::SetSurfaceId(napi_env env, napi_callback_info info) {
    int64_t surfaceId = ParseId(env, info);
    OHNativeWindow *nativeWindow;
    PluginRender *pluginRender;
    if (windowMap_.find(surfaceId) == windowMap_.end()) {
        OH_NativeWindow_CreateNativeWindowFromSurfaceId(surfaceId, &nativeWindow);
        windowMap_[surfaceId] = nativeWindow;
    }
    if (pluginRenderMap_.find(surfaceId) == pluginRenderMap_.end()) {
        pluginRender = new PluginRender(surfaceId);
        pluginRenderMap_[surfaceId] = pluginRender;
    }
    // todo surfaceId 与 PluginTexRender 的绑定
    pluginRender->InitNativeWindow(nativeWindow);
    return nullptr;
}

napi_value PluginManager::DestroySurface(napi_env env, napi_callback_info info) {
    int64_t surfaceId = ParseId(env, info);
    auto pluginRenderMapIter = pluginRenderMap_.find(surfaceId);
    if (pluginRenderMapIter != pluginRenderMap_.end()) {
        delete pluginRenderMapIter->second;
        pluginRenderMap_.erase(pluginRenderMapIter);
    }
    auto windowMapIter = windowMap_.find(surfaceId);
    if (windowMapIter != windowMap_.end()) {
        OH_NativeWindow_DestroyNativeWindow(windowMapIter->second);
        windowMap_.erase(windowMapIter);
    }
    // todo surfaceId 与 PluginTexRender 的绑定
    return nullptr;
}

napi_value PluginManager::ChangeSurface(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: OnLoad env or info is null");
        return nullptr;
    }
    int64_t surfaceId = 0;
    size_t argc = 3;
    napi_value args[3] = {nullptr};

    if (napi_ok != napi_get_cb_info(env, info, &argc, args, nullptr, nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG,
                     "ChangeSurface: GetContext napi_get_cb_info failed");
    }
    bool lossless = true;
    int index = 0;
    if (napi_ok != napi_get_value_bigint_int64(env, args[index++], &surfaceId, &lossless)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get value failed");
    }
    double width;
    if (napi_ok != napi_get_value_double(env, args[index++], &width)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get width failed");
    }
    double height;
    if (napi_ok != napi_get_value_double(env, args[index++], &height)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get height failed");
    }
    auto pluginRender = GetPluginRender(surfaceId);
    if (pluginRender == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get pluginRender failed");
        return nullptr;
    }
    pluginRender->UpdateNativeWindowSize(width, height);
    return nullptr;
}

napi_value PluginManager::DrawWithCustomFont(napi_env env, napi_callback_info info) {
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: OnLoad env or info is null");
        return nullptr;
    }
    int64_t surfaceId = 0;
    size_t argc = 4;
    napi_value args[4] = {nullptr};

    if (napi_ok != napi_get_cb_info(env, info, &argc, args, nullptr, nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG,
                     "ChangeSurface: GetContext napi_get_cb_info failed");
    }
    bool lossless = true;
    int index = 0;
    if (napi_ok != napi_get_value_bigint_int64(env, args[index++], &surfaceId, &lossless)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get value failed");
    }
    char fontPath[256] = "";
    size_t fontPath_str_length;
    if (napi_ok != napi_get_value_string_utf8(env, args[index++], fontPath, sizeof(fontPath), &fontPath_str_length)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get width failed");
    }
    char content[256] = "";
    size_t content_str_length;
    if (napi_ok != napi_get_value_string_utf8(env, args[index++], content, sizeof(content), &content_str_length)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get height failed");
    }
    double fontHeight;
    if (napi_ok != napi_get_value_double(env, args[index++], &fontHeight)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "ChangeSurface: Get value failed");
    }
    
    auto pluginRender = GetPluginRender(surfaceId);
    if (pluginRender == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "DrawPattern: Get pluginRender failed");
        return nullptr;
    }
    pluginRender->DrawPattern(fontPath, content, fontHeight);
    return nullptr;
}

napi_value PluginManager::GetXComponentStatus(napi_env env, napi_callback_info info) {
    int64_t surfaceId = ParseId(env, info);
    auto pluginRender = GetPluginRender(surfaceId);
    if (pluginRender == nullptr) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "GetXComponentStatus: Get pluginRender failed");
        return nullptr;
    }
    napi_value hasDraw;
    napi_value hasChangeColor;
    napi_status ret = napi_create_int32(env, pluginRender->HasDraw(), &(hasDraw));
    if (ret != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG,
                     "GetXComponentStatus: napi_create_int32 hasDraw_ error");
        return nullptr;
    }
    ret = napi_create_int32(env, pluginRender->HasChangedColor(), &(hasChangeColor));
    if (ret != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG,
                     "GetXComponentStatus: napi_create_int32 hasChangeColor_ error");
        return nullptr;
    }
    napi_value obj;
    ret = napi_create_object(env, &obj);
    if (ret != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG, "GetXComponentStatus: napi_create_object error");
        return nullptr;
    }
    ret = napi_set_named_property(env, obj, "hasDraw", hasDraw);
    if (ret != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG,
                     "GetXComponentStatus: napi_set_named_property hasDraw error");
        return nullptr;
    }
    ret = napi_set_named_property(env, obj, "hasChangeColor", hasChangeColor);
    if (ret != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, PM_LOG_TAG,
                     "GetXComponentStatus: napi_set_named_property hasChangeColor error");
        return nullptr;
    }
    return obj;
}
} // namespace NativeXComponentSample
