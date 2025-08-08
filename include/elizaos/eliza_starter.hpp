#pragma once

/**
 * ElizaOS C++ Starter Template
 * 
 * This header provides a simple starting point for creating ElizaOS agents in C++.
 * It includes a basic agent class that demonstrates core functionality and 
 * provides a foundation for building more sophisticated agents.
 * 
 * Key Features:
 * - Simple agent setup and initialization
 * - Basic conversation handling
 * - Memory management integration
 * - Character and personality configuration
 * - Interactive demo capabilities
 */

#include "elizaos/core.hpp"
#include "elizaos/agentloop.hpp"
#include "elizaos/agentmemory.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/characters.hpp"
#include <string>
#include <vector>
#include <memory>

namespace elizaos {

/**
 * ElizaStarterAgent - A simple demonstration agent class
 * 
 * This class provides a basic implementation of an Eliza-style conversational agent
 * using the ElizaOS C++ framework. It demonstrates:
 * 
 * - Agent initialization and configuration
 * - Basic conversation processing
 * - Memory storage and retrieval  
 * - Simple pattern matching for responses
 * - Integration with the AgentLoop system
 * 
 * Usage:
 *   auto agent = createElizaStarterAgent("MyAgent", "agent-001");
 *   agent->initialize();
 *   agent->start();
 *   std::string response = agent->processMessage("Hello!", "user-123");
 */
class ElizaStarterAgent {
private:
    std::unique_ptr<AgentLogger> logger_;
    std::unique_ptr<AgentMemoryManager> memory_;
    std::unique_ptr<State> state_;
    std::unique_ptr<CharacterManager> character_;
    std::unique_ptr<AgentLoop> loop_;
    bool running_;
    
public:
    /**
     * Constructor - Creates a new ElizaStarterAgent
     * @param agentName Display name for the agent
     * @param agentId Unique identifier for the agent
     */
    ElizaStarterAgent(const std::string& agentName = "ElizaStarter", 
                     const std::string& agentId = "eliza-starter-001");
    
    /**
     * Destructor - Clean shutdown of agent resources
     */
    ~ElizaStarterAgent();
    
    // === Core Lifecycle Methods ===
    
    /**
     * Initialize the agent and all its components
     * @return true if initialization successful, false otherwise
     */
    bool initialize();
    
    /**
     * Start the agent loop and begin processing
     */
    void start();
    
    /**
     * Stop the agent loop and cleanup resources
     */
    void stop();
    
    /**
     * Check if the agent is currently running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_; }
    
    // === Interaction Methods ===
    
    /**
     * Process a message from a user and generate a response
     * @param input The user's message
     * @param userId Identifier for the user (defaults to "user")
     * @return Generated response string
     */
    std::string processMessage(const std::string& input, const std::string& userId = "user");
    
    /**
     * Add a memory/message to the agent's memory system
     * @param content The content to remember
     * @param userId The user/source of the content
     */
    void addMemory(const std::string& content, const std::string& userId);
    
    /**
     * Retrieve recent memories from the agent
     * @param count Maximum number of memories to retrieve (default 10)
     * @return Vector of recent Memory objects
     */
    std::vector<std::shared_ptr<Memory>> getRecentMemories(size_t count = 10);
    
    // === Configuration Methods ===
    
    /**
     * Set the agent's character information
     * @param name Character name
     * @param bio Character biography/description
     * @param lore Character background story
     */
    void setCharacter(const std::string& name, const std::string& bio, const std::string& lore);
    
    /**
     * Add a personality trait to the agent
     * @param trait Name of the trait
     * @param description Description of the trait
     * @param strength Strength of the trait (0.0 to 1.0)
     */
    void addPersonalityTrait(const std::string& trait, const std::string& description, double strength);
    
    /**
     * Generate a response to user input using simple pattern matching
     * @param input The user's input message
     * @return Generated response string
     */
    std::string generateResponse(const std::string& input);
    
private:
    // Internal processing methods for the agent loop
    std::shared_ptr<void> processConversation(std::shared_ptr<void> input);
    std::shared_ptr<void> updateMemories(std::shared_ptr<void> input);
    std::shared_ptr<void> checkSystemStatus(std::shared_ptr<void> input);
    
    // Response generation helpers
    std::string getGreetingResponse();
    std::string getHelpResponse();
    std::string getDefaultResponse();
    std::string getGoodbyeResponse();
    
    // Pattern matching helpers
    bool containsGreeting(const std::string& input);
    bool containsHelp(const std::string& input);
    bool containsGoodbye(const std::string& input);
    bool containsQuestion(const std::string& input);
};

/**
 * Factory function for creating ElizaStarterAgent instances
 * @param name Agent name
 * @param id Agent ID
 * @return Unique pointer to new ElizaStarterAgent
 */
std::unique_ptr<ElizaStarterAgent> createElizaStarterAgent(const std::string& name, const std::string& id);

/**
 * Run an interactive demo of the ElizaStarterAgent
 * This function creates an agent, initializes it, and provides a command-line
 * interface for chatting with the agent. Useful for testing and demonstration.
 */
void runInteractiveDemo();

/**
 * Example usage patterns and code snippets
 * 
 * Basic Usage:
 * ```cpp
 * #include "elizaos/eliza_starter.hpp"
 * 
 * int main() {
 *     // Create and initialize agent
 *     auto agent = elizaos::createElizaStarterAgent("MyBot", "bot-001");
 *     if (!agent->initialize()) {
 *         std::cerr << "Failed to initialize agent" << std::endl;
 *         return 1;
 *     }
 *     
 *     agent->start();
 *     
 *     // Process some messages
 *     std::string response1 = agent->processMessage("Hello there!", "user1");
 *     std::string response2 = agent->processMessage("How are you?", "user1");
 *     std::string response3 = agent->processMessage("Can you help me?", "user2");
 *     
 *     agent->stop();
 *     return 0;
 * }
 * ```
 * 
 * Advanced Configuration:
 * ```cpp
 * auto agent = elizaos::createElizaStarterAgent("CustomBot", "custom-001");
 * 
 * // Configure character
 * agent->setCharacter(
 *     "Dr. Helper",
 *     "A knowledgeable assistant specializing in technical support",
 *     "Created to help users solve technical problems with patience and expertise"
 * );
 * 
 * // Add personality traits
 * agent->addPersonalityTrait("technical_knowledge", "Expert in technology", 0.95);
 * agent->addPersonalityTrait("patience", "Very patient with users", 0.9);
 * agent->addPersonalityTrait("helpfulness", "Eager to help", 0.85);
 * 
 * agent->initialize();
 * agent->start();
 * ```
 */

} // namespace elizaos