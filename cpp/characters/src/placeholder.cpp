#include "elizaos/characters.hpp"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <random>
#include <cmath>
#include <iomanip>
#include <any>

namespace elizaos {

// Global character manager instance
std::shared_ptr<CharacterManager> globalCharacterManager = std::make_shared<CharacterManager>();

// Simple UUID generator for characters
std::string generateCharacterUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::string uuid = "char-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
    for (auto& c : uuid) {
        if (c == 'x') {
            c = "0123456789abcdef"[dis(gen)];
        }
    }
    return uuid;
}

// Helper function to clamp values
float clamp(float value, float min, float max) {
    return std::max(min, std::min(max, value));
}

// =====================================================
// CharacterTrait Implementation
// =====================================================

CharacterTrait::CharacterTrait(const std::string& name, const std::string& description,
                              TraitCategory category, TraitValueType valueType)
    : name(name), description(description), category(category), valueType(valueType) {
    // Initialize with default values based on type
    switch (valueType) {
        case TraitValueType::NUMERIC:
            value = 0.5f;
            break;
        case TraitValueType::BOOLEAN:
            value = false;
            break;
        case TraitValueType::CATEGORICAL:
        case TraitValueType::TEXT:
            value = std::string("");
            break;
    }
}

void CharacterTrait::setNumericValue(float val) {
    if (valueType == TraitValueType::NUMERIC) {
        value = clamp(val, 0.0f, 1.0f);
    }
}

void CharacterTrait::setCategoricalValue(const std::string& val) {
    if (valueType == TraitValueType::CATEGORICAL) {
        value = val;
    }
}

void CharacterTrait::setBooleanValue(bool val) {
    if (valueType == TraitValueType::BOOLEAN) {
        value = val;
    }
}

void CharacterTrait::setTextValue(const std::string& val) {
    if (valueType == TraitValueType::TEXT) {
        value = val;
    }
}

float CharacterTrait::getNumericValue() const {
    if (valueType == TraitValueType::NUMERIC) {
        try {
            return std::any_cast<float>(value);
        } catch (const std::bad_any_cast&) {
            return 0.5f;
        }
    }
    return 0.0f;
}

std::string CharacterTrait::getCategoricalValue() const {
    if (valueType == TraitValueType::CATEGORICAL || valueType == TraitValueType::TEXT) {
        try {
            return std::any_cast<std::string>(value);
        } catch (const std::bad_any_cast&) {
            return "";
        }
    }
    return "";
}

bool CharacterTrait::getBooleanValue() const {
    if (valueType == TraitValueType::BOOLEAN) {
        try {
            return std::any_cast<bool>(value);
        } catch (const std::bad_any_cast&) {
            return false;
        }
    }
    return false;
}

std::string CharacterTrait::getTextValue() const {
    return getCategoricalValue();
}

JsonValue CharacterTrait::toJson() const {
    JsonValue json;
    json["name"] = std::string(name);
    json["description"] = std::string(description);
    json["category"] = std::string(traitCategoryToString(category));
    json["valueType"] = std::string(traitValueTypeToString(valueType));
    json["weight"] = std::string(std::to_string(weight));
    
    // Handle value serialization based on type
    switch (valueType) {
        case TraitValueType::NUMERIC:
            json["value"] = std::string(std::to_string(getNumericValue()));
            break;
        case TraitValueType::BOOLEAN:
            json["value"] = std::string(getBooleanValue() ? "true" : "false");
            break;
        case TraitValueType::CATEGORICAL:
        case TraitValueType::TEXT:
            json["value"] = std::string(getCategoricalValue());
            break;
    }
    
    return json;
}

CharacterTrait CharacterTrait::fromJson(const JsonValue& json) {
    auto getString = [&](const std::string& key) -> std::string {
        auto it = json.find(key);
        if (it != json.end()) {
            try {
                return std::any_cast<std::string>(it->second);
            } catch (const std::bad_any_cast&) {
                return "";
            }
        }
        return "";
    };
    
    std::string name = getString("name");
    std::string description = getString("description");
    TraitCategory category = stringToTraitCategory(getString("category"));
    TraitValueType valueType = stringToTraitValueType(getString("valueType"));
    
    CharacterTrait trait(name, description, category, valueType);
    
    try {
        trait.weight = std::stof(getString("weight"));
    } catch (const std::exception&) {
        trait.weight = 1.0f;
    }
    
    std::string valueStr = getString("value");
    switch (valueType) {
        case TraitValueType::NUMERIC:
            try {
                trait.setNumericValue(std::stof(valueStr));
            } catch (const std::exception&) {}
            break;
        case TraitValueType::BOOLEAN:
            trait.setBooleanValue(valueStr == "true");
            break;
        case TraitValueType::CATEGORICAL:
        case TraitValueType::TEXT:
            trait.setCategoricalValue(valueStr);
            break;
    }
    
    return trait;
}

bool CharacterTrait::isCompatibleWith(const CharacterTrait& other) const {
    return category == other.category && valueType == other.valueType;
}

float CharacterTrait::calculateSimilarity(const CharacterTrait& other) const {
    if (!isCompatibleWith(other)) {
        return 0.0f;
    }
    
    switch (valueType) {
        case TraitValueType::NUMERIC: {
            float diff = std::abs(getNumericValue() - other.getNumericValue());
            return 1.0f - diff; // Closer values = higher similarity
        }
        case TraitValueType::BOOLEAN:
            return (getBooleanValue() == other.getBooleanValue()) ? 1.0f : 0.0f;
        case TraitValueType::CATEGORICAL:
        case TraitValueType::TEXT:
            return (getCategoricalValue() == other.getCategoricalValue()) ? 1.0f : 0.0f;
    }
    
    return 0.0f;
}

// =====================================================
// PersonalityMatrix Implementation
// =====================================================

PersonalityMatrix::PersonalityMatrix(float o, float c, float e, float a, float n)
    : openness(clamp(o, 0.0f, 1.0f))
    , conscientiousness(clamp(c, 0.0f, 1.0f))
    , extraversion(clamp(e, 0.0f, 1.0f))
    , agreeableness(clamp(a, 0.0f, 1.0f))
    , neuroticism(clamp(n, 0.0f, 1.0f)) {
}

std::string PersonalityMatrix::getPersonalityType() const {
    // Simplified personality typing based on dominant traits
    std::string type = "";
    
    if (extraversion > 0.6f) type += "E"; else type += "I";
    if (openness > 0.6f) type += "N"; else type += "S";
    if (agreeableness > 0.6f) type += "F"; else type += "T";
    if (conscientiousness > 0.6f) type += "J"; else type += "P";
    
    return type;
}

std::vector<std::string> PersonalityMatrix::getDominantTraits() const {
    std::vector<std::pair<std::string, float>> traits = {
        {"openness", openness},
        {"conscientiousness", conscientiousness},
        {"extraversion", extraversion},
        {"agreeableness", agreeableness},
        {"neuroticism", neuroticism},
        {"creativity", creativity},
        {"empathy", empathy},
        {"assertiveness", assertiveness},
        {"curiosity", curiosity},
        {"loyalty", loyalty}
    };
    
    // Sort by value and return top traits
    std::sort(traits.begin(), traits.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    std::vector<std::string> dominant;
    for (size_t i = 0; i < std::min(size_t(3), traits.size()); ++i) {
        if (traits[i].second > 0.6f) {
            dominant.push_back(traits[i].first);
        }
    }
    
    return dominant;
}

float PersonalityMatrix::calculateCompatibility(const PersonalityMatrix& other) const {
    // Simple compatibility calculation based on trait differences
    float totalDiff = 0.0f;
    totalDiff += std::abs(openness - other.openness);
    totalDiff += std::abs(conscientiousness - other.conscientiousness);
    totalDiff += std::abs(extraversion - other.extraversion);
    totalDiff += std::abs(agreeableness - other.agreeableness);
    totalDiff += std::abs(neuroticism - other.neuroticism);
    
    // Convert difference to compatibility (lower difference = higher compatibility)
    float avgDiff = totalDiff / 5.0f;
    return 1.0f - avgDiff;
}

void PersonalityMatrix::adjustFromExperience(const std::string& experienceType, float intensity) {
    intensity = clamp(intensity, 0.0f, 1.0f);
    float adjustment = intensity * 0.1f; // Small adjustments
    
    if (experienceType == "social_success") {
        extraversion = clamp(extraversion + adjustment, 0.0f, 1.0f);
        agreeableness = clamp(agreeableness + adjustment * 0.5f, 0.0f, 1.0f);
    } else if (experienceType == "creative_achievement") {
        openness = clamp(openness + adjustment, 0.0f, 1.0f);
        creativity = clamp(creativity + adjustment, 0.0f, 1.0f);
    } else if (experienceType == "failure") {
        neuroticism = clamp(neuroticism + adjustment * 0.5f, 0.0f, 1.0f);
        conscientiousness = clamp(conscientiousness + adjustment * 0.3f, 0.0f, 1.0f);
    } else if (experienceType == "leadership") {
        assertiveness = clamp(assertiveness + adjustment, 0.0f, 1.0f);
        conscientiousness = clamp(conscientiousness + adjustment * 0.5f, 0.0f, 1.0f);
    }
}

void PersonalityMatrix::evolveOverTime(float timeFactorDays) {
    // Very gradual personality evolution over time
    float evolutionRate = timeFactorDays * 0.001f; // 0.1% change per day max
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-evolutionRate, evolutionRate);
    
    openness = clamp(openness + dis(gen), 0.0f, 1.0f);
    conscientiousness = clamp(conscientiousness + dis(gen), 0.0f, 1.0f);
    extraversion = clamp(extraversion + dis(gen), 0.0f, 1.0f);
    agreeableness = clamp(agreeableness + dis(gen), 0.0f, 1.0f);
    neuroticism = clamp(neuroticism + dis(gen), 0.0f, 1.0f);
}

JsonValue PersonalityMatrix::toJson() const {
    JsonValue json;
    json["openness"] = std::string(std::to_string(openness));
    json["conscientiousness"] = std::string(std::to_string(conscientiousness));
    json["extraversion"] = std::string(std::to_string(extraversion));
    json["agreeableness"] = std::string(std::to_string(agreeableness));
    json["neuroticism"] = std::string(std::to_string(neuroticism));
    json["creativity"] = std::string(std::to_string(creativity));
    json["empathy"] = std::string(std::to_string(empathy));
    json["assertiveness"] = std::string(std::to_string(assertiveness));
    json["curiosity"] = std::string(std::to_string(curiosity));
    json["loyalty"] = std::string(std::to_string(loyalty));
    return json;
}

PersonalityMatrix PersonalityMatrix::fromJson(const JsonValue& json) {
    auto getFloat = [&](const std::string& key, float defaultVal = 0.5f) -> float {
        auto it = json.find(key);
        if (it != json.end()) {
            try {
                return std::stof(std::any_cast<std::string>(it->second));
            } catch (const std::exception&) {
                return defaultVal;
            }
        }
        return defaultVal;
    };
    
    PersonalityMatrix matrix;
    matrix.openness = getFloat("openness");
    matrix.conscientiousness = getFloat("conscientiousness");
    matrix.extraversion = getFloat("extraversion");
    matrix.agreeableness = getFloat("agreeableness");
    matrix.neuroticism = getFloat("neuroticism");
    matrix.creativity = getFloat("creativity");
    matrix.empathy = getFloat("empathy");
    matrix.assertiveness = getFloat("assertiveness");
    matrix.curiosity = getFloat("curiosity");
    matrix.loyalty = getFloat("loyalty");
    
    return matrix;
}

// =====================================================
// CharacterProfile Implementation
// =====================================================

CharacterProfile::CharacterProfile(const std::string& name, const std::string& description) 
    : name(name), description(description) {
    id = generateUniqueId();
    created_at = std::chrono::system_clock::now();
    updated_at = created_at;
}

std::string CharacterProfile::generateUniqueId() {
    return generateCharacterUUID();
}

void CharacterProfile::updateTimestamp() {
    updated_at = std::chrono::system_clock::now();
}

void CharacterProfile::addTrait(const CharacterTrait& trait) {
    // Remove existing trait with same name if it exists
    removeTrait(trait.name);
    traits.push_back(trait);
    updateTimestamp();
}

void CharacterProfile::removeTrait(const std::string& traitName) {
    auto it = std::remove_if(traits.begin(), traits.end(),
                            [&traitName](const CharacterTrait& t) { return t.name == traitName; });
    if (it != traits.end()) {
        traits.erase(it, traits.end());
        updateTimestamp();
    }
}

std::optional<CharacterTrait> CharacterProfile::getTrait(const std::string& traitName) const {
    auto it = std::find_if(traits.begin(), traits.end(),
                          [&traitName](const CharacterTrait& t) { return t.name == traitName; });
    if (it != traits.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<CharacterTrait> CharacterProfile::getTraitsByCategory(TraitCategory category) const {
    std::vector<CharacterTrait> result;
    for (const auto& trait : traits) {
        if (trait.category == category) {
            result.push_back(trait);
        }
    }
    return result;
}

void CharacterProfile::updateTrait(const std::string& traitName, const CharacterTrait& newTrait) {
    auto it = std::find_if(traits.begin(), traits.end(),
                          [&traitName](const CharacterTrait& t) { return t.name == traitName; });
    if (it != traits.end()) {
        *it = newTrait;
        updateTimestamp();
    }
}

void CharacterProfile::adjustPersonalityDimension(const std::string& dimension, float adjustment) {
    if (dimension == "openness") {
        personality.openness = clamp(personality.openness + adjustment, 0.0f, 1.0f);
    } else if (dimension == "conscientiousness") {
        personality.conscientiousness = clamp(personality.conscientiousness + adjustment, 0.0f, 1.0f);
    } else if (dimension == "extraversion") {
        personality.extraversion = clamp(personality.extraversion + adjustment, 0.0f, 1.0f);
    } else if (dimension == "agreeableness") {
        personality.agreeableness = clamp(personality.agreeableness + adjustment, 0.0f, 1.0f);
    } else if (dimension == "neuroticism") {
        personality.neuroticism = clamp(personality.neuroticism + adjustment, 0.0f, 1.0f);
    }
    updateTimestamp();
}

std::string CharacterProfile::generateResponse(const std::string& input, const std::string& context) const {
    // Simple response generation based on personality and communication style
    std::stringstream response;
    
    // Adjust response based on personality traits
    if (personality.extraversion > 0.7f) {
        response << "Oh, ";
    }
    
    if (personality.agreeableness > 0.6f) {
        response << "I understand what you mean about " << input.substr(0, 20) << "... ";
    }
    
    if (communicationStyle.formality > 0.6f) {
        response << "I believe that ";
    } else {
        response << "I think ";
    }
    
    response << "this is an interesting point";
    
    if (personality.openness > 0.7f) {
        response << " that opens up many possibilities";
    }
    
    response << ".";
    
    // Add context awareness
    if (!context.empty()) {
        response << " In the context of " << context << ", this becomes even more significant.";
    }
    
    return response.str();
}

std::string CharacterProfile::getEmotionalState() const {
    // Determine emotional state based on personality dimensions
    float positivity = (personality.agreeableness + personality.extraversion + (1.0f - personality.neuroticism)) / 3.0f;
    float energy = (personality.extraversion + personality.openness) / 2.0f;
    
    if (positivity > 0.7f && energy > 0.7f) {
        return "excited";
    } else if (positivity > 0.6f) {
        return "positive";
    } else if (positivity < 0.3f) {
        return "melancholic";
    } else if (energy > 0.7f) {
        return "energetic";
    } else {
        return "neutral";
    }
}

void CharacterProfile::learnFromInteraction(const std::string& interaction, const std::string& outcome) {
    // Adjust personality based on interaction outcomes
    if (outcome == "positive") {
        personality.adjustFromExperience("social_success", 0.1f);
    } else if (outcome == "negative") {
        personality.adjustFromExperience("failure", 0.1f);
    }
    
    // Add to experiences
    background.experiences.push_back(interaction + " -> " + outcome);
    updateTimestamp();
}

void CharacterProfile::evolvePersonality(float timeDelta) {
    personality.evolveOverTime(timeDelta);
    updateTimestamp();
}

void CharacterProfile::addExperience(const std::string& experience) {
    background.experiences.push_back(experience);
    updateTimestamp();
}

float CharacterProfile::calculateCompatibility(const CharacterProfile& other) const {
    float personalityCompat = personality.calculateCompatibility(other.personality);
    
    // Factor in trait compatibility
    float traitCompat = 0.0f;
    int compatibleTraits = 0;
    
    for (const auto& myTrait : traits) {
        auto otherTrait = other.getTrait(myTrait.name);
        if (otherTrait) {
            traitCompat += myTrait.calculateSimilarity(*otherTrait);
            compatibleTraits++;
        }
    }
    
    if (compatibleTraits > 0) {
        traitCompat /= compatibleTraits;
    }
    
    // Weighted combination
    return (personalityCompat * 0.7f) + (traitCompat * 0.3f);
}

std::vector<std::string> CharacterProfile::findCommonTraits(const CharacterProfile& other) const {
    std::vector<std::string> common;
    
    for (const auto& myTrait : traits) {
        auto otherTrait = other.getTrait(myTrait.name);
        if (otherTrait && myTrait.calculateSimilarity(*otherTrait) > 0.7f) {
            common.push_back(myTrait.name);
        }
    }
    
    return common;
}

std::string CharacterProfile::predictInteractionStyle(const CharacterProfile& other) const {
    float compatibility = calculateCompatibility(other);
    
    if (compatibility > 0.8f) {
        return "harmonious";
    } else if (compatibility > 0.6f) {
        return "friendly";
    } else if (compatibility > 0.4f) {
        return "neutral";
    } else if (compatibility > 0.2f) {
        return "tense";
    } else {
        return "conflicting";
    }
}

JsonValue CharacterProfile::toJson() const {
    JsonValue json;
    json["id"] = std::string(id);
    json["name"] = std::string(name);
    json["description"] = std::string(description);
    json["version"] = std::string(version);
    json["creator"] = std::string(creator);
    
    // Timestamps
    json["created_at"] = std::string(std::to_string(std::chrono::system_clock::to_time_t(created_at)));
    json["updated_at"] = std::string(std::to_string(std::chrono::system_clock::to_time_t(updated_at)));
    
    return json;
}

CharacterProfile CharacterProfile::fromJson(const JsonValue& json) {
    auto getString = [&](const std::string& key) -> std::string {
        auto it = json.find(key);
        if (it != json.end()) {
            try {
                return std::any_cast<std::string>(it->second);
            } catch (const std::bad_any_cast&) {
                return "";
            }
        }
        return "";
    };
    
    CharacterProfile profile(getString("name"), getString("description"));
    profile.id = getString("id");
    profile.version = getString("version");
    profile.creator = getString("creator");
    
    // Parse timestamps
    try {
        auto created_time_t = std::stoll(getString("created_at"));
        auto updated_time_t = std::stoll(getString("updated_at"));
        profile.created_at = std::chrono::system_clock::from_time_t(created_time_t);
        profile.updated_at = std::chrono::system_clock::from_time_t(updated_time_t);
    } catch (const std::exception&) {
        // Use current time if parsing fails
    }
    
    return profile;
}

bool CharacterProfile::exportToFile(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << "Character Profile Export" << std::endl;
        file << "Name: " << name << std::endl;
        file << "Description: " << description << std::endl;
        file << "Personality Type: " << personality.getPersonalityType() << std::endl;
        file << "Emotional State: " << getEmotionalState() << std::endl;
        file << "Trait Count: " << traits.size() << std::endl;
        
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::optional<CharacterProfile> CharacterProfile::importFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        // Simple parsing - in practice would be more sophisticated
        std::string line;
        std::string name, description;
        
        while (std::getline(file, line)) {
            if (line.find("Name: ") == 0) {
                name = line.substr(6);
            } else if (line.find("Description: ") == 0) {
                description = line.substr(13);
            }
        }
        
        file.close();
        
        if (!name.empty()) {
            return CharacterProfile(name, description);
        }
    } catch (const std::exception&) {
        // Handle error
    }
    
    return std::nullopt;
}

bool CharacterProfile::validate() const {
    return !name.empty() && !id.empty();
}

std::vector<std::string> CharacterProfile::getValidationErrors() const {
    std::vector<std::string> errors;
    
    if (name.empty()) {
        errors.push_back("Name is required");
    }
    if (id.empty()) {
        errors.push_back("ID is required");
    }
    
    return errors;
}

void CharacterProfile::normalizeTraitValues() {
    for (auto& trait : traits) {
        if (trait.valueType == TraitValueType::NUMERIC) {
            float val = trait.getNumericValue();
            trait.setNumericValue(clamp(val, 0.0f, 1.0f));
        }
    }
    updateTimestamp();
}

float CharacterProfile::getTraitInfluence(const std::string& traitName, float defaultValue) const {
    auto trait = getTrait(traitName);
    if (trait && trait->valueType == TraitValueType::NUMERIC) {
        return trait->getNumericValue() * trait->weight;
    }
    return defaultValue;
}

// =====================================================
// CharacterTemplate Implementation
// =====================================================

CharacterTemplate::CharacterTemplate(const std::string& name, const std::string& description)
    : name(name), description(description) {
}

CharacterProfile CharacterTemplate::instantiate(const std::string& characterName) const {
    CharacterProfile profile(characterName, description);
    profile.personality = basePersonality;
    profile.traits = defaultTraits;
    profile.background = templateBackground;
    profile.communicationStyle = templateCommunication;
    return profile;
}

void CharacterTemplate::addVariation(const std::string& variationName, const PersonalityMatrix& personality) {
    // Simple implementation - could store multiple variations
    // Use the variationName parameter to avoid warning
    if (!variationName.empty()) {
        basePersonality = personality;
    }
}

JsonValue CharacterTemplate::toJson() const {
    JsonValue json;
    json["name"] = std::string(name);
    json["description"] = std::string(description);
    return json;
}

CharacterTemplate CharacterTemplate::fromJson(const JsonValue& json) {
    auto getString = [&](const std::string& key) -> std::string {
        auto it = json.find(key);
        if (it != json.end()) {
            try {
                return std::any_cast<std::string>(it->second);
            } catch (const std::bad_any_cast&) {
                return "";
            }
        }
        return "";
    };
    
    return CharacterTemplate(getString("name"), getString("description"));
}

// =====================================================
// CharacterManager Implementation
// =====================================================

CharacterManager::CharacterManager() {
    memory_ = std::make_shared<AgentMemoryManager>();
    logger_ = std::make_shared<AgentLogger>();
    logger_->log("Character manager initialized", "info", "characters");
}

CharacterManager::~CharacterManager() = default;

std::string CharacterManager::generateCharacterId() {
    return generateCharacterUUID();
}

std::string CharacterManager::registerCharacter(const CharacterProfile& character) {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    CharacterProfile newCharacter = character;
    if (newCharacter.id.empty()) {
        newCharacter.id = generateCharacterId();
    }
    
    characters_[newCharacter.id] = newCharacter;
    saveCharacterToMemory(newCharacter);
    
    logger_->log("Registered character: " + newCharacter.name, "info", "characters");
    return newCharacter.id;
}

bool CharacterManager::unregisterCharacter(const std::string& characterId) {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    auto it = characters_.find(characterId);
    if (it != characters_.end()) {
        characters_.erase(it);
        
        // Remove from memory
        UUID memoryId(characterId);
        memory_->deleteMemory(memoryId);
        
        logger_->log("Unregistered character: " + characterId, "info", "characters");
        return true;
    }
    
    return false;
}

std::optional<CharacterProfile> CharacterManager::getCharacter(const std::string& characterId) {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    auto it = characters_.find(characterId);
    if (it != characters_.end()) {
        return it->second;
    }
    
    // Try loading from memory
    auto memoryChar = loadCharacterFromMemory(characterId);
    if (memoryChar) {
        characters_[characterId] = *memoryChar;
        return *memoryChar;
    }
    
    return std::nullopt;
}

std::vector<CharacterProfile> CharacterManager::getAllCharacters() const {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    std::vector<CharacterProfile> result;
    for (const auto& pair : characters_) {
        result.push_back(pair.second);
    }
    
    // Also get any characters only in memory
    auto memoryChars = getAllCharactersFromMemory();
    for (const auto& character : memoryChars) {
        if (characters_.find(character.id) == characters_.end()) {
            result.push_back(character);
        }
    }
    
    return result;
}

bool CharacterManager::updateCharacter(const std::string& characterId, const CharacterProfile& character) {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    auto it = characters_.find(characterId);
    if (it != characters_.end()) {
        CharacterProfile updatedChar = character;
        updatedChar.id = characterId;
        updatedChar.updated_at = std::chrono::system_clock::now();
        
        characters_[characterId] = updatedChar;
        saveCharacterToMemory(updatedChar);
        
        logger_->log("Updated character: " + characterId, "info", "characters");
        return true;
    }
    
    return false;
}

std::vector<CharacterProfile> CharacterManager::searchCharacters(const std::string& query) const {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    std::vector<CharacterProfile> results;
    auto allChars = getAllCharacters();
    
    for (const auto& character : allChars) {
        if (character.name.find(query) != std::string::npos ||
            character.description.find(query) != std::string::npos) {
            results.push_back(character);
        }
    }
    
    return results;
}

std::vector<CharacterProfile> CharacterManager::findCharactersByTrait(const std::string& traitName, 
                                                                     const std::any& value) const {
    std::vector<CharacterProfile> results;
    auto allChars = getAllCharacters();
    
    for (const auto& character : allChars) {
        auto trait = character.getTrait(traitName);
        if (trait) {
            // Simple comparison - in practice would be more sophisticated
            // Use the value parameter to avoid warning
            if (!value.has_value() || trait->name == traitName) {
                results.push_back(character);
            }
        }
    }
    
    return results;
}

std::vector<CharacterProfile> CharacterManager::findCompatibleCharacters(const std::string& characterId,
                                                                        float minCompatibility) const {
    auto targetChar = const_cast<CharacterManager*>(this)->getCharacter(characterId);
    if (!targetChar) {
        return {};
    }
    
    std::vector<CharacterProfile> compatible;
    auto allChars = getAllCharacters();
    
    for (const auto& character : allChars) {
        if (character.id != characterId) {
            float compatibility = targetChar->calculateCompatibility(character);
            if (compatibility >= minCompatibility) {
                compatible.push_back(character);
            }
        }
    }
    
    // Sort by compatibility
    std::sort(compatible.begin(), compatible.end(),
              [&targetChar](const CharacterProfile& a, const CharacterProfile& b) {
                  return targetChar->calculateCompatibility(a) > targetChar->calculateCompatibility(b);
              });
    
    return compatible;
}

void CharacterManager::registerTemplate(const CharacterTemplate& template_) {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    templates_[template_.name] = template_;
    logger_->log("Registered character template: " + template_.name, "info", "characters");
}

std::optional<CharacterTemplate> CharacterManager::getTemplate(const std::string& templateName) const {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    auto it = templates_.find(templateName);
    if (it != templates_.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

std::vector<CharacterTemplate> CharacterManager::getAllTemplates() const {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    std::vector<CharacterTemplate> result;
    for (const auto& pair : templates_) {
        result.push_back(pair.second);
    }
    
    return result;
}

CharacterProfile CharacterManager::createFromTemplate(const std::string& templateName, 
                                                      const std::string& characterName) const {
    auto template_ = getTemplate(templateName);
    if (template_) {
        logger_->log("Creating character from template: " + templateName, "info", "characters");
        return template_->instantiate(characterName);
    }
    
    // Return basic character if template not found
    return CharacterProfile(characterName, "Character created without template");
}

void CharacterManager::evolveAllCharacters(float timeDelta) {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    for (auto& pair : characters_) {
        pair.second.evolvePersonality(timeDelta);
        saveCharacterToMemory(pair.second);
    }
    
    logger_->log("Evolved all characters with time delta: " + std::to_string(timeDelta), 
                "info", "characters");
}

void CharacterManager::saveAllCharacters(const std::string& directory) const {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    int saved = 0;
    for (const auto& pair : characters_) {
        std::string filename = directory + "/" + pair.second.name + "_" + pair.first + ".txt";
        if (pair.second.exportToFile(filename)) {
            saved++;
        }
    }
    
    logger_->log("Saved " + std::to_string(saved) + " characters to directory: " + directory, 
                "info", "characters");
}

bool CharacterManager::loadCharactersFromDirectory(const std::string& directory) {
    // Simple implementation - would scan directory for character files
    logger_->log("Loading characters from directory: " + directory, "info", "characters");
    return true;
}

std::unordered_map<TraitCategory, int> CharacterManager::getTraitCategoryStats() const {
    std::unordered_map<TraitCategory, int> stats;
    auto allChars = getAllCharacters();
    
    for (const auto& character : allChars) {
        for (const auto& trait : character.traits) {
            stats[trait.category]++;
        }
    }
    
    return stats;
}

std::string CharacterManager::getCharacterAnalytics() const {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    
    std::stringstream ss;
    ss << "Character Manager Analytics:" << std::endl;
    ss << "Total characters: " << characters_.size() << std::endl;
    ss << "Total templates: " << templates_.size() << std::endl;
    
    auto stats = getTraitCategoryStats();
    ss << "Trait category distribution:" << std::endl;
    for (const auto& pair : stats) {
        ss << "  " << traitCategoryToString(pair.first) << ": " << pair.second << std::endl;
    }
    
    return ss.str();
}

std::vector<std::pair<std::string, std::string>> CharacterManager::findBestMatches() const {
    std::vector<std::pair<std::string, std::string>> matches;
    auto allChars = getAllCharacters();
    
    for (size_t i = 0; i < allChars.size(); ++i) {
        for (size_t j = i + 1; j < allChars.size(); ++j) {
            float compatibility = allChars[i].calculateCompatibility(allChars[j]);
            if (compatibility > 0.8f) {
                matches.emplace_back(allChars[i].name, allChars[j].name);
            }
        }
    }
    
    return matches;
}

bool CharacterManager::exportToFile(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file << getCharacterAnalytics() << std::endl;
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool CharacterManager::importFromFile(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        logger_->log("Importing characters from file: " + filename, "info", "characters");
        file.close();
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void CharacterManager::clear() {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    characters_.clear();
    templates_.clear();
    memory_->clear();
    logger_->log("Character manager cleared", "info", "characters");
}

size_t CharacterManager::getCharacterCount() const {
    std::lock_guard<std::mutex> lock(charactersMutex_);
    return characters_.size();
}

void CharacterManager::saveCharacterToMemory(const CharacterProfile& character) {
    UUID memoryId(character.id);
    UUID entityId = generateCharacterUUID();
    UUID agentId = generateCharacterUUID();
    
    // Create CustomMetadata for the character
    CustomMetadata customMeta;
    customMeta.customData["id"] = character.id;
    customMeta.customData["name"] = character.name;
    customMeta.customData["description"] = character.description;
    customMeta.customData["version"] = character.version;
    customMeta.customData["creator"] = character.creator;
    customMeta.customData["personality_type"] = character.personality.getPersonalityType();
    customMeta.customData["trait_count"] = std::to_string(character.traits.size());
    customMeta.customData["created_at"] = std::to_string(std::chrono::system_clock::to_time_t(character.created_at));
    customMeta.customData["updated_at"] = std::to_string(std::chrono::system_clock::to_time_t(character.updated_at));
    
    MemoryMetadata metadata = customMeta;
    auto memory = std::make_shared<Memory>(memoryId, character.name + ": " + character.description, 
                                          entityId, agentId, metadata);
    
    memory_->createMemory(memory, "characters");
}

std::optional<CharacterProfile> CharacterManager::loadCharacterFromMemory(const std::string& id) {
    UUID memoryId(id);
    auto memory = memory_->getMemoryById(memoryId);
    
    if (!memory) {
        return std::nullopt;
    }
    
    // Parse metadata if it's CustomMetadata
    if (std::holds_alternative<CustomMetadata>(memory->getMetadata())) {
        const auto& customMeta = std::get<CustomMetadata>(memory->getMetadata());
        
        auto getValue = [&](const std::string& key) -> std::string {
            auto it = customMeta.customData.find(key);
            return it != customMeta.customData.end() ? it->second : "";
        };
        
        std::string name = getValue("name");
        std::string description = getValue("description");
        
        if (!name.empty()) {
            CharacterProfile character(name, description);
            character.id = id;
            character.version = getValue("version");
            character.creator = getValue("creator");
            
            // Parse timestamps
            try {
                auto created_time_t = std::stoll(getValue("created_at"));
                auto updated_time_t = std::stoll(getValue("updated_at"));
                character.created_at = std::chrono::system_clock::from_time_t(created_time_t);
                character.updated_at = std::chrono::system_clock::from_time_t(updated_time_t);
            } catch (const std::exception&) {
                // Use current time if parsing fails
            }
            
            return character;
        }
    }
    
    return std::nullopt;
}

std::vector<CharacterProfile> CharacterManager::getAllCharactersFromMemory() const {
    MemorySearchParams params;
    params.tableName = "characters";
    params.count = 1000; // Large number to get all
    
    auto memories = memory_->getMemories(params);
    std::vector<CharacterProfile> results;
    
    for (const auto& memory : memories) {
        auto character = const_cast<CharacterManager*>(this)->loadCharacterFromMemory(memory->getId());
        if (character) {
            results.push_back(*character);
        }
    }
    
    return results;
}

// =====================================================
// Utility Functions
// =====================================================

std::string traitCategoryToString(TraitCategory category) {
    switch (category) {
        case TraitCategory::PERSONALITY: return "personality";
        case TraitCategory::COGNITIVE: return "cognitive";
        case TraitCategory::BEHAVIORAL: return "behavioral";
        case TraitCategory::EMOTIONAL: return "emotional";
        case TraitCategory::SOCIAL: return "social";
        case TraitCategory::PHYSICAL: return "physical";
        case TraitCategory::SKILL: return "skill";
        case TraitCategory::PREFERENCE: return "preference";
        default: return "personality";
    }
}

TraitCategory stringToTraitCategory(const std::string& categoryStr) {
    if (categoryStr == "cognitive") return TraitCategory::COGNITIVE;
    if (categoryStr == "behavioral") return TraitCategory::BEHAVIORAL;
    if (categoryStr == "emotional") return TraitCategory::EMOTIONAL;
    if (categoryStr == "social") return TraitCategory::SOCIAL;
    if (categoryStr == "physical") return TraitCategory::PHYSICAL;
    if (categoryStr == "skill") return TraitCategory::SKILL;
    if (categoryStr == "preference") return TraitCategory::PREFERENCE;
    return TraitCategory::PERSONALITY;
}

std::string traitValueTypeToString(TraitValueType type) {
    switch (type) {
        case TraitValueType::NUMERIC: return "numeric";
        case TraitValueType::CATEGORICAL: return "categorical";
        case TraitValueType::BOOLEAN: return "boolean";
        case TraitValueType::TEXT: return "text";
        default: return "numeric";
    }
}

TraitValueType stringToTraitValueType(const std::string& typeStr) {
    if (typeStr == "categorical") return TraitValueType::CATEGORICAL;
    if (typeStr == "boolean") return TraitValueType::BOOLEAN;
    if (typeStr == "text") return TraitValueType::TEXT;
    return TraitValueType::NUMERIC;
}

// =====================================================
// Character Archetypes
// =====================================================

namespace CharacterArchetypes {

CharacterTemplate createScientist() {
    CharacterTemplate scientist("Scientist", "Analytical and curious researcher");
    scientist.basePersonality.openness = 0.9f;
    scientist.basePersonality.conscientiousness = 0.8f;
    scientist.basePersonality.curiosity = 0.95f;
    scientist.basePersonality.creativity = 0.7f;
    
    CharacterTrait analyticalTrait("analytical", "Tendency to analyze and break down problems", 
                                  TraitCategory::COGNITIVE, TraitValueType::NUMERIC);
    analyticalTrait.setNumericValue(0.9f);
    scientist.defaultTraits.push_back(analyticalTrait);
    
    scientist.templateCommunication.tone = "precise";
    scientist.templateCommunication.vocabulary = "technical";
    scientist.templateCommunication.formality = 0.7f;
    
    return scientist;
}

CharacterTemplate createArtist() {
    CharacterTemplate artist("Artist", "Creative and expressive individual");
    artist.basePersonality.openness = 0.95f;
    artist.basePersonality.creativity = 0.9f;
    
    CharacterTrait creativeTrait("creative", "Strong creative expression ability", 
                                TraitCategory::PERSONALITY, TraitValueType::NUMERIC);
    creativeTrait.setNumericValue(0.9f);
    artist.defaultTraits.push_back(creativeTrait);
    
    artist.templateCommunication.tone = "expressive";
    artist.templateCommunication.emotionality = 0.8f;
    
    return artist;
}

CharacterTemplate createLeader() {
    CharacterTemplate leader("Leader", "Natural leadership and organizational abilities");
    leader.basePersonality.extraversion = 0.8f;
    leader.basePersonality.conscientiousness = 0.85f;
    leader.basePersonality.assertiveness = 0.9f;
    
    CharacterTrait leadershipTrait("leadership", "Natural ability to lead and organize", 
                                  TraitCategory::SOCIAL, TraitValueType::NUMERIC);
    leadershipTrait.setNumericValue(0.85f);
    leader.defaultTraits.push_back(leadershipTrait);
    
    leader.templateCommunication.formality = 0.6f;
    
    return leader;
}

CharacterTemplate createHelper() {
    CharacterTemplate helper("Helper", "Supportive and caring individual");
    helper.basePersonality.agreeableness = 0.9f;
    helper.basePersonality.empathy = 0.9f;
    helper.basePersonality.loyalty = 0.8f;
    
    CharacterTrait empathyTrait("empathy", "Strong ability to understand others' feelings", 
                               TraitCategory::EMOTIONAL, TraitValueType::NUMERIC);
    empathyTrait.setNumericValue(0.9f);
    helper.defaultTraits.push_back(empathyTrait);
    
    helper.templateCommunication.tone = "warm";
    helper.templateCommunication.emotionality = 0.9f;
    
    return helper;
}

CharacterTemplate createExplorer() {
    CharacterTemplate explorer("Explorer", "Adventurous and discovery-oriented");
    explorer.basePersonality.openness = 0.9f;
    explorer.basePersonality.curiosity = 0.85f;
    explorer.basePersonality.extraversion = 0.7f;
    
    CharacterTrait adventureTrait("adventurous", "Seeks new experiences and challenges", 
                                 TraitCategory::BEHAVIORAL, TraitValueType::NUMERIC);
    adventureTrait.setNumericValue(0.85f);
    explorer.defaultTraits.push_back(adventureTrait);
    
    explorer.templateCommunication.tone = "enthusiastic";
    explorer.templateCommunication.emotionality = 0.8f;
    
    return explorer;
}

CharacterTemplate createGuardian() {
    CharacterTemplate guardian("Guardian", "Protective and responsible individual");
    guardian.basePersonality.conscientiousness = 0.9f;
    guardian.basePersonality.loyalty = 0.9f;
    guardian.basePersonality.agreeableness = 0.7f;
    
    CharacterTrait protectiveTrait("protective", "Strong desire to protect and care for others", 
                                  TraitCategory::BEHAVIORAL, TraitValueType::NUMERIC);
    protectiveTrait.setNumericValue(0.85f);
    guardian.defaultTraits.push_back(protectiveTrait);
    
    guardian.templateCommunication.tone = "steady";
    guardian.templateCommunication.formality = 0.9f;
    
    return guardian;
}

CharacterTemplate createInnovator() {
    CharacterTemplate innovator("Innovator", "Forward-thinking problem solver");
    innovator.basePersonality.openness = 0.85f;
    innovator.basePersonality.creativity = 0.9f;
    innovator.basePersonality.assertiveness = 0.7f;
    
    CharacterTrait innovativeTrait("innovative", "Ability to create novel solutions", 
                                  TraitCategory::COGNITIVE, TraitValueType::NUMERIC);
    innovativeTrait.setNumericValue(0.9f);
    innovator.defaultTraits.push_back(innovativeTrait);
    
    innovator.templateCommunication.tone = "forward-thinking";
    innovator.templateCommunication.verbosity = 0.8f;
    
    return innovator;
}

CharacterTemplate createMentor() {
    CharacterTemplate mentor("Mentor", "Wise and guidance-oriented teacher");
    mentor.basePersonality.openness = 0.8f;
    mentor.basePersonality.agreeableness = 0.8f;
    mentor.basePersonality.empathy = 0.8f;
    
    CharacterTrait wisdomTrait("wisdom", "Deep understanding and good judgment", 
                              TraitCategory::COGNITIVE, TraitValueType::NUMERIC);
    wisdomTrait.setNumericValue(0.85f);
    mentor.defaultTraits.push_back(wisdomTrait);
    
    mentor.templateCommunication.tone = "wise";
    mentor.templateCommunication.formality = 0.9f;
    
    return mentor;
}

} // namespace CharacterArchetypes

} // namespace elizaos
