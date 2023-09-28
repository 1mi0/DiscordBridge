cmake_minimum_required(VERSION 2.25.1)

project(discord_bridge_backend)

find_package(Threads)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(COMPILE_OPTIONS -O3 -Wall -Wextra -Werror -pthread)

if (DEBUG_ENABLED)
  set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -g)
endif ()

file(GLOB common_SOURCES "common/*.cxx")
file(GLOB bot_SOURCES "bot/*.cxx")

add_subdirectory(dependencies/fmt)
add_subdirectory(dependencies/DPP)
#add_subdirectory(boost)
add_subdirectory(src)
