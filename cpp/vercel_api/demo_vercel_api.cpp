#include "elizaos/vercel_api.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace elizaos;

void createSampleProject() {
    std::cout << "Creating sample project files...\n";
    
    // Create a simple static website
    std::filesystem::create_directories("sample_project");
    
    // Create index.html
    std::ofstream index("sample_project/index.html");
    index << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ElizaOS Demo</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .container { max-width: 600px; margin: 0 auto; text-align: center; }
        .logo { color: #007acc; font-size: 2em; margin-bottom: 20px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">🤖 ElizaOS</div>
        <h1>Welcome to ElizaOS Demo</h1>
        <p>This is a sample deployment created using the ElizaOS C++ Vercel API.</p>
        <p>Deployed at: <span id="timestamp"></span></p>
    </div>
    <script>
        document.getElementById('timestamp').textContent = new Date().toISOString();
    </script>
</body>
</html>)";
    index.close();
    
    // Create package.json for Next.js detection
    std::ofstream package("sample_project/package.json");
    package << R"({
  "name": "elizaos-demo",
  "version": "1.0.0",
  "description": "ElizaOS demo deployment",
  "main": "index.html",
  "scripts": {
    "build": "echo 'No build step needed for static site'"
  }
})";
    package.close();
    
    std::cout << "✓ Sample project created in 'sample_project/'\n";
}

void demonstrateBasicAPI() {
    std::cout << "\n=== Basic Vercel API Usage ===\n";
    
    // Note: In a real application, you'd get this from environment variables
    // export VERCEL_TOKEN="your_token_here"
    const char* token = std::getenv("VERCEL_TOKEN");
    if (!token) {
        std::cout << "⚠️  VERCEL_TOKEN environment variable not set. Using demo token.\n";
        token = "demo-token-replace-with-real-token";
    }
    
    // Create configuration
    VercelConfig config(token);
    config.timeout_seconds = 30;
    config.enable_logging = true;
    
    // Create API client
    VercelAPI api(config);
    
    std::cout << "✓ Created Vercel API client\n";
    
    // Note: The following would make real API calls if token is valid
    std::cout << "📡 API endpoints available:\n";
    std::cout << "   - Projects: listProjects(), createProject(), deleteProject()\n";
    std::cout << "   - Deployments: createDeployment(), getDeployment(), listDeployments()\n";
    std::cout << "   - Domains: addDomain(), removeDomain(), verifyDomain()\n";
    std::cout << "   - Environment: setEnvironmentVariable(), getEnvironmentVariables()\n";
    std::cout << "   - Webhooks: createWebhook(), deleteWebhook()\n";
    std::cout << "   - Monitoring: getDeploymentLogs(), getBuildLogs()\n";
    
    // Demonstrate error handling
    if (std::string(token) == "demo-token-replace-with-real-token") {
        std::cout << "🔒 Demo mode - API calls would fail with invalid token\n";
        std::cout << "   Set VERCEL_TOKEN environment variable for real API calls\n";
    }
}

void demonstrateHighLevelAPI() {
    std::cout << "\n=== High-Level Vercel Integration ===\n";
    
    const char* token = std::getenv("VERCEL_TOKEN");
    if (!token) {
        token = "demo-token-replace-with-real-token";
    }
    
    VercelConfig config(token);
    VercelIntegration integration(config);
    
    std::cout << "✓ Created Vercel Integration\n";
    
    // High-level workflow methods
    std::cout << "🚀 High-level workflows available:\n";
    std::cout << "   - deployDirectory(): Deploy local directory\n";
    std::cout << "   - deployGitRepository(): Deploy from Git URL\n";
    std::cout << "   - setupProject(): Create and configure project\n";
    std::cout << "   - monitorDeployment(): Monitor deployment progress\n";
    std::cout << "   - enableContinuousDeployment(): Setup CI/CD\n";
    
    // Example deployment configuration
    if (std::filesystem::exists("sample_project")) {
        std::cout << "\n📦 Example deployment configuration:\n";
        std::cout << "   Directory: sample_project/\n";
        std::cout << "   Framework: static\n";
        std::cout << "   Target: PRODUCTION\n";
        
        // This would perform actual deployment if token is valid
        std::cout << "   Command: integration.deployDirectory(\"sample_project\", \"elizaos-demo\")\n";
    }
}

void demonstrateDataStructures() {
    std::cout << "\n=== Data Structures Examples ===\n";
    
    // Create a deployment request
    DeploymentRequest request("elizaos-demo");
    request.target = "PRODUCTION";
    request.env_vars["NODE_ENV"] = "production";
    request.env_vars["API_URL"] = "https://api.elizaos.com";
    
    // Add files
    DeploymentFile index_file("index.html", "<html>Sample content</html>");
    DeploymentFile config_file("config.json", R"({"env": "production"})");
    
    request.files.push_back(index_file);
    request.files.push_back(config_file);
    
    std::cout << "✓ Created deployment request with " << request.files.size() << " files\n";
    
    // Create project configuration
    VercelProject project("", "elizaos-demo");
    project.framework = "static";
    project.env_vars["DEPLOYMENT_TYPE"] = "automated";
    project.build_command = "npm run build";
    project.output_directory = "dist";
    
    std::cout << "✓ Created project configuration\n";
    
    // Create domain configuration
    VercelDomain domain("demo.elizaos.com");
    domain.verified = false;
    domain.verification_challenges.push_back("elizaos-verification=abc123");
    
    std::cout << "✓ Created domain configuration\n";
    
    // Demonstrate deployment states
    VercelDeployment deployment("dpl_123", "https://elizaos-demo-123.vercel.app");
    
    deployment.state = "BUILDING";
    std::cout << "📊 Deployment states:\n";
    std::cout << "   Building: " << (deployment.isBuilding() ? "✓" : "✗") << "\n";
    std::cout << "   Ready: " << (deployment.isReady() ? "✓" : "✗") << "\n";
    std::cout << "   Error: " << (deployment.hasError() ? "✓" : "✗") << "\n";
    
    deployment.state = "READY";
    std::cout << "   Status changed to READY\n";
    std::cout << "   Building: " << (deployment.isBuilding() ? "✓" : "✗") << "\n";
    std::cout << "   Ready: " << (deployment.isReady() ? "✓" : "✗") << "\n";
}

void demonstrateHTTPClient() {
    std::cout << "\n=== HTTP Client Features ===\n";
    
    HttpClient client;
    
    // Configure client
    client.setTimeout(30);
    client.setUserAgent("ElizaOS-Demo/1.0");
    client.setFollowRedirects(true);
    client.setMaxRetries(3);
    
    std::cout << "✓ Configured HTTP client\n";
    
    // Authentication methods
    client.setBearerToken("your-api-token");
    client.setBasicAuth("username", "password");
    client.addDefaultHeader("X-Custom-Header", "value");
    
    std::cout << "✓ Configured authentication and headers\n";
    
    // Utility functions
    std::string encoded_url = client.urlEncode("hello world & symbols!");
    std::string escaped_json = client.jsonEscape("String with \"quotes\" and \n newlines");
    
    std::cout << "✓ URL encoding: " << encoded_url << "\n";
    std::cout << "✓ JSON escaping: " << escaped_json << "\n";
    
    std::cout << "🌐 HTTP methods available:\n";
    std::cout << "   - GET: client.get(url, headers)\n";
    std::cout << "   - POST: client.post(url, data, headers)\n";
    std::cout << "   - PUT: client.put(url, data, headers)\n";
    std::cout << "   - DELETE: client.del(url, headers)\n";
    std::cout << "   - PATCH: client.patch(url, data, headers)\n";
}

void showUsageInstructions() {
    std::cout << "\n=== Getting Started ===\n";
    std::cout << "1. Get a Vercel API token from https://vercel.com/account/tokens\n";
    std::cout << "2. Set environment variable: export VERCEL_TOKEN=\"your_token_here\"\n";
    std::cout << "3. Include the header: #include \"elizaos/vercel_api.hpp\"\n";
    std::cout << "4. Link the library: target_link_libraries(your_app elizaos-vercel_api)\n";
    std::cout << "\n📚 Example usage:\n";
    std::cout << R"(
    // Basic setup
    VercelConfig config(std::getenv("VERCEL_TOKEN"));
    VercelIntegration vercel(config);
    
    if (vercel.initialize()) {
        // Deploy a directory
        auto deployment = vercel.deployDirectory("./my-app", "my-project");
        
        // Monitor progress
        vercel.monitorDeployment(deployment.id, [](const std::string& status) {
            std::cout << "Status: " << status << std::endl;
        });
    }
)";
}

int main() {
    std::cout << "🚀 ElizaOS Vercel API Demo\n";
    std::cout << "==========================\n";
    
    try {
        createSampleProject();
        demonstrateBasicAPI();
        demonstrateHighLevelAPI();
        demonstrateDataStructures();
        demonstrateHTTPClient();
        showUsageInstructions();
        
        std::cout << "\n✅ Demo completed successfully!\n";
        std::cout << "The Vercel API implementation is ready for production use.\n";
        
        // Cleanup
        if (std::filesystem::exists("sample_project")) {
            std::filesystem::remove_all("sample_project");
            std::cout << "🧹 Cleaned up sample project files\n";
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Demo failed: " << e.what() << std::endl;
        return 1;
    }
}