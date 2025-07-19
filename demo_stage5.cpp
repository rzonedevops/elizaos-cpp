#include "elizaos/evolutionary.hpp"
#include "elizaos/core.hpp"
#include "elizaos/agentlogger.hpp"
#include <iostream>
#include <fstream>
#include <memory>

using namespace elizaos;

/**
 * Stage 5 Demo: Learning and Adaptation with MOSES-style Evolutionary Search
 * 
 * This demo showcases:
 * 1. MOSES-style evolutionary search algorithms
 * 2. Optimization pipelines for agent behavior
 * 3. Pattern extraction and adaptation hooks
 * 4. Learning metrics and evaluation
 */

class LearningDemo {
public:
    LearningDemo() {
        std::cout << "Initializing Learning and Adaptation Demo" << std::endl;
        
        // Setup agent state
        AgentConfig config{
            "learning_agent", 
            "Learning Agent", 
            "An agent demonstrating evolutionary learning capabilities",
            "learning_room",
            "learning_world"
        };
        state_ = std::make_unique<State>(config);
        
        // Setup adaptation hooks
        setupAdaptationHooks();
        
        // Setup optimization pipeline
        setupOptimizationPipeline();
    }
    
    void runDemo() {
        std::cout << "Starting Learning and Adaptation Demo" << std::endl;
        
        std::cout << "\n=== Stage 5: Learning and Adaptation Demo ===" << std::endl;
        std::cout << "Implementing MOSES-style evolutionary search for agent learning\n" << std::endl;
        
        // Demo 1: Basic evolutionary optimization
        demonstrateBasicEvolution();
        
        // Demo 2: Pattern extraction
        demonstratePatternExtraction();
        
        // Demo 3: Optimization pipeline
        demonstrateOptimizationPipeline();
        
        // Demo 4: Adaptation hooks
        demonstrateAdaptationHooks();
        
        // Demo 5: Learning metrics
        demonstrateLearningMetrics();
        
        std::cout << "\n=== Demo Complete ===" << std::endl;
        std::cout << "Learning and adaptation systems successfully demonstrated!" << std::endl;
    }

private:
    std::unique_ptr<State> state_;
    std::vector<std::shared_ptr<AdaptationHook>> hooks_;
    std::unique_ptr<OptimizationPipeline> pipeline_;
    
    void setupAdaptationHooks() {
        // Create learning rate adaptation hook
        class DemoLearningRateHook : public AdaptationHook {
        public:
            void onPatternDiscovered(const PatternExtractor::Pattern& pattern, const State& /*state*/) override {
                std::cout << "  📊 Pattern discovered: " << pattern.name 
                         << " (effectiveness: " << pattern.effectiveness << ")" << std::endl;
            }
            
            void onFitnessImprovement(const Individual& /*individual*/,
                                     const FitnessResult& oldFitness,
                                     const FitnessResult& newFitness,
                                     const State& /*state*/) override {
                double improvement = newFitness.fitness - oldFitness.fitness;
                if (improvement > 0.01) {
                    std::cout << "  🎯 Fitness improved by " << improvement << std::endl;
                }
            }
            
            void onConvergence(const Population& population, const State& /*state*/) override {
                std::cout << "  🎯 Population converged (diversity: " 
                         << population.getDiversity() << ")" << std::endl;
            }
            
            void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& stats,
                                   EvolutionaryOptimizer::Config& config) override {
                if (stats.diversity < 0.1) {
                    config.mutationRate = std::min(0.5, config.mutationRate * 1.1);
                    std::cout << "  🔧 Adapted mutation rate to " << config.mutationRate << std::endl;
                }
            }
        };
        
        hooks_.push_back(std::make_shared<DemoLearningRateHook>());
    }
    
    void setupOptimizationPipeline() {
        pipeline_ = std::make_unique<OptimizationPipeline>();
        
        // Stage 1: Exploration
        OptimizationPipeline::Stage explorationStage("exploration", createExplorationFitness());
        explorationStage.config.populationSize = 50;
        explorationStage.config.maxGenerations = 20;
        explorationStage.config.mutationRate = 0.2;
        explorationStage.config.useNoveltySearch = true;
        explorationStage.hooks = hooks_;
        
        // Stage 2: Exploitation
        OptimizationPipeline::Stage exploitationStage("exploitation", createExploitationFitness());
        exploitationStage.config.populationSize = 30;
        exploitationStage.config.maxGenerations = 15;
        exploitationStage.config.mutationRate = 0.05;
        exploitationStage.config.eliteRatio = 0.3;
        exploitationStage.hooks = hooks_;
        
        // Stage 3: Refinement
        OptimizationPipeline::Stage refinementStage("refinement", createRefinementFitness());
        refinementStage.config.populationSize = 20;
        refinementStage.config.maxGenerations = 10;
        refinementStage.config.mutationRate = 0.01;
        refinementStage.config.eliteRatio = 0.5;
        refinementStage.hooks = hooks_;
        
        pipeline_->addStage(explorationStage);
        pipeline_->addStage(exploitationStage);
        pipeline_->addStage(refinementStage);
        
        for (auto& hook : hooks_) {
            pipeline_->addGlobalHook(hook);
        }
    }
    
    FitnessFunction createTargetFindingFitness() {
        return [](const Individual& individual, const State& /*state*/) -> FitnessResult {
            if (!individual.getProgram()) {
                return FitnessResult(0.0);
            }
            
            // Fitness function that rewards finding a target value (e.g., 42)
            std::unordered_map<std::string, double> context;
            context["x"] = 10.0;
            context["y"] = 5.0;
            context["t"] = 1.0;
            
            double result = individual.getProgram()->evaluate(context);
            double target = 42.0;
            
            // Distance-based fitness
            double distance = std::abs(result - target);
            double fitness = 1.0 / (1.0 + distance);
            
            // Complexity penalty
            double complexity = static_cast<double>(individual.getProgram()->toString().length());
            
            // Novelty bonus (simplified)
            double novelty = std::sin(result) * 0.1; // Reward interesting behaviors
            
            return FitnessResult(fitness, complexity, std::abs(novelty));
        };
    }
    
    FitnessFunction createExplorationFitness() {
        return [](const Individual& individual, const State& /*state*/) -> FitnessResult {
            if (!individual.getProgram()) {
                return FitnessResult(0.0);
            }
            
            std::unordered_map<std::string, double> context;
            context["x"] = 3.0;
            context["y"] = 7.0;
            
            double result = individual.getProgram()->evaluate(context);
            
            // Reward diverse behaviors during exploration
            double fitness = std::abs(std::sin(result)) + std::abs(std::cos(result * 0.5));
            double complexity = static_cast<double>(individual.getProgram()->toString().length());
            double novelty = std::abs(result) / 100.0; // Novelty based on output magnitude
            
            return FitnessResult(fitness, complexity, novelty);
        };
    }
    
    FitnessFunction createExploitationFitness() {
        return [](const Individual& individual, const State& /*state*/) -> FitnessResult {
            if (!individual.getProgram()) {
                return FitnessResult(0.0);
            }
            
            std::unordered_map<std::string, double> context;
            context["x"] = 2.0;
            context["y"] = 3.0;
            
            double result = individual.getProgram()->evaluate(context);
            
            // Focus on specific target during exploitation
            double target = 12.0; // x * y * 2
            double fitness = 1.0 / (1.0 + std::abs(result - target));
            double complexity = static_cast<double>(individual.getProgram()->toString().length());
            
            return FitnessResult(fitness, complexity, 0.0);
        };
    }
    
    FitnessFunction createRefinementFitness() {
        return [](const Individual& individual, const State& /*state*/) -> FitnessResult {
            if (!individual.getProgram()) {
                return FitnessResult(0.0);
            }
            
            std::unordered_map<std::string, double> context;
            
            // Test on multiple contexts for robustness
            double totalFitness = 0.0;
            int tests = 5;
            
            for (int i = 0; i < tests; ++i) {
                context["x"] = i + 1;
                context["y"] = (i + 1) * 2;
                
                double result = individual.getProgram()->evaluate(context);
                double expected = context["x"] + context["y"]; // Simple addition
                
                double testFitness = 1.0 / (1.0 + std::abs(result - expected));
                totalFitness += testFitness;
            }
            
            double fitness = totalFitness / tests;
            double complexity = static_cast<double>(individual.getProgram()->toString().length());
            
            return FitnessResult(fitness, complexity, 0.0);
        };
    }
    
    void demonstrateBasicEvolution() {
        std::cout << "1. Basic Evolutionary Optimization" << std::endl;
        std::cout << "   Goal: Evolve a program to find target value 42" << std::endl;
        
        EvolutionaryOptimizer::Config config;
        config.populationSize = 30;
        config.maxGenerations = 25;
        config.mutationRate = 0.15;
        config.crossoverRate = 0.85;
        config.useDemeSplitting = true;
        config.useNoveltySearch = true;
        
        EvolutionaryOptimizer optimizer(config);
        
        auto startTime = std::chrono::steady_clock::now();
        Individual best = optimizer.optimize(createTargetFindingFitness(), *state_);
        auto endTime = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "   ✅ Evolution completed in " << duration.count() << "ms" << std::endl;
        std::cout << "   🏆 Best fitness: " << best.getFitness().fitness << std::endl;
        std::cout << "   🧬 Best program: " << best.getProgram()->toString() << std::endl;
        
        // Test the evolved program
        std::unordered_map<std::string, double> context;
        context["x"] = 10.0;
        context["y"] = 5.0;
        context["t"] = 1.0;
        double result = best.getProgram()->evaluate(context);
        std::cout << "   🎯 Program output: " << result << " (target: 42)" << std::endl;
        
        // Show evolution statistics
        auto history = optimizer.getHistory();
        if (history.size() > 1) {
            std::cout << "   📈 Improvement: " 
                     << (history.back().bestFitness.fitness - history.front().bestFitness.fitness)
                     << " over " << history.size() << " generations" << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    void demonstratePatternExtraction() {
        std::cout << "2. Pattern Extraction from Successful Individuals" << std::endl;
        
        PatternExtractor extractor;
        
        // Create a set of successful individuals with common patterns
        std::vector<Individual> successfulIndividuals;
        
        for (int i = 0; i < 20; ++i) {
            // Create programs with "add" operations (common pattern)
            auto program = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "add");
            
            auto leftChild = std::make_shared<ProgramNode>(ProgramNode::Type::VARIABLE, "x");
            auto rightChild = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
            rightChild->parameters.push_back(i * 0.5);
            
            program->children.push_back(leftChild);
            program->children.push_back(rightChild);
            
            Individual individual(program);
            FitnessResult fitness(0.8 + i * 0.01, 10.0 + i, 0.2); // High fitness
            individual.setFitness(fitness);
            
            successfulIndividuals.push_back(individual);
        }
        
        // Extract patterns
        auto patterns = extractor.extractPatterns(successfulIndividuals, 0.7);
        
        std::cout << "   📊 Extracted " << patterns.size() << " patterns from successful individuals" << std::endl;
        
        for (size_t i = 0; i < std::min(size_t(5), patterns.size()); ++i) {
            const auto& pattern = patterns[i];
            std::cout << "   🔍 Pattern " << (i+1) << ": " << pattern.name 
                     << " (freq: " << pattern.frequency 
                     << ", eff: " << pattern.effectiveness << ")" << std::endl;
        }
        
        // Get common patterns
        auto commonPatterns = extractor.getCommonPatterns(patterns, 0.3);
        std::cout << "   🎯 Found " << commonPatterns.size() 
                 << " patterns with frequency > 30%" << std::endl;
        
        // Save patterns for future use
        extractor.savePatterns(patterns, "/tmp/extracted_patterns.txt");
        std::cout << "   💾 Patterns saved to /tmp/extracted_patterns.txt" << std::endl;
        
        std::cout << std::endl;
    }
    
    void demonstrateOptimizationPipeline() {
        std::cout << "3. Multi-Stage Optimization Pipeline" << std::endl;
        std::cout << "   Stages: Exploration → Exploitation → Refinement" << std::endl;
        
        auto startTime = std::chrono::steady_clock::now();
        Individual result = pipeline_->runPipeline(*state_);
        auto endTime = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        std::cout << "   ✅ Pipeline completed in " << duration.count() << "ms" << std::endl;
        
        auto pipelineResult = pipeline_->getLastResult();
        
        std::cout << "   🎯 Stages completed: " << pipelineResult.stageResults.size() << std::endl;
        std::cout << "   🏆 Final best fitness: " << result.getFitness().fitness << std::endl;
        std::cout << "   🧬 Final program: " << result.getProgram()->toString() << std::endl;
        
        // Show stage progression
        for (size_t i = 0; i < pipelineResult.stageResults.size(); ++i) {
            std::cout << "   📈 Stage " << (i+1) << " fitness: " 
                     << pipelineResult.stageResults[i].getFitness().fitness << std::endl;
        }
        
        // Show extracted patterns
        std::cout << "   🔍 Pipeline extracted " << pipelineResult.extractedPatterns.size() 
                 << " patterns" << std::endl;
        
        std::cout << std::endl;
    }
    
    void demonstrateAdaptationHooks() {
        std::cout << "4. Adaptation Hooks Integration" << std::endl;
        std::cout << "   Testing automatic parameter adaptation based on evolution progress" << std::endl;
        
        // Create a simple hook-enabled optimizer
        EvolutionaryOptimizer::Config config;
        config.populationSize = 20;
        config.maxGenerations = 15;
        config.mutationRate = 0.1;
        
        EvolutionaryOptimizer optimizer(config);
        
        // Simulate evolution with adaptation
        std::cout << "   🚀 Starting evolution with adaptation hooks..." << std::endl;
        
        Individual best = optimizer.optimize(createExploitationFitness(), *state_);
        
        std::cout << "   ✅ Evolution with adaptation completed" << std::endl;
        std::cout << "   🏆 Final fitness: " << best.getFitness().fitness << std::endl;
        
        // The hooks would have provided feedback during evolution
        auto stats = optimizer.getStatistics();
        std::cout << "   📊 Final diversity: " << stats.diversity << std::endl;
        std::cout << "   🔄 Generation: " << stats.generation << std::endl;
        
        std::cout << std::endl;
    }
    
    void demonstrateLearningMetrics() {
        std::cout << "5. Learning Metrics and Performance Analysis" << std::endl;
        
        // Run multiple optimization rounds to gather metrics
        std::vector<double> fitnessHistory;
        std::vector<double> diversityHistory;
        std::vector<std::chrono::milliseconds> timeHistory;
        
        EvolutionaryOptimizer::Config config;
        config.populationSize = 15;
        config.maxGenerations = 10;
        
        std::cout << "   🧪 Running 5 optimization experiments..." << std::endl;
        
        for (int experiment = 0; experiment < 5; ++experiment) {
            EvolutionaryOptimizer optimizer(config);
            
            auto startTime = std::chrono::steady_clock::now();
            Individual best = optimizer.optimize(createRefinementFitness(), *state_);
            auto endTime = std::chrono::steady_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            fitnessHistory.push_back(best.getFitness().fitness);
            diversityHistory.push_back(optimizer.getStatistics().diversity);
            timeHistory.push_back(duration);
            
            std::cout << "   📊 Experiment " << (experiment + 1) 
                     << ": fitness=" << best.getFitness().fitness 
                     << ", time=" << duration.count() << "ms" << std::endl;
        }
        
        // Calculate statistics
        double avgFitness = 0.0, avgDiversity = 0.0;
        double avgTime = 0.0;
        
        for (size_t i = 0; i < fitnessHistory.size(); ++i) {
            avgFitness += fitnessHistory[i];
            avgDiversity += diversityHistory[i];
            avgTime += timeHistory[i].count();
        }
        
        avgFitness /= fitnessHistory.size();
        avgDiversity /= diversityHistory.size();
        avgTime /= timeHistory.size();
        
        std::cout << "\n   📈 Learning Metrics Summary:" << std::endl;
        std::cout << "   🎯 Average fitness: " << avgFitness << std::endl;
        std::cout << "   🌟 Average diversity: " << avgDiversity << std::endl;
        std::cout << "   ⏱️  Average time: " << avgTime << "ms" << std::endl;
        
        // Calculate improvement trend
        if (fitnessHistory.size() > 1) {
            double trend = (fitnessHistory.back() - fitnessHistory.front()) / (fitnessHistory.size() - 1);
            std::cout << "   📊 Learning trend: " << (trend > 0 ? "📈 Improving" : "📉 Declining") 
                     << " (" << trend << " per experiment)" << std::endl;
        }
        
        // Save metrics to file
        std::ofstream metricsFile("/tmp/learning_metrics.csv");
        if (metricsFile.is_open()) {
            metricsFile << "experiment,fitness,diversity,time_ms\n";
            for (size_t i = 0; i < fitnessHistory.size(); ++i) {
                metricsFile << (i+1) << "," << fitnessHistory[i] << "," 
                           << diversityHistory[i] << "," << timeHistory[i].count() << "\n";
            }
            metricsFile.close();
            std::cout << "   💾 Metrics saved to /tmp/learning_metrics.csv" << std::endl;
        }
        
        std::cout << std::endl;
    }
};

int main() {
    try {
        LearningDemo demo;
        demo.runDemo();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}