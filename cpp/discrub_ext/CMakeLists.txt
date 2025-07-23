# Stage 6 - Tools and Automation - Discrub Extension
add_library(elizaos-discrub_ext STATIC
    src/discrub_ext.cpp
)

target_include_directories(elizaos-discrub_ext PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-discrub_ext 
    elizaos-core
    elizaos-agentlogger
    elizaos-discord_summarizer
    Threads::Threads
)
