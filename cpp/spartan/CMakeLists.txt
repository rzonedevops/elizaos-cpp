# Stage 3 - Spartan DeFi Trading Agent
add_library(elizaos-spartan STATIC
    src/spartan.cpp
)

target_include_directories(elizaos-spartan PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-spartan 
    elizaos-core
)
