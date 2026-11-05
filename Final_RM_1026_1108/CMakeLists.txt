cmake_minimum_required(VERSION 3.16)#CMake最低版本要求3.16
project(Final_RM_1026_1108 LANGUAGES CXX)#项目名Final_RM_1026_1108，语言为C++

set(CMAKE_CXX_STANDARD 17)#C++标准为C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)#强制使用，编译器不支持就报错

find_package(OpenCV REQUIRED)#寻找OpenCV库，个人使用opencv5

add_executable(main main.cpp function.cpp)#生成可执行文件main.exe，并用main.cpp和function.cpp编译

target_include_directories(main PRIVATE ${OpenCV_INCLUDE_DIRS})#告知编译器头文件位置
target_link_libraries(main PRIVATE ${OpenCV_LIBS})#链接OpenCV头文件和库