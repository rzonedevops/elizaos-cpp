#include "elizaos/characterfile.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <iomanip>
#include <ctime>

namespace elizaos {

// JsonCharacterFileHandler Implementation
std::optional<CharacterProfile> JsonCharacterFileHandler::loadFromFile(
    const std::string& filepath, 
    const CharacterFileOptions& options) const {
    
    try {
        // Validate file exists
        if (!std::filesystem::exists(filepath)) {
            return std::nullopt;
        }
        
        // Read file content
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        // Validate schema if requested
        if (options.validateSchema) {
            auto validation = validateJsonSchema(content);
            if (!validation.isValid && options.strictValidation) {
                return std::nullopt;
            }
        }
        
        // Use existing JSON loader
        return CharacterJsonLoader::loadFromJsonString(content);
        
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

bool JsonCharacterFileHandler::saveToFile(
    const CharacterProfile& character, 
    const std::string& filepath,
    const CharacterFileOptions& options) const {
    
    try {
        std::string content = toString(character, options);
        
        // Ensure directory exists
        std::filesystem::path path(filepath);
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }
        
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }
        
        file << content;
        file.close();
        
        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

CharacterFileValidation JsonCharacterFileHandler::validateFile(
    const std::string& filepath,
    const CharacterFileOptions& /* options */) const {
    
    CharacterFileValidation result;
    
    try {
        if (!std::filesystem::exists(filepath)) {
            result.errorMessage = "File does not exist";
            return result;
        }
        
        std::ifstream file(filepath);
        if (!file.is_open()) {
            result.errorMessage = "Cannot open file";
            return result;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        result = validateJsonSchema(content);
        result.detectedFormat = CharacterFileFormat::JSON;
        
        return result;
        
    } catch (const std::exception& e) {
        result.errorMessage = "Validation error: " + std::string(e.what());
        return result;
    }
}

std::string JsonCharacterFileHandler::toString(
    const CharacterProfile& character,
    const CharacterFileOptions& /* options */) const {
    
    // Use existing JSON converter if available, otherwise create basic JSON
    return CharacterJsonLoader::toJsonString(character);
}

std::optional<CharacterProfile> JsonCharacterFileHandler::fromString(
    const std::string& content,
    const CharacterFileOptions& /* options */) const {
    
    return CharacterJsonLoader::loadFromJsonString(content);
}

CharacterFileValidation JsonCharacterFileHandler::validateJsonSchema(const std::string& content) const {
    CharacterFileValidation result;
    
    // Basic JSON validation - check for basic structure
    if (content.empty()) {
        result.errorMessage = "Content is empty";
        return result;
    }
    
    // Look for required JSON structure
    if (content.find('{') == std::string::npos || content.find('}') == std::string::npos) {
        result.errorMessage = "Invalid JSON structure";
        return result;
    }
    
    // Check for required character fields
    std::vector<std::string> requiredFields = {"name", "description"};
    for (const auto& field : requiredFields) {
        if (content.find("\"" + field + "\"") == std::string::npos) {
            result.warnings.push_back("Missing recommended field: " + field);
        }
    }
    
    result.isValid = true;
    result.schema = "eliza-character-1.0";
    return result;
}

// CharacterFileManager Implementation
CharacterFileManager::CharacterFileManager() {
    // Register default JSON handler
    registerHandler(std::make_shared<JsonCharacterFileHandler>());
}

CharacterFileManager::~CharacterFileManager() {
    handlers_.clear();
}

void CharacterFileManager::registerHandler(std::shared_ptr<CharacterFileHandler> handler) {
    if (handler) {
        handlers_[handler->getSupportedFormat()] = handler;
    }
}

void CharacterFileManager::unregisterHandler(CharacterFileFormat format) {
    handlers_.erase(format);
}

std::vector<CharacterFileFormat> CharacterFileManager::getSupportedFormats() const {
    std::vector<CharacterFileFormat> formats;
    for (const auto& [format, handler] : handlers_) {
        formats.push_back(format);
    }
    return formats;
}

std::optional<CharacterProfile> CharacterFileManager::loadCharacterFromFile(
    const std::string& filepath,
    const CharacterFileOptions& options) {
    
    auto handler = getHandlerForFile(filepath);
    if (!handler) {
        notifyEvent("error", filepath, "No handler available for file format");
        return std::nullopt;
    }
    
    notifyEvent("loading", filepath);
    auto result = handler->loadFromFile(filepath, options);
    
    if (result) {
        notifyEvent("loaded", filepath);
    } else {
        notifyEvent("load_failed", filepath);
    }
    
    return result;
}

bool CharacterFileManager::saveCharacterToFile(
    const CharacterProfile& character,
    const std::string& filepath,
    const CharacterFileOptions& options) {
    
    CharacterFileFormat format = detectFileFormat(filepath);
    if (format == CharacterFileFormat::UNKNOWN) {
        format = options.outputFormat;
    }
    
    auto handler = getHandler(format);
    if (!handler) {
        notifyEvent("error", filepath, "No handler available for format");
        return false;
    }
    
    notifyEvent("saving", filepath);
    bool result = handler->saveToFile(character, filepath, options);
    
    if (result) {
        notifyEvent("saved", filepath);
    } else {
        notifyEvent("save_failed", filepath);
    }
    
    return result;
}

std::vector<CharacterProfile> CharacterFileManager::loadCharactersFromDirectory(
    const std::string& directoryPath,
    bool recursive,
    const CharacterFileOptions& options) {
    
    std::vector<CharacterProfile> characters;
    
    try {
        if (!std::filesystem::exists(directoryPath)) {
            return characters;
        }
        
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
                if (entry.is_regular_file() && isCharacterFile(entry.path().string())) {
                    auto character = loadCharacterFromFile(entry.path().string(), options);
                    if (character) {
                        characters.push_back(*character);
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_regular_file() && isCharacterFile(entry.path().string())) {
                    auto character = loadCharacterFromFile(entry.path().string(), options);
                    if (character) {
                        characters.push_back(*character);
                    }
                }
            }
        }
    } catch (const std::exception& e) {
        notifyEvent("error", directoryPath, "Directory scan failed: " + std::string(e.what()));
    }
    
    return characters;
}

bool CharacterFileManager::saveCharactersToDirectory(
    const std::vector<CharacterProfile>& characters,
    const std::string& directoryPath,
    const CharacterFileOptions& options) {
    
    try {
        // Ensure directory exists
        if (!characterfile_utils::ensureDirectoryExists(directoryPath)) {
            return false;
        }
        
        bool allSucceeded = true;
        
        for (const auto& character : characters) {
            std::string filename = character.name + ".json";
            filename = characterfile_utils::generateUniqueFilename(directoryPath, character.name, ".json");
            
            std::string filepath = std::filesystem::path(directoryPath) / filename;
            if (!saveCharacterToFile(character, filepath, options)) {
                allSucceeded = false;
            }
        }
        
        return allSucceeded;
        
    } catch (const std::exception& e) {
        notifyEvent("error", directoryPath, "Batch save failed: " + std::string(e.what()));
        return false;
    }
}

std::vector<CharacterFileInfo> CharacterFileManager::discoverCharacterFiles(
    const std::string& directoryPath,
    bool recursive) {
    
    std::vector<CharacterFileInfo> files;
    
    try {
        if (!std::filesystem::exists(directoryPath)) {
            return files;
        }
        
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
                if (entry.is_regular_file() && isCharacterFile(entry.path().string())) {
                    CharacterFileInfo info = getCharacterFileInfo(entry.path().string());
                    files.push_back(info);
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
                if (entry.is_regular_file() && isCharacterFile(entry.path().string())) {
                    CharacterFileInfo info = getCharacterFileInfo(entry.path().string());
                    files.push_back(info);
                }
            }
        }
    } catch (const std::exception& e) {
        notifyEvent("error", directoryPath, "Discovery failed: " + std::string(e.what()));
    }
    
    return files;
}

CharacterFileValidation CharacterFileManager::validateCharacterFile(
    const std::string& filepath,
    const CharacterFileOptions& options) {
    
    auto handler = getHandlerForFile(filepath);
    if (!handler) {
        return CharacterFileValidation(false, "No handler available for file format");
    }
    
    return handler->validateFile(filepath, options);
}

CharacterFileFormat CharacterFileManager::detectFileFormat(const std::string& filepath) {
    std::string extension = getFileExtension(filepath);
    
    if (extension == ".json" || extension == ".jsonc") {
        return CharacterFileFormat::JSON;
    }
    if (extension == ".yaml" || extension == ".yml") {
        return CharacterFileFormat::YAML;
    }
    if (extension == ".xml") {
        return CharacterFileFormat::XML;
    }
    if (extension == ".toml") {
        return CharacterFileFormat::TOML;
    }
    
    return CharacterFileFormat::UNKNOWN;
}

CharacterFileFormat CharacterFileManager::detectContentFormat(const std::string& content) {
    // Simple content-based detection
    std::string trimmed = content;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    
    if (trimmed.empty()) {
        return CharacterFileFormat::UNKNOWN;
    }
    
    if (trimmed[0] == '{') {
        return CharacterFileFormat::JSON;
    }
    
    if (trimmed.find("<?xml") != std::string::npos) {
        return CharacterFileFormat::XML;
    }
    
    // YAML often starts with keys or ---
    if (trimmed.find("---") == 0 || std::regex_match(trimmed.substr(0, 20), std::regex("^[a-zA-Z_][a-zA-Z0-9_]*:"))) {
        return CharacterFileFormat::YAML;
    }
    
    return CharacterFileFormat::UNKNOWN;
}

CharacterFileInfo CharacterFileManager::getCharacterFileInfo(const std::string& filepath) {
    CharacterFileInfo info;
    
    try {
        std::filesystem::path path(filepath);
        
        info.filename = path.filename().string();
        info.fullPath = std::filesystem::absolute(path).string();
        info.format = detectFileFormat(filepath);
        
        if (std::filesystem::exists(filepath)) {
            info.fileSize = std::filesystem::file_size(filepath);
            
            // Get last modified time
            auto ftime = std::filesystem::last_write_time(filepath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
            
            std::stringstream ss;
            ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
            info.lastModified = ss.str();
            
            info.checksum = generateChecksum(filepath);
        }
        
        // Validate file
        auto validation = validateCharacterFile(filepath);
        info.isValid = validation.isValid;
        info.schema = validation.schema;
        
        // Try to extract character info
        auto character = loadCharacterFromFile(filepath);
        if (character) {
            info.characterName = character->name;
            info.characterId = character->id;
            info.version = character->version;
        }
        
    } catch (const std::exception& e) {
        info.isValid = false;
    }
    
    return info;
}

std::string CharacterFileManager::generateTemplate(
    CharacterFileFormat format,
    const std::string& characterName) {
    
    if (format == CharacterFileFormat::JSON) {
        return R"({
  "name": ")" + characterName + R"(",
  "description": "A new character for ElizaOS",
  "personality": {
    "traits": [],
    "communication_style": {
      "tone": "neutral",
      "formality": 0.5
    }
  },
  "background": {
    "summary": "Character background summary",
    "details": []
  },
  "capabilities": [],
  "knowledge_domains": [],
  "behavioral_patterns": {},
  "metadata": {
    "version": "1.0.0",
    "created": ")" + std::to_string(std::time(nullptr)) + R"(",
    "schema": "eliza-character-1.0"
  }
})";
    }
    
    return ""; // Other formats not implemented yet
}

std::shared_ptr<CharacterFileHandler> CharacterFileManager::getHandler(CharacterFileFormat format) {
    auto it = handlers_.find(format);
    return (it != handlers_.end()) ? it->second : nullptr;
}

std::shared_ptr<CharacterFileHandler> CharacterFileManager::getHandlerForFile(const std::string& filepath) {
    CharacterFileFormat format = detectFileFormat(filepath);
    return getHandler(format);
}

void CharacterFileManager::notifyEvent(const std::string& event, const std::string& filepath, const std::string& details) {
    if (eventCallback_) {
        eventCallback_(event, filepath, details);
    }
}

std::string CharacterFileManager::generateChecksum(const std::string& filepath) {
    // Simple checksum based on file size and name for now
    std::hash<std::string> hasher;
    try {
        size_t fileSize = std::filesystem::file_size(filepath);
        return std::to_string(hasher(filepath + std::to_string(fileSize)));
    } catch (...) {
        return "0";
    }
}

bool CharacterFileManager::isCharacterFile(const std::string& filepath) {
    std::string extension = getFileExtension(filepath);
    return extension == ".json" || extension == ".yaml" || extension == ".yml" || 
           extension == ".xml" || extension == ".toml" || extension == ".jsonc";
}

std::string CharacterFileManager::getFileExtension(const std::string& filepath) {
    std::filesystem::path path(filepath);
    return path.extension().string();
}

// Utility functions implementation
namespace characterfile_utils {

std::string formatToString(CharacterFileFormat format) {
    switch (format) {
        case CharacterFileFormat::JSON: return "json";
        case CharacterFileFormat::YAML: return "yaml";
        case CharacterFileFormat::XML: return "xml";
        case CharacterFileFormat::TOML: return "toml";
        case CharacterFileFormat::BINARY: return "binary";
        default: return "unknown";
    }
}

CharacterFileFormat formatFromString(const std::string& str) {
    if (str == "json") return CharacterFileFormat::JSON;
    if (str == "yaml") return CharacterFileFormat::YAML;
    if (str == "xml") return CharacterFileFormat::XML;
    if (str == "toml") return CharacterFileFormat::TOML;
    if (str == "binary") return CharacterFileFormat::BINARY;
    return CharacterFileFormat::UNKNOWN;
}

bool isValidCharacterName(const std::string& name) {
    if (name.empty() || name.length() > 64) {
        return false;
    }
    
    // Character names should be alphanumeric with spaces, hyphens, and underscores
    std::regex namePattern("^[a-zA-Z0-9 _-]+$");
    return std::regex_match(name, namePattern);
}

bool ensureDirectoryExists(const std::string& path) {
    try {
        return std::filesystem::create_directories(path) || std::filesystem::exists(path);
    } catch (...) {
        return false;
    }
}

std::string generateUniqueFilename(const std::string& basePath, const std::string& name, const std::string& extension) {
    std::filesystem::path dir(basePath);
    std::string cleanName = name;
    
    // Clean the name
    std::regex invalidChars("[^a-zA-Z0-9_-]");
    cleanName = std::regex_replace(cleanName, invalidChars, "_");
    
    std::string filename = cleanName + extension;
    std::filesystem::path fullPath = dir / filename;
    
    int counter = 1;
    while (std::filesystem::exists(fullPath)) {
        filename = cleanName + "_" + std::to_string(counter) + extension;
        fullPath = dir / filename;
        counter++;
    }
    
    return filename;
}

std::vector<std::string> findCharacterFiles(const std::string& directory, bool recursive) {
    std::vector<std::string> files;
    
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".json" || ext == ".yaml" || ext == ".yml" || ext == ".xml" || ext == ".toml") {
                        files.push_back(entry.path().string());
                    }
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".json" || ext == ".yaml" || ext == ".yml" || ext == ".xml" || ext == ".toml") {
                        files.push_back(entry.path().string());
                    }
                }
            }
        }
    } catch (...) {
        // Directory doesn't exist or access error
    }
    
    return files;
}

} // namespace characterfile_utils

} // namespace elizaos
