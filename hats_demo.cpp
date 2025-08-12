#include <iostream>
#include <fstream>
#include "elizaos/hats.hpp"

using namespace elizaos;

int main() {
    std::cout << "=== ElizaOS HATs Protocol Demo ===" << std::endl;
    
    // Create HATs manager
    HatsManager manager;
    
    // Create some test data files
    std::cout << "\n1. Creating test data files..." << std::endl;
    
    // Create CSV test file
    std::ofstream csvFile("/tmp/demo_data.csv");
    csvFile << "product,price,category\n";
    csvFile << "Laptop,999.99,Electronics\n";
    csvFile << "Book,29.99,Education\n";
    csvFile << "Coffee,4.50,Food\n";
    csvFile.close();
    std::cout << "   - Created CSV file with product data" << std::endl;
    
    // Create JSON test file
    std::ofstream jsonFile("/tmp/demo_reviews.json");
    jsonFile << "{\"product\": \"Laptop\", \"rating\": \"5\", \"comment\": \"Excellent\"}\n";
    jsonFile << "{\"product\": \"Book\", \"rating\": \"4\", \"comment\": \"Good read\"}\n";
    jsonFile.close();
    std::cout << "   - Created JSON file with review data" << std::endl;
    
    // Register CSV data source
    std::cout << "\n2. Registering data sources..." << std::endl;
    DataSourceConfig csvConfig;
    csvConfig.id = "products";
    csvConfig.type = DataSourceType::CSV;
    csvConfig.location = "/tmp/demo_data.csv";
    csvConfig.parameters["hasHeader"] = "true";
    csvConfig.parameters["delimiter"] = ",";
    
    auto csvSource = hats_utils::createDataSource(csvConfig);
    if (manager.registerDataSource(std::move(csvSource)) == HatsStatus::SUCCESS) {
        std::cout << "   - Registered CSV data source: products" << std::endl;
    }
    
    // Register JSON data source
    DataSourceConfig jsonConfig;
    jsonConfig.id = "reviews";
    jsonConfig.type = DataSourceType::JSON;
    jsonConfig.location = "/tmp/demo_reviews.json";
    
    auto jsonSource = hats_utils::createDataSource(jsonConfig);
    if (manager.registerDataSource(std::move(jsonSource)) == HatsStatus::SUCCESS) {
        std::cout << "   - Registered JSON data source: reviews" << std::endl;
    }
    
    std::cout << "   - Total registered sources: " << manager.getRegisteredSourceCount() << std::endl;
    
    // Load and display data from CSV source
    std::cout << "\n3. Loading data from CSV source..." << std::endl;
    DataSet products;
    if (manager.loadFromSource("products", products) == HatsStatus::SUCCESS) {
        std::cout << "   - Loaded " << products.size() << " products:" << std::endl;
        for (const auto& product : products) {
            std::cout << "     * " << hats_utils::dataValueToString(product.at("product")) 
                      << " - $" << hats_utils::dataValueToString(product.at("price"))
                      << " (" << hats_utils::dataValueToString(product.at("category")) << ")" << std::endl;
        }
    }
    
    // Load and display data from JSON source
    std::cout << "\n4. Loading data from JSON source..." << std::endl;
    DataSet reviews;
    if (manager.loadFromSource("reviews", reviews) == HatsStatus::SUCCESS) {
        std::cout << "   - Loaded " << reviews.size() << " reviews:" << std::endl;
        for (const auto& review : reviews) {
            std::cout << "     * " << hats_utils::dataValueToString(review.at("product"))
                      << " - Rating: " << hats_utils::dataValueToString(review.at("rating"))
                      << "/5 - " << hats_utils::dataValueToString(review.at("comment")) << std::endl;
        }
    }
    
    // Demonstrate data filtering
    std::cout << "\n5. Filtering expensive products (>$50)..." << std::endl;
    std::vector<ProcessingStep> filterSteps;
    ProcessingStep expensiveFilter;
    expensiveFilter.operation = ProcessingOperation::FILTER;
    expensiveFilter.condition = [](const DataRecord& record) {
        auto it = record.find("price");
        if (it != record.end()) {
            try {
                double price = std::get<double>(it->second);
                return price > 50.0;
            } catch (...) {
                // Handle case where price is stored as string
                std::string priceStr = hats_utils::dataValueToString(it->second);
                double price = std::stod(priceStr);
                return price > 50.0;
            }
        }
        return false;
    };
    filterSteps.push_back(expensiveFilter);
    
    DataSet expensiveProducts;
    if (manager.processData("products", filterSteps, expensiveProducts) == HatsStatus::SUCCESS) {
        std::cout << "   - Found " << expensiveProducts.size() << " expensive products:" << std::endl;
        for (const auto& product : expensiveProducts) {
            std::cout << "     * " << hats_utils::dataValueToString(product.at("product"))
                      << " - $" << hats_utils::dataValueToString(product.at("price")) << std::endl;
        }
    }
    
    // Merge data from multiple sources
    std::cout << "\n6. Merging data from multiple sources..." << std::endl;
    std::vector<std::string> sourceIds = {"products", "reviews"};
    DataSet mergedData;
    if (manager.loadFromMultipleSources(sourceIds, mergedData) == HatsStatus::SUCCESS) {
        std::cout << "   - Merged data contains " << mergedData.size() << " records total" << std::endl;
        std::cout << "   - First 3 merged records:" << std::endl;
        for (size_t i = 0; i < std::min(size_t(3), mergedData.size()); ++i) {
            std::cout << "     Record " << (i+1) << ": ";
            bool first = true;
            for (const auto& field : mergedData[i]) {
                if (!first) std::cout << ", ";
                std::cout << field.first << "=" << hats_utils::dataValueToString(field.second);
                first = false;
            }
            std::cout << std::endl;
        }
    }
    
    // Clean up
    std::cout << "\n7. Cleaning up..." << std::endl;
    manager.unregisterDataSource("products");
    manager.unregisterDataSource("reviews");
    std::cout << "   - Unregistered all data sources" << std::endl;
    
    // Remove test files
    std::remove("/tmp/demo_data.csv");
    std::remove("/tmp/demo_reviews.json");
    std::cout << "   - Removed test files" << std::endl;
    
    std::cout << "\n=== Demo completed successfully! ===" << std::endl;
    std::cout << "\nThe HATs protocol provides:" << std::endl;
    std::cout << "• Support for multiple data source types (CSV, JSON, etc.)" << std::endl;
    std::cout << "• Data source registration and management" << std::endl;
    std::cout << "• Data filtering and processing pipelines" << std::endl;
    std::cout << "• Merging data from multiple sources" << std::endl;
    std::cout << "• Type-safe data value handling" << std::endl;
    
    return 0;
}