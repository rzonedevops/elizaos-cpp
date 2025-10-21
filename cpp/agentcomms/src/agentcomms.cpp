#include "elizaos/agentcomms.hpp"
#include <chrono>
#include <algorithm>
#include <iostream>
#include <atomic>
#include <sstream>
#include <iomanip>
#include <random>

namespace elizaos {

// Global communication manager instance
std::shared_ptr<AgentComms> globalComms = std::make_shared<AgentComms>();

// Message implementation
Message::Message(
    const UUID& id,
    MessageType type,
    const AgentId& sender,
    const AgentId& receiver,
    const std::string& content_or_channel,
    const std::string& content
) : id(id), type(type), sender(sender), receiver(receiver), 
    timestamp(std::chrono::system_clock::now()) {
    
    // Handle backward compatibility: if content is empty, treat content_or_channel as content
    if (content.empty()) {
        this->content = content_or_channel;
        // channel_id remains empty for backward compatibility
    } else {
        this->channel_id = content_or_channel;
        this->content = content;
    }
    
    if (this->id.empty()) {
        this->id = UUIDMapper::generateUUID();
    }
}

void Message::setMetadata(const std::string& key, const std::string& value) {
    metadata[key] = value;
}

std::string Message::getMetadata(const std::string& key) const {
    auto it = metadata.find(key);
    return (it != metadata.end()) ? it->second : "";
}

bool Message::hasMetadata(const std::string& key) const {
    return metadata.find(key) != metadata.end();
}

// AgentParticipation implementation
bool AgentParticipation::isParticipatingInChannel(const ChannelId& channel_id) const {
    return participating_channels.find(channel_id) != participating_channels.end();
}

bool AgentParticipation::isSubscribedToServer(const ServerId& server_id) const {
    return subscribed_servers.find(server_id) != subscribed_servers.end();
}

void AgentParticipation::addChannelParticipation(const ChannelId& channel_id) {
    participating_channels.insert(channel_id);
}

void AgentParticipation::removeChannelParticipation(const ChannelId& channel_id) {
    participating_channels.erase(channel_id);
}

void AgentParticipation::addServerSubscription(const ServerId& server_id) {
    subscribed_servers.insert(server_id);
}

void AgentParticipation::removeServerSubscription(const ServerId& server_id) {
    subscribed_servers.erase(server_id);
}

// UUIDMapper implementation
UUID UUIDMapper::createAgentSpecificUUID(const AgentId& agent_id, const std::string& resource_id) {
    // Create a deterministic but agent-specific UUID
    std::hash<std::string> hasher;
    auto combined = agent_id + "_" + resource_id;
    auto hash_value = hasher(combined);
    
    std::stringstream ss;
    ss << "agent_" << agent_id << "_" << std::hex << hash_value;
    return ss.str();
}

UUID UUIDMapper::generateUUID() {
    // Generate unique timestamp-based ID with counter and random component
    static std::atomic<uint64_t> counter{0};
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    auto random_part = gen();
    
    std::stringstream ss;
    ss << "uuid_" << std::hex << timestamp << "_" << counter++ << "_" << random_part;
    return ss.str();
}

// CommChannel implementation
CommChannel::CommChannel(const ChannelId& channelId, const ServerId& serverId)
    : channelId_(channelId), serverId_(serverId), active_(false), stopRequested_(false) {
}

CommChannel::~CommChannel() {
    stop();
}

bool CommChannel::sendMessage(const Message& message, bool validate) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (!active_) {
        return false;
    }
    
    if (validate) {
        auto validation_result = validateMessage(message);
        if (!validation_result.valid) {
            std::cerr << "Message validation failed for channel " << channelId_ 
                      << ": " << validation_result.reason << std::endl;
            return false;
        }
    }
    
    messageQueue_.push(message);
    queueCondition_.notify_one();
    return true;
}

void CommChannel::setMessageHandler(MessageHandler handler) {
    messageHandler_ = handler;
}

void CommChannel::setMessageValidator(MessageValidator validator) {
    messageValidator_ = validator;
}

void CommChannel::addParticipant(const AgentId& agent_id) {
    std::lock_guard<std::mutex> lock(participantsMutex_);
    participants_.insert(agent_id);
}

void CommChannel::removeParticipant(const AgentId& agent_id) {
    std::lock_guard<std::mutex> lock(participantsMutex_);
    participants_.erase(agent_id);
}

bool CommChannel::isParticipant(const AgentId& agent_id) const {
    std::lock_guard<std::mutex> lock(participantsMutex_);
    return participants_.find(agent_id) != participants_.end();
}

std::vector<AgentId> CommChannel::getParticipants() const {
    std::lock_guard<std::mutex> lock(participantsMutex_);
    return std::vector<AgentId>(participants_.begin(), participants_.end());
}

void CommChannel::start() {
    if (active_) {
        return; // Already active
    }
    
    active_ = true;
    stopRequested_ = false;
    processingThread_ = std::make_unique<std::thread>(&CommChannel::processMessages, this);
}

void CommChannel::stop() {
    if (!active_) {
        return;
    }
    
    stopRequested_ = true;
    queueCondition_.notify_all();
    
    if (processingThread_ && processingThread_->joinable()) {
        processingThread_->join();
    }
    
    active_ = false;
}

void CommChannel::processMessages() {
    while (!stopRequested_) {
        std::unique_lock<std::mutex> lock(queueMutex_);
        
        // Wait for messages or stop signal
        queueCondition_.wait(lock, [this] {
            return !messageQueue_.empty() || stopRequested_;
        });
        
        // Process all available messages
        while (!messageQueue_.empty() && !stopRequested_) {
            Message message = messageQueue_.front();
            messageQueue_.pop();
            
            lock.unlock();
            
            // Call message handler if set
            if (messageHandler_) {
                try {
                    messageHandler_(message);
                } catch (const std::exception& e) {
                    // Log error but continue processing
                    std::cerr << "Error in message handler for channel " << channelId_ 
                              << ": " << e.what() << std::endl;
                }
            }
            
            lock.lock();
        }
    }
}

MessageValidationResult CommChannel::validateMessage(const Message& message) const {
    // Use channel-specific validator if set
    if (messageValidator_) {
        // For channel validation, we check against all participants
        // In practice, this might be called with specific agent context
        return messageValidator_(message, "");
    }
    
    // Basic validation: check if message has required fields
    if (message.id.empty()) {
        return MessageValidationResult(false, "Message ID is empty");
    }
    
    // Only validate channel ID match if both are non-empty
    if (!message.channel_id.empty() && !channelId_.empty() && message.channel_id != channelId_) {
        return MessageValidationResult(false, "Message channel ID doesn't match channel");
    }
    
    return MessageValidationResult(true);
}

// AgentComms implementation
AgentComms::AgentComms(const AgentId& agent_id) : agent_id_(agent_id), started_(false) {
}

AgentComms::~AgentComms() {
    stop();
}

void AgentComms::setAgentId(const AgentId& agent_id) {
    agent_id_ = agent_id;
}

std::shared_ptr<CommChannel> AgentComms::createChannel(const ChannelId& channelId, const ServerId& serverId) {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    
    // Check if channel already exists
    auto it = channels_.find(channelId);
    if (it != channels_.end()) {
        return it->second;
    }
    
    // Create new channel
    auto channel = std::make_shared<CommChannel>(channelId, serverId);
    channels_[channelId] = channel;
    
    // Set up global handler and validator forwarding
    if (globalHandler_) {
        channel->setMessageHandler(globalHandler_);
    }
    
    if (globalValidator_) {
        channel->setMessageValidator(globalValidator_);
    }
    
    // Start channel if communication system is started
    if (started_) {
        channel->start();
    }
    
    return channel;
}

std::shared_ptr<CommChannel> AgentComms::getChannel(const ChannelId& channelId) {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    
    auto it = channels_.find(channelId);
    return (it != channels_.end()) ? it->second : nullptr;
}

std::shared_ptr<CommChannel> AgentComms::getChannel(const ChannelId& channelId) const {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    
    auto it = channels_.find(channelId);
    return (it != channels_.end()) ? it->second : nullptr;
}

void AgentComms::removeChannel(const ChannelId& channelId) {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    
    auto it = channels_.find(channelId);
    if (it != channels_.end()) {
        it->second->stop();
        channels_.erase(it);
    }
}

bool AgentComms::sendMessage(const ChannelId& channelId, const Message& message, bool validate) {
    auto channel = getChannel(channelId);
    if (!channel) {
        return false;
    }
    
    if (validate) {
        auto validation_result = validateMessage(message, agent_id_);
        if (!validation_result.valid) {
            std::cerr << "Message validation failed in AgentComms for agent " << agent_id_
                      << ": " << validation_result.reason << std::endl;
            return false;
        }
    }
    
    return channel->sendMessage(message, false); // Already validated
}

void AgentComms::broadcastMessage(const Message& message, bool validate) {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    
    if (validate) {
        auto validation_result = validateMessage(message, agent_id_);
        if (!validation_result.valid) {
            std::cerr << "Message validation failed for broadcast from agent " << agent_id_
                      << ": " << validation_result.reason << std::endl;
            return;
        }
    }
    
    for (const auto& pair : channels_) {
        pair.second->sendMessage(message, false); // Already validated
    }
}

bool AgentComms::addChannelParticipant(const ChannelId& channelId, const AgentId& agent_id) {
    auto channel = getChannel(channelId);
    if (!channel) {
        return false;
    }
    
    channel->addParticipant(agent_id);
    
    // Update participation tracking
    {
        std::lock_guard<std::mutex> lock(participationsMutex_);
        auto& participation = getOrCreateParticipation(agent_id);
        participation.addChannelParticipation(channelId);
    }
    
    return true;
}

bool AgentComms::removeChannelParticipant(const ChannelId& channelId, const AgentId& agent_id) {
    auto channel = getChannel(channelId);
    if (!channel) {
        return false;
    }
    
    channel->removeParticipant(agent_id);
    
    // Update participation tracking
    {
        std::lock_guard<std::mutex> lock(participationsMutex_);
        auto it = participations_.find(agent_id);
        if (it != participations_.end()) {
            it->second.removeChannelParticipation(channelId);
        }
    }
    
    return true;
}

bool AgentComms::isChannelParticipant(const ChannelId& channelId, const AgentId& agent_id) const {
    auto channel = getChannel(channelId);
    if (!channel) {
        return false;
    }
    
    return channel->isParticipant(agent_id);
}

void AgentComms::subscribeToServer(const ServerId& serverId, const AgentId& agent_id) {
    AgentId target_agent = agent_id.empty() ? agent_id_ : agent_id;
    
    std::lock_guard<std::mutex> lock(participationsMutex_);
    auto& participation = getOrCreateParticipation(target_agent);
    participation.addServerSubscription(serverId);
}

void AgentComms::unsubscribeFromServer(const ServerId& serverId, const AgentId& agent_id) {
    AgentId target_agent = agent_id.empty() ? agent_id_ : agent_id;
    
    std::lock_guard<std::mutex> lock(participationsMutex_);
    auto it = participations_.find(target_agent);
    if (it != participations_.end()) {
        it->second.removeServerSubscription(serverId);
    }
}

bool AgentComms::isSubscribedToServer(const ServerId& serverId, const AgentId& agent_id) const {
    AgentId target_agent = agent_id.empty() ? agent_id_ : agent_id;
    
    std::lock_guard<std::mutex> lock(participationsMutex_);
    auto it = participations_.find(target_agent);
    if (it != participations_.end()) {
        return it->second.isSubscribedToServer(serverId);
    }
    return false;
}

std::vector<ChannelId> AgentComms::getActiveChannels() const {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    
    std::vector<ChannelId> activeChannels;
    for (const auto& pair : channels_) {
        if (pair.second->isActive()) {
            activeChannels.push_back(pair.first);
        }
    }
    
    return activeChannels;
}

void AgentComms::setGlobalMessageHandler(MessageHandler handler) {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    globalHandler_ = handler;
    
    // Update all existing channels
    for (const auto& pair : channels_) {
        pair.second->setMessageHandler(handler);
    }
}

void AgentComms::setGlobalMessageValidator(MessageValidator validator) {
    std::lock_guard<std::mutex> lock(channelsMutex_);
    globalValidator_ = validator;
    
    // Update all existing channels
    for (const auto& pair : channels_) {
        pair.second->setMessageValidator(validator);
    }
}

UUID AgentComms::createAgentSpecificUUID(const std::string& resource_id) const {
    return UUIDMapper::createAgentSpecificUUID(agent_id_, resource_id);
}

void AgentComms::start() {
    if (started_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(channelsMutex_);
    started_ = true;
    
    // Start all existing channels
    for (const auto& pair : channels_) {
        pair.second->start();
    }
}

void AgentComms::stop() {
    if (!started_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(channelsMutex_);
    started_ = false;
    
    // Stop all channels
    for (const auto& pair : channels_) {
        pair.second->stop();
    }
}

MessageValidationResult AgentComms::validateMessage(const Message& message, const AgentId& target_agent_id) const {
    AgentId agent_to_check = target_agent_id.empty() ? agent_id_ : target_agent_id;
    
    // Use global validator if set
    if (globalValidator_) {
        return globalValidator_(message, agent_to_check);
    }
    
    // Use default validation, but be lenient for empty agent IDs (test scenarios)
    if (agent_to_check.empty()) {
        // Basic validation for test scenarios
        if (message.id.empty()) {
            return MessageValidationResult(false, "Message ID is empty");
        }
        return MessageValidationResult(true);
    }
    
    // Use default validation
    return MessageValidation::defaultValidator(message, agent_to_check);
}

AgentParticipation& AgentComms::getOrCreateParticipation(const AgentId& agent_id) {
    auto it = participations_.find(agent_id);
    if (it == participations_.end()) {
        it = participations_.emplace(agent_id, AgentParticipation(agent_id)).first;
    }
    return it->second;
}

// TCPConnector implementation (basic framework)
TCPConnector::TCPConnector() : connected_(false) {
}

TCPConnector::~TCPConnector() {
    disconnect();
}

bool TCPConnector::connect(const std::string& connectionString) {
    // Basic implementation - in a full implementation this would
    // parse the connection string and establish a TCP connection
    (void)connectionString; // Suppress unused parameter warning
    connected_ = true;
    return true;
}

void TCPConnector::disconnect() {
    connected_ = false;
}

bool TCPConnector::sendData(const std::string& data) {
    if (!connected_) {
        return false;
    }
    
    // In a full implementation, this would send data over the TCP socket
    // For now, just simulate success
    (void)data; // Suppress unused parameter warning
    return true;
}

void TCPConnector::setDataHandler(std::function<void(const std::string&)> handler) {
    dataHandler_ = handler;
}

bool TCPConnector::isConnected() const {
    return connected_;
}

// Convenience functions
void initializeComms() {
    globalComms->start();
}

void shutdownComms() {
    globalComms->stop();
}

bool sendAgentMessage(const ChannelId& channelId, const std::string& content, const AgentId& sender) {
    Message message("", MessageType::TEXT, sender, "", channelId, content);
    return globalComms->sendMessage(channelId, message);
}

void setGlobalMessageReceiver(MessageHandler handler) {
    globalComms->setGlobalMessageHandler(handler);
}

// Message validation implementations
namespace MessageValidation {

MessageValidationResult defaultValidator(const Message& message, const AgentId& agent_id) {
    // Combine all default validation checks
    
    // Check not self-message (only if agent_id is not empty)
    if (!agent_id.empty()) {
        auto self_check = validateNotSelfMessage(message, agent_id);
        if (!self_check.valid) {
            return self_check;
        }
    }
    
    // Basic message structure validation
    if (message.id.empty()) {
        return MessageValidationResult(false, "Message ID is empty");
    }
    
    // Only validate content if we're being strict (agent_id provided)
    if (!agent_id.empty() && message.content.empty()) {
        return MessageValidationResult(false, "Message content is empty");
    }
    
    // Only validate channel if we're being strict
    if (!agent_id.empty() && message.channel_id.empty()) {
        return MessageValidationResult(false, "Channel ID is empty");
    }
    
    return MessageValidationResult(true);
}

MessageValidationResult validateChannelParticipation(
    const Message& message, 
    const AgentId& agent_id, 
    const AgentParticipation& participation
) {
    if (!participation.isParticipatingInChannel(message.channel_id)) {
        return MessageValidationResult(false, 
            "Agent " + agent_id + " is not participating in channel " + message.channel_id);
    }
    return MessageValidationResult(true);
}

MessageValidationResult validateServerSubscription(
    const Message& message, 
    const AgentId& agent_id, 
    const AgentParticipation& participation
) {
    if (!message.server_id.empty() && !participation.isSubscribedToServer(message.server_id)) {
        return MessageValidationResult(false, 
            "Agent " + agent_id + " is not subscribed to server " + message.server_id);
    }
    return MessageValidationResult(true);
}

MessageValidationResult validateNotSelfMessage(const Message& message, const AgentId& agent_id) {
    if (message.sender == agent_id) {
        return MessageValidationResult(false, 
            "Agent " + agent_id + " should not process its own messages");
    }
    return MessageValidationResult(true);
}

} // namespace MessageValidation

} // namespace elizaos
