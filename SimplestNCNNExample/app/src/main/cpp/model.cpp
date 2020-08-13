//
// Created by Administrator on 2020/8/13.
//

#include <vector>
#include "model.h"


int model::init(AAssetManager *mgr, const std::string crnn_param, const std::string crnn_bin)
{
    int ret1 = crnn.load_param(mgr, crnn_param.c_str());
    int ret2 = crnn.load_model(mgr, crnn_bin.c_str());
    LOGI("ret1 is %d, ret2 is %d", ret1, ret2);
    return (ret1||ret2);
}


int model::forward(const cv::Mat image, std::string &result){
    ncnn::Mat in = ncnn::Mat::from_pixels(image.data, ncnn::Mat::PIXEL_BGR2GRAY, image.cols, image.rows);
    in.substract_mean_normalize(mean_vals_crnn, norm_vals_crnn);
    LOGI("input size : %d, %d, %d", in.w, in.h, in.c);
    ncnn::Extractor ex = crnn.create_extractor();
    ex.input("input",in);
    ncnn::Mat preds;
    ex.extract("out",preds);
    LOGI("output size : %d, %d, %d", preds.w, preds.h, preds.c);
    decode(preds, alphabetChinese, result);
    return 0;
}


int model::decode(const ncnn::Mat score , const std::string alphabetChinese, std::string &result) {
    float *srcdata = (float* ) score.data;
    int last_index = 0;
    std::vector<std::string> res_str;
    for (int i = 0; i < score.h;i++){
        int max_index = 0;
        float max_value = -1000;
        for (int j =0; j< score.w; j++){
            if (srcdata[ i * score.w + j ] > max_value){
                max_value = srcdata[i * score.w + j ];
                max_index = j;
            }
        }

        if (max_index >0 && (not (i>0 && max_index == last_index))  ){
            std::string temp_str =  utf8_substr2(alphabetChinese,max_index-1,1)  ;
            res_str.push_back(temp_str);
            LOGI("temp_str %s", temp_str.c_str());
        }

        last_index = max_index;
    }
    result = "";
    for(const auto &st:res_str) result += st;
    LOGI("result %s", result.c_str());
    return 0;
}


std::string model::utf8_substr2(const std::string &str,int start, int length)
{
    int i,ix,j,realstart,reallength;
    if (length==0) return "";
    if (start<0 || length <0)
    {
        //find j=utf8_strlen(str);
        for(j=0,i=0,ix=str.length(); i<ix; i+=1, j++)
        {
            unsigned char c= str[i];
            if      (c>=0   && c<=127) i+=0;
            else if (c>=192 && c<=223) i+=1;
            else if (c>=224 && c<=239) i+=2;
            else if (c>=240 && c<=247) i+=3;
            else if (c>=248 && c<=255) return "";//invalid utf8
        }
        if (length !=INT_MAX && j+length-start<=0) return "";
        if (start  < 0 ) start+=j;
        if (length < 0 ) length=j+length-start;
    }

    j=0,realstart=0,reallength=0;
    for(i=0,ix=str.length(); i<ix; i+=1, j++)
    {
        if (j==start) { realstart=i; }
        if (j>=start && (length==INT_MAX || j<=start+length)) { reallength=i-realstart; }
        unsigned char c= str[i];
        if      (c>=0   && c<=127) i+=0;
        else if (c>=192 && c<=223) i+=1;
        else if (c>=224 && c<=239) i+=2;
        else if (c>=240 && c<=247) i+=3;
        else if (c>=248 && c<=255) return "";//invalid utf8
    }
    if (j==start) { realstart=i; }
    if (j>=start && (length==INT_MAX || j<=start+length)) { reallength=i-realstart; }

    return str.substr(realstart,reallength);
}

