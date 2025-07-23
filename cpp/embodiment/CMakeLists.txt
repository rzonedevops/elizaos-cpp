cmake_minimum_required(VERSION 3.16)

project(elizaos-embodiment)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
set(SOURCES
    src/embodiment.cpp
    src/perception_action_loop.cpp
    src/embodiment_manager.cpp
    src/console_text_interface.cpp
    src/file_sensory_interface.cpp
    src/mock_motor_interface.cpp
)

# Create the library
add_library(elizaos-embodiment STATIC ${SOURCES})

# Include directories
target_include_directories(elizaos-embodiment PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link dependencies
target_link_libraries(elizaos-embodiment
    elizaos-core
    elizaos-agentloop
    elizaos-agentmemory
    elizaos-agentaction
    elizaos-agentlogger
    Threads::Threads
)

# Compiler-specific options
if(MSVC)
    target_compile_options(elizaos-embodiment PRIVATE /W4 /WX)
else()
    target_compile_options(elizaos-embodiment PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()