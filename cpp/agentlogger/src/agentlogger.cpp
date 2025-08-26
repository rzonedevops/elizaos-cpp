#include "elizaos/agentlogger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <vector>
#include <fstream>

namespace elizaos {

// Global logger instance
std::shared_ptr<AgentLogger> globalLogger = std::make_shared<AgentLogger>();

AgentLogger::AgentLogger() : consoleEnabled_(true), fileEnabled_(true) {
    // Initialize default type colors
    typeColors_[LogLevel::UNKNOWN] = LogColor::WHITE;
    typeColors_[LogLevel::SYSTEM] = LogColor::MAGENTA;
    typeColors_[LogLevel::INFO] = LogColor::BLUE;
    typeColors_[LogLevel::WARNING] = LogColor::YELLOW;
    typeColors_[LogLevel::SUCCESS] = LogColor::GREEN;
    typeColors_[LogLevel::ERROR] = LogColor::RED;
    typeColors_[LogLevel::START] = LogColor::GREEN;
    typeColors_[LogLevel::STOP] = LogColor::RED;
    typeColors_[LogLevel::PAUSE] = LogColor::YELLOW;
    typeColors_[LogLevel::EPOCH] = LogColor::WHITE;
    typeColors_[LogLevel::SUMMARY] = LogColor::CYAN;
    typeColors_[LogLevel::REASONING] = LogColor::CYAN;
    typeColors_[LogLevel::ACTION] = LogColor::GREEN;
    typeColors_[LogLevel::PROMPT] = LogColor::CYAN;
}

AgentLogger::~AgentLogger() {
    // Destructor - file handles are automatically closed
}

void AgentLogger::log(
    const std::string& content,
    const std::string& source,
    const std::string& title,
    LogLevel level,
    LogColor color,
    bool /* expand */,
    bool panel,
    bool shouldLog
) {
    if (!shouldLog) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex_);
    
    // Build title with level and source
    std::string fullTitle = "(" + levelToString(level) + ") " + title;
    if (!source.empty()) {
        fullTitle += ": " + source;
    }
    
    // Use type color if color not overridden
    LogColor finalColor = (color == LogColor::BLUE) ? getDefaultColor(level) : color;
    
    if (consoleEnabled_) {
        if (panel) {
            std::cout << std::endl;
            std::cout << createPanel(content, fullTitle, finalColor) << std::endl;
        } else {
            std::cout << getColorCode(finalColor) << content << "\033[0m" << std::endl;
        }
    }
    
    // Also write to file if enabled
    if (fileEnabled_) {
        writeToFile(content, source, level);
    }
}

void AgentLogger::printHeader(const std::string& text, LogColor color) {
    std::lock_guard<std::mutex> lock(logMutex_);
    
    if (!consoleEnabled_) {
        return;
    }
    
    // Simple ASCII header (could be enhanced with figlet-style formatting)
    std::string header = "=== " + text + " ===";
    std::string colorCode = getColorCode(color);
    
    std::cout << std::endl;
    std::cout << colorCode << header << "\033[0m" << std::endl;
    std::cout << std::endl;
}

void AgentLogger::writeToFile(
    const std::string& content,
    const std::string& source,
    LogLevel level,
    const std::string& filename
) {
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        return; // Silently fail if file can't be opened
    }
    
    // Build header
    std::string header = "";
    if (!source.empty()) {
        header += source;
    }
    if (level != LogLevel::INFO) {
        if (!header.empty()) {
            header += ": ";
        }
        header += levelToString(level);
    }
    
    if (!header.empty()) {
        header = " " + header + " ";
    }
    
    // Create separator
    int barLength = (SEPARATOR_WIDTH - static_cast<int>(header.length())) / 2;
    std::string separator = std::string(barLength, '=') + header + std::string(barLength, '=');
    std::string footer = std::string(SEPARATOR_WIDTH, '=');
    
    // Get timestamp (cross-platform compatible)
    auto now = std::time(nullptr);
    struct tm tm;
    #ifdef _WIN32
        localtime_s(&tm, &now);  // Windows safe version
    #else
        localtime_r(&now, &tm);  // POSIX version
    #endif
    char timestamp[100];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
    
    // Write to file
    file << separator << std::endl << std::endl;
    file << "[" << timestamp << "] " << content << std::endl << std::endl;
    file << footer << std::endl << std::endl;
    
    file.close();
}

void AgentLogger::setTypeColor(LogLevel level, LogColor color) {
    std::lock_guard<std::mutex> lock(logMutex_);
    typeColors_[level] = color;
}

void AgentLogger::setConsoleEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(logMutex_);
    consoleEnabled_ = enabled;
}

void AgentLogger::setFileEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(logMutex_);
    fileEnabled_ = enabled;
}

std::string AgentLogger::getColorCode(LogColor color) const {
    switch (color) {
        case LogColor::WHITE:   return "\033[37m";
        case LogColor::MAGENTA: return "\033[35m";
        case LogColor::BLUE:    return "\033[34m";
        case LogColor::YELLOW:  return "\033[33m";
        case LogColor::GREEN:   return "\033[32m";
        case LogColor::RED:     return "\033[31m";
        case LogColor::CYAN:    return "\033[36m";
        default:                return "\033[37m";
    }
}

LogColor AgentLogger::getDefaultColor(LogLevel level) const {
    auto it = typeColors_.find(level);
    return (it != typeColors_.end()) ? it->second : LogColor::WHITE;
}

std::string AgentLogger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::UNKNOWN:     return "unknown";
        case LogLevel::SYSTEM:      return "system";
        case LogLevel::INFO:        return "info";
        case LogLevel::WARNING:     return "warning";
        case LogLevel::SUCCESS:     return "success";
        case LogLevel::ERROR:       return "error";
        case LogLevel::START:       return "start";
        case LogLevel::STOP:        return "stop";
        case LogLevel::PAUSE:       return "pause";
        case LogLevel::EPOCH:       return "epoch";
        case LogLevel::SUMMARY:     return "summary";
        case LogLevel::REASONING:   return "reasoning";
        case LogLevel::ACTION:      return "action";
        case LogLevel::PROMPT:      return "prompt";
        default:                    return "unknown";
    }
}

std::string AgentLogger::createPanel(
    const std::string& content,
    const std::string& title,
    LogColor color,
    int width
) const {
    std::string colorCode = getColorCode(color);
    std::string reset = "\033[0m";
    
    // Create top border with title (using simple ASCII characters)
    std::string topBorder = colorCode + "+";
    if (!title.empty()) {
        int titleSpace = width - 4 - static_cast<int>(title.length());
        if (titleSpace > 0) {
            topBorder += "- " + title + " " + std::string(titleSpace, '-');
        } else {
            topBorder += std::string(width - 2, '-');
        }
    } else {
        topBorder += std::string(width - 2, '-');
    }
    topBorder += "+" + reset;
    
    // Create content lines with proper wrapping
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.length() <= static_cast<size_t>(width - 4)) {
            lines.push_back(line);
        } else {
            // Simple word wrapping
            size_t pos = 0;
            while (pos < line.length()) {
                size_t end = std::min(pos + width - 4, line.length());
                lines.push_back(line.substr(pos, end - pos));
                pos = end;
            }
        }
    }
    
    // Create content with side borders
    std::string result = topBorder + "\n";
    for (const auto& contentLine : lines) {
        result += colorCode + "| " + reset + contentLine;
        // Pad to width
        int padding = width - 4 - static_cast<int>(contentLine.length());
        if (padding > 0) {
            result += std::string(padding, ' ');
        }
        result += colorCode + " |" + reset + "\n";
    }
    
    // Create bottom border (using simple ASCII characters)
    std::string bottomBorder = colorCode + "+" + std::string(width - 2, '-') + "+" + reset;
    result += bottomBorder;
    
    return result;
}

// Convenience functions
void logInfo(const std::string& content, const std::string& source) {
    globalLogger->log(content, source, "agentlogger", LogLevel::INFO);
}

void logWarning(const std::string& content, const std::string& source) {
    globalLogger->log(content, source, "agentlogger", LogLevel::WARNING);
}

void logError(const std::string& content, const std::string& source) {
    globalLogger->log(content, source, "agentlogger", LogLevel::ERROR);
}

void logSuccess(const std::string& content, const std::string& source) {
    globalLogger->log(content, source, "agentlogger", LogLevel::SUCCESS);
}

void logSystem(const std::string& content, const std::string& source) {
    globalLogger->log(content, source, "agentlogger", LogLevel::SYSTEM);
}

} // namespace elizaos
