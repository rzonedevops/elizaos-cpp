# Stage 3 - Registry system for plugin discovery and management
add_library(elizaos-registry STATIC
    src/registry.cpp
)

target_include_directories(elizaos-registry PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-registry 
    elizaos-core
    elizaos-agentlogger
    elizaos-plugins_automation
    nlohmann_json::nlohmann_json
)

# Add libcurl if available for HTTP registry access
if(CURL_FOUND)
    target_link_libraries(elizaos-registry ${CURL_LIBRARIES})
    target_include_directories(elizaos-registry PRIVATE ${CURL_INCLUDE_DIRS})
    target_compile_definitions(elizaos-registry PRIVATE LIBCURL_FOUND)
endif()
