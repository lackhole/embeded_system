cmake_minimum_required(VERSION 3.5)
project(cutemodel)

set(CUTEMODEL_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(cutemodel STATIC include/cutemodel/cute_model.cc)

target_link_libraries(cutemodel PUBLIC tflite)
target_include_directories(cutemodel PUBLIC ${CUTEMODEL_INCLUDE_DIR})
