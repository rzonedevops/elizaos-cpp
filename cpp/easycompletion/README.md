# EasyCompletion C++ Module

A C++ implementation of easy text and chat completion functionality for the ElizaOS framework, providing a simple interface to OpenAI-compatible APIs.

## Features

- **Text Completion**: Send text prompts and receive AI-generated responses
- **Chat Completion**: Manage conversation history with multiple message types
- **Function Calling**: Define and call functions through AI models
- **Prompt Utilities**: Template composition, token counting, and text processing
- **Configurable**: Support for different models, endpoints, and settings
- **Compatible**: Works with OpenAI API and LocalAI installations

## Quick Start

### Basic Usage

```cpp
#include "elizaos/easycompletion.hpp"
using namespace elizaos;

int main() {
    // Simple text completion
    CompletionResponse response = text_completion("Hello, how are you?");
    std::cout << response.text << std::endl;
    
    return 0;
}
```

### Using the Client Class

```cpp
#include "elizaos/easycompletion.hpp"
using namespace elizaos;

int main() {
    // Configure the client
    CompletionConfig config;
    config.model = "gpt-3.5-turbo";
    config.api_key = "your-api-key-here";
    config.temperature = 0.7f;
    
    EasyCompletionClient client(config);
    
    // Text completion
    CompletionResponse response = client.text_completion("Explain quantum computing");
    if (!response.error.has_value()) {
        std::cout << "Response: " << response.text << std::endl;
    }
    
    return 0;
}
```

### Chat Completion

```cpp
#include "elizaos/easycompletion.hpp"
using namespace elizaos;

int main() {
    EasyCompletionClient client; // Uses environment variables
    
    std::vector<ChatMessage> conversation = {
        {"system", "You are a helpful assistant."},
        {"user", "What is artificial intelligence?"},
        {"assistant", "AI is a field of computer science..."},
        {"user", "Can you give me an example?"}
    };
    
    CompletionResponse response = client.chat_completion(conversation);
    std::cout << response.text << std::endl;
    
    return 0;
}
```

### Function Calling

```cpp
#include "elizaos/easycompletion.hpp"
using namespace elizaos;

int main() {
    // Define a function
    FunctionDefinition weather_func = compose_function(
        "get_weather",
        "Get the current weather for a location",
        {{"location", "string - The city name"}},
        {"location"}
    );
    
    EasyCompletionClient client;
    CompletionResponse response = client.function_completion(
        "What's the weather like in Paris?",
        {weather_func}
    );
    
    if (response.function_name == "get_weather") {
        std::cout << "Function called: " << response.function_name << std::endl;
        // Access arguments from response.arguments
    }
    
    return 0;
}
```

### Prompt Utilities

```cpp
#include "elizaos/easycompletion.hpp"
using namespace elizaos;

int main() {
    // Compose prompts with templates
    std::string template_str = "Hello {{name}}, welcome to {{place}}!";
    std::unordered_map<std::string, std::string> vars = {
        {"name", "Alice"},
        {"place", "ElizaOS"}
    };
    std::string prompt = compose_prompt(template_str, vars);
    // Result: "Hello Alice, welcome to ElizaOS!"
    
    // Count tokens
    int tokens = count_tokens("This is a test sentence.");
    std::cout << "Tokens: " << tokens << std::endl;
    
    // Trim long prompts
    std::string long_text = "Very long text here...";
    std::string trimmed = trim_prompt(long_text, 100); // Max 100 tokens
    
    // Split into chunks
    std::vector<std::string> chunks = chunk_prompt(long_text, 50);
    
    return 0;
}
```

## Configuration

### Environment Variables

- `EASYCOMPLETION_API_KEY` or `OPENAI_API_KEY`: Your API key
- `EASYCOMPLETION_API_ENDPOINT`: Custom API endpoint (default: OpenAI)
- `EASYCOMPLETION_DEBUG`: Set to "true" for debug output

### Configuration Object

```cpp
CompletionConfig config;
config.model = "gpt-3.5-turbo";           // Model name
config.api_key = "your-key";              // API key
config.api_endpoint = "https://...";      // Custom endpoint
config.temperature = 0.7f;               // Response randomness
config.model_failure_retries = 5;        // Retry attempts
config.function_failure_retries = 10;    // Function retry attempts
config.chunk_length = 4000;              // Max tokens per request
config.debug = false;                     // Debug output
```

## Local AI Support

EasyCompletion works with LocalAI for running models locally:

```bash
# Set up LocalAI endpoint
export EASYCOMPLETION_API_ENDPOINT=localhost:8080
export EASYCOMPLETION_API_KEY=not-needed-for-local
```

## Building

The module is automatically built as part of the ElizaOS project:

```bash
mkdir build && cd build
cmake ..
make elizaos-easycompletion
```

### Dependencies

- libcurl (for HTTP requests on Linux/macOS)
- nlohmann/json (for JSON handling)
- elizaos-core (base framework)

## Testing

Run the test suite:

```bash
cd build
./cpp/tests/elizaos_tests --gtest_filter="*EasyCompletion*"
```

## Demo

Run the included demo:

```bash
cd build
./easycompletion_demo
```

## Error Handling

All completion functions return a `CompletionResponse` object:

```cpp
struct CompletionResponse {
    std::string text;                      // Generated text
    std::string function_name;             // Called function name
    std::unordered_map<std::string, std::string> arguments; // Function args
    std::string finish_reason;             // Completion reason
    std::optional<std::string> error;      // Error message if any
    Usage usage;                           // Token usage stats
};
```

Check for errors:

```cpp
CompletionResponse response = client.text_completion("Hello");
if (response.error.has_value()) {
    std::cerr << "Error: " << response.error.value() << std::endl;
} else {
    std::cout << "Success: " << response.text << std::endl;
}
```

## License

This module is part of the ElizaOS project and follows the same MIT license.