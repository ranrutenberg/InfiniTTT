// Smart Random AI - Random AI with stepwise optimizations
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "smart_random_ai.h"
#include "ai_utils.h"
#include "tictactoeboard.h"
#include <iostream>
#include <random>
#include <algorithm>

// SmartRandomAI implementation - Helper to check if a move results in a win
bool SmartRandomAI::isWinningMove(const TicTacToeBoard& board, int x, int y, char playerMark, int winLength) const {
    // Create a copy of the board and try the move
    TicTacToeBoard boardCopy = board;
    boardCopy.placeMarkDirect(x, y, playerMark);

    // Check if this move creates a win using checkWinQuiet with the move position
    bool wins = boardCopy.checkWinQuiet(x, y, winLength);

    return wins;
}

// Optimized helper - modifies board temporarily with make/unmake pattern
bool SmartRandomAI::isWinningMoveInPlace(TicTacToeBoard& board, int x, int y, char playerMark, int winLength) const {
    board.placeMarkDirect(x, y, playerMark);
    bool wins = board.checkWinQuiet(x, y, winLength);
    board.removeMarkDirect(x, y);
    return wins;
}

// SmartRandomAI implementation - Optimized random player with stepwise improvements
std::pair<int, int> SmartRandomAI::findBestMove(const TicTacToeBoard& board, char playerMark,
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

    // Create ONE mutable board copy for all win/block checks
    TicTacToeBoard boardCopy = board;

    // OPTIMIZATION LEVEL 1: Check for winning moves
    if (optimizationLevel >= 1) {
        std::vector<std::pair<int, int>> winningMoves;

        for (const auto& move : availableMoves) {
            if (isWinningMoveInPlace(boardCopy, move.first, move.second, playerMark)) {
                winningMoves.push_back(move);
            }
        }

        if (!winningMoves.empty()) {
            // Verbose output
            if (verboseMode) {
                std::cout << "\n══════════════════════════════════════════════════════\n";
                std::cout << "[SmartRandomAI Move Analysis - Player " << playerMark << "]\n";
                std::cout << "══════════════════════════════════════════════════════\n";
                std::cout << "Checked " << availableMoves.size() << " available moves\n";
                std::cout << "Winning moves found: " << winningMoves.size() << "\n";
                for (const auto& move : winningMoves) {
                    std::cout << "  - (" << move.first << ", " << move.second << ")\n";
                }
            }

            // Found winning move(s)! Pick one randomly
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, winningMoves.size() - 1);
            auto winningMove = winningMoves[dis(gen)];

            if (verboseMode) {
                std::cout << "\nSelected winning move: (" << winningMove.first << ", " << winningMove.second << ")\n";
                std::cout << "══════════════════════════════════════════════════════\n\n";
            }

            // Update internal available moves with our chosen move
            availableMoves.erase(winningMove);

            // Add adjacent positions
            for (int i = winningMove.first - 1; i <= winningMove.first + 1; ++i) {
                for (int j = winningMove.second - 1; j <= winningMove.second + 1; ++j) {
                    if (i == winningMove.first && j == winningMove.second) continue;
                    availableMoves.insert({i, j});
                }
            }

            return winningMove;
        }
    }

    // OPTIMIZATION LEVEL 2: Block opponent winning moves
    if (optimizationLevel >= 2) {
        char opponentMark = (playerMark == 'X') ? 'O' : 'X';
        std::vector<std::pair<int, int>> blockingMoves;

        for (const auto& move : availableMoves) {
            if (isWinningMoveInPlace(boardCopy, move.first, move.second, opponentMark)) {
                blockingMoves.push_back(move);
            }
        }

        if (!blockingMoves.empty()) {
            // Verbose output
            if (verboseMode) {
                std::cout << "\n══════════════════════════════════════════════════════\n";
                std::cout << "[SmartRandomAI Move Analysis - Player " << playerMark << "]\n";
                std::cout << "══════════════════════════════════════════════════════\n";
                std::cout << "Checked " << availableMoves.size() << " available moves\n";
                std::cout << "Opponent threatening moves found: " << blockingMoves.size() << "\n";
                for (const auto& move : blockingMoves) {
                    std::cout << "  - (" << move.first << ", " << move.second << ")\n";
                }
            }

            // Found opponent winning move(s) to block! Pick one randomly
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, blockingMoves.size() - 1);
            auto blockingMove = blockingMoves[dis(gen)];

            if (verboseMode) {
                std::cout << "\nSelected blocking move: (" << blockingMove.first << ", " << blockingMove.second << ")\n";
                std::cout << "══════════════════════════════════════════════════════\n\n";
            }

            // Update internal available moves with our chosen move
            availableMoves.erase(blockingMove);

            // Add adjacent positions
            for (int i = blockingMove.first - 1; i <= blockingMove.first + 1; ++i) {
                for (int j = blockingMove.second - 1; j <= blockingMove.second + 1; ++j) {
                    if (i == blockingMove.first && j == blockingMove.second) continue;
                    availableMoves.insert({i, j});
                }
            }

            return blockingMove;
        }
    }

    // No winning or blocking move found (or optimization disabled), pick random move
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, availableMoves.size() - 1);

    auto iter = availableMoves.begin();
    std::advance(iter, dis(gen));
    auto chosenMove = *iter;

    // Update internal available moves with our chosen move
    availableMoves.erase(chosenMove);

    // Add adjacent positions
    for (int i = chosenMove.first - 1; i <= chosenMove.first + 1; ++i) {
        for (int j = chosenMove.second - 1; j <= chosenMove.second + 1; ++j) {
            if (i == chosenMove.first && j == chosenMove.second) continue;
            availableMoves.insert({i, j});
        }
    }

    return chosenMove;
}
