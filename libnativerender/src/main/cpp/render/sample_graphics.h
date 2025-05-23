/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SAMPLE_GRAPHICS_H
#define SAMPLE_GRAPHICS_H
#include "napi/native_api.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <ace/xcomponent/native_interface_xcomponent.h>
#include <cstdint>
#include <native_drawing/drawing_bitmap.h>
#include <native_drawing/drawing_brush.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_color.h>
#include <native_drawing/drawing_path.h>
#include <native_drawing/drawing_pen.h>
#include <native_window/external_window.h>
#include <string>
#include <unordered_map>


class SampleGraphics {
public:
    SampleGraphics() = default;
    ~SampleGraphics();
    explicit SampleGraphics(std::int64_t id) : id_(id) { InitializeEglContext(); }
    static napi_value Draw(napi_env env, napi_callback_info info);
    static void Release(std::int64_t &id);
    void SetWidth(uint64_t width);
    void SetHeight(uint64_t height);
    void SetNativeWindow(OHNativeWindow *nativeWindow);
    
    void DrawMyTest(OH_Drawing_Canvas *canvas, const char* fontPath, const char * content, double fontHeight);
    // 创建画布及绘图结果显示
    void Prepare();
    void Create();
    void CreateByCPU();
    void CreateByGPU();
    void DisPlay();
    void DisPlayCPU();
    void DisPlayGPU();

    void Export(napi_env env, napi_value exports);
    void RegisterCallback(OH_NativeXComponent *nativeXComponent);
    void Destroy();
    static SampleGraphics *GetInstance(std::int64_t &id);
    std::int64_t id_;

private:
    std::unordered_map<std::string, void (SampleGraphics::*)(OH_Drawing_Canvas *)> drawFunctionMap_;
    void DoRender(SampleGraphics *render, char *canvasType, char *shapeType);
    int32_t InitializeEglContext();
    void DeInitializeEglContext();
    OH_NativeXComponent_Callback renderCallback_;

    uint64_t width_ = 0;
    uint64_t height_ = 0;
    
    const uint16_t rgbaMin_ = 0x00;
    const uint16_t rgbaMax_ = 0xFF;
    const uint16_t rgbaHalf_ = 0x80;

    static float value100_;
    static float value150_;
    static float value200_;
    static float value300_;
    static float value400_;
    static float value500_;
    static float value551_;
    static float value600_;
    static float value630_;
    static float value700_;
    static float value800_;
    static float value900_;
    static float value1000_;
    static float value1200_;
    bool desc = false;

    EGLDisplay EGLDisplay_ = EGL_NO_DISPLAY;
    EGLConfig EGLConfig_ = nullptr;
    EGLContext EGLContext_ = EGL_NO_CONTEXT;
    EGLSurface EGLSurface_ = nullptr;


    OHNativeWindow *nativeWindow_ = nullptr;
    uint32_t *mappedAddr_ = nullptr;
    BufferHandle *bufferHandle_ = nullptr;
    struct NativeWindowBuffer *buffer_ = nullptr;
    int fenceFd_ = 0;

public:
    OH_Drawing_Bitmap *cScreenBitmap_ = nullptr;
    OH_Drawing_Canvas *cScreenCanvas_ = nullptr;
    OH_Drawing_Bitmap *cOffScreenBitmap_ = nullptr;
    OH_Drawing_Canvas *cCPUCanvas_ = nullptr;
    OH_Drawing_Canvas *cGPUCanvas_ = nullptr;
};

#endif // SAMPLE_GRAPHICS_H
