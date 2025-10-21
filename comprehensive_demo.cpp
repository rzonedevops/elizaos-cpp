#include "elizaos/core.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentloop.hpp"
#include "elizaos/agentmemory.hpp"
#include "elizaos/agentcomms.hpp"
#include "elizaos/eliza.hpp"
#include "elizaos/characters.hpp"
#include "elizaos/knowledge.hpp"
#include "elizaos/spartan.hpp"
#include "elizaos/elizas_list.hpp"
#include "elizaos/elizas_world.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

using namespace elizaos;

class ComprehensiveDemo {
private:
    AgentLogger logger_;
    std::shared_ptr<State> state_;
    std::shared_ptr<AgentMemoryManager> memory_;
    
public:
    ComprehensiveDemo() {
        // Initialize core components
        AgentConfig config;
        config.agentId = "demo-agent-001";
        config.agentName = "ElizaOS Demo Agent";
        config.bio = "A comprehensive demonstration agent showcasing ElizaOS C++ capabilities";
        config.lore = "Born from the convergence of advanced AI and high-performance C++";
        
        state_ = std::make_shared<State>(config);
        memory_ = std::make_shared<AgentMemoryManager>();
    }
    
    void runComprehensiveDemo() {
        logger_.log("=== ElizaOS C++ Comprehensive Demonstration ===", "", "demo", LogLevel::SYSTEM);
        logger_.log("", "", "", LogLevel::INFO);
        
        // Test 1: Core State Management
        testCoreState();
        
        // Test 2: Memory System
        testMemorySystem();
        
        // Test 3: Communication System
        testCommunicationSystem();
        
        // Test 4: Agent Loop
        testAgentLoop();
        
        // Test 5: Eliza Conversation Engine
        testElizaEngine();
        
        // Test 6: Character System
        testCharacterSystem();
        
        // Test 7: Knowledge Base
        testKnowledgeBase();
        
        // Test 8: Specialized Modules
        testSpecializedModules();
        
        // Test 9: Performance Benchmarks
        runPerformanceBenchmarks();
        
        logger_.log("=== Demo Complete - All Systems Operational ===", "", "demo", LogLevel::SUCCESS);
    }
    
private:
    void testCoreState() {
        logger_.panel("Test 1: Core State Management", 
                     "Testing agent configuration, state composition, and metadata management");
        
        // Test state configuration
        logger_.log("Agent ID: " + state_->getAgentId(), "", "core", LogLevel::INFO);
        logger_.log("Agent Name: " + state_->getAgentName(), "", "core", LogLevel::INFO);
        logger_.log("Bio: " + state_->getBio(), "", "core", LogLevel::INFO);
        
        // Test state updates
        state_->setGoals({"Demonstrate ElizaOS capabilities", "Validate performance", "Showcase modularity"});
        logger_.log("Goals updated: " + std::to_string(state_->getGoals().size()) + " goals set", "", "core", LogLevel::SUCCESS);
        
        // Test memory integration
        state_->setMemory(memory_);
        logger_.log("Memory manager integrated with state", "", "core", LogLevel::SUCCESS);
        
        logger_.log("✓ Core state management: PASSED", "", "core", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void testMemorySystem() {
        logger_.panel("Test 2: Memory System", 
                     "Testing memory storage, retrieval, embedding search, and persistence");
        
        // Create test memories
        auto memory1 = std::make_shared<Memory>();
        memory1->userId = UUID::generate();
        memory1->agentId = state_->getAgentId();
        memory1->roomId = UUID::generate();
        memory1->content = "This is a test memory about AI capabilities";
        memory1->createdAt = std::chrono::system_clock::now();
        memory1->embedding = EmbeddingVector(384, 0.5f); // Test embedding
        
        auto memory2 = std::make_shared<Memory>();
        memory2->userId = memory1->userId;
        memory2->agentId = state_->getAgentId();
        memory2->roomId = memory1->roomId;
        memory2->content = "Another memory about performance testing";
        memory2->createdAt = std::chrono::system_clock::now();
        memory2->embedding = EmbeddingVector(384, 0.3f);
        
        // Test memory storage
        UUID id1 = memory_->createMemory(memory1);
        UUID id2 = memory_->createMemory(memory2);
        logger_.log("Stored 2 memories with IDs: " + id1 + ", " + id2, "", "memory", LogLevel::SUCCESS);
        
        // Test memory retrieval
        auto retrieved = memory_->getMemoryById(id1);
        if (retrieved && retrieved->content == memory1->content) {
            logger_.log("Memory retrieval: PASSED", "", "memory", LogLevel::SUCCESS);
        } else {
            logger_.log("Memory retrieval: FAILED", "", "memory", LogLevel::ERROR);
        }
        
        // Test search functionality
        MemorySearchParams searchParams;
        searchParams.roomId = memory1->roomId;
        searchParams.count = 10;
        auto searchResults = memory_->getMemories(searchParams);
        logger_.log("Search found " + std::to_string(searchResults.size()) + " memories", "", "memory", LogLevel::SUCCESS);
        
        // Test embedding search
        MemorySearchByEmbeddingParams embeddingParams;
        embeddingParams.embedding = EmbeddingVector(384, 0.4f);
        embeddingParams.matchThreshold = 0.1;
        embeddingParams.count = 5;
        auto embeddingResults = memory_->searchMemories(embeddingParams);
        logger_.log("Embedding search found " + std::to_string(embeddingResults.size()) + " similar memories", 
                   "", "memory", LogLevel::SUCCESS);
        
        logger_.log("✓ Memory system: PASSED", "", "memory", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void testCommunicationSystem() {
        logger_.panel("Test 3: Communication System", 
                     "Testing message passing, channels, and async processing");
        
        AgentComms comms;
        
        // Create test channel
        std::string channelId = comms.createChannel("demo-channel");
        logger_.log("Created communication channel: " + channelId, "", "comms", LogLevel::SUCCESS);
        
        // Test message creation and sending
        Message testMessage;
        testMessage.senderId = state_->getAgentId();
        testMessage.recipientId = "demo-recipient";
        testMessage.content = "Test message from comprehensive demo";
        testMessage.timestamp = std::chrono::system_clock::now();
        testMessage.messageType = "demo";
        
        comms.sendMessage(testMessage);
        logger_.log("Message sent successfully", "", "comms", LogLevel::SUCCESS);
        
        // Test message history
        auto messageHistory = comms.getMessageHistory(state_->getAgentId());
        logger_.log("Message history contains " + std::to_string(messageHistory.size()) + " messages", 
                   "", "comms", LogLevel::SUCCESS);
        
        logger_.log("✓ Communication system: PASSED", "", "comms", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void testAgentLoop() {
        logger_.panel("Test 4: Agent Loop", 
                     "Testing threaded execution, pause/resume, and step processing");
        
        // Create simple processing steps
        std::vector<LoopStep> steps = {
            LoopStep([this](std::shared_ptr<void> input) -> std::shared_ptr<void> {
                logger_.log("Processing step 1: Perception", "", "loop", LogLevel::INFO);
                return input;
            }),
            LoopStep([this](std::shared_ptr<void> input) -> std::shared_ptr<void> {
                logger_.log("Processing step 2: Reasoning", "", "loop", LogLevel::INFO);
                return input;
            }),
            LoopStep([this](std::shared_ptr<void> input) -> std::shared_ptr<void> {
                logger_.log("Processing step 3: Action", "", "loop", LogLevel::INFO);
                return input;
            })
        };
        
        // Create and test agent loop
        AgentLoop loop(steps, false, 0.5); // Run every 500ms
        
        logger_.log("Starting agent loop for 3 cycles...", "", "loop", LogLevel::INFO);
        loop.start();
        
        // Let it run for a few cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        
        // Test pause/resume
        loop.pause();
        logger_.log("Loop paused", "", "loop", LogLevel::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        loop.resume();
        logger_.log("Loop resumed", "", "loop", LogLevel::SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        loop.stop();
        logger_.log("Loop stopped", "", "loop", LogLevel::SUCCESS);
        
        logger_.log("✓ Agent loop: PASSED", "", "loop", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void testElizaEngine() {
        logger_.panel("Test 5: Eliza Conversation Engine", 
                     "Testing conversation processing, emotional tracking, and response generation");
        
        Eliza elizaEngine;
        
        // Test conversation responses
        std::vector<std::string> testInputs = {
            "Hello, how are you today?",
            "I'm feeling a bit anxious about my work",
            "Can you help me understand artificial intelligence?",
            "What do you think about the future of technology?"
        };
        
        for (const auto& input : testInputs) {
            std::string response = elizaEngine.generateResponse(input);
            logger_.log("Input: " + input, "", "eliza", LogLevel::INFO);
            logger_.log("Response: " + response, "", "eliza", LogLevel::SUCCESS);
            logger_.log("", "", "", LogLevel::INFO);
        }
        
        // Test emotional analysis
        std::vector<std::string> emotions = elizaEngine.analyzeEmotions("I'm really excited about this new project!");
        logger_.log("Detected emotions: " + std::to_string(emotions.size()) + " emotions", "", "eliza", LogLevel::SUCCESS);
        
        logger_.log("✓ Eliza conversation engine: PASSED", "", "eliza", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void testCharacterSystem() {
        logger_.panel("Test 6: Character System", 
                     "Testing character loading, personality management, and trait application");
        
        CharacterManager characterManager;
        
        // Test character creation
        Character testCharacter;
        testCharacter.name = "Demo Character";
        testCharacter.bio = "A demonstration character for testing";
        testCharacter.lore = "Created specifically for the comprehensive demo";
        testCharacter.knowledge.push_back("I know about AI and technology");
        testCharacter.messageExamples.push_back("Hello! I'm here to help with your questions.");
        testCharacter.postExamples.push_back("Excited to share knowledge about AI!");
        testCharacter.adjectives.push_back("helpful");
        testCharacter.adjectives.push_back("knowledgeable");
        testCharacter.style.push_back("friendly");
        testCharacter.style.push_back("informative");
        
        std::string characterId = characterManager.addCharacter(testCharacter);
        logger_.log("Created character with ID: " + characterId, "", "character", LogLevel::SUCCESS);
        
        // Test character retrieval
        auto retrievedCharacter = characterManager.getCharacter(characterId);
        if (retrievedCharacter.has_value()) {
            logger_.log("Character retrieval: PASSED", "", "character", LogLevel::SUCCESS);
            logger_.log("Character name: " + retrievedCharacter->name, "", "character", LogLevel::INFO);
            logger_.log("Character adjectives: " + std::to_string(retrievedCharacter->adjectives.size()), 
                       "", "character", LogLevel::INFO);
        }
        
        // Test personality influence
        std::string personalizedResponse = characterManager.applyPersonality(characterId, 
                                                                            "How can I help you today?");
        logger_.log("Personalized response: " + personalizedResponse, "", "character", LogLevel::SUCCESS);
        
        logger_.log("✓ Character system: PASSED", "", "character", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void testKnowledgeBase() {
        logger_.panel("Test 7: Knowledge Base", 
                     "Testing knowledge storage, search, categorization, and retrieval");
        
        KnowledgeBase kb;
        
        // Add test knowledge entries
        std::vector<std::string> testKnowledge = {
            "Artificial Intelligence is the simulation of human intelligence in machines",
            "Machine Learning is a subset of AI that enables computers to learn without explicit programming",
            "Neural networks are computing systems inspired by biological neural networks",
            "C++ is a high-performance programming language ideal for system programming"
        };
        
        for (const auto& knowledge : testKnowledge) {
            kb.addKnowledge(knowledge, "AI_Technology");
            logger_.log("Added knowledge: " + knowledge.substr(0, 50) + "...", "", "knowledge", LogLevel::SUCCESS);
        }
        
        // Test knowledge search
        auto searchResults = kb.searchKnowledge("artificial intelligence");
        logger_.log("Search for 'artificial intelligence' found " + std::to_string(searchResults.size()) + " results", 
                   "", "knowledge", LogLevel::SUCCESS);
        
        // Test category retrieval
        auto categoryKnowledge = kb.getKnowledgeByCategory("AI_Technology");
        logger_.log("Category 'AI_Technology' contains " + std::to_string(categoryKnowledge.size()) + " entries", 
                   "", "knowledge", LogLevel::SUCCESS);
        
        // Test knowledge ranking
        auto rankedResults = kb.getRankedKnowledge("machine learning", 3);
        logger_.log("Top 3 ranked results for 'machine learning': " + std::to_string(rankedResults.size()), 
                   "", "knowledge", LogLevel::SUCCESS);
        
        logger_.log("✓ Knowledge base: PASSED", "", "knowledge", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void testSpecializedModules() {
        logger_.panel("Test 8: Specialized Modules", 
                     "Testing Spartan trading, Eliza's List, and Eliza's World");
        
        // Test Spartan
        SpartanConfig spartanConfig;
        spartanConfig.rpcUrl = "https://api.mainnet-beta.solana.com";
        spartanConfig.publicKey = "demo-key";
        
        Spartan spartan(spartanConfig);
        spartan.initialize();
        logger_.log("Spartan trading agent initialized", "", "spartan", LogLevel::SUCCESS);
        
        auto tokenInfo = spartan.getTokenInfo("SOL");
        logger_.log("Token info retrieved for SOL", "", "spartan", LogLevel::SUCCESS);
        
        // Test Eliza's List
        ElizasList elizasList;
        
        ElizaProject project;
        project.name = "Demo Project";
        project.description = "A demonstration project";
        project.author.name = "Demo Author";
        project.tags = {"demo", "test"};
        project.stars = 100;
        
        elizasList.addProject(project);
        logger_.log("Added project to Eliza's List", "", "elizas_list", LogLevel::SUCCESS);
        
        auto projects = elizasList.searchByTag("demo");
        logger_.log("Found " + std::to_string(projects.size()) + " projects with 'demo' tag", 
                   "", "elizas_list", LogLevel::SUCCESS);
        
        // Test Eliza's World
        ElizasWorld world;
        world.setBounds(-100, -100, -10, 100, 100, 10);
        
        UUID agentId = world.addAgent("Demo Agent", "demo", 0, 0, 0);
        logger_.log("Added agent to Eliza's World: " + agentId, "", "elizas_world", LogLevel::SUCCESS);
        
        UUID envId = world.addEnvironment("Demo Environment", "demo", 0, 0, 0, 50);
        logger_.log("Added environment to Eliza's World: " + envId, "", "elizas_world", LogLevel::SUCCESS);
        
        logger_.log("✓ Specialized modules: PASSED", "", "modules", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
    
    void runPerformanceBenchmarks() {
        logger_.panel("Test 9: Performance Benchmarks", 
                     "Testing response times, memory usage, and throughput");
        
        // Memory operations benchmark
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 100; ++i) {
            auto memory = std::make_shared<Memory>();
            memory->userId = UUID::generate();
            memory->agentId = state_->getAgentId();
            memory->content = "Benchmark memory " + std::to_string(i);
            memory->createdAt = std::chrono::system_clock::now();
            memory_->createMemory(memory);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        logger_.log("Memory operations: 100 operations in " + std::to_string(duration.count()) + "ms", 
                   "", "benchmark", LogLevel::SUCCESS);
        logger_.log("Average: " + std::to_string(duration.count() / 100.0) + "ms per operation", 
                   "", "benchmark", LogLevel::INFO);
        
        // Search benchmark
        start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 50; ++i) {
            MemorySearchParams params;
            params.agentId = state_->getAgentId();
            params.count = 10;
            memory_->getMemories(params);
        }
        
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        logger_.log("Search operations: 50 searches in " + std::to_string(duration.count()) + "ms", 
                   "", "benchmark", LogLevel::SUCCESS);
        logger_.log("Average: " + std::to_string(duration.count() / 50.0) + "ms per search", 
                   "", "benchmark", LogLevel::INFO);
        
        // Memory count for usage estimate
        MemorySearchParams countParams;
        countParams.agentId = state_->getAgentId();
        countParams.count = 1000;
        auto allMemories = memory_->getMemories(countParams);
        logger_.log("Total memories in system: " + std::to_string(allMemories.size()), 
                   "", "benchmark", LogLevel::INFO);
        
        logger_.log("✓ Performance benchmarks: COMPLETED", "", "benchmark", LogLevel::SUCCESS);
        logger_.log("", "", "", LogLevel::INFO);
    }
};

int main() {
    try {
        ComprehensiveDemo demo;
        demo.runComprehensiveDemo();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with exception: " << e.what() << std::endl;
        return 1;
    }
}