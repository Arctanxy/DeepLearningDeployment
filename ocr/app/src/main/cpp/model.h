//
// Created by Administrator on 2020/11/8.
//

#ifndef OCR_MODEL_H
#define OCR_MODEL_H


#include <string>
#include "net.h"
#include <android/asset_manager.h>

class model {
public:
    int init(AAssetManager *mgr);
    model();
    ~model();
    std::string alphabetChinese;
private:
    ncnn::Net detnet;
    ncnn::Net anglenet;
    ncnn::Net recnet;
};


#endif //OCR_MODEL_H
