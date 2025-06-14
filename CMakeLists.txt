cmake_minimum_required(VERSION 3.16)
project(QtPhotoBoothApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON) # For the resources.qrc file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find Qt 6
# You will need to set CMAKE_PREFIX_PATH when running cmake if Qt6 is not in system paths
# e.g., cmake -DCMAKE_PREFIX_PATH=/path/to/your/Qt/6.7.3/gcc_arm64 ..
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Add executable
add_executable(QtPhotoBoothApp
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/photosessiondata.h
    resources/resources.qrc # Add the QRC file here
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,leak -fno-omit-frame-pointer")
    # For some setups, you might also need to link the sanitizer libraries explicitly,
    # but for GCC/Clang, -fsanitize=address often handles linking too.
    # As an alternative or addition for linking:
    # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,leak")
endif()

# Link to Qt6 modules
target_link_libraries(QtPhotoBoothApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Ensure includes from autogen are available
target_include_directories(QtPhotoBoothApp PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
