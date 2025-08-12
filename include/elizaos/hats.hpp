#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <optional>
#include <variant>
#include <any>
#include <chrono>

namespace elizaos {

// Forward declarations
class DataSource;
class DataProcessor;
class DataFilter;

// Basic types for HATs protocol
using DataValue = std::variant<int, double, std::string, bool>;
using DataRecord = std::unordered_map<std::string, DataValue>;
using DataSet = std::vector<DataRecord>;
using Timestamp = std::chrono::system_clock::time_point;

/**
 * Supported data source types
 */
enum class DataSourceType {
    JSON,
    CSV,
    XML,
    DATABASE,
    API,
    STREAM,
    CUSTOM
};

/**
 * Data processing operations
 */
enum class ProcessingOperation {
    FILTER,
    TRANSFORM,
    AGGREGATE,
    SORT,
    GROUP,
    JOIN
};

/**
 * HATs protocol status codes
 */
enum class HatsStatus {
    SUCCESS,
    ERROR_INVALID_SOURCE,
    ERROR_INVALID_FORMAT,
    ERROR_PROCESSING_FAILED,
    ERROR_NOT_FOUND,
    ERROR_ACCESS_DENIED
};

/**
 * Data source configuration
 */
struct DataSourceConfig {
    std::string id;
    DataSourceType type;
    std::string location;  // file path, URL, connection string, etc.
    std::unordered_map<std::string, std::string> parameters;
    bool isActive = true;
    std::optional<std::chrono::milliseconds> refreshInterval;
};

/**
 * Data processing pipeline step
 */
struct ProcessingStep {
    ProcessingOperation operation;
    std::unordered_map<std::string, std::any> parameters;
    std::function<bool(const DataRecord&)> condition;
};

/**
 * Abstract base class for data sources
 */
class DataSource {
public:
    DataSource(const DataSourceConfig& config) : config_(config) {}
    virtual ~DataSource() = default;

    virtual HatsStatus connect() = 0;
    virtual HatsStatus disconnect() = 0;
    virtual HatsStatus loadData(DataSet& data) = 0;
    virtual bool isConnected() const = 0;
    
    const DataSourceConfig& getConfig() const { return config_; }
    const std::string& getId() const { return config_.id; }
    DataSourceType getType() const { return config_.type; }

protected:
    DataSourceConfig config_;
};

/**
 * JSON data source implementation
 */
class JsonDataSource : public DataSource {
public:
    JsonDataSource(const DataSourceConfig& config);
    
    HatsStatus connect() override;
    HatsStatus disconnect() override;
    HatsStatus loadData(DataSet& data) override;
    bool isConnected() const override;

private:
    bool connected_ = false;
};

/**
 * CSV data source implementation
 */
class CsvDataSource : public DataSource {
public:
    CsvDataSource(const DataSourceConfig& config);
    
    HatsStatus connect() override;
    HatsStatus disconnect() override;
    HatsStatus loadData(DataSet& data) override;
    bool isConnected() const override;

private:
    bool connected_ = false;
    char delimiter_ = ',';
    bool hasHeader_ = true;
};

/**
 * Data processor for transforming and filtering data
 */
class DataProcessor {
public:
    DataProcessor() = default;
    
    // Add processing step to pipeline
    void addStep(const ProcessingStep& step);
    
    // Process data through pipeline
    HatsStatus process(const DataSet& input, DataSet& output);
    
    // Clear all processing steps
    void clearSteps();
    
    // Get number of processing steps
    size_t getStepCount() const;

private:
    std::vector<ProcessingStep> steps_;
    
    HatsStatus applyFilter(const DataSet& input, DataSet& output, 
                          const std::function<bool(const DataRecord&)>& filter);
    HatsStatus applyTransform(const DataSet& input, DataSet& output,
                             const std::unordered_map<std::string, std::any>& params);
};

/**
 * Main HATs protocol manager
 */
class HatsManager {
public:
    HatsManager() = default;
    ~HatsManager() = default;

    // Data source management
    HatsStatus registerDataSource(std::unique_ptr<DataSource> source);
    HatsStatus unregisterDataSource(const std::string& sourceId);
    DataSource* getDataSource(const std::string& sourceId);
    std::vector<std::string> getDataSourceIds() const;
    
    // Data operations
    HatsStatus loadFromSource(const std::string& sourceId, DataSet& data);
    HatsStatus loadFromMultipleSources(const std::vector<std::string>& sourceIds, DataSet& data);
    
    // Data processing
    HatsStatus processData(const std::string& sourceId, 
                          const std::vector<ProcessingStep>& steps,
                          DataSet& output);
    
    // Utility methods
    HatsStatus mergeDataSets(const std::vector<DataSet>& inputs, DataSet& merged);
    size_t getRegisteredSourceCount() const;
    bool isSourceRegistered(const std::string& sourceId) const;

private:
    std::unordered_map<std::string, std::unique_ptr<DataSource>> dataSources_;
    DataProcessor processor_;
};

/**
 * Utility functions for HATs protocol
 */
namespace hats_utils {
    // Convert DataValue to string representation
    std::string dataValueToString(const DataValue& value);
    
    // Parse string to DataValue with type inference
    DataValue parseDataValue(const std::string& str);
    
    // Create data source from configuration
    std::unique_ptr<DataSource> createDataSource(const DataSourceConfig& config);
    
    // Get status string for debugging
    std::string statusToString(HatsStatus status);
}

} // namespace elizaos