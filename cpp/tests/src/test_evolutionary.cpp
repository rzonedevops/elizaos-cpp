#include <gtest/gtest.h>
#include "elizaos/evolutionary.hpp"
#include "elizaos/core.hpp"
#include <thread>
#include <chrono>

using namespace elizaos;

class EvolutionaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test state
        AgentConfig config{"test_agent", "Test Agent", "A test agent", "test_room", "test_world"};
        state_ = std::make_unique<State>(config);
    }
    
    void TearDown() override {
        state_.reset();
    }
    
    std::unique_ptr<State> state_;
};

// Test ProgramNode basic functionality
TEST_F(EvolutionaryTest, ProgramNodeCreationAndEvaluation) {
    // Test constant node
    auto constantNode = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    constantNode->parameters.push_back(5.0);
    
    std::unordered_map<std::string, double> context;
    EXPECT_DOUBLE_EQ(constantNode->evaluate(context), 5.0);
    
    // Test variable node
    auto variableNode = std::make_shared<ProgramNode>(ProgramNode::Type::VARIABLE, "x");
    context["x"] = 10.0;
    EXPECT_DOUBLE_EQ(variableNode->evaluate(context), 10.0);
    
    // Test function node (addition)
    auto addNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "add");
    addNode->children.push_back(constantNode);
    addNode->children.push_back(variableNode);
    
    EXPECT_DOUBLE_EQ(addNode->evaluate(context), 15.0);
}

TEST_F(EvolutionaryTest, ProgramNodeCloning) {
    auto originalNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "add");
    auto constantChild = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    constantChild->parameters.push_back(3.0);
    originalNode->children.push_back(constantChild);
    
    auto clonedNode = originalNode->clone();
    
    EXPECT_EQ(clonedNode->type, originalNode->type);
    EXPECT_EQ(clonedNode->name, originalNode->name);
    EXPECT_EQ(clonedNode->children.size(), originalNode->children.size());
    
    // Verify deep copy
    EXPECT_NE(clonedNode.get(), originalNode.get());
    EXPECT_NE(clonedNode->children[0].get(), originalNode->children[0].get());
}

TEST_F(EvolutionaryTest, IndividualCreationAndFitness) {
    auto program = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    program->parameters.push_back(1.0);
    
    Individual individual(program);
    
    EXPECT_TRUE(individual.getProgram() != nullptr);
    EXPECT_EQ(individual.getAge(), 0);
    
    // Test fitness assignment
    FitnessResult fitness(0.8, 10.0, 0.2);
    individual.setFitness(fitness);
    
    EXPECT_DOUBLE_EQ(individual.getFitness().fitness, 0.8);
    EXPECT_DOUBLE_EQ(individual.getFitness().complexity, 10.0);
    EXPECT_DOUBLE_EQ(individual.getFitness().novelty, 0.2);
}

TEST_F(EvolutionaryTest, IndividualCrossover) {
    // Create two parent individuals
    auto program1 = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "add");
    auto const1 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    const1->parameters.push_back(1.0);
    auto const2 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    const2->parameters.push_back(2.0);
    program1->children.push_back(const1);
    program1->children.push_back(const2);
    
    auto program2 = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "mul");
    auto const3 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    const3->parameters.push_back(3.0);
    auto const4 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    const4->parameters.push_back(4.0);
    program2->children.push_back(const3);
    program2->children.push_back(const4);
    
    Individual parent1(program1);
    Individual parent2(program2);
    
    // Perform crossover
    Individual offspring = Individual::crossover(parent1, parent2);
    
    EXPECT_TRUE(offspring.getProgram() != nullptr);
    // The offspring should have a program (structure may vary due to random crossover)
    EXPECT_TRUE(offspring.getProgram()->children.size() > 0);
}

TEST_F(EvolutionaryTest, IndividualMutation) {
    auto program = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    program->parameters.push_back(5.0);
    
    Individual original(program);
    Individual mutated = original.mutate(1.0); // 100% mutation rate
    
    EXPECT_TRUE(mutated.getProgram() != nullptr);
    // Programs should be different objects
    EXPECT_NE(mutated.getProgram().get(), original.getProgram().get());
}

TEST_F(EvolutionaryTest, PopulationManagement) {
    Population population(10); // Max size 10
    
    EXPECT_TRUE(population.empty());
    EXPECT_EQ(population.size(), 0);
    
    // Add individuals
    for (int i = 0; i < 5; ++i) {
        auto program = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
        program->parameters.push_back(i);
        Individual individual(program);
        
        FitnessResult fitness(i * 0.2, 0.0, 0.0); // Varying fitness
        individual.setFitness(fitness);
        
        population.addIndividual(individual);
    }
    
    EXPECT_EQ(population.size(), 5);
    EXPECT_FALSE(population.empty());
    
    // Test fitness statistics
    auto bestFitness = population.getBestFitness();
    auto avgFitness = population.getAverageFitness();
    
    EXPECT_DOUBLE_EQ(bestFitness.fitness, 0.8); // 4 * 0.2
    EXPECT_DOUBLE_EQ(avgFitness.fitness, 0.4); // (0 + 0.2 + 0.4 + 0.6 + 0.8) / 5
    
    // Test sorting
    population.sort();
    EXPECT_DOUBLE_EQ(population.getIndividual(0).getFitness().fitness, 0.8);
    EXPECT_DOUBLE_EQ(population.getIndividual(4).getFitness().fitness, 0.0);
}

TEST_F(EvolutionaryTest, PopulationSelection) {
    Population population(10);
    
    // Add individuals with different fitness
    for (int i = 0; i < 10; ++i) {
        auto program = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
        program->parameters.push_back(i);
        Individual individual(program);
        
        FitnessResult fitness(i * 0.1, 0.0, 0.0);
        individual.setFitness(fitness);
        
        population.addIndividual(individual);
    }
    
    // Test elite selection
    auto elite = population.eliteSelection(3);
    EXPECT_EQ(elite.size(), 3);
    EXPECT_GE(elite[0].getFitness().fitness, elite[1].getFitness().fitness);
    EXPECT_GE(elite[1].getFitness().fitness, elite[2].getFitness().fitness);
    
    // Test tournament selection
    auto tournament = population.tournamentSelection(3, 5);
    EXPECT_EQ(tournament.size(), 5);
}

TEST_F(EvolutionaryTest, FitnessFunction) {
    // Define a simple fitness function that rewards higher constant values
    FitnessFunction testFitness = [](const Individual& individual, const State& /*state*/) -> FitnessResult {
        if (!individual.getProgram()) {
            return FitnessResult(0.0);
        }
        
        std::unordered_map<std::string, double> context;
        double result = individual.getProgram()->evaluate(context);
        
        // Simple fitness: reward positive values, penalize negative
        double fitness = std::max(0.0, result / 10.0);
        double complexity = static_cast<double>(individual.getProgram()->toString().length());
        
        return FitnessResult(fitness, complexity, 0.0);
    };
    
    // Test the fitness function
    auto program = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    program->parameters.push_back(5.0);
    Individual individual(program);
    
    FitnessResult result = testFitness(individual, *state_);
    
    EXPECT_DOUBLE_EQ(result.fitness, 0.5); // 5.0 / 10.0
    EXPECT_GT(result.complexity, 0.0);
}

TEST_F(EvolutionaryTest, EvolutionaryOptimizerBasic) {
    EvolutionaryOptimizer::Config config;
    config.populationSize = 20;
    config.maxGenerations = 10;
    config.mutationRate = 0.1;
    config.crossoverRate = 0.8;
    
    EvolutionaryOptimizer optimizer(config);
    
    // Simple fitness function: maximize constant values
    FitnessFunction fitness = [](const Individual& individual, const State& /*state*/) -> FitnessResult {
        if (!individual.getProgram()) {
            return FitnessResult(0.0);
        }
        
        std::unordered_map<std::string, double> context;
        double result = individual.getProgram()->evaluate(context);
        
        return FitnessResult(result > 0 ? result / 100.0 : 0.0, 
                           static_cast<double>(individual.getProgram()->toString().length()), 
                           0.0);
    };
    
    // Run optimization
    Individual best = optimizer.optimize(fitness, *state_);
    
    EXPECT_TRUE(best.getProgram() != nullptr);
    EXPECT_GE(best.getFitness().fitness, 0.0);
    
    // Check statistics
    auto stats = optimizer.getStatistics();
    EXPECT_GE(stats.generation, 0);
    EXPECT_GE(stats.bestFitness.fitness, 0.0);
}

TEST_F(EvolutionaryTest, PatternExtraction) {
    PatternExtractor extractor;
    
    // Create some individuals with patterns
    std::vector<Individual> individuals;
    
    for (int i = 0; i < 10; ++i) {
        auto program = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "add");
        auto const1 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
        const1->parameters.push_back(i);
        auto const2 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
        const2->parameters.push_back(i + 1);
        program->children.push_back(const1);
        program->children.push_back(const2);
        
        Individual individual(program);
        FitnessResult fitness(0.8 + i * 0.01, 0.0, 0.0); // High fitness
        individual.setFitness(fitness);
        
        individuals.push_back(individual);
    }
    
    // Extract patterns
    auto patterns = extractor.extractPatterns(individuals, 0.7);
    
    EXPECT_GT(patterns.size(), 0);
    
    // Test pattern properties
    for (const auto& pattern : patterns) {
        EXPECT_FALSE(pattern.name.empty());
        EXPECT_TRUE(pattern.structure != nullptr);
        EXPECT_GE(pattern.frequency, 0.0);
        EXPECT_LE(pattern.frequency, 1.0);
        EXPECT_GE(pattern.effectiveness, 0.0);
    }
}

TEST_F(EvolutionaryTest, OptimizationPipeline) {
    OptimizationPipeline pipeline;
    
    // Create a simple stage
    OptimizationPipeline::Stage stage1("test_stage", 
        [](const Individual& /*individual*/, const State& /*state*/) -> FitnessResult {
            return FitnessResult(0.5, 0.0, 0.0);
        });
    
    stage1.config.populationSize = 10;
    stage1.config.maxGenerations = 5;
    
    pipeline.addStage(stage1);
    
    // Run pipeline
    Individual result = pipeline.runPipeline(*state_);
    
    EXPECT_TRUE(result.getProgram() != nullptr);
    
    // Check pipeline results
    auto pipelineResult = pipeline.getLastResult();
    EXPECT_GE(pipelineResult.stageResults.size(), 1);
    EXPECT_GT(pipelineResult.totalTime.count(), 0);
}

TEST_F(EvolutionaryTest, AdaptationHooks) {
    // This test would use the adaptation hooks from adaptation_hooks.cpp
    // For now, we'll test the basic interface
    
    class TestAdaptationHook : public AdaptationHook {
    public:
        mutable int patternCount = 0;
        mutable int improvementCount = 0;
        mutable int convergenceCount = 0;
        mutable int updateCount = 0;
        
        void onPatternDiscovered(const PatternExtractor::Pattern& /*pattern*/, const State& /*state*/) override {
            patternCount++;
        }
        
        void onFitnessImprovement(const Individual& /*individual*/,
                                 const FitnessResult& /*oldFitness*/,
                                 const FitnessResult& /*newFitness*/,
                                 const State& /*state*/) override {
            improvementCount++;
        }
        
        void onConvergence(const Population& /*population*/, const State& /*state*/) override {
            convergenceCount++;
        }
        
        void onAdaptationUpdate(const EvolutionaryOptimizer::Statistics& /*stats*/,
                               EvolutionaryOptimizer::Config& /*config*/) override {
            updateCount++;
        }
    };
    
    auto hook = std::make_shared<TestAdaptationHook>();
    
    // Test hook interface
    PatternExtractor::Pattern testPattern("test", nullptr);
    hook->onPatternDiscovered(testPattern, *state_);
    EXPECT_EQ(hook->patternCount, 1);
    
    Individual testIndividual(nullptr);
    FitnessResult oldFitness(0.5);
    FitnessResult newFitness(0.8);
    hook->onFitnessImprovement(testIndividual, oldFitness, newFitness, *state_);
    EXPECT_EQ(hook->improvementCount, 1);
    
    Population testPopulation(10);
    hook->onConvergence(testPopulation, *state_);
    EXPECT_EQ(hook->convergenceCount, 1);
    
    EvolutionaryOptimizer::Statistics stats;
    EvolutionaryOptimizer::Config config;
    hook->onAdaptationUpdate(stats, config);
    EXPECT_EQ(hook->updateCount, 1);
}

TEST_F(EvolutionaryTest, ComplexProgramEvaluation) {
    // Test more complex program structures
    
    // Create: if(gt(x, 5), add(x, 1), sub(x, 1))
    auto ifNode = std::make_shared<ProgramNode>(ProgramNode::Type::CONDITIONAL, "if");
    
    // Condition: gt(x, 5)
    auto gtNode = std::make_shared<ProgramNode>(ProgramNode::Type::CONDITIONAL, "gt");
    auto xVar = std::make_shared<ProgramNode>(ProgramNode::Type::VARIABLE, "x");
    auto const5 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    const5->parameters.push_back(5.0);
    gtNode->children.push_back(xVar);
    gtNode->children.push_back(const5);
    
    // True branch: add(x, 1)
    auto addNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "add");
    auto xVar2 = std::make_shared<ProgramNode>(ProgramNode::Type::VARIABLE, "x");
    auto const1 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    const1->parameters.push_back(1.0);
    addNode->children.push_back(xVar2);
    addNode->children.push_back(const1);
    
    // False branch: sub(x, 1)
    auto subNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "sub");
    auto xVar3 = std::make_shared<ProgramNode>(ProgramNode::Type::VARIABLE, "x");
    auto constNeg1 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    constNeg1->parameters.push_back(1.0);
    subNode->children.push_back(xVar3);
    subNode->children.push_back(constNeg1);
    
    ifNode->children.push_back(gtNode);
    ifNode->children.push_back(addNode);
    ifNode->children.push_back(subNode);
    
    // Test with different values of x
    std::unordered_map<std::string, double> context;
    
    // x = 10 (> 5), should return 11
    context["x"] = 10.0;
    EXPECT_DOUBLE_EQ(ifNode->evaluate(context), 11.0);
    
    // x = 3 (< 5), should return 2
    context["x"] = 3.0;
    EXPECT_DOUBLE_EQ(ifNode->evaluate(context), 2.0);
    
    // x = 5 (= 5), should return 4 (false branch)
    context["x"] = 5.0;
    EXPECT_DOUBLE_EQ(ifNode->evaluate(context), 4.0);
}

TEST_F(EvolutionaryTest, FitnessResultOperations) {
    FitnessResult fitness1(0.8, 10.0, 0.2);
    FitnessResult fitness2(0.6, 15.0, 0.8);
    
    // Test overall score calculation
    double score1 = fitness1.getOverallScore();
    double score2 = fitness2.getOverallScore();
    
    // score = fitness - 0.1*complexity + 0.05*novelty
    // score1 = 0.8 - 0.1*10.0 + 0.05*0.2 = 0.8 - 1.0 + 0.01 = -0.19
    // score2 = 0.6 - 0.1*15.0 + 0.05*0.8 = 0.6 - 1.5 + 0.04 = -0.86
    
    EXPECT_DOUBLE_EQ(score1, -0.19);
    EXPECT_DOUBLE_EQ(score2, -0.86);
    EXPECT_GT(score1, score2); // fitness1 has better overall score
}

TEST_F(EvolutionaryTest, SerializationAndDeserialization) {
    auto program = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "add");
    auto const1 = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
    const1->parameters.push_back(3.0);
    program->children.push_back(const1);
    
    Individual original(program);
    FitnessResult fitness(0.7, 5.0, 0.3);
    original.setFitness(fitness);
    
    // Serialize
    std::string serialized = original.serialize();
    EXPECT_FALSE(serialized.empty());
    EXPECT_NE(serialized.find("fitness:0.7"), std::string::npos);
    
    // Deserialize (simplified test since actual parsing is basic)
    Individual deserialized = Individual::deserialize(serialized);
    EXPECT_DOUBLE_EQ(deserialized.getFitness().fitness, 0.7);
}

// Performance test
TEST_F(EvolutionaryTest, DISABLED_PerformanceTest) {
    // This test is disabled by default as it takes time
    EvolutionaryOptimizer::Config config;
    config.populationSize = 100;
    config.maxGenerations = 50;
    config.mutationRate = 0.1;
    config.crossoverRate = 0.8;
    
    EvolutionaryOptimizer optimizer(config);
    
    // Fitness function that rewards finding specific target values
    FitnessFunction targetFitness = [](const Individual& individual, const State& /*state*/) -> FitnessResult {
        if (!individual.getProgram()) {
            return FitnessResult(0.0);
        }
        
        std::unordered_map<std::string, double> context;
        context["x"] = 1.0;
        context["y"] = 2.0;
        
        double result = individual.getProgram()->evaluate(context);
        double target = 42.0; // Target value to evolve towards
        
        double fitness = 1.0 / (1.0 + std::abs(result - target));
        double complexity = static_cast<double>(individual.getProgram()->toString().length());
        
        return FitnessResult(fitness, complexity, 0.0);
    };
    
    auto startTime = std::chrono::steady_clock::now();
    Individual best = optimizer.optimize(targetFitness, *state_);
    auto endTime = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    EXPECT_TRUE(best.getProgram() != nullptr);
    EXPECT_GT(best.getFitness().fitness, 0.0);
    
    std::cout << "Optimization completed in " << duration.count() << "ms" << std::endl;
    std::cout << "Best fitness: " << best.getFitness().fitness << std::endl;
    std::cout << "Best program: " << best.getProgram()->toString() << std::endl;
    
    // The optimization should show some improvement
    auto history = optimizer.getHistory();
    EXPECT_GT(history.size(), 0);
    
    if (history.size() > 1) {
        // Should show improvement over time
        EXPECT_GE(history.back().bestFitness.fitness, history.front().bestFitness.fitness);
    }
}