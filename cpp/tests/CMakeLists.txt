add_executable(elizaos_tests
    src/test_core.cpp
    src/test_agentloop.cpp
    src/test_cognitive_primitives.cpp
    src/test_agentlogger.cpp
    src/test_agentcomms.cpp
    src/test_agentmemory.cpp
    src/test_attention_allocation.cpp
    src/test_agentaction.cpp
    src/test_agentagenda.cpp
    src/test_characters.cpp
    src/test_ljspeechtools.cpp
    src/test_livevideochat.cpp
    src/test_evolutionary.cpp
    src/test_stage6_automation.cpp
    src/test_knowledge.cpp
    src/test_easycompletion.cpp
    src/test_elizas_world.cpp
    src/test_registry.cpp
    test_awesome_eliza.cpp
    # src/test_embodiment.cpp  # Disabled due to compilation errors
    ../autofun_idl/tests/test_autofun_idl.cpp
    ../eliza_3d_hyperfy_starter/test_eliza_3d_hyperfy_starter.cpp
)

target_include_directories(elizaos_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(elizaos_tests
    elizaos-core
    elizaos-agentloop
    elizaos-agentmemory
    elizaos-agentlogger
    elizaos-agentcomms
    elizaos-agentaction
    elizaos-agentagenda
    elizaos-characters
    elizaos-ljspeechtools
    elizaos-livevideochat
    elizaos-evolutionary
    elizaos-knowledge
    elizaos-easycompletion
    elizaos-elizas_world
    elizaos-awesome_eliza
    elizaos-eliza_3d_hyperfy_starter
    # elizaos-embodiment  # Disabled due to compilation errors
    elizaos-plugins_automation
    elizaos-discord_summarizer
    elizaos-discrub_ext
    elizaos-autofun_idl
    elizaos-registry
    gtest_main
    gmock_main
    Threads::Threads
)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(elizaos_tests)