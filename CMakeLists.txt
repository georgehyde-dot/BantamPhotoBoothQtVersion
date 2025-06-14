cmake_minimum_required(VERSION 3.16)
project(QtPhotoBoothApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON) # For the resources.qrc file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Detect platform
if(APPLE)
    set(IS_MAC TRUE)
    message(STATUS "Building for macOS")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(IS_RASPBERRY_PI TRUE)
    message(STATUS "Building for Raspberry Pi (ARM Linux)")
else()
    message(STATUS "Building for generic Linux/Windows")
endif()

# Find Qt 6 - different components based on platform
if(IS_MAC)
    # Mac needs Multimedia components for camera
    find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets REQUIRED)
    message(STATUS "Found Qt6 with Multimedia support for macOS")
elseif(IS_RASPBERRY_PI)
    # Pi might not have Multimedia, so make it optional
    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
    find_package(Qt6 COMPONENTS Multimedia MultimediaWidgets QUIET)
    if(Qt6Multimedia_FOUND)
        message(STATUS "Found Qt6 with Multimedia support for Raspberry Pi")
        set(HAS_QT_MULTIMEDIA TRUE)
    else()
        message(STATUS "Qt6 Multimedia not found - using system camera commands on Pi")
        set(HAS_QT_MULTIMEDIA FALSE)
    endif()
else()
    # Generic Linux/Windows
    find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets REQUIRED)
    set(HAS_QT_MULTIMEDIA TRUE)
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/photosessiondata.h
    src/icamera.h
    src/camerafactory.cpp
    src/camerafactory.h
    src/mockcamera.cpp
    src/mockcamera.h
    resources/resources.qrc
)

# Add platform-specific camera implementations
if(IS_MAC OR HAS_QT_MULTIMEDIA)
    list(APPEND SOURCES
        src/qtcamera.cpp
        src/qtcamera.h
    )
endif()

if(IS_RASPBERRY_PI)
    list(APPEND SOURCES
        src/picamera.cpp
        src/picamera.h
    )
endif()

# Add executable
add_executable(QtPhotoBoothApp ${SOURCES})

# Compiler-specific flags for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(IS_MAC)
            # macOS: Use AddressSanitizer but not LeakSanitizer (not well supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
            message(STATUS "Enabled AddressSanitizer for macOS debug build")
        elseif(IS_RASPBERRY_PI)
            # Raspberry Pi: Use both AddressSanitizer and LeakSanitizer
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak -fno-omit-frame-pointer")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,leak -fno-omit-frame-pointer")
            message(STATUS "Enabled AddressSanitizer and LeakSanitizer for Raspberry Pi debug build")
        else()
            # Generic Linux: Try both, fallback if needed
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,leak -fno-omit-frame-pointer")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,leak -fno-omit-frame-pointer")
            message(STATUS "Enabled AddressSanitizer and LeakSanitizer for debug build")
        endif()
    endif()
endif()

# Link to Qt6 modules - base modules for all platforms
target_link_libraries(QtPhotoBoothApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# Add multimedia libraries if available
if(IS_MAC OR HAS_QT_MULTIMEDIA)
    target_link_libraries(QtPhotoBoothApp PRIVATE
        Qt6::Multimedia
        Qt6::MultimediaWidgets
    )
    target_compile_definitions(QtPhotoBoothApp PRIVATE HAS_QT_MULTIMEDIA)
    message(STATUS "Linking Qt6 Multimedia libraries")
endif()

# Platform-specific compile definitions
if(IS_MAC)
    target_compile_definitions(QtPhotoBoothApp PRIVATE IS_MAC)
elseif(IS_RASPBERRY_PI)
    target_compile_definitions(QtPhotoBoothApp PRIVATE IS_RASPBERRY_PI)
endif()

# Ensure includes from autogen are available
target_include_directories(QtPhotoBoothApp PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Platform-specific installation and packaging
if(IS_MAC)
    # Configure Info.plist with proper substitutions
    configure_file(
        "${CMAKE_SOURCE_DIR}/Info.plist.in"
        "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
        @ONLY
    )
    
    # macOS app bundle configuration
    set_target_properties(QtPhotoBoothApp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.qtphotoboothapp"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
    )
    
    # Add required frameworks for camera permissions
    find_library(AVFOUNDATION_FRAMEWORK AVFoundation)
    find_library(COREMEDIA_FRAMEWORK CoreMedia)
    if(AVFOUNDATION_FRAMEWORK AND COREMEDIA_FRAMEWORK)
        target_link_libraries(QtPhotoBoothApp PRIVATE
            ${AVFOUNDATION_FRAMEWORK}
            ${COREMEDIA_FRAMEWORK}
        )
    endif()
elseif(IS_RASPBERRY_PI)
    # Raspberry Pi specific settings
    # Ensure we can find system camera commands
    find_program(LIBCAMERA_STILL_EXECUTABLE libcamera-still)
    find_program(RASPISTILL_EXECUTABLE raspistill)
    
    if(LIBCAMERA_STILL_EXECUTABLE)
        message(STATUS "Found libcamera-still: ${LIBCAMERA_STILL_EXECUTABLE}")
        target_compile_definitions(QtPhotoBoothApp PRIVATE HAS_LIBCAMERA_STILL)
    elseif(RASPISTILL_EXECUTABLE)
        message(STATUS "Found raspistill: ${RASPISTILL_EXECUTABLE}")
        target_compile_definitions(QtPhotoBoothApp PRIVATE HAS_RASPISTILL)
    else()
        message(WARNING "No camera capture commands found on Raspberry Pi")
    endif()
endif()

# Build configuration summary
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Qt6 Version: ${Qt6_VERSION}")
if(IS_MAC)
    message(STATUS "macOS specific features enabled")
elseif(IS_RASPBERRY_PI)
    message(STATUS "Raspberry Pi specific features enabled")
    message(STATUS "Qt Multimedia available: ${HAS_QT_MULTIMEDIA}")
endif()
message(STATUS "===================================")
