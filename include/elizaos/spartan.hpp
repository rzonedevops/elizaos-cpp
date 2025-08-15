#pragma once

#include "core.hpp"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace elizaos {

/**
 * @brief Configuration for Spartan DeFi trading agent
 */
struct SpartanConfig {
    std::string solanaRpcUrl;
    std::string solanaPublicKey;
    std::string solanaPrivateKey;
    std::vector<std::string> preferredDexes;
    double maxTradeAmount = 1000.0;
    double maxRiskPercentage = 0.1;
    bool requireConfirmation = true;
    
    SpartanConfig() = default;
    
    SpartanConfig(const std::string& rpcUrl, 
                  const std::string& publicKey,
                  const std::string& privateKey)
        : solanaRpcUrl(rpcUrl), solanaPublicKey(publicKey), solanaPrivateKey(privateKey) {
        preferredDexes = {"Orca", "Raydium", "Meteora"};
    }
};

/**
 * @brief Token information structure
 */
struct TokenInfo {
    std::string symbol;
    std::string address;
    double price = 0.0;
    double marketCap = 0.0;
    double volume24h = 0.0;
    double change24h = 0.0;
    
    TokenInfo() = default;
    TokenInfo(const std::string& sym, const std::string& addr) 
        : symbol(sym), address(addr) {}
};

/**
 * @brief Trading pool information
 */
struct TradingPool {
    std::string id;
    std::string name;
    std::vector<std::string> owners;
    double totalValue = 0.0;
    std::unordered_map<std::string, double> allocations;
    
    TradingPool() = default;
    TradingPool(const std::string& poolId, const std::string& poolName)
        : id(poolId), name(poolName) {}
};

/**
 * @brief Trade execution result
 */
struct TradeResult {
    bool success = false;
    std::string transactionId;
    std::string message;
    double amountTraded = 0.0;
    double executionPrice = 0.0;
    
    TradeResult() = default;
    TradeResult(bool succeeded, const std::string& txId, const std::string& msg)
        : success(succeeded), transactionId(txId), message(msg) {}
};

/**
 * @brief Main Spartan DeFi trading agent class
 * 
 * Spartan is a DeFi trading agent specializing in Solana-based trading and 
 * liquidity pool management. It provides functionality for:
 * - Managing shared trading pools
 * - Executing trades across Solana DEXs 
 * - Tracking token data and market trends
 * - Managing LP positions
 * - Copy trading from elite wallets
 */
class SpartanAgent {
public:
    explicit SpartanAgent(const SpartanConfig& config);
    virtual ~SpartanAgent() = default;
    
    // Core functionality
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized_; }
    
    // Configuration
    const SpartanConfig& getConfig() const { return config_; }
    void updateConfig(const SpartanConfig& config);
    
    // Pool management
    std::string createTradingPool(const std::string& name, 
                                 const std::vector<std::string>& owners);
    bool addPoolOwner(const std::string& poolId, const std::string& owner);
    bool removePoolOwner(const std::string& poolId, const std::string& owner);
    std::vector<TradingPool> getTradingPools() const;
    TradingPool* getTradingPool(const std::string& poolId);
    
    // Token data and market information
    TokenInfo getTokenInfo(const std::string& symbol);
    TokenInfo getTokenInfoByAddress(const std::string& address);
    std::vector<TokenInfo> getTopTokens(size_t count = 10);
    
    // Trading functionality
    TradeResult executeSwap(const std::string& fromToken, 
                           const std::string& toToken,
                           double amount,
                           const std::string& dex = "");
    TradeResult addLiquidity(const std::string& tokenA, 
                            const std::string& tokenB,
                            double amountA,
                            double amountB,
                            const std::string& dex = "");
    TradeResult removeLiquidity(const std::string& tokenA,
                               const std::string& tokenB, 
                               double lpTokens,
                               const std::string& dex = "");
    
    // Copy trading
    bool setupCopyTrading(const std::string& walletAddress, double allocation);
    bool stopCopyTrading(const std::string& walletAddress);
    std::vector<std::string> getCopyTradingWallets() const;
    
    // Risk management
    bool validateTrade(const std::string& fromToken, 
                      const std::string& toToken, 
                      double amount);
    double calculateRiskPercentage(double amount, double portfolioValue);
    
    // Utility functions
    std::string generateResponse(const std::string& query);
    std::string formatTokenPrice(const TokenInfo& token);
    bool requiresConfirmation(const std::string& action);
    
private:
    SpartanConfig config_;
    bool initialized_ = false;
    std::unordered_map<std::string, TradingPool> pools_;
    std::unordered_map<std::string, TokenInfo> tokenCache_;
    std::vector<std::string> copyTradingWallets_;
    
    // Helper methods
    bool validateConfig() const;
    std::string generatePoolId();
    void updateTokenCache(const TokenInfo& token);
    bool connectToSolana();
    void disconnectFromSolana();
};

/**
 * @brief Factory function to create a Spartan agent instance
 */
std::unique_ptr<SpartanAgent> createSpartanAgent(const SpartanConfig& config);

/**
 * @brief Helper function to get default Spartan configuration
 */
SpartanConfig getDefaultSpartanConfig();

/**
 * @brief Helper function to parse Spartan configuration from environment
 */
SpartanConfig getSpartanConfigFromEnvironment();

} // namespace elizaos