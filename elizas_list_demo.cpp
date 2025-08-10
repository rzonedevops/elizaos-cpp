#include "elizaos/elizas_list.hpp"
#include <iostream>

using namespace elizaos;

int main() {
    std::cout << "=== ElizaOS ElizasList Demo ===" << std::endl;
    
    // Create an ElizasList instance
    ElizasList elizasList;
    
    // Create some sample data
    Author author1{"Shaw", "https://github.com/lalalun", "https://x.com/shawmakesmagic"};
    Donation donation1{"0x1234567890abcdef1234567890abcdef12345678", "10000000", "2024-10-31T00:00:00Z"};
    Metrics metrics1{342, 89};
    
    Project project1{
        "degen-spartan-ai",
        "Degen Spartan AI",
        "The First Eliza",
        "https://t.me/degenspartan",
        "https://github.com/ai16z/eliza",
        "/project-images/degenai.png",
        author1,
        donation1,
        {"AI", "Machine Learning"},
        "2024-03-21T00:00:00Z",
        metrics1
    };
    
    // Add the project to the list
    if (elizasList.addProject(project1)) {
        std::cout << "Successfully added project: " << project1.name << std::endl;
    } else {
        std::cout << "Failed to add project!" << std::endl;
    }
    
    // Create another project
    Author author2{"Alice", "https://github.com/alice", std::nullopt};
    Donation donation2{"0xabcdef1234567890abcdef1234567890abcdef12", "5000000", "2024-11-01T00:00:00Z"};
    
    Project project2{
        "awesome-chatbot",
        "Awesome Chatbot",
        "An intelligent chatbot powered by AI",
        "https://awesomechatbot.com",
        "https://github.com/alice/awesome-chatbot",
        "/project-images/chatbot.png",
        author2,
        donation2,
        {"AI", "Chatbot", "NLP"},
        "2024-11-01T00:00:00Z",
        std::nullopt
    };
    
    elizasList.addProject(project2);
    std::cout << "Added project: " << project2.name << std::endl;
    
    // Display statistics
    std::cout << "\n=== Statistics ===" << std::endl;
    std::cout << "Total projects: " << elizasList.getProjectCount() << std::endl;
    std::cout << "Total collections: " << elizasList.getCollectionCount() << std::endl;
    
    // Show all tags
    auto tags = elizasList.getAllTags();
    std::cout << "Available tags: ";
    for (const auto& tag : tags) {
        std::cout << tag << " ";
    }
    std::cout << std::endl;
    
    // Search for projects
    std::cout << "\n=== Project Search ===" << std::endl;
    auto aiProjects = elizasList.getProjectsByTag("AI");
    std::cout << "Projects with 'AI' tag: " << aiProjects.size() << std::endl;
    for (const auto& proj : aiProjects) {
        std::cout << "  - " << proj.name << std::endl;
    }
    
    // Search by author
    auto shawProjects = elizasList.getProjectsByAuthor("https://github.com/lalalun");
    std::cout << "Projects by Shaw: " << shawProjects.size() << std::endl;
    for (const auto& proj : shawProjects) {
        std::cout << "  - " << proj.name << std::endl;
    }
    
    // Text search
    auto searchResults = elizasList.searchProjects("chatbot");
    std::cout << "Search results for 'chatbot': " << searchResults.size() << std::endl;
    for (const auto& proj : searchResults) {
        std::cout << "  - " << proj.name << std::endl;
    }
    
    // Sort by stars
    auto sortedProjects = elizasList.getProjectsSortedByStars();
    std::cout << "\n=== Projects sorted by stars ===" << std::endl;
    for (const auto& proj : sortedProjects) {
        int stars = proj.metrics ? proj.metrics->stars : 0;
        std::cout << "  - " << proj.name << " (" << stars << " stars)" << std::endl;
    }
    
    // Test JSON export/import
    std::cout << "\n=== JSON Export Test ===" << std::endl;
    std::string jsonExport = elizasList.exportProjectsToJson();
    std::cout << "Exported JSON (first 200 chars):" << std::endl;
    std::cout << jsonExport.substr(0, 200) << "..." << std::endl;
    
    // Test file I/O
    const std::string testFile = "/tmp/elizas_list_test.json";
    if (elizasList.saveToJson(testFile)) {
        std::cout << "Successfully saved data to " << testFile << std::endl;
        
        // Test loading
        ElizasList newElizasList;
        if (newElizasList.loadFromJson(testFile)) {
            std::cout << "Successfully loaded data from file" << std::endl;
            std::cout << "Loaded " << newElizasList.getProjectCount() << " projects" << std::endl;
        } else {
            std::cout << "Failed to load data from file" << std::endl;
        }
    } else {
        std::cout << "Failed to save data to file" << std::endl;
    }
    
    // Test collection functionality
    std::cout << "\n=== Collection Test ===" << std::endl;
    Curator curator1{"ElizaOS Team", "https://github.com/ai16z"};
    Collection collection1{
        "ai-projects",
        "AI & Machine Learning Projects",
        "A curated collection of AI and ML projects",
        {"degen-spartan-ai", "awesome-chatbot"},
        curator1,
        true
    };
    
    if (elizasList.addCollection(collection1)) {
        std::cout << "Successfully added collection: " << collection1.name << std::endl;
    }
    
    auto featuredCollections = elizasList.getFeaturedCollections();
    std::cout << "Featured collections: " << featuredCollections.size() << std::endl;
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    return 0;
}