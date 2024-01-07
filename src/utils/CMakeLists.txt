aux_source_directory(. DIR_LIB_SRCS)

add_library (utils ${DIR_LIB_SRCS})

link_directories(${PROJECT_SOURCE_DIR}/lib/)

find_library(LIBYAML NAMES yaml-cpp PATHS ${PROJECT_SOURCE_DIR}/lib/)



add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} ${LIBYAML} utils)