// Random AI - AI that picks random adjacent moves
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "aiplayer.h"
#include <set>

class TicTacToeBoard;

// Random AI implementation - picks random adjacent moves
class RandomAI : public AIPlayer {
private:
    std::set<std::pair<int, int>> availableMoves;  // Maintained internally by AI

public:
    RandomAI(bool verbose = false) : AIPlayer(verbose) {}

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                      std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) override;
};
