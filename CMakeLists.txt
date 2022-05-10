cmake_minimum_required(VERSION 3.21)
project(embed)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
add_subdirectory(third_party/tflite)
add_subdirectory(third_party/cutemodel)

set(EMBED_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

add_executable(embed
    main.cc
    ${EMBED_INCLUDE_DIR}/embed/async_camera_controller.cc
    ${EMBED_INCLUDE_DIR}/embed/cross_camera.cc
    ${EMBED_INCLUDE_DIR}/embed/object_detection_model.cc

#    input/image_input.cc
#    input/camera_input.cc
#    input/video_input.cc
    )

target_include_directories(embed PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(embed PUBLIC ${OpenCV_LIBS} tflite cutemodel)
