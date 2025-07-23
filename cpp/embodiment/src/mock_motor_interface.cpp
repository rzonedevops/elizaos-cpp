#include "elizaos/embodiment.hpp"
#include "elizaos/agentlogger.hpp"
#include <algorithm>

namespace elizaos {

/**
 * Mock Motor Interface Implementation
 */
MockMotorInterface::MockMotorInterface(MotorActionType type) : type_(type) {}

bool MockMotorInterface::initialize() {
    AgentLogger logger;
    logger.logInfo("Initializing Mock Motor Interface: " + getName());
    
    if (active_) {
        return true; // Already initialized
    }
    
    active_ = true;
    logger.logSuccess("Mock Motor Interface initialized: " + getName());
    return true;
}

void MockMotorInterface::shutdown() {
    if (!active_) {
        return;
    }
    
    AgentLogger logger;
    logger.logInfo("Shutting down Mock Motor Interface: " + getName());
    
    active_ = false;
    
    // Clear all active actions
    {
        std::lock_guard<std::mutex> lock(actionsMutex_);
        activeActions_.clear();
    }
    
    logger.logInfo("Mock Motor Interface shutdown complete");
}

bool MockMotorInterface::executeAction(std::shared_ptr<MotorAction> action) {
    if (!active_ || !action) {
        return false;
    }
    
    // Check if this interface can handle the action type
    if (action->type != type_) {
        return false;
    }
    
    AgentLogger logger;
    
    // Generate action ID
    std::string actionId = generateUUID();
    
    // Log the action execution
    std::string actionDescription;
    switch (action->type) {
        case MotorActionType::MOVEMENT: {
            auto moveAction = std::dynamic_pointer_cast<MovementAction>(action);
            if (moveAction) {
                actionDescription = "Movement to position (" + 
                    std::to_string(moveAction->targetPosition[0]) + ", " +
                    std::to_string(moveAction->targetPosition[1]) + ", " +
                    std::to_string(moveAction->targetPosition[2]) + ")";
            }
            break;
        }
        
        case MotorActionType::SPEECH: {
            auto speechAction = std::dynamic_pointer_cast<SpeechAction>(action);
            if (speechAction) {
                actionDescription = "Speech: \"" + speechAction->text + "\"";
            }
            break;
        }
        
        case MotorActionType::DISPLAY: {
            auto displayAction = std::dynamic_pointer_cast<DisplayAction>(action);
            if (displayAction) {
                actionDescription = "Display: " + displayAction->content;
            }
            break;
        }
        
        case MotorActionType::GESTURE: {
            auto gestureAction = std::dynamic_pointer_cast<GestureAction>(action);
            if (gestureAction) {
                actionDescription = "Gesture: " + gestureAction->gestureName;
            }
            break;
        }
        
        case MotorActionType::MANIPULATION: {
            auto manipAction = std::dynamic_pointer_cast<ManipulationAction>(action);
            if (manipAction) {
                actionDescription = "Manipulation: " + manipAction->actionType + 
                    " on object " + manipAction->objectId;
            }
            break;
        }
        
        case MotorActionType::COMMUNICATION: {
            auto commAction = std::dynamic_pointer_cast<CommunicationAction>(action);
            if (commAction) {
                actionDescription = "Communication to " + commAction->recipient + 
                    ": \"" + commAction->message + "\"";
            }
            break;
        }
        
        default:
            actionDescription = "Custom action";
            break;
    }
    
    logger.logInfo("Executing " + getName() + " action: " + actionDescription);
    
    // Store the action
    {
        std::lock_guard<std::mutex> lock(actionsMutex_);
        executedActions_.push_back(action);
        
        // Add to active actions list if it's a blocking action
        if (action->blocking) {
            activeActions_.push_back(actionId);
        }
    }
    
    // Simulate action execution time for blocking actions
    if (action->blocking) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate 100ms execution
        
        // Remove from active actions
        {
            std::lock_guard<std::mutex> lock(actionsMutex_);
            activeActions_.erase(
                std::remove(activeActions_.begin(), activeActions_.end(), actionId),
                activeActions_.end()
            );
        }
    }
    
    logger.logSuccess("Completed " + getName() + " action");
    return true;
}

bool MockMotorInterface::canExecute(std::shared_ptr<MotorAction> action) const {
    if (!active_ || !action) {
        return false;
    }
    
    // This interface can only handle actions of its designated type
    return action->type == type_;
}

void MockMotorInterface::stopAction(const std::string& actionId) {
    std::lock_guard<std::mutex> lock(actionsMutex_);
    
    // Remove the action from active list
    activeActions_.erase(
        std::remove(activeActions_.begin(), activeActions_.end(), actionId),
        activeActions_.end()
    );
    
    AgentLogger logger;
    logger.logInfo("Stopped action " + actionId + " on " + getName());
}

void MockMotorInterface::stopAllActions() {
    std::lock_guard<std::mutex> lock(actionsMutex_);
    
    size_t stoppedCount = activeActions_.size();
    activeActions_.clear();
    
    AgentLogger logger;
    logger.logInfo("Stopped " + std::to_string(stoppedCount) + " actions on " + getName());
}

bool MockMotorInterface::isActionComplete(const std::string& actionId) const {
    std::lock_guard<std::mutex> lock(actionsMutex_);
    
    // Action is complete if it's not in the active list
    return std::find(activeActions_.begin(), activeActions_.end(), actionId) == activeActions_.end();
}

std::vector<std::string> MockMotorInterface::getActiveActions() const {
    std::lock_guard<std::mutex> lock(actionsMutex_);
    return activeActions_;
}

double MockMotorInterface::getActionProgress(const std::string& actionId) const {
    std::lock_guard<std::mutex> lock(actionsMutex_);
    
    // If action is not active, it's complete
    if (std::find(activeActions_.begin(), activeActions_.end(), actionId) == activeActions_.end()) {
        return 1.0;
    }
    
    // For mock purposes, return a random progress
    return 0.5; // 50% progress for active actions
}

void MockMotorInterface::setConfiguration(const std::unordered_map<std::string, std::string>& config) {
    std::lock_guard<std::mutex> lock(configMutex_);
    config_ = config;
    
    AgentLogger logger;
    logger.logInfo("Updated configuration for " + getName());
}

std::unordered_map<std::string, std::string> MockMotorInterface::getConfiguration() const {
    std::lock_guard<std::mutex> lock(configMutex_);
    return config_;
}

std::vector<std::shared_ptr<MotorAction>> MockMotorInterface::getExecutedActions() const {
    std::lock_guard<std::mutex> lock(actionsMutex_);
    return executedActions_;
}

void MockMotorInterface::clearExecutedActions() {
    std::lock_guard<std::mutex> lock(actionsMutex_);
    executedActions_.clear();
    
    AgentLogger logger;
    logger.logInfo("Cleared executed actions history for " + getName());
}

} // namespace elizaos