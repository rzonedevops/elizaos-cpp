# Plugin Development Guide for Eliza Plugins

This guide provides step-by-step instructions for implementing plugins for the Eliza AI agent framework, including search, calculation, weather, and translation capabilities.

## Available Plugin Examples

This starter template includes five example plugin implementations:

1. **Tavily Search Plugin** - Web search using Tavily API
2. **Exa Search Plugin** - Semantic search using Exa API  
3. **Calculator Plugin** - Basic arithmetic calculations
4. **Weather Plugin** - Weather information with OpenWeatherMap integration
5. **Translation Plugin** - Text translation between languages

## Prerequisites

1. Node.js 23 or higher
2. pnpm package manager
3. TypeScript knowledge
4. API keys for external services (when needed):
   - Tavily API key from [Tavily](https://tavily.com)
   - Exa API key from [Exa](https://exa.ai)
   - OpenWeatherMap API key from [OpenWeatherMap](https://openweathermap.org/api)

## Project Setup

1. Clone the starter repository:

   ```bash
   git clone https://github.com/tmc/eliza-plugin-starter
   cd eliza-plugin-starter
   pnpm install
   ```

2. Set up your environment:

   ```bash
   # Create a .env file
   touch .env

   # Add your API keys
   echo "TAVILY_API_KEY=your_key_here" >> .env
   echo "EXA_API_KEY=your_key_here" >> .env
   echo "OPENWEATHER_API_KEY=your_key_here" >> .env
   ```

## Plugin Structure

Each search plugin follows this structure:

```typescript
// PLACEHOLDER: imports

export interface SearchPluginConfig {
  apiKey: string;
  maxResults?: number;
  searchType?: string;
  filters?: Record<string, unknown>;
}

export class SearchPlugin implements Plugin {
  name: string;
  description: string;
  config: SearchPluginConfig;
  actions: SearchAction[];
}
```

Please refer to the [official documentation](https://ai16z.github.io/eliza/docs/packages/plugins/) for further details about how to implement a plugin for your specific use case.

## Implementing Tavily Search Plugin

1. **Create Plugin Configuration**

   ```typescript
   interface TavilyPluginConfig extends SearchPluginConfig {
     searchType?: "search" | "news" | "academic";
   }

   const DEFAULT_CONFIG: Partial<TavilyPluginConfig> = {
     maxResults: 5,
     searchType: "search",
   };
   ```

2. **Implement Search Action**

   ```typescript
   const TAVILY_SEARCH: SearchAction = {
     name: "TAVILY_SEARCH",
     description: "Search the web using Tavily API",
     // Refer to the documentation for proper examples of example array format, these should show the agent how the plugin is used, not be a single message that indicates what might trigger it.
     examples: [
       [
         {
           user: "user",
           content: { text: "Search for recent AI developments" },
         },
       ],
     ],
     // the similes array is just in case the model decides the action it's trying to perform is called something different today, which occasionally happens with small models. Mostly superfluous these days, but we still keep it around just in case.
     similes: ["tavilysearch", "tavily"],
     validate: async (runtime, message, state) => {
       try {
         validateSearchQuery(message.content);
         return true;
       } catch {
         return false;
       }
     },
     handler: async (runtime, message, state) => {
       // Implementation details below
     },
   };
   ```

3. **Implement API Integration**

   ```typescript
   async handler(runtime, message, state) {
     try {
       const query = validateSearchQuery(message.content);
       const response = await fetch('https://api.tavily.com/search', {
         method: 'POST',
         headers: {
           'Content-Type': 'application/json',
           'Authorization': `Bearer ${this.config.apiKey}`,
         },
         body: JSON.stringify({
           query,
           search_type: this.config.searchType,
           max_results: this.config.maxResults,
         }),
       });

       // Process results
       const data = await response.json();
       return {
         success: true,
         response: formatSearchResults(data.results),
       };
     } catch (error) {
       return handleApiError(error);
     }
   }
   ```

## Implementing Exa Search Plugin

1. **Create Plugin Configuration**

   ```typescript
   // It will be helpful here to examine the SearchPlugin that already exists in the @ai16z/eliza main repository to see what fields are required when defining a plugin interface, as well as how we extend them here.
   interface ExaPluginConfig extends SearchPluginConfig {
     searchType?: "semantic" | "code" | "document";
     filters?: {
       language?: string;
       fileType?: string;
     };
   }

   const DEFAULT_CONFIG: Partial<ExaPluginConfig> = {
     maxResults: 5,
     searchType: "semantic",
   };
   ```

2. **Implement Search Action**

   ```typescript
   const EXA_SEARCH: SearchAction = {
     name: "EXA_SEARCH",
     description:
       "Search using Exa API for semantic, code, and document search",
     // this is also a bad example of an examples array. I'll update this in the future to be more useful, but for now, refer to the official documenation for how the examples array ought to be formatted.
     examples: [
       [
         {
           user: "user",
           content: { text: "Find code examples for implementing OAuth" },
         },
       ],
     ],
     similes: ["exa", "exasearch"],
     validate: async (runtime, message, state) => {
       try {
         validateSearchQuery(message.content);
         return true;
       } catch {
         return false;
       }
     },
     handler: async (runtime, message, state) => {
       // Implementation details below
     },
   };
   ```

3. **Implement API Integration**

   ```typescript
   async handler(runtime, message, state) {
     try {
       const query = validateSearchQuery(message.content);
       const response = await fetch('https://api.exa.ai/search', {
         method: 'POST',
         headers: {
           'Content-Type': 'application/json',
           'Authorization': `Bearer ${this.config.apiKey}`,
         },
         body: JSON.stringify({
           query,
           search_type: this.config.searchType,
           max_results: this.config.maxResults,
           filters: this.config.filters,
         }),
       });

       // Process results
       const data = await response.json();
       return {
         success: true,
         response: formatSearchResults(data.results),
       };
     } catch (error) {
       return handleApiError(error);
     }
   }
   ```

## Implementing Calculator Plugin

The calculator plugin demonstrates a simple local computation plugin that doesn't require external APIs.

1. **Plugin Structure**

   ```typescript
   export const calculatorPlugin: Plugin = {
     name: "calculator",
     description: "Basic arithmetic calculator plugin",
     actions: [calculateAction],
     evaluators: [calculateEvaluator],
     providers: [],
   };
   ```

2. **Action Implementation**

   ```typescript
   export const calculateAction: Action = {
     name: 'CALCULATE',
     description: 'Performs basic arithmetic calculations',
     examples: [
       [
         {
           user: '{{user1}}',
           content: { text: '2 + 2' }
         },
         {
           user: '{{agentName}}',
           content: {
             text: 'The result of 2 + 2 is 4',
             action: 'CALCULATE'
           }
         }
       ]
     ],
     validate: async (runtime, message, state) => {
       const parts = message.content.text.split(/\s+/);
       return parts.length === 3 && !isNaN(parseFloat(parts[0])) && !isNaN(parseFloat(parts[2]));
     },
     handler: async (runtime, message, state) => {
       const parts = message.content.text.split(/\s+/);
       const [leftStr, operator, rightStr] = parts;
       const left = parseFloat(leftStr);
       const right = parseFloat(rightStr);

       let result: number;
       switch (operator) {
         case '+': result = left + right; break;
         case '-': result = left - right; break;
         case '*': result = left * right; break;
         case '/': 
           if (right === 0) throw new Error('Division by zero');
           result = left / right;
           break;
         default: throw new Error('Invalid operator');
       }

       return {
         success: true,
         response: `${message.content.text} = ${result}`
       };
     }
   };
   ```

## Implementing Weather Plugin

The weather plugin demonstrates integration with external APIs and provider patterns.

1. **Plugin Structure with Provider**

   ```typescript
   export const weatherPlugin: Plugin = {
     name: "weather",
     description: "Weather information plugin with OpenWeatherMap integration",
     actions: [getWeatherAction],
     evaluators: [weatherEvaluator],
     providers: [weatherProvider],
   };
   ```

2. **Weather Provider Implementation**

   ```typescript
   export const weatherProvider: Provider = {
     get: async (runtime, message, state) => {
       const content = message.content as { text: string };
       const locationMatch = content.text.match(/weather (?:in|at|for) (.+?)(?:\?|$)/i);
       
       if (!locationMatch) {
         throw new Error("Location not found in message");
       }
       
       const location = locationMatch[1].trim();
       const url = `${baseUrl}/weather?q=${encodeURIComponent(location)}&appid=${apiKey}&units=metric`;
       
       const response = await fetch(url);
       const data = await response.json();
       
       return {
         success: true,
         data: {
           location: data.name,
           temperature: data.main.temp,
           humidity: data.main.humidity,
           windSpeed: data.wind.speed,
           description: data.weather[0].description,
           units: 'metric'
         }
       };
     }
   };
   ```

## Implementing Translation Plugin

The translation plugin demonstrates service integration and configuration patterns.

1. **Plugin Structure with Service**

   ```typescript
   export const translationPlugin: Plugin = {
     name: "translation",
     description: "A plugin for translating text between different languages",
     actions: [getTranslationAction],
     evaluators: [translationEvaluator],
     providers: [translationProvider],
     services: [translationService],
   };
   ```

2. **Translation Service Implementation**

   ```typescript
   export const translationService: TranslationService = {
     serviceType: ServiceType.TEXT_GENERATION,
     
     translate: async (text: string, targetLang: string, sourceLang?: string) => {
       // Implementation includes API calls or local translation logic
       return {
         success: true,
         translation: translatedText,
         metadata: {
           model: 'translation-model',
           confidence: 0.95,
           tokensUsed: Math.ceil(text.length / 4)
         }
       };
     }
   };
   ```

## Testing Your Implementation

1. **Unit Tests**

   ```typescript
   describe("SearchPlugin", () => {
     it("should validate search queries", async () => {
       const plugin = new SearchPlugin(config);
       const result = await plugin.actions[0].validate(runtime, {
         content: { text: "test query" },
       });
       expect(result).toBe(true);
     });
   });
   ```

2. **Integration Testing**

   ```typescript
   describe("API Integration", () => {
     it("should return search results", async () => {
       const plugin = new SearchPlugin(config);
       const results = await plugin.actions[0].handler(runtime, {
         content: { text: "test query" },
       });
       expect(results.success).toBe(true);
       expect(Array.isArray(results.response)).toBe(true);
     });
   });
   ```

## Error Handling and Best Practices

1. **Rate Limiting**

   ```typescript
   const rateLimiter = createRateLimiter(60, 60000); // 60 requests per minute

   if (!rateLimiter.checkLimit()) {
     return {
       success: false,
       response: "Rate limit exceeded. Please try again later.",
     };
   }
   ```

2. **Error Handling**

   ```typescript
   try {
     // API calls
   } catch (error) {
     return handleApiError(error);
   }
   ```

3. **Input Validation**

   ```typescript
   function validateSearchQuery(content: Content): string {
     if (!content?.text) {
       throw new Error("Search query is required");
     }
     return content.text.trim();
   }
   ```

## Further Notes

This starter template demonstrates the core abstractions in the Eliza framework:

- **Actions**: Direct commands that agents can perform (like search, calculate, get weather)
- **Providers**: Data sources that supply information to actions (like weather data, translation services)
- **Evaluators**: Components that assess and validate the quality of responses
- **Services**: Background services that provide ongoing functionality

The included plugins showcase different patterns:

- **Simple Actions** (Calculator): Local computation without external dependencies
- **API Integration** (Tavily, Exa): External service integration with error handling and rate limiting
- **Provider Pattern** (Weather): Separating data fetching from action execution
- **Service Integration** (Translation): Background services with configuration management

For production use, consider:

1. **Error Handling**: Robust error handling for network failures, API limits, and invalid inputs
2. **Rate Limiting**: Implement rate limiting to respect API quotas
3. **Caching**: Cache responses to improve performance and reduce API costs
4. **Configuration**: Externalize configuration for different environments
5. **Testing**: Comprehensive unit and integration tests
6. **Security**: Secure API key management and input validation

Further development on this repo will illustrate adding plugins which add additional Providers and Evaluators, as well as the preferred way to add external services not directly implemented within the plugin code. I STRONGLY encourage you to review the documentation as well as the first few episodes of Agent Dev School on YouTube to guide you along your journey with Eliza.

## Usage Examples

### Running the Starter Template

```bash
# Build the project
pnpm build

# Run with all plugins enabled
pnpm mock-eliza --characters=./characters/eternalai.character.json
```

### Testing Individual Plugins

```bash
# Calculator
echo "What is 15 + 27?"

# Weather (requires API key)
echo "What's the weather in London?"

# Translation (mock implementation)
echo "Translate 'hello world' to Spanish"

# Search (requires API keys)
echo "Search for recent AI developments"
```

## Resources

- [Eliza Documentation](https://ai16z.github.io/eliza/)
- [Tavily API Documentation](https://tavily.com/docs)
- [Exa API Documentation](https://exa.ai/docs)
- [Example Implementations](https://github.com/ai16z/eliza/tree/main/packages)
- [AI Agent Dev School](https://www.youtube.com/watch?v=ArptLpQiKfI&list=PLx5pnFXdPTRzWla0RaOxALTSTnVq53fKL&ab_channel=ShawMakesMagic)
