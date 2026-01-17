// Random AI - AI that picks random adjacent moves
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "random_ai.h"
#include "ai_utils.h"
#include "tictactoeboard.h"
#include <random>
#include <algorithm>

// RandomAI implementation - picks a random adjacent move
std::pair<int, int> RandomAI::findBestMove(const TicTacToeBoard& board, char playerMark,
                                            std::pair<int, int> lastMove) {
    // If board is empty, initialize available moves with origin and return it
    if (board.getOccupiedPositions().empty()) {
        availableMoves.clear();
        availableMoves.insert({0, 0});
        return {0, 0};
    }

    // Update internal available moves based on lastMove
    if (lastMove.first != INT_MIN && lastMove.second != INT_MIN) {
        AIUtils::updateAvailableMoves(availableMoves, board, lastMove.first, lastMove.second);
    } else if (availableMoves.empty()) {
        // First call - compute from board
        auto tempMoves = AIUtils::computeAdjacentMoves(board);
        availableMoves.insert(tempMoves.begin(), tempMoves.end());
    }

    // Filter out any occupied positions that may have accumulated
    auto it = availableMoves.begin();
    while (it != availableMoves.end()) {
        if (board.isPositionOccupied(it->first, it->second)) {
            it = availableMoves.erase(it);
        } else {
            ++it;
        }
    }

    if (availableMoves.empty()) {
        return {0, 0};
    }

    // Randomly select a move from available moves
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, availableMoves.size() - 1);

    auto iter = availableMoves.begin();
    std::advance(iter, dis(gen));
    auto chosenMove = *iter;

    // Update internal available moves with our chosen move
    // Remove the chosen position
    availableMoves.erase(chosenMove);

    // Add adjacent positions (will be filtered on next call when opponent's move is placed)
    for (int i = chosenMove.first - 1; i <= chosenMove.first + 1; ++i) {
        for (int j = chosenMove.second - 1; j <= chosenMove.second + 1; ++j) {
            if (i == chosenMove.first && j == chosenMove.second) continue;
            // Don't check if occupied - will be filtered on next call with updated board
            availableMoves.insert({i, j});
        }
    }

    return chosenMove;
}
