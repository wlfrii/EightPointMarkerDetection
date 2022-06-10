cmake_minimum_required(VERSION 3.0)

project(EightPointMarkerDetection)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    eight_point_marker_locator.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${OpenCV_INCLUDE_DIRS}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
)
