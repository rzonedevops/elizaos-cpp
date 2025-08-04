#include "elizaos/auto_fun.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <cmath>
#include <algorithm>

namespace elizaos {
namespace auto_fun {

// AutoFunClient implementation
AutoFunClient::AutoFunClient(const std::string& program_address) 
    : program_address_(program_address) {
    // Initialize with default configuration
    global_config_ = Config();
}

Result<bool> AutoFunClient::configure(const Config& new_config) {
    if (!validateConfig(new_config)) {
        return Result<bool>(AutoFunError::VALUE_INVALID, "Invalid configuration parameters");
    }
    
    global_config_ = new_config;
    return Result<bool>(true);
}

Result<Config> AutoFunClient::getConfig() const {
    return Result<Config>(global_config_);
}

Result<bool> AutoFunClient::nominateAuthority(const Pubkey& new_admin) {
    if (new_admin.empty()) {
        return Result<bool>(AutoFunError::INCORRECT_AUTHORITY, "Invalid admin address");
    }
    
    global_config_.pending_authority = new_admin;
    return Result<bool>(true);
}

Result<bool> AutoFunClient::acceptAuthority() {
    if (global_config_.pending_authority.empty()) {
        return Result<bool>(AutoFunError::INCORRECT_AUTHORITY, "No pending authority to accept");
    }
    
    global_config_.authority = global_config_.pending_authority;
    global_config_.pending_authority.clear();
    return Result<bool>(true);
}

Result<BondingCurve> AutoFunClient::launch(const LaunchParams& params) {
    // Validate parameters
    if (params.name.empty() || params.symbol.empty()) {
        return Result<BondingCurve>(AutoFunError::VALUE_INVALID, "Token name and symbol cannot be empty");
    }
    
    if (!utils::validateTokenName(params.name) || !utils::validateTokenSymbol(params.symbol)) {
        return Result<BondingCurve>(AutoFunError::VALUE_INVALID, "Invalid token name or symbol");
    }
    
    if (!utils::validateURI(params.uri)) {
        return Result<BondingCurve>(AutoFunError::VALUE_INVALID, "Invalid metadata URI");
    }
    
    // Create new bonding curve
    BondingCurve curve;
    curve.token_mint = params.symbol + "_" + std::to_string(std::time(nullptr)); // Simplified token mint generation
    curve.creator = "creator_" + std::to_string(std::time(nullptr));
    curve.init_lamport = params.virtual_lamport_reserves;
    curve.reserve_lamport = params.virtual_lamport_reserves;
    curve.reserve_token = params.token_supply;
    curve.curve_limit = global_config_.curve_limit;
    curve.is_completed = false;
    
    bonding_curves_[curve.token_mint] = curve;
    
    return Result<BondingCurve>(curve);
}

Result<std::pair<BondingCurve, u64>> AutoFunClient::launchAndSwap(const LaunchAndSwapParams& params) {
    // First launch the token
    auto launch_result = launch(params);
    if (!launch_result.success) {
        return Result<std::pair<BondingCurve, u64>>(launch_result.error, launch_result.error_message);
    }
    
    // Then perform the swap
    SwapParams swap_params;
    swap_params.amount = params.swap_amount;
    swap_params.direction = 0; // Buy
    swap_params.minimum_receive_amount = params.minimum_receive_amount;
    swap_params.deadline = params.deadline;
    
    auto swap_result = swap(launch_result.value.token_mint, swap_params);
    if (!swap_result.success) {
        return Result<std::pair<BondingCurve, u64>>(swap_result.error, swap_result.error_message);
    }
    
    return Result<std::pair<BondingCurve, u64>>(std::make_pair(launch_result.value, swap_result.value));
}

Result<u64> AutoFunClient::swap(const Pubkey& token_mint, const SwapParams& params) {
    auto it = bonding_curves_.find(token_mint);
    if (it == bonding_curves_.end()) {
        return Result<u64>(AutoFunError::VALUE_INVALID, "Token not found");
    }
    
    BondingCurve& curve = it->second;
    
    if (curve.is_completed) {
        return Result<u64>(AutoFunError::CURVE_ALREADY_COMPLETED, "Cannot swap after curve completion");
    }
    
    // Check deadline
    auto now = std::chrono::system_clock::now();
    auto deadline = std::chrono::system_clock::from_time_t(params.deadline);
    if (now > deadline) {
        return Result<u64>(AutoFunError::TRANSACTION_EXPIRED, "Transaction expired");
    }
    
    // Calculate swap output
    u64 output = calculateSwapOutput(curve, params.amount, params.direction == 0);
    
    if (output < params.minimum_receive_amount) {
        return Result<u64>(AutoFunError::RETURN_AMOUNT_TOO_SMALL, "Output amount too small");
    }
    
    // Update curve state
    if (params.direction == 0) { // Buy
        curve.reserve_lamport += params.amount;
        curve.reserve_token -= output;
    } else { // Sell
        curve.reserve_lamport -= output;
        curve.reserve_token += params.amount;
    }
    
    // Check if curve is completed
    if (curve.reserve_lamport >= curve.curve_limit) {
        curve.is_completed = true;
    }
    
    return Result<u64>(output);
}

Result<bool> AutoFunClient::withdraw(const Pubkey& token_mint) {
    auto it = bonding_curves_.find(token_mint);
    if (it == bonding_curves_.end()) {
        return Result<bool>(AutoFunError::VALUE_INVALID, "Token not found");
    }
    
    if (!it->second.is_completed) {
        return Result<bool>(AutoFunError::CURVE_NOT_COMPLETED, "Curve must be completed before withdrawal");
    }
    
    return Result<bool>(true);
}

Result<BondingCurve> AutoFunClient::getBondingCurve(const Pubkey& token_mint) const {
    auto it = bonding_curves_.find(token_mint);
    if (it == bonding_curves_.end()) {
        return Result<BondingCurve>(AutoFunError::VALUE_INVALID, "Token not found");
    }
    
    return Result<BondingCurve>(it->second);
}

std::vector<BondingCurve> AutoFunClient::getAllBondingCurves() const {
    std::vector<BondingCurve> curves;
    for (const auto& pair : bonding_curves_) {
        curves.push_back(pair.second);
    }
    return curves;
}

bool AutoFunClient::validateConfig(const Config& config) const {
    return !config.authority.empty() && 
           !config.team_wallet.empty() && 
           config.curve_limit > 0 &&
           config.init_bonding_curve > 0.0;
}

u64 AutoFunClient::calculateSwapOutput(const BondingCurve& curve, u64 input_amount, bool is_buy) const {
    // Simplified bonding curve calculation: constant product formula
    // For buy: output_tokens = (input_lamports * reserve_tokens) / (reserve_lamports + input_lamports)
    // For sell: output_lamports = (input_tokens * reserve_lamports) / (reserve_tokens + input_tokens)
    
    if (is_buy) {
        if (curve.reserve_lamport == 0) return 0;
        return (input_amount * curve.reserve_token) / (curve.reserve_lamport + input_amount);
    } else {
        if (curve.reserve_token == 0) return 0;
        return (input_amount * curve.reserve_lamport) / (curve.reserve_token + input_amount);
    }
}

bool AutoFunClient::isCurveCompleted(const Pubkey& token_mint) const {
    auto it = bonding_curves_.find(token_mint);
    return it != bonding_curves_.end() && it->second.is_completed;
}

// Utility functions implementation
namespace utils {

std::string generateTokenMetadata(const std::string& name, const std::string& symbol, const std::string& uri) {
    std::ostringstream metadata;
    metadata << "{"
             << "\"name\":\"" << name << "\","
             << "\"symbol\":\"" << symbol << "\","
             << "\"uri\":\"" << uri << "\""
             << "}";
    return metadata.str();
}

bool validateTokenName(const std::string& name) {
    return !name.empty() && name.length() <= 32 && 
           std::all_of(name.begin(), name.end(), [](char c) {
               return std::isalnum(c) || c == ' ' || c == '_' || c == '-';
           });
}

bool validateTokenSymbol(const std::string& symbol) {
    return !symbol.empty() && symbol.length() <= 10 &&
           std::all_of(symbol.begin(), symbol.end(), [](char c) {
               return std::isupper(c) || std::isdigit(c);
           });
}

bool validateURI(const std::string& uri) {
    if (uri.empty()) return true; // URI is optional
    
    // Basic URI validation
    std::regex uri_pattern(R"(^https?:\/\/.+)");
    return std::regex_match(uri, uri_pattern);
}

u64 calculateBondingCurvePrice(u64 supply, f64 curve_factor) {
    // Price = curve_factor * supply^2
    return static_cast<u64>(curve_factor * supply * supply);
}

std::string formatError(AutoFunError error) {
    switch (error) {
        case AutoFunError::VALUE_TOO_SMALL: return "ValueTooSmall";
        case AutoFunError::VALUE_TOO_LARGE: return "ValueTooLarge";
        case AutoFunError::VALUE_INVALID: return "ValueInvalid";
        case AutoFunError::INCORRECT_CONFIG_ACCOUNT: return "IncorrectConfigAccount";
        case AutoFunError::INCORRECT_AUTHORITY: return "IncorrectAuthority";
        case AutoFunError::OVERFLOW_OR_UNDERFLOW_OCCURRED: return "OverflowOrUnderflowOccurred";
        case AutoFunError::INVALID_AMOUNT: return "InvalidAmount";
        case AutoFunError::INCORRECT_TEAM_WALLET: return "IncorrectTeamWallet";
        case AutoFunError::CURVE_NOT_COMPLETED: return "CurveNotCompleted";
        case AutoFunError::CURVE_ALREADY_COMPLETED: return "CurveAlreadyCompleted";
        case AutoFunError::MINT_AUTHORITY_ENABLED: return "MintAuthorityEnabled";
        case AutoFunError::FREEZE_AUTHORITY_ENABLED: return "FreezeAuthorityEnabled";
        case AutoFunError::RETURN_AMOUNT_TOO_SMALL: return "ReturnAmountTooSmall";
        case AutoFunError::TRANSACTION_EXPIRED: return "TransactionExpired";
        case AutoFunError::DECIMAL_OVERFLOW: return "DecimalOverflow";
        default: return "UnknownError";
    }
}

} // namespace utils
} // namespace auto_fun
} // namespace elizaos
