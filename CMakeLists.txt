cmake_minimum_required(VERSION 3.20)
project(untitled CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)

# Tutaj dopisujesz WSZYSTKIE pliki .cpp Twojego projektu:
add_executable(untitled
        main.cpp
        ImageProcessing.h
        ImageProcessing.cpp
)

target_include_directories(untitled PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(untitled PRIVATE ${OpenCV_LIBS})