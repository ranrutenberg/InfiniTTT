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

// Count distinct open-3 windows that pass through (x, y) after placing playerMark there.
// An open-3 is a 5-cell window with exactly 3 friendly marks, 2 empty cells, no opponent
// marks, and both cells just outside the window unblocked by the opponent.
// Uses in-place make/unmake pattern.
int countOpenThreesAtPosition(TicTacToeBoard& board, int x, int y, char playerMark) {
    board.placeMarkDirect(x, y, playerMark);
    char opponent = (playerMark == 'X') ? 'O' : 'X';
    int count = 0;

    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> countedWindows;
    const auto& occupied = board.getOccupiedPositions();

    for (int d = 0; d < 4; ++d) {
        int dx = directions[d][0];
        int dy = directions[d][1];

        for (int offset = 0; offset < 5; ++offset) {
            int startX = x - offset * dx;
            int startY = y - offset * dy;
            int endX = startX + 4 * dx;
            int endY = startY + 4 * dy;

            auto p1 = std::make_pair(startX, startY);
            auto p2 = std::make_pair(endX, endY);
            auto windowKey = (p1 < p2) ? std::make_pair(p1, p2) : std::make_pair(p2, p1);

            if (countedWindows.count(windowKey) > 0) continue;
            countedWindows.insert(windowKey);

            int friendlyCount = 0, opponentCount = 0, emptyCount = 0;
            for (int k = 0; k < 5; ++k) {
                int cellX = startX + k * dx;
                int cellY = startY + k * dy;
                if (!board.isPositionOccupied(cellX, cellY)) {
                    emptyCount++;
                } else if (occupied.at({cellX, cellY}) == playerMark) {
                    friendlyCount++;
                } else {
                    opponentCount++;
                }
            }

            if (opponentCount > 0 || friendlyCount != 3 || emptyCount != 2) continue;

            int beforeX = startX - dx, beforeY = startY - dy;
            int afterX = endX + dx, afterY = endY + dy;

            bool openBefore = !board.isPositionOccupied(beforeX, beforeY) ||
                              occupied.at({beforeX, beforeY}) != opponent;
            bool openAfter = !board.isPositionOccupied(afterX, afterY) ||
                             occupied.at({afterX, afterY}) != opponent;

            if (openBefore && openAfter) {
                count++;
            }
        }
    }

    board.removeMarkDirect(x, y);
    return count;
}

} // namespace AIUtils
