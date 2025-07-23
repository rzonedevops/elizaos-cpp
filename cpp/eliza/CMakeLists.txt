# Stage 3 - Application-specific - Core Eliza Conversational AI
add_library(elizaos-eliza STATIC
    src/placeholder.cpp
)

target_include_directories(elizaos-eliza PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-eliza 
    elizaos-core
    elizaos-agentmemory
    elizaos-agentlogger
    elizaos-agentaction
    elizaos-knowledge
    elizaos-characters
    Threads::Threads
)
