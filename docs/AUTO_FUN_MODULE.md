# Auto.Fun Module for ElizaOS C++

This module provides a C++ API for integrating with the auto.fun Solana token platform, which implements bonding curve token mechanics.

## Overview

The auto.fun module allows you to:
- Configure platform settings
- Launch new tokens with bonding curves
- Perform token swaps (buy/sell)
- Query bonding curve states
- Manage platform authority

## Basic Usage

### Creating a Client

```cpp
#include "elizaos/auto_fun.hpp"

using namespace elizaos::auto_fun;

// Create client (uses default program address)
AutoFunClient client;

// Or specify custom program address
AutoFunClient client("your_program_address_here");
```

### Launching a Token

```cpp
LaunchParams params;
params.decimals = 9;
params.token_supply = 1000000000; // 1 billion tokens
params.virtual_lamport_reserves = 500000; // Initial liquidity
params.name = "My Token";
params.symbol = "MTK";
params.uri = "https://example.com/metadata.json";

auto result = client.launch(params);
if (result.success) {
    BondingCurve curve = result.value;
    std::cout << "Token launched: " << curve.token_mint << std::endl;
} else {
    std::cout << "Error: " << result.error_message << std::endl;
}
```

### Performing Swaps

```cpp
SwapParams swap_params;
swap_params.amount = 10000; // Amount to spend
swap_params.direction = 0; // 0 = buy, 1 = sell
swap_params.minimum_receive_amount = 1; // Slippage protection
swap_params.deadline = std::chrono::system_clock::to_time_t(
    std::chrono::system_clock::now() + std::chrono::hours(1)
);

auto swap_result = client.swap("token_mint_address", swap_params);
if (swap_result.success) {
    std::cout << "Received: " << swap_result.value << " tokens" << std::endl;
}
```

### Launch and Swap in One Transaction

```cpp
LaunchAndSwapParams params;
// ... set launch parameters ...
params.swap_amount = 50000;
params.minimum_receive_amount = 1000;
params.deadline = std::chrono::system_clock::to_time_t(
    std::chrono::system_clock::now() + std::chrono::hours(1)
);

auto result = client.launchAndSwap(params);
if (result.success) {
    auto [curve, tokens_received] = result.value;
    std::cout << "Token launched and " << tokens_received << " tokens received" << std::endl;
}
```

### Querying Bonding Curves

```cpp
// Get specific bonding curve
auto curve_result = client.getBondingCurve("token_mint_address");
if (curve_result.success) {
    BondingCurve curve = curve_result.value;
    std::cout << "Reserve lamports: " << curve.reserve_lamport << std::endl;
    std::cout << "Reserve tokens: " << curve.reserve_token << std::endl;
    std::cout << "Completed: " << (curve.is_completed ? "Yes" : "No") << std::endl;
}

// Get all bonding curves
auto all_curves = client.getAllBondingCurves();
for (const auto& curve : all_curves) {
    std::cout << "Token: " << curve.token_mint << std::endl;
}
```

## Data Structures

### BondingCurve
Represents the state of a token's bonding curve:
- `token_mint`: The token's mint address
- `creator`: Address of the token creator
- `reserve_lamport`: SOL reserves
- `reserve_token`: Token reserves
- `curve_limit`: Completion threshold
- `is_completed`: Whether curve has graduated

### Config
Platform configuration:
- `authority`: Admin address
- `team_wallet`: Team fee recipient
- `platform_buy_fee`/`platform_sell_fee`: Trading fees
- `curve_limit`: Graduation threshold

## Error Handling

All operations return `Result<T>` objects that contain:
- `success`: Boolean indicating success/failure
- `value`: The result value (if successful)
- `error`: Error code (if failed)
- `error_message`: Human-readable error description

```cpp
auto result = client.launch(params);
if (!result.success) {
    switch (result.error) {
        case AutoFunError::VALUE_INVALID:
            std::cout << "Invalid parameters" << std::endl;
            break;
        case AutoFunError::CURVE_ALREADY_COMPLETED:
            std::cout << "Cannot trade completed curve" << std::endl;
            break;
        // ... handle other errors
    }
}
```

## Utility Functions

The `utils` namespace provides helper functions:

```cpp
// Validation
bool valid_name = utils::validateTokenName("My Token");
bool valid_symbol = utils::validateTokenSymbol("MTK");
bool valid_uri = utils::validateURI("https://example.com");

// Price calculation
u64 price = utils::calculateBondingCurvePrice(supply, curve_factor);

// Error formatting
std::string error_str = utils::formatError(AutoFunError::VALUE_INVALID);
```

## Integration with ElizaOS

The auto_fun module is built as part of the ElizaOS C++ framework and can be linked with:

```cmake
target_link_libraries(your_target elizaos-auto_fun elizaos-core)
```

## Building

The module is automatically built as part of the main ElizaOS C++ build:

```bash
mkdir build && cd build
cmake ..
make elizaos-auto_fun
```