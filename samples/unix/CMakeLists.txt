cmake_minimum_required(VERSION 3.5)
project(rtnode-unix C)

add_executable(rtnode-unix main.c curl.c hello.c)
target_link_libraries(rtnode-unix rtnode curl)
