cmake_minimum_required(VERSION 3.29)
project(SeaBattleDemchenko C CXX)

set(CMAKE_C_STANDARD 11)

add_executable(SeaBattleDemchenko Game/NewSeaBattle.cpp)

target_link_libraries(SeaBattleDemchenko PRIVATE gdiplus)
target_link_libraries(SeaBattleDemchenko PRIVATE gdi32)
