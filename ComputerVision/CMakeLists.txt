cmake_minimum_required(VERSION 3.16)
project(ComputerVision LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.11.2 REQUIRED COMPONENTS Widgets Gui Core Concurrent Charts)

qt_add_executable(ComputerVision
    main.cpp
)

target_link_libraries(ComputerVision PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Concurrent Qt6::Charts)

target_sources(ComputerVision PRIVATE
    "ui/MainWindow.cpp"
    "ui/MainWindow.h"
 "image/ImageProcessor.h" "image/ImageProcessor.cpp" "ui/ImageView.h" "ui/ImageView.cpp" "image/ImageModel.h" "image/ImageModel.cpp" "ui/ImageInfo.h" "ui/ImageInfo.cpp" "image/ImageMetadata.h" "lib/easyexif/exif.cpp" "lib/easyexif/exif.h" "ui/ImageAdjustmentPanel.h" "ui/ImageAdjustmentPanel.cpp" "ui/ImageAdjustmentPanel.h" "ui/HistogramWidget.h" "ui/HistogramWidget.cpp")