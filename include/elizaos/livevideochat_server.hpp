#pragma once

#include "elizaos/livevideochat.hpp"
#include <string>
#include <functional>
#include <map>
#include <memory>

namespace elizaos {

/**
 * HTTP request structure
 */
struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> query_params;
};

/**
 * HTTP response structure
 */
struct HttpResponse {
    int status_code = 200;
    std::string body;
    std::map<std::string, std::string> headers;
    
    HttpResponse() {
        headers["Content-Type"] = "application/json";
        headers["Access-Control-Allow-Origin"] = "*";
        headers["Access-Control-Allow-Methods"] = "GET, POST, PUT, DELETE, OPTIONS";
        headers["Access-Control-Allow-Headers"] = "Content-Type, Authorization";
    }
};

/**
 * HTTP request handler function type
 */
using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * WebSocket message handler function type
 */
using WebSocketHandler = std::function<void(const std::string& client_id, const std::string& message)>;

/**
 * HTTP Server interface for LiveVideoChat
 */
class LiveVideoChatServer {
public:
    LiveVideoChatServer();
    ~LiveVideoChatServer();
    
    /**
     * Initialize the server
     */
    bool initialize(int port = 3000);
    
    /**
     * Start the server
     */
    bool start();
    
    /**
     * Stop the server
     */
    void stop();
    
    /**
     * Set the LiveVideoChat instance to handle requests
     */
    void setVideoChatInstance(std::shared_ptr<LiveVideoChat> video_chat);
    
    /**
     * Register HTTP route handler
     */
    void registerRoute(const std::string& method, const std::string& path, HttpHandler handler);
    
    /**
     * Register WebSocket handler
     */
    void setWebSocketHandler(WebSocketHandler handler);
    
    /**
     * Send message to WebSocket client
     */
    bool sendWebSocketMessage(const std::string& client_id, const std::string& message);
    
    /**
     * Broadcast message to all WebSocket clients
     */
    void broadcastMessage(const std::string& message);
    
    /**
     * Check if server is running
     */
    bool isRunning() const;
    
    /**
     * Get server port
     */
    int getPort() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * Enhanced LiveVideoChat with HTTP server integration
 */
class LiveVideoChatWithServer : public LiveVideoChat {
public:
    LiveVideoChatWithServer();
    ~LiveVideoChatWithServer();
    
    /**
     * Initialize with HTTP server support
     */
    bool initialize(const VideoChatConfig& config = {}, int server_port = 3000);
    
    /**
     * Start the HTTP server
     */
    bool startServer();
    
    /**
     * Stop the HTTP server
     */
    void stopServer();
    
    /**
     * Get server instance
     */
    std::shared_ptr<LiveVideoChatServer> getServer();
    
    /**
     * Handle agent message API call
     */
    HttpResponse handleAgentMessage(const HttpRequest& request);
    
    /**
     * Handle Whisper transcription API call
     */
    HttpResponse handleWhisperTranscription(const HttpRequest& request);
    
    /**
     * Handle WebRTC signaling
     */
    HttpResponse handleWebRTCSignaling(const HttpRequest& request);
    
    /**
     * Handle session management
     */
    HttpResponse handleSessionManagement(const HttpRequest& request);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace elizaos