cmake_minimum_required(VERSION 3.16)
project(ComputerVision LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.11.2 REQUIRED COMPONENTS Widgets Gui Core)

qt_add_executable(ComputerVision
    main.cpp
)

target_link_libraries(ComputerVision PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)

target_sources(ComputerVision PRIVATE
    "ui/MainWindow.cpp"
    "ui/MainWindow.h"
 "image/ImageProcessor.h" "image/ImageProcessor.cpp" "ui/ImageView.h" "ui/ImageView.cpp" "image/ImageModel.h" "image/ImageModel.cpp")