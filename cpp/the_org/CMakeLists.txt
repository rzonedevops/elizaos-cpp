# Stage 3 - Application-specific - Full the_org multi-agent system implementation
add_library(elizaos-the_org STATIC
    src/the_org.cpp
    src/placeholder.cpp
)

target_include_directories(elizaos-the_org PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-the_org 
    elizaos-core
    elizaos-agentlogger
    elizaos-agentmemory
    elizaos-agentloop
    Threads::Threads
)
