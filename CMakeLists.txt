cmake_minimum_required(VERSION 3.12)
project(MyKernel C)

set(CMAKE_C_STANDARD 99)

add_executable(MyKernel ls.c wc.c)