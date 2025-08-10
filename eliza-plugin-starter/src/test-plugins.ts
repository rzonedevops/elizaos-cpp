#!/usr/bin/env node
/**
 * Simple test script to verify all plugins are properly exported and loadable
 */
import * as plugins from './index.js';

console.log('Testing plugin exports...\n');

const getErrorMessage = (error: unknown): string => {
  return error instanceof Error ? error.message : String(error);
};

// Test individual plugin imports
console.log('Testing individual plugin imports:');
try {
  const { calculatorPlugin } = await import('./plugins/calculator/index.js');
  console.log('✅ Calculator plugin loaded:', calculatorPlugin.name);
} catch (error) {
  console.error('❌ Failed to load calculator plugin:', getErrorMessage(error));
}

try {
  const { weatherPlugin } = await import('./plugins/weather/index.js');
  console.log('✅ Weather plugin loaded:', weatherPlugin.name);
} catch (error) {
  console.error('❌ Failed to load weather plugin:', getErrorMessage(error));
}

try {
  const { translationPlugin } = await import('./plugins/translation/index.js');
  console.log('✅ Translation plugin loaded:', translationPlugin.name);
} catch (error) {
  console.error('❌ Failed to load translation plugin:', getErrorMessage(error));
}

try {
  const tavilyPlugin = (await import('./plugins/tavily/index.js')).default;
  console.log('✅ Tavily plugin loaded:', tavilyPlugin.name);
} catch (error) {
  console.error('❌ Failed to load Tavily plugin:', getErrorMessage(error));
}

try {
  const exaPlugin = (await import('./plugins/exa/index.js')).default;
  console.log('✅ Exa plugin loaded:', exaPlugin.name);
} catch (error) {
  console.error('❌ Failed to load Exa plugin:', getErrorMessage(error));
}

console.log('\nTesting main index exports:');
console.log('Available exports:', Object.keys(plugins));

console.log('\n✅ Plugin test completed successfully!');