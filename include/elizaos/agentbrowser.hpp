#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <mutex>

namespace elizaos {

// Forward declarations
class AgentMemoryManager;
class AgentLogger;

/**
 * Browser automation result types
 */
enum class BrowserActionResult {
    SUCCESS,
    FAILED,
    TIMEOUT,
    ELEMENT_NOT_FOUND,
    NAVIGATION_ERROR
};

/**
 * Web element selector types
 */
enum class SelectorType {
    CSS,
    XPATH,
    ID,
    CLASS_NAME,
    TAG_NAME
};

/**
 * Browser configuration options
 */
struct BrowserConfig {
    bool headless = true;
    int windowWidth = 1280;
    int windowHeight = 720;
    std::string userAgent = "ElizaOS-Agent/1.0";
    int pageLoadTimeout = 30; // seconds
    int elementTimeout = 10;   // seconds
    bool enableJavaScript = true;
    bool enableImages = false; // Faster loading for text extraction
    std::string downloadPath = "/tmp/elizaos_downloads";
};

/**
 * Web element representation
 */
struct WebElement {
    std::string id;
    std::string tag;
    std::string text;
    std::string innerHTML;
    std::unordered_map<std::string, std::string> attributes;
    
    bool isVisible = true;
    bool isEnabled = true;
    
    // Position and size (for screenshots/verification)
    int x = 0, y = 0, width = 0, height = 0;
};

/**
 * Page information structure
 */
struct PageInfo {
    std::string url;
    std::string title;
    std::string html;
    std::vector<std::string> links;
    std::vector<std::string> images;
    std::chrono::system_clock::time_point loadTime;
    bool isLoaded = false;
};

/**
 * Browser action result with detailed information
 */
struct BrowserResult {
    BrowserActionResult result;
    std::string message;
    std::optional<std::string> data;
    std::chrono::milliseconds duration{0};
    
    explicit operator bool() const {
        return result == BrowserActionResult::SUCCESS;
    }
};

/**
 * AgentBrowser - Web automation interface for ElizaOS agents
 * 
 * Provides headless browser automation capabilities including:
 * - Page navigation and content extraction
 * - Form interaction and data input
 * - Screenshot capture and visual verification
 * - Integration with agent memory for learned patterns
 */
class AgentBrowser {
public:
    explicit AgentBrowser(const BrowserConfig& config = BrowserConfig{});
    ~AgentBrowser();

    // Core lifecycle
    BrowserResult initialize();
    BrowserResult shutdown();
    bool isInitialized() const { return initialized_.load(); }

    // Navigation
    BrowserResult navigateTo(const std::string& url);
    BrowserResult goBack();
    BrowserResult goForward();
    BrowserResult refresh();
    BrowserResult waitForPageLoad(int timeoutSec = 30);

    // Content extraction
    std::optional<PageInfo> getCurrentPageInfo();
    std::optional<std::string> getPageTitle();
    std::optional<std::string> getPageText();
    std::optional<std::string> getPageHTML();
    std::vector<std::string> getLinks();
    std::vector<std::string> getImages();

    // Element interaction
    std::optional<WebElement> findElement(const std::string& selector, SelectorType type = SelectorType::CSS);
    std::vector<WebElement> findElements(const std::string& selector, SelectorType type = SelectorType::CSS);
    BrowserResult clickElement(const std::string& selector, SelectorType type = SelectorType::CSS);
    BrowserResult typeText(const std::string& selector, const std::string& text, SelectorType type = SelectorType::CSS);
    BrowserResult clearText(const std::string& selector, SelectorType type = SelectorType::CSS);
    
    // Form handling
    BrowserResult fillForm(const std::unordered_map<std::string, std::string>& formData);
    BrowserResult submitForm(const std::string& formSelector = "form");
    BrowserResult selectOption(const std::string& selector, const std::string& value);
    BrowserResult checkCheckbox(const std::string& selector, bool checked = true);

    // Advanced features
    BrowserResult executeJavaScript(const std::string& script);
    std::optional<std::string> evaluateJavaScript(const std::string& expression);
    BrowserResult scrollToElement(const std::string& selector);
    BrowserResult scrollBy(int x, int y);

    // Screenshot and debugging
    BrowserResult captureScreenshot(const std::string& filename = "");
    std::vector<uint8_t> getScreenshotData();
    BrowserResult savePageHTML(const std::string& filename);

    // Wait operations  
    BrowserResult waitForElement(const std::string& selector, int timeoutSec = 10);
    BrowserResult waitForElementVisible(const std::string& selector, int timeoutSec = 10);
    BrowserResult waitForElementClickable(const std::string& selector, int timeoutSec = 10);
    BrowserResult waitForText(const std::string& text, int timeoutSec = 10);

    // Memory integration
    void setMemory(std::shared_ptr<AgentMemoryManager> memory);
    void setLogger(std::shared_ptr<AgentLogger> logger);
    
    // Remember browsing patterns for future automation
    void rememberPage(const std::string& url, const std::string& purpose);
    std::vector<std::string> getSimilarPages(const std::string& purpose);

    // Configuration
    void setConfig(const BrowserConfig& config);
    BrowserConfig getConfig() const { return config_; }

    // Statistics and monitoring
    struct Statistics {
        int pagesVisited = 0;
        int elementsClicked = 0;
        int formsSubmitted = 0;
        int screenshotsTaken = 0;
        std::chrono::milliseconds totalNavigationTime{0};
        std::chrono::system_clock::time_point sessionStart;
    };
    
    Statistics getStatistics() const { return stats_; }
    void resetStatistics();

private:
    BrowserConfig config_;
    std::atomic<bool> initialized_{false};
    std::atomic<bool> shouldStop_{false};
    
    // Browser session management
    std::string sessionId_;
    std::string currentUrl_;
    mutable std::mutex sessionMutex_;
    
    // Memory and logging integration
    std::shared_ptr<AgentMemoryManager> memory_;
    std::shared_ptr<AgentLogger> logger_;
    
    // Statistics
    mutable std::mutex statsMutex_;
    Statistics stats_;
    
    // Internal browser driver management
    // NOTE: In full implementation, this would contain WebDriver/DevTools client
    void* browserDriver_; // Placeholder for actual browser driver

    // Internal helper methods
    BrowserResult validateSelector(const std::string& selector, SelectorType type);
    std::string generateScreenshotFilename();
    void logAction(const std::string& action, const BrowserResult& result);
    void updateStatistics(const std::string& action, std::chrono::milliseconds duration);
    
    // Browser driver operations (implementation-specific)
    BrowserResult initializeBrowserDriver();
    void shutdownBrowserDriver();
    BrowserResult sendBrowserCommand(const std::string& command, const std::unordered_map<std::string, std::string>& params = {});
};

/**
 * Browser automation helper functions
 */
namespace browser_utils {
    std::string cssSelector(const std::string& element, const std::string& attribute = "", const std::string& value = "");
    std::string xpathSelector(const std::string& element, const std::string& text = "");
    bool isValidUrl(const std::string& url);
    std::string extractDomain(const std::string& url);
    std::vector<std::string> extractEmails(const std::string& text);
    std::vector<std::string> extractPhoneNumbers(const std::string& text);
}

} // namespace elizaos