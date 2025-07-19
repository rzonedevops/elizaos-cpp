#include "elizaos/evolutionary.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>

namespace elizaos {

/**
 * Example adaptation hooks that demonstrate integration with the agent system
 */

// Learning rate adaptation hook
class LearningRateAdaptationHook : public AdaptationHook {
public:
    LearningRateAdaptationHook(double initialRate = 0.1) : currentLearningRate_(initialRate) {}
    
    void onPatternDiscovered(const PatternExtractor::Pattern& pattern, const State& state) override {
        AgentLogger::log("Pattern discovered: " + pattern.name + 
                        " (effectiveness: " + std::to_string(pattern.effectiveness) + ")",
                        "adaptation", "adaptation", LogLevel::INFO);
        
        // Increase learning rate when finding effective patterns
        if (pattern.effectiveness > 0.8) {
            currentLearningRate_ = std::min(1.0, currentLearningRate_ * 1.1);
            AgentLogger::log("adaptation", "Increased learning rate to " + std::to_string(currentLearningRate_),
                            LogLevel::INFO);
        }
    }
    
    void onFitnessImprovement(const Individual& individual, 
                             const FitnessResult& oldFitness,
                             const FitnessResult& newFitness,
                             const State& state) override {
        double improvement = newFitness.fitness - oldFitness.fitness;
        
        if (improvement > 0.1) {
            AgentLogger::log("adaptation", "Significant fitness improvement: " + std::to_string(improvement),
                            LogLevel::INFO);
            
            // Reduce learning rate when making good progress
            currentLearningRate_ = std::max(0.01, currentLearningRate_ * 0.95);
        }
    }
    
    void onConvergence(const Population& population, const State& state) override {
        AgentLogger::log("adaptation", "Population converged. Diversity: " + std::to_string(population.getDiversity()),
                        LogLevel::INFO);
        
        // Reset learning rate on convergence
        currentLearningRate_ = 0.1;
    }
    
    void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& stats,
                           EvolutionaryOptimizer::Config& config) override {
        // Adapt mutation rate based on diversity
        if (stats.diversity < 0.1) {
            config.mutationRate = std::min(0.5, config.mutationRate * 1.2);
            AgentLogger::log("adaptation", "Low diversity, increased mutation rate to " + std::to_string(config.mutationRate),
                            LogLevel::INFO);
        } else if (stats.diversity > 0.8) {
            config.mutationRate = std::max(0.01, config.mutationRate * 0.8);
            AgentLogger::log("adaptation", "High diversity, reduced mutation rate to " + std::to_string(config.mutationRate),
                            LogLevel::INFO);
        }
        
        // Adapt population size based on stagnation
        if (stats.stagnationCount > 20) {
            config.populationSize = std::min(size_t(500), config.populationSize + 10);
            AgentLogger::log("adaptation", "Stagnation detected, increased population size to " + std::to_string(config.populationSize),
                            LogLevel::INFO);
        }
    }
    
    double getCurrentLearningRate() const { return currentLearningRate_; }
    
private:
    double currentLearningRate_;
};

// Memory integration hook
class MemoryIntegrationHook : public AdaptationHook {
public:
    MemoryIntegrationHook() {}
    
    void onPatternDiscovered(const PatternExtractor::Pattern& pattern, const State& state) override {
        // Store successful patterns in agent memory
        auto memory = std::make_shared<Memory>(
            generateUUID(),
            "Discovered pattern: " + pattern.name + " with effectiveness " + std::to_string(pattern.effectiveness),
            state.getAgentId(),
            state.getAgentId()
        );
        
        // Add pattern metadata
        MemoryMetadata metadata;
        metadata.type = MemoryType::CUSTOM;
        metadata.scope = MemoryScope::PRIVATE;
        memory->setMetadata(metadata);
        
        // In a full implementation, this would be stored in the agent's memory system
        discoveredPatterns_.push_back(pattern);
        
        AgentLogger::log("memory", "Stored pattern in memory: " + pattern.name, LogLevel::INFO);
    }
    
    void onFitnessImprovement(const Individual& individual, 
                             const FitnessResult& oldFitness,
                             const FitnessResult& newFitness,
                             const State& state) override {
        // Store successful strategies
        if (newFitness.fitness > 0.9) {
            successfulStrategies_.push_back(individual);
            
            AgentLogger::log("memory", "Stored high-performing strategy (fitness: " + 
                            std::to_string(newFitness.fitness) + ")", LogLevel::INFO);
        }
    }
    
    void onConvergence(const Population& population, const State& state) override {
        // Analyze convergence patterns
        auto bestIndividuals = population.eliteSelection(5);
        
        for (const auto& individual : bestIndividuals) {
            convergenceExamples_.push_back(individual);
        }
        
        AgentLogger::log("memory", "Stored " + std::to_string(bestIndividuals.size()) + " convergence examples",
                        LogLevel::INFO);
    }
    
    void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& stats,
                           EvolutionaryOptimizer::Config& config) override {
        // Use historical patterns to inform future searches
        if (discoveredPatterns_.size() > 10) {
            // Bias towards previously successful patterns
            config.eliteRatio = std::min(0.3, config.eliteRatio + 0.05);
            
            AgentLogger::log("memory", "Adjusted elite ratio based on pattern history: " + 
                            std::to_string(config.eliteRatio), LogLevel::INFO);
        }
    }
    
    const std::vector<PatternExtractor::Pattern>& getDiscoveredPatterns() const {
        return discoveredPatterns_;
    }
    
    const std::vector<Individual>& getSuccessfulStrategies() const {
        return successfulStrategies_;
    }
    
private:
    std::vector<PatternExtractor::Pattern> discoveredPatterns_;
    std::vector<Individual> successfulStrategies_;
    std::vector<Individual> convergenceExamples_;
};

// Performance monitoring hook
class PerformanceMonitoringHook : public AdaptationHook {
public:
    PerformanceMonitoringHook() : totalImprovements_(0), totalPatterns_(0) {}
    
    void onPatternDiscovered(const PatternExtractor::Pattern& pattern, const State& state) override {
        totalPatterns_++;
        patternHistory_.push_back({
            std::chrono::steady_clock::now(),
            pattern.name,
            pattern.effectiveness,
            pattern.frequency
        });
        
        // Log performance metrics
        AgentLogger::log("performance", "Pattern discovery rate: " + std::to_string(totalPatterns_) + 
                        " patterns discovered", LogLevel::INFO);
    }
    
    void onFitnessImprovement(const Individual& individual, 
                             const FitnessResult& oldFitness,
                             const FitnessResult& newFitness,
                             const State& state) override {
        totalImprovements_++;
        double improvement = newFitness.fitness - oldFitness.fitness;
        
        improvementHistory_.push_back({
            std::chrono::steady_clock::now(),
            improvement,
            newFitness.fitness,
            newFitness.complexity
        });
        
        // Calculate improvement rate
        if (improvementHistory_.size() >= 2) {
            auto timeDiff = std::chrono::duration_cast<std::chrono::seconds>(
                improvementHistory_.back().timestamp - improvementHistory_[improvementHistory_.size()-2].timestamp
            );
            
            if (timeDiff.count() > 0) {
                double improvementRate = improvement / timeDiff.count();
                AgentLogger::log("performance", "Improvement rate: " + std::to_string(improvementRate) + " fitness/sec",
                                LogLevel::INFO);
            }
        }
    
    void onConvergence(const Population& population, const State& /*state*/) override {
        convergenceEvents_++;
        
        // Log convergence statistics
        auto bestFitness = population.getBestFitness();
        auto avgFitness = population.getAverageFitness();
        
        AgentLogger::log("performance", "Convergence #" + std::to_string(convergenceEvents_) + 
                        " - Best: " + std::to_string(bestFitness.fitness) + 
                        ", Avg: " + std::to_string(avgFitness.fitness) + 
                        ", Diversity: " + std::to_string(population.getDiversity()),
                        LogLevel::INFO);
    }
    
    void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& stats,
                           EvolutionaryOptimizer::Config& /*config*/) override {
        // Monitor and log adaptation efficiency
        if (stats.generation % 10 == 0) {
            double avgImprovement = 0.0;
            if (!improvementHistory_.empty()) {
                for (const auto& record : improvementHistory_) {
                    avgImprovement += record.improvement;
                }
                avgImprovement /= improvementHistory_.size();
            }
            
            AgentLogger::log("performance", "Generation " + std::to_string(stats.generation) + 
                            " - Avg improvement: " + std::to_string(avgImprovement) + 
                            ", Diversity: " + std::to_string(stats.diversity),
                            LogLevel::INFO);
        }
    }
    
    // Performance analysis methods
    double getAverageImprovementRate() const {
        if (improvementHistory_.size() < 2) return 0.0;
        
        double totalImprovement = 0.0;
        auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(
            improvementHistory_.back().timestamp - improvementHistory_.front().timestamp
        );
        
        for (const auto& record : improvementHistory_) {
            totalImprovement += record.improvement;
        }
        
        return totalTime.count() > 0 ? totalImprovement / totalTime.count() : 0.0;
    }
    
    double getPatternDiscoveryRate() const {
        if (patternHistory_.size() < 2) return 0.0;
        
        auto totalTime = std::chrono::duration_cast<std::chrono::seconds>(
            patternHistory_.back().timestamp - patternHistory_.front().timestamp
        );
        
        return totalTime.count() > 0 ? static_cast<double>(patternHistory_.size()) / totalTime.count() : 0.0;
    }
    
private:
    struct ImprovementRecord {
        std::chrono::steady_clock::time_point timestamp;
        double improvement;
        double fitness;
        double complexity;
    };
    
    struct PatternRecord {
        std::chrono::steady_clock::time_point timestamp;
        std::string name;
        double effectiveness;
        double frequency;
    };
    
    int totalImprovements_;
    int totalPatterns_;
    int convergenceEvents_ = 0;
    std::vector<ImprovementRecord> improvementHistory_;
    std::vector<PatternRecord> patternHistory_;
};

} // namespace elizaos