#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <cstdint>
#include <optional>

namespace elizaos {
namespace auto_fun {

// Type aliases matching Solana/Rust conventions
using Pubkey = std::string;
using u8 = std::uint8_t;
using u64 = std::uint64_t;
using u128 = __uint128_t;
using i64 = std::int64_t;
using f64 = double;

/**
 * Amount configuration enum for validation
 */
template<typename T>
struct AmountConfig {
    enum class Type { RANGE, ENUM };
    Type type;
    
    // For Range type
    std::optional<T> min;
    std::optional<T> max;
    
    // For Enum type
    std::vector<T> values;
    
    AmountConfig(Type t) : type(t) {}
};

/**
 * Configuration structure for auto.fun platform
 */
struct Config {
    Pubkey authority;
    Pubkey pending_authority;
    Pubkey team_wallet;
    f64 init_bonding_curve;
    u128 platform_buy_fee;
    u128 platform_sell_fee;
    u64 curve_limit;
    AmountConfig<u64> lamport_amount_config;
    AmountConfig<u64> token_supply_config;
    AmountConfig<u8> token_decimals_config;
    
    Config() 
        : init_bonding_curve(0.0)
        , platform_buy_fee(0)
        , platform_sell_fee(0)
        , curve_limit(0)
        , lamport_amount_config(AmountConfig<u64>::Type::RANGE)
        , token_supply_config(AmountConfig<u64>::Type::RANGE)
        , token_decimals_config(AmountConfig<u8>::Type::RANGE) {}
};

/**
 * Bonding curve structure representing token economics
 */
struct BondingCurve {
    Pubkey token_mint;
    Pubkey creator;
    u64 init_lamport;
    u64 reserve_lamport;
    u64 reserve_token;
    u64 curve_limit;
    bool is_completed;
    
    BondingCurve()
        : init_lamport(0)
        , reserve_lamport(0)
        , reserve_token(0)
        , curve_limit(0)
        , is_completed(false) {}
};

/**
 * Event emitted when bonding curve completes
 */
struct CompleteEvent {
    Pubkey user;
    Pubkey mint;
    Pubkey bonding_curve;
};

/**
 * Launch parameters for new tokens
 */
struct LaunchParams {
    u8 decimals;
    u64 token_supply;
    u64 virtual_lamport_reserves;
    std::string name;
    std::string symbol;
    std::string uri;
};

/**
 * Launch and swap parameters
 */
struct LaunchAndSwapParams : LaunchParams {
    u64 swap_amount;
    u64 minimum_receive_amount;
    i64 deadline;
};

/**
 * Swap parameters
 */
struct SwapParams {
    u64 amount;
    u8 direction; // 0 = buy, 1 = sell
    u64 minimum_receive_amount;
    i64 deadline;
};

/**
 * Error codes matching the IDL
 */
enum class AutoFunError {
    VALUE_TOO_SMALL = 6000,
    VALUE_TOO_LARGE = 6001,
    VALUE_INVALID = 6002,
    INCORRECT_CONFIG_ACCOUNT = 6003,
    INCORRECT_AUTHORITY = 6004,
    OVERFLOW_OR_UNDERFLOW_OCCURRED = 6005,
    INVALID_AMOUNT = 6006,
    INCORRECT_TEAM_WALLET = 6007,
    CURVE_NOT_COMPLETED = 6008,
    CURVE_ALREADY_COMPLETED = 6009,
    MINT_AUTHORITY_ENABLED = 6010,
    FREEZE_AUTHORITY_ENABLED = 6011,
    RETURN_AMOUNT_TOO_SMALL = 6012,
    TRANSACTION_EXPIRED = 6013,
    DECIMAL_OVERFLOW = 6014
};

/**
 * Result wrapper for operations
 */
template<typename T>
struct Result {
    bool success;
    T value;
    AutoFunError error;
    std::string error_message;
    
    Result(T val) : success(true), value(val), error(AutoFunError::VALUE_TOO_SMALL) {}
    Result(AutoFunError err, const std::string& msg) 
        : success(false), error(err), error_message(msg) {}
};

/**
 * Main AutoFun client class for interacting with the platform
 */
class AutoFunClient {
private:
    std::string program_address_;
    Config global_config_;
    std::unordered_map<std::string, BondingCurve> bonding_curves_;
    
public:
    explicit AutoFunClient(const std::string& program_address = "autoUmixaMaYKFjexMpQuBpNYntgbkzCo2b1ZqUaAZ5");
    
    // Configuration operations
    Result<bool> configure(const Config& new_config);
    Result<Config> getConfig() const;
    
    // Authority management
    Result<bool> nominateAuthority(const Pubkey& new_admin);
    Result<bool> acceptAuthority();
    
    // Token operations
    Result<BondingCurve> launch(const LaunchParams& params);
    Result<std::pair<BondingCurve, u64>> launchAndSwap(const LaunchAndSwapParams& params);
    Result<u64> swap(const Pubkey& token_mint, const SwapParams& params);
    Result<bool> withdraw(const Pubkey& token_mint);
    
    // Query operations
    Result<BondingCurve> getBondingCurve(const Pubkey& token_mint) const;
    std::vector<BondingCurve> getAllBondingCurves() const;
    
    // Utility functions
    bool validateConfig(const Config& config) const;
    u64 calculateSwapOutput(const BondingCurve& curve, u64 input_amount, bool is_buy) const;
    bool isCurveCompleted(const Pubkey& token_mint) const;
};

/**
 * Helper functions for auto.fun operations
 */
namespace utils {
    std::string generateTokenMetadata(const std::string& name, const std::string& symbol, const std::string& uri);
    bool validateTokenName(const std::string& name);
    bool validateTokenSymbol(const std::string& symbol);
    bool validateURI(const std::string& uri);
    u64 calculateBondingCurvePrice(u64 supply, f64 curve_factor);
    std::string formatError(AutoFunError error);
}

} // namespace auto_fun
} // namespace elizaos