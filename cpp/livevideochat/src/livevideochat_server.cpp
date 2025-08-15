#include "elizaos/livevideochat_server.hpp"
#include "elizaos/agentlogger.hpp"
#include <thread>
#include <chrono>
#include <sstream>
#include <iostream>
#include <regex>

namespace elizaos {

// Global logger instance for the module
static AgentLogger g_logger;

/**
 * Simple HTTP server implementation
 */
class LiveVideoChatServer::Impl {
public:
    std::shared_ptr<LiveVideoChat> video_chat_;
    std::map<std::string, std::map<std::string, HttpHandler>> route_handlers_;
    WebSocketHandler websocket_handler_;
    std::map<std::string, std::string> websocket_clients_; // client_id -> connection_info
    
    int port_;
    bool running_;
    std::thread server_thread_;
    
    Impl() : port_(3000), running_(false) {}
    
    bool initialize(int port) {
        port_ = port;
        setupDefaultRoutes();
        g_logger.log("LiveVideoChatServer initialized on port " + std::to_string(port), 
                    "", "livevideochat_server", LogLevel::INFO);
        return true;
    }
    
    void setupDefaultRoutes() {
        // CORS preflight handler
        registerRoute("OPTIONS", ".*", [](const HttpRequest& /*req*/) -> HttpResponse {
            HttpResponse response;
            response.status_code = 204;
            return response;
        });
        
        // Health check
        registerRoute("GET", "/health", [](const HttpRequest& /*req*/) -> HttpResponse {
            HttpResponse response;
            response.body = R"({"status":"healthy","service":"LiveVideoChatServer"})";
            return response;
        });
        
        // API info
        registerRoute("GET", "/api/info", [this](const HttpRequest& /*req*/) -> HttpResponse {
            HttpResponse response;
            std::ostringstream json;
            json << "{";
            json << "\"service\":\"LiveVideoChatServer\",";
            json << "\"version\":\"1.0.0\",";
            json << "\"port\":" << port_ << ",";
            json << "\"endpoints\":[";
            json << "\"/health\",";
            json << "\"/api/info\",";
            json << "\"/:agent_id/message\",";
            json << "\"/:agent_id/whisper\",";
            json << "\"/webrtc/signaling\",";
            json << "\"/sessions\"";
            json << "]}";
            response.body = json.str();
            return response;
        });
    }
    
    void registerRoute(const std::string& method, const std::string& path, HttpHandler handler) {
        route_handlers_[method][path] = handler;
        g_logger.log("Registered route: " + method + " " + path, 
                    "", "livevideochat_server", LogLevel::INFO, LogColor::BLUE, true, false);
    }
    
    HttpResponse handleRequest(const HttpRequest& request) {
        // Find matching route
        auto method_it = route_handlers_.find(request.method);
        if (method_it != route_handlers_.end()) {
            for (const auto& route : method_it->second) {
                std::regex pattern(route.first);
                if (std::regex_match(request.path, pattern)) {
                    try {
                        return route.second(request);
                    } catch (const std::exception& e) {
                        g_logger.log("Error handling request: " + std::string(e.what()), 
                                    "", "livevideochat_server", LogLevel::ERROR);
                        HttpResponse error_response;
                        error_response.status_code = 500;
                        error_response.body = R"({"error":"Internal server error"})";
                        return error_response;
                    }
                }
            }
        }
        
        // Route not found
        HttpResponse not_found;
        not_found.status_code = 404;
        not_found.body = R"({"error":"Route not found"})";
        return not_found;
    }
    
    bool start() {
        if (running_) {
            return false;
        }
        
        running_ = true;
        server_thread_ = std::thread([this]() {
            runServer();
        });
        
        g_logger.log("LiveVideoChatServer started on port " + std::to_string(port_), 
                    "", "livevideochat_server", LogLevel::INFO);
        return true;
    }
    
    void stop() {
        if (running_) {
            running_ = false;
            if (server_thread_.joinable()) {
                server_thread_.join();
            }
            g_logger.log("LiveVideoChatServer stopped", "", "livevideochat_server", LogLevel::INFO);
        }
    }
    
    void runServer() {
        // Mock HTTP server implementation
        // In a real implementation, this would use a proper HTTP server library
        while (running_) {
            // Simulate handling requests
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Log periodic status (every 10 seconds)
            static auto last_log = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_log).count() >= 10) {
                g_logger.log("Server running on port " + std::to_string(port_) + 
                           " with " + std::to_string(route_handlers_.size()) + " route groups", 
                           "", "livevideochat_server", LogLevel::INFO, LogColor::BLUE, true, false);
                last_log = now;
            }
        }
    }
    
    bool sendWebSocketMessage(const std::string& client_id, const std::string& message) {
        // Mock WebSocket implementation
        g_logger.log("Sending WebSocket message to " + client_id + ": " + 
                    (message.length() > 100 ? message.substr(0, 100) + "..." : message), 
                    "", "livevideochat_server", LogLevel::INFO, LogColor::BLUE, true, false);
        return true;
    }
    
    void broadcastMessage(const std::string& message) {
        // Mock broadcast implementation
        g_logger.log("Broadcasting WebSocket message: " + 
                    (message.length() > 100 ? message.substr(0, 100) + "..." : message), 
                    "", "livevideochat_server", LogLevel::INFO, LogColor::BLUE, true, false);
    }
};

// LiveVideoChatServer implementation
LiveVideoChatServer::LiveVideoChatServer() : impl_(std::make_unique<Impl>()) {}
LiveVideoChatServer::~LiveVideoChatServer() = default;

bool LiveVideoChatServer::initialize(int port) {
    return impl_->initialize(port);
}

bool LiveVideoChatServer::start() {
    return impl_->start();
}

void LiveVideoChatServer::stop() {
    impl_->stop();
}

void LiveVideoChatServer::setVideoChatInstance(std::shared_ptr<LiveVideoChat> video_chat) {
    impl_->video_chat_ = video_chat;
    g_logger.log("Set LiveVideoChat instance for server", "", "livevideochat_server", LogLevel::INFO);
}

void LiveVideoChatServer::registerRoute(const std::string& method, const std::string& path, HttpHandler handler) {
    impl_->registerRoute(method, path, handler);
}

void LiveVideoChatServer::setWebSocketHandler(WebSocketHandler handler) {
    impl_->websocket_handler_ = handler;
    g_logger.log("Set WebSocket handler", "", "livevideochat_server", LogLevel::INFO);
}

bool LiveVideoChatServer::sendWebSocketMessage(const std::string& client_id, const std::string& message) {
    return impl_->sendWebSocketMessage(client_id, message);
}

void LiveVideoChatServer::broadcastMessage(const std::string& message) {
    impl_->broadcastMessage(message);
}

bool LiveVideoChatServer::isRunning() const {
    return impl_->running_;
}

int LiveVideoChatServer::getPort() const {
    return impl_->port_;
}

/**
 * Enhanced LiveVideoChat with server implementation
 */
class LiveVideoChatWithServer::Impl {
public:
    std::shared_ptr<LiveVideoChatServer> server_;
    VideoChatConfig config_;
    
    Impl() {
        server_ = std::make_shared<LiveVideoChatServer>();
    }
    
    bool initialize(const VideoChatConfig& config, int server_port) {
        config_ = config;
        
        if (!server_->initialize(server_port)) {
            return false;
        }
        
        setupAPIRoutes();
        g_logger.log("LiveVideoChatWithServer initialized", "", "livevideochat_server", LogLevel::INFO);
        return true;
    }
    
    void setupAPIRoutes() {
        // Agent message endpoint
        server_->registerRoute("POST", "/([^/]+)/message", 
            [this](const HttpRequest& req) -> HttpResponse {
                return handleAgentMessage(req);
            });
        
        // Whisper transcription endpoint
        server_->registerRoute("POST", "/([^/]+)/whisper", 
            [this](const HttpRequest& req) -> HttpResponse {
                return handleWhisperTranscription(req);
            });
        
        // WebRTC signaling endpoint
        server_->registerRoute("POST", "/webrtc/signaling", 
            [this](const HttpRequest& req) -> HttpResponse {
                return handleWebRTCSignaling(req);
            });
        
        // Session management
        server_->registerRoute("GET", "/sessions", 
            [this](const HttpRequest& req) -> HttpResponse {
                return handleSessionManagement(req);
            });
        
        server_->registerRoute("POST", "/sessions", 
            [this](const HttpRequest& req) -> HttpResponse {
                return handleSessionManagement(req);
            });
        
        server_->registerRoute("DELETE", "/sessions/([^/]+)", 
            [this](const HttpRequest& req) -> HttpResponse {
                return handleSessionManagement(req);
            });
    }
    
    HttpResponse handleAgentMessage(const HttpRequest& /*request*/) {
        HttpResponse response;
        
        // Mock agent response - in real implementation would integrate with Eliza agent
        std::ostringstream json;
        json << "[{";
        json << "\"text\":\"Hello! I'm the C++ LiveVideoChat agent. I received your message and I'm ready to help with video chat functionality.\",";
        json << "\"timestamp\":" << std::time(nullptr) << ",";
        json << "\"agent_id\":\"cpp_livevideochat\",";
        json << "\"session_id\":\"mock_session\"";
        json << "}]";
        
        response.body = json.str();
        
        g_logger.log("Handled agent message request", "", "livevideochat_server", LogLevel::INFO);
        return response;
    }
    
    HttpResponse handleWhisperTranscription(const HttpRequest& /*request*/) {
        HttpResponse response;
        
        // Mock transcription response - in real implementation would integrate with Whisper
        std::ostringstream json;
        json << "{";
        json << "\"text\":\"Mock transcription: Hello, this is a test transcription from the C++ server.\",";
        json << "\"confidence\":0.95,";
        json << "\"language\":\"en\",";
        json << "\"timestamp\":" << std::time(nullptr);
        json << "}";
        
        response.body = json.str();
        
        g_logger.log("Handled Whisper transcription request", "", "livevideochat_server", LogLevel::INFO);
        return response;
    }
    
    HttpResponse handleWebRTCSignaling(const HttpRequest& /*request*/) {
        HttpResponse response;
        
        // Mock WebRTC signaling response
        std::ostringstream json;
        json << "{";
        json << "\"type\":\"answer\",";
        json << "\"sdp\":\"Mock SDP answer from C++ server\",";
        json << "\"timestamp\":" << std::time(nullptr);
        json << "}";
        
        response.body = json.str();
        
        g_logger.log("Handled WebRTC signaling request", "", "livevideochat_server", LogLevel::INFO);
        return response;
    }
    
    HttpResponse handleSessionManagement(const HttpRequest& request) {
        HttpResponse response;
        
        if (request.method == "GET") {
            // List sessions
            std::ostringstream json;
            json << "{";
            json << "\"sessions\":[";
            json << "{\"id\":\"session1\",\"status\":\"active\",\"participants\":2},";
            json << "{\"id\":\"session2\",\"status\":\"idle\",\"participants\":0}";
            json << "],";
            json << "\"total\":2";
            json << "}";
            response.body = json.str();
        } else if (request.method == "POST") {
            // Create session
            std::ostringstream json;
            json << "{";
            json << "\"session_id\":\"new_session_" << std::time(nullptr) << "\",";
            json << "\"status\":\"created\",";
            json << "\"timestamp\":" << std::time(nullptr);
            json << "}";
            response.body = json.str();
            response.status_code = 201;
        } else if (request.method == "DELETE") {
            // Delete session
            std::ostringstream json;
            json << "{";
            json << "\"status\":\"deleted\",";
            json << "\"timestamp\":" << std::time(nullptr);
            json << "}";
            response.body = json.str();
        }
        
        g_logger.log("Handled session management request: " + request.method, 
                    "", "livevideochat_server", LogLevel::INFO);
        return response;
    }
};

LiveVideoChatWithServer::LiveVideoChatWithServer() : impl_(std::make_unique<Impl>()) {}
LiveVideoChatWithServer::~LiveVideoChatWithServer() = default;

bool LiveVideoChatWithServer::initialize(const VideoChatConfig& config, int server_port) {
    if (!LiveVideoChat::initialize(config)) {
        return false;
    }
    
    if (!impl_->initialize(config, server_port)) {
        return false;
    }
    
    // Set this instance as the video chat handler for the server
    impl_->server_->setVideoChatInstance(std::shared_ptr<LiveVideoChat>(this, [](LiveVideoChat*){}));
    
    return true;
}

bool LiveVideoChatWithServer::startServer() {
    return impl_->server_->start();
}

void LiveVideoChatWithServer::stopServer() {
    impl_->server_->stop();
}

std::shared_ptr<LiveVideoChatServer> LiveVideoChatWithServer::getServer() {
    return impl_->server_;
}

HttpResponse LiveVideoChatWithServer::handleAgentMessage(const HttpRequest& request) {
    return impl_->handleAgentMessage(request);
}

HttpResponse LiveVideoChatWithServer::handleWhisperTranscription(const HttpRequest& request) {
    return impl_->handleWhisperTranscription(request);
}

HttpResponse LiveVideoChatWithServer::handleWebRTCSignaling(const HttpRequest& request) {
    return impl_->handleWebRTCSignaling(request);
}

HttpResponse LiveVideoChatWithServer::handleSessionManagement(const HttpRequest& request) {
    return impl_->handleSessionManagement(request);
}

} // namespace elizaos