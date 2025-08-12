# HATs Protocol - Hub of All Things

The HATs (Hub of All Things) protocol is a C++ implementation for handling multiple data sources in the ElizaOS framework. It provides a unified interface for loading, processing, and managing data from various formats and sources.

## Features

- **Multiple Data Source Support**: CSV, JSON, with extensible architecture for XML, Database, API, and custom sources
- **Data Source Management**: Register, unregister, and manage multiple data sources through a centralized manager
- **Type-Safe Data Handling**: Automatic type inference and conversion using std::variant for robust data operations  
- **Processing Pipelines**: Filter, transform, sort, and aggregate data using configurable processing steps
- **Multi-Source Operations**: Load and merge data from multiple sources seamlessly

## Quick Start

### Basic Usage

```cpp
#include "elizaos/hats.hpp"
using namespace elizaos;

// Create HATs manager
HatsManager manager;

// Configure CSV data source
DataSourceConfig csvConfig;
csvConfig.id = "my_data";
csvConfig.type = DataSourceType::CSV;
csvConfig.location = "/path/to/data.csv";
csvConfig.parameters["hasHeader"] = "true";
csvConfig.parameters["delimiter"] = ",";

// Register data source
auto source = hats_utils::createDataSource(csvConfig);
manager.registerDataSource(std::move(source));

// Load data
DataSet data;
manager.loadFromSource("my_data", data);

// Process data - filter records
std::vector<ProcessingStep> steps;
ProcessingStep filter;
filter.operation = ProcessingOperation::FILTER;
filter.condition = [](const DataRecord& record) {
    auto it = record.find("price");
    return it != record.end() && std::get<double>(it->second) > 100.0;
};
steps.push_back(filter);

DataSet filtered;
manager.processData("my_data", steps, filtered);
```

### Supported Data Types

The protocol automatically handles type conversion for:
- `int` - Integer values
- `double` - Floating-point values  
- `bool` - Boolean values (true/false)
- `std::string` - Text values

### Data Source Types

#### CSV Sources
```cpp
DataSourceConfig config;
config.type = DataSourceType::CSV;
config.parameters["hasHeader"] = "true";  // Parse first row as headers
config.parameters["delimiter"] = ",";      // Field delimiter
```

#### JSON Sources
```cpp
DataSourceConfig config;
config.type = DataSourceType::JSON;
// Supports line-by-line JSON objects
```

### Processing Operations

- **FILTER**: Apply conditional filtering to records
- **TRANSFORM**: Transform data values (extensible)
- **SORT**: Sort records by field values
- **AGGREGATE**: Aggregate data (planned)
- **GROUP**: Group records (planned)
- **JOIN**: Join datasets (planned)

### Error Handling

All operations return `HatsStatus` enum values:
- `SUCCESS` - Operation completed successfully
- `ERROR_INVALID_SOURCE` - Invalid or missing data source
- `ERROR_INVALID_FORMAT` - Unsupported data format
- `ERROR_PROCESSING_FAILED` - Processing pipeline error
- `ERROR_NOT_FOUND` - Data source not found
- `ERROR_ACCESS_DENIED` - File access error

## Architecture

### Core Classes

- **HatsManager**: Central coordinator for data source management and operations
- **DataSource**: Abstract base class for data source implementations
- **DataProcessor**: Handles data transformation pipelines
- **DataSourceConfig**: Configuration structure for data sources

### Data Structures

- **DataValue**: Type-safe variant holding int, double, bool, or string
- **DataRecord**: Map of field names to DataValue objects  
- **DataSet**: Vector of DataRecord objects representing a dataset

## Building and Testing

The HATs module is integrated with the main ElizaOS CMake build system:

```bash
# Build the module
make elizaos-hats

# Build and run tests
cmake . -B build -DBUILD_TESTING=ON
make test_hats
./build/cpp/hats/test_hats
```

## Example: Multi-Source Data Integration

```cpp
// Register multiple sources
manager.registerDataSource(createDataSource(csvConfig));
manager.registerDataSource(createDataSource(jsonConfig));

// Load from multiple sources
std::vector<std::string> sourceIds = {"csv_data", "json_data"};  
DataSet merged;
manager.loadFromMultipleSources(sourceIds, merged);

// Process merged data
DataSet processed;
manager.processData("csv_data", filterSteps, processed);
```

This protocol provides a foundation for building sophisticated data processing pipelines that can handle diverse data sources and formats in autonomous agent systems.