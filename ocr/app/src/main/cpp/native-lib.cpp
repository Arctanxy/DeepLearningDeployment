#include <jni.h>
#include <string>

#include "ocr.h"
#include <android/asset_manager_jni.h>

OCR net;

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


//extern "C" JNIEXPORT jstring JNICALL
//Java_com_example_ocr_MainActivity_stringFromJNI(
//        JNIEnv* env,
//        jobject /* this */, jobject assetManager, jobject bitmap) {
//    std::string hello = "Hello from C++";
//    OCR net;
//    static AAssetManager *mgr = nullptr;
//    mgr = AAssetManager_fromJava(env, assetManager);
//    int ret =  net.init(mgr);
//    if(ret)
//    {
//        hello = "init failed ";
//    }else{
//        hello = "init sucessed";
//    }
//    LOGI("######%s\n", hello.c_str());
//    cv::Mat img_bgr = convertToMat(env, bitmap);
//    net.detect(img_bgr, 640);
//
//    return env->NewStringUTF(hello.c_str());
//}

extern "C" JNIEXPORT jint JNICALL
        Java_com_example_ocr_MainActivity_initModel(
        JNIEnv * env, jobject, jobject assetManager
        )
{
    static AAssetManager *mgr = nullptr;
    mgr = AAssetManager_fromJava(env, assetManager);
    int ret = net.init(mgr);
    return ret;
}

extern "C" JNIEXPORT jstring JNICALL
        Java_com_example_ocr_MainActivity_detect(
        JNIEnv* env,
        jobject /* this */, jobject bitmap) {
    cv::Mat img_bgr = convertToMat(env, bitmap);
    std::vector<std::string> res = net.detect(img_bgr, 640);
    std::string result = "";
    int i = 0;
//    for(auto a:res)
//    {
//        LOGI("%s", a.c_str());
//    }
    while(i < res.size())
    {

//        处理边框坐标
        if(res[i] == "*")
        {
//            result += "[";
            result += res[i + 1];
            result += ",";
            result += res[i + 2];
            result += ",";
            result += res[i + 3];
            result += ",";
            result += res[i + 4];
//            result += "]";
            result += ",";
            i += 5;
            continue;
        }
        result += res[i];
        i += 1;
//        处理文本
//            文本留到java端处理
//        if(res[i] == "#")
//        {
//            result += ""
//        }
    }
//    for(auto s:res)
//    {
////        LOGI("%s", s.c_str());
//        if(s == "*")
//        {
//            result += s;
//            result += ",";
//        }
//    }
//    std::string hello = "hello ";
    return env->NewStringUTF(result.c_str());
}
