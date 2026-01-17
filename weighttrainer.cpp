// Weight Trainer Implementation
// Implements self-play learning through genetic algorithms
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "weighttrainer.h"
#include <iostream>
#include <algorithm>

// Play a single game silently between two weight configurations
// Returns: 1 if weights1 wins, -1 if weights2 wins, 0 for draw
int WeightTrainer::playSilentGame(const EvaluationWeights& weights1,
                                   const EvaluationWeights& weights2,
                                   int maxMoves, bool player1First) {
    TicTacToeBoard board;
    MinimaxAI ai1(100, 3, &weights1);
    MinimaxAI ai2(100, 3, &weights2);

    char player1Mark = player1First ? 'X' : 'O';
    char player2Mark = player1First ? 'O' : 'X';

    std::pair<int, int> lastMove = {INT_MIN, INT_MIN};
    int moveCount = 0;

    while (moveCount < maxMoves) {
        // Player 1's turn
        auto move1 = ai1.findBestMove(board, player1Mark, lastMove);
        board.placeMarkDirect(move1.first, move1.second, player1Mark);
        lastMove = move1;
        moveCount++;

        // Check if player 1 won
        if (board.checkWinQuiet(move1.first, move1.second, 5)) {
            return 1;  // weights1 wins
        }

        if (moveCount >= maxMoves) break;

        // Player 2's turn
        auto move2 = ai2.findBestMove(board, player2Mark, lastMove);
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
    // Reset statistics
    for (auto& candidate : population) {
        candidate.wins = 0;
        candidate.losses = 0;
        candidate.draws = 0;
    }

    // Each candidate plays against every other candidate
    for (size_t i = 0; i < population.size(); ++i) {
        for (size_t j = i + 1; j < population.size(); ++j) {
            // Play multiple games, alternating who goes first
            for (int game = 0; game < gamesPerMatchup; ++game) {
                bool player1First = (game % 2 == 0);
                int result = playSilentGame(population[i].weights,
                                           population[j].weights,
                                           maxMoves, player1First);

                if (result == 1) {
                    population[i].wins++;
                    population[j].losses++;
                } else if (result == -1) {
                    population[i].losses++;
                    population[j].wins++;
                } else {
                    population[i].draws++;
                    population[j].draws++;
                }
            }
        }

        // Progress indicator
        std::cout << ".";
        std::cout.flush();
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
              << " candidates over " << generations << " generations...\n\n";

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
