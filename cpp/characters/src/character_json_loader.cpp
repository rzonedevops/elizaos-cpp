#include "elizaos/character_json_loader.hpp"
#include "elizaos/agentlogger.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace elizaos {

using json = nlohmann::json;

std::optional<CharacterProfile> CharacterJsonLoader::loadFromFile(const std::string& filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        
        return loadFromJsonString(buffer.str());
    } catch (const std::exception& e) {
        // Log error but don't throw
        return std::nullopt;
    }
}

std::optional<CharacterProfile> CharacterJsonLoader::loadFromJsonString(const std::string& jsonString) {
    try {
        json j = json::parse(jsonString);
        
        // Extract basic character information
        std::string name = j.value("name", "");
        std::string description = "";
        
        // Try different description fields
        if (j.contains("bio") && j["bio"].is_array() && !j["bio"].empty()) {
            // If bio is an array, take the first element or concatenate
            if (j["bio"][0].is_string()) {
                description = j["bio"][0].get<std::string>();
            }
        } else if (j.contains("bio") && j["bio"].is_string()) {
            description = j["bio"].get<std::string>();
        } else if (j.contains("description")) {
            description = j["description"].get<std::string>();
        }
        
        if (name.empty()) {
            return std::nullopt;
        }
        
        CharacterProfile character(name, description);
        
        // Parse personality from style information
        if (j.contains("style")) {
            auto style = j["style"];
            
            // Extract communication style preferences
            if (style.contains("all")) {
                auto allStyles = style["all"];
                for (const auto& styleItem : allStyles) {
                    if (styleItem.is_string()) {
                        std::string styleStr = styleItem.get<std::string>();
                        
                        // Map style descriptions to personality traits
                        if (styleStr.find("formal") != std::string::npos || 
                            styleStr.find("proper") != std::string::npos) {
                            character.communicationStyle.formality = 0.8f;
                        }
                        if (styleStr.find("verbose") != std::string::npos || 
                            styleStr.find("detailed") != std::string::npos) {
                            character.communicationStyle.verbosity = 0.8f;
                        }
                        if (styleStr.find("emotional") != std::string::npos || 
                            styleStr.find("expressive") != std::string::npos) {
                            character.communicationStyle.emotionality = 0.8f;
                        }
                    }
                }
            }
            
            // Handle chat-specific styles
            if (style.contains("chat")) {
                auto chatStyles = style["chat"];
                for (const auto& styleItem : chatStyles) {
                    if (styleItem.is_string()) {
                        std::string styleStr = styleItem.get<std::string>();
                        character.communicationStyle.responseStyles["chat"] = styleStr;
                    }
                }
            }
            
            // Handle post-specific styles  
            if (style.contains("post")) {
                auto postStyles = style["post"];
                for (const auto& styleItem : postStyles) {
                    if (styleItem.is_string()) {
                        std::string styleStr = styleItem.get<std::string>();
                        character.communicationStyle.responseStyles["post"] = styleStr;
                    }
                }
            }
        }
        
        // Parse bio information into personality traits
        if (j.contains("bio") && j["bio"].is_array()) {
            for (const auto& bioItem : j["bio"]) {
                if (bioItem.is_string()) {
                    std::string bioStr = bioItem.get<std::string>();
                    character.background.experiences.push_back(bioStr);
                    
                    // Infer personality traits from bio
                    if (bioStr.find("loyal") != std::string::npos || bioStr.find("faithful") != std::string::npos) {
                        character.personality.loyalty = std::min(1.0f, character.personality.loyalty + 0.1f);
                    }
                    if (bioStr.find("creative") != std::string::npos || bioStr.find("artistic") != std::string::npos) {
                        character.personality.creativity = std::min(1.0f, character.personality.creativity + 0.2f);
                    }
                    if (bioStr.find("intelligent") != std::string::npos || bioStr.find("smart") != std::string::npos) {
                        character.personality.openness = std::min(1.0f, character.personality.openness + 0.1f);
                    }
                    if (bioStr.find("friendly") != std::string::npos || bioStr.find("kind") != std::string::npos) {
                        character.personality.agreeableness = std::min(1.0f, character.personality.agreeableness + 0.15f);
                    }
                    if (bioStr.find("organized") != std::string::npos || bioStr.find("disciplined") != std::string::npos) {
                        character.personality.conscientiousness = std::min(1.0f, character.personality.conscientiousness + 0.15f);
                    }
                }
            }
        }
        
        // Parse lore information
        if (j.contains("lore") && j["lore"].is_array()) {
            for (const auto& loreItem : j["lore"]) {
                if (loreItem.is_string()) {
                    character.background.backstory += loreItem.get<std::string>() + " ";
                }
            }
        }
        
        // Parse knowledge information  
        if (j.contains("knowledge") && j["knowledge"].is_array()) {
            for (const auto& knowledgeItem : j["knowledge"]) {
                if (knowledgeItem.is_string()) {
                    // Handle simple string format
                    character.background.additionalContext["knowledge"] += knowledgeItem.get<std::string>() + "; ";
                } else if (knowledgeItem.is_object() && knowledgeItem.contains("content")) {
                    // Handle object format with id, path, content
                    std::string content = knowledgeItem["content"].get<std::string>();
                    character.background.additionalContext["knowledge"] += content + "; ";
                    
                    // Store additional metadata if present
                    if (knowledgeItem.contains("id")) {
                        std::string id = knowledgeItem["id"].get<std::string>();
                        character.background.additionalContext["knowledge_id_" + id] = content;
                    }
                    if (knowledgeItem.contains("path")) {
                        std::string path = knowledgeItem["path"].get<std::string>();
                        character.background.additionalContext["knowledge_source"] += path + "; ";
                    }
                }
            }
        }
        
        // Parse adjectives into traits
        if (j.contains("adjectives") && j["adjectives"].is_array()) {
            for (const auto& adjective : j["adjectives"]) {
                if (adjective.is_string()) {
                    std::string adj = adjective.get<std::string>();
                    
                    // Create categorical traits from adjectives
                    CharacterTrait adjectiveTrait(adj, "Character adjective: " + adj, 
                                                 TraitCategory::PERSONALITY, TraitValueType::BOOLEAN);
                    adjectiveTrait.setBooleanValue(true);
                    character.addTrait(adjectiveTrait);
                    
                    // Map adjectives to personality dimensions
                    if (adj == "creative" || adj == "imaginative" || adj == "innovative") {
                        character.personality.creativity = std::min(1.0f, character.personality.creativity + 0.2f);
                        character.personality.openness = std::min(1.0f, character.personality.openness + 0.1f);
                    } else if (adj == "loyal" || adj == "faithful" || adj == "devoted") {
                        character.personality.loyalty = std::min(1.0f, character.personality.loyalty + 0.2f);
                    } else if (adj == "intelligent" || adj == "wise" || adj == "analytical") {
                        character.personality.openness = std::min(1.0f, character.personality.openness + 0.15f);
                    } else if (adj == "friendly" || adj == "kind" || adj == "compassionate") {
                        character.personality.agreeableness = std::min(1.0f, character.personality.agreeableness + 0.15f);
                        character.personality.empathy = std::min(1.0f, character.personality.empathy + 0.15f);
                    } else if (adj == "energetic" || adj == "outgoing" || adj == "social") {
                        character.personality.extraversion = std::min(1.0f, character.personality.extraversion + 0.15f);
                    } else if (adj == "disciplined" || adj == "organized" || adj == "responsible") {
                        character.personality.conscientiousness = std::min(1.0f, character.personality.conscientiousness + 0.15f);
                    } else if (adj == "confident" || adj == "assertive" || adj == "strong") {
                        character.personality.assertiveness = std::min(1.0f, character.personality.assertiveness + 0.15f);
                    }
                }
            }
        }
        
        // Parse topics into interests
        if (j.contains("topics") && j["topics"].is_array()) {
            for (const auto& topic : j["topics"]) {
                if (topic.is_string()) {
                    character.background.additionalContext["interests"] += topic.get<std::string>() + ", ";
                }
            }
        }
        
        // Parse message examples for communication patterns
        if (j.contains("messageExamples") && j["messageExamples"].is_array()) {
            int exampleCount = 0;
            for (const auto& example : j["messageExamples"]) {
                if (example.is_array() && exampleCount < 3) { // Limit examples for performance
                    for (const auto& message : example) {
                        if (message.is_object() && message.contains("content") && 
                            message["content"].contains("text")) {
                            std::string text = message["content"]["text"].get<std::string>();
                            character.background.additionalContext["message_example_" + std::to_string(exampleCount)] = text;
                            exampleCount++;
                            break; // One message per example
                        }
                    }
                }
            }
        }
        
        // Set metadata
        if (j.contains("modelProvider")) {
            character.metadata["modelProvider"] = j["modelProvider"].get<std::string>();
        }
        if (j.contains("clients") && j["clients"].is_array()) {
            std::string clients = "";
            for (const auto& client : j["clients"]) {
                if (client.is_string()) {
                    clients += client.get<std::string>() + ",";
                }
            }
            character.metadata["clients"] = clients;
        }
        
        // Create character-specific traits based on the character type
        if (name.find("trump") != std::string::npos || name.find("Trump") != std::string::npos) {
            CharacterTrait assertiveTrait("assertiveness", "Strong assertive communication", 
                                        TraitCategory::PERSONALITY, TraitValueType::NUMERIC);
            assertiveTrait.setNumericValue(0.95f);
            character.addTrait(assertiveTrait);
            
            character.personality.assertiveness = 0.95f;
            character.personality.extraversion = 0.9f;
            character.communicationStyle.tone = "assertive";
            character.communicationStyle.verbosity = 0.8f;
        } else if (name.find("Alfred") != std::string::npos || name.find("alfred") != std::string::npos) {
            CharacterTrait formalityTrait("formality", "Formal and proper demeanor", 
                                        TraitCategory::SOCIAL, TraitValueType::NUMERIC);
            formalityTrait.setNumericValue(0.9f);
            character.addTrait(formalityTrait);
            
            character.personality.conscientiousness = 0.9f;
            character.personality.loyalty = 0.95f;
            character.communicationStyle.tone = "formal";
            character.communicationStyle.formality = 0.9f;
        }
        
        return character;
        
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::vector<CharacterProfile> CharacterJsonLoader::loadFromDirectory(const std::string& directoryPath) {
    std::vector<CharacterProfile> characters;
    
    try {
        if (!std::filesystem::exists(directoryPath) || !std::filesystem::is_directory(directoryPath)) {
            return characters;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                std::string filepath = entry.path().string();
                std::string extension = entry.path().extension().string();
                
                // Check for JSON files
                if (extension == ".json" || filepath.find(".character.json") != std::string::npos) {
                    auto character = loadFromFile(filepath);
                    if (character.has_value()) {
                        characters.push_back(character.value());
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        // Log error but continue
    }
    
    return characters;
}

bool CharacterJsonLoader::saveToFile(const CharacterProfile& character, const std::string& filepath) {
    try {
        std::string jsonStr = toJsonString(character);
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        file << jsonStr;
        file.close();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string CharacterJsonLoader::toJsonString(const CharacterProfile& character) {
    try {
        json j;
        
        j["name"] = character.name;
        j["description"] = character.description;
        j["version"] = character.version;
        j["creator"] = character.creator;
        
        // Bio from experiences
        j["bio"] = json::array();
        for (const auto& experience : character.background.experiences) {
            j["bio"].push_back(experience);
        }
        
        // Lore from backstory
        if (!character.background.backstory.empty()) {
            j["lore"] = json::array();
            j["lore"].push_back(character.background.backstory);
        }
        
        // Personality as adjectives
        j["adjectives"] = json::array();
        for (const auto& trait : character.traits) {
            if (trait.valueType == TraitValueType::BOOLEAN && trait.getBooleanValue()) {
                j["adjectives"].push_back(trait.name);
            }
        }
        
        // Communication style
        j["style"] = json::object();
        j["style"]["all"] = json::array();
        
        if (character.communicationStyle.formality > 0.7f) {
            j["style"]["all"].push_back("formal and proper communication");
        }
        if (character.communicationStyle.verbosity > 0.7f) {
            j["style"]["all"].push_back("detailed and verbose responses");
        }
        if (character.communicationStyle.emotionality > 0.7f) {
            j["style"]["all"].push_back("expressive and emotional communication");
        }
        
        // Metadata
        for (const auto& [key, value] : character.metadata) {
            j[key] = value;
        }
        
        // Topics from interests
        auto interestsIt = character.background.additionalContext.find("interests");
        if (interestsIt != character.background.additionalContext.end()) {
            j["topics"] = json::array();
            std::stringstream ss(interestsIt->second);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) {
                    j["topics"].push_back(item);
                }
            }
        }
        
        return j.dump(2); // Pretty print with 2-space indentation
    } catch (const std::exception& e) {
        return "{}";
    }
}

// Helper functions implementation

std::string CharacterJsonLoader::getStringFromJson(const JsonValue& json, const std::string& key, const std::string& defaultValue) {
    auto it = json.find(key);
    if (it != json.end()) {
        try {
            return std::any_cast<std::string>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

std::vector<std::string> CharacterJsonLoader::getStringArrayFromJson(const JsonValue& json, const std::string& key) {
    std::vector<std::string> result;
    auto it = json.find(key);
    if (it != json.end()) {
        // This is a simplified implementation - would need proper JSON array parsing
        // For now, just return empty vector
    }
    return result;
}

float CharacterJsonLoader::getFloatFromJson(const JsonValue& json, const std::string& key, float defaultValue) {
    auto it = json.find(key);
    if (it != json.end()) {
        try {
            std::string valueStr = std::any_cast<std::string>(it->second);
            return std::stof(valueStr);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

} // namespace elizaos