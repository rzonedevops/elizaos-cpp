# Stage 3 - Application-specific placeholder - implementation planned
add_library(elizaos-plugin_specification STATIC
    src/placeholder.cpp
)

target_include_directories(elizaos-plugin_specification PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-plugin_specification 
    elizaos-core
    ${CMAKE_DL_LIBS}
)
