#pragma once

#include "elizaos/characters.hpp"
#include "elizaos/character_json_loader.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <filesystem>
#include <functional>

namespace elizaos {

/**
 * Character file format types supported by the system
 */
enum class CharacterFileFormat {
    JSON,           // Standard ElizaOS JSON format
    YAML,           // YAML format for easier editing
    XML,            // XML format for structured data
    TOML,           // TOML format for configuration-like characters
    BINARY,         // Binary format for optimized storage
    UNKNOWN
};

/**
 * Character file validation results
 */
struct CharacterFileValidation {
    bool isValid = false;
    std::string errorMessage;
    std::vector<std::string> warnings;
    CharacterFileFormat detectedFormat = CharacterFileFormat::UNKNOWN;
    std::string schema = "";
    
    CharacterFileValidation() = default;
    CharacterFileValidation(bool valid, const std::string& error = "")
        : isValid(valid), errorMessage(error) {}
};

/**
 * Character file metadata
 */
struct CharacterFileInfo {
    std::string filename;
    std::string fullPath;
    CharacterFileFormat format;
    size_t fileSize;
    std::string lastModified;
    std::string checksum;
    bool isValid;
    std::string schema;
    
    // Quick character info extracted from file
    std::string characterName;
    std::string characterId;
    std::string version;
    std::vector<std::string> tags;
};

/**
 * Character import/export options
 */
struct CharacterFileOptions {
    bool validateSchema = true;
    bool generateId = false;          // Generate new UUID if missing
    bool preserveMetadata = true;
    bool includePrivateFields = false;
    bool compressOutput = false;
    std::string encoding = "utf-8";
    CharacterFileFormat outputFormat = CharacterFileFormat::JSON;
    
    // Validation options
    bool strictValidation = false;
    bool allowDeprecatedFields = true;
    std::vector<std::string> requiredFields;
    std::vector<std::string> optionalFields;
};

/**
 * Character file handler interface for different formats
 */
class CharacterFileHandler {
public:
    virtual ~CharacterFileHandler() = default;
    
    virtual CharacterFileFormat getSupportedFormat() const = 0;
    virtual std::vector<std::string> getSupportedExtensions() const = 0;
    
    virtual std::optional<CharacterProfile> loadFromFile(
        const std::string& filepath, 
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const = 0;
    
    virtual bool saveToFile(
        const CharacterProfile& character, 
        const std::string& filepath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const = 0;
    
    virtual CharacterFileValidation validateFile(
        const std::string& filepath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const = 0;
    
    virtual std::string toString(
        const CharacterProfile& character,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const = 0;
    
    virtual std::optional<CharacterProfile> fromString(
        const std::string& content,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const = 0;
};

/**
 * JSON format handler
 */
class JsonCharacterFileHandler : public CharacterFileHandler {
public:
    CharacterFileFormat getSupportedFormat() const override { return CharacterFileFormat::JSON; }
    std::vector<std::string> getSupportedExtensions() const override { return {".json", ".jsonc"}; }
    
    std::optional<CharacterProfile> loadFromFile(
        const std::string& filepath, 
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const override;
    
    bool saveToFile(
        const CharacterProfile& character, 
        const std::string& filepath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const override;
    
    CharacterFileValidation validateFile(
        const std::string& filepath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const override;
    
    std::string toString(
        const CharacterProfile& character,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const override;
    
    std::optional<CharacterProfile> fromString(
        const std::string& content,
        const CharacterFileOptions& options = CharacterFileOptions{}
    ) const override;
    
private:
    CharacterFileValidation validateJsonSchema(const std::string& content) const;
};

/**
 * Main character file manager
 */
class CharacterFileManager {
public:
    CharacterFileManager();
    ~CharacterFileManager();
    
    // Handler registration
    void registerHandler(std::shared_ptr<CharacterFileHandler> handler);
    void unregisterHandler(CharacterFileFormat format);
    std::vector<CharacterFileFormat> getSupportedFormats() const;
    
    // File operations
    std::optional<CharacterProfile> loadCharacterFromFile(
        const std::string& filepath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    );
    
    bool saveCharacterToFile(
        const CharacterProfile& character,
        const std::string& filepath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    );
    
    // Batch operations
    std::vector<CharacterProfile> loadCharactersFromDirectory(
        const std::string& directoryPath,
        bool recursive = false,
        const CharacterFileOptions& options = CharacterFileOptions{}
    );
    
    bool saveCharactersToDirectory(
        const std::vector<CharacterProfile>& characters,
        const std::string& directoryPath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    );
    
    // File discovery and management
    std::vector<CharacterFileInfo> discoverCharacterFiles(
        const std::string& directoryPath,
        bool recursive = false
    );
    
    // Validation
    CharacterFileValidation validateCharacterFile(
        const std::string& filepath,
        const CharacterFileOptions& options = CharacterFileOptions{}
    );
    
    std::vector<CharacterFileValidation> validateDirectory(
        const std::string& directoryPath,
        bool recursive = false,
        const CharacterFileOptions& options = CharacterFileOptions{}
    );
    
    // Format detection and conversion
    CharacterFileFormat detectFileFormat(const std::string& filepath);
    CharacterFileFormat detectContentFormat(const std::string& content);
    
    bool convertCharacterFile(
        const std::string& inputPath,
        const std::string& outputPath,
        CharacterFileFormat outputFormat,
        const CharacterFileOptions& options = CharacterFileOptions{}
    );
    
    // Metadata extraction
    CharacterFileInfo getCharacterFileInfo(const std::string& filepath);
    std::unordered_map<std::string, std::string> extractMetadata(const std::string& filepath);
    
    // Character file templates
    std::string generateTemplate(
        CharacterFileFormat format,
        const std::string& characterName = "NewCharacter"
    );
    
    bool createCharacterFileFromTemplate(
        const std::string& templateName,
        const std::string& characterName,
        const std::string& outputPath,
        const std::unordered_map<std::string, std::string>& placeholders = {}
    );
    
    // Backup and versioning
    bool createBackup(const std::string& filepath);
    std::vector<std::string> getBackupVersions(const std::string& filepath);
    bool restoreFromBackup(const std::string& filepath, const std::string& backupVersion = "");
    
    // Event callbacks
    using FileEventCallback = std::function<void(const std::string& event, const std::string& filepath, const std::string& details)>;
    void setFileEventCallback(FileEventCallback callback) { eventCallback_ = callback; }

private:
    std::unordered_map<CharacterFileFormat, std::shared_ptr<CharacterFileHandler>> handlers_;
    FileEventCallback eventCallback_;
    
    // Helper methods
    std::shared_ptr<CharacterFileHandler> getHandler(CharacterFileFormat format);
    std::shared_ptr<CharacterFileHandler> getHandlerForFile(const std::string& filepath);
    void notifyEvent(const std::string& event, const std::string& filepath, const std::string& details = "");
    
    std::string generateChecksum(const std::string& filepath);
    std::string formatFileSize(size_t bytes);
    bool isCharacterFile(const std::string& filepath);
    std::string getFileExtension(const std::string& filepath);
};

/**
 * Character file utility functions
 */
namespace characterfile_utils {
    
    // Format utilities
    std::string formatToString(CharacterFileFormat format);
    CharacterFileFormat formatFromString(const std::string& str);
    std::vector<std::string> getExtensionsForFormat(CharacterFileFormat format);
    
    // Validation utilities
    bool isValidCharacterName(const std::string& name);
    bool isValidCharacterFilename(const std::string& filename);
    std::vector<std::string> sanitizeCharacterData(const CharacterProfile& character);
    
    // File system utilities
    bool ensureDirectoryExists(const std::string& path);
    std::string generateUniqueFilename(const std::string& basePath, const std::string& name, const std::string& extension);
    std::vector<std::string> findCharacterFiles(const std::string& directory, bool recursive = false);
    
    // Schema validation
    std::string getDefaultJsonSchema();
    bool validateAgainstSchema(const std::string& content, const std::string& schema);
    std::vector<std::string> extractSchemaErrors(const std::string& content, const std::string& schema);
    
    // Content analysis
    std::unordered_map<std::string, int> analyzeCharacterComplexity(const CharacterProfile& character);
    std::vector<std::string> detectPotentialIssues(const CharacterProfile& character);
    double calculateCharacterSimilarity(const CharacterProfile& char1, const CharacterProfile& char2);
}

} // namespace elizaos