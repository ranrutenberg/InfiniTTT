// Hybrid Evaluator AI - Combines tactical play with strategic position evaluation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "aiplayer.h"
#include <set>

class EvaluationWeights;
class TicTacToeBoard;

// Hybrid Evaluator AI - Combines tactical play with strategic position evaluation
// Priority 1: Take winning moves
// Priority 2: Block opponent winning moves
// Priority 3: Maximize position score using trainable sequence evaluation
class HybridEvaluatorAI : public AIPlayer {
private:
    std::set<std::pair<int, int>> availableMoves;  // Maintained internally by AI
    const EvaluationWeights* weights;  // Optional custom weights for learning

    // Helper to check if a move results in a win (from SmartRandomAI)
    bool isWinningMove(const TicTacToeBoard& board, int x, int y, char playerMark, int winLength = 5) const;

    // Evaluate board position using sequence scoring (from MinimaxAI)
    int evaluatePosition(const TicTacToeBoard& board, char mark) const;

public:
    HybridEvaluatorAI(const EvaluationWeights* w = nullptr, bool verbose = false)
        : AIPlayer(verbose), weights(w) {}

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                      std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) override;
};
