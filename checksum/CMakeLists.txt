cmake_minimum_required(VERSION 2.8)
project(md5)
AUX_SOURCE_DIRECTORY(. TOP_SRCS)
include_directories(/home/gaofeng/gaofeng/md5/md5/include)
add_definitions("-Wall -g")
add_subdirectory(checksum)
add_subdirectory(account)
IF(CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)                            
ENDIF()

add_executable(md5.out ${TOP_SRCS})

target_link_libraries(md5.out checksum pthread mysqlclient account)
