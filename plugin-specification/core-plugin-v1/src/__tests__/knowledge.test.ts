import { describe, it, expect } from 'vitest';
import {
  knowledgeItemToMemory,
  memoryToKnowledgeItem,
  ragKnowledgeToMemory,
  memoryToRagKnowledge,
  isSharedKnowledge,
  createKnowledgeMetadata,
  filterKnowledgeByScope,
  KNOWLEDGE_TABLE_NAMES,
} from '../knowledge';
import { UUID, KnowledgeItem, RAGKnowledgeItem, Memory } from '../types';

// Helper function to create test UUIDs
const createTestUUID = (num: number): UUID => {
  return `00000000-0000-0000-0000-${num.toString().padStart(12, '0')}`;
};

describe('Knowledge adapter', () => {
  const testAgentId = createTestUUID(1);
  const testRoomId = createTestUUID(2);

  describe('KnowledgeItem to Memory conversion', () => {
    it('should convert KnowledgeItem to Memory correctly', () => {
      // Arrange
      const knowledgeItem: KnowledgeItem = {
        id: createTestUUID(100),
        content: {
          text: 'This is knowledge about AI',
          source: 'documentation',
        },
      };

      // Act
      const memory = knowledgeItemToMemory(knowledgeItem, testAgentId, testRoomId);

      // Assert
      expect(memory.id).toBe(knowledgeItem.id);
      expect(memory.entityId).toBe(testAgentId); // V2 uses entityId
      expect(memory.agentId).toBe(testAgentId);
      expect(memory.content).toEqual(knowledgeItem.content);
      expect(memory.roomId).toBe(testRoomId);
      expect(memory.unique).toBe(false);
      expect(memory.createdAt).toBeDefined();
    });

    it('should convert Memory to KnowledgeItem correctly', () => {
      // Arrange
      const memory: Memory = {
        id: createTestUUID(101),
        entityId: testAgentId,
        agentId: testAgentId,
        content: {
          text: 'Memory content',
          action: 'remember',
        },
        roomId: testRoomId,
        createdAt: Date.now(),
      };

      // Act
      const knowledgeItem = memoryToKnowledgeItem(memory);

      // Assert
      expect(knowledgeItem.id).toBe(memory.id);
      expect(knowledgeItem.content).toEqual(memory.content);
    });
  });

  describe('RAGKnowledgeItem to Memory conversion', () => {
    it('should convert RAGKnowledgeItem to Memory correctly', () => {
      // Arrange
      const ragKnowledge: RAGKnowledgeItem = {
        id: createTestUUID(200),
        agentId: testAgentId,
        content: {
          text: 'RAG knowledge content',
          metadata: {
            source: 'pdf',
            type: 'document',
            isShared: true,
          },
        },
        embedding: new Float32Array([0.1, 0.2, 0.3]),
        createdAt: 1234567890,
        similarity: 0.95,
      };

      // Act
      const memory = ragKnowledgeToMemory(ragKnowledge, testRoomId);

      // Assert
      expect(memory.id).toBe(ragKnowledge.id);
      expect(memory.entityId).toBe(testAgentId);
      expect(memory.agentId).toBe(testAgentId);
      expect(memory.content.text).toBe('RAG knowledge content');
      expect(memory.content.source).toBe('pdf');
      expect(memory.content.type).toBe('document');
      expect(memory.content.isShared).toBe(true);
      expect(memory.embedding).toEqual([0.1, 0.2, 0.3]);
      expect(memory.roomId).toBe(testRoomId);
      expect(memory.createdAt).toBe(1234567890);
      expect(memory.similarity).toBe(0.95);
    });

    it('should convert Memory to RAGKnowledgeItem correctly', () => {
      // Arrange
      const memory: Memory = {
        id: createTestUUID(201),
        entityId: testAgentId,
        agentId: testAgentId,
        content: {
          text: 'Memory text content',
          source: 'chat',
          category: 'conversation',
        },
        embedding: [0.4, 0.5, 0.6],
        roomId: testRoomId,
        createdAt: 9876543210,
        similarity: 0.88,
      };

      // Act
      const ragKnowledge = memoryToRagKnowledge(memory);

      // Assert
      expect(ragKnowledge.id).toBe(memory.id);
      expect(ragKnowledge.agentId).toBe(testAgentId);
      expect(ragKnowledge.content.text).toBe('Memory text content');
      expect(ragKnowledge.content.metadata!.source).toBe('chat');
      expect(ragKnowledge.content.metadata!.category).toBe('conversation');
      expect(ragKnowledge.content.metadata!.type).toBe('knowledge');
      expect(Array.from(ragKnowledge.embedding!)).toEqual([0.4, 0.5, 0.6]);
      expect(ragKnowledge.createdAt).toBe(9876543210);
      expect(ragKnowledge.similarity).toBe(0.88);
    });
  });

  describe('Knowledge scope utilities', () => {
    it('should identify shared knowledge correctly', () => {
      // Arrange
      const sharedKnowledge: RAGKnowledgeItem = {
        id: createTestUUID(300),
        agentId: testAgentId,
        content: {
          text: 'Shared knowledge',
          metadata: {
            isShared: true,
          },
        },
      };

      const privateKnowledge: KnowledgeItem = {
        id: createTestUUID(301),
        content: {
          text: 'Private knowledge',
          shared: false,
        },
      };

      // Act & Assert
      expect(isSharedKnowledge(sharedKnowledge)).toBe(true);
      expect(isSharedKnowledge(privateKnowledge)).toBe(false);
    });

    it('should create knowledge metadata correctly', () => {
      // Act
      const metadata = createKnowledgeMetadata('test-source', true);

      // Assert
      expect(metadata.type).toBe('knowledge');
      expect(metadata.source).toBe('test-source');
      expect(metadata.isShared).toBe(true);
      expect(metadata.timestamp).toBeDefined();
    });

    it('should filter knowledge by scope correctly', () => {
      // Arrange
      const items: RAGKnowledgeItem[] = [
        {
          id: createTestUUID(400),
          agentId: testAgentId,
          content: {
            text: 'Shared item',
            metadata: { isShared: true },
          },
        },
        {
          id: createTestUUID(401),
          agentId: testAgentId,
          content: {
            text: 'Private item',
            metadata: { isShared: false },
          },
        },
      ];

      // Act
      const sharedOnly = filterKnowledgeByScope(items, true, false);
      const privateOnly = filterKnowledgeByScope(items, false, true);
      const all = filterKnowledgeByScope(items, true, true);

      // Assert
      expect(sharedOnly).toHaveLength(1);
      expect(sharedOnly[0].content.text).toBe('Shared item');
      expect(privateOnly).toHaveLength(1);
      expect(privateOnly[0].content.text).toBe('Private item');
      expect(all).toHaveLength(2);
    });
  });

  describe('Constants and table names', () => {
    it('should define knowledge table names', () => {
      expect(KNOWLEDGE_TABLE_NAMES.DOCUMENTS).toBe('documents');
      expect(KNOWLEDGE_TABLE_NAMES.KNOWLEDGE).toBe('knowledge');
      expect(KNOWLEDGE_TABLE_NAMES.RAG_KNOWLEDGE).toBe('rag_knowledge');
      expect(KNOWLEDGE_TABLE_NAMES.MEMORIES).toBe('memories');
      expect(KNOWLEDGE_TABLE_NAMES.FRAGMENTS).toBe('fragments');
    });
  });
});