#include "elizaos/evolutionary.hpp"
#include "elizaos/core.hpp"
#include <sstream>
#include <fstream>
#include <queue>
#include <cmath>
#include <thread>
#include <algorithm>
#include <random>
#include <set>

namespace elizaos {

// ProgramNode implementation
std::shared_ptr<ProgramNode> ProgramNode::clone() const {
    auto cloned = std::make_shared<ProgramNode>(type, name);
    cloned->parameters = parameters;
    
    for (const auto& child : children) {
        cloned->children.push_back(child->clone());
    }
    
    return cloned;
}

double ProgramNode::evaluate(const std::unordered_map<std::string, double>& context) const {
    switch (type) {
        case Type::CONSTANT:
            return parameters.empty() ? 0.0 : parameters[0];
            
        case Type::VARIABLE: {
            auto it = context.find(name);
            return it != context.end() ? it->second : 0.0;
        }
        
        case Type::FUNCTION: {
            if (name == "add" && children.size() >= 2) {
                return children[0]->evaluate(context) + children[1]->evaluate(context);
            }
            else if (name == "sub" && children.size() >= 2) {
                return children[0]->evaluate(context) - children[1]->evaluate(context);
            }
            else if (name == "mul" && children.size() >= 2) {
                return children[0]->evaluate(context) * children[1]->evaluate(context);
            }
            else if (name == "div" && children.size() >= 2) {
                double divisor = children[1]->evaluate(context);
                return divisor != 0.0 ? children[0]->evaluate(context) / divisor : 0.0;
            }
            else if (name == "sin" && children.size() >= 1) {
                return std::sin(children[0]->evaluate(context));
            }
            else if (name == "cos" && children.size() >= 1) {
                return std::cos(children[0]->evaluate(context));
            }
            else if (name == "exp" && children.size() >= 1) {
                return std::exp(children[0]->evaluate(context));
            }
            else if (name == "log" && children.size() >= 1) {
                double value = children[0]->evaluate(context);
                return value > 0 ? std::log(value) : 0.0;
            }
            else if (name == "max" && children.size() >= 2) {
                return std::max(children[0]->evaluate(context), children[1]->evaluate(context));
            }
            else if (name == "min" && children.size() >= 2) {
                return std::min(children[0]->evaluate(context), children[1]->evaluate(context));
            }
            return 0.0;
        }
        
        case Type::CONDITIONAL: {
            if (name == "if" && children.size() >= 3) {
                double condition = children[0]->evaluate(context);
                return condition > 0 ? children[1]->evaluate(context) : children[2]->evaluate(context);
            }
            else if (name == "gt" && children.size() >= 2) {
                return children[0]->evaluate(context) > children[1]->evaluate(context) ? 1.0 : 0.0;
            }
            else if (name == "lt" && children.size() >= 2) {
                return children[0]->evaluate(context) < children[1]->evaluate(context) ? 1.0 : 0.0;
            }
            return 0.0;
        }
    }
    
    return 0.0;
}

std::string ProgramNode::toString() const {
    std::ostringstream oss;
    
    switch (type) {
        case Type::CONSTANT:
            oss << (parameters.empty() ? 0.0 : parameters[0]);
            break;
            
        case Type::VARIABLE:
            oss << name;
            break;
            
        case Type::FUNCTION:
            oss << "(" << name;
            for (const auto& child : children) {
                oss << " " << child->toString();
            }
            oss << ")";
            break;
            
        case Type::CONDITIONAL:
            oss << "(" << name;
            for (const auto& child : children) {
                oss << " " << child->toString();
            }
            oss << ")";
            break;
    }
    
    return oss.str();
}

// Individual implementation
Individual::Individual(std::shared_ptr<ProgramNode> program) 
    : program_(program), age_(0), id_(elizaos::generateUUID()) {
}

Individual::Individual(const Individual& other) 
    : program_(other.program_ ? other.program_->clone() : nullptr),
      fitness_(other.fitness_),
      age_(other.age_),
      id_(elizaos::generateUUID()) {
}

Individual& Individual::operator=(const Individual& other) {
    if (this != &other) {
        program_ = other.program_ ? other.program_->clone() : nullptr;
        fitness_ = other.fitness_;
        age_ = other.age_;
        id_ = elizaos::generateUUID();
    }
    return *this;
}

Individual Individual::crossover(const Individual& parent1, const Individual& parent2) {
    if (!parent1.program_ || !parent2.program_) {
        return Individual(nullptr);
    }
    
    // Create offspring by copying parent1's program
    auto offspring = std::make_shared<ProgramNode>(*parent1.program_);
    
    // Perform subtree crossover
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Select random subtree from parent1 to replace
    std::vector<std::shared_ptr<ProgramNode>> subtrees1;
    std::function<void(std::shared_ptr<ProgramNode>)> collectSubtrees1 = 
        [&](std::shared_ptr<ProgramNode> node) {
            subtrees1.push_back(node);
            for (auto& child : node->children) {
                collectSubtrees1(child);
            }
        };
    collectSubtrees1(offspring);
    
    // Select random subtree from parent2 to insert
    std::vector<std::shared_ptr<ProgramNode>> subtrees2;
    std::function<void(std::shared_ptr<ProgramNode>)> collectSubtrees2 = 
        [&](std::shared_ptr<ProgramNode> node) {
            subtrees2.push_back(node);
            for (auto& child : node->children) {
                collectSubtrees2(child);
            }
        };
    collectSubtrees2(parent2.program_);
    
    if (!subtrees1.empty() && !subtrees2.empty()) {
        std::uniform_int_distribution<size_t> dist1(0, subtrees1.size() - 1);
        std::uniform_int_distribution<size_t> dist2(0, subtrees2.size() - 1);
        
        size_t replaceIndex = dist1(gen);
        size_t insertIndex = dist2(gen);
        
        // Replace subtree
        auto& replaceNode = subtrees1[replaceIndex];
        auto& insertNode = subtrees2[insertIndex];
        
        *replaceNode = *insertNode->clone();
    }
    
    return Individual(offspring);
}

Individual Individual::mutate(double mutationRate) const {
    if (!program_) {
        return Individual(nullptr);
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    
    auto mutated = program_->clone();
    
    std::function<void(std::shared_ptr<ProgramNode>)> mutateNode = 
        [&](std::shared_ptr<ProgramNode> node) {
            if (prob(gen) < mutationRate) {
                // Mutate this node
                if (node->type == ProgramNode::Type::CONSTANT && !node->parameters.empty()) {
                    // Mutate constant value
                    std::normal_distribution<double> dist(node->parameters[0], 0.1);
                    node->parameters[0] = dist(gen);
                }
                else if (node->type == ProgramNode::Type::FUNCTION) {
                    // Change function type
                    std::vector<std::string> functions = {"add", "sub", "mul", "div", "sin", "cos", "max", "min"};
                    std::uniform_int_distribution<size_t> funcDist(0, functions.size() - 1);
                    node->name = functions[funcDist(gen)];
                }
            }
            
            // Recursively mutate children
            for (auto& child : node->children) {
                mutateNode(child);
            }
        };
    
    mutateNode(mutated);
    
    return Individual(mutated);
}

double Individual::similarity(const Individual& other) const {
    if (!program_ || !other.program_) {
        return 0.0;
    }
    
    // Simple structural similarity based on program string representation
    std::string str1 = program_->toString();
    std::string str2 = other.program_->toString();
    
    // Calculate edit distance (simplified)
    if (str1 == str2) {
        return 1.0;
    }
    
    // Calculate Jaccard similarity on tokens
    std::set<std::string> tokens1, tokens2;
    
    std::stringstream ss1(str1), ss2(str2);
    std::string token;
    
    while (ss1 >> token) {
        tokens1.insert(token);
    }
    
    while (ss2 >> token) {
        tokens2.insert(token);
    }
    
    std::set<std::string> intersection, union_set;
    
    std::set_intersection(tokens1.begin(), tokens1.end(),
                         tokens2.begin(), tokens2.end(),
                         std::inserter(intersection, intersection.begin()));
    
    std::set_union(tokens1.begin(), tokens1.end(),
                   tokens2.begin(), tokens2.end(),
                   std::inserter(union_set, union_set.begin()));
    
    if (union_set.empty()) {
        return 0.0;
    }
    
    return static_cast<double>(intersection.size()) / union_set.size();
}

std::string Individual::serialize() const {
    std::ostringstream oss;
    oss << "Individual{";
    oss << "id:" << id_ << ",";
    oss << "age:" << age_ << ",";
    oss << "fitness:" << fitness_.fitness << ",";
    oss << "program:" << (program_ ? program_->toString() : "null");
    oss << "}";
    return oss.str();
}

Individual Individual::deserialize(const std::string& data) {
    // Simplified deserialization - in practice this would be more robust
    Individual individual(nullptr);
    
    // Extract fields from serialized string
    size_t idPos = data.find("id:");
    size_t agePos = data.find("age:");
    size_t fitnessPos = data.find("fitness:");
    
    if (idPos != std::string::npos && agePos != std::string::npos && fitnessPos != std::string::npos) {
        // Extract values (simplified parsing)
        std::string idStr = data.substr(idPos + 3, agePos - idPos - 4);
        std::string ageStr = data.substr(agePos + 4, fitnessPos - agePos - 5);
        std::string fitnessStr = data.substr(fitnessPos + 8);
        
        // Find the end of fitness value
        size_t commaPos = fitnessStr.find(',');
        if (commaPos != std::string::npos) {
            fitnessStr = fitnessStr.substr(0, commaPos);
        }
        
        individual.id_ = idStr;
        individual.age_ = std::stoi(ageStr);
        individual.fitness_.fitness = std::stod(fitnessStr);
    }
    
    return individual;
}

// Population implementation
Population::Population(size_t maxSize) : maxSize_(maxSize) {
}

void Population::addIndividual(const Individual& individual) {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    if (individuals_.size() < maxSize_) {
        individuals_.push_back(individual);
    } else {
        // Replace worst individual if new one is better
        auto worst = std::min_element(individuals_.begin(), individuals_.end(),
                                     [](const Individual& a, const Individual& b) {
                                         return a.getFitness().getOverallScore() < b.getFitness().getOverallScore();
                                     });
        
        if (worst != individuals_.end() && 
            individual.getFitness().getOverallScore() > worst->getFitness().getOverallScore()) {
            *worst = individual;
        }
    }
}

void Population::removeIndividual(size_t index) {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    if (index < individuals_.size()) {
        individuals_.erase(individuals_.begin() + index);
    }
}

const Individual& Population::getIndividual(size_t index) const {
    std::lock_guard<std::mutex> lock(populationMutex_);
    return individuals_.at(index);
}

Individual& Population::getIndividual(size_t index) {
    std::lock_guard<std::mutex> lock(populationMutex_);
    return individuals_.at(index);
}

FitnessResult Population::getBestFitness() const {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    if (individuals_.empty()) {
        return FitnessResult();
    }
    
    auto best = std::max_element(individuals_.begin(), individuals_.end(),
                                [](const Individual& a, const Individual& b) {
                                    return a.getFitness().getOverallScore() < b.getFitness().getOverallScore();
                                });
    
    return best->getFitness();
}

FitnessResult Population::getAverageFitness() const {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    if (individuals_.empty()) {
        return FitnessResult();
    }
    
    double totalFitness = 0.0;
    double totalComplexity = 0.0;
    double totalNovelty = 0.0;
    
    for (const auto& individual : individuals_) {
        const auto& fitness = individual.getFitness();
        totalFitness += fitness.fitness;
        totalComplexity += fitness.complexity;
        totalNovelty += fitness.novelty;
    }
    
    size_t size = individuals_.size();
    return FitnessResult(totalFitness / size, totalComplexity / size, totalNovelty / size);
}

double Population::getDiversity() const {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    if (individuals_.size() < 2) {
        return 0.0;
    }
    
    double totalSimilarity = 0.0;
    int comparisons = 0;
    
    for (size_t i = 0; i < individuals_.size(); ++i) {
        for (size_t j = i + 1; j < individuals_.size(); ++j) {
            totalSimilarity += individuals_[i].similarity(individuals_[j]);
            comparisons++;
        }
    }
    
    return comparisons > 0 ? 1.0 - (totalSimilarity / comparisons) : 0.0;
}

std::vector<Individual> Population::tournamentSelection(size_t tournamentSize, size_t numSelected) const {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    std::vector<Individual> selected;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, individuals_.size() - 1);
    
    for (size_t i = 0; i < numSelected; ++i) {
        Individual best = individuals_[dist(gen)];
        
        for (size_t j = 1; j < tournamentSize; ++j) {
            const Individual& candidate = individuals_[dist(gen)];
            if (candidate.getFitness().getOverallScore() > best.getFitness().getOverallScore()) {
                best = candidate;
            }
        }
        
        selected.push_back(best);
    }
    
    return selected;
}

std::vector<Individual> Population::eliteSelection(size_t numElite) const {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    std::vector<Individual> sorted = individuals_;
    std::sort(sorted.begin(), sorted.end(),
              [](const Individual& a, const Individual& b) {
                  return a.getFitness().getOverallScore() > b.getFitness().getOverallScore();
              });
    
    std::vector<Individual> elite;
    for (size_t i = 0; i < std::min(numElite, sorted.size()); ++i) {
        elite.push_back(sorted[i]);
    }
    
    return elite;
}

void Population::sort() {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    std::sort(individuals_.begin(), individuals_.end(),
              [](const Individual& a, const Individual& b) {
                  return a.getFitness().getOverallScore() > b.getFitness().getOverallScore();
              });
}

void Population::ageIndividuals() {
    std::lock_guard<std::mutex> lock(populationMutex_);
    
    for (auto& individual : individuals_) {
        individual.incrementAge();
    }
}

// EvolutionaryOptimizer implementation
EvolutionaryOptimizer::EvolutionaryOptimizer(const Config& config) 
    : config_(config), population_(config.populationSize) {
}

EvolutionaryOptimizer::~EvolutionaryOptimizer() {
    stop();
}

Individual EvolutionaryOptimizer::optimize(const FitnessFunction& fitnessFunc, const State& state) {
    running_ = true;
    stopped_ = false;
    paused_ = false;
    
    // Initialize population if empty
    if (population_.empty()) {
        for (size_t i = 0; i < config_.populationSize; ++i) {
            auto program = generateRandomProgram();
            population_.addIndividual(Individual(program));
        }
    }
    
    // Main evolution loop
    for (size_t generation = 0; generation < config_.maxGenerations && !stopped_; ++generation) {
        while (paused_ && !stopped_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (stopped_) break;
        
        auto startTime = std::chrono::steady_clock::now();
        
        // Evaluate fitness
        evaluateFitness(fitnessFunc, state);
        
        // Check for convergence
        if (checkStagnation()) {
            break;
        }
        
        // Evolve to next generation
        evolveGeneration(fitnessFunc, state);
        
        // Update statistics
        updateStatistics(generation);
        
        // Age individuals
        population_.ageIndividuals();
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        if (!history_.empty()) {
            history_.back().generationTime = duration;
        }
    }
    
    running_ = false;
    
    // Return best individual
    population_.sort();
    return population_.size() > 0 ? population_.getIndividual(0) : Individual(nullptr);
}

void EvolutionaryOptimizer::evaluateFitness(const FitnessFunction& fitnessFunc, const State& state) {
    // Evaluate fitness for all individuals
    for (size_t i = 0; i < population_.size(); ++i) {
        Individual& individual = population_.getIndividual(i);
        FitnessResult fitness = fitnessFunc(individual, state);
        individual.setFitness(fitness);
    }
}

void EvolutionaryOptimizer::evolveGeneration(const FitnessFunction& fitnessFunc, const State& state) {
    // Selection
    std::vector<Individual> parents;
    selectParents(parents);
    
    // Reproduction
    std::vector<Individual> offspring;
    reproduction(parents, offspring);
    
    // Evaluate offspring
    for (auto& individual : offspring) {
        FitnessResult fitness = fitnessFunc(individual, state);
        individual.setFitness(fitness);
    }
    
    // Environmental selection
    environmentalSelection(offspring);
    
    // Apply MOSES-specific techniques
    if (config_.useDemeSplitting) {
        demeSplitting();
    }
    
    if (config_.useNoveltySearch) {
        noveltySearch();
    }
    
    complexityControl();
}

void EvolutionaryOptimizer::selectParents(std::vector<Individual>& parents) {
    size_t numParents = config_.populationSize;
    parents = population_.tournamentSelection(config_.tournamentSize, numParents);
}

void EvolutionaryOptimizer::reproduction(const std::vector<Individual>& parents, std::vector<Individual>& offspring) {
    std::uniform_real_distribution<double> prob(0.0, 1.0);
    
    for (size_t i = 0; i < parents.size(); i += 2) {
        const Individual& parent1 = parents[i];
        const Individual& parent2 = parents[i + 1 < parents.size() ? i + 1 : 0];
        
        if (prob(rng_) < config_.crossoverRate) {
            // Crossover
            offspring.push_back(Individual::crossover(parent1, parent2));
            if (i + 1 < parents.size()) {
                offspring.push_back(Individual::crossover(parent2, parent1));
            }
        } else {
            // Direct copy
            offspring.push_back(parent1);
            if (i + 1 < parents.size()) {
                offspring.push_back(parent2);
            }
        }
    }
    
    // Mutation
    for (auto& individual : offspring) {
        if (prob(rng_) < config_.mutationRate) {
            individual = individual.mutate(config_.mutationRate);
        }
    }
}

void EvolutionaryOptimizer::environmentalSelection(const std::vector<Individual>& offspring) {
    // Combine population and offspring
    std::vector<Individual> combined;
    
    // Add elite individuals
    auto elite = population_.eliteSelection(static_cast<size_t>(config_.populationSize * config_.eliteRatio));
    combined.insert(combined.end(), elite.begin(), elite.end());
    
    // Add offspring
    combined.insert(combined.end(), offspring.begin(), offspring.end());
    
    // Sort by fitness
    std::sort(combined.begin(), combined.end(),
              [](const Individual& a, const Individual& b) {
                  return a.getFitness().getOverallScore() > b.getFitness().getOverallScore();
              });
    
    // Replace population with best individuals
    population_.clear();
    for (size_t i = 0; i < std::min(config_.populationSize, combined.size()); ++i) {
        population_.addIndividual(combined[i]);
    }
}

void EvolutionaryOptimizer::demeSplitting() {
    // Implement deme splitting if diversity is low
    if (population_.getDiversity() < config_.diversityThreshold) {
        // Split population into sub-populations and evolve separately
        // This is a simplified implementation
        
        size_t halfSize = population_.size() / 2;
        std::vector<Individual> deme1, deme2;
        
        for (size_t i = 0; i < halfSize; ++i) {
            deme1.push_back(population_.getIndividual(i));
        }
        
        for (size_t i = halfSize; i < population_.size(); ++i) {
            deme2.push_back(population_.getIndividual(i));
        }
        
        // Add some random individuals to increase diversity
        for (size_t i = 0; i < halfSize / 4; ++i) {
            auto randomProgram = generateRandomProgram();
            deme1.push_back(Individual(randomProgram));
            
            randomProgram = generateRandomProgram();
            deme2.push_back(Individual(randomProgram));
        }
        
        // Replace population with combined demes
        population_.clear();
        for (const auto& individual : deme1) {
            population_.addIndividual(individual);
        }
        for (const auto& individual : deme2) {
            population_.addIndividual(individual);
        }
    }
}

void EvolutionaryOptimizer::noveltySearch() {
    // Implement novelty search to maintain diversity
    // This is a simplified implementation that rewards individuals with unique behavior signatures
    
    for (size_t i = 0; i < population_.size(); ++i) {
        Individual& individual = population_.getIndividual(i);
        FitnessResult fitness = individual.getFitness();
        
        // Calculate novelty based on behavior signature uniqueness
        double novelty = 0.0;
        for (size_t j = 0; j < population_.size(); ++j) {
            if (i != j) {
                double similarity = individual.similarity(population_.getIndividual(j));
                novelty += (1.0 - similarity);
            }
        }
        
        if (population_.size() > 1) {
            novelty /= (population_.size() - 1);
        }
        
        // Update fitness with novelty component
        fitness.novelty = novelty;
        individual.setFitness(fitness);
    }
}

void EvolutionaryOptimizer::complexityControl() {
    // Control complexity to prevent bloat
    for (size_t i = 0; i < population_.size(); ++i) {
        Individual& individual = population_.getIndividual(i);
        FitnessResult fitness = individual.getFitness();
        
        // Calculate complexity based on program size
        if (individual.getProgram()) {
            std::string programStr = individual.getProgram()->toString();
            fitness.complexity = static_cast<double>(programStr.length());
            
            // Penalize overly complex programs
            if (fitness.complexity > config_.maxComplexity) {
                fitness.fitness *= 0.5; // Reduce fitness for complex programs
            }
        }
        
        individual.setFitness(fitness);
    }
}

std::shared_ptr<ProgramNode> EvolutionaryOptimizer::generateRandomProgram(int maxDepth) const {
    std::uniform_int_distribution<int> typeDist(0, 3);
    std::uniform_real_distribution<double> valueDist(-10.0, 10.0);
    
    ProgramNode::Type type = static_cast<ProgramNode::Type>(typeDist(rng_));
    
    if (maxDepth <= 0) {
        // Force terminal nodes at maximum depth
        type = (typeDist(rng_) % 2 == 0) ? ProgramNode::Type::CONSTANT : ProgramNode::Type::VARIABLE;
    }
    
    switch (type) {
        case ProgramNode::Type::CONSTANT: {
            auto node = std::make_shared<ProgramNode>(ProgramNode::Type::CONSTANT, "const");
            node->parameters.push_back(valueDist(rng_));
            return node;
        }
        
        case ProgramNode::Type::VARIABLE: {
            std::vector<std::string> vars = {"x", "y", "z", "t", "fitness", "age"};
            std::uniform_int_distribution<size_t> varDist(0, vars.size() - 1);
            return std::make_shared<ProgramNode>(ProgramNode::Type::VARIABLE, vars[varDist(rng_)]);
        }
        
        case ProgramNode::Type::FUNCTION: {
            std::vector<std::string> functions = {"add", "sub", "mul", "div", "sin", "cos", "exp", "log", "max", "min"};
            std::uniform_int_distribution<size_t> funcDist(0, functions.size() - 1);
            
            auto node = std::make_shared<ProgramNode>(ProgramNode::Type::FUNCTION, functions[funcDist(rng_)]);
            
            // Add children based on function arity
            int arity = (node->name == "sin" || node->name == "cos" || node->name == "exp" || node->name == "log") ? 1 : 2;
            
            for (int i = 0; i < arity; ++i) {
                node->children.push_back(generateRandomProgram(maxDepth - 1));
            }
            
            return node;
        }
        
        case ProgramNode::Type::CONDITIONAL: {
            std::vector<std::string> conditionals = {"if", "gt", "lt"};
            std::uniform_int_distribution<size_t> condDist(0, conditionals.size() - 1);
            
            auto node = std::make_shared<ProgramNode>(ProgramNode::Type::CONDITIONAL, conditionals[condDist(rng_)]);
            
            // Add children based on conditional arity
            int arity = (node->name == "if") ? 3 : 2;
            
            for (int i = 0; i < arity; ++i) {
                node->children.push_back(generateRandomProgram(maxDepth - 1));
            }
            
            return node;
        }
    }
    
    return nullptr;
}

bool EvolutionaryOptimizer::checkStagnation() const {
    if (history_.size() < static_cast<size_t>(config_.maxStagnationGenerations)) {
        return false;
    }
    
    // Check if fitness has improved significantly in recent generations
    double recentBest = history_.back().bestFitness.fitness;
    double oldBest = history_[history_.size() - config_.maxStagnationGenerations].bestFitness.fitness;
    
    return (recentBest - oldBest) < config_.stagnationThreshold;
}

void EvolutionaryOptimizer::updateStatistics(int generation) {
    Statistics stats;
    stats.generation = generation;
    stats.bestFitness = population_.getBestFitness();
    stats.averageFitness = population_.getAverageFitness();
    stats.diversity = population_.getDiversity();
    stats.stagnationCount = 0;
    
    // Calculate convergence rate
    if (history_.size() > 0) {
        double currentBest = stats.bestFitness.fitness;
        double previousBest = history_.back().bestFitness.fitness;
        stats.convergenceRate = currentBest - previousBest;
        
        // Update stagnation count
        if (stats.convergenceRate < config_.stagnationThreshold) {
            stats.stagnationCount = history_.back().stagnationCount + 1;
        }
    }
    
    history_.push_back(stats);
}

EvolutionaryOptimizer::Statistics EvolutionaryOptimizer::getStatistics() const {
    return history_.empty() ? Statistics{} : history_.back();
}

std::vector<EvolutionaryOptimizer::Statistics> EvolutionaryOptimizer::getHistory() const {
    return history_;
}

void EvolutionaryOptimizer::setPopulation(const Population& population) {
    population_.clear();
    for (size_t i = 0; i < population.size(); ++i) {
        population_.addIndividual(population.getIndividual(i));
    }
}

std::shared_ptr<Population> EvolutionaryOptimizer::getPopulation() const {
    auto copy = std::make_shared<Population>(population_.size());
    for (size_t i = 0; i < population_.size(); ++i) {
        copy->addIndividual(population_.getIndividual(i));
    }
    return copy;
}

} // namespace elizaos