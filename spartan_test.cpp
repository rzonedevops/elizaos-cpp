#include "elizaos/spartan.hpp"
#include <iostream>
#include <cassert>

void test_spartan_basic_functionality() {
    std::cout << "Testing Spartan basic functionality...\n";
    
    // Test configuration
    elizaos::SpartanConfig config = elizaos::getDefaultSpartanConfig();
    auto agent = elizaos::createSpartanAgent(config);
    
    // Test initialization
    assert(!agent->isInitialized());
    assert(agent->initialize());
    assert(agent->isInitialized());
    std::cout << "✓ Initialization works\n";
    
    // Test token information
    auto solToken = agent->getTokenInfo("SOL");
    assert(solToken.symbol == "SOL");
    assert(solToken.price > 0);
    std::cout << "✓ Token information works\n";
    
    // Test trading pools
    std::vector<std::string> owners = {"alice", "bob"};
    std::string poolId = agent->createTradingPool("Test Pool", owners);
    assert(!poolId.empty());
    
    elizaos::TradingPool* pool = agent->getTradingPool(poolId);
    assert(pool != nullptr);
    assert(pool->name == "Test Pool");
    assert(pool->owners.size() == 2);
    
    // Test adding/removing owners
    assert(agent->addPoolOwner(poolId, "charlie"));
    assert(pool->owners.size() == 3);
    assert(agent->removePoolOwner(poolId, "alice"));  
    assert(pool->owners.size() == 2);
    
    // Use pool variable to avoid unused warning
    (void)pool;
    
    std::cout << "✓ Trading pool management works\n";
    
    // Test trade validation
    assert(agent->validateTrade("SOL", "USDC", 100.0));
    assert(!agent->validateTrade("SOL", "USDC", 2000.0)); // Exceeds max
    assert(!agent->validateTrade("", "USDC", 100.0)); // Empty token
    std::cout << "✓ Trade validation works\n";
    
    // Test swap execution
    auto swapResult = agent->executeSwap("SOL", "USDC", 10.0);
    assert(swapResult.success);
    assert(!swapResult.transactionId.empty());
    assert(swapResult.amountTraded == 10.0);
    std::cout << "✓ Swap execution works\n";
    
    // Test copy trading
    std::string testWallet = "TestWallet123";
    assert(agent->setupCopyTrading(testWallet, 0.1));
    assert(!agent->setupCopyTrading(testWallet, 0.1)); // Already exists
    
    auto wallets = agent->getCopyTradingWallets();
    assert(wallets.size() == 1);
    assert(wallets[0] == testWallet);
    
    assert(agent->stopCopyTrading(testWallet));
    wallets = agent->getCopyTradingWallets();
    assert(wallets.size() == 0);
    std::cout << "✓ Copy trading management works\n";
    
    // Test response generation
    std::string response = agent->generateResponse("What's the price of BONK?");
    assert(response.find("BONK") != std::string::npos);
    assert(response.find("$") != std::string::npos);
    std::cout << "✓ Response generation works\n";
    
    // Cleanup
    agent->shutdown();
    assert(!agent->isInitialized());
    std::cout << "✓ Shutdown works\n";
}

void test_spartan_configuration() {
    std::cout << "\nTesting Spartan configuration...\n";
    
    // Test default configuration
    auto defaultConfig = elizaos::getDefaultSpartanConfig();
    assert(!defaultConfig.solanaRpcUrl.empty());
    assert(defaultConfig.maxTradeAmount > 0);
    assert(defaultConfig.maxRiskPercentage > 0);
    assert(defaultConfig.requireConfirmation);
    std::cout << "✓ Default configuration works\n";
    
    // Test environment configuration (won't have actual env vars but should not crash)
    auto envConfig = elizaos::getSpartanConfigFromEnvironment();
    assert(!envConfig.solanaRpcUrl.empty());
    std::cout << "✓ Environment configuration works\n";
}

void test_spartan_edge_cases() {
    std::cout << "\nTesting Spartan edge cases...\n";
    
    // Test invalid configuration
    elizaos::SpartanConfig invalidConfig;
    invalidConfig.solanaRpcUrl = ""; // Invalid - empty RPC URL
    auto agent = elizaos::createSpartanAgent(invalidConfig);
    assert(!agent->initialize()); // Should fail
    std::cout << "✓ Invalid configuration handling works\n";
    
    // Test nonexistent pool operations
    elizaos::SpartanConfig config = elizaos::getDefaultSpartanConfig();
    auto validAgent = elizaos::createSpartanAgent(config);
    assert(validAgent->initialize());
    
    assert(validAgent->getTradingPool("nonexistent") == nullptr);
    assert(!validAgent->addPoolOwner("nonexistent", "alice"));
    assert(!validAgent->removePoolOwner("nonexistent", "alice"));
    std::cout << "✓ Nonexistent pool handling works\n";
    
    // Test unknown token
    auto unknownToken = validAgent->getTokenInfo("UNKNOWN");
    assert(unknownToken.symbol == "UNKNOWN");
    assert(unknownToken.price == 0.0); // Should be default
    std::cout << "✓ Unknown token handling works\n";
    
    validAgent->shutdown();
}

int main() {
    std::cout << "Running Spartan Unit Tests\n";
    std::cout << "==========================\n";
    
    try {
        test_spartan_basic_functionality();
        test_spartan_configuration();
        test_spartan_edge_cases();
        
        std::cout << "\n🎉 All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n❌ Test failed with unknown exception\n";
        return 1;
    }
}