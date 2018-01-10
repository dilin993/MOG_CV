cmake_minimum_required(VERSION 3.5)
project(testcode)
set(CMAKE_CXX_STANDARD 11)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS})

file(GLOB SOURCE_FILES
        "./*.h"
        "./*.cpp"
        "./*.hpp"
        )

add_executable(testgmm ${SOURCE_FILES})

target_link_libraries(testgmm ${OpenCV_LIBS} -lpthread )