// AI Player interface for Tic-Tac-Toe - Header file
// Defines abstract AI player interface and minimax implementation with alpha-beta pruning
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <utility>
#include <chrono>
#include <climits>
#include <set>
#include "evaluationweights.h"

class TicTacToeBoard;

// Abstract base class for AI players
class AIPlayer {
public:
    virtual ~AIPlayer() = default;

    // Find and return the best move for the current board state
    // lastMove: the last move made by the opponent (x, y coordinates)
    //           Use {INT_MIN, INT_MIN} to indicate no last move (first move of game)
    virtual std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                              std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) = 0;
};

// Minimax-based AI implementation
class MinimaxAI : public AIPlayer {
private:
    int timeLimitMs;
    int maxDepth;
    std::set<std::pair<int, int>> availableMoves;  // Maintained internally by AI
    const EvaluationWeights* weights;  // Optional custom weights for learning

    int minimax(TicTacToeBoard& board, int depth, bool isMaximizing,
                char computerMark, char humanMark, int timeLimitMs,
                std::chrono::time_point<std::chrono::high_resolution_clock> startTime,
                int alpha, int beta, std::pair<int, int> lastMove);

public:
    MinimaxAI(int timeLimit = 100, int depth = 3, const EvaluationWeights* w = nullptr)
        : timeLimitMs(timeLimit), maxDepth(depth), weights(w) {}

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                      std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) override;
};

// Random AI implementation - picks random adjacent moves
class RandomAI : public AIPlayer {
private:
    std::set<std::pair<int, int>> availableMoves;  // Maintained internally by AI

public:
    RandomAI() = default;

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                      std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) override;
};

// Smart Random AI implementation - random with stepwise optimizations
// Level 1: Check for winning moves first
// Level 2: (Future) Block opponent winning moves
// Level 3: (Future) Prioritize center positions
class SmartRandomAI : public AIPlayer {
private:
    std::set<std::pair<int, int>> availableMoves;  // Maintained internally by AI
    int optimizationLevel;  // 0 = pure random, 1 = check wins, 2 = block opponent, etc.

    // Helper method to check if a move results in a win
    bool isWinningMove(const TicTacToeBoard& board, int x, int y, char playerMark, int winLength = 5) const;

public:
    SmartRandomAI(int level = 1) : optimizationLevel(level) {}

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                      std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) override;
};

#endif // AIPLAYER_H
