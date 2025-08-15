import {
  Relationship as RelationshipFromTypes,
  UUID,
} from './types';
import {
  Relationship as RelationshipV2,
} from '@elizaos/core-plugin-v2';

/**
 * Relationship interface for v1 compatibility
 */
export type Relationship = RelationshipFromTypes;

/**
 * Converts V2 Relationship to V1 compatible Relationship
 * Main differences:
 * - V2 uses sourceEntityId/targetEntityId, V1 uses userA/userB
 * - V2 has tags array and metadata object, V1 has status string
 * - V1 has userId and roomId fields that don't exist in V2
 */
export function fromV2Relationship(relationshipV2: RelationshipV2): Relationship {
  return {
    id: relationshipV2.id,
    userA: relationshipV2.sourceEntityId,
    userB: relationshipV2.targetEntityId,
    userId: relationshipV2.sourceEntityId, // Use source as primary user
    roomId: relationshipV2.id, // V1 expects roomId, use relationship ID as fallback
    status: relationshipV2.tags.join(','), // Convert tags array to comma-separated string
    createdAt: relationshipV2.createdAt,
  };
}

/**
 * Converts V1 Relationship to V2 compatible Relationship
 * Maps V1 relationship structure to V2 format
 */
export function toV2Relationship(relationship: Relationship, agentId: UUID): RelationshipV2 {
  return {
    id: relationship.id,
    sourceEntityId: relationship.userA,
    targetEntityId: relationship.userB,
    agentId: agentId,
    tags: relationship.status ? relationship.status.split(',').map(s => s.trim()) : [],
    metadata: {
      // Preserve any V1-specific fields in metadata
      userId: relationship.userId,
      roomId: relationship.roomId,
    },
    createdAt: relationship.createdAt,
  };
}

/**
 * Converts an array of V2 Relationships to V1 format
 */
export function fromV2Relationships(relationshipsV2: RelationshipV2[]): Relationship[] {
  return relationshipsV2.map(rel => fromV2Relationship(rel));
}

/**
 * Converts an array of V1 Relationships to V2 format
 */
export function toV2Relationships(relationships: Relationship[], agentId: UUID): RelationshipV2[] {
  return relationships.map(rel => toV2Relationship(rel, agentId));
}

/**
 * Relationship status constants commonly used in V1
 * These can be used to standardize relationship statuses
 */
export const RELATIONSHIP_STATUSES = {
  FRIEND: 'friend',
  BLOCKED: 'blocked',
  MUTED: 'muted',
  FOLLOWING: 'following',
  FOLLOWED_BY: 'followed_by',
  ACQUAINTANCE: 'acquaintance',
  UNKNOWN: 'unknown',
} as const;

/**
 * Converts V2 tags to V1 status string with common mappings
 * Provides more intelligent conversion than simple join
 */
export function tagsToStatus(tags: string[]): string {
  if (tags.length === 0) return RELATIONSHIP_STATUSES.UNKNOWN;
  
  // Map common V2 tags to V1 statuses
  const statusMap: Record<string, string> = {
    'friend': RELATIONSHIP_STATUSES.FRIEND,
    'blocked': RELATIONSHIP_STATUSES.BLOCKED,
    'muted': RELATIONSHIP_STATUSES.MUTED,
    'following': RELATIONSHIP_STATUSES.FOLLOWING,
    'follower': RELATIONSHIP_STATUSES.FOLLOWED_BY,
    'acquaintance': RELATIONSHIP_STATUSES.ACQUAINTANCE,
  };

  // Check for known status mappings first
  for (const tag of tags) {
    const normalizedTag = tag.toLowerCase();
    if (statusMap[normalizedTag]) {
      return statusMap[normalizedTag];
    }
  }

  // If no known mapping, join all tags
  return tags.join(',');
}

/**
 * Converts V1 status string to V2 tags array with common mappings
 * Provides more intelligent conversion than simple split
 */
export function statusToTags(status: string): string[] {
  if (!status) return [];
  
  // Map common V1 statuses to V2 tags
  const tagMap: Record<string, string[]> = {
    [RELATIONSHIP_STATUSES.FRIEND]: ['friend'],
    [RELATIONSHIP_STATUSES.BLOCKED]: ['blocked'],
    [RELATIONSHIP_STATUSES.MUTED]: ['muted'],
    [RELATIONSHIP_STATUSES.FOLLOWING]: ['following'],
    [RELATIONSHIP_STATUSES.FOLLOWED_BY]: ['follower'],
    [RELATIONSHIP_STATUSES.ACQUAINTANCE]: ['acquaintance'],
    [RELATIONSHIP_STATUSES.UNKNOWN]: [],
  };

  const normalizedStatus = status.toLowerCase();
  if (tagMap[normalizedStatus]) {
    return tagMap[normalizedStatus];
  }

  // If no known mapping, split by comma
  return status.split(',').map(s => s.trim()).filter(Boolean);
}

/**
 * Enhanced conversion with intelligent status mapping
 */
export function fromV2RelationshipEnhanced(relationshipV2: RelationshipV2): Relationship {
  return {
    id: relationshipV2.id,
    userA: relationshipV2.sourceEntityId,
    userB: relationshipV2.targetEntityId,
    userId: relationshipV2.sourceEntityId,
    roomId: relationshipV2.metadata?.roomId as UUID || relationshipV2.id,
    status: tagsToStatus(relationshipV2.tags),
    createdAt: relationshipV2.createdAt,
  };
}

/**
 * Enhanced conversion with intelligent tag mapping
 */
export function toV2RelationshipEnhanced(relationship: Relationship, agentId: UUID): RelationshipV2 {
  return {
    id: relationship.id,
    sourceEntityId: relationship.userA,
    targetEntityId: relationship.userB,
    agentId: agentId,
    tags: statusToTags(relationship.status),
    metadata: {
      userId: relationship.userId,
      roomId: relationship.roomId,
      // Preserve original status for reference
      originalStatus: relationship.status,
    },
    createdAt: relationship.createdAt,
  };
}

/**
 * Utility functions for relationship management
 */

/**
 * Creates a new V1 relationship with default values
 */
export function createV1Relationship(
  userA: UUID,
  userB: UUID,
  status: string = RELATIONSHIP_STATUSES.UNKNOWN,
  roomId?: UUID
): Relationship {
  return {
    id: `${userA}-${userB}-${Date.now()}` as UUID,
    userA,
    userB,
    userId: userA,
    roomId: roomId || userA, // Use userA as fallback roomId
    status,
    createdAt: new Date().toISOString(),
  };
}

/**
 * Checks if two relationships represent the same connection (bidirectional)
 */
export function areRelationshipsEquivalent(rel1: Relationship, rel2: Relationship): boolean {
  return (
    (rel1.userA === rel2.userA && rel1.userB === rel2.userB) ||
    (rel1.userA === rel2.userB && rel1.userB === rel2.userA)
  );
}

/**
 * Filters relationships by status
 */
export function filterRelationshipsByStatus(relationships: Relationship[], status: string): Relationship[] {
  return relationships.filter(rel => rel.status === status);
}

/**
 * Gets all relationships for a specific user (where user is either userA or userB)
 */
export function getRelationshipsForUser(relationships: Relationship[], userId: UUID): Relationship[] {
  return relationships.filter(rel => rel.userA === userId || rel.userB === userId);
}