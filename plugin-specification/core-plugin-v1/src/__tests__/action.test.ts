import { describe, it, expect } from 'vitest';
import { fromV2Action, toV2Action, Action } from '../action';
import { UUID } from '../types';

// Helper function to create test UUIDs
const createTestUUID = (num: number): UUID => {
  return `00000000-0000-0000-0000-${num.toString().padStart(12, '0')}`;
};

// Mock V2 Action interface for testing
interface ActionV2 {
  name: string;
  description: string;
  similes?: string[];
  examples?: Array<Array<{ name: string; content: any }>>;
  handler: any;
  validate: any;
}

describe('Action adapter', () => {
  const mockHandler = async () => ({ success: true });
  const mockValidator = async () => true;

  it('should convert from v2 action to v1 action correctly', () => {
    // Arrange
    const actionV2: ActionV2 = {
      name: 'test-action',
      description: 'A test action',
      similes: ['similar-action', 'another-action'],
      examples: [
        [
          {
            name: 'testuser',
            content: { text: 'test command' }
          }
        ]
      ],
      handler: mockHandler,
      validate: mockValidator,
    };

    // Act
    const actionV1 = fromV2Action(actionV2 as any);

    // Assert
    expect(actionV1.name).toBe('test-action');
    expect(actionV1.description).toBe('A test action');
    expect(actionV1.similes).toEqual(['similar-action', 'another-action']);
    expect(actionV1.examples).toHaveLength(1);
    expect(actionV1.examples[0][0].user).toBe('testuser'); // V1 uses 'user' instead of 'name'
    expect(actionV1.examples[0][0].content.text).toBe('test command');
    expect(actionV1.suppressInitialMessage).toBe(false); // Default value
    expect(typeof actionV1.handler).toBe('function');
    expect(typeof actionV1.validate).toBe('function');
  });

  it('should convert from v1 action to v2 action correctly', () => {
    // Arrange
    const actionV1: Action = {
      name: 'test-action',
      description: 'A test action',
      similes: ['similar-action'],
      examples: [
        [
          {
            user: 'testuser',
            content: { text: 'test command', action: 'test' }
          }
        ]
      ],
      handler: mockHandler,
      validate: mockValidator,
      suppressInitialMessage: true,
    };

    // Act
    const actionV2 = toV2Action(actionV1);

    // Assert
    expect(actionV2.name).toBe('test-action');
    expect(actionV2.description).toBe('A test action');
    expect(actionV2.similes).toEqual(['similar-action']);
    expect(actionV2.examples![0][0].name).toBe('testuser'); // V2 uses 'name'
    expect(actionV2.examples![0][0].content.text).toBe('test command');
    expect(actionV2.examples![0][0].content.actions).toEqual(['test']); // V2 converts action to actions array
    expect(typeof actionV2.handler).toBe('function');
    expect(typeof actionV2.validate).toBe('function');
  });

  it('should handle empty similes correctly', () => {
    // Arrange
    const actionV2WithoutSimiles: ActionV2 = {
      name: 'test-action',
      description: 'A test action',
      handler: mockHandler,
      validate: mockValidator,
    };

    // Act
    const actionV1 = fromV2Action(actionV2WithoutSimiles as any);

    // Assert
    expect(actionV1.similes).toEqual([]); // V1 requires array, should default to empty
  });

  it('should handle round-trip conversion', () => {
    // Arrange
    const originalAction: Action = {
      name: 'round-trip-action',
      description: 'Tests round-trip conversion',
      similes: ['test1', 'test2'],
      examples: [
        [
          {
            user: 'alice',
            content: { text: 'hello world', action: 'greet' }
          }
        ]
      ],
      handler: mockHandler,
      validate: mockValidator,
      suppressInitialMessage: false,
    };

    // Act - convert to v2 and back to v1
    const actionV2 = toV2Action(originalAction);
    const convertedBack = fromV2Action(actionV2 as any);

    // Assert - key properties should be preserved
    expect(convertedBack.name).toBe(originalAction.name);
    expect(convertedBack.description).toBe(originalAction.description);
    expect(convertedBack.similes).toEqual(originalAction.similes);
    expect(convertedBack.examples[0][0].user).toBe('alice');
    expect(convertedBack.examples[0][0].content.text).toBe('hello world');
    // Note: suppressInitialMessage is lost in v2 -> v1 conversion as v2 doesn't have this field
  });
});