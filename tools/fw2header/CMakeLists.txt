cmake_minimum_required(VERSION 3.25)

set(PROJECT fw2header)
project({PROJECT} C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

file(GLOB FILES *.c *.cpp *.h *.hpp)

add_executable(${PROJECT} ${FILES})