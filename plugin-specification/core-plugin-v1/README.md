# ElizaOS Plugin Specification - Core Plugin V1

This package provides a compatibility layer that allows v1 plugins to work with the v2 plugin system. It includes adapters and utilities to convert between v1 and v2 interfaces seamlessly.

## Overview

The core-plugin-v1 package serves as a bridge between the legacy v1 plugin system and the modern v2 architecture. It provides:

- **Bidirectional adapters** for converting between v1 and v2 data structures
- **Type compatibility** ensuring v1 plugins can integrate with v2 runtime
- **Comprehensive test coverage** to ensure reliability of conversions
- **Utility functions** for common plugin operations

## Implemented Adapters

### 1. State Adapters (`state.ts`)
- `fromV2State()` - Converts v2 State to v1 compatible State
- `toV2State()` - Converts v1 State to v2 State

### 2. Action Example Adapters (`actionExample.ts`)
- `fromV2ActionExample()` - Converts v2 ActionExample to v1 format
- `toV2ActionExample()` - Converts v1 ActionExample to v2 format
- `convertContentToV1()` / `convertContentToV2()` - Content conversion utilities

### 3. Provider Adapters (`provider.ts`)
- `fromV2Provider()` - Converts v2 Provider to v1 compatible Provider
- `toV2Provider()` - Converts v1 Provider to v2 Provider

### 4. Template Utilities (`templates.ts`)
- Template processing functions for both v1 and v2 formats

### 5. Action/Handler Adapters (`action.ts`) ✨ NEW
- `fromV2Action()` - Converts v2 Action to v1 compatible Action
- `toV2Action()` - Converts v1 Action to v2 Action
- Handler and Validator function adapters with proper signature mapping

### 6. Database Adapters (`database.ts`) ✨ NEW
- `fromV2DatabaseAdapter()` - Wraps v2 IDatabaseAdapter for v1 compatibility
- Handles major API differences between v1 and v2 database interfaces
- Maps Entity ↔ Account, adjusts method signatures, provides compatibility shims

### 7. Knowledge/Memory Adapters (`knowledge.ts`) ✨ NEW
- `knowledgeItemToMemory()` / `memoryToKnowledgeItem()` - Basic knowledge conversion
- `ragKnowledgeToMemory()` / `memoryToRagKnowledge()` - RAG knowledge conversion
- Batch conversion utilities for arrays
- Knowledge scope and metadata management utilities

### 8. Relationship Adapters (`relationship.ts`) ✨ NEW
- `fromV2Relationship()` / `toV2Relationship()` - Basic relationship conversion
- Enhanced conversion with intelligent status ↔ tags mapping
- Utility functions for relationship management
- Support for common relationship statuses (friend, blocked, muted, etc.)

## Key Differences Handled

### V1 → V2 Conversions
- **Account → Entity**: V1's single name/username → V2's names array
- **Action string → Actions array**: V1's single action → V2's actions array  
- **Status string → Tags array**: V1's relationship status → V2's tags system
- **User/Agent IDs → Entity IDs**: V1's separate user/agent concepts → V2's unified entities
- **Required → Optional fields**: V1's required similes → V2's optional similes

### V2 → V1 Conversions
- **Entity → Account**: V2's names array → V1's name/username
- **Actions array → Action string**: V2's actions array → V1's single action (first item)
- **Tags array → Status string**: V2's tags → V1's comma-separated status string
- **Entity IDs → User/Agent IDs**: V2's entities → V1's separate user/agent concepts
- **Optional → Required fields**: V2's optional fields → V1's required with defaults

## Usage Examples

### Converting a V1 Action to V2
```typescript
import { toV2Action } from '@elizaos/core-plugin-v1';

const v1Action = {
  name: 'greet',
  description: 'Greets the user',
  similes: ['hello', 'hi'],
  examples: [[{ user: 'alice', content: { text: 'hello', action: 'greet' } }]],
  handler: async (runtime, message, state) => { /* ... */ },
  validate: async (runtime, message, state) => true,
};

const v2Action = toV2Action(v1Action);
// v2Action.examples[0][0].name === 'alice'
// v2Action.examples[0][0].content.actions === ['greet']
```

### Using Database Adapter
```typescript
import { fromV2DatabaseAdapter } from '@elizaos/core-plugin-v1';

// Wrap a v2 database adapter for v1 compatibility
const v2Adapter = new MyV2DatabaseAdapter();
const v1Adapter = fromV2DatabaseAdapter(v2Adapter);

// Now use v1 methods
const account = await v1Adapter.getAccountById(userId);
await v1Adapter.createAccount(newAccount);
```

### Converting Knowledge Items
```typescript
import { knowledgeItemToMemory, memoryToKnowledgeItem } from '@elizaos/core-plugin-v1';

// Convert v1 knowledge to v2 memory
const memory = knowledgeItemToMemory(knowledgeItem, agentId, roomId);

// Convert v2 memory back to v1 knowledge
const knowledge = memoryToKnowledgeItem(memory);
```

## Testing

The adapters include comprehensive test suites covering:
- ✅ Basic conversion functionality
- ✅ Edge cases and error handling  
- ✅ Round-trip conversion preservation
- ✅ Real-world usage scenarios
- ✅ Type safety and compatibility

Run tests with:
```bash
npm test
# or
vitest
```

## Dependencies

- `@elizaos/core-plugin-v2` - The v2 plugin system this package adapts to
- Core TypeScript types and interfaces

## Architecture Notes

This package follows the adapter pattern to provide a clean compatibility layer. Each adapter:

1. **Preserves semantics** - Maintains the intended behavior of v1 interfaces
2. **Handles missing fields** - Provides sensible defaults for fields that don't exist in one version
3. **Maps structural differences** - Converts between different data structures (arrays ↔ strings, etc.)
4. **Includes error handling** - Gracefully handles conversion failures
5. **Maintains performance** - Lightweight conversions with minimal overhead

The goal is to make v1 plugins work seamlessly with v2 infrastructure without requiring plugin authors to rewrite their code.