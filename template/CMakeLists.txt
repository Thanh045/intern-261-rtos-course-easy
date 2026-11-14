
cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME template)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
enable_language(C ASM)

add_executable(${CMAKE_PROJECT_NAME})

add_subdirectory(cmake/stm32cubemx)

# Duong dan bai Ex4
set(APP_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../week3/ex4/app"
)

add_library(freertos_config INTERFACE)

target_include_directories(
    freertos_config SYSTEM INTERFACE
    "${APP_DIR}"
)

target_link_libraries(
    freertos_config INTERFACE
    stm32cubemx
)

set(FREERTOS_PORT GCC_ARM_CM3 CACHE STRING "" FORCE)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/FreeRTOS-Kernel"
    "${CMAKE_CURRENT_BINARY_DIR}/freertos-kernel"
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
"${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c"
    "${APP_DIR}/app_main.c"
    "${APP_DIR}/deadlock_demo.c"
    "${APP_DIR}/runtime_stats.c"
    "${APP_DIR}/supervisor.c"
    "${APP_DIR}/hooks.c"
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${APP_DIR}"
)

# Cau hinh thuc nghiem
# 1 = deadlock goc, 0 = da sua
# Watchdog: 1 = bat, 0 = tat


list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    freertos_kernel
)
