#include <jni.h>
#include <string>

#include "ocr.h"
#include <android/asset_manager_jni.h>

//todo: cv::Mat convertToMat(jobject bitmap)

cv::Mat convertToMat(JNIEnv* env,jobject bitmap)
{
    LOGI("convert bitmap to cv::Mat");
    // convert bitmap to mat
    int *data = NULL;
    AndroidBitmapInfo info = {0};
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, (void **) &data);

    // 检查图片格式
    if(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888)
    {
        LOGI("info format is RGBA");
    } else if(info.format == ANDROID_BITMAP_FORMAT_RGB_565)
    {
        LOGI("info format is RGB");
    }else{
        LOGI("Unknown format");
    }

    cv::Mat test(info.height, info.width, CV_8UC4, (char*)data); // RGBA
    cv::Mat img_bgr;
    cvtColor(test, img_bgr, CV_RGBA2BGR);
    return img_bgr;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ocr_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */, jobject assetManager, jobject bitmap) {
    std::string hello = "Hello from C++";
    OCR net;
    static AAssetManager *mgr = nullptr;
    mgr = AAssetManager_fromJava(env, assetManager);
    int ret =  net.init(mgr);
    cv::Mat img_bgr = convertToMat(env, bitmap);
    net.detect(img_bgr, 640);
    if(ret)
    {
        hello = "init failed ";
    }else{
        hello = "init sucessed";
    }
    return env->NewStringUTF(hello.c_str());
}