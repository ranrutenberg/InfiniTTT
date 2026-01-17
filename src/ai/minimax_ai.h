// Minimax AI - Strategic AI using minimax algorithm with alpha-beta pruning
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "aiplayer.h"
#include <set>
#include <chrono>

class EvaluationWeights;
class TicTacToeBoard;

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

    // Evaluate board position for a given player mark
    int evaluatePosition(const TicTacToeBoard& board, char mark) const;

    // Helper struct for storing move scores
    struct MoveScore {
        std::pair<int, int> move;
        int score;
    };

    // Print top N moves with scores (verbose mode)
    void printTopMoves(const std::vector<MoveScore>& moveScores, std::pair<int, int> selected, char playerMark) const;

public:
    MinimaxAI(int timeLimit = 100, int depth = 3, const EvaluationWeights* w = nullptr, bool verbose = false)
        : AIPlayer(verbose), timeLimitMs(timeLimit), maxDepth(depth), weights(w) {}

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                      std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) override;
};
