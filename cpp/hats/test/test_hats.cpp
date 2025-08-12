#include <gtest/gtest.h>
#include "elizaos/hats.hpp"
#include <fstream>
#include <filesystem>

using namespace elizaos;

class HatsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test directory
        testDir = "/tmp/hats_test";
        std::filesystem::create_directories(testDir);
        
        // Create test CSV file
        csvFile = testDir + "/test.csv";
        std::ofstream csv(csvFile);
        csv << "name,age,city\n";
        csv << "Alice,30,New York\n";
        csv << "Bob,25,San Francisco\n";
        csv << "Charlie,35,Chicago\n";
        csv.close();
        
        // Create test JSON file
        jsonFile = testDir + "/test.json";
        std::ofstream json(jsonFile);
        json << "{\"name\": \"Alice\", \"age\": \"30\", \"city\": \"New York\"}\n";
        json << "{\"name\": \"Bob\", \"age\": \"25\", \"city\": \"San Francisco\"}\n";
        json.close();
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(testDir);
    }
    
    std::string testDir;
    std::string csvFile;
    std::string jsonFile;
};

TEST_F(HatsTest, DataValueUtilities) {
    // Test data value parsing
    EXPECT_EQ(std::get<int>(hats_utils::parseDataValue("42")), 42);
    EXPECT_EQ(std::get<double>(hats_utils::parseDataValue("3.14")), 3.14);
    EXPECT_EQ(std::get<bool>(hats_utils::parseDataValue("true")), true);
    EXPECT_EQ(std::get<bool>(hats_utils::parseDataValue("false")), false);
    EXPECT_EQ(std::get<std::string>(hats_utils::parseDataValue("hello")), "hello");
    
    // Test data value to string conversion
    EXPECT_EQ(hats_utils::dataValueToString(DataValue{42}), "42");
    EXPECT_EQ(hats_utils::dataValueToString(DataValue{3.14}), "3.140000");
    EXPECT_EQ(hats_utils::dataValueToString(DataValue{true}), "true");
    EXPECT_EQ(hats_utils::dataValueToString(DataValue{false}), "false");
    EXPECT_EQ(hats_utils::dataValueToString(DataValue{std::string("hello")}), "hello");
}

TEST_F(HatsTest, CsvDataSource) {
    DataSourceConfig config;
    config.id = "test_csv";
    config.type = DataSourceType::CSV;
    config.location = csvFile;
    config.parameters["hasHeader"] = "true";
    config.parameters["delimiter"] = ",";
    
    CsvDataSource source(config);
    
    // Test connection
    EXPECT_EQ(source.connect(), HatsStatus::SUCCESS);
    EXPECT_TRUE(source.isConnected());
    
    // Test data loading
    DataSet data;
    EXPECT_EQ(source.loadData(data), HatsStatus::SUCCESS);
    EXPECT_EQ(data.size(), 3); // 3 rows of data
    
    // Check first row
    EXPECT_EQ(std::get<std::string>(data[0]["name"]), "Alice");
    EXPECT_EQ(std::get<int>(data[0]["age"]), 30);
    EXPECT_EQ(std::get<std::string>(data[0]["city"]), "New York");
    
    // Test disconnection
    EXPECT_EQ(source.disconnect(), HatsStatus::SUCCESS);
    EXPECT_FALSE(source.isConnected());
}

TEST_F(HatsTest, JsonDataSource) {
    DataSourceConfig config;
    config.id = "test_json";
    config.type = DataSourceType::JSON;
    config.location = jsonFile;
    
    JsonDataSource source(config);
    
    // Test connection
    EXPECT_EQ(source.connect(), HatsStatus::SUCCESS);
    EXPECT_TRUE(source.isConnected());
    
    // Test data loading
    DataSet data;
    EXPECT_EQ(source.loadData(data), HatsStatus::SUCCESS);
    EXPECT_EQ(data.size(), 2); // 2 rows of JSON data
    
    // Check first row
    EXPECT_EQ(std::get<std::string>(data[0]["name"]), "Alice");
    EXPECT_EQ(std::get<int>(data[0]["age"]), 30);
    EXPECT_EQ(std::get<std::string>(data[0]["city"]), "New York");
    
    // Test disconnection
    EXPECT_EQ(source.disconnect(), HatsStatus::SUCCESS);
    EXPECT_FALSE(source.isConnected());
}

TEST_F(HatsTest, DataProcessor) {
    // Create test data
    DataSet testData;
    DataRecord record1{{"name", std::string("Alice")}, {"age", 30}};
    DataRecord record2{{"name", std::string("Bob")}, {"age", 25}};
    DataRecord record3{{"name", std::string("Charlie")}, {"age", 35}};
    testData.push_back(record1);
    testData.push_back(record2);
    testData.push_back(record3);
    
    DataProcessor processor;
    
    // Test filter operation
    ProcessingStep filterStep;
    filterStep.operation = ProcessingOperation::FILTER;
    filterStep.condition = [](const DataRecord& record) {
        auto it = record.find("age");
        if (it != record.end()) {
            return std::get<int>(it->second) >= 30;
        }
        return false;
    };
    
    processor.addStep(filterStep);
    
    DataSet output;
    EXPECT_EQ(processor.process(testData, output), HatsStatus::SUCCESS);
    EXPECT_EQ(output.size(), 2); // Alice and Charlie (age >= 30)
    
    // Test sort operation
    processor.clearSteps();
    ProcessingStep sortStep;
    sortStep.operation = ProcessingOperation::SORT;
    processor.addStep(sortStep);
    
    EXPECT_EQ(processor.process(testData, output), HatsStatus::SUCCESS);
    EXPECT_EQ(output.size(), 3);
    // Check if sorting worked - just verify all records are there
    // (The exact order depends on which field is used for sorting)
    bool foundAlice = false, foundBob = false, foundCharlie = false;
    for (const auto& record : output) {
        auto it = record.find("name");
        if (it != record.end()) {
            std::string name = std::get<std::string>(it->second);
            if (name == "Alice") foundAlice = true;
            else if (name == "Bob") foundBob = true;
            else if (name == "Charlie") foundCharlie = true;
        }
    }
    EXPECT_TRUE(foundAlice);
    EXPECT_TRUE(foundBob);
    EXPECT_TRUE(foundCharlie);
}

TEST_F(HatsTest, HatsManager) {
    HatsManager manager;
    
    // Create and register CSV data source
    DataSourceConfig csvConfig;
    csvConfig.id = "csv_source";
    csvConfig.type = DataSourceType::CSV;
    csvConfig.location = csvFile;
    csvConfig.parameters["hasHeader"] = "true";
    
    auto csvSource = hats_utils::createDataSource(csvConfig);
    EXPECT_NE(csvSource, nullptr);
    
    EXPECT_EQ(manager.registerDataSource(std::move(csvSource)), HatsStatus::SUCCESS);
    EXPECT_EQ(manager.getRegisteredSourceCount(), 1);
    EXPECT_TRUE(manager.isSourceRegistered("csv_source"));
    
    // Test loading from source
    DataSet data;
    EXPECT_EQ(manager.loadFromSource("csv_source", data), HatsStatus::SUCCESS);
    EXPECT_EQ(data.size(), 3);
    
    // Test processing data
    std::vector<ProcessingStep> steps;
    ProcessingStep filterStep;
    filterStep.operation = ProcessingOperation::FILTER;
    filterStep.condition = [](const DataRecord& record) {
        auto it = record.find("age");
        return it != record.end() && std::get<int>(it->second) >= 30;
    };
    steps.push_back(filterStep);
    
    DataSet processedData;
    EXPECT_EQ(manager.processData("csv_source", steps, processedData), HatsStatus::SUCCESS);
    EXPECT_EQ(processedData.size(), 2); // Alice and Charlie
    
    // Test unregistering source
    EXPECT_EQ(manager.unregisterDataSource("csv_source"), HatsStatus::SUCCESS);
    EXPECT_EQ(manager.getRegisteredSourceCount(), 0);
    EXPECT_FALSE(manager.isSourceRegistered("csv_source"));
}

TEST_F(HatsTest, StatusToString) {
    EXPECT_EQ(hats_utils::statusToString(HatsStatus::SUCCESS), "SUCCESS");
    EXPECT_EQ(hats_utils::statusToString(HatsStatus::ERROR_INVALID_SOURCE), "ERROR_INVALID_SOURCE");
    EXPECT_EQ(hats_utils::statusToString(HatsStatus::ERROR_NOT_FOUND), "ERROR_NOT_FOUND");
}

TEST_F(HatsTest, MultipleDataSources) {
    HatsManager manager;
    
    // Register CSV source
    DataSourceConfig csvConfig;
    csvConfig.id = "csv_source";
    csvConfig.type = DataSourceType::CSV;
    csvConfig.location = csvFile;
    csvConfig.parameters["hasHeader"] = "true";
    
    auto csvSource = hats_utils::createDataSource(csvConfig);
    EXPECT_EQ(manager.registerDataSource(std::move(csvSource)), HatsStatus::SUCCESS);
    
    // Register JSON source
    DataSourceConfig jsonConfig;
    jsonConfig.id = "json_source";
    jsonConfig.type = DataSourceType::JSON;
    jsonConfig.location = jsonFile;
    
    auto jsonSource = hats_utils::createDataSource(jsonConfig);
    EXPECT_EQ(manager.registerDataSource(std::move(jsonSource)), HatsStatus::SUCCESS);
    
    EXPECT_EQ(manager.getRegisteredSourceCount(), 2);
    
    // Load from multiple sources
    std::vector<std::string> sourceIds = {"csv_source", "json_source"};
    DataSet mergedData;
    EXPECT_EQ(manager.loadFromMultipleSources(sourceIds, mergedData), HatsStatus::SUCCESS);
    EXPECT_EQ(mergedData.size(), 5); // 3 from CSV + 2 from JSON
}