#include "elizaos/elizas_list.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace elizaos {

// JSON serialization implementations
void to_json(nlohmann::json& j, const Author& a) {
    j = nlohmann::json{
        {"name", a.name},
        {"github", a.github}
    };
    if (a.twitter.has_value()) {
        j["twitter"] = a.twitter.value();
    }
}

void from_json(const nlohmann::json& j, Author& a) {
    j.at("name").get_to(a.name);
    j.at("github").get_to(a.github);
    if (j.contains("twitter") && !j["twitter"].is_null()) {
        a.twitter = j["twitter"].get<std::string>();
    }
}

void to_json(nlohmann::json& j, const Donation& d) {
    j = nlohmann::json{
        {"transactionHash", d.transactionHash},
        {"amount", d.amount},
        {"date", d.date}
    };
}

void from_json(const nlohmann::json& j, Donation& d) {
    j.at("transactionHash").get_to(d.transactionHash);
    j.at("amount").get_to(d.amount);
    j.at("date").get_to(d.date);
}

void to_json(nlohmann::json& j, const Metrics& m) {
    j = nlohmann::json{
        {"stars", m.stars},
        {"forks", m.forks}
    };
}

void from_json(const nlohmann::json& j, Metrics& m) {
    j.at("stars").get_to(m.stars);
    j.at("forks").get_to(m.forks);
}

void to_json(nlohmann::json& j, const Project& p) {
    j = nlohmann::json{
        {"id", p.id},
        {"name", p.name},
        {"description", p.description},
        {"projectUrl", p.projectUrl},
        {"github", p.github},
        {"image", p.image},
        {"author", p.author},
        {"donation", p.donation},
        {"tags", p.tags},
        {"addedOn", p.addedOn}
    };
    if (p.metrics.has_value()) {
        j["metrics"] = p.metrics.value();
    }
}

void from_json(const nlohmann::json& j, Project& p) {
    j.at("id").get_to(p.id);
    j.at("name").get_to(p.name);
    j.at("description").get_to(p.description);
    j.at("projectUrl").get_to(p.projectUrl);
    j.at("github").get_to(p.github);
    j.at("image").get_to(p.image);
    j.at("author").get_to(p.author);
    j.at("donation").get_to(p.donation);
    j.at("tags").get_to(p.tags);
    j.at("addedOn").get_to(p.addedOn);
    if (j.contains("metrics") && !j["metrics"].is_null()) {
        p.metrics = j["metrics"].get<Metrics>();
    }
}

void to_json(nlohmann::json& j, const Curator& c) {
    j = nlohmann::json{
        {"name", c.name},
        {"github", c.github}
    };
}

void from_json(const nlohmann::json& j, Curator& c) {
    j.at("name").get_to(c.name);
    j.at("github").get_to(c.github);
}

void to_json(nlohmann::json& j, const Collection& c) {
    j = nlohmann::json{
        {"id", c.id},
        {"name", c.name},
        {"description", c.description},
        {"projects", c.projects},
        {"curator", c.curator},
        {"featured", c.featured}
    };
}

void from_json(const nlohmann::json& j, Collection& c) {
    j.at("id").get_to(c.id);
    j.at("name").get_to(c.name);
    j.at("description").get_to(c.description);
    j.at("projects").get_to(c.projects);
    j.at("curator").get_to(c.curator);
    j.at("featured").get_to(c.featured);
}

// Project management implementation
bool ElizasList::addProject(const Project& project) {
    // Check if project with same ID already exists
    if (findProject(project.id) != projects_.end()) {
        return false;
    }
    
    projects_.push_back(project);
    return true;
}

bool ElizasList::removeProject(const std::string& projectId) {
    auto it = findProject(projectId);
    if (it != projects_.end()) {
        projects_.erase(it);
        return true;
    }
    return false;
}

std::optional<Project> ElizasList::getProject(const std::string& projectId) const {
    auto it = findProject(projectId);
    if (it != projects_.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<Project> ElizasList::getAllProjects() const {
    return projects_;
}

std::vector<Project> ElizasList::getProjectsByTag(const std::string& tag) const {
    std::vector<Project> result;
    std::copy_if(projects_.begin(), projects_.end(), std::back_inserter(result),
                 [&tag](const Project& project) {
                     return std::find(project.tags.begin(), project.tags.end(), tag) != project.tags.end();
                 });
    return result;
}

std::vector<Project> ElizasList::getProjectsByAuthor(const std::string& authorGithub) const {
    std::vector<Project> result;
    std::copy_if(projects_.begin(), projects_.end(), std::back_inserter(result),
                 [&authorGithub](const Project& project) {
                     return project.author.github == authorGithub;
                 });
    return result;
}

bool ElizasList::updateProject(const Project& project) {
    auto it = findProject(project.id);
    if (it != projects_.end()) {
        *it = project;
        return true;
    }
    return false;
}

// Collection management implementation
bool ElizasList::addCollection(const Collection& collection) {
    // Check if collection with same ID already exists
    if (findCollection(collection.id) != collections_.end()) {
        return false;
    }
    
    collections_.push_back(collection);
    return true;
}

bool ElizasList::removeCollection(const std::string& collectionId) {
    auto it = findCollection(collectionId);
    if (it != collections_.end()) {
        collections_.erase(it);
        return true;
    }
    return false;
}

std::optional<Collection> ElizasList::getCollection(const std::string& collectionId) const {
    auto it = findCollection(collectionId);
    if (it != collections_.end()) {
        return *it;
    }
    return std::nullopt;
}

std::vector<Collection> ElizasList::getAllCollections() const {
    return collections_;
}

std::vector<Collection> ElizasList::getFeaturedCollections() const {
    std::vector<Collection> result;
    std::copy_if(collections_.begin(), collections_.end(), std::back_inserter(result),
                 [](const Collection& collection) {
                     return collection.featured;
                 });
    return result;
}

bool ElizasList::updateCollection(const Collection& collection) {
    auto it = findCollection(collection.id);
    if (it != collections_.end()) {
        *it = collection;
        return true;
    }
    return false;
}

// Project search and filtering implementation
std::vector<Project> ElizasList::searchProjects(const std::string& query) const {
    std::vector<Project> result;
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    std::copy_if(projects_.begin(), projects_.end(), std::back_inserter(result),
                 [&lowerQuery](const Project& project) {
                     std::string lowerName = project.name;
                     std::string lowerDesc = project.description;
                     std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                     std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
                     
                     return lowerName.find(lowerQuery) != std::string::npos ||
                            lowerDesc.find(lowerQuery) != std::string::npos;
                 });
    return result;
}

std::vector<Project> ElizasList::getProjectsSortedByStars() const {
    std::vector<Project> result = projects_;
    std::sort(result.begin(), result.end(),
              [](const Project& a, const Project& b) {
                  int starsA = a.metrics ? a.metrics->stars : 0;
                  int starsB = b.metrics ? b.metrics->stars : 0;
                  return starsA > starsB;
              });
    return result;
}

std::vector<Project> ElizasList::getRecentProjects(int limit) const {
    std::vector<Project> result = projects_;
    // Sort by addedOn date (descending - most recent first)
    std::sort(result.begin(), result.end(),
              [](const Project& a, const Project& b) {
                  return a.addedOn > b.addedOn;
              });
    
    if (static_cast<size_t>(limit) < result.size()) {
        result.resize(limit);
    }
    return result;
}

// Data persistence implementation
bool ElizasList::loadFromJson(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        nlohmann::json json;
        file >> json;
        
        if (json.contains("projects")) {
            projects_ = json["projects"].get<std::vector<Project>>();
        }
        
        if (json.contains("collections")) {
            collections_ = json["collections"].get<std::vector<Collection>>();
        }
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ElizasList::saveToJson(const std::string& filePath) const {
    try {
        nlohmann::json json;
        json["projects"] = projects_;
        json["collections"] = collections_;
        
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }
        
        file << json.dump(2); // Pretty print with 2 spaces
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool ElizasList::loadProjectsFromJson(const std::string& jsonData) {
    try {
        nlohmann::json json = nlohmann::json::parse(jsonData);
        
        if (json.contains("projects")) {
            projects_ = json["projects"].get<std::vector<Project>>();
            return true;
        }
        return false;
    } catch (const std::exception&) {
        return false;
    }
}

std::string ElizasList::exportProjectsToJson() const {
    try {
        nlohmann::json json;
        json["projects"] = projects_;
        return json.dump(2);
    } catch (const std::exception&) {
        return "";
    }
}

// Statistics implementation
size_t ElizasList::getProjectCount() const {
    return projects_.size();
}

size_t ElizasList::getCollectionCount() const {
    return collections_.size();
}

std::vector<std::string> ElizasList::getAllTags() const {
    std::vector<std::string> allTags;
    
    for (const auto& project : projects_) {
        for (const auto& tag : project.tags) {
            if (std::find(allTags.begin(), allTags.end(), tag) == allTags.end()) {
                allTags.push_back(tag);
            }
        }
    }
    
    std::sort(allTags.begin(), allTags.end());
    return allTags;
}

// Helper methods implementation
std::vector<Project>::iterator ElizasList::findProject(const std::string& projectId) {
    return std::find_if(projects_.begin(), projects_.end(),
                        [&projectId](const Project& project) {
                            return project.id == projectId;
                        });
}

std::vector<Project>::const_iterator ElizasList::findProject(const std::string& projectId) const {
    return std::find_if(projects_.begin(), projects_.end(),
                        [&projectId](const Project& project) {
                            return project.id == projectId;
                        });
}

std::vector<Collection>::iterator ElizasList::findCollection(const std::string& collectionId) {
    return std::find_if(collections_.begin(), collections_.end(),
                        [&collectionId](const Collection& collection) {
                            return collection.id == collectionId;
                        });
}

std::vector<Collection>::const_iterator ElizasList::findCollection(const std::string& collectionId) const {
    return std::find_if(collections_.begin(), collections_.end(),
                        [&collectionId](const Collection& collection) {
                            return collection.id == collectionId;
                        });
}

} // namespace elizaos
