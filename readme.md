本项目是深度学习部署案例的集合

我们不生产模型，模型文件均来自于GitHub开源项目。

包括的内容：

# 1. SimplestNCNNExample

一个非常精简的NCNN安卓端部署的例子。模型来自于Chineseocr_lite项目中的crnn_dense。

依赖： NCNN、 OpenCV 

# 2. PyTorchYOLOv3Example

使用flask搭建一个PyTorchYOLOv3的演示界面。项目模型来自于PyTorchYOLOv3开源项目。

依赖: Flask、 PyTorch 


# todo list

安卓端部署：

- [x] 最简安卓部署案例——不加RNN的CRNN模型的安卓端部署
- [x] 将Chineseocr_lite项目打包成安卓app

服务端部署：

- [x] 部署一个PyTorch-YOLOv3的demo

计划添加的内容

- [ ] Docker相关案例 
- [ ] 完整的人脸识别部署案例（检测+活体+特征提取+搜索）
