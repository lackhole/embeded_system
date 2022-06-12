cmake_minimum_required(VERSION 3.5)
project(boost)
add_library(boost INTERFACE)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  target_include_directories(boost INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

elseif (UNIX)
  find_package(Boost REQUIRED)

  target_include_directories(boost PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(boost PUBLIC ${Boost_LIBS})
endif()
