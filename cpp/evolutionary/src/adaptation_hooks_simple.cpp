#include "elizaos/evolutionary.hpp"
#include "elizaos/core.hpp"
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
    
    void onPatternDiscovered(const PatternExtractor::Pattern& pattern, const State& /*state*/) override {
        std::cout << "  📊 Pattern discovered: " << pattern.name 
                 << " (effectiveness: " << pattern.effectiveness << ")" << std::endl;
        
        // Increase learning rate when finding effective patterns
        if (pattern.effectiveness > 0.8) {
            currentLearningRate_ = std::min(1.0, currentLearningRate_ * 1.1);
            std::cout << "  🔧 Increased learning rate to " << currentLearningRate_ << std::endl;
        }
    }
    
    void onFitnessImprovement(const Individual& /*individual*/, 
                             const FitnessResult& oldFitness,
                             const FitnessResult& newFitness,
                             const State& /*state*/) override {
        double improvement = newFitness.fitness - oldFitness.fitness;
        
        if (improvement > 0.1) {
            std::cout << "  🎯 Significant fitness improvement: " << improvement << std::endl;
            
            // Reduce learning rate when making good progress
            currentLearningRate_ = std::max(0.01, currentLearningRate_ * 0.95);
        }
    }
    
    void onConvergence(const Population& population, const State& /*state*/) override {
        std::cout << "  🎯 Population converged (diversity: " 
                 << population.getDiversity() << ")" << std::endl;
        
        // Reset learning rate on convergence
        currentLearningRate_ = 0.1;
    }
    
    void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& stats,
                           EvolutionaryOptimizer::Config& config) override {
        // Adapt mutation rate based on diversity
        if (stats.diversity < 0.1) {
            config.mutationRate = std::min(0.5, config.mutationRate * 1.2);
            std::cout << "  🔧 Low diversity, increased mutation rate to " << config.mutationRate << std::endl;
        } else if (stats.diversity > 0.8) {
            config.mutationRate = std::max(0.01, config.mutationRate * 0.8);
            std::cout << "  🔧 High diversity, reduced mutation rate to " << config.mutationRate << std::endl;
        }
        
        // Adapt population size based on stagnation
        if (stats.stagnationCount > 20) {
            config.populationSize = std::min(size_t(500), config.populationSize + 10);
            std::cout << "  🔧 Stagnation detected, increased population size to " << config.populationSize << std::endl;
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
            elizaos::generateUUID(),
            "Discovered pattern: " + pattern.name + " with effectiveness " + std::to_string(pattern.effectiveness),
            state.getAgentId(),
            state.getAgentId()
        );
        
        // Add pattern metadata - simplified for demo
        CustomMetadata metadata;
        memory->setMetadata(metadata);
        
        // In a full implementation, this would be stored in the agent's memory system
        discoveredPatterns_.push_back(pattern);
        
        std::cout << "  💾 Stored pattern in memory: " << pattern.name << std::endl;
    }
    
    void onFitnessImprovement(const Individual& /*individual*/, 
                             const FitnessResult& /*oldFitness*/,
                             const FitnessResult& newFitness,
                             const State& /*state*/) override {
        // Store successful strategies
        if (newFitness.fitness > 0.9) {
            std::cout << "  💾 Stored high-performing strategy (fitness: " 
                     << newFitness.fitness << ")" << std::endl;
        }
    }
    
    void onConvergence(const Population& population, const State& /*state*/) override {
        // Analyze convergence patterns
        auto bestIndividuals = population.eliteSelection(5);
        
        std::cout << "  💾 Stored " << bestIndividuals.size() << " convergence examples" << std::endl;
    }
    
    void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& /*stats*/,
                           EvolutionaryOptimizer::Config& config) override {
        // Use historical patterns to inform future searches
        if (discoveredPatterns_.size() > 10) {
            // Bias towards previously successful patterns
            config.eliteRatio = std::min(0.3, config.eliteRatio + 0.05);
            
            std::cout << "  💾 Adjusted elite ratio based on pattern history: " 
                     << config.eliteRatio << std::endl;
        }
    }
    
    const std::vector<PatternExtractor::Pattern>& getDiscoveredPatterns() const {
        return discoveredPatterns_;
    }
    
private:
    std::vector<PatternExtractor::Pattern> discoveredPatterns_;
};

// Performance monitoring hook
class PerformanceMonitoringHook : public AdaptationHook {
public:
    PerformanceMonitoringHook() : totalImprovements_(0), totalPatterns_(0) {}
    
    void onPatternDiscovered(const PatternExtractor::Pattern& /*pattern*/, const State& /*state*/) override {
        totalPatterns_++;
        
        std::cout << "  📊 Pattern discovery rate: " << totalPatterns_ 
                 << " patterns discovered" << std::endl;
    }
    
    void onFitnessImprovement(const Individual& /*individual*/, 
                             const FitnessResult& oldFitness,
                             const FitnessResult& newFitness,
                             const State& /*state*/) override {
        totalImprovements_++;
        double improvement = newFitness.fitness - oldFitness.fitness;
        
        std::cout << "  📈 Fitness improvement: " << improvement 
                 << " (total improvements: " << totalImprovements_ << ")" << std::endl;
    }
    
    void onConvergence(const Population& population, const State& /*state*/) override {
        convergenceEvents_++;
        
        // Log convergence statistics
        auto bestFitness = population.getBestFitness();
        auto avgFitness = population.getAverageFitness();
        
        std::cout << "  📊 Convergence #" << convergenceEvents_ 
                 << " - Best: " << bestFitness.fitness 
                 << ", Avg: " << avgFitness.fitness 
                 << ", Diversity: " << population.getDiversity() << std::endl;
    }
    
    void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& stats,
                           EvolutionaryOptimizer::Config& /*config*/) override {
        // Monitor and log adaptation efficiency
        if (stats.generation % 10 == 0) {
            std::cout << "  📊 Generation " << stats.generation 
                     << " - Best fitness: " << stats.bestFitness.fitness
                     << ", Diversity: " << stats.diversity << std::endl;
        }
    }
    
private:
    int totalImprovements_;
    int totalPatterns_;
    int convergenceEvents_ = 0;
};

} // namespace elizaos