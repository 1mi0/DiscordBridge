add_executable(server)

target_include_directories(server PRIVATE ..)
target_include_directories(server PRIVATE ../common)

message("Includes: ${Boost_INCLUDE_DIRS}")
message("Link libraries: ${Boost_LIBRARIES}")
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})

target_include_directories(server PRIVATE ../fmt/include/)
target_link_libraries(server PRIVATE fmt::fmt Threads::Threads)

target_include_directories(server PRIVATE ../DPP/include)
target_link_libraries(server PRIVATE dpp)

target_compile_options(server PRIVATE ${COMPILE_OPTIONS})

target_compile_definitions(server PRIVATE ${COMPILE_DEFINITIONS})

target_precompile_headers(server PRIVATE ../common/common.hpp)
target_sources(server PRIVATE main.cxx ${common_SOURCES} ${bot_SOURCES})
