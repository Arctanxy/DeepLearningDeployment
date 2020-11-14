#include "ocr.h"
#include <queue>

#define CRNN_LSTM 0


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

OCR::OCR()
{}

int OCR::init(AAssetManager *mgr)
{
    int ret1, ret2, ret3, ret4, ret5, ret6, ret7, ret8;
    ret1 = this->psenet.load_param(mgr, "psenet_lite_mbv2.param");
    ret2 = this->psenet.load_model(mgr, "psenet_lite_mbv2.bin");
    ret3 = this->angle_net.load_param(mgr, "shufflenetv2_05_angle.param");
    ret4 = this->angle_net.load_model(mgr, "shufflenetv2_05_angle.bin");
#if CRNN_LSTM
    ret5 = this->crnn_net.load_param(mgr, "crnn_lite_lstm_v2.param");
    ret6 = this->crnn_net.load_model(mgr, "crnn_lite_lstm_v2.bin");
    ret7 = crnn_vertical_net.load_param(mgr, "crnn_lite_lstm_vertical.param");
    ret8 = crnn_vertical_net.load_model(mgr, "crnn_lite_lstm_vertical.bin");
#else
    ret5 = this->crnn_net.load_param(mgr, "crnn_lite_dw_dense.param");
    ret6 = this->crnn_net.load_model(mgr, "crnn_lite_dw_dense.bin");
    ret7 = this->crnn_vertical_net.load_param(mgr, "crnn_lite_dw_dense.param");
    ret8 = this->crnn_vertical_net.load_model(mgr, "crnn_lite_dw_dense.bin");


#endif

    char * buffer = readKeysFromAssets(mgr);

    if (buffer != NULL) {
        std::istringstream inStr(buffer);
        std::string line;
        while (getline(inStr, line)) {
            alphabetChinese.emplace_back(line);
        }
        free(buffer);
    } else {
        LOGE(" txt file not found");
    }
    LOGI("Init Models Success!");
//
//    alphabetChinese = buffer;
//    if(buffer == nullptr)
//    {
//    return -1;
//    }else{
////    LOGI("%s",alphabetChinese.c_str());
//        LOGI("%s", "loaded alphabet\n");
//    }
    LOGI("%d, %d, %d, %d, %d, %d, %d, %d", (ret1, ret2, ret3, ret4, ret5, ret6, ret7, ret8));
    return ret1 || ret2 || ret3 ||ret4 ||ret5 ||ret6 || ret7 || ret8;
}



std::vector<std::string> crnn_deocde(const ncnn::Mat score , std::vector<std::string> alphabetChinese) {
    float *srcdata = (float* ) score.data;
    std::vector<std::string> str_res;
    int last_index = 0;  
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

                str_res.emplace_back(alphabetChinese[max_index - 1]);

        }



        last_index = max_index;
    }
    return str_res;
}



cv::Mat resize_img(cv::Mat src,const int long_size)
{
    int w = src.cols;
    int h = src.rows;
    // std::cout<<"原图尺寸 (" << w << ", "<<h<<")"<<std::endl;
    float scale = 1.f;
    if (w > h)
    {
        scale = (float)long_size / w;
        w = long_size;
        h = h * scale;
    }
    else
    {
        scale = (float)long_size / h;
        h = long_size;
        w = w * scale;
    }
    if (h % 32 != 0)
    {
        h = (h / 32 + 1) * 32;
    }
    if (w % 32 != 0)
    {
        w = (w / 32 + 1) * 32;
    }
    // std::cout<<"缩放尺寸 (" << w << ", "<<h<<")"<<std::endl;
    cv::Mat result;
    cv::resize(src, result, cv::Size(w, h));
    return result;
}

cv::Mat draw_bbox(cv::Mat &src, const std::vector<std::vector<cv::Point>> &bboxs) {
    cv::Mat dst;
    if (src.channels() == 1) {
        cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);
    } else {
        dst = src.clone();
    }
    auto color = cv::Scalar(0, 0, 255);
    for (auto bbox :bboxs) {

        cv::line(dst, bbox[0], bbox[1], color, 3);
        cv::line(dst, bbox[1], bbox[2], color, 3);
        cv::line(dst, bbox[2], bbox[3], color, 3);
        cv::line(dst, bbox[3], bbox[0], color, 3);
    }
    return dst;
}


void pse_deocde(ncnn::Mat& features,
                              std::map<int, std::vector<cv::Point>>& contours_map,
                              const float thresh,
                              const float min_area,
                              const float ratio
                              )
{

        /// get kernels
        float *srcdata = (float *) features.data;
        std::vector<cv::Mat> kernels;

        float _thresh = thresh;
        cv::Mat scores = cv::Mat::zeros(features.h, features.w, CV_32FC1);
        for (int c = features.c - 1; c >= 0; --c){
            cv::Mat kernel(features.h, features.w, CV_8UC1);
            for (int i = 0; i < features.h; i++) {
                for (int j = 0; j < features.w; j++) {

                    if (c==features.c - 1) scores.at<float>(i, j) = srcdata[i * features.w + j + features.w*features.h*c ] ;

                    if (srcdata[i * features.w + j + features.w*features.h*c ] >= _thresh) {
                    // std::cout << srcdata[i * src.w + j] << std::endl;
                        kernel.at<uint8_t>(i, j) = 1;
                    } else {
                        kernel.at<uint8_t>(i, j) = 0;
                        }

                }
            }
            kernels.push_back(kernel);
            _thresh = thresh * ratio;
        }


        /// make label
        cv::Mat label;
        std::map<int, int> areas;
        std::map<int, float> scores_sum;
        cv::Mat mask(features.h, features.w, CV_32S, cv::Scalar(0));
        cv::connectedComponents(kernels[features.c  - 1], label, 4);




        for (int y = 0; y < label.rows; ++y) {
            for (int x = 0; x < label.cols; ++x) {
                int value = label.at<int32_t>(y, x);
                float score = scores.at<float>(y,x);
                if (value == 0) continue;
                areas[value] += 1;

                scores_sum[value] += score;
            }
        }

        std::queue<cv::Point> queue, next_queue;

        for (int y = 0; y < label.rows; ++y) {

            for (int x = 0; x < label.cols; ++x) {
                int value = label.at<int>(y, x);

                if (value == 0) continue;
                if (areas[value] < min_area) {
                    areas.erase(value);
                    continue;
                }

                if (scores_sum[value]*1.0 /areas[value] < 0.93  )
                {
                    areas.erase(value);
                    scores_sum.erase(value);
                    continue;
                }
                cv::Point point(x, y);
                queue.push(point);
                mask.at<int32_t>(y, x) = value;
            }
        }

        /// growing text line
        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        for (int idx = features.c  - 2; idx >= 0; --idx) {
            while (!queue.empty()) {
                cv::Point point = queue.front(); queue.pop();
                int x = point.x;
                int y = point.y;
                int value = mask.at<int32_t>(y, x);

                bool is_edge = true;
                for (int d = 0; d < 4; ++d) {
                    int _x = x + dx[d];
                    int _y = y + dy[d];

                    if (_y < 0 || _y >= mask.rows) continue;
                    if (_x < 0 || _x >= mask.cols) continue;
                    if (kernels[idx].at<uint8_t>(_y, _x) == 0) continue;
                    if (mask.at<int32_t>(_y, _x) > 0) continue;

                    cv::Point point_dxy(_x, _y);
                    queue.push(point_dxy);

                    mask.at<int32_t>(_y, _x) = value;
                    is_edge = false;
                }

                if (is_edge) next_queue.push(point);
            }
            std::swap(queue, next_queue);
        }

        /// make contoursMap
        for (int y=0; y < mask.rows; ++y)
            for (int x=0; x < mask.cols; ++x) {
                int idx = mask.at<int32_t>(y, x);
                if (idx == 0) continue;
                contours_map[idx].emplace_back(cv::Point(x, y));
            }
}




cv::Mat matRotateClockWise180(cv::Mat src)//顺时针180
{
	//0: 沿X轴翻转； >0: 沿Y轴翻转； <0: 沿X轴和Y轴翻转
	flip(src, src, 0);// 翻转模式，flipCode == 0垂直翻转（沿X轴翻转），flipCode>0水平翻转（沿Y轴翻转），flipCode<0水平垂直翻转（先沿X轴翻转，再沿Y轴翻转，等价于旋转180°）
	flip(src, src, 1);
	return src;
	//transpose(src, src);// 矩阵转置
}

cv::Mat matRotateClockWise90(cv::Mat src)
{

	// 矩阵转置
	transpose(src, src);
	//0: 沿X轴翻转； >0: 沿Y轴翻转； <0: 沿X轴和Y轴翻转
	flip(src, src, 1);// 翻转模式，flipCode == 0垂直翻转（沿X轴翻转），flipCode>0水平翻转（沿Y轴翻转），flipCode<0水平垂直翻转（先沿X轴翻转，再沿Y轴翻转，等价于旋转180°）
	return src;
}


void  OCR::detect(cv::Mat im_bgr,int long_size)
{

        // 图像缩放
    auto im = resize_img(im_bgr, long_size);


    float h_scale = im_bgr.rows * 1.0 / im.rows;
    float w_scale = im_bgr.cols * 1.0 / im.cols;

    ncnn::Mat in = ncnn::Mat::from_pixels(im.data, ncnn::Mat::PIXEL_BGR2RGB, im.cols, im.rows);
    in.substract_mean_normalize(mean_vals_pse_angle,norm_vals_pse_angle);

    // std::cout << "输入尺寸 (" << in.w << ", " << in.h << ")" << std::endl;

    LOGI("正在检测\n");
    ncnn::Extractor ex = psenet.create_extractor();
    ex.set_num_threads(num_thread);
    ex.input("input", in);
    ncnn::Mat preds;
    double time1 = static_cast<double>( cv::getTickCount());
    ex.extract("out", preds);
//    std::cout << "psenet前向时间:" << (static_cast<double>( cv::getTickCount()) - time1) / cv::getTickFrequency() << "s" << std::endl;
    // std::cout << "网络输出尺寸 (" << preds.w << ", " << preds.h << ", " << preds.c << ")" << std::endl;

    time1 = static_cast<double>( cv::getTickCount());
    std::map<int, std::vector<cv::Point>> contoursMap;
    pse_deocde(preds, contoursMap, 0.7311, 10, 1);
    std::vector<std::vector<cv::Point>> bboxs;
    std::vector<cv::RotatedRect> rects ;
    for (auto &cnt: contoursMap) {
        cv::Mat bbox;
        cv::RotatedRect rect = cv::minAreaRect(cnt.second);
        rect.size.width = rect.size.width * w_scale;
        rect.size.height = rect.size.height * h_scale;
        rect.center.x = rect.center.x * w_scale;
        rect.center.y = rect.center.y * h_scale;
        rects.push_back(rect);
        cv::boxPoints(rect, bbox);
        std::vector<cv::Point> points;
        for (int i = 0; i < bbox.rows; ++i) {
                points.emplace_back(cv::Point(int(bbox.at<float>(i, 0) ), int(bbox.at<float>(i, 1) )));
            }
        bboxs.emplace_back(points);

    }
    LOGI("检测到%d个文本框, %d个contourmap", (bboxs.size(), contoursMap.size()));
//    std::cout << "psenet decode 时间:" << (static_cast<double>( cv::getTickCount()) - time1) / cv::getTickFrequency() << "s" << std::endl;
//    std::cout << "boxzie" << bboxs.size() << std::endl;

    auto result = draw_bbox(im_bgr, bboxs);
//    cv::imwrite("./imgs/result.jpg", result);

    time1 = static_cast<double>( cv::getTickCount());
    //开始行文本角度检测和文字识别
//    std::cout << "预测结果：\n";
    LOGI("正在识别\n");
    for (int i = 0; i < rects.size() ; i++ ){
        cv::RotatedRect  temprect = rects[i];
        // std::cout<<  temprect.size.width << "," << temprect.size.height << "," <<  temprect.center.x <<
        // "," <<  temprect.center.y  << "," <<  temprect.angle << std::endl;
        // cv::Mat part_im = crop_text_area(temprect,im_bgr);
        cv::Mat part_im ;

        int  min_size   = temprect.size.width>temprect.size.height?temprect.size.height:temprect.size.width;
        temprect.size.width  =   int(temprect.size.width + min_size * 0.15);
        temprect.size.height =   int(temprect.size.height + min_size * 0.15);


        RRLib::getRotRectImg(temprect, im_bgr, part_im);

        int part_im_w = part_im.cols;
        int part_im_h = part_im.rows;
        // std::cout << "网络输出尺寸 (" << part_im_w<< ", " << part_im_h <<  ")" << std::endl;
        if (part_im_h > 1.5 *  part_im_w) part_im = matRotateClockWise90(part_im);

        cv::Mat angle_input = part_im.clone();
        // part_im_w = angle_input.cols;
        // part_im_h = angle_input.rows;
        // std::cout << "网络输出尺寸 2(" << part_im_w<< ", " << part_im_h <<  ")" << std::endl;
        // cv::imwrite("test.jpg",part_im);
        //分类
        ncnn::Mat  shufflenet_input = ncnn::Mat::from_pixels_resize(angle_input.data, 
                ncnn::Mat::PIXEL_BGR2RGB, angle_input.cols, part_im.rows ,shufflenetv2_target_w ,shufflenetv2_target_h );

        shufflenet_input.substract_mean_normalize(mean_vals_pse_angle,norm_vals_pse_angle );
        ncnn::Extractor shufflenetv2_ex = angle_net.create_extractor();
        shufflenetv2_ex.set_num_threads(num_thread);
        shufflenetv2_ex.input("input", shufflenet_input);
        ncnn::Mat angle_preds;
        double time2 = static_cast<double>( cv::getTickCount());
        shufflenetv2_ex.extract("out", angle_preds);

        // std::cout << "anglenet前向时间:" << (static_cast<double>( cv::getTickCount()) - time2) / cv::getTickFrequency() << "s" << std::endl;
        // std::cout << "网络输出尺寸 (" << preds.w << ", " << preds.h << ", " << preds.c << ")" << std::endl;

        float *srcdata =(float*) angle_preds.data;

        int angle_index = 0;
        int max_value ;
        for (int i=0; i<angle_preds.w;i++){
            // std::cout << srcdata[i] << std::endl;
            if (i==0)max_value = srcdata[i];
            else if (srcdata[i] > angle_index) {
                angle_index = i ;
                max_value = srcdata[i];
            }
        }
        
        if (angle_index == 0 || angle_index ==2) part_im = matRotateClockWise180(part_im);

        // 开始文本识别
        int crnn_w_target ;
        float scale  = crnn_h * 1.0/ part_im.rows ;
        crnn_w_target = int(part_im.cols * scale ) ;

        char *svavePath = new char[25];
        sprintf( svavePath, "debug_im/%d.jpg", i);
//        cv::imwrite(svavePath,part_im);
        // part_im = cv::imread("test.jpg");

        cv::Mat img2 = part_im.clone();

        ncnn::Mat  crnn_in = ncnn::Mat::from_pixels_resize(img2.data, 
                    ncnn::Mat::PIXEL_BGR2GRAY, img2.cols, img2.rows , crnn_w_target, crnn_h );

        // ncnn::Mat  crnn_in = ncnn::Mat::from_pixels_resize(part_im.data, 
        //             ncnn::Mat::PIXEL_BGR2GRAY, part_im.cols, part_im.rows , crnn_w_target, crnn_h );
        
        crnn_in.substract_mean_normalize(mean_vals_crnn,norm_vals_crnn );
       
        ncnn::Mat crnn_preds;

        //判断用横排还是竖排模型 { 0 : "hengdao",  1:"hengzhen",  2:"shudao",  3:"shuzhen"} #hengdao: 文本行横向倒立 其他类似
       
        // time1 = static_cast<double>( cv::getTickCount());
        // std::cout << angle_index << std::endl;
        if (angle_index ==0 || angle_index ==1 ){

            ncnn::Extractor crnn_ex = crnn_net.create_extractor();
            crnn_ex.set_num_threads(num_thread);
            crnn_ex.input("input", crnn_in);
#if CRNN_LSTM
            // lstm

            ncnn::Mat blob162;
            crnn_ex.extract("234", blob162);

            // batch fc
            ncnn::Mat blob182(256, blob162.h);
            for (int i=0; i<blob162.h; i++)
            {
                ncnn::Extractor crnn_ex_1 = crnn_net.create_extractor();
                crnn_ex_1.set_num_threads(num_thread);
                ncnn::Mat blob162_i = blob162.row_range(i, 1);
                crnn_ex_1.input("253", blob162_i);

                ncnn::Mat blob182_i;
                crnn_ex_1.extract("254", blob182_i);

                memcpy(blob182.row(i), blob182_i, 256 * sizeof(float));
            }

            // lstm
            ncnn::Mat blob243;
            crnn_ex.input("260", blob182);
            crnn_ex.extract("387", blob243);

            // batch fc
            ncnn::Mat blob263(5530, blob243.h);
            for (int i=0; i<blob243.h; i++)
            {
                ncnn::Extractor crnn_ex_2 = crnn_net.create_extractor();
                crnn_ex_2.set_num_threads(num_thread);
                ncnn::Mat blob243_i = blob243.row_range(i, 1);
                crnn_ex_2.input("406", blob243_i);

                ncnn::Mat blob263_i;
                crnn_ex_2.extract("407", blob263_i);

                memcpy(blob263.row(i), blob263_i, 5530 * sizeof(float));
            }

            crnn_preds = blob263;
#else // CRNN_LSTM
            crnn_ex.extract("out", crnn_preds);
#endif // CRNN_LSTM
        }
        else{


            ncnn::Extractor crnn_ex = crnn_vertical_net.create_extractor();
            crnn_ex.set_num_threads(num_thread);
            crnn_ex.input("input", crnn_in);
#if CRNN_LSTM
            // lstm

            ncnn::Mat blob162;
            crnn_ex.extract("234", blob162);

            // batch fc
            ncnn::Mat blob182(256, blob162.h);
            for (int i=0; i<blob162.h; i++)
            {
                ncnn::Extractor crnn_ex_1 = crnn_vertical_net.create_extractor();
                crnn_ex_1.set_num_threads(num_thread);
                ncnn::Mat blob162_i = blob162.row_range(i, 1);
                crnn_ex_1.input("253", blob162_i);

                ncnn::Mat blob182_i;
                crnn_ex_1.extract("254", blob182_i);

                memcpy(blob182.row(i), blob182_i, 256 * sizeof(float));
            }

            // lstm
            ncnn::Mat blob243;
            crnn_ex.input("260", blob182);
            crnn_ex.extract("387", blob243);

            // batch fc
            ncnn::Mat blob263(5530, blob243.h);
            for (int i=0; i<blob243.h; i++)
            {
                ncnn::Extractor crnn_ex_2 = crnn_vertical_net.create_extractor();
                crnn_ex_2.set_num_threads(num_thread);
                ncnn::Mat blob243_i = blob243.row_range(i, 1);
                crnn_ex_2.input("406", blob243_i);

                ncnn::Mat blob263_i;
                crnn_ex_2.extract("407", blob263_i);

                memcpy(blob263.row(i), blob263_i, 5530 * sizeof(float));
            }

            crnn_preds = blob263;
#else // CRNN_LSTM
            crnn_ex.extract("out", crnn_preds);
#endif // CRNN_LSTM
        }
         
       
    //    crnn_ex.set_num_threads(4);ss
     

        // std::cout << "前向时间:" << (static_cast<double>( cv::getTickCount()) - time1) / cv::getTickFrequency() << "s" << std::endl;
        // std::cout << "网络输出尺寸 (" << crnn_preds.w << ", " << crnn_preds.h << ", " << crnn_preds.c << ")" << std::endl;


        auto res_pre = crnn_deocde(crnn_preds,alphabetChinese);

        for (int i=0; i<res_pre.size();i++){
//            std::cout << res_pre[i] ;
            LOGI("%s", res_pre[i].c_str());
        }
//        std::cout  <<std::endl;


    }
//    std::cout << "角度检测和文字识别总时间:" << (static_cast<double>( cv::getTickCount()) - time1) / cv::getTickFrequency() << "s" << std::endl;

    LOGI("角度识别时间：%lf s", (static_cast<double>( cv::getTickCount()) - time1) / cv::getTickFrequency());


}


