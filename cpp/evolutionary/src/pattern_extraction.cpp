#include "elizaos/evolutionary.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <future>
#include <thread>
#include <map>
#include <set>

namespace elizaos {

// PatternExtractor implementation
PatternExtractor::PatternExtractor() {
}

std::vector<PatternExtractor::Pattern> PatternExtractor::extractPatterns(
    const std::vector<Individual>& individuals, double minFitness) const {
    
    std::vector<Pattern> patterns;
    
    // Filter individuals by fitness
    std::vector<Individual> successfulIndividuals;
    for (const auto& individual : individuals) {
        if (individual.getFitness().fitness >= minFitness) {
            successfulIndividuals.push_back(individual);
        }
    }
    
    if (successfulIndividuals.empty()) {
        return patterns;
    }
    
    // Extract different types of patterns
    auto subtreePatterns = extractSubtreePatterns(successfulIndividuals);
    auto behaviorPatterns = extractBehaviorPatterns(successfulIndividuals);
    auto structuralPatterns = extractStructuralPatterns(successfulIndividuals);
    
    // Combine all patterns
    patterns.insert(patterns.end(), subtreePatterns.begin(), subtreePatterns.end());
    patterns.insert(patterns.end(), behaviorPatterns.begin(), behaviorPatterns.end());
    patterns.insert(patterns.end(), structuralPatterns.begin(), structuralPatterns.end());
    
    // Calculate pattern frequencies and effectiveness
    for (auto& pattern : patterns) {
        int count = 0;
        double totalFitness = 0.0;
        
        for (const auto& individual : successfulIndividuals) {
            if (individual.getProgram()) {
                std::string programStr = individual.getProgram()->toString();
                std::string patternStr = pattern.structure->toString();
                
                if (programStr.find(patternStr) != std::string::npos) {
                    count++;
                    totalFitness += individual.getFitness().fitness;
                }
            }
        }
        
        pattern.frequency = static_cast<double>(count) / successfulIndividuals.size();
        pattern.effectiveness = count > 0 ? totalFitness / count : 0.0;
    }
    
    // Sort patterns by effectiveness
    std::sort(patterns.begin(), patterns.end(),
              [](const Pattern& a, const Pattern& b) {
                  return a.effectiveness > b.effectiveness;
              });
    
    return patterns;
}

std::vector<PatternExtractor::Pattern> PatternExtractor::extractPatternsFromHistory(
    const std::vector<EvolutionaryOptimizer::Statistics>& history) const {
    
    std::vector<Pattern> patterns;
    
    // Analyze fitness progression patterns
    if (history.size() >= 3) {
        // Look for convergence patterns
        bool hasConvergence = true;
        double convergenceThreshold = 0.01;
        
        for (size_t i = history.size() - 3; i < history.size() - 1; ++i) {
            double improvement = history[i + 1].bestFitness.fitness - history[i].bestFitness.fitness;
            if (improvement > convergenceThreshold) {
                hasConvergence = false;
                break;
            }
        }
        
        if (hasConvergence) {
            auto convergencePattern = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "convergence");
            Pattern pattern("convergence", convergencePattern);
            pattern.effectiveness = history.back().bestFitness.fitness;
            pattern.frequency = 1.0;
            pattern.contexts.push_back("fitness_plateau");
            patterns.push_back(pattern);
        }
        
        // Look for diversity patterns
        double avgDiversity = 0.0;
        for (const auto& stat : history) {
            avgDiversity += stat.diversity;
        }
        avgDiversity /= history.size();
        
        if (avgDiversity > 0.5) {
            auto diversityPattern = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "high_diversity");
            Pattern pattern("high_diversity", diversityPattern);
            pattern.effectiveness = avgDiversity;
            pattern.frequency = 1.0;
            pattern.contexts.push_back("exploration");
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::vector<PatternExtractor::Pattern> PatternExtractor::getCommonPatterns(
    const std::vector<Pattern>& patterns, double minFrequency) const {
    
    std::vector<Pattern> commonPatterns;
    
    for (const auto& pattern : patterns) {
        if (pattern.frequency >= minFrequency) {
            commonPatterns.push_back(pattern);
        }
    }
    
    // Sort by frequency
    std::sort(commonPatterns.begin(), commonPatterns.end(),
              [](const Pattern& a, const Pattern& b) {
                  return a.frequency > b.frequency;
              });
    
    return commonPatterns;
}

double PatternExtractor::patternSimilarity(const Pattern& p1, const Pattern& p2) const {
    if (!p1.structure || !p2.structure) {
        return 0.0;
    }
    
    std::string str1 = p1.structure->toString();
    std::string str2 = p2.structure->toString();
    
    if (str1 == str2) {
        return 1.0;
    }
    
    // Simple token-based similarity
    std::set<std::string> tokens1, tokens2;
    std::stringstream ss1(str1), ss2(str2);
    std::string token;
    
    while (ss1 >> token) {
        tokens1.insert(token);
    }
    
    while (ss2 >> token) {
        tokens2.insert(token);
    }
    
    std::set<std::string> intersection;
    std::set_intersection(tokens1.begin(), tokens1.end(),
                         tokens2.begin(), tokens2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    std::set<std::string> union_set;
    std::set_union(tokens1.begin(), tokens1.end(),
                   tokens2.begin(), tokens2.end(),
                   std::inserter(union_set, union_set.begin()));
    
    if (union_set.empty()) {
        return 0.0;
    }
    
    return static_cast<double>(intersection.size()) / union_set.size();
}

void PatternExtractor::savePatterns(const std::vector<Pattern>& patterns, const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }
    
    file << "# Extracted Patterns\n";
    file << "# Format: name|structure|frequency|effectiveness|contexts\n";
    
    for (const auto& pattern : patterns) {
        file << pattern.name << "|";
        file << (pattern.structure ? pattern.structure->toString() : "null") << "|";
        file << pattern.frequency << "|";
        file << pattern.effectiveness << "|";
        
        for (size_t i = 0; i < pattern.contexts.size(); ++i) {
            file << pattern.contexts[i];
            if (i < pattern.contexts.size() - 1) {
                file << ";";
            }
        }
        file << "\n";
    }
    
    file.close();
}

std::vector<PatternExtractor::Pattern> PatternExtractor::loadPatterns(const std::string& filename) const {
    std::vector<Pattern> patterns;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        return patterns;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        std::stringstream ss(line);
        std::string name, structure, frequencyStr, effectivenessStr, contextsStr;
        
        if (std::getline(ss, name, '|') &&
            std::getline(ss, structure, '|') &&
            std::getline(ss, frequencyStr, '|') &&
            std::getline(ss, effectivenessStr, '|') &&
            std::getline(ss, contextsStr)) {
            
            // Create pattern (simplified - in practice would parse the structure)
            auto patternNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, structure);
            Pattern pattern(name, patternNode);
            
            try {
                pattern.frequency = std::stod(frequencyStr);
                pattern.effectiveness = std::stod(effectivenessStr);
                
                // Parse contexts
                std::stringstream contextStream(contextsStr);
                std::string context;
                while (std::getline(contextStream, context, ';')) {
                    pattern.contexts.push_back(context);
                }
                
                patterns.push_back(pattern);
            } catch (...) {
                // Skip invalid patterns
            }
        }
    }
    
    file.close();
    return patterns;
}

std::vector<PatternExtractor::Pattern> PatternExtractor::extractSubtreePatterns(
    const std::vector<Individual>& individuals) const {
    
    std::vector<Pattern> patterns;
    std::map<std::string, int> subtreeCount;
    std::map<std::string, double> subtreeFitness;
    
    // Collect all subtrees and their frequencies
    for (const auto& individual : individuals) {
        if (individual.getProgram()) {
            auto subtrees = getAllSubtrees(individual.getProgram());
            for (const auto& subtree : subtrees) {
                std::string subtreeStr = subtree->toString();
                subtreeCount[subtreeStr]++;
                subtreeFitness[subtreeStr] += individual.getFitness().fitness;
            }
        }
    }
    
    // Create patterns from frequent subtrees
    for (const auto& [subtreeStr, count] : subtreeCount) {
        if (count >= 2) { // At least 2 occurrences
            auto patternNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, subtreeStr);
            Pattern pattern("subtree_" + std::to_string(patterns.size()), patternNode);
            pattern.frequency = static_cast<double>(count) / individuals.size();
            pattern.effectiveness = subtreeFitness[subtreeStr] / count;
            pattern.contexts.push_back("subtree");
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::vector<PatternExtractor::Pattern> PatternExtractor::extractBehaviorPatterns(
    const std::vector<Individual>& individuals) const {
    
    std::vector<Pattern> patterns;
    
    // Group individuals by behavior signature similarity
    std::map<std::string, std::vector<Individual>> behaviorGroups;
    
    for (const auto& individual : individuals) {
        // Create behavior signature based on fitness characteristics
        std::string behaviorSignature;
        const auto& fitness = individual.getFitness();
        
        // Discretize fitness into ranges
        int fitnessRange = static_cast<int>(fitness.fitness * 10);
        int complexityRange = static_cast<int>(fitness.complexity / 10);
        int noveltyRange = static_cast<int>(fitness.novelty * 10);
        
        behaviorSignature = std::to_string(fitnessRange) + "_" + 
                           std::to_string(complexityRange) + "_" + 
                           std::to_string(noveltyRange);
        
        behaviorGroups[behaviorSignature].push_back(individual);
    }
    
    // Create patterns from behavior groups
    for (const auto& [signature, group] : behaviorGroups) {
        if (group.size() >= 2) {
            auto patternNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "behavior_" + signature);
            Pattern pattern("behavior_" + signature, patternNode);
            pattern.frequency = static_cast<double>(group.size()) / individuals.size();
            
            double totalFitness = 0.0;
            for (const auto& individual : group) {
                totalFitness += individual.getFitness().fitness;
            }
            pattern.effectiveness = totalFitness / group.size();
            pattern.contexts.push_back("behavior");
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::vector<PatternExtractor::Pattern> PatternExtractor::extractStructuralPatterns(
    const std::vector<Individual>& individuals) const {
    
    std::vector<Pattern> patterns;
    std::map<std::string, int> structureCount;
    std::map<std::string, double> structureFitness;
    
    // Analyze structural properties
    for (const auto& individual : individuals) {
        if (individual.getProgram()) {
            // Count node types
            std::map<std::string, int> nodeTypes;
            std::function<void(std::shared_ptr<ProgramNode>)> countNodes = 
                [&](std::shared_ptr<ProgramNode> node) {
                    nodeTypes[node->name]++;
                    for (const auto& child : node->children) {
                        countNodes(child);
                    }
                };
            
            countNodes(individual.getProgram());
            
            // Create structural signature
            std::string structuralSignature;
            for (const auto& [nodeName, count] : nodeTypes) {
                structuralSignature += nodeName + ":" + std::to_string(count) + ";";
            }
            
            structureCount[structuralSignature]++;
            structureFitness[structuralSignature] += individual.getFitness().fitness;
        }
    }
    
    // Create patterns from structural signatures
    for (const auto& [signature, count] : structureCount) {
        if (count >= 2) {
            auto patternNode = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, "structure");
            Pattern pattern("structure_" + std::to_string(patterns.size()), patternNode);
            pattern.frequency = static_cast<double>(count) / individuals.size();
            pattern.effectiveness = structureFitness[signature] / count;
            pattern.contexts.push_back("structure");
            patterns.push_back(pattern);
        }
    }
    
    return patterns;
}

std::vector<std::shared_ptr<ProgramNode>> PatternExtractor::getAllSubtrees(
    std::shared_ptr<ProgramNode> program) const {
    
    std::vector<std::shared_ptr<ProgramNode>> subtrees;
    
    if (!program) {
        return subtrees;
    }
    
    // Add the current node
    subtrees.push_back(program);
    
    // Recursively add all child subtrees
    for (const auto& child : program->children) {
        auto childSubtrees = getAllSubtrees(child);
        subtrees.insert(subtrees.end(), childSubtrees.begin(), childSubtrees.end());
    }
    
    return subtrees;
}

// OptimizationPipeline implementation
OptimizationPipeline::OptimizationPipeline() {
}

OptimizationPipeline::~OptimizationPipeline() {
    stop();
}

void OptimizationPipeline::addStage(const Stage& stage) {
    stages_.push_back(stage);
    if (stageOrder_.empty() || 
        std::find(stageOrder_.begin(), stageOrder_.end(), stage.name) == stageOrder_.end()) {
        stageOrder_.push_back(stage.name);
    }
}

void OptimizationPipeline::removeStage(const std::string& name) {
    stages_.erase(std::remove_if(stages_.begin(), stages_.end(),
                                [&name](const Stage& stage) {
                                    return stage.name == name;
                                }),
                 stages_.end());
    
    stageOrder_.erase(std::remove(stageOrder_.begin(), stageOrder_.end(), name),
                     stageOrder_.end());
}

void OptimizationPipeline::setStageOrder(const std::vector<std::string>& order) {
    stageOrder_ = order;
}

Individual OptimizationPipeline::runPipeline(const State& state) {
    running_ = true;
    stopped_ = false;
    paused_ = false;
    
    auto startTime = std::chrono::steady_clock::now();
    
    lastResult_.stageResults.clear();
    lastResult_.stageStatistics.clear();
    lastResult_.extractedPatterns.clear();
    
    Individual currentBest(nullptr);
    
    // Run stages in order
    for (const auto& stageName : stageOrder_) {
        while (paused_ && !stopped_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (stopped_) break;
        
        // Find stage by name
        auto stageIt = std::find_if(stages_.begin(), stages_.end(),
                                   [&stageName](const Stage& stage) {
                                       return stage.name == stageName;
                                   });
        
        if (stageIt != stages_.end()) {
            Individual stageResult = runStage(*stageIt, state, currentBest);
            
            if (stageResult.getProgram()) {
                currentBest = stageResult;
                lastResult_.stageResults.push_back(stageResult);
                
                // Notify hooks
                notifyHooks(*stageIt, stageResult, state);
            }
        }
    }
    
    // Extract patterns from all stage results
    if (!lastResult_.stageResults.empty()) {
        lastResult_.extractedPatterns = patternExtractor_.extractPatterns(lastResult_.stageResults, 0.5);
    }
    
    auto endTime = std::chrono::steady_clock::now();
    lastResult_.totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    lastResult_.bestIndividual = currentBest;
    
    running_ = false;
    
    return currentBest;
}

std::future<Individual> OptimizationPipeline::runPipelineAsync(const State& state) {
    return std::async(std::launch::async, [this, state]() {
        return runPipeline(state);
    });
}

OptimizationPipeline::PipelineResult OptimizationPipeline::getLastResult() const {
    return lastResult_;
}

void OptimizationPipeline::addGlobalHook(std::shared_ptr<AdaptationHook> hook) {
    globalHooks_.push_back(hook);
}

void OptimizationPipeline::removeGlobalHook(std::shared_ptr<AdaptationHook> hook) {
    globalHooks_.erase(std::remove(globalHooks_.begin(), globalHooks_.end(), hook),
                      globalHooks_.end());
}

Individual OptimizationPipeline::runStage(const Stage& stage, const State& state, const Individual& input) {
    EvolutionaryOptimizer optimizer(stage.config);
    
    // Set initial population if input is provided
    if (input.getProgram()) {
        Population initialPopulation(stage.config.populationSize);
        initialPopulation.addIndividual(input);
        
        // Fill rest of population with variants of the input
        for (size_t i = 1; i < stage.config.populationSize; ++i) {
            Individual variant = input.mutate(0.1);
            initialPopulation.addIndividual(variant);
        }
        
        optimizer.setPopulation(initialPopulation);
    }
    
    // Run optimization
    Individual result = optimizer.optimize(stage.fitnessFunc, state);
    
    // Store statistics
    lastResult_.stageStatistics.push_back(optimizer.getStatistics());
    
    return result;
}

void OptimizationPipeline::notifyHooks(const Stage& stage, const Individual& result, const State& state) {
    // Notify stage-specific hooks
    for (auto& hook : stage.hooks) {
        // For now, just call onFitnessImprovement as a generic notification
        // In a full implementation, this would be more sophisticated
        hook->onFitnessImprovement(result, FitnessResult(), result.getFitness(), state);
    }
    
    // Notify global hooks
    for (auto& hook : globalHooks_) {
        hook->onFitnessImprovement(result, FitnessResult(), result.getFitness(), state);
    }
}

} // namespace elizaos