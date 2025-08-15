import { describe, it, expect } from 'vitest';
import {
  fromV2Relationship,
  toV2Relationship,
  fromV2RelationshipEnhanced,
  toV2RelationshipEnhanced,
  tagsToStatus,
  statusToTags,
  createV1Relationship,
  areRelationshipsEquivalent,
  RELATIONSHIP_STATUSES,
} from '../relationship';
import { UUID, Relationship } from '../types';

// Helper function to create test UUIDs
const createTestUUID = (num: number): UUID => {
  return `00000000-0000-0000-0000-${num.toString().padStart(12, '0')}`;
};

// Mock V2 Relationship interface for testing
interface RelationshipV2 {
  id: UUID;
  sourceEntityId: UUID;
  targetEntityId: UUID;
  agentId: UUID;
  tags: string[];
  metadata: { [key: string]: any };
  createdAt?: string;
}

describe('Relationship adapter', () => {
  const testAgentId = createTestUUID(1);
  const testUserA = createTestUUID(2);
  const testUserB = createTestUUID(3);
  const testRelId = createTestUUID(4);

  describe('Basic conversion', () => {
    it('should convert from v2 relationship to v1 relationship correctly', () => {
      // Arrange
      const relationshipV2: RelationshipV2 = {
        id: testRelId,
        sourceEntityId: testUserA,
        targetEntityId: testUserB,
        agentId: testAgentId,
        tags: ['friend', 'colleague'],
        metadata: {
          strength: 0.8,
          lastContact: '2023-01-01',
        },
        createdAt: '2023-01-01T00:00:00Z',
      };

      // Act
      const relationshipV1 = fromV2Relationship(relationshipV2);

      // Assert
      expect(relationshipV1.id).toBe(testRelId);
      expect(relationshipV1.userA).toBe(testUserA);
      expect(relationshipV1.userB).toBe(testUserB);
      expect(relationshipV1.userId).toBe(testUserA); // Uses source as primary user
      expect(relationshipV1.roomId).toBe(testRelId); // Uses relationship ID as fallback
      expect(relationshipV1.status).toBe('friend,colleague'); // Tags joined by comma
      expect(relationshipV1.createdAt).toBe('2023-01-01T00:00:00Z');
    });

    it('should convert from v1 relationship to v2 relationship correctly', () => {
      // Arrange
      const relationshipV1: Relationship = {
        id: testRelId,
        userA: testUserA,
        userB: testUserB,
        userId: testUserA,
        roomId: createTestUUID(5),
        status: 'friend,blocked',
        createdAt: '2023-01-01T00:00:00Z',
      };

      // Act
      const relationshipV2 = toV2Relationship(relationshipV1, testAgentId);

      // Assert
      expect(relationshipV2.id).toBe(testRelId);
      expect(relationshipV2.sourceEntityId).toBe(testUserA);
      expect(relationshipV2.targetEntityId).toBe(testUserB);
      expect(relationshipV2.agentId).toBe(testAgentId);
      expect(relationshipV2.tags).toEqual(['friend', 'blocked']); // Status split by comma
      expect(relationshipV2.metadata.userId).toBe(testUserA);
      expect(relationshipV2.metadata.roomId).toBe(createTestUUID(5));
      expect(relationshipV2.createdAt).toBe('2023-01-01T00:00:00Z');
    });
  });

  describe('Enhanced conversion with status mapping', () => {
    it('should map common V2 tags to V1 statuses', () => {
      // Test individual mappings
      expect(tagsToStatus(['friend'])).toBe(RELATIONSHIP_STATUSES.FRIEND);
      expect(tagsToStatus(['blocked'])).toBe(RELATIONSHIP_STATUSES.BLOCKED);
      expect(tagsToStatus(['muted'])).toBe(RELATIONSHIP_STATUSES.MUTED);
      expect(tagsToStatus(['following'])).toBe(RELATIONSHIP_STATUSES.FOLLOWING);
      expect(tagsToStatus(['follower'])).toBe(RELATIONSHIP_STATUSES.FOLLOWED_BY);
      expect(tagsToStatus(['acquaintance'])).toBe(RELATIONSHIP_STATUSES.ACQUAINTANCE);
    });

    it('should map common V1 statuses to V2 tags', () => {
      expect(statusToTags(RELATIONSHIP_STATUSES.FRIEND)).toEqual(['friend']);
      expect(statusToTags(RELATIONSHIP_STATUSES.BLOCKED)).toEqual(['blocked']);
      expect(statusToTags(RELATIONSHIP_STATUSES.MUTED)).toEqual(['muted']);
      expect(statusToTags(RELATIONSHIP_STATUSES.FOLLOWING)).toEqual(['following']);
      expect(statusToTags(RELATIONSHIP_STATUSES.FOLLOWED_BY)).toEqual(['follower']);
      expect(statusToTags(RELATIONSHIP_STATUSES.ACQUAINTANCE)).toEqual(['acquaintance']);
      expect(statusToTags(RELATIONSHIP_STATUSES.UNKNOWN)).toEqual([]);
    });

    it('should handle unknown tags and statuses gracefully', () => {
      // Unknown tags should be joined
      expect(tagsToStatus(['custom-tag', 'another-tag'])).toBe('custom-tag,another-tag');
      
      // Unknown status should be split
      expect(statusToTags('custom,status')).toEqual(['custom', 'status']);
    });

    it('should use enhanced conversion correctly', () => {
      // Arrange
      const relationshipV2: RelationshipV2 = {
        id: testRelId,
        sourceEntityId: testUserA,
        targetEntityId: testUserB,
        agentId: testAgentId,
        tags: ['friend'],
        metadata: {
          roomId: createTestUUID(5),
        },
      };

      // Act
      const relationshipV1 = fromV2RelationshipEnhanced(relationshipV2);

      // Assert
      expect(relationshipV1.status).toBe(RELATIONSHIP_STATUSES.FRIEND);
      expect(relationshipV1.roomId).toBe(createTestUUID(5)); // From metadata
    });
  });

  describe('Utility functions', () => {
    it('should create V1 relationship with defaults', () => {
      // Act
      const relationship = createV1Relationship(testUserA, testUserB);

      // Assert
      expect(relationship.userA).toBe(testUserA);
      expect(relationship.userB).toBe(testUserB);
      expect(relationship.userId).toBe(testUserA);
      expect(relationship.roomId).toBe(testUserA); // Uses userA as fallback
      expect(relationship.status).toBe(RELATIONSHIP_STATUSES.UNKNOWN);
      expect(relationship.id).toContain(testUserA);
      expect(relationship.createdAt).toBeDefined();
    });

    it('should identify equivalent relationships', () => {
      // Arrange
      const rel1: Relationship = {
        id: createTestUUID(10),
        userA: testUserA,
        userB: testUserB,
        userId: testUserA,
        roomId: createTestUUID(5),
        status: 'friend',
      };

      const rel2: Relationship = {
        id: createTestUUID(11),
        userA: testUserB, // Swapped
        userB: testUserA, // Swapped
        userId: testUserB,
        roomId: createTestUUID(6),
        status: 'colleague',
      };

      const rel3: Relationship = {
        id: createTestUUID(12),
        userA: testUserA,
        userB: createTestUUID(99), // Different user
        userId: testUserA,
        roomId: createTestUUID(7),
        status: 'friend',
      };

      // Act & Assert
      expect(areRelationshipsEquivalent(rel1, rel2)).toBe(true); // Same users, different order
      expect(areRelationshipsEquivalent(rel1, rel3)).toBe(false); // Different users
    });
  });

  describe('Round-trip conversion', () => {
    it('should preserve key information in round-trip conversion', () => {
      // Arrange
      const originalRelationship: Relationship = {
        id: testRelId,
        userA: testUserA,
        userB: testUserB,
        userId: testUserA,
        roomId: createTestUUID(5),
        status: RELATIONSHIP_STATUSES.FRIEND,
        createdAt: '2023-01-01T00:00:00Z',
      };

      // Act - convert to v2 and back to v1
      const relationshipV2 = toV2RelationshipEnhanced(originalRelationship, testAgentId);
      const convertedBack = fromV2RelationshipEnhanced(relationshipV2);

      // Assert - key properties should be preserved
      expect(convertedBack.id).toBe(originalRelationship.id);
      expect(convertedBack.userA).toBe(originalRelationship.userA);
      expect(convertedBack.userB).toBe(originalRelationship.userB);
      expect(convertedBack.userId).toBe(originalRelationship.userId);
      expect(convertedBack.roomId).toBe(originalRelationship.roomId);
      expect(convertedBack.status).toBe(originalRelationship.status);
      expect(convertedBack.createdAt).toBe(originalRelationship.createdAt);
    });
  });
});