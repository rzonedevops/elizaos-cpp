# Stage 5 - Web and Documentation - Vercel API module
add_library(elizaos-vercel_api STATIC
    src/placeholder.cpp
)

target_include_directories(elizaos-vercel_api PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-vercel_api 
    elizaos-core
    elizaos-agentlogger
    nlohmann_json::nlohmann_json
)

# Link curl if available
if(CURL_FOUND)
    if(TARGET CURL::libcurl)
        target_link_libraries(elizaos-vercel_api CURL::libcurl)
        target_compile_definitions(elizaos-vercel_api PRIVATE CURL_FOUND)
    elseif(CURL_LIBRARIES)
        target_link_libraries(elizaos-vercel_api ${CURL_LIBRARIES})
        target_include_directories(elizaos-vercel_api PRIVATE ${CURL_INCLUDE_DIRS})
        target_compile_definitions(elizaos-vercel_api PRIVATE CURL_FOUND)
    endif()
endif()

# Add test executable
add_executable(test_vercel_api test_vercel_api.cpp)
target_link_libraries(test_vercel_api elizaos-vercel_api)

# Add demo executable
add_executable(demo_vercel_api demo_vercel_api.cpp)
target_link_libraries(demo_vercel_api elizaos-vercel_api)

# Add test to CTest
add_test(NAME VercelAPITest COMMAND test_vercel_api)
