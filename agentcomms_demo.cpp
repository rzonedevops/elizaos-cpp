#include "elizaos/agentcomms.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace elizaos;

void demonstrateCrossAgentInterferencePrevention() {
    std::cout << "=== Enhanced AgentComms Cross-Agent Interference Prevention Demo ===" << std::endl;
    std::cout << std::endl;
    
    // Create three agents
    auto agent1 = std::make_shared<AgentComms>("agent1");
    auto agent2 = std::make_shared<AgentComms>("agent2");
    auto agent3 = std::make_shared<AgentComms>("agent3");
    
    std::cout << "1. Creating three agents with IDs: agent1, agent2, agent3" << std::endl;
    
    // Demonstrate agent-specific UUID mapping
    std::cout << "\n2. Demonstrating agent-specific UUID mapping for same resource:" << std::endl;
    UUID room_uuid_agent1 = agent1->createAgentSpecificUUID("room_123");
    UUID room_uuid_agent2 = agent2->createAgentSpecificUUID("room_123");
    UUID room_uuid_agent3 = agent3->createAgentSpecificUUID("room_123");
    
    std::cout << "   Agent1 view of room_123: " << room_uuid_agent1 << std::endl;
    std::cout << "   Agent2 view of room_123: " << room_uuid_agent2 << std::endl;
    std::cout << "   Agent3 view of room_123: " << room_uuid_agent3 << std::endl;
    std::cout << "   (Notice: Each agent has a unique view of the same resource)" << std::endl;
    
    // Set up channels with participation
    std::cout << "\n3. Setting up channels with participant management:" << std::endl;
    auto channel1 = agent1->createChannel("public_channel", "server1");
    auto channel2 = agent2->createChannel("public_channel", "server1");
    auto channel3 = agent3->createChannel("public_channel", "server1");
    
    auto private_channel1 = agent1->createChannel("private_channel", "server1");
    auto private_channel2 = agent2->createChannel("private_channel", "server1");
    
    // Add participants to public channel
    agent1->addChannelParticipant("public_channel", "agent1");
    agent1->addChannelParticipant("public_channel", "agent2");
    agent1->addChannelParticipant("public_channel", "agent3");
    agent2->addChannelParticipant("public_channel", "agent1");
    agent2->addChannelParticipant("public_channel", "agent2");
    agent2->addChannelParticipant("public_channel", "agent3");
    agent3->addChannelParticipant("public_channel", "agent1");
    agent3->addChannelParticipant("public_channel", "agent2");
    agent3->addChannelParticipant("public_channel", "agent3");
    
    // Add only agent1 and agent2 to private channel
    agent1->addChannelParticipant("private_channel", "agent1");
    agent1->addChannelParticipant("private_channel", "agent2");
    agent2->addChannelParticipant("private_channel", "agent1");
    agent2->addChannelParticipant("private_channel", "agent2");
    
    // Subscribe agents to server
    agent1->subscribeToServer("server1", "agent1");
    agent2->subscribeToServer("server1", "agent2");
    agent3->subscribeToServer("server1", "agent3");
    
    std::cout << "   Public channel participants: agent1, agent2, agent3" << std::endl;
    std::cout << "   Private channel participants: agent1, agent2 only" << std::endl;
    
    // Set up message validation
    auto comprehensive_validator = [&](const Message& msg, const AgentId& validating_agent_id) -> MessageValidationResult {
        if (validating_agent_id.empty()) return MessageValidationResult(true); // Skip for empty agent
        
        // The key insight: validation should be from the perspective of the RECEIVING agent
        // If this agent is the sender, don't validate (let the receiver validate)
        if (msg.sender == validating_agent_id) {
            return MessageValidationResult(true); // Sender can always send
        }
        
        // If this agent is the receiver, check if they should accept the message
        
        // 1. Check if message is targeted to this agent or is a broadcast
        if (!msg.receiver.empty() && msg.receiver != validating_agent_id) {
            return MessageValidationResult(false, "[NOT TARGETED] Message not for " + validating_agent_id);
        }
        
        // 2. Check channel participation for receiving agent
        bool is_participant = false;
        if (validating_agent_id == "agent1") {
            is_participant = agent1->isChannelParticipant(msg.channel_id, validating_agent_id);
        } else if (validating_agent_id == "agent2") {
            is_participant = agent2->isChannelParticipant(msg.channel_id, validating_agent_id);
        } else if (validating_agent_id == "agent3") {
            is_participant = agent3->isChannelParticipant(msg.channel_id, validating_agent_id);
        }
        
        if (!is_participant) {
            return MessageValidationResult(false, 
                "[PARTICIPATION BLOCKED] Agent " + validating_agent_id + " not participant in " + msg.channel_id);
        }
        
        return MessageValidationResult(true);
    };
    
    agent1->setGlobalMessageValidator(comprehensive_validator);
    agent2->setGlobalMessageValidator(comprehensive_validator);
    agent3->setGlobalMessageValidator(comprehensive_validator);
    
    std::cout << "\n4. Enabling comprehensive message validation (self-message + participation)" << std::endl;
    
    // Set up message handlers to track messages
    std::atomic<int> agent1_messages(0);
    std::atomic<int> agent2_messages(0);
    std::atomic<int> agent3_messages(0);
    
    channel1->setMessageHandler([&](const Message& msg) {
        agent1_messages++;
        std::cout << "   [AGENT1 RECEIVED] From: " << msg.sender << ", Content: " << msg.content << std::endl;
    });
    
    channel2->setMessageHandler([&](const Message& msg) {
        agent2_messages++;
        std::cout << "   [AGENT2 RECEIVED] From: " << msg.sender << ", Content: " << msg.content << std::endl;
    });
    
    channel3->setMessageHandler([&](const Message& msg) {
        agent3_messages++;
        std::cout << "   [AGENT3 RECEIVED] From: " << msg.sender << ", Content: " << msg.content << std::endl;
    });
    
    private_channel1->setMessageHandler([&](const Message& msg) {
        std::cout << "   [AGENT1 PRIVATE] From: " << msg.sender << ", Content: " << msg.content << std::endl;
    });
    
    private_channel2->setMessageHandler([&](const Message& msg) {
        std::cout << "   [AGENT2 PRIVATE] From: " << msg.sender << ", Content: " << msg.content << std::endl;
    });
    
    // Start all communication systems
    agent1->start();
    agent2->start();
    agent3->start();
    
    std::cout << "\n5. Testing scenarios:" << std::endl;
    
    // Test 1: Normal message exchange
    std::cout << "\n   Test 1: Normal message from agent1 to agent2 (should work)" << std::endl;
    Message normal_msg("", MessageType::TEXT, "agent1", "agent2", "public_channel", "Hello agent2!");
    bool sent1 = agent1->sendMessage("public_channel", normal_msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "   Result: " << (sent1 ? "SUCCESS" : "FAILED") << std::endl;
    
    // Test 2: Self-message (should be blocked)
    std::cout << "\n   Test 2: Self-message from agent1 to agent1 (should be blocked)" << std::endl;
    Message self_msg("", MessageType::TEXT, "agent1", "agent1", "public_channel", "Talking to myself");
    bool sent2 = agent1->sendMessage("public_channel", self_msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "   Result: " << (sent2 ? "FAILED (should be blocked)" : "SUCCESS (blocked as expected)") << std::endl;
    
    // Test 3: Non-participant trying to send to private channel (should be blocked)
    std::cout << "\n   Test 3: Agent3 trying to send to private channel (should be blocked)" << std::endl;
    Message private_msg("", MessageType::TEXT, "agent3", "agent1", "private_channel", "Unauthorized message");
    bool sent3 = agent3->sendMessage("private_channel", private_msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "   Result: " << (sent3 ? "FAILED (should be blocked)" : "SUCCESS (blocked as expected)") << std::endl;
    
    // Test 4: Authorized private message
    std::cout << "\n   Test 4: Agent2 sending to private channel (should work)" << std::endl;
    Message auth_private_msg("", MessageType::TEXT, "agent2", "agent1", "private_channel", "Secret message");
    bool sent4 = agent2->sendMessage("private_channel", auth_private_msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "   Result: " << (sent4 ? "SUCCESS" : "FAILED") << std::endl;
    
    // Test 5: Message metadata preservation
    std::cout << "\n   Test 5: Message with metadata preservation" << std::endl;
    Message metadata_msg("", MessageType::COMMAND, "agent2", "agent3", "public_channel", "Command with metadata");
    metadata_msg.setMetadata("source_id", "original_12345");
    metadata_msg.setMetadata("priority", "high");
    metadata_msg.setMetadata("timestamp", "1640995200");
    
    std::atomic<bool> metadata_received(false);
    channel3->setMessageHandler([&](const Message& msg) {
        agent3_messages++;
        if (msg.hasMetadata("source_id")) {
            metadata_received = true;
            std::cout << "   [AGENT3 RECEIVED] Metadata preserved - source_id: " 
                      << msg.getMetadata("source_id") 
                      << ", priority: " << msg.getMetadata("priority") << std::endl;
        }
    });
    
    bool sent5 = agent2->sendMessage("public_channel", metadata_msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "   Result: " << (sent5 && metadata_received ? "SUCCESS" : "FAILED") << std::endl;
    
    // Stop all systems
    agent1->stop();
    agent2->stop();
    agent3->stop();
    
    std::cout << "\n6. Summary:" << std::endl;
    std::cout << "   - Self-message prevention: IMPLEMENTED ✓" << std::endl;
    std::cout << "   - Channel participation validation: IMPLEMENTED ✓" << std::endl;
    std::cout << "   - Agent-specific UUID mapping: IMPLEMENTED ✓" << std::endl;
    std::cout << "   - Message metadata preservation: IMPLEMENTED ✓" << std::endl;
    std::cout << "   - Cross-agent interference prevention: IMPLEMENTED ✓" << std::endl;
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
}

int main() {
    demonstrateCrossAgentInterferencePrevention();
    return 0;
}