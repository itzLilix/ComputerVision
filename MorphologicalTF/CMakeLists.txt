cmake_minimum_required(VERSION 3.21)
project(MorphologicalTF VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup() # enables AUTOMOC / AUTOUIC / AUTORCC (Qt 6.3+)

qt_add_executable(${PROJECT_NAME}
    src/main.cpp

    src/model/ImageModel.h
    src/model/ImageModel.cpp
    src/model/Kernel.h
    src/model/Kernel.cpp
    src/model/KernelPresets.h
    src/model/KernelPresets.cpp
    src/model/MorphologyOp.h
    src/model/MorphProcessor.h
    src/model/MorphProcessor.cpp
 
    src/viewmodel/ImageViewModel.h
    src/viewmodel/ImageViewModel.cpp
    src/viewmodel/KernelViewModel.h
    src/viewmodel/KernelViewModel.cpp
    src/viewmodel/ProcessingViewModel.h
    src/viewmodel/ProcessingViewModel.cpp
 
    src/view/ImageView.h
    src/view/ConvolutionMatrixEditor.h
    src/view/ConvolutionMatrixEditor.cpp
    src/view/ImageView.cpp
    src/view/KernelGridView.h
    src/view/KernelGridView.cpp
    src/view/KernelPanel.h
    src/view/KernelPanel.cpp
    src/view/MainWindow.h
    src/view/MainWindow.cpp
    src/view/ProcessingPanel.h
    src/view/ProcessingPanel.cpp
)

# Lets us write #include "model/ImageModel.h" from any layer
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

# Uncomment to hide the console window on Windows:
# set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)