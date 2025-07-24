/**
 * AgentBrowser Implementation Example
 * 
 * This demonstrates how the AgentBrowser starter implementation works
 * and shows the structure for the full implementation.
 */

#include "agentbrowser_starter.cpp"
#include "elizaos/agentlogger.hpp"
#include "elizaos/agentmemory.hpp"

#include <iostream>
#include <thread>

using namespace elizaos;

int main() {
    std::cout << "=== AgentBrowser Starter Implementation Demo ===" << std::endl;
    std::cout << std::endl;
    
    // Create browser with configuration
    BrowserConfig config;
    config.headless = true;
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.userAgent = "ElizaOS-DemoAgent/1.0";
    
    auto browser = std::make_shared<AgentBrowser>(config);
    
    // Set up logging and memory (optional)
    auto logger = std::make_shared<AgentLogger>();
    auto memory = std::make_shared<AgentMemoryManager>();
    
    browser->setLogger(logger);
    browser->setMemory(memory);
    
    // Initialize browser
    std::cout << "🚀 Initializing browser..." << std::endl;
    auto initResult = browser->initialize();
    if (!initResult) {
        std::cerr << "❌ Failed to initialize browser: " << initResult.message << std::endl;
        return 1;
    }
    std::cout << "✅ Browser initialized: " << initResult.message << " (took " << initResult.duration.count() << "ms)" << std::endl;
    std::cout << std::endl;
    
    // Demonstrate navigation
    std::cout << "🌐 Navigating to example.com..." << std::endl;
    auto navResult = browser->navigateTo("https://example.com");
    if (navResult) {
        std::cout << "✅ Navigation successful: " << navResult.message << " (took " << navResult.duration.count() << "ms)" << std::endl;
    } else {
        std::cout << "❌ Navigation failed: " << navResult.message << std::endl;
    }
    std::cout << std::endl;
    
    // Get page information
    std::cout << "📄 Getting page information..." << std::endl;
    auto pageInfo = browser->getCurrentPageInfo();
    if (pageInfo) {
        std::cout << "✅ Page Info Retrieved:" << std::endl;
        std::cout << "   URL: " << pageInfo->url << std::endl;
        std::cout << "   Title: " << pageInfo->title << std::endl;
        std::cout << "   Links found: " << pageInfo->links.size() << std::endl;
        std::cout << "   Images found: " << pageInfo->images.size() << std::endl;
        std::cout << "   Is loaded: " << (pageInfo->isLoaded ? "Yes" : "No") << std::endl;
    } else {
        std::cout << "❌ Could not retrieve page information" << std::endl;
    }
    std::cout << std::endl;
    
    // Demonstrate element interaction
    std::cout << "🖱️ Demonstrating element interaction..." << std::endl;
    
    // Click a button (placeholder implementation)
    auto clickResult = browser->clickElement("#search-button", SelectorType::CSS);
    if (clickResult) {
        std::cout << "✅ Element clicked: " << clickResult.message << " (took " << clickResult.duration.count() << "ms)" << std::endl;
    } else {
        std::cout << "❌ Click failed: " << clickResult.message << std::endl;
    }
    
    // Type text into input field
    auto typeResult = browser->typeText("#search-input", "ElizaOS autonomous agents", SelectorType::CSS);
    if (typeResult) {
        std::cout << "✅ Text typed: " << typeResult.message << " (took " << typeResult.duration.count() << "ms)" << std::endl;
    } else {
        std::cout << "❌ Type failed: " << typeResult.message << std::endl;
    }
    std::cout << std::endl;
    
    // Demonstrate screenshot capability
    std::cout << "📸 Taking screenshot..." << std::endl;
    auto screenshotResult = browser->captureScreenshot("demo_page.png");
    if (screenshotResult) {
        std::cout << "✅ Screenshot saved: " << screenshotResult.message << " (took " << screenshotResult.duration.count() << "ms)" << std::endl;
    } else {
        std::cout << "❌ Screenshot failed: " << screenshotResult.message << std::endl;
    }
    std::cout << std::endl;
    
    // Navigate to another page
    std::cout << "🌐 Navigating to GitHub..." << std::endl;
    auto navResult2 = browser->navigateTo("https://github.com/ZoneCog/elizaos-cpp");
    if (navResult2) {
        std::cout << "✅ Navigation successful: " << navResult2.message << std::endl;
        
        // Remember this page for future reference
        browser->rememberPage("https://github.com/ZoneCog/elizaos-cpp", "source_code_research");
        std::cout << "✅ Page remembered in agent memory" << std::endl;
    }
    std::cout << std::endl;
    
    // Demonstrate memory integration
    std::cout << "🧠 Testing memory integration..." << std::endl;
    auto similarPages = browser->getSimilarPages("research");
    std::cout << "📚 Found " << similarPages.size() << " similar pages for 'research' purposes:" << std::endl;
    for (const auto& url : similarPages) {
        std::cout << "   - " << url << std::endl;
    }
    std::cout << std::endl;
    
    // Show browser statistics
    std::cout << "📊 Browser session statistics:" << std::endl;
    auto stats = browser->getStatistics();
    std::cout << "   Pages visited: " << stats.pagesVisited << std::endl;
    std::cout << "   Elements clicked: " << stats.elementsClicked << std::endl;
    std::cout << "   Forms submitted: " << stats.formsSubmitted << std::endl;
    std::cout << "   Screenshots taken: " << stats.screenshotsTaken << std::endl;
    std::cout << "   Total navigation time: " << stats.totalNavigationTime.count() << "ms" << std::endl;
    
    auto sessionDuration = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now() - stats.sessionStart);
    std::cout << "   Session duration: " << sessionDuration.count() << " seconds" << std::endl;
    std::cout << std::endl;
    
    // Demonstrate browser utilities
    std::cout << "🔧 Testing browser utilities..." << std::endl;
    
    std::string testUrl = "https://example.com/page";
    std::cout << "   URL validation for '" << testUrl << "': " << (browser_utils::isValidUrl(testUrl) ? "Valid" : "Invalid") << std::endl;
    std::cout << "   Domain extraction: " << browser_utils::extractDomain(testUrl) << std::endl;
    
    std::string cssSelector = browser_utils::cssSelector("input", "type", "email");
    std::cout << "   Generated CSS selector: " << cssSelector << std::endl;
    
    std::string xpathSelector = browser_utils::xpathSelector("button", "Submit");
    std::cout << "   Generated XPath selector: " << xpathSelector << std::endl;
    
    std::string testText = "Contact us at support@example.com or call +1-555-123-4567";
    auto emails = browser_utils::extractEmails(testText);
    auto phones = browser_utils::extractPhoneNumbers(testText);
    std::cout << "   Extracted emails: " << emails.size() << " found" << std::endl;
    std::cout << "   Extracted phones: " << phones.size() << " found" << std::endl;
    std::cout << std::endl;
    
    // Shutdown browser
    std::cout << "🔄 Shutting down browser..." << std::endl;
    auto shutdownResult = browser->shutdown();
    if (shutdownResult) {
        std::cout << "✅ Browser shutdown: " << shutdownResult.message << " (took " << shutdownResult.duration.count() << "ms)" << std::endl;
    } else {
        std::cout << "❌ Shutdown failed: " << shutdownResult.message << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "=== Demo completed! ===" << std::endl;
    std::cout << std::endl;
    std::cout << "📋 Implementation Notes:" << std::endl;
    std::cout << "   • This starter implementation demonstrates the API structure" << std::endl;
    std::cout << "   • Real browser automation would require WebDriver or Chrome DevTools integration" << std::endl;
    std::cout << "   • Memory integration shows how browsing patterns can be learned" << std::endl;
    std::cout << "   • Full implementation would add: screenshot capture, JavaScript execution, form handling" << std::endl;
    std::cout << "   • Thread-safe design allows multiple browser instances in agent swarms" << std::endl;
    
    return 0;
}