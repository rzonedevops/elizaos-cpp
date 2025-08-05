#include <gtest/gtest.h>
#include "elizaos/easycompletion.hpp"

namespace elizaos {
namespace test {

class EasyCompletionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up test configuration
        config_.model = "gpt-3.5-turbo";
        config_.api_key = "test_key";
        config_.api_endpoint = "https://api.openai.com/v1";
        config_.debug = false;
    }

    CompletionConfig config_;
};

TEST_F(EasyCompletionTest, ConstructorWithConfig) {
    EasyCompletionClient client(config_);
    
    // The client should be created successfully
    EXPECT_EQ(client.get_config().model, "gpt-3.5-turbo");
    EXPECT_EQ(client.get_config().api_key, "test_key");
}

TEST_F(EasyCompletionTest, ComposePrompt) {
    std::string template_str = "Hello {{name}}, welcome to {{place}}!";
    std::unordered_map<std::string, std::string> variables = {
        {"name", "Alice"},
        {"place", "Wonderland"}
    };
    
    std::string result = compose_prompt(template_str, variables);
    EXPECT_EQ(result, "Hello Alice, welcome to Wonderland!");
}

TEST_F(EasyCompletionTest, ComposePromptMultipleOccurrences) {
    std::string template_str = "{{greeting}} {{name}}, {{greeting}} again!";
    std::unordered_map<std::string, std::string> variables = {
        {"greeting", "Hello"},
        {"name", "Bob"}
    };
    
    std::string result = compose_prompt(template_str, variables);
    EXPECT_EQ(result, "Hello Bob, Hello again!");
}

TEST_F(EasyCompletionTest, ComposeFunction) {
    std::unordered_map<std::string, std::string> properties = {
        {"lyrics", "string"},
        {"genre", "string"}
    };
    std::vector<std::string> required = {"lyrics"};
    
    FunctionDefinition func = compose_function(
        "write_song",
        "Write a song about AI",
        properties,
        required
    );
    
    EXPECT_EQ(func.name, "write_song");
    EXPECT_EQ(func.description, "Write a song about AI");
    EXPECT_EQ(func.properties.size(), 2);
    EXPECT_EQ(func.required_properties.size(), 1);
    EXPECT_EQ(func.required_properties[0], "lyrics");
}

TEST_F(EasyCompletionTest, CountTokens) {
    std::string text = "This is a test string for token counting.";
    int tokens = count_tokens(text);
    
    // Should be approximately text.length() / 4
    EXPECT_GT(tokens, 0);
    EXPECT_LT(tokens, static_cast<int>(text.length())); // Should be less than character count
}

TEST_F(EasyCompletionTest, TrimPrompt) {
    std::string long_text = "This is a very long text that should be trimmed when it exceeds the maximum token limit.";
    
    std::string trimmed = trim_prompt(long_text, 5, true); // Limit to 5 tokens
    
    // Should be shorter than original
    EXPECT_LT(trimmed.length(), long_text.length());
    
    // Should preserve top when preserve_top is true
    EXPECT_EQ(trimmed, long_text.substr(0, trimmed.length()));
}

TEST_F(EasyCompletionTest, ChunkPrompt) {
    std::string text = "This is a test string that will be split into multiple chunks for processing.";
    
    std::vector<std::string> chunks = chunk_prompt(text, 5); // 5 tokens per chunk (approx 20 chars)
    
    // Should have multiple chunks
    EXPECT_GT(chunks.size(), 1);
    
    // Concatenating chunks should give back original text
    std::string reconstructed;
    for (const auto& chunk : chunks) {
        reconstructed += chunk;
    }
    EXPECT_EQ(reconstructed, text);
}

TEST_F(EasyCompletionTest, TextCompletionWithoutApiKey) {
    CompletionConfig empty_config;
    empty_config.api_key = ""; // No API key
    
    EasyCompletionClient client(empty_config);
    CompletionResponse response = client.text_completion("Hello, world!");
    
    // Should return error when no API key
    EXPECT_TRUE(response.error.has_value());
    EXPECT_EQ(response.error.value(), "API key not provided");
}

TEST_F(EasyCompletionTest, ChatCompletionWithoutApiKey) {
    CompletionConfig empty_config;
    empty_config.api_key = ""; // No API key
    
    EasyCompletionClient client(empty_config);
    std::vector<ChatMessage> messages = {
        {"user", "Hello, how are you?"}
    };
    
    CompletionResponse response = client.chat_completion(messages);
    
    // Should return error when no API key
    EXPECT_TRUE(response.error.has_value());
    EXPECT_EQ(response.error.value(), "API key not provided");
}

TEST_F(EasyCompletionTest, FunctionCompletionWithoutApiKey) {
    CompletionConfig empty_config;
    empty_config.api_key = ""; // No API key
    
    EasyCompletionClient client(empty_config);
    std::vector<FunctionDefinition> functions = {
        compose_function("test_func", "A test function", {{"param", "string"}})
    };
    
    CompletionResponse response = client.function_completion("Call test function", functions);
    
    // Should return error when no API key
    EXPECT_TRUE(response.error.has_value());
    EXPECT_EQ(response.error.value(), "API key not provided");
}

TEST_F(EasyCompletionTest, FunctionCompletionWithEmptyFunctions) {
    EasyCompletionClient client(config_);
    std::vector<FunctionDefinition> empty_functions;
    
    CompletionResponse response = client.function_completion("Call test function", empty_functions);
    
    // Should return error when functions list is empty
    EXPECT_TRUE(response.error.has_value());
    EXPECT_EQ(response.error.value(), "Functions list cannot be empty");
}

TEST_F(EasyCompletionTest, ConvenienceFunctions) {
    // Test convenience functions (they should create clients internally)
    // Note: These will fail with HTTP errors due to test environment, but we test structure
    
    CompletionResponse text_resp = text_completion("Hello", "gpt-3.5-turbo", "test_key");
    // Should have attempted to make request (will fail due to fake key, but structure is correct)
    
    std::vector<ChatMessage> messages = {{"user", "Hello"}};
    CompletionResponse chat_resp = chat_completion(messages, "gpt-3.5-turbo", "test_key");
    // Should have attempted to make request
    
    std::vector<FunctionDefinition> functions = {
        compose_function("test", "test", {{"p", "string"}})
    };
    CompletionResponse func_resp = function_completion("Test", functions, "auto", "gpt-3.5-turbo", "test_key");
    // Should have attempted to make request
}

} // namespace test
} // namespace elizaos