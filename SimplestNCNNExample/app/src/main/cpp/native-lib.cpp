#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include "model.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

model *ocr = new model();
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_simplestncnnexample_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */,jobject assetManager, jobject bitmap) {

    LOGI("loading assetmanager");
    static AAssetManager * mgr = NULL;
    mgr = AAssetManager_fromJava( env, assetManager);

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

    LOGI("loading model");
    std::string crnn_param = "crnn_lite_dw_dense.param";
    std::string crnn_bin = "crnn_lite_dw_dense.bin";
    int ret = ocr->init(mgr, crnn_param, crnn_bin);
    std::string result;
    if(ret){
        result = "Model loading failed";
        return env->NewStringUTF(result.c_str());
    }
    LOGI("running model");
    int r = ocr->forward(img_bgr, result);
    LOGI("final result %s", result.c_str());
    return env->NewStringUTF(result.c_str());
}
