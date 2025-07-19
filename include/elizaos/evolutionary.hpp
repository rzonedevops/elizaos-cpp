#pragma once

#include "core.hpp"
#include <random>
#include <algorithm>
#include <functional>
#include <numeric>
#include <atomic>
#include <future>

namespace elizaos {

// Forward declarations
class EvolutionaryProgram;
class EvolutionaryOptimizer;
class PatternExtractor;
class AdaptationHook;

/**
 * Evolutionary search framework inspired by MOSES (Meta-Optimizing Semantic Evolutionary Search)
 * 
 * This framework implements evolutionary algorithms for optimizing agent behavior,
 * learning patterns, and adapting to new environments.
 */

// Program representation for evolutionary search
struct ProgramNode {
    enum class Type {
        CONSTANT,
        VARIABLE,
        FUNCTION,
        CONDITIONAL
    };
    
    Type type;
    std::string name;
    std::vector<double> parameters;
    std::vector<std::shared_ptr<ProgramNode>> children;
    
    ProgramNode(Type t, const std::string& n) : type(t), name(n) {}
    
    // Deep copy
    std::shared_ptr<ProgramNode> clone() const;
    
    // Evaluate the node given a context
    double evaluate(const std::unordered_map<std::string, double>& context) const;
    
    // Get string representation
    std::string toString() const;
};

// Fitness evaluation result
struct FitnessResult {
    double fitness;
    double complexity;
    double novelty;
    std::vector<double> behaviorSignature;
    std::string description;
    
    FitnessResult() : fitness(0.0), complexity(0.0), novelty(0.0) {}
    FitnessResult(double f, double c = 0.0, double n = 0.0) 
        : fitness(f), complexity(c), novelty(n) {}
    
    // Overall score combining fitness, complexity, and novelty
    double getOverallScore() const {
        return fitness - 0.1 * complexity + 0.05 * novelty;
    }
};

// Individual in the evolutionary population
class Individual {
public:
    Individual(std::shared_ptr<ProgramNode> program);
    Individual(const Individual& other);
    Individual& operator=(const Individual& other);
    
    // Get the program representation
    std::shared_ptr<ProgramNode> getProgram() const { return program_; }
    
    // Fitness evaluation
    FitnessResult getFitness() const { return fitness_; }
    void setFitness(const FitnessResult& fitness) { fitness_ = fitness; }
    
    // Age tracking
    int getAge() const { return age_; }
    void incrementAge() { age_++; }
    
    // Genetic operations
    static Individual crossover(const Individual& parent1, const Individual& parent2);
    Individual mutate(double mutationRate = 0.1) const;
    
    // Similarity comparison
    double similarity(const Individual& other) const;
    
    // Serialization
    std::string serialize() const;
    static Individual deserialize(const std::string& data);
    
private:
    std::shared_ptr<ProgramNode> program_;
    FitnessResult fitness_;
    int age_ = 0;
    UUID id_;
    
    // Helper methods for genetic operations
    std::shared_ptr<ProgramNode> subtreeCrossover(
        std::shared_ptr<ProgramNode> parent1, 
        std::shared_ptr<ProgramNode> parent2) const;
    std::shared_ptr<ProgramNode> subtreeMutate(
        std::shared_ptr<ProgramNode> node, 
        double mutationRate) const;
};

// Population management
class Population {
public:
    Population(size_t maxSize = 100);
    
    // Population management
    void addIndividual(const Individual& individual);
    void removeIndividual(size_t index);
    size_t size() const { return individuals_.size(); }
    bool empty() const { return individuals_.empty(); }
    
    // Access individuals
    const Individual& getIndividual(size_t index) const;
    Individual& getIndividual(size_t index);
    std::vector<Individual>& getIndividuals() { return individuals_; }
    const std::vector<Individual>& getIndividuals() const { return individuals_; }
    
    // Population statistics
    FitnessResult getBestFitness() const;
    FitnessResult getAverageFitness() const;
    double getDiversity() const;
    
    // Selection methods
    std::vector<Individual> tournamentSelection(size_t tournamentSize, size_t numSelected) const;
    std::vector<Individual> rouletteWheelSelection(size_t numSelected) const;
    std::vector<Individual> eliteSelection(size_t numElite) const;
    
    // Population operations
    void sort(); // Sort by fitness
    void ageIndividuals(); // Increment age of all individuals
    void clear() { individuals_.clear(); }
    
private:
    std::vector<Individual> individuals_;
    size_t maxSize_;
    mutable std::mutex populationMutex_;
};

// Fitness evaluation function type
using FitnessFunction = std::function<FitnessResult(const Individual&, const State&)>;

// Evolutionary optimizer implementing MOSES-style search
class EvolutionaryOptimizer {
public:
    struct Config {
        size_t populationSize = 100;
        size_t maxGenerations = 1000;
        double mutationRate = 0.1;
        double crossoverRate = 0.8;
        double eliteRatio = 0.1;
        size_t tournamentSize = 3;
        double diversityThreshold = 0.1;
        bool useDemeSplitting = true;
        bool useNoveltySearch = true;
        int maxComplexity = 50;
        double stagnationThreshold = 0.001;
        int maxStagnationGenerations = 50;
    };
    
    EvolutionaryOptimizer(const Config& config);
    ~EvolutionaryOptimizer();
    
    // Main optimization loop
    Individual optimize(const FitnessFunction& fitnessFunc, const State& state);
    
    // Optimization with initial population
    Individual optimize(const FitnessFunction& fitnessFunc, const State& state, 
                       const std::vector<Individual>& initialPopulation);
    
    // Asynchronous optimization
    std::future<Individual> optimizeAsync(const FitnessFunction& fitnessFunc, const State& state);
    
    // Population management
    void setPopulation(const Population& population);
    std::shared_ptr<Population> getPopulation() const;
    
    // Configuration
    void setConfig(const Config& config) { config_ = config; }
    Config getConfig() const { return config_; }
    
    // Optimization control
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    void stop() { stopped_ = true; }
    bool isRunning() const { return running_; }
    
    // Statistics
    struct Statistics {
        int generation;
        FitnessResult bestFitness;
        FitnessResult averageFitness;
        double diversity;
        double convergenceRate;
        int stagnationCount;
        std::chrono::milliseconds generationTime;
    };
    
    Statistics getStatistics() const;
    std::vector<Statistics> getHistory() const;
    
private:
    Config config_;
    Population population_;
    std::vector<Statistics> history_;
    
    // Evolution state
    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::atomic<bool> stopped_{false};
    
    // Random number generation
    mutable std::mt19937 rng_{std::random_device{}()};
    
    // Evolution methods
    void evolveGeneration(const FitnessFunction& fitnessFunc, const State& state);
    void evaluateFitness(const FitnessFunction& fitnessFunc, const State& state);
    void selectParents(std::vector<Individual>& parents);
    void reproduction(const std::vector<Individual>& parents, std::vector<Individual>& offspring);
    void environmentalSelection(const std::vector<Individual>& offspring);
    
    // Specialized MOSES techniques
    void demeSplitting();
    void noveltySearch();
    void complexityControl();
    
    // Utility methods
    std::shared_ptr<ProgramNode> generateRandomProgram(int maxDepth = 5) const;
    bool checkStagnation() const;
    void updateStatistics(int generation);
};

// Pattern extraction from successful individuals
class PatternExtractor {
public:
    struct Pattern {
        std::string name;
        std::shared_ptr<ProgramNode> structure;
        double frequency;
        double effectiveness;
        std::vector<std::string> contexts;
        
        Pattern(const std::string& n, std::shared_ptr<ProgramNode> s) 
            : name(n), structure(s), frequency(0.0), effectiveness(0.0) {}
    };
    
    PatternExtractor();
    
    // Extract patterns from successful individuals
    std::vector<Pattern> extractPatterns(const std::vector<Individual>& individuals, 
                                        double minFitness = 0.8) const;
    
    // Extract patterns from evolution history
    std::vector<Pattern> extractPatternsFromHistory(
        const std::vector<EvolutionaryOptimizer::Statistics>& history) const;
    
    // Pattern analysis
    std::vector<Pattern> getCommonPatterns(const std::vector<Pattern>& patterns, 
                                          double minFrequency = 0.1) const;
    
    // Pattern similarity
    double patternSimilarity(const Pattern& p1, const Pattern& p2) const;
    
    // Save/load patterns
    void savePatterns(const std::vector<Pattern>& patterns, const std::string& filename) const;
    std::vector<Pattern> loadPatterns(const std::string& filename) const;
    
private:
    // Pattern extraction algorithms
    std::vector<Pattern> extractSubtreePatterns(const std::vector<Individual>& individuals) const;
    std::vector<Pattern> extractBehaviorPatterns(const std::vector<Individual>& individuals) const;
    std::vector<Pattern> extractStructuralPatterns(const std::vector<Individual>& individuals) const;
    
    // Pattern utilities
    std::shared_ptr<ProgramNode> findCommonSubtree(
        const std::vector<std::shared_ptr<ProgramNode>>& programs) const;
    std::vector<std::shared_ptr<ProgramNode>> getAllSubtrees(
        std::shared_ptr<ProgramNode> program) const;
};

// Adaptation hooks for integrating learning into the agent system
class AdaptationHook {
public:
    virtual ~AdaptationHook() = default;
    
    // Called when a new pattern is discovered
    virtual void onPatternDiscovered(const PatternExtractor::Pattern& pattern, 
                                    const State& state) = 0;
    
    // Called when fitness improves significantly
    virtual void onFitnessImprovement(const Individual& individual, 
                                     const FitnessResult& oldFitness,
                                     const FitnessResult& newFitness,
                                     const State& state) = 0;
    
    // Called when population converges
    virtual void onConvergence(const Population& population, 
                              const State& state) = 0;
    
    // Called when adaptation parameters should be adjusted
    virtual void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& stats,
                                   EvolutionaryOptimizer::Config& config) = 0;
};

// Optimization pipeline for coordinating multiple evolutionary processes
class OptimizationPipeline {
public:
    struct Stage {
        std::string name;
        FitnessFunction fitnessFunc;
        EvolutionaryOptimizer::Config config;
        std::vector<std::shared_ptr<AdaptationHook>> hooks;
        
        Stage(const std::string& n, const FitnessFunction& f) 
            : name(n), fitnessFunc(f) {}
    };
    
    OptimizationPipeline();
    ~OptimizationPipeline();
    
    // Pipeline management
    void addStage(const Stage& stage);
    void removeStage(const std::string& name);
    void setStageOrder(const std::vector<std::string>& order);
    
    // Execution
    Individual runPipeline(const State& state);
    std::future<Individual> runPipelineAsync(const State& state);
    
    // Pipeline control
    void pause() { paused_ = true; }
    void resume() { paused_ = false; }
    void stop() { stopped_ = true; }
    bool isRunning() const { return running_; }
    
    // Results and statistics
    struct PipelineResult {
        Individual bestIndividual{nullptr};
        std::vector<Individual> stageResults;
        std::vector<EvolutionaryOptimizer::Statistics> stageStatistics;
        std::vector<PatternExtractor::Pattern> extractedPatterns;
        std::chrono::milliseconds totalTime{0};
    };
    
    PipelineResult getLastResult() const;
    
    // Adaptation hooks
    void addGlobalHook(std::shared_ptr<AdaptationHook> hook);
    void removeGlobalHook(std::shared_ptr<AdaptationHook> hook);
    
private:
    std::vector<Stage> stages_;
    std::vector<std::string> stageOrder_;
    std::vector<std::shared_ptr<AdaptationHook>> globalHooks_;
    
    PipelineResult lastResult_;
    PatternExtractor patternExtractor_;
    
    // Pipeline state
    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::atomic<bool> stopped_{false};
    
    // Execution methods
    Individual runStage(const Stage& stage, const State& state, 
                       const Individual& input = Individual(nullptr));
    void notifyHooks(const Stage& stage, const Individual& result, const State& state);
};

} // namespace elizaos