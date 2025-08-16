# Vercel API Module for ElizaOS C++

A comprehensive C++ implementation of the Vercel API client for serverless deployment automation and management.

## Features

### Core Functionality
- **HTTP Client**: Full-featured HTTP client with libcurl support
- **Authentication**: Bearer token and basic auth support with proper base64 encoding
- **Project Management**: Create, update, delete, and list Vercel projects
- **Deployment Operations**: Deploy from directories or Git repositories
- **Domain Management**: Add, remove, and verify custom domains
- **Environment Variables**: Manage project environment variables
- **Webhook Support**: Create and manage webhooks for CI/CD integration
- **Monitoring**: Deployment logs and build logs retrieval
- **Error Handling**: Comprehensive error reporting and logging

### High-Level Integration
- **VercelIntegration**: Simplified wrapper for common workflows
- **Directory Deployment**: Direct deployment from local directories
- **Git Repository Deployment**: Deploy from remote Git repositories
- **Continuous Deployment**: Setup and manage CI/CD pipelines
- **Progress Monitoring**: Real-time deployment status tracking

## Requirements

- C++17 or later
- CMake 3.16+
- libcurl (for HTTP operations)
- nlohmann/json (automatically fetched by CMake)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install libcurl4-openssl-dev pkg-config
```

**macOS:**
```bash
brew install curl
```

**Windows:**
Install vcpkg and then:
```bash
vcpkg install curl
```

## Building

The module is integrated into the ElizaOS C++ build system:

```bash
mkdir build && cd build
cmake ..
make elizaos-vercel_api -j$(nproc)
```

### Running Tests

```bash
make test_vercel_api
./cpp/vercel_api/test_vercel_api

# Or through CTest
ctest -R VercelAPITest --verbose
```

### Running Demo

```bash
make demo_vercel_api
export VERCEL_TOKEN="your_token_here"  # Optional
./cpp/vercel_api/demo_vercel_api
```

## Quick Start

### Basic Usage

```cpp
#include "elizaos/vercel_api.hpp"
#include <iostream>

using namespace elizaos;

int main() {
    // Get your token from https://vercel.com/account/tokens
    VercelConfig config(std::getenv("VERCEL_TOKEN"));
    VercelAPI api(config);
    
    // Authenticate
    if (!api.authenticate()) {
        std::cerr << "Authentication failed: " << api.getLastError().message << std::endl;
        return 1;
    }
    
    // List projects
    auto projects = api.listProjects();
    for (const auto& project : projects) {
        std::cout << "Project: " << project.name << " (" << project.id << ")" << std::endl;
    }
    
    return 0;
}
```

### High-Level Integration

```cpp
#include "elizaos/vercel_api.hpp"
#include <iostream>

using namespace elizaos;

int main() {
    VercelConfig config(std::getenv("VERCEL_TOKEN"));
    VercelIntegration vercel(config);
    
    if (!vercel.initialize()) {
        std::cerr << "Failed to initialize Vercel integration" << std::endl;
        return 1;
    }
    
    // Deploy a directory
    auto deployment = vercel.deployDirectory("./my-website", "my-project");
    
    if (!deployment.id.empty()) {
        std::cout << "Deployment created: " << deployment.url << std::endl;
        
        // Monitor progress with callback
        bool success = vercel.monitorDeployment(deployment.id, 
            [](const std::string& status) {
                std::cout << "Status: " << status << std::endl;
            });
        
        if (success) {
            std::cout << "Deployment completed successfully!" << std::endl;
        }
    }
    
    return 0;
}
```

## API Reference

### Core Classes

#### VercelConfig
Configuration for the Vercel API client.

```cpp
VercelConfig config("your-api-token");
config.team_id = "team-id";           // Optional
config.timeout_seconds = 30;          // Default: 30
config.max_retries = 3;               // Default: 3
config.enable_logging = true;         // Default: true
```

#### VercelAPI
Low-level API client for direct Vercel API access.

**Authentication:**
```cpp
bool authenticate();
bool validateCredentials();
```

**Project Management:**
```cpp
std::vector<VercelProject> listProjects();
VercelProject getProject(const std::string& project_id);
VercelProject createProject(const std::string& name, const std::string& framework = "");
bool deleteProject(const std::string& project_id);
bool updateProject(const VercelProject& project);
```

**Deployment Operations:**
```cpp
VercelDeployment createDeployment(const DeploymentRequest& request);
VercelDeployment getDeployment(const std::string& deployment_id);
std::vector<VercelDeployment> listDeployments(const std::string& project_id = "");
bool deleteDeployment(const std::string& deployment_id);
bool cancelDeployment(const std::string& deployment_id);
bool waitForDeployment(const std::string& deployment_id, int timeout_seconds = 300);
```

**File Operations:**
```cpp
bool uploadFiles(const std::vector<DeploymentFile>& files);
std::string uploadFile(const std::string& file_path, const std::string& content);
bool downloadDeploymentFiles(const std::string& deployment_id, const std::string& output_dir);
```

**Domain Management:**
```cpp
std::vector<VercelDomain> listDomains();
VercelDomain addDomain(const std::string& domain_name, const std::string& project_id);
bool removeDomain(const std::string& domain_name);
bool verifyDomain(const std::string& domain_name);
```

**Environment Variables:**
```cpp
bool setEnvironmentVariable(const std::string& project_id, const std::string& key, 
                           const std::string& value, const std::string& target = "production");
bool removeEnvironmentVariable(const std::string& project_id, const std::string& key);
std::unordered_map<std::string, std::string> getEnvironmentVariables(const std::string& project_id);
```

**Monitoring:**
```cpp
std::string getDeploymentLogs(const std::string& deployment_id);
std::string getBuildLogs(const std::string& deployment_id);
```

**Webhooks:**
```cpp
bool createWebhook(const std::string& project_id, const std::string& url, 
                  const std::vector<std::string>& events);
bool deleteWebhook(const std::string& webhook_id);
```

#### VercelIntegration
High-level integration wrapper for common workflows.

```cpp
bool initialize();
bool isInitialized() const;

// Quick deployment
VercelDeployment deployDirectory(const std::string& directory_path, 
                               const std::string& project_name = "",
                               bool production = true);
VercelDeployment deployGitRepository(const std::string& git_url, 
                                   const std::string& project_name = "",
                                   const std::string& branch = "main");

// Project workflow
bool setupProject(const std::string& project_name, const std::string& framework = "static");
bool configureProject(const std::string& project_id, 
                     const std::unordered_map<std::string, std::string>& env_vars);
bool linkDomain(const std::string& project_id, const std::string& domain);

// Monitoring
bool monitorDeployment(const std::string& deployment_id, 
                      std::function<void(const std::string&)> progress_callback = nullptr);
std::vector<VercelDeployment> getRecentDeployments(const std::string& project_id, int limit = 10);

// Automation
bool enableContinuousDeployment(const std::string& project_id, const std::string& git_branch);
bool disableContinuousDeployment(const std::string& project_id);
```

### Data Structures

#### VercelDeployment
```cpp
struct VercelDeployment {
    std::string id;
    std::string url;
    std::string project_id;
    std::string state;  // BUILDING, READY, ERROR, CANCELED
    std::string type;   // LAMBDAS
    std::string target; // PRODUCTION, PREVIEW
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point ready_at;
    std::string git_branch;
    std::string git_commit_sha;
    std::string git_commit_message;
    std::vector<std::string> domains;
    std::unordered_map<std::string, std::string> env_vars;
    std::unordered_map<std::string, std::string> build_env;
    
    bool isReady() const;
    bool hasError() const;
    bool isBuilding() const;
};
```

#### VercelProject
```cpp
struct VercelProject {
    std::string id;
    std::string name;
    std::string framework;
    std::string account_id;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::vector<std::string> domains;
    std::unordered_map<std::string, std::string> env_vars;
    std::string git_repository;
    std::string root_directory;
    std::string build_command;
    std::string install_command;
    std::string output_directory;
    std::string node_version;
};
```

#### DeploymentRequest
```cpp
struct DeploymentRequest {
    std::string name;
    std::vector<DeploymentFile> files;
    std::string target = "PRODUCTION"; // or "PREVIEW"
    std::string project_id;
    std::unordered_map<std::string, std::string> env_vars;
    std::unordered_map<std::string, std::string> build_env;
    std::string git_source;
    bool force_new_deployment = false;
};
```

## Error Handling

All API methods provide comprehensive error handling:

```cpp
VercelAPI api(config);

auto projects = api.listProjects();
if (api.hasError()) {
    auto error = api.getLastError();
    std::cerr << "Error " << error.code << ": " << error.message << std::endl;
    api.clearError(); // Clear the error state
}
```

## Examples

### Deploy Static Website

```cpp
#include "elizaos/vercel_api.hpp"

using namespace elizaos;

void deployStaticSite() {
    VercelConfig config(std::getenv("VERCEL_TOKEN"));
    VercelIntegration vercel(config);
    
    if (!vercel.initialize()) {
        std::cerr << "Failed to initialize" << std::endl;
        return;
    }
    
    // Deploy website directory
    auto deployment = vercel.deployDirectory("./dist", "my-website", true);
    
    if (!deployment.id.empty()) {
        std::cout << "Deployed to: " << deployment.url << std::endl;
        
        // Add custom domain
        vercel.linkDomain(deployment.project_id, "www.mysite.com");
    }
}
```

### Environment Variables Management

```cpp
void manageEnvironmentVariables() {
    VercelAPI api(VercelConfig(std::getenv("VERCEL_TOKEN")));
    
    std::string project_id = "prj_123";
    
    // Set environment variables
    api.setEnvironmentVariable(project_id, "API_URL", "https://api.example.com");
    api.setEnvironmentVariable(project_id, "NODE_ENV", "production");
    
    // Get all environment variables
    auto env_vars = api.getEnvironmentVariables(project_id);
    for (const auto& [key, value] : env_vars) {
        std::cout << key << "=" << value << std::endl;
    }
    
    // Remove an environment variable
    api.removeEnvironmentVariable(project_id, "DEBUG");
}
```

### Webhook Setup

```cpp
void setupWebhooks() {
    VercelAPI api(VercelConfig(std::getenv("VERCEL_TOKEN")));
    
    std::vector<std::string> events = {
        "deployment.created",
        "deployment.ready",
        "deployment.error"
    };
    
    bool success = api.createWebhook("prj_123", "https://my-app.com/webhook", events);
    if (success) {
        std::cout << "Webhook created successfully" << std::endl;
    }
}
```

## Integration with ElizaOS

This module integrates seamlessly with the ElizaOS C++ framework:

```cmake
# In your CMakeLists.txt
target_link_libraries(your_app 
    elizaos-core
    elizaos-vercel_api
)
```

```cpp
// In your code
#include "elizaos/vercel_api.hpp"
#include "elizaos/agentlogger.hpp"

// The module automatically uses the ElizaOS logging system
```

## Contributing

1. Follow the ElizaOS C++ coding standards
2. Add tests for new functionality in `test_vercel_api.cpp`
3. Update documentation for new features
4. Ensure all tests pass before submitting changes

## License

This module is part of the ElizaOS C++ project and follows the same licensing terms.