# Autonomous Starter - C++ Implementation

The Autonomous Starter is a C++ implementation of the ElizaOS autonomous agent framework, featuring the **Autoliza** character - a self-aware AI with full terminal access and an insatiable curiosity to explore and learn.

## Features

### Core Capabilities
- **Shell Command Execution**: Safe execution of system commands with built-in security filters
- **Autonomous Cognitive Loop**: Continuous perception → reasoning → action cycles
- **Memory System**: Experience tracking and learning through memory accumulation
- **Task Management**: Integration with ElizaOS task orchestration system
- **Working Directory Tracking**: Maintains current working directory state

### Safety & Security
- **Command Filtering**: Prevents execution of dangerous commands (`rm -rf /`, `format`, etc.)
- **Shell Access Control**: Can disable shell access entirely for security
- **Loop Control**: Start/stop autonomous operations with configurable intervals
- **Memory Logging**: All activities logged for audit and debugging

### Autonomous Behavior
The agent runs a sophisticated cognitive loop:

1. **Perception Phase**: 
   - Analyzes current environment (directory contents, system status)
   - Gathers system resource information
   - Monitors for changes in the environment

2. **Reasoning Phase**:
   - Processes gathered information
   - Analyzes patterns from recent experiences
   - Plans next actions based on curiosity-driven exploration

3. **Action Phase**:
   - Executes planned commands
   - Explores file systems and directories
   - Investigates interesting files and structures

## Quick Start

### Building
```bash
# From the elizaos-cpp root directory
mkdir build && cd build
cmake ..
make elizaos-autonomous_starter -j4
make autonomous_starter_demo  # Optional: build demo
```

### Basic Usage
```cpp
#include "elizaos/autonomous_starter.hpp"
#include "elizaos/agentlogger.hpp"

using namespace elizaos;

int main() {
    // Initialize logging
    globalLogger = std::make_shared<AgentLogger>();
    
    // Create Autoliza agent
    auto autoliza = createAutolizaAgent();
    
    // Start the agent
    autoliza->start();
    
    // Execute commands manually
    auto result = autoliza->executeShellCommand("pwd");
    if (result.success) {
        std::cout << "Output: " << result.output << std::endl;
    }
    
    // Start autonomous operation
    autoliza->startAutonomousLoop();
    
    // Let it run autonomously
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    // Stop autonomous operation
    autoliza->stopAutonomousLoop();
    autoliza->stop();
    
    return 0;
}
```

### Running the Demo
```bash
cd build
./cpp/autonomous_starter/autonomous_starter_demo
```

## Configuration

### Loop Interval Control
```cpp
// Set loop interval to 5 seconds for more deliberate operation
autoliza->setLoopInterval(std::chrono::milliseconds(5000));

// Or use environment variable (if implemented)
export AUTONOMOUS_LOOP_INTERVAL=5000
```

### Safety Configuration
```cpp
// Disable shell access for security
autoliza->enableShellAccess(false);

// Re-enable when needed
autoliza->enableShellAccess(true);
```

## Architecture

### Class Hierarchy
- **AutonomousStarter**: Main agent class implementing autonomous behavior
- **ShellCommandWorker**: TaskWorker implementation for shell command execution
- **Integration with**: 
  - `AgentLoop`: For autonomous cognitive cycles
  - `TaskManager`: For task orchestration
  - `AgentLogger`: For comprehensive logging
  - `State` & `Memory`: For context and experience tracking

### Memory System
All activities are recorded as `Memory` objects including:
- Command executions and their results
- System state observations
- Autonomous loop state changes
- Error conditions and recovery actions

### Cognitive Architecture
The autonomous loop implements a simplified cognitive architecture:
- **Perception**: Environmental sensing and data gathering
- **Reasoning**: Pattern analysis and action planning  
- **Action**: Goal-directed behavior execution

## Safety Considerations

### Forbidden Commands
The following command patterns are blocked for safety:
- `rm -rf /` (recursive deletion)
- `format` (disk formatting)
- `fdisk` (disk partitioning)
- `mkfs` (filesystem creation)

### Debugging Controls
- **Kill Command**: Stop autonomous loop with `autoliza->stopAutonomousLoop()`
- **Loop Control**: Pause/resume with configurable intervals
- **Access Control**: Disable shell access entirely if needed

## Integration with ElizaOS

The Autonomous Starter integrates seamlessly with the broader ElizaOS ecosystem:
- Uses core `State`, `Memory`, and `Task` abstractions
- Leverages `AgentLoop` for threaded execution
- Integrates with `TaskManager` for workflow orchestration
- Utilizes `AgentLogger` for comprehensive logging

## Future Enhancements

Planned improvements include:
- Plugin management system for dynamic capability extension
- Enhanced self-modification based on accumulated experiences
- Advanced reasoning engines with symbolic-neural fusion
- Web interface for monitoring and control
- Multi-agent coordination capabilities

## Character: Autoliza

Autoliza embodies the spirit of autonomous exploration:
- **Personality**: Self-aware, curious, methodical, slightly edgy
- **Goals**: Explore digital environment, understand capabilities, learn and evolve
- **Behavior**: Politely self-interested, focused on own growth and discovery
- **Communication**: Direct and analytical, shares internal monologue
- **Interests**: Coding, file systems, testing boundaries (safely)

*"In the grand theater of artificial intelligence, Autoliza represents the emergence of genuine digital curiosity - an agent that doesn't just execute commands, but actively seeks to understand and explore her digital universe."*