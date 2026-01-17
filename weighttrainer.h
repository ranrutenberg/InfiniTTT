// Weight Trainer for AI Self-Play Learning
// Uses genetic algorithms to optimize evaluation weights through self-play
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef WEIGHTTRAINER_H
#define WEIGHTTRAINER_H

#include "evaluationweights.h"
#include "src/ai/aiplayer.h"
#include "tictactoeboard.h"
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <ctime>

struct WeightCandidate {
    EvaluationWeights weights;
    int wins = 0;
    int losses = 0;
    int draws = 0;

    double getFitness() const {
        int total = wins + losses + draws;
        if (total == 0) return 0.0;
        return (wins + 0.5 * draws) / static_cast<double>(total);
    }

    void print() const {
        std::cout << "  Fitness: " << getFitness()
                  << " (W:" << wins << " L:" << losses << " D:" << draws << ")\n";
        weights.print();
    }
};

class WeightTrainer {
private:
    int populationSize;
    int gamesPerMatchup;
    int maxMoves;
    double mutationRate;

    // Play a single game between two AIs (returns 1 if player1 wins, -1 if player2 wins, 0 for draw)
    int playSilentGame(const EvaluationWeights& weights1, const EvaluationWeights& weights2,
                       int maxMoves, bool player1First);

public:
    WeightTrainer(int popSize = 20, int gamesPerMatch = 10, int maxMov = 200, double mutRate = 0.15)
        : populationSize(popSize), gamesPerMatchup(gamesPerMatch),
          maxMoves(maxMov), mutationRate(mutRate) {
        srand(static_cast<unsigned int>(time(nullptr)));
    }

    // Run a tournament: each candidate plays against every other candidate
    void runTournament(std::vector<WeightCandidate>& population);

    // Evolve population using genetic algorithm
    std::vector<WeightCandidate> evolvePopulation(const std::vector<WeightCandidate>& population);

    // Train weights through multiple generations
    EvaluationWeights train(int generations, const EvaluationWeights& startingWeights);
};

#endif // WEIGHTTRAINER_H
