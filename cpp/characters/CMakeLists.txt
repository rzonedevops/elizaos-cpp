# Stage 3 - Application-specific - Character Management System
add_library(elizaos-characters STATIC
    src/placeholder.cpp
)

target_include_directories(elizaos-characters PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-characters 
    elizaos-core
    elizaos-agentmemory
    elizaos-agentlogger
    elizaos-agentaction
    Threads::Threads
)
