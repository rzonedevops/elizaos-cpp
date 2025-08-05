#include "elizaos/easycompletion.hpp"
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cstdlib>
#include <nlohmann/json.hpp>

#ifdef _WIN32
    #include <windows.h>
    #include <wininet.h>
    #pragma comment(lib, "wininet.lib")
#else
    #include <curl/curl.h>
#endif

using json = nlohmann::json;

namespace elizaos {

// Utility function to get environment variable
std::string get_env_var(const std::string& var_name, const std::string& default_value = "") {
    const char* env = std::getenv(var_name.c_str());
    return env ? std::string(env) : default_value;
}

// HTTP response callback for libcurl
#ifndef _WIN32
struct HttpResponse {
    std::string data;
    long response_code = 0;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HttpResponse* userp) {
    size_t realsize = size * nmemb;
    userp->data.append(static_cast<char*>(contents), realsize);
    return realsize;
}
#endif

EasyCompletionClient::EasyCompletionClient(const CompletionConfig& config) : config_(config) {
    // Set defaults from environment variables if not specified
    if (config_.api_key.empty()) {
        config_.api_key = get_env_var("EASYCOMPLETION_API_KEY", get_env_var("OPENAI_API_KEY"));
    }
    
    std::string env_endpoint = get_env_var("EASYCOMPLETION_API_ENDPOINT");
    if (!env_endpoint.empty()) {
        if (env_endpoint.find("http") != 0) {
            config_.api_endpoint = "http://" + env_endpoint + "/v1";
        } else {
            config_.api_endpoint = env_endpoint;
        }
    }
    
    std::string debug_env = get_env_var("EASYCOMPLETION_DEBUG");
    if (debug_env == "true" || debug_env == "1") {
        config_.debug = true;
    }
    
#ifndef _WIN32
    // Initialize libcurl if available
    curl_global_init(CURL_GLOBAL_DEFAULT);
#endif
}

std::string EasyCompletionClient::make_http_request(const std::string& url, const std::string& json_payload, const std::vector<std::string>& headers) {
    if (config_.debug) {
        std::cout << "Making request to: " << url << std::endl;
        std::cout << "Payload: " << json_payload << std::endl;
    }
    
#ifdef _WIN32
    // Windows implementation using WinINet (simplified)
    // For production, consider using a proper HTTP library
    return "{\"error\": \"HTTP requests not implemented for Windows yet\"}";
#else
    // Use libcurl if available
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "{\"error\": \"Failed to initialize HTTP client\"}";
    }
    
    HttpResponse response;
    
    // Set headers
    struct curl_slist* header_list = nullptr;
    for (const auto& header : headers) {
        header_list = curl_slist_append(header_list, header.c_str());
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.response_code);
    
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return "{\"error\": \"HTTP request failed: " + std::string(curl_easy_strerror(res)) + "\"}";
    }
    
    if (response.response_code != 200) {
        return "{\"error\": \"HTTP error " + std::to_string(response.response_code) + "\"}";
    }
    
    if (config_.debug) {
        std::cout << "Response: " << response.data << std::endl;
    }
    
    return response.data;
#endif
}

CompletionResponse EasyCompletionClient::text_completion(const std::string& text) {
    CompletionResponse result;
    
    if (config_.api_key.empty()) {
        result.error = "API key not provided";
        return result;
    }
    
    // Create JSON payload for text completion
    json payload = {
        {"model", config_.model},
        {"messages", json::array({
            {{"role", "user"}, {"content", text}}
        })},
        {"temperature", config_.temperature}
    };
    
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Authorization: Bearer " + config_.api_key
    };
    
    std::string url = config_.api_endpoint + "/chat/completions";
    std::string response_json = make_http_request(url, payload.dump(), headers);
    
    // Parse response with proper JSON parsing
    try {
        json response = json::parse(response_json);
        
        if (response.contains("error")) {
            result.error = response["error"].get<std::string>();
            return result;
        }
        
        if (response.contains("choices") && !response["choices"].empty()) {
            auto& choice = response["choices"][0];
            if (choice.contains("message") && choice["message"].contains("content")) {
                result.text = choice["message"]["content"].get<std::string>();
            }
            if (choice.contains("finish_reason")) {
                result.finish_reason = choice["finish_reason"].get<std::string>();
            }
        }
        
        if (response.contains("usage")) {
            auto& usage = response["usage"];
            if (usage.contains("prompt_tokens")) {
                result.usage.prompt_tokens = usage["prompt_tokens"].get<int>();
            }
            if (usage.contains("completion_tokens")) {
                result.usage.completion_tokens = usage["completion_tokens"].get<int>();
            }
            if (usage.contains("total_tokens")) {
                result.usage.total_tokens = usage["total_tokens"].get<int>();
            }
        }
        
    } catch (const json::exception& e) {
        result.error = "JSON parsing error: " + std::string(e.what());
        return result;
    }
    
    return result;
}

CompletionResponse EasyCompletionClient::chat_completion(const std::vector<ChatMessage>& messages) {
    CompletionResponse result;
    
    if (config_.api_key.empty()) {
        result.error = "API key not provided";
        return result;
    }
    
    // Create JSON payload for chat completion
    json messages_array = json::array();
    for (const auto& msg : messages) {
        messages_array.push_back({
            {"role", msg.role},
            {"content", msg.content}
        });
    }
    
    json payload = {
        {"model", config_.model},
        {"messages", messages_array},
        {"temperature", config_.temperature}
    };
    
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Authorization: Bearer " + config_.api_key
    };
    
    std::string url = config_.api_endpoint + "/chat/completions";
    std::string response_json = make_http_request(url, payload.dump(), headers);
    
    // Parse response with proper JSON parsing
    try {
        json response = json::parse(response_json);
        
        if (response.contains("error")) {
            result.error = response["error"].get<std::string>();
            return result;
        }
        
        if (response.contains("choices") && !response["choices"].empty()) {
            auto& choice = response["choices"][0];
            if (choice.contains("message") && choice["message"].contains("content")) {
                result.text = choice["message"]["content"].get<std::string>();
            }
            if (choice.contains("finish_reason")) {
                result.finish_reason = choice["finish_reason"].get<std::string>();
            }
        }
        
        if (response.contains("usage")) {
            auto& usage = response["usage"];
            if (usage.contains("prompt_tokens")) {
                result.usage.prompt_tokens = usage["prompt_tokens"].get<int>();
            }
            if (usage.contains("completion_tokens")) {
                result.usage.completion_tokens = usage["completion_tokens"].get<int>();
            }
            if (usage.contains("total_tokens")) {
                result.usage.total_tokens = usage["total_tokens"].get<int>();
            }
        }
        
    } catch (const json::exception& e) {
        result.error = "JSON parsing error: " + std::string(e.what());
        return result;
    }
    
    return result;
}

CompletionResponse EasyCompletionClient::function_completion(
    const std::string& text,
    const std::vector<FunctionDefinition>& functions,
    const std::optional<std::string>& function_call,
    const std::optional<std::string>& system_message,
    const std::vector<ChatMessage>& messages
) {
    CompletionResponse result;
    
    if (config_.api_key.empty()) {
        result.error = "API key not provided";
        return result;
    }
    
    if (functions.empty()) {
        result.error = "Functions list cannot be empty";
        return result;
    }
    
    // Create JSON payload for function completion
    std::ostringstream payload;
    payload << R"({
        "model": ")" << config_.model << R"(",
        "messages": [)";
    
    // Add system message if provided
    bool first_message = true;
    if (system_message.has_value()) {
        payload << R"({"role": "system", "content": ")" << system_message.value() << R"("})";
        first_message = false;
    }
    
    // Add existing messages
    for (const auto& msg : messages) {
        if (!first_message) payload << ",";
        payload << R"({"role": ")" << msg.role << R"(", "content": ")" << msg.content << R"("})";
        first_message = false;
    }
    
    // Add user message
    if (!first_message) payload << ",";
    payload << R"({"role": "user", "content": ")" << text << R"("})";
    
    payload << R"(],
        "functions": [)";
    
    // Add function definitions
    for (size_t i = 0; i < functions.size(); ++i) {
        if (i > 0) payload << ",";
        payload << R"({"name": ")" << functions[i].name << R"(", "description": ")" << functions[i].description << R"("})";
        // Note: Simplified function schema - would need proper JSON schema in production
    }
    
    payload << R"(],
        "temperature": )" << config_.temperature;
    
    if (function_call.has_value()) {
        payload << R"(, "function_call": {"name": ")" << function_call.value() << R"("})";
    }
    
    payload << "}";
    
    std::vector<std::string> headers = {
        "Content-Type: application/json",
        "Authorization: Bearer " + config_.api_key
    };
    
    std::string url = config_.api_endpoint + "/chat/completions";
    std::string response_json = make_http_request(url, payload.str(), headers);
    
    // Parse response (simplified)
    if (response_json.find("\"error\"") != std::string::npos) {
        result.error = "API request failed";
        return result;
    }
    
    // Extract function call information (simplified parsing)
    size_t func_call_start = response_json.find("\"function_call\"");
    if (func_call_start != std::string::npos) {
        size_t name_start = response_json.find("\"name\":", func_call_start);
        if (name_start != std::string::npos) {
            name_start = response_json.find("\"", name_start + 7);
            if (name_start != std::string::npos) {
                size_t name_end = response_json.find("\"", name_start + 1);
                if (name_end != std::string::npos) {
                    result.function_name = response_json.substr(name_start + 1, name_end - name_start - 1);
                }
            }
        }
    }
    
    result.finish_reason = "function_call";
    return result;
}

void EasyCompletionClient::set_config(const CompletionConfig& config) {
    config_ = config;
}

const CompletionConfig& EasyCompletionClient::get_config() const {
    return config_;
}

// Utility functions implementation

std::string compose_prompt(const std::string& template_str, const std::unordered_map<std::string, std::string>& variables) {
    std::string result = template_str;
    
    for (const auto& pair : variables) {
        std::string placeholder = "{{" + pair.first + "}}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), pair.second);
            pos += pair.second.length();
        }
    }
    
    return result;
}

FunctionDefinition compose_function(
    const std::string& name,
    const std::string& description,
    const std::unordered_map<std::string, std::string>& properties,
    const std::vector<std::string>& required_properties
) {
    FunctionDefinition func;
    func.name = name;
    func.description = description;
    func.properties = properties;
    func.required_properties = required_properties;
    return func;
}

int count_tokens(const std::string& text) {
    // Simple approximation: ~4 characters per token for English text
    return static_cast<int>(text.length() / 4);
}

std::string trim_prompt(const std::string& text, int max_tokens, bool preserve_top) {
    int estimated_tokens = count_tokens(text);
    if (estimated_tokens <= max_tokens) {
        return text;
    }
    
    // Simple trimming - would need proper tokenizer in production
    int chars_to_keep = max_tokens * 4;
    if (preserve_top) {
        return text.substr(0, chars_to_keep);
    } else {
        return text.substr(text.length() - chars_to_keep);
    }
}

std::vector<std::string> chunk_prompt(const std::string& prompt, int chunk_length) {
    std::vector<std::string> chunks;
    int chars_per_chunk = chunk_length * 4; // Approximate
    
    for (size_t i = 0; i < prompt.length(); i += chars_per_chunk) {
        chunks.push_back(prompt.substr(i, chars_per_chunk));
    }
    
    return chunks;
}

// Convenience functions
CompletionResponse text_completion(const std::string& text, const std::string& model, const std::string& api_key) {
    CompletionConfig config;
    config.model = model;
    if (!api_key.empty()) {
        config.api_key = api_key;
    }
    
    EasyCompletionClient client(config);
    return client.text_completion(text);
}

CompletionResponse chat_completion(const std::vector<ChatMessage>& messages, const std::string& model, const std::string& api_key) {
    CompletionConfig config;
    config.model = model;
    if (!api_key.empty()) {
        config.api_key = api_key;
    }
    
    EasyCompletionClient client(config);
    return client.chat_completion(messages);
}

CompletionResponse function_completion(
    const std::string& text,
    const std::vector<FunctionDefinition>& functions,
    const std::string& function_call,
    const std::string& model,
    const std::string& api_key
) {
    CompletionConfig config;
    config.model = model;
    if (!api_key.empty()) {
        config.api_key = api_key;
    }
    
    EasyCompletionClient client(config);
    
    std::optional<std::string> func_call;
    if (function_call != "auto") {
        func_call = function_call;
    }
    
    return client.function_completion(text, functions, func_call);
}

} // namespace elizaos
