// AI Player interface for Tic-Tac-Toe - Header file
// Defines abstract AI player interface and minimax implementation with alpha-beta pruning
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <utility>
#include <chrono>

class TicTacToeBoard;

// Abstract base class for AI players
class AIPlayer {
public:
    virtual ~AIPlayer() = default;

    // Find and return the best move for the current board state
    virtual std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark) = 0;
};

// Minimax-based AI implementation
class MinimaxAI : public AIPlayer {
private:
    int timeLimitMs;
    int maxDepth;

    int minimax(TicTacToeBoard& board, int depth, bool isMaximizing,
                char computerMark, char humanMark, int timeLimitMs,
                std::chrono::time_point<std::chrono::high_resolution_clock> startTime,
                int alpha, int beta);

public:
    MinimaxAI(int timeLimit = 100, int depth = 3) : timeLimitMs(timeLimit), maxDepth(depth) {}

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark) override;
};

// Random AI implementation - picks random adjacent moves
class RandomAI : public AIPlayer {
public:
    RandomAI() = default;

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark) override;
};

#endif // AIPLAYER_H
