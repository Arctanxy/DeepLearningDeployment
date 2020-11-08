#include <jni.h>
#include <string>

#include "model.h"
#include <android/asset_manager_jni.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ocr_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */, jobject assetManager) {
    std::string hello = "Hello from C++";
    model net;
    static AAssetManager *mgr = nullptr;
    mgr = AAssetManager_fromJava(env, assetManager);
    int ret =  net.init(mgr);
    if(ret)
    {
        hello = "init failed ";
    }else{
        hello = "init sucessed";
    }
    return env->NewStringUTF(hello.c_str());
}