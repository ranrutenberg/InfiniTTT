// Smart Random AI - Random AI with stepwise optimizations
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "aiplayer.h"
#include <set>

class TicTacToeBoard;

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
