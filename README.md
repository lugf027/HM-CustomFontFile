# CustomFontFile Unexpected BaseLine Demo

#### 1 录屏表现:

<video id="video" controls=""src="./assets/video_demo.mp4" preload="none" style="zoom:50%;">



#### 2 Native Render 代码见 `sample_graphics.cpp`

函数位置：`libnativerender/src/main/cpp/render/sample_graphics.cpp:434` `SampleGraphics::DrawMyTest`

![image-20250523140631312](./assets/image-20250523140631312.png)

```c++
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
    OH_Drawing_TypographyPaint(typography, canvas, x, y);

    // 销毁创建的资源
    OH_Drawing_DestroyTypography(typography);
    OH_Drawing_DestroyTypographyHandler(handler);
    OH_Drawing_DestroyTextStyle(txtStyle);
    OH_Drawing_DestroyTypographyStyle(typoStyle);
    OH_Drawing_DestroyFontCollection(fontCollection);
}
```



* 3 .ttf文件在这个压缩包里

![image-20250523140354132](./assets/image-20250523140354132.png)