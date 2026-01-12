cmake_minimum_required(VERSION 3.16)
project(ChessGui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(ChessGui main.cpp)
target_link_libraries(ChessGui Qt6::Core Qt6::Gui Qt6::Widgets)
