//
// Created by Administrator on 2020/11/8.
//

#include "model.h"
#include "utils.h"

#define CRNN_LSTM


model::model()
{}

int model::init(AAssetManager *mgr)
{
    int ret1, ret2, ret3, ret4, ret5, ret6;
    ret1 = this->detnet.load_param(mgr, "crnn_lite_dw_dense.param");
    ret2 = this->detnet.load_model(mgr, "crnn_lite_dw_dense.bin");
    ret3 = this->anglenet.load_param(mgr, "shufflenetv2_05_angle.param");
    ret4 = this->anglenet.load_model(mgr, "shufflenetv2_05_angle.bin");
#ifdef CRNN_LSTM
    ret5 = this->recnet.load_param(mgr, "crnn_lite_lstm_v2.param");
    ret6 = this->recnet.load_model(mgr, "crnn_lite_lstm_v2.bin");
#elif
    ret5 = this->recnet.load_param(mgr, "crnn_lite_dw_dense.param");
    ret6 = this->recnet.load_model(mgr, "crnn_lite_dw_dense.bin");
#endif

    char * buffer = readKeysFromAssets(mgr);
    alphabetChinese = buffer;
    if(buffer == nullptr)
    {
        return -1;
    }else{
        LOGI("%s",alphabetChinese.c_str());
    }
    return ret1 || ret2 || ret3 ||ret4 ||ret5 ||ret6;
}

model::~model()
{
}


