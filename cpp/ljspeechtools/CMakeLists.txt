# Stage 4 - Multimedia - LJSpeechTools implementation
add_library(elizaos-ljspeechtools STATIC
    src/ljspeechtools.cpp
)

target_include_directories(elizaos-ljspeechtools PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Try to find audio processing libraries
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(SNDFILE QUIET sndfile)
endif()

target_link_libraries(elizaos-ljspeechtools 
    elizaos-core
    elizaos-agentlogger
)

# Link with sndfile if found for real audio I/O
if(SNDFILE_FOUND)
    target_link_libraries(elizaos-ljspeechtools ${SNDFILE_LIBRARIES})
    target_include_directories(elizaos-ljspeechtools PRIVATE ${SNDFILE_INCLUDE_DIRS})
    target_compile_definitions(elizaos-ljspeechtools PRIVATE HAVE_SNDFILE)
    message(STATUS "LJSpeechTools: Using libsndfile for audio I/O")
else()
    message(STATUS "LJSpeechTools: Using mock implementation (libsndfile not found)")
endif()

# Future: Add other audio processing libraries as needed
# - FLAC, OGG, MP3 support
# - Real speech recognition engine 
# - Real TTS engine
