// Evaluation Weights for Position Scoring
// Configurable weights that can be tuned through self-play learning
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef EVALUATIONWEIGHTS_H
#define EVALUATIONWEIGHTS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>

struct EvaluationWeights {
    // 4 pieces in a 5-cell window (one move from winning)
    int four_open = 500;      // Both ends open - can win multiple ways
    int four_blocked = 200;   // One end blocked - still very strong

    // 3 pieces in a 5-cell window (building position)
    int three_open = 50;      // Both ends open - good potential
    int three_blocked = 20;   // One end blocked - still useful

    // 2 pieces in a 5-cell window (early positioning)
    int two_open = 5;         // Both ends open - minor value

    // Synergy bonus for multiple winning threats
    int double_threat = 10000;  // 2+ positions that create immediate win

    // Default constructor uses hand-tuned weights
    EvaluationWeights() = default;

    // Constructor with custom weights
    EvaluationWeights(int four_o, int four_b, int three_o, int three_b, int two_o, int dbl_threat = 10000)
        : four_open(four_o), four_blocked(four_b), three_open(three_o),
          three_blocked(three_b), two_open(two_o), double_threat(dbl_threat) {}

    // Save weights to file
    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) return false;

        file << four_open << "\n"
             << four_blocked << "\n"
             << three_open << "\n"
             << three_blocked << "\n"
             << two_open << "\n"
             << double_threat << "\n";

        return file.good();
    }

    // Load weights from file
    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        std::string line;
        if (std::getline(file, line)) four_open = std::stoi(line);
        if (std::getline(file, line)) four_blocked = std::stoi(line);
        if (std::getline(file, line)) three_open = std::stoi(line);
        if (std::getline(file, line)) three_blocked = std::stoi(line);
        if (std::getline(file, line)) two_open = std::stoi(line);
        if (std::getline(file, line)) double_threat = std::stoi(line);  // Backward compatible - will use default if missing

        return file.good() || file.eof();
    }

    // Create a mutated version for genetic algorithm
    EvaluationWeights mutate(double mutationRate = 0.1) const {
        auto mutateValue = [mutationRate](int value) {
            double change = ((rand() % 100) / 100.0 - 0.5) * 2 * mutationRate;
            int newValue = static_cast<int>(value * (1.0 + change));
            return std::max(1, newValue);  // Ensure positive weights
        };

        return EvaluationWeights(
            mutateValue(four_open),
            mutateValue(four_blocked),
            mutateValue(three_open),
            mutateValue(three_blocked),
            mutateValue(two_open),
            mutateValue(double_threat)
        );
    }

    // Crossover with another weight set
    EvaluationWeights crossover(const EvaluationWeights& other) const {
        return EvaluationWeights(
            (rand() % 2) ? four_open : other.four_open,
            (rand() % 2) ? four_blocked : other.four_blocked,
            (rand() % 2) ? three_open : other.three_open,
            (rand() % 2) ? three_blocked : other.three_blocked,
            (rand() % 2) ? two_open : other.two_open,
            (rand() % 2) ? double_threat : other.double_threat
        );
    }

    // Display weights
    void print() const {
        std::cout << "Evaluation Weights:\n"
                  << "  4-open: " << four_open << "\n"
                  << "  4-blocked: " << four_blocked << "\n"
                  << "  3-open: " << three_open << "\n"
                  << "  3-blocked: " << three_blocked << "\n"
                  << "  2-open: " << two_open << "\n"
                  << "  double-threat: " << double_threat << "\n";
    }
};

#endif // EVALUATIONWEIGHTS_H
