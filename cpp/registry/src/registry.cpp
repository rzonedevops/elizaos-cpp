#include "elizaos/registry.hpp"
#include "elizaos/agentlogger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>
#include <ctime>
#include <chrono>

#ifdef LIBCURL_FOUND
#include <curl/curl.h>
#endif

namespace elizaos {

// Global registry instance
static std::unique_ptr<Registry> g_globalRegistry = nullptr;
static std::mutex g_globalRegistryMutex;

// Helper function for HTTP responses
struct HttpResponse {
    std::string data;
    long responseCode = 0;
};

#ifdef LIBCURL_FOUND
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HttpResponse* response) {
    size_t totalSize = size * nmemb;
    response->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}
#endif

Registry::Registry() : config_(), lastRefreshTime_("") {
    logInfo("Registry initialized with default configuration", "registry");
}

Registry::Registry(const RegistryConfig& config) : config_(config), lastRefreshTime_("") {
    logInfo("Registry initialized with custom configuration", "registry");
}

Registry::~Registry() {
    logInfo("Registry destructor called", "registry");
}

std::future<bool> Registry::refreshRegistry() {
    return std::async(std::launch::async, [this]() {
        logInfo("Refreshing registry data...", "registry");
        
        // First try to load from remote if enabled
        if (config_.enableRemoteRegistry && !config_.registryUrl.empty()) {
            std::string registryData = downloadRegistryData(config_.registryUrl);
            if (!registryData.empty()) {
                if (parseRegistryJson(registryData)) {
                    saveToCache();
                    updateLastRefreshTime();
                    logInfo("Registry refreshed from remote source", "registry");
                    return true;
                }
            }
        }
        
        // Fall back to local cache
        if (loadFromCache()) {
            logInfo("Registry loaded from cache", "registry");
            return true;
        }
        
        logError("Failed to refresh registry data", "registry");
        return false;
    });
}

bool Registry::loadLocalRegistry(const std::string& registryFilePath) {
    std::string filePath = registryFilePath;
    if (filePath.empty()) {
        filePath = "registry/index.json"; // Default local path
    }
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            logError("Failed to open registry file: " + filePath, "registry");
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string jsonContent = buffer.str();
        
        if (parseRegistryJson(jsonContent)) {
            updateLastRefreshTime();
            logInfo("Local registry loaded from: " + filePath, "registry");
            return true;
        }
        
    } catch (const std::exception& e) {
        logError("Exception while loading local registry: " + std::string(e.what()), "registry");
    }
    
    return false;
}

std::vector<RegistryEntry> Registry::getAllPlugins() const {
    std::lock_guard<std::mutex> lock(registryMutex_);
    std::vector<RegistryEntry> plugins;
    plugins.reserve(entries_.size());
    
    for (const auto& [name, entry] : entries_) {
        plugins.push_back(entry);
    }
    
    return plugins;
}

std::vector<RegistryEntry> Registry::searchPlugins(const std::string& query) const {
    std::lock_guard<std::mutex> lock(registryMutex_);
    std::vector<RegistryEntry> results;
    
    std::regex searchRegex(query, std::regex_constants::icase);
    
    for (const auto& [name, entry] : entries_) {
        if (std::regex_search(entry.name, searchRegex) ||
            std::regex_search(entry.description, searchRegex) ||
            std::regex_search(entry.author, searchRegex)) {
            results.push_back(entry);
        }
    }
    
    return results;
}

std::optional<RegistryEntry> Registry::getPlugin(const std::string& name) const {
    std::lock_guard<std::mutex> lock(registryMutex_);
    auto it = entries_.find(name);
    if (it != entries_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Registry::isRegistryCached() const {
    return std::filesystem::exists(getCacheFilePath());
}

std::string Registry::getLastRefreshTime() const {
    std::lock_guard<std::mutex> lock(registryMutex_);
    return lastRefreshTime_;
}

size_t Registry::getPluginCount() const {
    std::lock_guard<std::mutex> lock(registryMutex_);
    return entries_.size();
}

void Registry::setConfig(const RegistryConfig& config) {
    std::lock_guard<std::mutex> lock(registryMutex_);
    config_ = config;
    logInfo("Registry configuration updated", "registry");
}

bool Registry::parseRegistryJson(const std::string& jsonContent) {
    try {
        auto json = nlohmann::json::parse(jsonContent);
        
        std::lock_guard<std::mutex> lock(registryMutex_);
        entries_.clear();
        
        // Handle different JSON formats
        if (json.is_object() && json.contains("__v2")) {
            // Handle v2 format
            if (json["__v2"].contains("packages")) {
                for (const auto& [name, packageInfo] : json["__v2"]["packages"].items()) {
                    RegistryEntry entry(name, packageInfo.is_string() ? packageInfo : "");
                    entries_[name] = entry;
                }
            }
        }
        
        // Handle flat format (main entries)
        for (const auto& [key, value] : json.items()) {
            if (key == "__v2") continue; // Skip metadata
            
            if (value.is_string()) {
                RegistryEntry entry(key, value);
                entries_[key] = entry;
            }
        }
        
        logInfo("Parsed " + std::to_string(entries_.size()) + " registry entries", "registry");
        return true;
        
    } catch (const std::exception& e) {
        logError("Failed to parse registry JSON: " + std::string(e.what()), "registry");
        return false;
    }
}

bool Registry::loadFromCache() {
    std::string cacheFile = getCacheFilePath();
    if (!std::filesystem::exists(cacheFile)) {
        return false;
    }
    
    try {
        std::ifstream file(cacheFile);
        if (!file.is_open()) {
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return parseRegistryJson(buffer.str());
        
    } catch (const std::exception& e) {
        logError("Failed to load from cache: " + std::string(e.what()), "registry");
        return false;
    }
}

bool Registry::saveToCache() const {
    try {
        std::string cacheDir = expandPath(config_.cacheDirectory);
        std::filesystem::create_directories(cacheDir);
        
        std::string cacheFile = getCacheFilePath();
        std::ofstream file(cacheFile);
        if (!file.is_open()) {
            return false;
        }
        
        // Create JSON from current entries
        nlohmann::json json;
        for (const auto& [name, entry] : entries_) {
            json[name] = entry.repositoryUrl;
        }
        
        file << json.dump(2);
        logInfo("Registry data saved to cache: " + cacheFile, "registry");
        return true;
        
    } catch (const std::exception& e) {
        logError("Failed to save to cache: " + std::string(e.what()), "registry");
        return false;
    }
}

std::string Registry::getCacheFilePath() const {
    return expandPath(config_.cacheDirectory) + "/registry_index.json";
}

bool Registry::isHttpUrl(const std::string& url) const {
    return url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://";
}

std::string Registry::downloadRegistryData(const std::string& url) const {
#ifdef LIBCURL_FOUND
    if (!isHttpUrl(url)) {
        return "";
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        logError("Failed to initialize CURL", "registry");
        return "";
    }
    
    HttpResponse response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.responseCode);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK || response.responseCode != 200) {
        logError("Failed to download registry data from: " + url, "registry");
        return "";
    }
    
    logInfo("Successfully downloaded registry data from: " + url, "registry");
    return response.data;
#else
    logError("HTTP support not available, cannot download from: " + url, "registry");
    return "";
#endif
}

std::string Registry::expandPath(const std::string& path) const {
    if (path.empty()) return path;
    
    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (home) {
            return std::string(home) + path.substr(1);
        }
    }
    
    return path;
}

void Registry::updateLastRefreshTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    std::lock_guard<std::mutex> lock(registryMutex_);
    lastRefreshTime_ = ss.str();
}

// Global registry functions
Registry& getGlobalRegistry() {
    std::lock_guard<std::mutex> lock(g_globalRegistryMutex);
    if (!g_globalRegistry) {
        g_globalRegistry = std::make_unique<Registry>();
    }
    return *g_globalRegistry;
}

void setGlobalRegistry(std::unique_ptr<Registry> registry) {
    std::lock_guard<std::mutex> lock(g_globalRegistryMutex);
    g_globalRegistry = std::move(registry);
}

} // namespace elizaos
