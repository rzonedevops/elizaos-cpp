/**
 * Simple example demonstrating how to use the ElizaOS C++ Starter
 * 
 * This example shows:
 * - Creating and configuring an agent
 * - Processing messages
 * - Retrieving memories
 * - Basic conversation flow
 */

#include "elizaos/eliza_starter.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "ElizaOS C++ Starter Example" << std::endl;
    std::cout << "============================" << std::endl << std::endl;

    // Create a new agent
    std::cout << "Creating agent..." << std::endl;
    auto agent = elizaos::createElizaStarterAgent("ExampleAgent", "example-001");

    // Configure the agent's character
    agent->setCharacter(
        "Alex the Helper",
        "A knowledgeable and friendly AI assistant",
        "Created to demonstrate the capabilities of the ElizaOS C++ framework"
    );

    // Add some personality traits
    agent->addPersonalityTrait("helpfulness", "Eager to assist users", 0.9);
    agent->addPersonalityTrait("friendliness", "Warm and welcoming", 0.85);
    agent->addPersonalityTrait("knowledge", "Well-informed about various topics", 0.8);

    // Initialize and start the agent
    if (!agent->initialize()) {
        std::cerr << "Failed to initialize agent!" << std::endl;
        return 1;
    }

    agent->start();
    std::cout << "Agent initialized and started successfully!" << std::endl << std::endl;

    // Simulate a conversation
    std::cout << "=== Simulated Conversation ===" << std::endl;
    
    struct ConversationTurn {
        std::string user;
        std::string message;
    };

    std::vector<ConversationTurn> conversation = {
        {"Alice", "Hello there!"},
        {"Alice", "I'm new to C++ programming. Can you help me?"},
        {"Bob", "Hi! What can this agent do?"},
        {"Alice", "What's the difference between ElizaOS and other AI frameworks?"},
        {"Bob", "How does the memory system work?"},
        {"Alice", "Thank you for your help!"}
    };

    for (const auto& turn : conversation) {
        std::cout << turn.user << ": " << turn.message << std::endl;
        
        std::string response = agent->processMessage(turn.message, turn.user);
        std::cout << "Agent: " << response << std::endl << std::endl;
    }

    // Show memory retrieval
    std::cout << "=== Recent Memories ===" << std::endl;
    auto memories = agent->getRecentMemories(5);
    
    int count = 1;
    for (const auto& memory : memories) {
        std::cout << count++ << ". " << memory->getContent() << std::endl;
    }

    std::cout << std::endl << "Total memories: " << memories.size() << std::endl;

    // Clean shutdown
    agent->stop();
    std::cout << std::endl << "Example completed successfully!" << std::endl;

    return 0;
}