import {
  IDatabaseAdapter as IDatabaseAdapterFromTypes,
  Account,
  Memory,
  UUID,
  Actor,
  Goal,
  GoalStatus,
  Relationship,
  Participant,
  RAGKnowledgeItem,
} from './types';
import {
  IDatabaseAdapter as IDatabaseAdapterV2,
  Entity,
} from '@elizaos/core-plugin-v2';

/**
 * Database adapter interface for v1 compatibility
 */
export type IDatabaseAdapter = IDatabaseAdapterFromTypes;

/**
 * Converts V2 Entity to V1 Account
 * V2 uses Entity with names array, V1 uses Account with single name/username
 */
function fromV2Entity(entity: Entity): Account {
  return {
    id: entity.id!,
    name: entity.names[0] || 'Unknown',
    username: entity.names[1] || entity.names[0] || 'unknown',
    details: entity.metadata || {},
    email: entity.metadata?.email as string,
    avatarUrl: entity.metadata?.avatarUrl as string,
  };
}

/**
 * Converts V1 Account to V2 Entity
 * V1 uses Account with single name/username, V2 uses Entity with names array
 */
function toV2Entity(account: Account): Entity {
  return {
    id: account.id,
    names: [account.name, account.username].filter(Boolean),
    metadata: {
      ...account.details,
      email: account.email,
      avatarUrl: account.avatarUrl,
    },
    agentId: account.id, // Use account ID as default agent ID
  };
}

/**
 * Creates a wrapper that converts V2 IDatabaseAdapter to V1 compatible interface
 * This handles the major API differences between v1 and v2
 */
export function fromV2DatabaseAdapter(adapterV2: IDatabaseAdapterV2): IDatabaseAdapter {
  return {
    db: adapterV2.db,

    init: () => adapterV2.init(),
    close: () => adapterV2.close(),

    // V1 Account methods mapped to V2 Entity methods
    getAccountById: async (userId: UUID): Promise<Account | null> => {
      try {
        const entities = await adapterV2.getEntityByIds([userId]);
        if (entities && entities.length > 0) {
          return fromV2Entity(entities[0]);
        }
        return null;
      } catch (error) {
        console.error('Error getting account by ID:', error);
        return null;
      }
    },

    createAccount: async (account: Account): Promise<boolean> => {
      try {
        const entity = toV2Entity(account);
        return await adapterV2.createEntities([entity]);
      } catch (error) {
        console.error('Error creating account:', error);
        return false;
      }
    },

    // Memory methods - map V1 params to V2 structure
    getMemories: async (params) => {
      return adapterV2.getMemories({
        entityId: params.agentId, // V2 uses entityId instead of agentId
        agentId: params.agentId,
        roomId: params.roomId,
        count: params.count,
        unique: params.unique,
        tableName: params.tableName,
        start: params.start,
        end: params.end,
      });
    },

    getMemoryById: (id: UUID) => adapterV2.getMemoryById(id),
    
    getMemoriesByIds: (ids: UUID[], tableName?: string) => 
      adapterV2.getMemoriesByIds(ids, tableName),

    getMemoriesByRoomIds: (params) => 
      adapterV2.getMemoriesByRoomIds(params),

    getCachedEmbeddings: (params) => 
      adapterV2.getCachedEmbeddings(params),

    log: async (params) => {
      // V2 uses entityId, V1 uses userId
      return adapterV2.log({
        body: params.body,
        entityId: params.userId,
        roomId: params.roomId,
        type: params.type,
      });
    },

    // Actor details - V2 doesn't have this exact method, implement basic version
    getActorDetails: async (params): Promise<Actor[]> => {
      try {
        const entities = await adapterV2.getEntitiesForRoom(params.roomId);
        return entities.map(entity => ({
          id: entity.id!,
          name: entity.names[0] || 'Unknown',
          username: entity.names[1] || entity.names[0] || 'unknown',
          details: {
            tagline: entity.metadata?.tagline as string || '',
            summary: entity.metadata?.summary as string || '',
            quote: entity.metadata?.quote as string || '',
          }
        }));
      } catch (error) {
        console.error('Error getting actor details:', error);
        return [];
      }
    },

    searchMemories: (params) => {
      return adapterV2.searchMemories({
        embedding: params.embedding,
        match_threshold: params.match_threshold,
        count: params.match_count,
        unique: params.unique,
        tableName: params.tableName,
        roomId: params.roomId,
        entityId: params.agentId, // Map agentId to entityId
      });
    },

    // Goal methods - V2 doesn't have goals, implement stub methods
    updateGoalStatus: async (params): Promise<void> => {
      console.warn('updateGoalStatus not implemented in V2 adapter');
    },

    searchMemoriesByEmbedding: (embedding: number[], params) => {
      return adapterV2.searchMemories({
        embedding,
        match_threshold: params.match_threshold,
        count: params.count,
        roomId: params.roomId,
        entityId: params.agentId,
        unique: params.unique,
        tableName: params.tableName,
      });
    },

    createMemory: async (memory: Memory, tableName: string, unique?: boolean) => {
      await adapterV2.createMemory(memory, tableName, unique);
    },

    removeMemory: (memoryId: UUID, tableName: string) => {
      // V2 uses deleteMemory instead of removeMemory
      return adapterV2.deleteMemory(memoryId);
    },

    removeAllMemories: (roomId: UUID, tableName: string) => {
      return adapterV2.deleteAllMemories(roomId, tableName);
    },

    countMemories: (roomId: UUID, unique?: boolean, tableName?: string) => 
      adapterV2.countMemories(roomId, unique, tableName),

    // Goal methods - not implemented in V2, return empty/stub implementations
    getGoals: async (): Promise<Goal[]> => {
      console.warn('getGoals not implemented in V2 adapter');
      return [];
    },

    updateGoal: async (goal: Goal): Promise<void> => {
      console.warn('updateGoal not implemented in V2 adapter');
    },

    createGoal: async (goal: Goal): Promise<void> => {
      console.warn('createGoal not implemented in V2 adapter');
    },

    removeGoal: async (goalId: UUID): Promise<void> => {
      console.warn('removeGoal not implemented in V2 adapter');
    },

    removeAllGoals: async (roomId: UUID): Promise<void> => {
      console.warn('removeAllGoals not implemented in V2 adapter');
    },

    // Room methods
    getRoom: async (roomId: UUID) => {
      const rooms = await adapterV2.getRoomsByIds([roomId]);
      return rooms && rooms.length > 0 ? roomId : null;
    },

    createRoom: async (roomId?: UUID): Promise<UUID> => {
      // V2 createRooms expects array and returns array
      const room = {
        id: roomId,
        source: 'v1-adapter',
        type: 'GROUP' as any,
      };
      const roomIds = await adapterV2.createRooms([room as any]);
      return roomIds[0];
    },

    removeRoom: (roomId: UUID) => adapterV2.deleteRoom(roomId),

    getRoomsForParticipant: (userId: UUID) => 
      adapterV2.getRoomsForParticipant(userId),

    getRoomsForParticipants: (userIds: UUID[]) => 
      adapterV2.getRoomsForParticipants(userIds),

    addParticipant: (userId: UUID, roomId: UUID) => 
      adapterV2.addParticipantsRoom([userId], roomId),

    removeParticipant: (userId: UUID, roomId: UUID) => 
      adapterV2.removeParticipant(userId, roomId),

    // Participant methods
    getParticipantsForAccount: async (userId: UUID): Promise<Participant[]> => {
      const participants = await adapterV2.getParticipantsForEntity(userId);
      return participants.map(p => ({
        id: p.id,
        account: fromV2Entity(p.entity),
      }));
    },

    getParticipantsForRoom: (roomId: UUID) => 
      adapterV2.getParticipantsForRoom(roomId),

    getParticipantUserState: (roomId: UUID, userId: UUID) => 
      adapterV2.getParticipantUserState(roomId, userId),

    setParticipantUserState: (roomId: UUID, userId: UUID, state) => 
      adapterV2.setParticipantUserState(roomId, userId, state),

    // Relationship methods - V2 has different signature
    createRelationship: async (params): Promise<boolean> => {
      return adapterV2.createRelationship({
        sourceEntityId: params.userA,
        targetEntityId: params.userB,
      });
    },

    getRelationship: async (params): Promise<Relationship | null> => {
      const relationship = await adapterV2.getRelationship({
        sourceEntityId: params.userA,
        targetEntityId: params.userB,
      });
      
      if (!relationship) return null;

      // Convert V2 relationship to V1 format
      return {
        id: relationship.id,
        userA: relationship.sourceEntityId,
        userB: relationship.targetEntityId,
        userId: relationship.sourceEntityId, // Use source as primary user
        roomId: relationship.id, // V1 expects roomId, use relationship ID
        status: relationship.tags.join(','), // Convert tags to status string
        createdAt: relationship.createdAt,
      };
    },

    getRelationships: async (params): Promise<Relationship[]> => {
      const relationships = await adapterV2.getRelationships({
        entityId: params.userId,
      });

      return relationships.map(rel => ({
        id: rel.id,
        userA: rel.sourceEntityId,
        userB: rel.targetEntityId,
        userId: params.userId,
        roomId: rel.id, // Use relationship ID as roomId
        status: rel.tags.join(','),
        createdAt: rel.createdAt,
      }));
    },

    // Knowledge methods - V2 doesn't have exact equivalent, implement stubs
    getKnowledge: async (): Promise<RAGKnowledgeItem[]> => {
      console.warn('getKnowledge not fully implemented in V2 adapter');
      return [];
    },

    searchKnowledge: async (): Promise<RAGKnowledgeItem[]> => {
      console.warn('searchKnowledge not fully implemented in V2 adapter');
      return [];
    },

    createKnowledge: async (knowledge: RAGKnowledgeItem): Promise<void> => {
      console.warn('createKnowledge not fully implemented in V2 adapter');
    },

    removeKnowledge: async (id: UUID): Promise<void> => {
      console.warn('removeKnowledge not fully implemented in V2 adapter');
    },

    clearKnowledge: async (agentId: UUID, shared?: boolean): Promise<void> => {
      console.warn('clearKnowledge not fully implemented in V2 adapter');
    },
  };
}

/**
 * Creates a wrapper that converts V1 IDatabaseAdapter to V2 compatible interface
 * This is a more complex conversion due to the significant API differences
 */
export function toV2DatabaseAdapter(adapterV1: IDatabaseAdapter): IDatabaseAdapterV2 {
  // This is a complex conversion that would require extensive implementation
  // For now, we'll provide a basic stub that logs warnings for unimplemented methods
  throw new Error('toV2DatabaseAdapter not yet fully implemented - V2 interface is significantly different');
}