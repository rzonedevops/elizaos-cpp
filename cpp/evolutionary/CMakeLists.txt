add_library(elizaos-evolutionary STATIC
    src/evolutionary.cpp
    src/pattern_extraction.cpp
    src/adaptation_hooks_simple.cpp
)

target_include_directories(elizaos-evolutionary PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-evolutionary 
    elizaos-core
    Threads::Threads
)