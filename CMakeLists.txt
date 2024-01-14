cmake_minimum_required(VERSION 3.16)
project(colo_manage_tools)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(ZMQ REQUIRED libzmq)

link_directories(${PROJECT_SOURCE_DIR}/lib/)

include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${ZMQ_INCLUDE_DIRS})

find_library(LIBYAML NAMES yaml-cpp PATHS ${PROJECT_SOURCE_DIR}/lib/)
find_library(LIBZMQ NAMES zmq PATHS ${PROJECT_SOURCE_DIR}/lib/)

set(SRC_LIST src/main.cpp src/utils/domain.cpp src/utils/config_parser.cpp 
src/utils/runvm.cpp src/utils/shell_command.cpp src/utils/colod_utils.cpp)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${LIBYAML} ${LIBZMQ} ${GLIB_LDFLAGS} utils)

add_executable(colod src/colod.cpp src/utils/config_parser.cpp src/utils/colod_utils.cpp src/utils/colod_server.cpp
src/utils/colod_service.cpp)
target_link_libraries(colod ${LIBYAML} ${LIBZMQ})



