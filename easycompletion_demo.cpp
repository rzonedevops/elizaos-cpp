#include <iostream>
#include <vector>
#include "elizaos/easycompletion.hpp"

using namespace elizaos;

int main() {
    std::cout << "=== ElizaOS EasyCompletion Demo ===" << std::endl;
    
    // Demo 1: Compose Prompt
    std::cout << "\n1. Compose Prompt Demo:" << std::endl;
    std::string template_str = "Hello {{name}}, welcome to {{place}}! Today is {{day}}.";
    std::unordered_map<std::string, std::string> variables = {
        {"name", "Alice"},
        {"place", "ElizaOS"},
        {"day", "Monday"}
    };
    
    std::string composed = compose_prompt(template_str, variables);
    std::cout << "Template: " << template_str << std::endl;
    std::cout << "Result: " << composed << std::endl;
    
    // Demo 2: Token Counting
    std::cout << "\n2. Token Counting Demo:" << std::endl;
    std::string text = "This is a sample text for token counting demonstration.";
    int tokens = count_tokens(text);
    std::cout << "Text: \"" << text << "\"" << std::endl;
    std::cout << "Estimated tokens: " << tokens << std::endl;
    
    // Demo 3: Prompt Trimming
    std::cout << "\n3. Prompt Trimming Demo:" << std::endl;
    std::string long_text = "This is a very long text that needs to be trimmed when it exceeds the maximum token limit. We want to demonstrate how the trimming function works.";
    std::string trimmed = trim_prompt(long_text, 10, true);
    std::cout << "Original: \"" << long_text << "\"" << std::endl;
    std::cout << "Trimmed (10 tokens): \"" << trimmed << "\"" << std::endl;
    
    // Demo 4: Chunk Prompt
    std::cout << "\n4. Chunk Prompt Demo:" << std::endl;
    std::string chunk_text = "This text will be split into multiple chunks for processing by the AI system.";
    std::vector<std::string> chunks = chunk_prompt(chunk_text, 8);
    std::cout << "Original: \"" << chunk_text << "\"" << std::endl;
    std::cout << "Split into " << chunks.size() << " chunks:" << std::endl;
    for (size_t i = 0; i < chunks.size(); ++i) {
        std::cout << "  Chunk " << (i+1) << ": \"" << chunks[i] << "\"" << std::endl;
    }
    
    // Demo 5: Function Definition
    std::cout << "\n5. Function Definition Demo:" << std::endl;
    std::unordered_map<std::string, std::string> properties = {
        {"lyrics", "string - The lyrics for the song"},
        {"genre", "string - The musical genre"},
        {"duration", "number - Song duration in minutes"}
    };
    std::vector<std::string> required = {"lyrics", "genre"};
    
    FunctionDefinition song_func = compose_function(
        "write_song",
        "Write a song about AI and technology",
        properties,
        required
    );
    
    std::cout << "Function Name: " << song_func.name << std::endl;
    std::cout << "Description: " << song_func.description << std::endl;
    std::cout << "Properties: " << song_func.properties.size() << " defined" << std::endl;
    std::cout << "Required: " << song_func.required_properties.size() << " properties" << std::endl;
    
    // Demo 6: Completion Client Configuration
    std::cout << "\n6. Completion Client Demo:" << std::endl;
    CompletionConfig config;
    config.model = "gpt-3.5-turbo";
    config.api_key = "your-api-key-here";
    config.temperature = 0.7f;
    config.debug = true;
    
    EasyCompletionClient client(config);
    std::cout << "Client configured with model: " << client.get_config().model << std::endl;
    std::cout << "Temperature: " << client.get_config().temperature << std::endl;
    
    // Demo 7: Text Completion (will show structure but not make real API calls)
    std::cout << "\n7. Text Completion Structure Demo:" << std::endl;
    std::cout << "Note: This would make an API call if a valid key was provided" << std::endl;
    
    CompletionResponse response = client.text_completion("Hello, how are you?");
    if (response.error.has_value()) {
        std::cout << "Expected error (no valid API key): " << response.error.value() << std::endl;
    }
    
    // Demo 8: Chat Messages Structure
    std::cout << "\n8. Chat Messages Structure Demo:" << std::endl;
    std::vector<ChatMessage> messages = {
        {"system", "You are a helpful assistant."},
        {"user", "What is artificial intelligence?"},
        {"assistant", "AI is a field of computer science..."},
        {"user", "Can you explain more about machine learning?"}
    };
    
    std::cout << "Chat conversation with " << messages.size() << " messages:" << std::endl;
    for (const auto& msg : messages) {
        std::cout << "  " << msg.role << ": " << msg.content.substr(0, 50) << "..." << std::endl;
    }
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    std::cout << "The EasyCompletion module provides a simple C++ interface for AI completions." << std::endl;
    std::cout << "To use with real API calls, set the EASYCOMPLETION_API_KEY environment variable." << std::endl;
    
    return 0;
}