cmake_minimum_required(VERSION 3.15)
project(embed)

set(CMAKE_CXX_STANDARD 17)

if (UNIX AND NOT APPLE)
    set(CMAKE_MODULE_PATH "/usr/local/lib/cmake/${CMAKE_MODULE_PATH}")
    find_package(raspicam REQUIRED)
    add_compile_definitions(USE_XNN_DELEGATE=0)
endif()
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



if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(EMBED_INCLUDE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${OpenCV_INCLUDE_DIRS})
    set(EMBED_LIBS
        ${OpenCV_LIBS} tflite cutemodel)

elseif (UNIX)
    set(EMBED_INCLUDE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${OpenCV_INCLUDE_DIRS}
        ${raspicam_INCLUDE_DIRS})
    set(EMBED_LIBS
        ${OpenCV_LIBS}
        ${raspicam_CV_LIBS}
        tflite
        cutemodel
        -lpthread
        -ldl)

else()
    message(FATAL_ERROR "Unknown platform")
endif ()

target_include_directories(embed PUBLIC ${EMBED_INCLUDE_DIRS})
target_link_libraries(embed PUBLIC ${EMBED_LIBS})
#target_compile_definitions(embed PUBLIC USE_XNN_DELEGATE=0)
