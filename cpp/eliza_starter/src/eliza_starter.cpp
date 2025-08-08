/**
 * ElizaOS C++ Starter Implementation
 * 
 * This provides a basic starter template for creating ElizaOS agents in C++.
 * It demonstrates core functionality including:
 * - Agent configuration and initialization
 * - Memory management and storage
 * - Basic conversation loops
 * - Character loading and personality traits
 * - Simple interaction patterns
 */

#include "elizaos/eliza_starter.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>

namespace elizaos {

/**
 * Implementation of ElizaStarterAgent
 */

ElizaStarterAgent::ElizaStarterAgent(const std::string& agentName, const std::string& agentId)
    : running_(false) {
    
    // Initialize logger
    logger_ = std::make_unique<AgentLogger>();
    logger_->log("Initializing ElizaStarterAgent: " + agentName + " (" + agentId + ")", 
                "ElizaStarterAgent", "Initialization", LogLevel::INFO);
    
    // Create agent configuration
    AgentConfig config;
    config.agentId = agentId;
    config.agentName = agentName;
    config.bio = "A friendly AI assistant built with ElizaOS C++";
    config.lore = "I am a demonstration agent showing how to use the ElizaOS C++ framework";
    config.adjective = "helpful";
    
    // Initialize core components
    state_ = std::make_unique<State>(config);
    memory_ = std::make_unique<AgentMemoryManager>();
    character_ = std::make_unique<CharacterManager>();
}

ElizaStarterAgent::~ElizaStarterAgent() {
    if (running_) {
        stop();
    }
    logger_->log("ElizaStarterAgent destroyed", "ElizaStarterAgent", "Cleanup", LogLevel::INFO);
}

bool ElizaStarterAgent::initialize() {
    try {
        logger_->log("Initializing ElizaStarterAgent components...", 
                    "ElizaStarterAgent", "Initialize", LogLevel::INFO);
        
        // Setup basic character traits
        setCharacter(
            "Eliza Starter",
            "A helpful AI assistant demonstrating ElizaOS C++ capabilities",
            "Born from the desire to make AI development accessible and enjoyable"
        );
        
        // Add some personality traits
        addPersonalityTrait("friendliness", "How friendly and welcoming the agent is", 0.9);
        addPersonalityTrait("helpfulness", "How eager the agent is to help users", 0.95);
        addPersonalityTrait("curiosity", "How curious the agent is about learning", 0.8);
        addPersonalityTrait("patience", "How patient the agent is with users", 0.85);
        
        // Setup the agent loop with processing steps
        std::vector<LoopStep> steps = {
            LoopStep([this](std::shared_ptr<void> input) -> std::shared_ptr<void> {
                return this->processConversation(input);
            }),
            LoopStep([this](std::shared_ptr<void> input) -> std::shared_ptr<void> {
                return this->updateMemories(input);
            }),
            LoopStep([this](std::shared_ptr<void> input) -> std::shared_ptr<void> {
                return this->checkSystemStatus(input);
            })
        };
        
        loop_ = std::make_unique<AgentLoop>(steps, true, 1.0); // Start paused, 1 second intervals
        
        logger_->log("ElizaStarterAgent initialization complete", 
                    "ElizaStarterAgent", "Initialize", LogLevel::SUCCESS);
        return true;
        
    } catch (const std::exception& e) {
        logger_->log("Failed to initialize ElizaStarterAgent: " + std::string(e.what()), 
                    "ElizaStarterAgent", "Initialize", LogLevel::ERROR);
        return false;
    }
}

void ElizaStarterAgent::start() {
    if (!loop_) {
        logger_->log("Agent loop not initialized. Call initialize() first.", 
                    "ElizaStarterAgent", "Start", LogLevel::ERROR);
        return;
    }
    
    logger_->log("Starting ElizaStarterAgent...", 
                "ElizaStarterAgent", "Start", LogLevel::INFO);
    running_ = true;
    loop_->start();
    logger_->log("ElizaStarterAgent started successfully", 
                "ElizaStarterAgent", "Start", LogLevel::SUCCESS);
}

void ElizaStarterAgent::stop() {
    if (loop_ && running_) {
        logger_->log("Stopping ElizaStarterAgent...", 
                    "ElizaStarterAgent", "Stop", LogLevel::INFO);
        running_ = false;
        loop_->stop();
        logger_->log("ElizaStarterAgent stopped", 
                    "ElizaStarterAgent", "Stop", LogLevel::SUCCESS);
    }
}

std::string ElizaStarterAgent::processMessage(const std::string& input, const std::string& userId) {
    logger_->log("Processing message from " + userId + ": " + input, 
                "ElizaStarterAgent", "ProcessMessage", LogLevel::INFO);
    
    // Add the input to memory
    addMemory(input, userId);
    
    // Generate response
    std::string response = generateResponse(input);
    
    // Add our response to memory too
    addMemory(response, state_->getAgentId());
    
    logger_->log("Generated response: " + response, 
                "ElizaStarterAgent", "ProcessMessage", LogLevel::INFO);
    return response;
}

void ElizaStarterAgent::addMemory(const std::string& content, const std::string& userId) {
    try {
        MessageMetadata metadata;
        metadata.source = userId;
        metadata.scope = MemoryScope::SHARED;
        metadata.tags = {"conversation", "starter"};
        
        auto memory = std::make_shared<Memory>(
            "mem-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()),
            content,
            userId,
            state_->getAgentId(),
            metadata
        );
        
        // Add to recent messages in state
        state_->addRecentMessage(memory);
        
        logger_->log("Added memory: " + content, 
                    "ElizaStarterAgent", "AddMemory", LogLevel::INFO);
    } catch (const std::exception& e) {
        logger_->log("Failed to add memory: " + std::string(e.what()), 
                    "ElizaStarterAgent", "AddMemory", LogLevel::ERROR);
    }
}

std::vector<std::shared_ptr<Memory>> ElizaStarterAgent::getRecentMemories(size_t count) {
    auto recent = state_->getRecentMessages();
    if (recent.size() <= count) {
        return recent;
    }
    
    // Return the most recent 'count' memories
    return std::vector<std::shared_ptr<Memory>>(recent.end() - count, recent.end());
}

void ElizaStarterAgent::setCharacter(const std::string& name, const std::string& bio, const std::string& lore) {
    if (state_) {
        // Update the state with character information
        // This would be expanded to work with the character manager
        logger_->log("Setting character: " + name, 
                    "ElizaStarterAgent", "SetCharacter", LogLevel::INFO);
        logger_->log("Bio: " + bio, 
                    "ElizaStarterAgent", "SetCharacter", LogLevel::INFO);
        logger_->log("Lore: " + lore, 
                    "ElizaStarterAgent", "SetCharacter", LogLevel::INFO);
    }
}

void ElizaStarterAgent::addPersonalityTrait(const std::string& trait, const std::string& description, double strength) {
    // Suppress unused parameter warning
    (void)description;
    
    logger_->log("Added personality trait: " + trait + " (" + std::to_string(strength) + ")", 
                "ElizaStarterAgent", "AddPersonalityTrait", LogLevel::INFO);
    // This would be expanded to work with the character system
}

std::string ElizaStarterAgent::generateResponse(const std::string& input) {
    // Simple pattern matching for demo purposes
    
    if (containsGreeting(input)) {
        return getGreetingResponse();
    }
    
    if (containsHelp(input)) {
        return getHelpResponse();
    }
    
    if (containsGoodbye(input)) {
        return getGoodbyeResponse();
    }
    
    if (containsQuestion(input)) {
        // For questions, try to be more thoughtful
        return "That's an interesting question. Based on what I understand, I'd say that " + 
               input + " is something worth exploring further. What are your thoughts on it?";
    }
    
    return getDefaultResponse();
}

// Internal processing steps for the agent loop
std::shared_ptr<void> ElizaStarterAgent::processConversation(std::shared_ptr<void> input) {
    // This would contain the main conversation processing logic
    // For now, just a placeholder that demonstrates the concept
    logger_->log("Processing conversation step", 
                "ElizaStarterAgent", "ProcessConversation", LogLevel::INFO);
    return input;
}

std::shared_ptr<void> ElizaStarterAgent::updateMemories(std::shared_ptr<void> input) {
    // This would handle memory consolidation, cleanup, etc.
    logger_->log("Updating memories step", 
                "ElizaStarterAgent", "UpdateMemories", LogLevel::INFO);
    return input;
}

std::shared_ptr<void> ElizaStarterAgent::checkSystemStatus(std::shared_ptr<void> input) {
    // This would monitor system health, performance, etc.
    logger_->log("System status check step", 
                "ElizaStarterAgent", "CheckSystemStatus", LogLevel::INFO);
    return input;
}

// Response generation helpers
std::string ElizaStarterAgent::getGreetingResponse() {
    std::vector<std::string> greetings = {
        "Hello! I'm your ElizaOS C++ assistant. How can I help you today?",
        "Hi there! Welcome to the ElizaOS C++ framework demonstration. What would you like to explore?",
        "Greetings! I'm here to show you how ElizaOS C++ works. What can I do for you?",
        "Hello! Nice to meet you. I'm a demonstration agent built with ElizaOS C++."
    };
    
    return greetings[rand() % greetings.size()];
}

std::string ElizaStarterAgent::getHelpResponse() {
    return "I'm a starter agent built with the ElizaOS C++ framework. I can:\n" +
           std::string("• Have basic conversations\n") +
           "• Remember our chat history\n" +
           "• Demonstrate core ElizaOS features\n" +
           "• Show how to build C++ AI agents\n\n" +
           "Try asking me questions or just chat with me!";
}

std::string ElizaStarterAgent::getDefaultResponse() {
    std::vector<std::string> responses = {
        "That's interesting. Tell me more about that.",
        "I see. How does that make you feel?",
        "Can you elaborate on that point?",
        "That's a fascinating perspective. What led you to that conclusion?",
        "I'm listening. Please continue.",
        "What do you think about that situation?"
    };
    
    return responses[rand() % responses.size()];
}

std::string ElizaStarterAgent::getGoodbyeResponse() {
    std::vector<std::string> goodbyes = {
        "Goodbye! Thank you for trying the ElizaOS C++ starter. Have a great day!",
        "See you later! I hope this gave you a good introduction to ElizaOS C++.",
        "Farewell! Feel free to come back anytime to explore more ElizaOS features.",
        "Until next time! Thank you for the conversation."
    };
    
    return goodbyes[rand() % goodbyes.size()];
}

// Pattern matching helpers
bool ElizaStarterAgent::containsGreeting(const std::string& input) {
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    return lower.find("hello") != std::string::npos ||
           lower.find("hi") != std::string::npos ||
           lower.find("hey") != std::string::npos ||
           lower.find("greetings") != std::string::npos ||
           lower.find("good morning") != std::string::npos ||
           lower.find("good afternoon") != std::string::npos ||
           lower.find("good evening") != std::string::npos;
}

bool ElizaStarterAgent::containsHelp(const std::string& input) {
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    return lower.find("help") != std::string::npos ||
           lower.find("assist") != std::string::npos ||
           lower.find("what can you do") != std::string::npos ||
           lower.find("how do you work") != std::string::npos;
}

bool ElizaStarterAgent::containsGoodbye(const std::string& input) {
    std::string lower = input;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    return lower.find("goodbye") != std::string::npos ||
           lower.find("bye") != std::string::npos ||
           lower.find("see you") != std::string::npos ||
           lower.find("farewell") != std::string::npos ||
           lower.find("exit") != std::string::npos ||
           lower.find("quit") != std::string::npos;
}

bool ElizaStarterAgent::containsQuestion(const std::string& input) {
    return input.find("?") != std::string::npos ||
           input.find("what") != std::string::npos ||
           input.find("how") != std::string::npos ||
           input.find("why") != std::string::npos ||
           input.find("when") != std::string::npos ||
           input.find("where") != std::string::npos ||
           input.find("who") != std::string::npos;
}

/**
 * Factory function for creating ElizaStarterAgent instances
 */
std::unique_ptr<ElizaStarterAgent> createElizaStarterAgent(const std::string& name, const std::string& id) {
    return std::make_unique<ElizaStarterAgent>(name, id);
}

/**
 * Simple interactive demo function
 */
void runInteractiveDemo() {
    std::cout << "=== ElizaOS C++ Starter Demo ===" << std::endl;
    std::cout << "Initializing agent..." << std::endl;
    
    auto agent = createElizaStarterAgent("DemoEliza", "demo-001");
    
    if (!agent->initialize()) {
        std::cerr << "Failed to initialize agent!" << std::endl;
        return;
    }
    
    agent->start();
    
    std::cout << "Agent ready! Type 'quit' to exit." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    std::string input;
    while (true) {
        std::cout << "You: ";
        std::getline(std::cin, input);
        
        if (input == "quit" || input == "exit") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        std::string response = agent->processMessage(input, "user");
        std::cout << "Eliza: " << response << std::endl << std::endl;
    }
    
    agent->stop();
    std::cout << "Demo ended. Thank you!" << std::endl;
}

} // namespace elizaos

#ifdef ELIZA_STARTER_DEMO_MAIN
/**
 * Main function for the ElizaOS C++ Starter Demo
 * Compile with -DELIZA_STARTER_DEMO_MAIN to enable
 */
int main() {
    // Initialize random seed for response variation
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    std::cout << "ElizaOS C++ Framework" << std::endl;
    std::cout << "Starter Template Demo" << std::endl;
    std::cout << "=====================" << std::endl << std::endl;
    
    try {
        elizaos::runInteractiveDemo();
    } catch (const std::exception& e) {
        std::cerr << "Demo encountered an error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
#endif
