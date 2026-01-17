// AI Utilities - Shared helper functions for AI implementations
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "ai_utils.h"
#include "tictactoeboard.h"
#include <set>

namespace AIUtils {

// Helper function to compute all valid adjacent moves from scratch
// Used during minimax recursion where we don't maintain a state
std::vector<std::pair<int, int>> computeAdjacentMoves(const TicTacToeBoard& board) {
    std::vector<std::pair<int, int>> moves;
    auto occupiedPositions = board.getOccupiedPositions();
    std::set<std::pair<int, int>> uniquePositions;

    for (const auto& [pos, mark] : occupiedPositions) {
        int x = pos.first, y = pos.second;

        // Check all adjacent positions
        for (int i = x - 1; i <= x + 1; ++i) {
            for (int j = y - 1; j <= y + 1; ++j) {
                if (!board.isPositionOccupied(i, j)) {
                    uniquePositions.insert({i, j});
                }
            }
        }
    }

    moves.assign(uniquePositions.begin(), uniquePositions.end());
    return moves;
}

// Helper function to update available moves after a move
// Removes the played position and adds adjacent empty positions
void updateAvailableMoves(std::set<std::pair<int, int>>& availableMoves,
                         const TicTacToeBoard& board,
                         int moveX, int moveY) {
    // Remove the position that was just played
    availableMoves.erase({moveX, moveY});

    // Check all positions adjacent to the move and add empty ones
    for (int i = moveX - 1; i <= moveX + 1; ++i) {
        for (int j = moveY - 1; j <= moveY + 1; ++j) {
            // Skip if this is the move itself
            if (i == moveX && j == moveY) continue;

            // Add if not occupied
            if (!board.isPositionOccupied(i, j)) {
                availableMoves.insert({i, j});
            }
        }
    }
}

} // namespace AIUtils
