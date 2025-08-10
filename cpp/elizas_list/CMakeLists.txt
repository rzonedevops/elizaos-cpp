# Stage 3 - Application-specific - Eliza's List project management
add_library(elizaos-elizas_list STATIC
    src/elizas_list.cpp
)

target_include_directories(elizaos-elizas_list PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-elizas_list 
    elizaos-core
    nlohmann_json::nlohmann_json
)
