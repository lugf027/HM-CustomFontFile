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

#include "sample_graphics.h"
#include "common/log_common.h"
#include "utils/adaptation_util.h"
#include <cstdint>
#include <map>
#include <multimedia/image_framework/image/pixelmap_native.h>
#include <native_display_soloist/native_display_soloist.h>
#include <native_drawing/drawing_color_filter.h>
#include <native_drawing/drawing_filter.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_drawing/drawing_gpu_context.h>
#include <native_drawing/drawing_image_filter.h>
#include <native_drawing/drawing_mask_filter.h>
#include <native_drawing/drawing_matrix.h>
#include <native_drawing/drawing_path_effect.h>
#include <native_drawing/drawing_pixel_map.h>
#include <native_drawing/drawing_point.h>
#include <native_drawing/drawing_rect.h>
#include <native_drawing/drawing_region.h>
#include <native_drawing/drawing_register_font.h>
#include <native_drawing/drawing_round_rect.h>
#include <native_drawing/drawing_sampling_options.h>
#include <native_drawing/drawing_shader_effect.h>
#include <native_drawing/drawing_surface.h>
#include <native_drawing/drawing_text_blob.h>
#include <native_drawing/drawing_text_typography.h>
#include <sys/mman.h>

#include <native_drawing/drawing_gpu_context.h>
#include <native_drawing/drawing_surface.h>

const uint16_t RGBA_MIN = 0x00;
const uint16_t RGBA_MAX = 0xFF;
const uint16_t RGBA_SIZE = 4;

static std::unordered_map<std::string, OH_DisplaySoloist *> g_displaySync;
// 多设备适配工具类,按设备宽度等比例缩放适配
AdaptationUtil *adaptationUtil = AdaptationUtil::GetInstance();
float SampleGraphics::value100_ = adaptationUtil->GetWidth(100.f);
float SampleGraphics::value150_ = adaptationUtil->GetWidth(150.f);
float SampleGraphics::value200_ = adaptationUtil->GetWidth(200.f);
float SampleGraphics::value300_ = adaptationUtil->GetWidth(300.f);
float SampleGraphics::value400_ = adaptationUtil->GetWidth(400.f);
float SampleGraphics::value500_ = adaptationUtil->GetWidth(500.f);
float SampleGraphics::value551_ = adaptationUtil->GetWidth(551.f);
float SampleGraphics::value600_ = adaptationUtil->GetWidth(600.f);
float SampleGraphics::value630_ = adaptationUtil->GetWidth(630.f);
float SampleGraphics::value700_ = adaptationUtil->GetWidth(700.f);
float SampleGraphics::value800_ = adaptationUtil->GetWidth(800.f);
float SampleGraphics::value900_ = adaptationUtil->GetWidth(900.f);
float SampleGraphics::value1000_ = adaptationUtil->GetWidth(1000.f);
float SampleGraphics::value1200_ = adaptationUtil->GetWidth(1200.f);

EGLConfig getConfig(int version, EGLDisplay eglDisplay) {
    int attribList[] = {EGL_SURFACE_TYPE,
                        EGL_WINDOW_BIT,
                        EGL_RED_SIZE,
                        8,
                        EGL_GREEN_SIZE,
                        8,
                        EGL_BLUE_SIZE,
                        8,
                        EGL_ALPHA_SIZE,
                        8,
                        EGL_RENDERABLE_TYPE,
                        EGL_OPENGL_ES2_BIT,
                        EGL_NONE};
    EGLConfig configs = NULL;
    int configsNum;

    if (!eglChooseConfig(eglDisplay, attribList, &configs, 1, &configsNum)) {
        SAMPLE_LOGE("eglChooseConfig ERROR");
        return NULL;
    }

    return configs;
}

int32_t SampleGraphics::InitializeEglContext() {
    EGLDisplay_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGLDisplay_ == EGL_NO_DISPLAY) {
        SAMPLE_LOGE("unable to get EGL display.");
        return -1;
    }

    EGLint eglMajVers;
    EGLint eglMinVers;
    if (!eglInitialize(EGLDisplay_, &eglMajVers, &eglMinVers)) {
        EGLDisplay_ = EGL_NO_DISPLAY;
        SAMPLE_LOGE("unable to initialize display");
        return -1;
    }

    int version = 3;
    EGLConfig_ = getConfig(version, EGLDisplay_);
    if (EGLConfig_ == nullptr) {
        SAMPLE_LOGE("GLContextInit config ERROR");
        return -1;
    }

    /* Create EGLContext from */
    int attribList[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE}; // 2 size

    EGLContext_ = eglCreateContext(EGLDisplay_, EGLConfig_, EGL_NO_CONTEXT, attribList);

    EGLint attribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
    EGLSurface_ = eglCreatePbufferSurface(EGLDisplay_, EGLConfig_, attribs);
    if (!eglMakeCurrent(EGLDisplay_, EGLSurface_, EGLSurface_, EGLContext_)) {
        SAMPLE_LOGE("eglMakeCurrent error = %{public}d", eglGetError());
        return -1;
    }

    SAMPLE_LOGE("Demo Init success.");
    return 0;
}

void SampleGraphics::DeInitializeEglContext() {
    EGLBoolean ret = eglDestroySurface(EGLDisplay_, EGLSurface_);
    if (!ret) {
        SAMPLE_LOGE("eglDestroySurface failure.");
    }

    ret = eglDestroyContext(EGLDisplay_, EGLContext_);
    if (!ret) {
        SAMPLE_LOGE("eglDestroyContext failure.");
    }

    ret = eglTerminate(EGLDisplay_);
    if (!ret) {
        SAMPLE_LOGE("eglTerminate failure.");
    }

    EGLSurface_ = NULL;
    EGLContext_ = NULL;
    EGLDisplay_ = NULL;
}

static void OnSurfaceCreatedCB(OH_NativeXComponent *component, void *window) {
    SAMPLE_LOGI("OnSurfaceCreatedCB");
    if ((component == nullptr) || (window == nullptr)) {
        SAMPLE_LOGE("OnSurfaceCreatedCB: component or window is null");
        return;
    }
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        SAMPLE_LOGE("OnSurfaceCreatedCB: Unable to get XComponent id");
        return;
    }
//    std::string id(idStr);
//    auto render = SampleGraphics::GetInstance(id);
//    OHNativeWindow *nativeWindow = static_cast<OHNativeWindow *>(window);
//    render->SetNativeWindow(nativeWindow);
//
//    uint64_t width;
//    uint64_t height;
//    int32_t xSize = OH_NativeXComponent_GetXComponentSize(component, window, &width, &height);
//    if ((xSize == OH_NATIVEXCOMPONENT_RESULT_SUCCESS) && (render != nullptr)) {
//        render->SetHeight(height);
//        render->SetWidth(width);
//        SAMPLE_LOGI("xComponent width = %{public}llu, height = %{public}llu", width, height);
//    }
}

static void OnSurfaceDestroyedCB(OH_NativeXComponent *component, void *window) {
    SAMPLE_LOGI("OnSurfaceDestroyedCB");
    if ((component == nullptr) || (window == nullptr)) {
        SAMPLE_LOGE("OnSurfaceDestroyedCB: component or window is null");
        return;
    }
    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(component, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        SAMPLE_LOGE("OnSurfaceDestroyedCB: Unable to get XComponent id");
        return;
    }
//    std::int64_t id(idStr);
//    SampleGraphics::Release(id);
}

static std::unordered_map<std::int64_t, SampleGraphics *> g_instance;

void SampleGraphics::SetWidth(uint64_t width) { width_ = width; }

void SampleGraphics::SetHeight(uint64_t height) { height_ = height; }

void SampleGraphics::SetNativeWindow(OHNativeWindow *nativeWindow) { nativeWindow_ = nativeWindow; }

void SampleGraphics::Prepare() {
    if (nativeWindow_ == nullptr) {
        SAMPLE_LOGE("nativeWindow_ is nullptr");
        return;
    }
    // 这里的nativeWindow是从上一步骤中的回调函数中获得的
    // 通过 OH_NativeWindow_NativeWindowRequestBuffer 获取 OHNativeWindowBuffer 实例
    int ret = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow_, &buffer_, &fenceFd_);
    SAMPLE_LOGI("request buffer ret = %{public}d", ret);
    // 通过 OH_NativeWindow_GetBufferHandleFromNative 获取 buffer 的 handle
    bufferHandle_ = OH_NativeWindow_GetBufferHandleFromNative(buffer_);
    // 使用系统mmap接口拿到bufferHandle的内存虚拟地址
    mappedAddr_ = static_cast<uint32_t *>(
        mmap(bufferHandle_->virAddr, bufferHandle_->size, PROT_READ | PROT_WRITE, MAP_SHARED, bufferHandle_->fd, 0));
    if (mappedAddr_ == MAP_FAILED) {
        SAMPLE_LOGE("mmap failed");
    }
}

void SampleGraphics::Create() {
    uint32_t width = static_cast<uint32_t>(bufferHandle_->stride / 4);
    // 创建一个bitmap对象
    cScreenBitmap_ = OH_Drawing_BitmapCreate();
    // 定义bitmap的像素格式
    OH_Drawing_BitmapFormat cFormat{COLOR_FORMAT_RGBA_8888, ALPHA_FORMAT_OPAQUE};
    // 构造对应格式的bitmap
    OH_Drawing_BitmapBuild(cScreenBitmap_, width, height_, &cFormat);

    // 创建一个canvas对象
    cScreenCanvas_ = OH_Drawing_CanvasCreate();
    // 将画布与bitmap绑定，画布画的内容会输出到绑定的bitmap内存中
    OH_Drawing_CanvasBind(cScreenCanvas_, cScreenBitmap_);
    // 使用白色清除画布内容
    OH_Drawing_CanvasClear(cScreenCanvas_, OH_Drawing_ColorSetArgb(RGBA_MAX, RGBA_MAX, RGBA_MAX, 0xFF));
}

void SampleGraphics::CreateByCPU() {
    // 创建一个离屏位图对象
    cOffScreenBitmap_ = OH_Drawing_BitmapCreate();
    // 设置位图属性
    OH_Drawing_BitmapFormat cFormat{COLOR_FORMAT_RGBA_8888, ALPHA_FORMAT_PREMUL};
    // 设置位图长宽（按需设置）
    uint32_t width = 800;
    uint32_t height = 800;
    // 初始化位图
    OH_Drawing_BitmapBuild(cOffScreenBitmap_, width, height, &cFormat);
    // 创建一个离屏Canvas对象
    cCPUCanvas_ = OH_Drawing_CanvasCreate();
    // 将离屏Canvas与离屏bitmap绑定，离屏Canvas绘制的内容会输出到绑定的离屏bitmap内存中
    OH_Drawing_CanvasBind(cCPUCanvas_, cOffScreenBitmap_);
    // 将背景设置为白色
    OH_Drawing_CanvasClear(cCPUCanvas_, OH_Drawing_ColorSetArgb(RGBA_MAX, RGBA_MAX, RGBA_MAX, 0xFF));

    // 创建一个bitmap对象
    cScreenBitmap_ = OH_Drawing_BitmapCreate();
    // 构造对应格式的bitmap
    OH_Drawing_BitmapBuild(cScreenBitmap_, width_, height_, &cFormat);
    // 创建一个canvas对象
    cScreenCanvas_ = OH_Drawing_CanvasCreate();
    // 将Canvas与bitmap绑定，Canvas绘制的内容会输出到绑定的bitmap内存中
    OH_Drawing_CanvasBind(cScreenCanvas_, cScreenBitmap_);
}

void SampleGraphics::CreateByGPU() {
    // 创建一个bitmap对象
    cScreenBitmap_ = OH_Drawing_BitmapCreate();
    // 定义bitmap的像素格式
    OH_Drawing_BitmapFormat cFormat{COLOR_FORMAT_RGBA_8888, ALPHA_FORMAT_OPAQUE};
    // 构造对应格式的bitmap
    OH_Drawing_BitmapBuild(cScreenBitmap_, width_, height_, &cFormat);
    // 创建一个canvas对象
    cScreenCanvas_ = OH_Drawing_CanvasCreate();
    // 将画布与bitmap绑定，画布画的内容会输出到绑定的bitmap内存中
    OH_Drawing_CanvasBind(cScreenCanvas_, cScreenBitmap_);

    // 设置宽高（按需设定）
    int32_t cWidth = 800;
    int32_t cHeight = 800;
    // 设置图像，包括宽度、高度、颜色格式和透明度格式
    OH_Drawing_Image_Info imageInfo = {cWidth, cHeight, COLOR_FORMAT_RGBA_8888, ALPHA_FORMAT_PREMUL};
    // GPU上下文的选项
    OH_Drawing_GpuContextOptions options{true};

    // 创建一个使用OpenGL（GL）作为其GPU后端的绘图上下文
    OH_Drawing_GpuContext *gpuContext = OH_Drawing_GpuContextCreateFromGL(options);
    // 创建surface对象
    OH_Drawing_Surface *surface = OH_Drawing_SurfaceCreateFromGpuContext(gpuContext, true, imageInfo);
    // 创建一个canvas对象
    cGPUCanvas_ = OH_Drawing_SurfaceGetCanvas(surface);
    // 将背景设置为白色
    OH_Drawing_CanvasClear(cGPUCanvas_, OH_Drawing_ColorSetArgb(RGBA_MAX, RGBA_MAX, RGBA_MAX, 0xFF));
}

void SampleGraphics::DisPlay() {
    // 画完后获取像素地址，地址指向的内存包含画布画的像素数据
    void *bitmapAddr = OH_Drawing_BitmapGetPixels(cScreenBitmap_);
    uint32_t *value = static_cast<uint32_t *>(bitmapAddr);

    uint32_t *pixel = static_cast<uint32_t *>(mappedAddr_); // 使用mmap获取到的地址来访问内存
    if (pixel == nullptr) {
        SAMPLE_LOGE("pixel is null");
        return;
    }
    if (value == nullptr) {
        SAMPLE_LOGE("value is null");
        return;
    }
    for (uint32_t x = 0; x < width_; x++) {
        for (uint32_t y = 0; y < height_; y++) {
            *pixel++ = *value++;
        }
    }
    // 设置刷新区域，如果Region中的Rect为nullptr,或者rectNumber为0，则认为OHNativeWindowBuffer全部有内容更改。
    Region region{nullptr, 0};
    // 通过OH_NativeWindow_NativeWindowFlushBuffer 提交给消费者使用，例如：显示在屏幕上。
    OH_NativeWindow_NativeWindowFlushBuffer(nativeWindow_, buffer_, fenceFd_, region);
    // 内存使用完记得去掉内存映射
    int result = munmap(mappedAddr_, bufferHandle_->size);
    if (result == -1) {
        SAMPLE_LOGE("munmap failed!");
    }
}

void SampleGraphics::DisPlayCPU() {
    // 将离屏bitmap中的内容绘制到屏幕画布，实现上屏操作
    OH_Drawing_CanvasDrawBitmap(cScreenCanvas_, cOffScreenBitmap_, 0, 0);
    DisPlay();
}

void SampleGraphics::DisPlayGPU() {
    // 设置宽高（按需设定）
    int32_t cWidth = 800;
    int32_t cHeight = 800;
    // 设置图像，包括宽度、高度、颜色格式和透明度格式
    OH_Drawing_Image_Info imageInfo = {cWidth, cHeight, COLOR_FORMAT_RGBA_8888, ALPHA_FORMAT_PREMUL};
    void *dstPixels = malloc(cWidth * cHeight * RGBA_SIZE); // 4 for rgba
    OH_Drawing_CanvasReadPixels(cGPUCanvas_, &imageInfo, dstPixels, RGBA_SIZE * cWidth, 0, 0);
    OH_Drawing_Bitmap *cOffScreenBitmap_ = OH_Drawing_BitmapCreateFromPixels(&imageInfo, dstPixels, RGBA_SIZE * cWidth);
    OH_Drawing_CanvasDrawBitmap(cScreenCanvas_, cOffScreenBitmap_, 0, 0);
    DisPlay();
}

static void drawStringWithoutCustomFont(OH_Drawing_Canvas *canvas, const char *content, int x, int y) {
    OH_Drawing_Brush *brush = OH_Drawing_BrushCreate();
    OH_Drawing_BrushSetColor(brush, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
    OH_Drawing_CanvasAttachBrush(canvas, brush);
    OH_Drawing_Font *systemFont = OH_Drawing_FontCreate();
    OH_Drawing_FontSetTextSize(systemFont, 17);
    OH_Drawing_TextBlob *textBlob =
        OH_Drawing_TextBlobCreateFromString(content, systemFont, OH_Drawing_TextEncoding::TEXT_ENCODING_UTF8);
    OH_Drawing_CanvasDrawTextBlob(canvas, textBlob, x, y);
    OH_Drawing_TextBlobDestroy(textBlob);
    OH_Drawing_FontDestroy(systemFont);
    OH_Drawing_CanvasDetachBrush(canvas);
}

// 在类的实现部分添加这个静态方法
static std::string extractFontFamily(const char *fontFilePath) {
    std::string fontPath(fontFilePath);
    size_t lastSlash = fontPath.find_last_of("/\\");
    size_t lastDot = fontPath.find_last_of(".");
    std::string fontName = "latex-";
    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastSlash < lastDot) {
        fontName += fontPath.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }
    return fontName;
}

static void NativeOnDrawText(OH_Drawing_Canvas *canvas,  const char *fontPath, const char *content, double fontHeight, int x,int y) {
    std::string fontPathStr(fontPath);
    if (fontPathStr.length() <= 0) {
        drawStringWithoutCustomFont(canvas, content, x, y);
        return;
    }

    OH_Drawing_FontCollection *fontCollection = OH_Drawing_CreateFontCollection();
    const char *fontFamily = extractFontFamily(fontPath).c_str();
    int errorCode = OH_Drawing_RegisterFont(fontCollection, fontFamily, fontPath);
    const char *myFontFamilies[] = {fontFamily};

    // 选择从左到右/左对齐等排版属性
    OH_Drawing_TypographyStyle *typoStyle = OH_Drawing_CreateTypographyStyle();
    OH_Drawing_SetTypographyTextDirection(typoStyle, TEXT_DIRECTION_LTR);
    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_START);
    OH_Drawing_SetTypographyTextFontFamily(typoStyle, fontFamily);
    OH_Drawing_SetTypographyTextFontHeight(typoStyle, fontHeight);

    OH_Drawing_TextStyle *txtStyle = OH_Drawing_CreateTextStyle();
    OH_Drawing_SetTextStyleColor(txtStyle, OH_Drawing_ColorSetArgb(0xFF, 0x00, 0x00, 0x00));
    OH_Drawing_SetTextStyleFontSize(txtStyle, 17);
    OH_Drawing_SetTextStyleFontWeight(txtStyle, FONT_WEIGHT_100);
    // 修改基线设置为ALPHABETIC，这样可以保持与TextBlob一致的绘制位置
    OH_Drawing_SetTextStyleBaseLine(txtStyle, TEXT_BASELINE_ALPHABETIC);
    // 设置行高为1.0，避免额外的行间距影响
    OH_Drawing_SetTextStyleFontHeight(txtStyle, fontHeight); // 0.01 影响 baseline？
    // 设置字体类型等
    OH_Drawing_SetTextStyleFontFamilies(txtStyle, 1, myFontFamilies);
    OH_Drawing_SetTextStyleFontStyle(txtStyle, FONT_STYLE_NORMAL);
    OH_Drawing_SetTextStyleLocale(txtStyle, "en");
    OH_Drawing_TextStyleSetBaselineShift(txtStyle, 0.0);

    OH_Drawing_TypographyCreate *handler = OH_Drawing_CreateTypographyHandler(typoStyle, fontCollection);
    OH_Drawing_TypographyHandlerPushTextStyle(handler, txtStyle);
    // 设置文字内容
    OH_Drawing_TypographyHandlerAddText(handler, content);
    OH_Drawing_Typography *typography = OH_Drawing_CreateTypography(handler);
    OH_Drawing_TypographyLayout(typography, 10000);
    
    OH_Drawing_LineMetrics* lineInfo = OH_Drawing_TypographyGetLineMetrics(typography);
    SAMPLE_LOGI("OH_Drawing_LineMetricsl ascender=%{public}.2f descender=%{public}.2f capHeight=%{public}.2f height=%{public}.2f y=%{public}.2f", 
    lineInfo->ascender, lineInfo->descender, lineInfo->capHeight, lineInfo->height, lineInfo->y);
    
    OH_Drawing_Font_Metrics fontInfo = lineInfo->firstCharMetrics;
    SAMPLE_LOGI("OH_Drawing_Font_Metrics ascent=%{public}.2f descent=%{public}.2f top=%{public}.2f bottom=%{public}.2f leading=%{public}.2f underlinePosition=%{public}.2f", 
    fontInfo.ascent, fontInfo.descent, fontInfo.top, fontInfo.bottom, fontInfo.leading, fontInfo.underlinePosition);
    if (lineInfo->descender > lineInfo->ascender) {
        OH_Drawing_TypographyPaint(typography, canvas, x, y);
    } else {
        OH_Drawing_TypographyPaint(typography, canvas, x, y + fontInfo.ascent);
    }

    // 销毁创建的资源
    OH_Drawing_DestroyTypography(typography);
    OH_Drawing_DestroyTypographyHandler(handler);
    OH_Drawing_DestroyTextStyle(txtStyle);
    OH_Drawing_DestroyTypographyStyle(typoStyle);
    OH_Drawing_DestroyFontCollection(fontCollection);
}

static void NativeOnDrawLine(OH_Drawing_Canvas *canvas, int x, int y) {
    // 绘制y所在的线
    OH_Drawing_Pen *pen = OH_Drawing_PenCreate();
    OH_Drawing_PenSetColor(pen, OH_Drawing_ColorSetArgb(0xFF, 0xFF, 0x00, 0x00));
    OH_Drawing_PenSetWidth(pen, 0.3f);
    OH_Drawing_CanvasAttachPen(canvas, pen);

    OH_Drawing_CanvasDrawLine(canvas, x, y, x + 200, y);

    OH_Drawing_PenDestroy(pen);
    OH_Drawing_CanvasDetachPen(canvas);
}

void SampleGraphics::DrawMyTest(OH_Drawing_Canvas *canvas, const char *fontPath, const char *content,
                                double fontHeight) {
    SAMPLE_LOGI("DrawMyTest");
    // 先放大十倍(模拟实际场景需要)
    OH_Drawing_Matrix *matrix = OH_Drawing_MatrixCreateScale(10, 10, 0, 0);
    OH_Drawing_CanvasConcatMatrix(canvas, matrix);
    OH_Drawing_MatrixDestroy(matrix);

    int x = 0;
    int y = 10;
    NativeOnDrawText(canvas, fontPath, content, fontHeight, x, y);
    NativeOnDrawLine(canvas, x, y);
}

void SampleGraphics::DoRender(SampleGraphics *render, char *canvasType, char *shapeType) {
    SAMPLE_LOGI("DoRender");
    render->Prepare();
    // 不同画布
    if (strcmp(canvasType, "CanvasGet") == 0) {
        SAMPLE_LOGI("CanvasGet");
        render->Create();
        // 绘制图案
        auto it = render->drawFunctionMap_.find(shapeType);
        if (it != render->drawFunctionMap_.end()) {
            (render->*(it->second))(render->cScreenCanvas_);
        } else {
            SAMPLE_LOGI("Unsupported shape type: %{public}s", shapeType);
        }
        render->DisPlay();
    } else if (strcmp(canvasType, "CanvasGetByCPU") == 0) {
        render->CreateByCPU();
        // 绘制图案
        auto it = render->drawFunctionMap_.find(shapeType);
        if (it != render->drawFunctionMap_.end()) {
            (render->*(it->second))(render->cCPUCanvas_);
        } else {
            SAMPLE_LOGI("Unsupported shape type: %{public}s", shapeType);
        }
        render->DisPlayCPU();
    } else if (strcmp(canvasType, "CanvasGetByGPU") == 0) {
        render->CreateByGPU();
        // 绘制图案
        auto it = render->drawFunctionMap_.find(shapeType);
        if (it != render->drawFunctionMap_.end()) {
            (render->*(it->second))(render->cGPUCanvas_);
        } else {
            SAMPLE_LOGI("Unsupported shape type: %{public}s", shapeType);
        }
        render->DisPlayGPU();
    }
    render->Destroy();
}

napi_value SampleGraphics::Draw(napi_env env, napi_callback_info info) {
    SAMPLE_LOGI("Draw");
    if ((env == nullptr) || (info == nullptr)) {
        SAMPLE_LOGE("NapiRegister: env or info is null");
        return nullptr;
    }

    napi_value thisArg;
    size_t argc = 2;
    napi_value args[2];
    if (napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr) != napi_ok) {
        SAMPLE_LOGE("NapiRegister: napi_get_cb_info fail");
        return nullptr;
    }

    napi_value exportInstance;
    if (napi_get_named_property(env, thisArg, OH_NATIVE_XCOMPONENT_OBJ, &exportInstance) != napi_ok) {
        SAMPLE_LOGE("NapiRegister: napi_get_named_property fail");
        return nullptr;
    }

    OH_NativeXComponent *nativeXComponent = nullptr;
    if (napi_unwrap(env, exportInstance, reinterpret_cast<void **>(&nativeXComponent)) != napi_ok) {
        SAMPLE_LOGE("NapiRegister: napi_unwrap fail");
        return nullptr;
    }

    char idStr[OH_XCOMPONENT_ID_LEN_MAX + 1] = {'\0'};
    uint64_t idSize = OH_XCOMPONENT_ID_LEN_MAX + 1;
    if (OH_NativeXComponent_GetXComponentId(nativeXComponent, idStr, &idSize) != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        SAMPLE_LOGE("NapiRegister: Unable to get XComponent id");
        return nullptr;
    }
    SAMPLE_LOGI("RegisterID = %{public}s", idStr);
    std::string id(idStr);

    // 获取形状参数
    char canvasType[32] = {0};
    napi_get_value_string_utf8(env, args[0], canvasType, sizeof(canvasType), nullptr);

    // 获取形状参数
    char shapeType[32] = {0};
    napi_get_value_string_utf8(env, args[1], shapeType, sizeof(shapeType), nullptr);
    
//    SampleGraphics *render = SampleGraphics().GetInstance(id);
//    if (render != nullptr) {
//        render->DoRender(render, canvasType, shapeType);
//    }
    return nullptr;
}

SampleGraphics::~SampleGraphics() {
    // 销毁canvas对象
    OH_Drawing_CanvasDestroy(cScreenCanvas_);
    cScreenCanvas_ = nullptr;
    OH_Drawing_CanvasDestroy(cCPUCanvas_);
    cCPUCanvas_ = nullptr;
    // 销毁bitmap对象
    OH_Drawing_BitmapDestroy(cScreenBitmap_);
    cScreenBitmap_ = nullptr;
    OH_Drawing_BitmapDestroy(cOffScreenBitmap_);
    cOffScreenBitmap_ = nullptr;

    buffer_ = nullptr;
    bufferHandle_ = nullptr;
    nativeWindow_ = nullptr;
    mappedAddr_ = nullptr;
    DeInitializeEglContext();
}

void SampleGraphics::Destroy() {
    // 销毁canvas对象
    OH_Drawing_CanvasDestroy(cScreenCanvas_);
    cScreenCanvas_ = nullptr;
    OH_Drawing_CanvasDestroy(cCPUCanvas_);
    cCPUCanvas_ = nullptr;
    // 销毁bitmap对象
    OH_Drawing_BitmapDestroy(cScreenBitmap_);
    cScreenBitmap_ = nullptr;
    OH_Drawing_BitmapDestroy(cOffScreenBitmap_);
    cOffScreenBitmap_ = nullptr;
    DeInitializeEglContext();
}

void SampleGraphics::Release(std::int64_t &id) {
    SampleGraphics *render = SampleGraphics::GetInstance(id);
    if (render != nullptr) {
        delete render;
        render = nullptr;
        g_instance.erase(g_instance.find(id));
    }
}

void SampleGraphics::Export(napi_env env, napi_value exports) {
    if ((env == nullptr) || (exports == nullptr)) {
        SAMPLE_LOGE("Export: env or exports is null");
        return;
    }
    napi_property_descriptor desc[] = {
        {"draw", nullptr, SampleGraphics::Draw, nullptr, nullptr, nullptr, napi_default, nullptr}};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    if (napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc) != napi_ok) {
        SAMPLE_LOGE("Export: napi_define_properties failed");
    }
}

void SampleGraphics::RegisterCallback(OH_NativeXComponent *nativeXComponent) {
    SAMPLE_LOGI("register callback");
    renderCallback_.OnSurfaceCreated = OnSurfaceCreatedCB;
    renderCallback_.OnSurfaceDestroyed = OnSurfaceDestroyedCB;
    // Callback must be initialized
    renderCallback_.DispatchTouchEvent = nullptr;
    renderCallback_.OnSurfaceChanged = nullptr;
    OH_NativeXComponent_RegisterCallback(nativeXComponent, &renderCallback_);
}


SampleGraphics *SampleGraphics::GetInstance(std::int64_t &id) {
    if (g_instance.find(id) == g_instance.end()) {
        SampleGraphics *render = new SampleGraphics(id);
        g_instance[id] = render;
        return render;
    } else {
        return g_instance[id];
    }
}