#pragma once

#include "elizaos/core.hpp"
#include "elizaos/agentmemory.hpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentaction.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <optional>
#include <any>

namespace elizaos {

// Character trait categories
enum class TraitCategory {
    PERSONALITY,
    COGNITIVE,
    BEHAVIORAL,
    EMOTIONAL,
    SOCIAL,
    PHYSICAL,
    SKILL,
    PREFERENCE
};

// Trait value types
enum class TraitValueType {
    NUMERIC,     // 0.0 - 1.0 scale
    CATEGORICAL, // discrete categories
    BOOLEAN,     // true/false
    TEXT         // free text description
};

// Forward declarations
class CharacterTrait;
class CharacterProfile;
class CharacterManager;
class PersonalityMatrix;

// Character trait structure
class CharacterTrait {
public:
    std::string name;
    std::string description;
    TraitCategory category;
    TraitValueType valueType;
    std::any value;
    float weight = 1.0f;  // Importance weight 0.0-1.0
    std::vector<std::string> tags;
    std::unordered_map<std::string, std::string> metadata;
    
    CharacterTrait(const std::string& name, const std::string& description,
                  TraitCategory category, TraitValueType valueType);
    
    // Value setters/getters with type safety
    void setNumericValue(float val);
    void setCategoricalValue(const std::string& val);
    void setBooleanValue(bool val);
    void setTextValue(const std::string& val);
    
    float getNumericValue() const;
    std::string getCategoricalValue() const;
    bool getBooleanValue() const;
    std::string getTextValue() const;
    
    // Utility methods
    JsonValue toJson() const;
    static CharacterTrait fromJson(const JsonValue& json);
    bool isCompatibleWith(const CharacterTrait& other) const;
    float calculateSimilarity(const CharacterTrait& other) const;
};

// Personality matrix for psychological modeling
class PersonalityMatrix {
public:
    // Big Five personality dimensions (0.0 - 1.0)
    float openness = 0.5f;
    float conscientiousness = 0.5f;
    float extraversion = 0.5f;
    float agreeableness = 0.5f;
    float neuroticism = 0.5f;
    
    // Additional dimensions
    float creativity = 0.5f;
    float empathy = 0.5f;
    float assertiveness = 0.5f;
    float curiosity = 0.5f;
    float loyalty = 0.5f;
    
    PersonalityMatrix() = default;
    PersonalityMatrix(float o, float c, float e, float a, float n);
    
    // Personality analysis
    std::string getPersonalityType() const;
    std::vector<std::string> getDominantTraits() const;
    float calculateCompatibility(const PersonalityMatrix& other) const;
    
    // Adjustments based on experience
    void adjustFromExperience(const std::string& experienceType, float intensity);
    void evolveOverTime(float timeFactorDays);
    
    JsonValue toJson() const;
    static PersonalityMatrix fromJson(const JsonValue& json);
};

// Character background and context
struct CharacterBackground {
    std::string backstory;
    std::string origin;
    std::string occupation;
    std::vector<std::string> relationships;
    std::vector<std::string> experiences;
    std::vector<std::string> goals;
    std::vector<std::string> fears;
    std::vector<std::string> motivations;
    std::unordered_map<std::string, std::string> additionalContext;
};

// Character speaking style and communication patterns
struct CommunicationStyle {
    std::string tone = "neutral";              // casual, formal, friendly, etc.
    std::string vocabulary = "standard";       // simple, advanced, technical, etc.
    float verbosity = 0.5f;                   // 0.0 (concise) to 1.0 (verbose)
    float formality = 0.5f;                   // 0.0 (casual) to 1.0 (formal)
    float emotionality = 0.5f;                // 0.0 (stoic) to 1.0 (expressive)
    std::vector<std::string> catchphrases;
    std::vector<std::string> speakingPatterns;
    std::unordered_map<std::string, std::string> responseStyles;
};

// Main character profile class
class CharacterProfile {
public:
    std::string id;
    std::string name;
    std::string description;
    std::string version = "1.0";
    
    // Core personality
    PersonalityMatrix personality;
    std::vector<CharacterTrait> traits;
    CharacterBackground background;
    CommunicationStyle communicationStyle;
    
    // Metadata
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::string creator;
    std::vector<std::string> tags;
    std::unordered_map<std::string, std::string> metadata;
    
    CharacterProfile() = default;  // Add default constructor
    CharacterProfile(const std::string& name, const std::string& description = "");
    ~CharacterProfile() = default;
    
    // Trait management
    void addTrait(const CharacterTrait& trait);
    void removeTrait(const std::string& traitName);
    std::optional<CharacterTrait> getTrait(const std::string& traitName) const;
    std::vector<CharacterTrait> getTraitsByCategory(TraitCategory category) const;
    void updateTrait(const std::string& traitName, const CharacterTrait& newTrait);
    
    // Personality operations
    void adjustPersonalityDimension(const std::string& dimension, float adjustment);
    std::string generateResponse(const std::string& input, const std::string& context = "") const;
    std::string getEmotionalState() const;
    
    // Character development
    void learnFromInteraction(const std::string& interaction, const std::string& outcome);
    void evolvePersonality(float timeDelta);
    void addExperience(const std::string& experience);
    
    // Compatibility and relationships
    float calculateCompatibility(const CharacterProfile& other) const;
    std::vector<std::string> findCommonTraits(const CharacterProfile& other) const;
    std::string predictInteractionStyle(const CharacterProfile& other) const;
    
    // Serialization
    JsonValue toJson() const;
    static CharacterProfile fromJson(const JsonValue& json);
    bool exportToFile(const std::string& filename) const;
    static std::optional<CharacterProfile> importFromFile(const std::string& filename);
    
    // Validation and integrity
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;
    void normalizeTraitValues();
    
private:
    void updateTimestamp();
    std::string generateUniqueId();
    float getTraitInfluence(const std::string& traitName, float defaultValue = 0.5f) const;
};

// Character template system
class CharacterTemplate {
public:
    std::string name;
    std::string description;
    PersonalityMatrix basePersonality;
    std::vector<CharacterTrait> defaultTraits;
    CharacterBackground templateBackground;
    CommunicationStyle templateCommunication;
    
    CharacterTemplate() = default;  // Add default constructor
    CharacterTemplate(const std::string& name, const std::string& description);
    
    CharacterProfile instantiate(const std::string& characterName) const;
    void addVariation(const std::string& variationName, const PersonalityMatrix& personality);
    
    JsonValue toJson() const;
    static CharacterTemplate fromJson(const JsonValue& json);
};

// Character manager for handling multiple characters
class CharacterManager {
public:
    CharacterManager();
    ~CharacterManager();
    
    // Character lifecycle management
    std::string registerCharacter(const CharacterProfile& character);
    bool unregisterCharacter(const std::string& characterId);
    std::optional<CharacterProfile> getCharacter(const std::string& characterId);
    std::vector<CharacterProfile> getAllCharacters() const;
    bool updateCharacter(const std::string& characterId, const CharacterProfile& character);
    
    // Search and discovery
    std::vector<CharacterProfile> searchCharacters(const std::string& query) const;
    std::vector<CharacterProfile> findCharactersByTrait(const std::string& traitName, 
                                                       const std::any& value) const;
    std::vector<CharacterProfile> findCompatibleCharacters(const std::string& characterId,
                                                          float minCompatibility = 0.7f) const;
    
    // Character templates
    void registerTemplate(const CharacterTemplate& template_);
    std::optional<CharacterTemplate> getTemplate(const std::string& templateName) const;
    std::vector<CharacterTemplate> getAllTemplates() const;
    CharacterProfile createFromTemplate(const std::string& templateName, 
                                       const std::string& characterName) const;
    
    // Batch operations
    void evolveAllCharacters(float timeDelta);
    void saveAllCharacters(const std::string& directory) const;
    bool loadCharactersFromDirectory(const std::string& directory);
    
    // Analytics and insights
    std::unordered_map<TraitCategory, int> getTraitCategoryStats() const;
    std::string getCharacterAnalytics() const;
    std::vector<std::pair<std::string, std::string>> findBestMatches() const;
    
    // Import/Export
    bool exportToFile(const std::string& filename) const;
    bool importFromFile(const std::string& filename);
    void clear();
    
    size_t getCharacterCount() const;
    
private:
    std::unordered_map<std::string, CharacterProfile> characters_;
    std::unordered_map<std::string, CharacterTemplate> templates_;
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<AgentLogger> logger_;
    mutable std::mutex charactersMutex_;
    
    void saveCharacterToMemory(const CharacterProfile& character);
    std::optional<CharacterProfile> loadCharacterFromMemory(const std::string& id);
    std::vector<CharacterProfile> getAllCharactersFromMemory() const;
    std::string generateCharacterId();
};

// Global character manager instance
extern std::shared_ptr<CharacterManager> globalCharacterManager;

// Utility functions
std::string traitCategoryToString(TraitCategory category);
TraitCategory stringToTraitCategory(const std::string& categoryStr);
std::string traitValueTypeToString(TraitValueType type);
TraitValueType stringToTraitValueType(const std::string& typeStr);

// Predefined character archetypes
namespace CharacterArchetypes {
    CharacterTemplate createScientist();
    CharacterTemplate createArtist();
    CharacterTemplate createLeader();
    CharacterTemplate createHelper();
    CharacterTemplate createExplorer();
    CharacterTemplate createGuardian();
    CharacterTemplate createInnovator();
    CharacterTemplate createMentor();
}

} // namespace elizaos