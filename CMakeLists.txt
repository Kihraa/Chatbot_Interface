cmake_minimum_required(VERSION 3.31)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(Chat)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
if(NOT CMAKE_BUILD_TYPE)
     set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(MD4C CONFIG REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

include_directories(${Qt6Core_INCLUDE_DIRS} ${Qt6Gui_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${MD4C_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")


add_executable(chat ${SOURCES} ${HEADERS})
target_link_libraries(chat PRIVATE
    Qt6::Widgets
    Qt6::Network
    md4c::md4c
    md4c::md4c-html
)
