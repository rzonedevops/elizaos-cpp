# ElizaOS C++ Starter - Functional implementation demonstrating core framework usage
add_library(elizaos-eliza_starter STATIC
    src/eliza_starter.cpp
)

target_include_directories(elizaos-eliza_starter PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-eliza_starter 
    elizaos-core
    elizaos-agentloop
    elizaos-agentmemory
    elizaos-agentlogger
    elizaos-characters
)

# Interactive demonstration executable
add_executable(eliza_starter_demo
    src/eliza_starter.cpp
)

target_include_directories(eliza_starter_demo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(eliza_starter_demo
    elizaos-core
    elizaos-agentloop
    elizaos-agentmemory  
    elizaos-agentlogger
    elizaos-characters
    Threads::Threads
)

# Add a main function for the demo
target_compile_definitions(eliza_starter_demo PRIVATE ELIZA_STARTER_DEMO_MAIN)

# Simple usage example executable
add_executable(eliza_starter_example
    example.cpp
)

target_include_directories(eliza_starter_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(eliza_starter_example
    elizaos-eliza_starter
    Threads::Threads
)
