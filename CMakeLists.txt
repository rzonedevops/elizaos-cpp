cmake_minimum_required(VERSION 3.16)

project(ElizaOS-CPP 
    VERSION 1.0.0
    DESCRIPTION "C++ implementation of ElizaOS agent framework"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
endif()

# Find packages
find_package(Threads REQUIRED)

# Find libcurl for HTTP operations
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(CURL QUIET libcurl)
endif()

if(NOT CURL_FOUND)
    find_package(CURL QUIET)
endif()

# Enable testing
enable_testing()

# Add GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add nlohmann/json for JSON parsing
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Stage 1 - Core Logic (COMPLETE)
add_subdirectory(cpp/core)           # Core data structures and interfaces
add_subdirectory(cpp/agentloop)      # Main event loop
add_subdirectory(cpp/agentmemory)    # Memory management

# Stage 5 - Learning and Adaptation (NEW)
add_subdirectory(cpp/evolutionary)   # MOSES-style evolutionary search

# Stage 6 - Embodiment & Integration (NEW)
add_subdirectory(cpp/embodiment)     # Sensory/motor interfaces and perception-action loop

# Stage 2 - Infrastructure (COMPLETE + new additions)
add_subdirectory(cpp/agentcomms)     # Communications
add_subdirectory(cpp/agentlogger)    # Logging system
add_subdirectory(cpp/agentshell)     # Shell interface

# Stage 3 - Application-specific modules
add_subdirectory(cpp/agentaction)    # Action processing
add_subdirectory(cpp/agentagenda)    # Task scheduling
add_subdirectory(cpp/agentbrowser)   # Browser automation
add_subdirectory(cpp/auto_fun)       # Auto.fun integration
add_subdirectory(cpp/autofun_idl)    # Auto.fun IDL
add_subdirectory(cpp/autonomous_starter)     # Autonomous starter
add_subdirectory(cpp/awesome_eliza)  # Awesome Eliza resources
add_subdirectory(cpp/brandkit)       # Brand kit resources
add_subdirectory(cpp/characterfile)  # Character file handling
add_subdirectory(cpp/characters)     # Character management
add_subdirectory(cpp/easycompletion) # Easy completion utilities
add_subdirectory(cpp/eliza)          # Core Eliza implementation
add_subdirectory(cpp/eliza_3d_hyperfy_starter)   # 3D Hyperfy starter
add_subdirectory(cpp/eliza_nextjs_starter)       # Next.js starter
add_subdirectory(cpp/eliza_plugin_starter)       # Plugin starter
add_subdirectory(cpp/eliza_starter)  # Basic Eliza starter
add_subdirectory(cpp/elizas_list)    # Eliza's list management
add_subdirectory(cpp/elizas_world)   # Eliza's world features
add_subdirectory(cpp/hat)            # HAT protocol
add_subdirectory(cpp/hats)           # HATs protocol
add_subdirectory(cpp/knowledge)      # Knowledge management
add_subdirectory(cpp/plugin_specification)      # Plugin specifications
add_subdirectory(cpp/registry)       # Registry services
add_subdirectory(cpp/spartan)        # Spartan features
add_subdirectory(cpp/the_org)        # Organization management
add_subdirectory(cpp/trust_scoreboard)          # Trust scoreboard
add_subdirectory(cpp/workgroups)     # Workgroup management

# Stage 4 - Multimedia modules
add_subdirectory(cpp/ljspeechtools)  # Speech processing
add_subdirectory(cpp/livevideochat)  # Video chat

# Stage 5 - Web and Documentation modules
add_subdirectory(cpp/elizaos_github_io)         # GitHub.io site
add_subdirectory(cpp/vercel_api)     # Vercel API integration
add_subdirectory(cpp/website)        # Website functionality

# Stage 6 - Tools and Automation modules
add_subdirectory(cpp/discord_summarizer)        # Discord summarizer
add_subdirectory(cpp/discrub_ext)    # Discrub extension
add_subdirectory(cpp/plugins_automation)        # Plugin automation

# Testing
add_subdirectory(cpp/tests)

# Stage 4 Demo
add_executable(stage4_demo
    demo_stage4.cpp
)

target_include_directories(stage4_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(stage4_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-ljspeechtools
    elizaos-livevideochat
    elizaos-evolutionary
    elizaos-embodiment
    Threads::Threads
)

# Stage 5 Demo
add_executable(stage5_demo
    demo_stage5.cpp
)

target_include_directories(stage5_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(stage5_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-evolutionary
    elizaos-website
    elizaos-elizaos_github_io
    elizaos-vercel_api
    elizaos-embodiment
    Threads::Threads
)

# Stage 6 Demo
add_executable(stage6_demo
    demo_stage6.cpp
)

target_include_directories(stage6_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(stage6_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-agentloop
    elizaos-agentmemory
    elizaos-agentaction
    elizaos-embodiment
    Threads::Threads
)

# Awesome Eliza Demo
add_executable(awesome_eliza_demo
    awesome_eliza_demo.cpp
)

target_include_directories(awesome_eliza_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(awesome_eliza_demo
    elizaos-core
    elizaos-awesome_eliza
    Threads::Threads
)

# Main Eliza CLI executable
add_executable(eliza
    eliza_main.cpp
)

target_include_directories(eliza PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(eliza
    elizaos-core
    elizaos-agentlogger
    elizaos-agentloop
    elizaos-agentmemory
    elizaos-eliza
    elizaos-knowledge
    elizaos-characters
    Threads::Threads
)

# EasyCompletion Demo
add_executable(easycompletion_demo
    easycompletion_demo.cpp
)

target_include_directories(easycompletion_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(easycompletion_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-easycompletion
    Threads::Threads
)

# Characters Demo
add_executable(characters_demo
    characters_demo.cpp
)

target_include_directories(characters_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(characters_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-agentmemory
    elizaos-characters
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# Knowledge Demo
add_executable(knowledge_demo
    knowledge_demo.cpp
)

target_include_directories(knowledge_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(knowledge_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-agentmemory
    elizaos-agentaction
    elizaos-knowledge
    Threads::Threads
)

# Eliza's List Demo
add_executable(elizas_list_demo
    elizas_list_demo.cpp
)

target_include_directories(elizas_list_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizas_list_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-elizas_list
    nlohmann_json::nlohmann_json
    Threads::Threads
)
add_executable(knowledge_quick_demo
    knowledge_quick_demo.cpp
)

target_include_directories(knowledge_quick_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(knowledge_quick_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-agentmemory
    elizaos-agentaction
    elizaos-knowledge
    Threads::Threads
)

# Eliza 3D Hyperfy Starter Demo
add_executable(eliza_3d_hyperfy_starter_demo
    eliza_3d_hyperfy_starter_demo.cpp
)

target_include_directories(eliza_3d_hyperfy_starter_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(eliza_3d_hyperfy_starter_demo
    elizaos-core
    elizaos-agentlogger
    elizaos-eliza_3d_hyperfy_starter
    Threads::Threads
)

# Add example executable (optional)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation - install all libraries and executables
install(TARGETS 
    # Main executables
    eliza
    
    # Stage 1-2 - Core and Infrastructure
    elizaos-core elizaos-agentloop elizaos-agentmemory
    elizaos-agentcomms elizaos-agentlogger elizaos-agentshell
    
    # Stage 5 - Learning and Adaptation
    elizaos-evolutionary
    
    # Stage 6 - Embodiment & Integration
    elizaos-embodiment
    
    # Stage 3 - Application-specific
    elizaos-agentaction elizaos-agentagenda elizaos-agentbrowser
    elizaos-auto_fun elizaos-autofun_idl elizaos-autonomous_starter
    elizaos-awesome_eliza elizaos-brandkit elizaos-characterfile
    elizaos-characters elizaos-easycompletion elizaos-eliza
    elizaos-eliza_3d_hyperfy_starter elizaos-eliza_nextjs_starter
    elizaos-eliza_plugin_starter elizaos-eliza_starter
    elizaos-elizas_list elizaos-elizas_world elizaos-hat elizaos-hats
    elizaos-knowledge elizaos-plugin_specification elizaos-registry
    elizaos-spartan elizaos-the_org elizaos-trust_scoreboard
    elizaos-workgroups
    
    # Stage 4 - Multimedia
    elizaos-ljspeechtools elizaos-livevideochat
    
    # Stage 5 - Web and Documentation
    elizaos-elizaos_github_io elizaos-vercel_api elizaos-website
    
    # Stage 6 - Tools and Automation
    elizaos-discord_summarizer elizaos-discrub_ext elizaos-plugins_automation
    
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Generate package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/ElizaOSConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Print configuration summary
message(STATUS "ElizaOS-CPP Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
if(CURL_FOUND)
    message(STATUS "  libcurl: Found")
else()
    message(STATUS "  libcurl: Not found (HTTP functionality will be limited)")
endif()