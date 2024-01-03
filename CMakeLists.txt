cmake_minimum_required(VERSION 3.16)
project(colo_manage_tools)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)

link_directories(${PROJECT_SOURCE_DIR}/lib/)

include_directories(${PROJECT_SOURCE_DIR}/include/)

find_library(LIBYAML NAMES yaml-cpp PATHS ${PROJECT_SOURCE_DIR}/lib/)


add_executable(${PROJECT_NAME} src/main.cpp src/utils/utils.cpp)
target_link_libraries(${PROJECT_NAME} ${LIBYAML})