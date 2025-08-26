#include <gtest/gtest.h>
#include "elizaos/agentcomms.hpp"
#include <thread>
#include <chrono>
#include <atomic>

using namespace elizaos;

class CrossAgentInterferenceTest : public ::testing::Test {
protected:
    void SetUp() override {
        agent1_comms = std::make_shared<AgentComms>("agent1");
        agent2_comms = std::make_shared<AgentComms>("agent2");
        agent3_comms = std::make_shared<AgentComms>("agent3");
    }
    
    void TearDown() override {
        agent1_comms->stop();
        agent2_comms->stop();
        agent3_comms->stop();
    }
    
    std::shared_ptr<AgentComms> agent1_comms;
    std::shared_ptr<AgentComms> agent2_comms;
    std::shared_ptr<AgentComms> agent3_comms;
};

TEST_F(CrossAgentInterferenceTest, MessageValidation_SelfMessagePrevention) {
    // Test that agents don't process their own messages
    agent1_comms->setAgentId("agent1");
    
    // Set up validation to use default validator
    agent1_comms->setGlobalMessageValidator(MessageValidation::defaultValidator);
    
    auto channel = agent1_comms->createChannel("test_channel", "test_server");
    agent1_comms->addChannelParticipant("test_channel", "agent1");
    
    std::atomic<int> messagesReceived(0);
    channel->setMessageHandler([&](const Message&) {
        messagesReceived++;
    });
    
    agent1_comms->start();
    
    // Send message from agent1 to itself (should be rejected)
    Message self_msg("", MessageType::TEXT, "agent1", "agent1", "test_channel", "self message");
    bool sent = agent1_comms->sendMessage("test_channel", self_msg);
    
    // Wait a bit for processing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Message should be rejected by validation
    EXPECT_FALSE(sent);
    EXPECT_EQ(messagesReceived, 0);
}

TEST_F(CrossAgentInterferenceTest, ChannelParticipationValidation) {
    // Test that only participants can send/receive messages in a channel
    auto channel1 = agent1_comms->createChannel("private_channel", "test_server");
    auto channel2 = agent2_comms->createChannel("private_channel", "test_server");
    
    // Add only agent1 as participant
    agent1_comms->addChannelParticipant("private_channel", "agent1");
    agent1_comms->subscribeToServer("test_server", "agent1");
    
    // Agent2 is not a participant
    agent2_comms->subscribeToServer("test_server", "agent2");
    
    std::atomic<int> agent1_messages(0);
    std::atomic<int> agent2_messages(0);
    
    channel1->setMessageHandler([&](const Message&) { agent1_messages++; });
    channel2->setMessageHandler([&](const Message&) { agent2_messages++; });
    
    // Set up validation with channel participation checking
    auto validator = [this](const Message& msg, const AgentId& agent_id) -> MessageValidationResult {
        if (agent_id.empty()) return MessageValidationResult(true); // Skip for empty agent
        
        // Check self-message
        auto self_check = MessageValidation::validateNotSelfMessage(msg, agent_id);
        if (!self_check.valid) return self_check;
        
        // Check channel participation
        bool is_participant = false;
        if (agent_id == "agent1") {
            is_participant = agent1_comms->isChannelParticipant(msg.channel_id, agent_id);
        } else if (agent_id == "agent2") {
            is_participant = agent2_comms->isChannelParticipant(msg.channel_id, agent_id);
        }
        
        if (!is_participant) {
            return MessageValidationResult(false, "Agent not participant in channel");
        }
        
        return MessageValidationResult(true);
    };
    
    agent1_comms->setGlobalMessageValidator(validator);
    agent2_comms->setGlobalMessageValidator(validator);
    
    agent1_comms->start();
    agent2_comms->start();
    
    // Send message from agent2 to private channel (should be rejected)
    Message msg("", MessageType::TEXT, "agent2", "agent1", "private_channel", "unauthorized message");
    bool sent = agent2_comms->sendMessage("private_channel", msg);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_FALSE(sent); // Should be rejected
    EXPECT_EQ(agent1_messages, 0);
    EXPECT_EQ(agent2_messages, 0);
}

TEST_F(CrossAgentInterferenceTest, AgentSpecificUUIDs) {
    // Test that agents have different UUIDs for same resources
    UUID agent1_room_uuid = agent1_comms->createAgentSpecificUUID("room123");
    UUID agent2_room_uuid = agent2_comms->createAgentSpecificUUID("room123");
    UUID agent3_room_uuid = agent3_comms->createAgentSpecificUUID("room123");
    
    // All UUIDs should be different (agents have different views)
    EXPECT_NE(agent1_room_uuid, agent2_room_uuid);
    EXPECT_NE(agent2_room_uuid, agent3_room_uuid);
    EXPECT_NE(agent1_room_uuid, agent3_room_uuid);
    
    // But should be consistent for same agent
    UUID agent1_room_uuid2 = agent1_comms->createAgentSpecificUUID("room123");
    EXPECT_EQ(agent1_room_uuid, agent1_room_uuid2);
}

TEST_F(CrossAgentInterferenceTest, MessageMetadataPreservation) {
    // Test that message metadata is preserved correctly
    auto channel = agent1_comms->createChannel("metadata_channel");
    
    std::atomic<bool> messageReceived(false);
    Message receivedMessage;
    
    channel->setMessageHandler([&](const Message& msg) {
        receivedMessage = msg;
        messageReceived = true;
    });
    
    agent1_comms->start();
    
    Message original_msg("test_msg_id", MessageType::COMMAND, "sender", "receiver", "metadata_channel", "test content");
    original_msg.setMetadata("source_id", "original_123");
    original_msg.setMetadata("agent_id", "sender");
    original_msg.setMetadata("timestamp", "1234567890");
    
    channel->sendMessage(original_msg, false); // Skip validation for this test
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_TRUE(messageReceived);
    EXPECT_EQ(receivedMessage.id, "test_msg_id");
    EXPECT_EQ(receivedMessage.getMetadata("source_id"), "original_123");
    EXPECT_EQ(receivedMessage.getMetadata("agent_id"), "sender");
    EXPECT_EQ(receivedMessage.getMetadata("timestamp"), "1234567890");
    EXPECT_TRUE(receivedMessage.hasMetadata("source_id"));
    EXPECT_FALSE(receivedMessage.hasMetadata("non_existent"));
}

TEST_F(CrossAgentInterferenceTest, ServerSubscriptionValidation) {
    // Test server subscription validation
    agent1_comms->subscribeToServer("server1", "agent1");
    agent2_comms->subscribeToServer("server2", "agent2");
    
    EXPECT_TRUE(agent1_comms->isSubscribedToServer("server1", "agent1"));
    EXPECT_FALSE(agent1_comms->isSubscribedToServer("server2", "agent1"));
    EXPECT_TRUE(agent2_comms->isSubscribedToServer("server2", "agent2"));
    EXPECT_FALSE(agent2_comms->isSubscribedToServer("server1", "agent2"));
    
    // Test unsubscription
    agent1_comms->unsubscribeFromServer("server1", "agent1");
    EXPECT_FALSE(agent1_comms->isSubscribedToServer("server1", "agent1"));
}

TEST_F(CrossAgentInterferenceTest, PreventInfiniteLoops) {
    // Test demonstration that self-message validation prevents loops
    auto channel = agent1_comms->createChannel("loop_channel");
    
    std::atomic<int> messagesReceived(0);
    
    // Set up agent to potentially respond to its own messages (bad behavior)
    channel->setMessageHandler([&](const Message& msg) {
        messagesReceived++;
        if (messagesReceived < 5) { // Prevent actual infinite loop in test
            Message response("", MessageType::RESPONSE, "agent1", "agent1", "loop_channel", "self response");
            channel->sendMessage(response, false); // Try to send to self without validation
        }
    });
    
    agent1_comms->start();
    
    // Send initial message to self without validation (bad scenario)
    Message initial_msg("", MessageType::TEXT, "agent1", "agent1", "loop_channel", "start loop");
    channel->sendMessage(initial_msg, false);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    int messages_without_validation = messagesReceived;
    
    // Now enable validation to prevent self-messages
    auto anti_loop_validator = [](const Message& msg, const AgentId& agent_id) -> MessageValidationResult {
        if (agent_id.empty()) return MessageValidationResult(true);
        return MessageValidation::validateNotSelfMessage(msg, agent_id);
    };
    
    agent1_comms->setGlobalMessageValidator(anti_loop_validator);
    
    // Reset counter and try again with validation
    messagesReceived = 0;
    
    channel->setMessageHandler([&](const Message& msg) {
        messagesReceived++;
        if (messagesReceived < 5) {
            Message response("", MessageType::RESPONSE, "agent1", "agent1", "loop_channel", "validated response");
            // This should be rejected by validation
            bool sent = agent1_comms->sendMessage("loop_channel", response, true);
            EXPECT_FALSE(sent); // Should be rejected
        }
    });
    
    // Try to start loop again with validation enabled
    Message validated_msg("", MessageType::TEXT, "agent1", "agent1", "loop_channel", "validated start");
    bool sent = agent1_comms->sendMessage("loop_channel", validated_msg, true);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Without validation, we had some messages
    EXPECT_GT(messages_without_validation, 0);
    // With validation, the initial self-message should be rejected
    EXPECT_FALSE(sent);
    EXPECT_EQ(messagesReceived, 0); // No messages should be processed with validation
}

TEST_F(CrossAgentInterferenceTest, MessageTargeting) {
    // Test that messages are properly targeted to specific agents
    auto channel = agent1_comms->createChannel("targeting_channel");
    
    agent1_comms->addChannelParticipant("targeting_channel", "agent1");
    agent1_comms->addChannelParticipant("targeting_channel", "agent2");
    agent1_comms->addChannelParticipant("targeting_channel", "agent3");
    
    std::atomic<int> messagesReceived(0);
    std::string lastReceiver;
    
    channel->setMessageHandler([&](const Message& msg) {
        messagesReceived++;
        lastReceiver = msg.receiver;
    });
    
    agent1_comms->start();
    
    // Send targeted message
    Message targeted_msg("", MessageType::TEXT, "agent1", "agent2", "targeting_channel", "message for agent2");
    channel->sendMessage(targeted_msg, false);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    EXPECT_EQ(messagesReceived, 1);
    EXPECT_EQ(lastReceiver, "agent2");
}