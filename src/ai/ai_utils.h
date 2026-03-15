// AI Utilities - Shared helper functions for AI implementations
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <set>
#include <utility>

class TicTacToeBoard;

namespace AIUtils {
    // Compute all valid adjacent moves from scratch
    // Used during minimax recursion where we don't maintain state
    std::vector<std::pair<int, int>> computeAdjacentMoves(const TicTacToeBoard& board);

    // Update available moves after a move
    // Removes the played position and adds adjacent empty positions
    void updateAvailableMoves(std::set<std::pair<int, int>>& availableMoves,
                             const TicTacToeBoard& board,
                             int moveX, int moveY);

    // Count distinct open-3 windows that pass through (x, y) after placing playerMark there.
    // An open-3 is a 5-cell window with exactly 3 friendly marks, 2 empty cells, no opponent
    // marks, and both cells just outside the window unblocked by the opponent.
    // A move creating >= 2 such windows is a "second-order double threat" (double open-3 fork).
    // Uses in-place make/unmake pattern on the board.
    int countOpenThreesAtPosition(TicTacToeBoard& board, int x, int y, char playerMark);
}
