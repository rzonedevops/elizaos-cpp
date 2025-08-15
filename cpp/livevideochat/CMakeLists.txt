# Stage 4 - Multimedia - LiveVideoChat implementation with server support
add_library(elizaos-livevideochat STATIC
    src/placeholder.cpp
    src/livevideochat_server.cpp
)

target_include_directories(elizaos-livevideochat PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-livevideochat 
    elizaos-core
    elizaos-agentlogger
    Threads::Threads
)
