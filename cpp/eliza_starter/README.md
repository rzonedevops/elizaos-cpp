# ElizaOS C++ Starter

This is a functional C++ implementation of an Eliza-style conversational agent built with the ElizaOS framework. It serves as a starting point for developers who want to create C++ agents using the ElizaOS cognitive architecture.

## Features

- **Complete Agent Implementation**: A fully functional `ElizaStarterAgent` class demonstrating core ElizaOS concepts
- **Interactive Demo**: Command-line interface for chatting with the agent
- **Memory Management**: Integration with the ElizaOS memory system for conversation history
- **Pattern Matching**: Simple but effective conversation patterns for natural interactions
- **Personality Configuration**: Support for configurable agent personality traits
- **Agent Loop Integration**: Demonstrates the ElizaOS agent loop architecture
- **Comprehensive Logging**: Detailed logging of all agent operations

## Quick Start

### Building the Starter

```bash
# From the project root directory
mkdir build && cd build
cmake ..
make elizaos-eliza_starter eliza_starter_demo -j$(nproc)
```

### Running the Interactive Demo

```bash
# Run the demo executable
./cpp/eliza_starter/eliza_starter_demo
```

This will start an interactive chat session where you can:
- Ask questions and have conversations
- See the agent's memory system in action
- Experience the personality traits and response patterns
- Test the agent loop processing

### Basic Usage Example

```cpp
#include "elizaos/eliza_starter.hpp"
#include <iostream>

int main() {
    // Create and initialize the agent
    auto agent = elizaos::createElizaStarterAgent("MyBot", "bot-001");
    
    if (!agent->initialize()) {
        std::cerr << "Failed to initialize agent!" << std::endl;
        return 1;
    }
    
    agent->start();
    
    // Process some messages
    std::string response1 = agent->processMessage("Hello there!", "user1");
    std::cout << "Response: " << response1 << std::endl;
    
    std::string response2 = agent->processMessage("How are you?", "user1");
    std::cout << "Response: " << response2 << std::endl;
    
    agent->stop();
    return 0;
}
```

## Architecture Overview

The `ElizaStarterAgent` demonstrates several key ElizaOS concepts:

### Core Components

- **AgentLogger**: Comprehensive logging system with colored output
- **AgentMemoryManager**: Memory storage and retrieval system
- **State**: Agent state management including actors and goals
- **CharacterManager**: Character and personality configuration
- **AgentLoop**: Event loop system for continuous agent processing

### Processing Pipeline

The agent uses a three-step processing pipeline:

1. **Conversation Processing**: Handles incoming messages and context analysis
2. **Memory Updates**: Consolidates and organizes conversation history
3. **System Status**: Monitors agent health and performance

### Response Generation

The agent includes several response patterns:

- **Greeting Detection**: Recognizes various greeting patterns
- **Help Requests**: Provides information about agent capabilities
- **Question Handling**: Responds thoughtfully to user questions
- **Goodbye Recognition**: Handles conversation endings gracefully
- **Default Responses**: Falls back to engaging conversation prompts

## Customization

### Character Configuration

```cpp
agent->setCharacter(
    "Dr. Helper",
    "A knowledgeable assistant specializing in technical support",
    "Created to help users solve technical problems with patience and expertise"
);
```

### Personality Traits

```cpp
agent->addPersonalityTrait("technical_knowledge", "Expert in technology", 0.95);
agent->addPersonalityTrait("patience", "Very patient with users", 0.9);
agent->addPersonalityTrait("helpfulness", "Eager to help", 0.85);
```

### Custom Response Patterns

You can extend the agent by modifying the pattern matching methods:

- `containsGreeting()`: Customize greeting recognition
- `containsHelp()`: Modify help request detection
- `containsQuestion()`: Adjust question recognition
- `generateResponse()`: Add new response patterns

## Advanced Usage

### Memory System Integration

The starter demonstrates how to integrate with the ElizaOS memory system:

```cpp
// Add custom memories
agent->addMemory("Important information to remember", "system");

// Retrieve conversation history
auto memories = agent->getRecentMemories(5);
for (const auto& memory : memories) {
    std::cout << "Memory: " << memory->getContent() << std::endl;
}
```

### Agent Loop Customization

The agent loop can be customized with your own processing steps:

```cpp
std::vector<elizaos::LoopStep> customSteps = {
    elizaos::LoopStep([](std::shared_ptr<void> input) -> std::shared_ptr<void> {
        // Custom processing logic
        return input;
    })
};
```

## File Structure

```
cpp/eliza_starter/
├── CMakeLists.txt              # Build configuration
├── src/
│   └── eliza_starter.cpp       # Main implementation
└── README.md                   # This file

include/elizaos/
└── eliza_starter.hpp           # Public API header
```

## API Reference

### ElizaStarterAgent Class

#### Core Methods

- `bool initialize()`: Initialize the agent and all components
- `void start()`: Start the agent loop and begin processing
- `void stop()`: Stop the agent loop and cleanup resources
- `bool isRunning()`: Check if the agent is currently running

#### Interaction Methods

- `std::string processMessage(input, userId)`: Process a message and return response
- `void addMemory(content, userId)`: Add content to the agent's memory
- `std::vector<std::shared_ptr<Memory>> getRecentMemories(count)`: Get recent memories

#### Configuration Methods

- `void setCharacter(name, bio, lore)`: Configure the agent's character
- `void addPersonalityTrait(trait, description, strength)`: Add personality traits

#### Response Generation

- `std::string generateResponse(input)`: Generate response using pattern matching

### Factory Functions

- `std::unique_ptr<ElizaStarterAgent> createElizaStarterAgent(name, id)`: Create new agent
- `void runInteractiveDemo()`: Run the interactive command-line demo

## Dependencies

The starter requires these ElizaOS C++ modules:

- `elizaos-core`: Core data structures and types
- `elizaos-agentloop`: Agent loop system
- `elizaos-agentmemory`: Memory management
- `elizaos-agentlogger`: Logging system
- `elizaos-characters`: Character management

## Building from Source

```bash
# Clone the ElizaOS C++ repository
git clone https://github.com/ZoneCog/elizaos-cpp.git
cd elizaos-cpp

# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y cmake build-essential libcurl4-openssl-dev

# Build the project
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make elizaos-eliza_starter eliza_starter_demo -j$(nproc)

# Run the demo
./cpp/eliza_starter/eliza_starter_demo
```

## Next Steps

This starter provides a foundation for building more sophisticated agents. Consider these extensions:

1. **Enhanced NLP**: Integrate with more advanced language processing libraries
2. **External APIs**: Add support for web APIs and external services
3. **Persistent Storage**: Add database integration for long-term memory
4. **Multi-Modal**: Support for images, audio, and other media types
5. **Agent Communication**: Enable communication between multiple agents
6. **Learning**: Add machine learning capabilities for behavior adaptation

## Examples

### Simple Chatbot

```cpp
#include "elizaos/eliza_starter.hpp"

int main() {
    auto chatbot = elizaos::createElizaStarterAgent("Chatbot", "chat-001");
    chatbot->setCharacter("ChatBot", "A friendly chatbot", "Here to help and chat");
    
    chatbot->initialize();
    chatbot->start();
    
    // Simple conversation
    std::cout << chatbot->processMessage("Hi there!", "user") << std::endl;
    std::cout << chatbot->processMessage("What's the weather like?", "user") << std::endl;
    
    chatbot->stop();
    return 0;
}
```

### Customer Service Agent

```cpp
#include "elizaos/eliza_starter.hpp"

int main() {
    auto agent = elizaos::createElizaStarterAgent("SupportBot", "support-001");
    
    agent->setCharacter(
        "Support Assistant",
        "Professional customer service representative",
        "Trained to help customers with technical and billing issues"
    );
    
    agent->addPersonalityTrait("professionalism", "Maintains professional tone", 0.9);
    agent->addPersonalityTrait("technical_expertise", "Deep technical knowledge", 0.85);
    agent->addPersonalityTrait("empathy", "Understanding of customer frustration", 0.8);
    
    agent->initialize();
    agent->start();
    
    // Handle customer interactions
    std::cout << agent->processMessage("I'm having trouble with my account", "customer123") << std::endl;
    
    agent->stop();
    return 0;
}
```

## Contributing

This starter is part of the ElizaOS C++ project. Contributions are welcome! Please see the main project repository for contribution guidelines.

## License

This project is licensed under the MIT License - see the main project LICENSE file for details.