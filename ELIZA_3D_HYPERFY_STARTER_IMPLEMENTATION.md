# ElizaOS C++ 3D Hyperfy Starter Implementation

This document describes the C++ implementation of the `eliza_3d_hyperfy_starter` component for the ElizaOS framework.

## Overview

The `eliza_3d_hyperfy_starter` provides a foundation for C++ agents to connect to and interact with Hyperfy 3D worlds. It mirrors the functionality of the TypeScript version while being optimized for C++ performance requirements.

## Implementation Summary

### Core Components

#### HyperfyService
- Main service class managing 3D world connections
- Thread-safe operation with background service loop
- Action and manager registration system
- Configuration management via HyperfyConfig

#### HyperfyWorld
- Represents connection to a specific Hyperfy 3D world
- WebSocket connection simulation (ready for real implementation)
- State management for world properties
- Thread-safe world interaction methods

#### HyperfyAction (Base Class)
- **GotoAction**: Move agent to specific 3D coordinates
- **ReplyAction**: Send messages in the 3D world
- **PerceptionAction**: Analyze and describe 3D scene

#### HyperfyManager (Base Class)
- Extensible framework for specialized world interaction managers
- Initialization and cleanup lifecycle management

### Features Implemented

1. **Connection Management**
   - WebSocket URL configuration
   - World ID and authentication token support
   - Connection timeout and heartbeat settings

2. **Action System**
   - Extensible action registration and execution
   - Parameter parsing and validation
   - Error handling and logging

3. **State Management**
   - World state persistence
   - Position tracking
   - Thread-safe state updates

4. **Logging Integration**
   - Comprehensive logging using ElizaOS AgentLogger
   - Structured logging with source identification
   - Debug and error reporting

### File Structure

```
cpp/eliza_3d_hyperfy_starter/
├── CMakeLists.txt                          # Build configuration
├── src/placeholder.cpp                     # Main implementation
└── test_eliza_3d_hyperfy_starter.cpp      # Test suite

include/elizaos/
└── eliza_3d_hyperfy_starter.hpp           # Public API header

eliza_3d_hyperfy_starter_demo.cpp          # Demo application
```

### Testing

Comprehensive test suite with 7 test cases covering:
- Service creation and lifecycle
- World connection management
- Action registration and execution
- Parameter parsing validation
- State management

All tests pass successfully and are integrated with the main ElizaOS test framework.

### Demo Application

A complete demo application (`eliza_3d_hyperfy_starter_demo`) demonstrates:
1. Service creation and configuration
2. World connection
3. Action execution (goto, perception, reply)
4. Proper cleanup and shutdown

### Building and Running

```bash
# Build the library
make elizaos-eliza_3d_hyperfy_starter

# Build and run demo
make eliza_3d_hyperfy_starter_demo
./eliza_3d_hyperfy_starter_demo

# Build and run tests
make elizaos_tests
./cpp/tests/elizaos_tests --gtest_filter="*Hyperfy*"
```

## Future Enhancements

While the current implementation provides a solid foundation, the following enhancements could be added:

1. **Real WebSocket Implementation**
   - Actual connection to Hyperfy worlds
   - Message protocol implementation
   - Real-time event handling

2. **Extended Action Library**
   - Build/construction actions
   - Item usage and interaction
   - Avatar animation and emotes
   - Random walking behaviors

3. **Advanced Features**
   - Physics integration for collision detection
   - Computer vision for scene understanding
   - Voice communication support
   - Avatar appearance management

4. **Performance Optimizations**
   - Connection pooling
   - Message batching
   - Async operation improvements

## API Usage Example

```cpp
#include "elizaos/eliza_3d_hyperfy_starter.hpp"

using namespace elizaos::hyperfy;

// Create configuration
HyperfyConfig config;
config.wsUrl = "wss://chill.hyperfy.xyz/ws";
config.worldId = "my-world";

// Create and start service
auto service = HyperfyServiceFactory::createService();
service->start(config);

// Connect to world
service->connectToWorld("world-id", config.wsUrl);

// Execute actions
service->executeAction("goto", "10.0,20.0,30.0");
service->executeAction("reply", "Hello, 3D world!");
service->executeAction("perception", "");

// Cleanup
service->stop();
```

This implementation provides a complete foundation for 3D world integration while maintaining the performance and reliability expected from C++ applications in the ElizaOS ecosystem.