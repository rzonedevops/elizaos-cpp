#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <optional>

namespace elizaos {

/**
 * Response structure for completion operations
 */
struct CompletionResponse {
    std::string text;
    std::string function_name;
    std::unordered_map<std::string, std::string> arguments;
    std::string finish_reason;
    std::optional<std::string> error;
    
    // Usage statistics
    struct Usage {
        int prompt_tokens = 0;
        int completion_tokens = 0;
        int total_tokens = 0;
    } usage;
};

/**
 * Function definition for function calling
 */
struct FunctionDefinition {
    std::string name;
    std::string description;
    std::unordered_map<std::string, std::string> properties;
    std::vector<std::string> required_properties;
};

/**
 * Chat message for chat completion
 */
struct ChatMessage {
    std::string role;  // "user", "assistant", "system"
    std::string content;
};

/**
 * Configuration for completion operations
 */
struct CompletionConfig {
    std::string model = "gpt-3.5-turbo";
    std::string api_key;
    std::string api_endpoint = "https://api.openai.com/v1";
    int model_failure_retries = 5;
    int function_failure_retries = 10;
    int chunk_length = 4000;
    float temperature = 0.0f;
    bool debug = false;
};

/**
 * Easy completion client for OpenAI-compatible APIs
 */
class EasyCompletionClient {
public:
    explicit EasyCompletionClient(const CompletionConfig& config = {});
    
    /**
     * Send text and get a text completion response
     */
    CompletionResponse text_completion(const std::string& text);
    
    /**
     * Send messages and get a chat completion response
     */
    CompletionResponse chat_completion(const std::vector<ChatMessage>& messages);
    
    /**
     * Send text with functions and get a function call response
     */
    CompletionResponse function_completion(
        const std::string& text,
        const std::vector<FunctionDefinition>& functions,
        const std::optional<std::string>& function_call = std::nullopt,
        const std::optional<std::string>& system_message = std::nullopt,
        const std::vector<ChatMessage>& messages = {}
    );
    
    /**
     * Update configuration
     */
    void set_config(const CompletionConfig& config);
    const CompletionConfig& get_config() const;

private:
    CompletionConfig config_;
    
    // Internal HTTP request method
    std::string make_http_request(const std::string& url, const std::string& json_payload, const std::vector<std::string>& headers);
    
    // Validation methods
    bool validate_functions(const CompletionResponse& response, const std::vector<FunctionDefinition>& functions, const std::string& expected_function);
    std::unordered_map<std::string, std::string> parse_arguments(const std::string& args_json);
};

// Utility functions

/**
 * Compose a prompt using template variables
 * Example: compose_prompt("Hello {{name}}!", {{"name", "World"}})
 */
std::string compose_prompt(const std::string& template_str, const std::unordered_map<std::string, std::string>& variables);

/**
 * Create a function definition for function calling
 */
FunctionDefinition compose_function(
    const std::string& name,
    const std::string& description,
    const std::unordered_map<std::string, std::string>& properties,
    const std::vector<std::string>& required_properties = {}
);

/**
 * Simple token counter (approximate)
 */
int count_tokens(const std::string& text);

/**
 * Trim prompt to maximum token count
 */
std::string trim_prompt(const std::string& text, int max_tokens = 4000, bool preserve_top = true);

/**
 * Split prompt into chunks
 */
std::vector<std::string> chunk_prompt(const std::string& prompt, int chunk_length = 4000);

// Convenience functions for backward compatibility

/**
 * Simple text completion function
 */
CompletionResponse text_completion(
    const std::string& text,
    const std::string& model = "gpt-3.5-turbo",
    const std::string& api_key = ""
);

/**
 * Simple chat completion function
 */
CompletionResponse chat_completion(
    const std::vector<ChatMessage>& messages,
    const std::string& model = "gpt-3.5-turbo",
    const std::string& api_key = ""
);

/**
 * Simple function completion function
 */
CompletionResponse function_completion(
    const std::string& text,
    const std::vector<FunctionDefinition>& functions,
    const std::string& function_call = "auto",
    const std::string& model = "gpt-3.5-turbo",
    const std::string& api_key = ""
);

} // namespace elizaos