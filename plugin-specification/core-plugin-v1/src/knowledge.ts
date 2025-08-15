import {
  KnowledgeItem,
  RAGKnowledgeItem,
  Memory,
  UUID,
} from './types';

/**
 * Knowledge and memory management adapters for V1 compatibility
 * These provide conversion between V1 knowledge structures and V2 memory structures
 */

/**
 * Converts V1 KnowledgeItem to V2 Memory format
 * V1 KnowledgeItem has id and content, V2 Memory has more fields
 */
export function knowledgeItemToMemory(item: KnowledgeItem, agentId: UUID, roomId: UUID): Memory {
  return {
    id: item.id,
    entityId: agentId, // V2 uses entityId instead of userId
    agentId: agentId,
    content: item.content,
    roomId: roomId,
    createdAt: Date.now(),
    unique: false,
  };
}

/**
 * Converts V2 Memory to V1 KnowledgeItem format
 * Extracts the essential knowledge information from memory
 */
export function memoryToKnowledgeItem(memory: Memory): KnowledgeItem {
  return {
    id: memory.id!,
    content: memory.content,
  };
}

/**
 * Converts V1 RAGKnowledgeItem to V2 Memory format
 * RAG knowledge items have additional embedding and metadata
 */
export function ragKnowledgeToMemory(ragItem: RAGKnowledgeItem, roomId: UUID): Memory {
  return {
    id: ragItem.id,
    entityId: ragItem.agentId, // Map agentId to entityId
    agentId: ragItem.agentId,
    content: {
      text: ragItem.content.text,
      ...ragItem.content.metadata,
    },
    embedding: ragItem.embedding ? Array.from(ragItem.embedding) : undefined,
    roomId: roomId,
    createdAt: ragItem.createdAt || Date.now(),
    similarity: ragItem.similarity,
    unique: false,
  };
}

/**
 * Converts V2 Memory to V1 RAGKnowledgeItem format
 * Reconstructs RAG knowledge structure from memory
 */
export function memoryToRagKnowledge(memory: Memory): RAGKnowledgeItem {
  return {
    id: memory.id!,
    agentId: memory.agentId!,
    content: {
      text: memory.content.text || '',
      metadata: {
        // Extract metadata from content
        ...Object.keys(memory.content)
          .filter(key => key !== 'text')
          .reduce((meta, key) => ({
            ...meta,
            [key]: memory.content[key]
          }), {}),
        source: memory.content.source,
        type: 'knowledge',
      }
    },
    embedding: memory.embedding ? new Float32Array(memory.embedding) : undefined,
    createdAt: memory.createdAt,
    similarity: memory.similarity,
  };
}

/**
 * Memory table name mapping for different types of knowledge
 * V1 uses specific table names, V2 uses a more generic approach
 */
export const KNOWLEDGE_TABLE_NAMES = {
  DOCUMENTS: 'documents',
  KNOWLEDGE: 'knowledge',
  RAG_KNOWLEDGE: 'rag_knowledge',
  MEMORIES: 'memories',
  FRAGMENTS: 'fragments',
} as const;

/**
 * Utility functions for knowledge scope and metadata handling
 */

/**
 * Determines if a knowledge item should be shared based on V1 conventions
 */
export function isSharedKnowledge(item: KnowledgeItem | RAGKnowledgeItem): boolean {
  if ('content' in item && typeof item.content === 'object') {
    // Check RAGKnowledgeItem metadata
    if ('metadata' in item.content && item.content.metadata?.isShared === true) {
      return true;
    }
  }
  
  // Check general content for shared indicators
  if (item.content && typeof item.content === 'object' && 'shared' in item.content) {
    return item.content.shared === true;
  }
  
  return false;
}

/**
 * Creates appropriate metadata for knowledge items based on V1 patterns
 */
export function createKnowledgeMetadata(source?: string, isShared?: boolean) {
  return {
    type: 'knowledge',
    source: source || 'unknown',
    isShared: isShared || false,
    timestamp: Date.now(),
  };
}

/**
 * Filters knowledge items by scope (shared/private)
 */
export function filterKnowledgeByScope<T extends KnowledgeItem | RAGKnowledgeItem>(
  items: T[],
  includeShared: boolean = true,
  includePrivate: boolean = true
): T[] {
  return items.filter(item => {
    const isShared = isSharedKnowledge(item);
    return (includeShared && isShared) || (includePrivate && !isShared);
  });
}

/**
 * Converts knowledge search parameters from V1 to V2 format
 */
export function adaptKnowledgeSearchParams(params: {
  agentId: UUID;
  query?: string;
  limit?: number;
  embedding?: Float32Array;
  match_threshold?: number;
  match_count?: number;
  searchText?: string;
}) {
  return {
    entityId: params.agentId, // V2 uses entityId
    agentId: params.agentId,
    embedding: params.embedding ? Array.from(params.embedding) : [],
    match_threshold: params.match_threshold || 0.8,
    count: params.match_count || params.limit || 10,
    query: params.query || params.searchText,
    tableName: KNOWLEDGE_TABLE_NAMES.KNOWLEDGE,
  };
}

/**
 * Batch converts an array of KnowledgeItems to Memories
 */
export function knowledgeItemsToMemories(
  items: KnowledgeItem[],
  agentId: UUID,
  roomId: UUID
): Memory[] {
  return items.map(item => knowledgeItemToMemory(item, agentId, roomId));
}

/**
 * Batch converts an array of Memories to KnowledgeItems
 */
export function memoriesToKnowledgeItems(memories: Memory[]): KnowledgeItem[] {
  return memories.map(memory => memoryToKnowledgeItem(memory));
}

/**
 * Batch converts an array of RAGKnowledgeItems to Memories
 */
export function ragKnowledgeItemsToMemories(
  items: RAGKnowledgeItem[],
  roomId: UUID
): Memory[] {
  return items.map(item => ragKnowledgeToMemory(item, roomId));
}

/**
 * Batch converts an array of Memories to RAGKnowledgeItems
 */
export function memoriesToRagKnowledgeItems(memories: Memory[]): RAGKnowledgeItem[] {
  return memories.map(memory => memoryToRagKnowledge(memory));
}