#include <gtest/gtest.h>
#include "elizaos/spartan.hpp"

namespace elizaos {

class SpartanAgentTest : public ::testing::Test {
protected:
    void SetUp() override {
        config = getDefaultSpartanConfig();
        config.solanaRpcUrl = "https://test.rpc.com";
        config.solanaPublicKey = "TestPublicKey123";
        agent = createSpartanAgent(config);
    }
    
    SpartanConfig config;
    std::unique_ptr<SpartanAgent> agent;
};

TEST_F(SpartanAgentTest, InitializationWorks) {
    EXPECT_FALSE(agent->isInitialized());
    EXPECT_TRUE(agent->initialize());
    EXPECT_TRUE(agent->isInitialized());
    agent->shutdown();
    EXPECT_FALSE(agent->isInitialized());
}

TEST_F(SpartanAgentTest, ConfigurationHandling) {
    const auto& retrievedConfig = agent->getConfig();
    EXPECT_EQ(retrievedConfig.solanaRpcUrl, config.solanaRpcUrl);
    EXPECT_EQ(retrievedConfig.solanaPublicKey, config.solanaPublicKey);
    EXPECT_EQ(retrievedConfig.maxTradeAmount, config.maxTradeAmount);
    EXPECT_EQ(retrievedConfig.requireConfirmation, config.requireConfirmation);
}

TEST_F(SpartanAgentTest, TradingPoolManagement) {
    EXPECT_TRUE(agent->initialize());
    
    std::vector<std::string> owners = {"owner1", "owner2"};
    std::string poolId = agent->createTradingPool("Test Pool", owners);
    EXPECT_FALSE(poolId.empty());
    
    // Test getting pool
    TradingPool* pool = agent->getTradingPool(poolId);
    ASSERT_NE(pool, nullptr);
    EXPECT_EQ(pool->name, "Test Pool");
    EXPECT_EQ(pool->owners.size(), 2);
    
    // Test adding owner
    EXPECT_TRUE(agent->addPoolOwner(poolId, "owner3"));
    EXPECT_EQ(pool->owners.size(), 3);
    EXPECT_FALSE(agent->addPoolOwner(poolId, "owner1")); // Already exists
    
    // Test removing owner
    EXPECT_TRUE(agent->removePoolOwner(poolId, "owner1"));
    EXPECT_EQ(pool->owners.size(), 2);
    EXPECT_FALSE(agent->removePoolOwner(poolId, "owner1")); // Already removed
    
    // Test getting all pools
    auto pools = agent->getTradingPools();
    EXPECT_EQ(pools.size(), 1);
    EXPECT_EQ(pools[0].name, "Test Pool");
    
    agent->shutdown();
}

TEST_F(SpartanAgentTest, TokenInformation) {
    EXPECT_TRUE(agent->initialize());
    
    // Test getting token by symbol
    TokenInfo solToken = agent->getTokenInfo("SOL");
    EXPECT_EQ(solToken.symbol, "SOL");
    EXPECT_FALSE(solToken.address.empty());
    EXPECT_GT(solToken.price, 0);
    
    // Test getting token by address
    TokenInfo tokenByAddress = agent->getTokenInfoByAddress(solToken.address);
    EXPECT_EQ(tokenByAddress.symbol, "SOL");
    
    // Test getting top tokens
    auto topTokens = agent->getTopTokens(5);
    EXPECT_LE(topTokens.size(), 5);
    EXPECT_GT(topTokens.size(), 0);
    
    agent->shutdown();
}

TEST_F(SpartanAgentTest, TradingOperations) {
    EXPECT_TRUE(agent->initialize());
    
    // Test swap execution
    TradeResult swapResult = agent->executeSwap("SOL", "USDC", 10.0, "Orca");
    EXPECT_TRUE(swapResult.success);
    EXPECT_FALSE(swapResult.transactionId.empty());
    EXPECT_GT(swapResult.amountTraded, 0);
    
    // Test liquidity operations
    TradeResult addLiqResult = agent->addLiquidity("SOL", "USDC", 5.0, 100.0, "Orca");
    EXPECT_TRUE(addLiqResult.success);
    EXPECT_FALSE(addLiqResult.transactionId.empty());
    
    TradeResult removeLiqResult = agent->removeLiquidity("SOL", "USDC", 1000.0, "Orca");
    EXPECT_TRUE(removeLiqResult.success);
    EXPECT_FALSE(removeLiqResult.transactionId.empty());
    
    agent->shutdown();
}

TEST_F(SpartanAgentTest, CopyTradingManagement) {
    EXPECT_TRUE(agent->initialize());
    
    std::string testWallet = "TestWallet123456";
    
    // Test setup copy trading
    EXPECT_TRUE(agent->setupCopyTrading(testWallet, 0.1));
    EXPECT_FALSE(agent->setupCopyTrading(testWallet, 0.1)); // Already exists
    
    // Test getting copy trading wallets
    auto wallets = agent->getCopyTradingWallets();
    EXPECT_EQ(wallets.size(), 1);
    EXPECT_EQ(wallets[0], testWallet);
    
    // Test stop copy trading
    EXPECT_TRUE(agent->stopCopyTrading(testWallet));
    EXPECT_FALSE(agent->stopCopyTrading(testWallet)); // Already removed
    
    wallets = agent->getCopyTradingWallets();
    EXPECT_EQ(wallets.size(), 0);
    
    agent->shutdown();
}

TEST_F(SpartanAgentTest, TradeValidation) {
    EXPECT_TRUE(agent->initialize());
    
    // Test valid trade
    EXPECT_TRUE(agent->validateTrade("SOL", "USDC", 100.0));
    
    // Test invalid trades
    EXPECT_FALSE(agent->validateTrade("", "USDC", 100.0)); // Empty from token
    EXPECT_FALSE(agent->validateTrade("SOL", "", 100.0));  // Empty to token
    EXPECT_FALSE(agent->validateTrade("SOL", "USDC", 0.0)); // Zero amount
    EXPECT_FALSE(agent->validateTrade("SOL", "USDC", -10.0)); // Negative amount
    EXPECT_FALSE(agent->validateTrade("SOL", "USDC", 2000.0)); // Exceeds max trade amount
    
    agent->shutdown();
}

TEST_F(SpartanAgentTest, ResponseGeneration) {
    EXPECT_TRUE(agent->initialize());
    
    // Test various query responses
    std::string priceResponse = agent->generateResponse("What's the price of BONK?");
    EXPECT_NE(priceResponse.find("BONK"), std::string::npos);
    EXPECT_NE(priceResponse.find("$"), std::string::npos);
    
    std::string poolResponse = agent->generateResponse("Can you create a trading pool?");
    EXPECT_NE(poolResponse.find("shared wallet"), std::string::npos);
    
    std::string liquidityResponse = agent->generateResponse("Add liquidity to Orca");
    EXPECT_NE(liquidityResponse.find("APR"), std::string::npos);
    
    std::string copyTradeResponse = agent->generateResponse("Set up copy trading");
    EXPECT_NE(copyTradeResponse.find("wallet"), std::string::npos);
    
    agent->shutdown();
}

TEST_F(SpartanAgentTest, RiskManagement) {
    EXPECT_TRUE(agent->initialize());
    
    // Test risk percentage calculation
    double risk1 = agent->calculateRiskPercentage(100.0, 1000.0);
    EXPECT_DOUBLE_EQ(risk1, 0.1);
    
    double risk2 = agent->calculateRiskPercentage(50.0, 0.0); // Zero portfolio value
    EXPECT_DOUBLE_EQ(risk2, 1.0);
    
    // Test confirmation requirements
    EXPECT_TRUE(agent->requiresConfirmation("execute swap"));
    EXPECT_TRUE(agent->requiresConfirmation("add liquidity"));
    EXPECT_FALSE(agent->requiresConfirmation("get token price"));
    
    agent->shutdown();
}

TEST_F(SpartanAgentTest, ConfigurationFromEnvironment) {
    // Test default configuration
    SpartanConfig defaultConfig = getDefaultSpartanConfig();
    EXPECT_FALSE(defaultConfig.solanaRpcUrl.empty());
    EXPECT_GT(defaultConfig.maxTradeAmount, 0);
    EXPECT_GT(defaultConfig.maxRiskPercentage, 0);
    EXPECT_TRUE(defaultConfig.requireConfirmation);
    
    // Test configuration from environment (would need actual env vars set)
    SpartanConfig envConfig = getSpartanConfigFromEnvironment();
    EXPECT_FALSE(envConfig.solanaRpcUrl.empty());
}

} // namespace elizaos