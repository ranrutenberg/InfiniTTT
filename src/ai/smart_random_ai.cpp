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

// Count how many positions become winning moves after placing at (x, y).
// Checks candidates PLUS neighbors of (x, y), because winning extensions of the
// new sequence may lie adjacent to the hypothetical position and not yet be in
// the maintained availableMoves set.
int SmartRandomAI::countWinningFollowUps(TicTacToeBoard& board, int x, int y, char playerMark,
                                         const std::set<std::pair<int, int>>& candidates) const {
    board.placeMarkDirect(x, y, playerMark);

    // Expand candidate set with the 8 neighbors of (x, y)
    std::set<std::pair<int, int>> expanded = candidates;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            expanded.insert({x + dx, y + dy});
        }
    }

    int count = 0;
    for (const auto& candidate : expanded) {
        if (candidate.first == x && candidate.second == y) continue;
        if (board.isPositionOccupied(candidate.first, candidate.second)) continue;
        board.placeMarkDirect(candidate.first, candidate.second, playerMark);
        bool wins = board.checkWinQuiet(candidate.first, candidate.second, 5);
        board.removeMarkDirect(candidate.first, candidate.second);
        if (wins) ++count;
    }
    board.removeMarkDirect(x, y);
    return count;
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

    // OPTIMIZATION LEVEL 3: Create a double threat (fork)
    // A double threat move leaves >= 2 winning follow-up positions, e.g. an open triplet _xxx_
    if (optimizationLevel >= 3) {
        std::vector<std::pair<int, int>> doubleThreatMoves;

        for (const auto& move : availableMoves) {
            if (countWinningFollowUps(boardCopy, move.first, move.second, playerMark, availableMoves) >= 2) {
                doubleThreatMoves.push_back(move);
            }
        }

        if (!doubleThreatMoves.empty()) {
            if (verboseMode) {
                std::cout << "\n══════════════════════════════════════════════════════\n";
                std::cout << "[SmartRandomAI Move Analysis - Player " << playerMark << "]\n";
                std::cout << "══════════════════════════════════════════════════════\n";
                std::cout << "Checked " << availableMoves.size() << " available moves\n";
                std::cout << "Double-threat moves found: " << doubleThreatMoves.size() << "\n";
                for (const auto& move : doubleThreatMoves) {
                    std::cout << "  - (" << move.first << ", " << move.second << ")\n";
                }
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, doubleThreatMoves.size() - 1);
            auto chosenMove = doubleThreatMoves[dis(gen)];

            if (verboseMode) {
                std::cout << "\nSelected double-threat move: (" << chosenMove.first << ", " << chosenMove.second << ")\n";
                std::cout << "══════════════════════════════════════════════════════\n\n";
            }

            availableMoves.erase(chosenMove);
            for (int i = chosenMove.first - 1; i <= chosenMove.first + 1; ++i) {
                for (int j = chosenMove.second - 1; j <= chosenMove.second + 1; ++j) {
                    if (i == chosenMove.first && j == chosenMove.second) continue;
                    availableMoves.insert({i, j});
                }
            }

            return chosenMove;
        }
    }

    // OPTIMIZATION LEVEL 4: Block opponent double threats (fork prevention)
    if (optimizationLevel >= 4) {
        char opponentMark = (playerMark == 'X') ? 'O' : 'X';
        std::vector<std::pair<int, int>> blockForkMoves;

        for (const auto& move : availableMoves) {
            if (countWinningFollowUps(boardCopy, move.first, move.second, opponentMark, availableMoves) >= 2) {
                blockForkMoves.push_back(move);
            }
        }

        if (!blockForkMoves.empty()) {
            if (verboseMode) {
                std::cout << "\n══════════════════════════════════════════════════════\n";
                std::cout << "[SmartRandomAI Move Analysis - Player " << playerMark << "]\n";
                std::cout << "══════════════════════════════════════════════════════\n";
                std::cout << "Checked " << availableMoves.size() << " available moves\n";
                std::cout << "Opponent fork-blocking moves found: " << blockForkMoves.size() << "\n";
                for (const auto& move : blockForkMoves) {
                    std::cout << "  - (" << move.first << ", " << move.second << ")\n";
                }
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, blockForkMoves.size() - 1);
            auto chosenMove = blockForkMoves[dis(gen)];

            if (verboseMode) {
                std::cout << "\nSelected fork-blocking move: (" << chosenMove.first << ", " << chosenMove.second << ")\n";
                std::cout << "══════════════════════════════════════════════════════\n\n";
            }

            availableMoves.erase(chosenMove);
            for (int i = chosenMove.first - 1; i <= chosenMove.first + 1; ++i) {
                for (int j = chosenMove.second - 1; j <= chosenMove.second + 1; ++j) {
                    if (i == chosenMove.first && j == chosenMove.second) continue;
                    availableMoves.insert({i, j});
                }
            }

            return chosenMove;
        }
    }

    // OPTIMIZATION LEVEL 5: Create a second-order double threat
    // A second-order double threat creates >= 2 open-3 sequences simultaneously.
    // The opponent can block at most one, so the other becomes a first-order double threat.
    if (optimizationLevel >= 5) {
        std::vector<std::pair<int, int>> doubleOpenThreeMoves;

        for (const auto& move : availableMoves) {
            if (AIUtils::countOpenThreesAtPosition(boardCopy, move.first, move.second, playerMark) >= 2) {
                doubleOpenThreeMoves.push_back(move);
            }
        }

        if (!doubleOpenThreeMoves.empty()) {
            if (verboseMode) {
                std::cout << "\n══════════════════════════════════════════════════════\n";
                std::cout << "[SmartRandomAI Move Analysis - Player " << playerMark << "]\n";
                std::cout << "══════════════════════════════════════════════════════\n";
                std::cout << "Checked " << availableMoves.size() << " available moves\n";
                std::cout << "Second-order double-threat moves found: " << doubleOpenThreeMoves.size() << "\n";
                for (const auto& move : doubleOpenThreeMoves) {
                    std::cout << "  - (" << move.first << ", " << move.second << ")\n";
                }
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, doubleOpenThreeMoves.size() - 1);
            auto chosenMove = doubleOpenThreeMoves[dis(gen)];

            if (verboseMode) {
                std::cout << "\nSelected second-order double-threat move: (" << chosenMove.first << ", " << chosenMove.second << ")\n";
                std::cout << "══════════════════════════════════════════════════════\n\n";
            }

            availableMoves.erase(chosenMove);
            for (int i = chosenMove.first - 1; i <= chosenMove.first + 1; ++i) {
                for (int j = chosenMove.second - 1; j <= chosenMove.second + 1; ++j) {
                    if (i == chosenMove.first && j == chosenMove.second) continue;
                    availableMoves.insert({i, j});
                }
            }

            return chosenMove;
        }
    }

    // OPTIMIZATION LEVEL 6: Block opponent second-order double threats
    if (optimizationLevel >= 6) {
        char opponentMark = (playerMark == 'X') ? 'O' : 'X';
        std::vector<std::pair<int, int>> blockDoubleOpenThreeMoves;

        for (const auto& move : availableMoves) {
            if (AIUtils::countOpenThreesAtPosition(boardCopy, move.first, move.second, opponentMark) >= 2) {
                blockDoubleOpenThreeMoves.push_back(move);
            }
        }

        if (!blockDoubleOpenThreeMoves.empty()) {
            if (verboseMode) {
                std::cout << "\n══════════════════════════════════════════════════════\n";
                std::cout << "[SmartRandomAI Move Analysis - Player " << playerMark << "]\n";
                std::cout << "══════════════════════════════════════════════════════\n";
                std::cout << "Checked " << availableMoves.size() << " available moves\n";
                std::cout << "Opponent second-order double-threat blocking moves found: " << blockDoubleOpenThreeMoves.size() << "\n";
                for (const auto& move : blockDoubleOpenThreeMoves) {
                    std::cout << "  - (" << move.first << ", " << move.second << ")\n";
                }
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, blockDoubleOpenThreeMoves.size() - 1);
            auto chosenMove = blockDoubleOpenThreeMoves[dis(gen)];

            if (verboseMode) {
                std::cout << "\nSelected second-order double-threat block: (" << chosenMove.first << ", " << chosenMove.second << ")\n";
                std::cout << "══════════════════════════════════════════════════════\n\n";
            }

            availableMoves.erase(chosenMove);
            for (int i = chosenMove.first - 1; i <= chosenMove.first + 1; ++i) {
                for (int j = chosenMove.second - 1; j <= chosenMove.second + 1; ++j) {
                    if (i == chosenMove.first && j == chosenMove.second) continue;
                    availableMoves.insert({i, j});
                }
            }

            return chosenMove;
        }
    }

    // No winning, blocking, fork, or fork-blocking move found (or optimization disabled), pick random move
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
