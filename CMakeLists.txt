cmake_minimum_required(VERSION 3.12)

add_subdirectory(external)

include_directories(include)

add_executable(http_client http_client.cc)
