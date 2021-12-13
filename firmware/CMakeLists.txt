cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(lidar_project C CXX ASM)

pico_sdk_init()

file( GLOB source_files
                    "src/*.c"
)

add_executable(lidar ${source_files})

target_link_libraries(lidar pico_stdlib hardware_pwm pico_bootrom hardware_i2c hardware_clocks)
target_include_directories(lidar PRIVATE "./include")

pico_enable_stdio_usb(lidar true)

pico_add_extra_outputs(lidar)