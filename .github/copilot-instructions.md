# Copilot Instructions for ElizaOS C++

## Project Overview
ElizaOS C++ is a high-performance C++ implementation of the ElizaOS agent framework for building sophisticated autonomous agents with advanced cognitive capabilities, distributed cognition, and adaptive attention allocation.

## Development Philosophy
- **Rigorous Implementation**: Every feature should be implemented with complete functionality including all methods, functions, and supporting infrastructure
- **Production Quality**: Prioritize robust, production-ready implementations over mock, placeholder, or simplified solutions
- **Complete Feature Implementation**: When implementing features, indicate all remaining steps rather than reducing scope or functionality
- **C++ Best Practices**: Follow modern C++17 standards with proper RAII, smart pointers, and STL usage

## Code Standards

### Architecture Patterns
- **Event-Driven Design**: Implement threaded agent loops with proper synchronization
- **Memory Management**: Use smart pointers (shared_ptr, unique_ptr) and RAII principles
- **Error Handling**: Implement comprehensive error handling with exceptions where appropriate
- **Thread Safety**: Ensure thread-safe operations for multi-threaded agent systems
- **Modular Design**: Maintain clear separation between cognitive subsystems

### Implementation Guidelines

#### Core Components
When implementing or modifying core agent components:

1. **Agent Loop (`agentloop/`)**:
   - Implement full event-driven execution with pause/resume/step capabilities
   - Include proper thread synchronization and lifecycle management
   - Add comprehensive state management and error recovery

2. **Memory System (`agentmemory/`)**:
   - Implement complete persistent storage with embedding-based retrieval
   - Include attention allocation mechanisms (ECAN-inspired)
   - Support hypergraph knowledge representation
   - Provide full context management capabilities

3. **Task Orchestration (`agentagenda/`)**:
   - Build complete workflow sequencing with dependency resolution
   - Implement distributed agent coordination protocols
   - Include adaptive scheduling based on cognitive load

4. **AI Core**:
   - Implement full decision-making engines with state composition
   - Include pattern recognition and reasoning capabilities
   - Support symbolic-neural integration approaches

5. **Communication (`agentcomms/`)**:
   - Implement high-performance inter-agent messaging
   - Include complete external interface handlers
   - Support various communication protocols

#### Code Quality Requirements
- **Complete Function Implementation**: Every function should be fully implemented with proper error handling
- **Comprehensive Testing**: Include unit tests for all new functionality
- **Documentation**: Provide clear inline documentation for complex algorithms
- **Performance Optimization**: Consider performance implications for high-frequency operations
- **Memory Efficiency**: Optimize memory usage for large-scale agent deployments

#### Build System
- **CMake Integration**: Ensure all new components integrate properly with existing CMakeLists.txt
- **Dependency Management**: Properly handle external dependencies and version requirements
- **Cross-Platform Support**: Consider Windows (MSVC) and Unix-like systems
- **Build Configurations**: Support both Debug and Release builds with appropriate optimizations

### Cognitive Architecture Considerations
When working with cognitive components:

- **Attention Mechanisms**: Implement full attention allocation systems, not simplified versions
- **Meta-Cognition**: Build complete self-modification and introspective capabilities
- **Distributed Cognition**: Support full agent swarm coordination
- **Emergent Behavior**: Design systems that support complex emergent patterns

### File Organization
- **Headers**: Place all headers in `include/elizaos/` with proper namespace organization
- **Implementation**: Place source files in `src/` with matching directory structure
- **Tests**: Create comprehensive test suites for all components
- **Examples**: Provide working examples that demonstrate full feature capabilities

## Implementation Approach
When asked to implement features:

1. **Full Scope Analysis**: First analyze the complete scope of what needs to be implemented
2. **Architecture Design**: Design the full system architecture before coding
3. **Incremental Implementation**: Implement in logical increments, but always with full functionality
4. **Integration Testing**: Test integration with existing systems at each step
5. **Documentation**: Document the implementation approach and remaining work
6. **Performance Validation**: Validate performance characteristics of the implementation

## Avoid These Patterns
- **Mock Implementations**: Don't create placeholder or mock implementations
- **Simplified Solutions**: Don't reduce functionality to make implementation easier
- **Incomplete Features**: Don't leave features partially implemented without clear next steps
- **TODO Comments**: Instead of TODO comments, provide detailed implementation plans
- **Single-File Solutions**: Don't put everything in one file; maintain proper modular structure

## When You Can't Complete Everything
If a feature is too large to implement in one session:
1. Implement the core foundation with full functionality
2. Clearly document the remaining implementation steps
3. Provide a detailed roadmap for completion
4. Ensure what is implemented is production-quality and fully functional
5. Include comprehensive tests for the implemented portions

This approach ensures that every contribution moves the project forward with high-quality, production-ready code while maintaining the integrity of the cognitive architecture.