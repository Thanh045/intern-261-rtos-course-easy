cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME template)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)
# Đường dẫn tới mã ứng dụng bài 1
set(APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../week2/ex1/app")


# Cung cấp FreeRTOSConfig.h và cấu hình chip cho kernel
add_library(freertos_config INTERFACE)

target_include_directories(freertos_config SYSTEM INTERFACE
    "${APP_DIR}"
)

target_link_libraries(freertos_config INTERFACE
    stm32cubemx
)

# Chọn port Cortex-M3 và bộ cấp phát heap_4
set(FREERTOS_PORT GCC_ARM_CM3 CACHE STRING "" FORCE)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

# Thêm kernel nằm ngoài thư mục template
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/FreeRTOS-Kernel"
    "${CMAKE_CURRENT_BINARY_DIR}/freertos-kernel"
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
         "${APP_DIR}/app_main.c"
    "${APP_DIR}/sensor_task.c"
    "${APP_DIR}/filter_task.c"
    "${APP_DIR}/uart_task.c"
    "${APP_DIR}/hooks.c"
)
# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
freertos_kernel
    # Add user defined libraries
)
