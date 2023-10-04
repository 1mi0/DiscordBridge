cmake_minimum_required(VERSION 2.25.1)

project(discord_bridge_backend)

find_package(Threads REQUIRED)

find_package(Boost REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(COMPILE_OPTIONS -O3 -Wall -Wextra -Werror -pthread)
set(COMPILE_DEFINITIONS -DNDEBUG)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -ggdb)
  set(COMPILE_DEFINITIONS -DDEBUG)
endif ()

file(GLOB common_SOURCES "common/*.cxx")
file(GLOB bot_SOURCES "bot/*.cxx")

add_subdirectory(dependencies/fmt)
add_subdirectory(dependencies/DPP)
add_subdirectory(src)
