//
// Created by Administrator on 2020/11/9.
//

#ifndef OCR_UTILS_H
#define OCR_UTILS_H

#include <android/log.h>
#include <android/asset_manager.h>

#define TAG "OcrLite"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)



char *readKeysFromAssets(AAssetManager *mgr) {
    //LOGI("readKeysFromAssets start...");
    if (mgr == NULL) {
        LOGE(" %s", "AAssetManager==NULL");
        return NULL;
    }
    char *buffer;
    /*获取文件名并打开*/
    AAsset *asset = AAssetManager_open(mgr, "keys.txt", AASSET_MODE_UNKNOWN);
    if (asset == NULL) {
        LOGE(" %s", "asset==NULL");
        return NULL;
    }
    /*获取文件大小*/
    off_t bufferSize = AAsset_getLength(asset);
    //LOGI("file size : %d", bufferSize);
    buffer = (char *) malloc(bufferSize + 1);
    buffer[bufferSize] = 0;
    int numBytesRead = AAsset_read(asset, buffer, bufferSize);
    //LOGI("readKeysFromAssets: %d", numBytesRead);
    /*关闭文件*/
    AAsset_close(asset);
    //LOGI("readKeysFromAssets exit...");
    return buffer;
}

#endif //OCR_UTILS_H
