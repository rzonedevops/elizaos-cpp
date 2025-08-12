# Stage 3 - HATs protocol for multiple data sources
add_library(elizaos-hats STATIC
    src/hats.cpp
)

target_include_directories(elizaos-hats PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos-hats 
    elizaos-core
)

# Add test executable
if(BUILD_TESTING)
    add_executable(test_hats test/test_hats.cpp)
    target_link_libraries(test_hats
        elizaos-hats
        elizaos-core
        gtest
        gtest_main
    )
    add_test(NAME HatsTest COMMAND test_hats)
endif()
