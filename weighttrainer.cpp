// Weight Trainer Implementation
// Implements self-play learning through genetic algorithms
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "weighttrainer.h"
#include "src/ai/hybrid_evaluator_ai.h"
#include <iostream>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <chrono>

// Create AI instance for training
std::unique_ptr<AIPlayer> WeightTrainer::createTrainingAI(const EvaluationWeights& weights) {
    switch (trainingAIType) {
        case AIType::HYBRID_EVALUATOR:
            return std::make_unique<HybridEvaluatorAI>(&weights);
        default:
            std::cerr << "Error: AI type does not support weight training\n";
            return std::make_unique<HybridEvaluatorAI>(&weights);
    }
}

// Play a single game silently between two weight configurations
// Returns: 1 if weights1 wins, -1 if weights2 wins, 0 for draw
int WeightTrainer::playSilentGame(const EvaluationWeights& weights1,
                                   const EvaluationWeights& weights2,
                                   int maxMoves, bool player1First) {
    TicTacToeBoard board;
    auto ai1 = createTrainingAI(weights1);
    auto ai2 = createTrainingAI(weights2);

    char player1Mark = player1First ? 'X' : 'O';
    char player2Mark = player1First ? 'O' : 'X';

    std::pair<int, int> lastMove = {INT_MIN, INT_MIN};
    int moveCount = 0;

    while (moveCount < maxMoves) {
        // Player 1's turn
        auto move1 = ai1->findBestMove(board, player1Mark, lastMove);
        board.placeMarkDirect(move1.first, move1.second, player1Mark);
        lastMove = move1;
        moveCount++;

        // Check if player 1 won
        if (board.checkWinQuiet(move1.first, move1.second, 5)) {
            return 1;  // weights1 wins
        }

        if (moveCount >= maxMoves) break;

        // Player 2's turn
        auto move2 = ai2->findBestMove(board, player2Mark, lastMove);
        board.placeMarkDirect(move2.first, move2.second, player2Mark);
        lastMove = move2;
        moveCount++;

        // Check if player 2 won
        if (board.checkWinQuiet(move2.first, move2.second, 5)) {
            return -1;  // weights2 wins
        }
    }

    return 0;  // Draw
}

// Run a round-robin tournament where each candidate plays against every other
void WeightTrainer::runTournament(std::vector<WeightCandidate>& population) {
    for (auto& candidate : population) {
        candidate.wins = 0;
        candidate.losses = 0;
        candidate.draws = 0;
    }

    // Build the flat list of all (i,j) matchups up front
    struct Matchup { size_t i, j; };
    std::vector<Matchup> matchups;
    matchups.reserve(population.size() * (population.size() - 1) / 2);
    for (size_t i = 0; i < population.size(); ++i)
        for (size_t j = i + 1; j < population.size(); ++j)
            matchups.push_back({i, j});

    // Per-matchup result storage — no locks needed during game play
    struct MatchupResult { int iWins = 0, jWins = 0, draws = 0; };
    std::vector<MatchupResult> results(matchups.size());

    size_t totalMatchups = matchups.size();
    size_t dotInterval = std::max(size_t(1), totalMatchups / population.size());

    std::atomic<size_t> nextIdx{0};
    std::atomic<size_t> completedCount{0};
    std::mutex printMutex;

    auto worker = [&]() {
        size_t idx;
        while ((idx = nextIdx.fetch_add(1, std::memory_order_relaxed)) < totalMatchups) {
            const Matchup& m = matchups[idx];
            MatchupResult& r = results[idx];
            for (int game = 0; game < gamesPerMatchup; ++game) {
                int result = playSilentGame(population[m.i].weights,
                                           population[m.j].weights,
                                           maxMoves, game % 2 == 0);
                if (result == 1)       ++r.iWins;
                else if (result == -1) ++r.jWins;
                else                   ++r.draws;
            }
            size_t done = completedCount.fetch_add(1, std::memory_order_relaxed) + 1;
            if (done % dotInterval == 0) {
                std::lock_guard<std::mutex> lock(printMutex);
                std::cout << ".";
                std::cout.flush();
            }
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int t = 0; t < numThreads; ++t)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    // Merge results into population (single-threaded — no contention)
    for (size_t idx = 0; idx < matchups.size(); ++idx) {
        size_t i = matchups[idx].i, j = matchups[idx].j;
        const MatchupResult& r = results[idx];
        population[i].wins   += r.iWins;
        population[i].losses += r.jWins;
        population[i].draws  += r.draws;
        population[j].wins   += r.jWins;
        population[j].losses += r.iWins;
        population[j].draws  += r.draws;
    }
    std::cout << "\n";
}

// Evolve the population using genetic algorithm
std::vector<WeightCandidate> WeightTrainer::evolvePopulation(
    const std::vector<WeightCandidate>& population) {

    // Sort by fitness (best first)
    std::vector<WeightCandidate> sorted = population;
    std::sort(sorted.begin(), sorted.end(),
              [](const WeightCandidate& a, const WeightCandidate& b) {
                  return a.getFitness() > b.getFitness();
              });

    std::vector<WeightCandidate> newPopulation;

    // Keep top 20% (elitism)
    int eliteCount = populationSize / 5;
    for (int i = 0; i < eliteCount && i < static_cast<int>(sorted.size()); ++i) {
        newPopulation.push_back({{sorted[i].weights}});
    }

    // Generate rest through crossover and mutation
    while (static_cast<int>(newPopulation.size()) < populationSize) {
        // Select two parents (weighted by fitness)
        int parent1Idx = rand() % std::min(populationSize / 2, static_cast<int>(sorted.size()));
        int parent2Idx = rand() % std::min(populationSize / 2, static_cast<int>(sorted.size()));

        // Crossover
        EvaluationWeights child = sorted[parent1Idx].weights.crossover(sorted[parent2Idx].weights);

        // Mutation
        if ((rand() % 100) / 100.0 < mutationRate) {
            child = child.mutate(mutationRate);
        }

        newPopulation.push_back({child});
    }

    return newPopulation;
}

// Train through multiple generations
EvaluationWeights WeightTrainer::train(int generations, const EvaluationWeights& startingWeights) {
    std::cout << "Initializing weight training with " << populationSize
              << " candidates over " << generations << " generations"
              << " (" << numThreads << " threads)...\n\n";

    // Initialize population around starting weights
    std::vector<WeightCandidate> population;
    population.push_back({startingWeights});  // Include the starting weights

    // Generate variations
    for (int i = 1; i < populationSize; ++i) {
        EvaluationWeights mutated = startingWeights.mutate(0.3);  // Large initial variation
        population.push_back({mutated});
    }

    EvaluationWeights bestEver = startingWeights;
    double bestEverFitness = 0.0;

    // Evolution loop
    for (int gen = 0; gen < generations; ++gen) {
        std::cout << "Generation " << (gen + 1) << "/" << generations << ":\n";
        std::cout << "  Running tournament";

        runTournament(population);

        // Find best in this generation
        auto best = std::max_element(population.begin(), population.end(),
                                     [](const WeightCandidate& a, const WeightCandidate& b) {
                                         return a.getFitness() < b.getFitness();
                                     });

        std::cout << "  Best candidate:\n";
        best->print();

        // Track best ever
        if (best->getFitness() > bestEverFitness) {
            bestEverFitness = best->getFitness();
            bestEver = best->weights;
            std::cout << "  *** New best fitness! ***\n";
        }

        // Evolve to next generation
        if (gen < generations - 1) {
            population = evolvePopulation(population);
        }

        std::cout << "\n";
    }

    std::cout << "Training complete!\n";
    std::cout << "Best weights found (fitness: " << bestEverFitness << "):\n";
    bestEver.print();

    return bestEver;
}
