# Stage 3 - Application-specific - Knowledge Management System
add_library(elizaos-knowledge STATIC
    src/placeholder.cpp
)

target_include_directories(elizaos-knowledge PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-knowledge 
    elizaos-core
    elizaos-agentmemory
    elizaos-agentlogger
    elizaos-agentaction
    Threads::Threads
)
