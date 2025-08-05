# EasyCompletion module - C++ implementation of easy text and chat completion
add_library(elizaos-easycompletion STATIC
    src/easycompletion.cpp
)

target_include_directories(elizaos-easycompletion PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-easycompletion 
    elizaos-core
)

# Try to link libcurl if available (for HTTP requests)
if(CURL_FOUND)
    target_link_libraries(elizaos-easycompletion ${CURL_LIBRARIES})
    target_include_directories(elizaos-easycompletion PRIVATE ${CURL_INCLUDE_DIRS})
    target_compile_definitions(elizaos-easycompletion PRIVATE HAVE_CURL)
endif()

# For JSON handling - already available in the main project
target_link_libraries(elizaos-easycompletion nlohmann_json::nlohmann_json)
