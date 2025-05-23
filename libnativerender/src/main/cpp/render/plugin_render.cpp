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

#include "plugin_render.h"
#include <cstdint>
#include <js_native_api_types.h>

namespace NativeXComponentSample {

PluginRender::PluginRender(int64_t &id) {
    this->id_ = id;
    this->sampleGraphics_ = new SampleGraphics(id);
    hasDraw_ = 0;
    hasChangeColor_ = 0;
}

void PluginRender::DrawPattern(const char* fontPath, const char * content, double fontHeight) {
//    eglCore_->Draw(hasDraw_);
    this->sampleGraphics_->Prepare();
    this->sampleGraphics_->Create();
    this->sampleGraphics_->DrawMyTest(this->sampleGraphics_->cScreenCanvas_, fontPath, content, fontHeight);
    this->sampleGraphics_->DisPlay();
}

void PluginRender::InitNativeWindow(OHNativeWindow *window) { this->sampleGraphics_->SetNativeWindow(window); }

void PluginRender::UpdateNativeWindowSize(int width, int height) {
    this->sampleGraphics_->SetWidth(width);
    this->sampleGraphics_->SetHeight(height);
}

int32_t PluginRender::HasDraw() { return hasDraw_; }

int32_t PluginRender::HasChangedColor() { return hasChangeColor_; }
} // namespace NativeXComponentSample
