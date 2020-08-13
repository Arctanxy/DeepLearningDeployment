cmake_minimum_required(VERSION 3.4.1)

## add ncnn prebuilt 0413
set(ncnn_path D:\\scripts\\ocr_android\\ncnn_0413)
include_directories(${ncnn_path}\\include\\ncnn)
link_directories(${ncnn_path}\\armeabi-v7a)
set(ncnn_lib ${ncnn_path}\\armeabi-v7a\\libncnn.a)
add_library (ncnn STATIC IMPORTED)
set_target_properties(ncnn PROPERTIES IMPORTED_LOCATION ${ncnn_lib})

# add opencv
set(OpenCV_DIR D:\\scripts\\ocr_android\\opencv_release_armeabi\\build)
find_package(OpenCV REQUIRED)

# openmp
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fopenmp")

add_library( # Sets the name of the library.
        native-lib
        SHARED
        native-lib.cpp model.cpp)

find_library(
        log-lib
        log
        android)

target_link_libraries( # Specifies the target library.
        native-lib
        ncnn
        ${OpenCV_LIBS}
        android
        jnigraphics
        ${log-lib})