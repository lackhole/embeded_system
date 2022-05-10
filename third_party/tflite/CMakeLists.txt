cmake_minimum_required(VERSION 3.5)
project(tflite)
add_library(tflite INTERFACE)

set(TFLITE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(TFLITE_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/mac")
  set(TFLITE_LIBS ${TFLITE_LIBRARY_DIR}/libtensorflowlite_c.dylib)

elseif (UNIX)
  set(TFLITE_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/pi")
  set(TFLITE_LIBS ${TFLITE_LIBRARY_DIR}/libtensorflowlite_c.so)

else()
  message(FATAL_ERROR "Unknown platform")
endif ()

target_link_libraries(tflite INTERFACE ${TFLITE_LIBS})
target_include_directories(tflite INTERFACE ${TFLITE_INCLUDE_DIRS})
