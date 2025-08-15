#include "elizaos/spartan.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>

int main() {
    std::cout << "Spartan DeFi Trading Agent Demo\n";
    std::cout << "================================\n\n";
    
    // Create and configure Spartan agent
    elizaos::SpartanConfig config = elizaos::getDefaultSpartanConfig();
    config.solanaPublicKey = "BzsJQeZ7cvk3pTHmKeuvdhNDkDxcZ6uCXxW2rjwC7RTq";
    
    std::cout << "Creating Spartan agent with configuration:\n";
    std::cout << "- RPC URL: " << config.solanaRpcUrl << "\n";
    std::cout << "- Public Key: " << config.solanaPublicKey << "\n";
    std::cout << "- Max Trade Amount: $" << config.maxTradeAmount << "\n";
    std::cout << "- Max Risk Percentage: " << (config.maxRiskPercentage * 100) << "%\n\n";
    
    auto agent = elizaos::createSpartanAgent(config);
    
    // Initialize agent
    std::cout << "Initializing Spartan agent...\n";
    if (!agent->initialize()) {
        std::cerr << "Failed to initialize Spartan agent!\n";
        return 1;
    }
    std::cout << "✓ Spartan agent initialized successfully\n\n";
    
    // Test token information
    std::cout << "Testing token information:\n";
    auto solToken = agent->getTokenInfo("SOL");
    std::cout << "- " << agent->formatTokenPrice(solToken) << "\n";
    
    auto bonkToken = agent->getTokenInfo("BONK");
    std::cout << "- " << agent->formatTokenPrice(bonkToken) << "\n\n";
    
    // Test trading pool creation
    std::cout << "Testing trading pool creation:\n";
    std::vector<std::string> owners = {"alice", "bob"};
    std::string poolId = agent->createTradingPool("Demo Pool", owners);
    std::cout << "✓ Created trading pool: " << poolId << "\n";
    
    // Add another owner
    agent->addPoolOwner(poolId, "charlie");
    auto pool = agent->getTradingPool(poolId);
    std::cout << "✓ Pool now has " << pool->owners.size() << " owners\n\n";
    
    // Test trade validation
    std::cout << "Testing trade validation:\n";
    bool validTrade = agent->validateTrade("SOL", "USDC", 100.0);
    std::cout << "- Trade SOL->USDC (100): " << (validTrade ? "✓ Valid" : "✗ Invalid") << "\n";
    
    bool invalidTrade = agent->validateTrade("SOL", "USDC", 2000.0);
    std::cout << "- Trade SOL->USDC (2000): " << (invalidTrade ? "✓ Valid" : "✗ Invalid") << "\n\n";
    
    // Test swap execution
    std::cout << "Testing swap execution:\n";
    auto swapResult = agent->executeSwap("SOL", "USDC", 10.0, "Orca");
    if (swapResult.success) {
        std::cout << "✓ Swap executed successfully\n";
        std::cout << "  - Transaction ID: " << swapResult.transactionId << "\n";
        std::cout << "  - Amount traded: " << swapResult.amountTraded << "\n";
        std::cout << "  - Message: " << swapResult.message << "\n";
    } else {
        std::cout << "✗ Swap failed: " << swapResult.message << "\n";
    }
    std::cout << "\n";
    
    // Test copy trading setup
    std::cout << "Testing copy trading setup:\n";
    std::string eliteWallet = "abc123def456ghi789";
    bool copySetup = agent->setupCopyTrading(eliteWallet, 0.1);
    std::cout << "- Setup copy trading: " << (copySetup ? "✓ Success" : "✗ Failed") << "\n";
    
    auto copyWallets = agent->getCopyTradingWallets();
    std::cout << "- Copy trading wallets: " << copyWallets.size() << "\n\n";
    
    // Test response generation
    std::cout << "Testing conversational responses:\n";
    std::vector<std::string> queries = {
        "What's the price of BONK?",
        "Can you create a trading pool?", 
        "Add liquidity to Orca",
        "Set up copy trading",
        "What's your take on the crypto market?"
    };
    
    for (const auto& query : queries) {
        std::string response = agent->generateResponse(query);
        std::cout << "Q: " << query << "\n";
        std::cout << "A: " << response << "\n\n";
    }
    
    // Cleanup
    std::cout << "Shutting down Spartan agent...\n";
    agent->shutdown();
    std::cout << "✓ Spartan agent shut down successfully\n";
    
    std::cout << "\nDemo completed successfully!\n";
    return 0;
}