# Eliza Plugin Starter Template

This repository provides a comprehensive starter template for creating plugins for the [Eliza](https://github.com/ai16z/eliza) AI agent framework. It includes five complete example implementations demonstrating different plugin patterns and integrations.

## Available Plugin Examples

1. **Search Plugins**
   - **Tavily Search**: Web search using Tavily API for news, academic, and general web content
   - **Exa Search**: Semantic search using Exa API for code, documents, and specialized content

2. **Utility Plugins**
   - **Calculator**: Basic arithmetic calculations (addition, subtraction, multiplication, division)
   - **Weather**: Weather information using OpenWeatherMap API
   - **Translation**: Text translation between different languages (with mock implementation)

## Prerequisites

- Node.js 23+
- pnpm
- TypeScript knowledge

## Getting Started

1. Clone this repository:
```bash
git clone https://github.com/yourusername/eliza-plugin-starter.git
cd eliza-plugin-starter
```

2. Install dependencies:
```bash
pnpm install
```

3. Set up environment variables (optional, for API-based plugins):
```bash
# Create .env file
cp .env.example .env

# Add your API keys
TAVILY_API_KEY=your_tavily_key_here
EXA_API_KEY=your_exa_key_here
OPENWEATHER_API_KEY=your_openweather_key_here
```

4. Compile the TypeScript code:
```bash
pnpm build
```

5. Run the project using the 'direct' client:
```bash
pnpm exec node --loader ts-node/esm ./src/scripts/load-with-plugin.ts --characters=./characters/eternalai.character.json
```

**Note:** Only the 'direct' client will work within this repo since it uses mocked capabilities of the real client. Plugins developed here can be directly transposed into the main Eliza repository.

## Project Overview

This starter template is designed to work with the 'direct' client within this repository due to the mocked capabilities of the real client. Plugins developed here are fully compatible with the main Eliza repository and can be directly transposed.

## Project Structure

```
src/
  ├── plugins/
  │   ├── calculator/     # Basic arithmetic calculator plugin
  │   ├── weather/        # Weather information plugin with OpenWeatherMap
  │   ├── translation/    # Text translation plugin with mock implementation
  │   ├── tavily/         # Tavily search plugin implementation
  │   └── exa/            # Exa search plugin implementation
  ├── common/             # Shared utilities and types
  └── scripts/            # Loading and runtime scripts
```

## Plugin Features

### Calculator Plugin
- Basic arithmetic operations (+, -, *, /)
- Input validation and error handling
- No external dependencies required
- Demonstrates simple action and evaluator patterns

### Weather Plugin  
- Real-time weather data via OpenWeatherMap API
- Location parsing from natural language
- Provider pattern for data fetching
- Comprehensive error handling

### Translation Plugin
- Text translation between languages
- Service integration pattern
- Mock implementation (easily replaceable with real API)
- Demonstrates service configuration

### Search Plugins (Tavily & Exa)
- Web search capabilities
- Rate limiting and API error handling
- Different search types and filtering
- External API integration examples

## Creating a Plugin

See the [Plugin Development Guide](docs/PLUGIN_GUIDE.md) for detailed instructions on creating your own plugin.

## Running the Project

You can run the project using the following command:

```bash
pnpm exec node --loader ts-node/esm ./src/scripts/load-with-plugin.ts --characters=./characters/eternalai.character.json
```

**Alternatively,** to simplify this process, use the predefined script:

```bash
pnpm mock-eliza --characters=./characters/eternalai.character.json
```

This script will prompt for a comma-separated list of character files to load.

**Note:** The 'mock-eliza' script uses the 'direct' client because the project contains mocked capabilities of the real client.

## Example Plugins

This template includes five example plugin implementations:

1. **Calculator Plugin**: Demonstrates basic arithmetic operations with local computation
2. **Weather Plugin**: Shows external API integration with the OpenWeatherMap service
3. **Translation Plugin**: Illustrates service pattern with mock translation functionality
4. **Tavily Search Plugin**: Demonstrates web search capabilities using the Tavily API
5. **Exa Search Plugin**: Shows semantic search integration with the Exa API

Check the individual plugin directories for specific documentation and usage instructions.

## Contributing

Contributions are welcome! Please read our [Contributing Guide](CONTRIBUTING.md) for details.

## License

MIT
