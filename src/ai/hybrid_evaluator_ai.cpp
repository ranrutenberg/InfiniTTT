// Hybrid Evaluator AI - Combines tactical play with strategic position evaluation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "hybrid_evaluator_ai.h"
#include "ai_utils.h"
#include "tictactoeboard.h"
#include "evaluationweights.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <limits>

// Helper to check if a move results in a win (from SmartRandomAI)
bool HybridEvaluatorAI::isWinningMove(const TicTacToeBoard& board, int x, int y, char playerMark, int winLength) const {
    // Create a copy of the board and try the move
    TicTacToeBoard boardCopy = board;
    boardCopy.placeMarkDirect(x, y, playerMark);

    // Check if this move creates a win using checkWinQuiet with the move position
    bool wins = boardCopy.checkWinQuiet(x, y, winLength);

    return wins;
}

// Evaluate board position for a given player mark (from MinimaxAI)
int HybridEvaluatorAI::evaluatePosition(const TicTacToeBoard& board, char mark) const
{
    // Use default weights if none provided
    EvaluationWeights defaultWeights;
    const EvaluationWeights& w = weights ? *weights : defaultWeights;

    int score = 0;
    char opponent = (mark == 'X') ? 'O' : 'X';
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> countedWindows;
    std::set<std::pair<int, int>> winningMoves;  // Positions that create immediate win

    // Directions: horizontal, vertical, diagonal \, diagonal /
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    // Get all occupied positions from the board
    auto occupiedPositions = board.getOccupiedPositions();

    // For each occupied position of our mark, examine 5-cell windows in all directions
    for (const auto& [pos, m] : occupiedPositions) {
        if (m != mark) continue;

        int x = pos.first, y = pos.second;

        for (int d = 0; d < 4; ++d) {
            int dx = directions[d][0];
            int dy = directions[d][1];

            // Try different starting positions for 5-cell windows containing this piece
            // We want to check windows where this piece is at positions 0, 1, 2, 3, or 4
            for (int offset = 0; offset < 5; ++offset) {
                // Calculate window start position
                int startX = x - offset * dx;
                int startY = y - offset * dy;
                int endX = startX + 4 * dx;
                int endY = startY + 4 * dy;

                // Create canonical key (normalize by direction and position)
                // For any window, store it with the lexicographically smaller endpoint first
                auto p1 = std::make_pair(startX, startY);
                auto p2 = std::make_pair(endX, endY);
                std::pair<std::pair<int, int>, std::pair<int, int>> windowKey =
                    (p1 < p2) ? std::make_pair(p1, p2) : std::make_pair(p2, p1);

                // Skip if already evaluated this window
                if (countedWindows.count(windowKey) > 0) continue;
                countedWindows.insert(windowKey);

                // Analyze this 5-cell window
                int friendlyCount = 0;
                int opponentCount = 0;
                int emptyCount = 0;

                for (int k = 0; k < 5; ++k) {
                    int cellX = startX + k * dx;
                    int cellY = startY + k * dy;

                    if (!board.isPositionOccupied(cellX, cellY)) {
                        emptyCount++;
                    } else if (occupiedPositions.at({cellX, cellY}) == mark) {
                        friendlyCount++;
                    } else {
                        opponentCount++;
                    }
                }

                // If opponent has any pieces in this window, it's blocked - skip it
                if (opponentCount > 0) continue;

                // If we have less than 2 pieces in this window, it's not valuable
                if (friendlyCount < 2) continue;

                // Check if the ends are open (for extending beyond 5)
                int beforeX = startX - dx;
                int beforeY = startY - dy;
                int afterX = endX + dx;
                int afterY = endY + dy;

                bool openBefore = !board.isPositionOccupied(beforeX, beforeY) ||
                                  occupiedPositions.at({beforeX, beforeY}) != opponent;
                bool openAfter = !board.isPositionOccupied(afterX, afterY) ||
                                 occupiedPositions.at({afterX, afterY}) != opponent;

                // Score based on pattern quality
                int windowScore = 0;

                if (friendlyCount == 4) {
                    // 4 pieces in a 5-cell window - one move from winning
                    // Patterns: XXXX_, XXX_X, XX_XX, X_XXX, _XXXX

                    // Find the empty position(s) that create a win
                    for (int k = 0; k < 5; ++k) {
                        int cellX = startX + k * dx;
                        int cellY = startY + k * dy;
                        if (!board.isPositionOccupied(cellX, cellY)) {
                            winningMoves.insert({cellX, cellY});
                        }
                    }

                    if (openBefore && openAfter) {
                        windowScore = w.four_open;  // Can complete in the gap or extend
                        // For open-4, both ends can also create a win if extendable
                        // But we already counted the gap, so this is already a double threat
                    } else {
                        windowScore = w.four_blocked;  // Can still complete in the gap
                    }
                } else if (friendlyCount == 3) {
                    // 3 pieces in a 5-cell window - building toward a win
                    // Patterns: XXX__, XX_X_, XX__X, X_XX_, X_X_X, X__XX, etc.
                    if (emptyCount == 2) {
                        if (openBefore && openAfter) {
                            windowScore = w.three_open;  // Good potential, multiple ways to extend
                        } else {
                            windowScore = w.three_blocked;  // Still useful
                        }
                    }
                } else if (friendlyCount == 2) {
                    // 2 pieces in a 5-cell window - early building
                    if (emptyCount == 3 && openBefore && openAfter) {
                        windowScore = w.two_open;  // Minor value for positioning
                    }
                }

                score += windowScore;
            }
        }
    }

    // Apply double threat bonus if we have multiple winning moves
    // This represents positions where opponent cannot defend all threats
    if (winningMoves.size() >= 2) {
        score += w.double_threat;
    }

    return score;
}

// Find best move using three-level priority system
std::pair<int, int> HybridEvaluatorAI::findBestMove(const TicTacToeBoard& board, char playerMark,
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

    if (verboseMode) {
        std::cout << "\n[HybridEvaluatorAI - Player " << playerMark << "]\n";
        std::cout << "Evaluating " << availableMoves.size() << " available moves\n";
    }

    // PRIORITY LEVEL 1: Check for winning moves
    std::vector<std::pair<int, int>> winningMoves;

    for (const auto& move : availableMoves) {
        if (isWinningMove(board, move.first, move.second, playerMark)) {
            winningMoves.push_back(move);
        }
    }

    if (!winningMoves.empty()) {
        if (verboseMode) {
            std::cout << "Priority 1: Winning moves - " << winningMoves.size() << " found\n";
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
            std::cout << "Selected winning move: (" << winningMove.first << ", " << winningMove.second << ")\n\n";
        }

        // Update internal available moves
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

    if (verboseMode) {
        std::cout << "Priority 1: Winning moves - 0 found\n";
    }

    // PRIORITY LEVEL 2: Block opponent winning moves
    char opponentMark = (playerMark == 'X') ? 'O' : 'X';
    std::vector<std::pair<int, int>> blockingMoves;

    for (const auto& move : availableMoves) {
        if (isWinningMove(board, move.first, move.second, opponentMark)) {
            blockingMoves.push_back(move);
        }
    }

    if (!blockingMoves.empty()) {
        if (verboseMode) {
            std::cout << "Priority 2: Blocking moves - " << blockingMoves.size() << " found\n";
            for (const auto& move : blockingMoves) {
                std::cout << "  Blocking threat at (" << move.first << ", " << move.second << ")\n";
            }
        }

        // Found opponent winning move(s) to block! Pick one randomly
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, blockingMoves.size() - 1);
        auto blockingMove = blockingMoves[dis(gen)];

        if (verboseMode) {
            std::cout << "Selected blocking move: (" << blockingMove.first << ", " << blockingMove.second << ")\n\n";
        }

        // Update internal available moves
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

    if (verboseMode) {
        std::cout << "Priority 2: Blocking moves - 0 found\n";
        std::cout << "Priority 3: Position evaluation\n";
    }

    // PRIORITY LEVEL 3: Evaluate all moves using position scoring
    struct MoveScore {
        std::pair<int, int> move;
        int score;
        int ourSeq;
        int oppSeq;
    };
    std::vector<MoveScore> moveScores;

    for (const auto& move : availableMoves) {
        TicTacToeBoard boardCopy = board;
        boardCopy.placeMarkDirect(move.first, move.second, playerMark);

        int ourScore = evaluatePosition(boardCopy, playerMark);
        int oppScore = evaluatePosition(boardCopy, opponentMark);
        int netScore = ourScore - oppScore;

        moveScores.push_back({move, netScore, ourScore, oppScore});
    }

    // Find best score
    int bestScore = std::numeric_limits<int>::min();
    std::vector<MoveScore> bestMoves;

    for (const auto& ms : moveScores) {
        if (ms.score > bestScore) {
            bestScore = ms.score;
            bestMoves.clear();
            bestMoves.push_back(ms);
        } else if (ms.score == bestScore) {
            bestMoves.push_back(ms);
        }
    }

    if (verboseMode && !bestMoves.empty()) {
        std::cout << "  Best score: " << bestScore << " (" << bestMoves.size() << " move(s) tied)\n";
        std::cout << "  Top move: (" << bestMoves[0].move.first << ", " << bestMoves[0].move.second << ")";
        std::cout << " (Our: " << bestMoves[0].ourSeq << ", Opp: " << bestMoves[0].oppSeq << ")\n";
    }

    // Random tie-breaking
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, bestMoves.size() - 1);
    auto chosenMove = bestMoves[dis(gen)].move;

    if (verboseMode) {
        std::cout << "Selected: (" << chosenMove.first << ", " << chosenMove.second << ")\n\n";
    }

    // Update internal available moves
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
