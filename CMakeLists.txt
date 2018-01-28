cmake_minimum_required(VERSION 2.8)

project(hardware-test)
add_executable(${PROJECT_NAME} "main.cpp" "hardware/IO.cpp" "hardware/socket.cpp" "function/function.cpp")

set(CMAKE_CXX_FLAGS "-std=c++11 -lpthread -o3 -g")

INCLUDE_DIRECTORIES("/home/ted/freescale/opencv-arm-include/include")

LINK_DIRECTORIES("/home/ted/freescale/opencv-arm-include/lib")



TARGET_LINK_LIBRARIES(hardware-test
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_videoio.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_stitching.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_imgproc.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_objdetect.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_shape.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_video.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_photo.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_dnn.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_videostab.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_highgui.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_calib3d.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_superres.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_flann.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_features2d.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_imgcodecs.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_ml.so.3.4"
    "/home/ted/freescale/opencv-arm-include/lib/libopencv_core.so.3.4"
    )
