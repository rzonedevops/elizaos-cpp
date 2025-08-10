#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>

namespace elizaos {

/**
 * @brief Represents author information for a project
 */
struct Author {
    std::string name;
    std::string github;
    std::optional<std::string> twitter;
};

/**
 * @brief Represents donation information for a project
 */
struct Donation {
    std::string transactionHash;
    std::string amount;
    std::string date; // ISO 8601 format
};

/**
 * @brief Represents project metrics (GitHub stars, forks, etc.)
 */
struct Metrics {
    int stars = 0;
    int forks = 0;
};

/**
 * @brief Represents a project in Eliza's List
 */
struct Project {
    std::string id;
    std::string name;
    std::string description;
    std::string projectUrl;
    std::string github;
    std::string image;
    Author author;
    Donation donation;
    std::vector<std::string> tags;
    std::string addedOn; // ISO 8601 format
    std::optional<Metrics> metrics;
};

/**
 * @brief Represents a curator for a collection
 */
struct Curator {
    std::string name;
    std::string github;
};

/**
 * @brief Represents a collection of projects
 */
struct Collection {
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> projects; // Project IDs
    Curator curator;
    bool featured = false;
};

// JSON serialization functions
void to_json(nlohmann::json& j, const Author& a);
void from_json(const nlohmann::json& j, Author& a);
void to_json(nlohmann::json& j, const Donation& d);
void from_json(const nlohmann::json& j, Donation& d);
void to_json(nlohmann::json& j, const Metrics& m);
void from_json(const nlohmann::json& j, Metrics& m);
void to_json(nlohmann::json& j, const Project& p);
void from_json(const nlohmann::json& j, Project& p);
void to_json(nlohmann::json& j, const Curator& c);
void from_json(const nlohmann::json& j, Curator& c);
void to_json(nlohmann::json& j, const Collection& c);
void from_json(const nlohmann::json& j, Collection& c);

/**
 * @brief Main class for managing Eliza's List projects and collections
 */
class ElizasList {
public:
    ElizasList() = default;
    ~ElizasList() = default;

    // Project management
    bool addProject(const Project& project);
    bool removeProject(const std::string& projectId);
    std::optional<Project> getProject(const std::string& projectId) const;
    std::vector<Project> getAllProjects() const;
    std::vector<Project> getProjectsByTag(const std::string& tag) const;
    std::vector<Project> getProjectsByAuthor(const std::string& authorGithub) const;
    bool updateProject(const Project& project);

    // Collection management
    bool addCollection(const Collection& collection);
    bool removeCollection(const std::string& collectionId);
    std::optional<Collection> getCollection(const std::string& collectionId) const;
    std::vector<Collection> getAllCollections() const;
    std::vector<Collection> getFeaturedCollections() const;
    bool updateCollection(const Collection& collection);

    // Project search and filtering
    std::vector<Project> searchProjects(const std::string& query) const;
    std::vector<Project> getProjectsSortedByStars() const;
    std::vector<Project> getRecentProjects(int limit = 10) const;

    // Data persistence
    bool loadFromJson(const std::string& filePath);
    bool saveToJson(const std::string& filePath) const;
    bool loadProjectsFromJson(const std::string& jsonData);
    std::string exportProjectsToJson() const;

    // Statistics
    size_t getProjectCount() const;
    size_t getCollectionCount() const;
    std::vector<std::string> getAllTags() const;

private:
    std::vector<Project> projects_;
    std::vector<Collection> collections_;

    // Helper methods
    std::vector<Project>::iterator findProject(const std::string& projectId);
    std::vector<Project>::const_iterator findProject(const std::string& projectId) const;
    std::vector<Collection>::iterator findCollection(const std::string& collectionId);
    std::vector<Collection>::const_iterator findCollection(const std::string& collectionId) const;
};

} // namespace elizaos