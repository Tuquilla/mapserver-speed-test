cmake_minimum_required(VERSION 3.29)
project(mapserver-speed-test C)

set(CMAKE_C_STANDARD 23)

find_package(CURL REQUIRED)

add_executable(mapserver-speed-test main.c)

target_link_libraries(mapserver-speed-test PRIVATE CURL::libcurl)
