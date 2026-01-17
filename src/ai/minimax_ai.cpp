// Minimax AI - Strategic AI using minimax algorithm with alpha-beta pruning
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "minimax_ai.h"
#include "ai_utils.h"
#include "tictactoeboard.h"
#include "evaluationweights.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

// Implementation of minimax algorithm with alpha-beta pruning
int MinimaxAI::minimax(TicTacToeBoard& board, int depth, bool isMaximizing,
                       char computerMark, char humanMark, int timeLimitMs,
                       std::chrono::time_point<std::chrono::high_resolution_clock> startTime,
                       int alpha, int beta, std::pair<int, int> lastMove) {

    // Check time limit early
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
    if (elapsedTime.count() >= timeLimitMs) {
        return 0;  // Return neutral score if time limit reached
    }

    // Check for terminal states (if a move was just made)
    if (lastMove.first != INT_MIN && lastMove.second != INT_MIN) {
        if (board.checkWinQuiet(lastMove.first, lastMove.second, 5)) {
            char lastMark = board.getOccupiedPositions().at(lastMove);
            if (lastMark == computerMark) {
                return 1000 - depth;  // Prefer faster wins
            } else {
                return -1000 + depth;  // Prefer slower losses
            }
        }
    }

    // Depth limit reached - evaluate position based on winning sequences
    if (depth >= maxDepth) {
        int computerScore = evaluatePosition(board, computerMark);
        int opponentScore = evaluatePosition(board, humanMark);
        return computerScore - opponentScore;
    }

    // Get all possible moves (compute from board during minimax search)
    auto moves = AIUtils::computeAdjacentMoves(board);

    if (moves.empty()) {
        return 0;  // Draw - no moves available
    }

    if (isMaximizing) {
        int bestScore = std::numeric_limits<int>::min();

        for (const auto& [i, j] : moves) {
            board.placeMarkDirect(i, j, computerMark);
            int score = minimax(board, depth + 1, false, computerMark, humanMark, timeLimitMs, startTime, alpha, beta, {i, j});
            board.removeMarkDirect(i, j);

            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, score);

            if (beta <= alpha) {
                break;  // Beta cutoff
            }
        }

        return bestScore;
    } else {
        int bestScore = std::numeric_limits<int>::max();

        for (const auto& [i, j] : moves) {
            board.placeMarkDirect(i, j, humanMark);
            int score = minimax(board, depth + 1, true, computerMark, humanMark, timeLimitMs, startTime, alpha, beta, {i, j});
            board.removeMarkDirect(i, j);

            bestScore = std::min(bestScore, score);
            beta = std::min(beta, score);

            if (beta <= alpha) {
                break;  // Alpha cutoff
            }
        }

        return bestScore;
    }
}

// Find the best move using minimax
std::pair<int, int> MinimaxAI::findBestMove(const TicTacToeBoard& board, char playerMark,
                                             std::pair<int, int> lastMove) {
    // Create a mutable copy for simulation
    TicTacToeBoard boardCopy = board;

    int bestValue = std::numeric_limits<int>::min();
    std::vector<std::pair<int, int>> bestMoves;  // Store all moves with best score

    char humanMark = (playerMark == 'X') ? 'O' : 'X';
    auto startTime = std::chrono::high_resolution_clock::now();

    // If board is empty, initialize available moves with origin and return it
    if (boardCopy.getOccupiedPositions().empty()) {
        availableMoves.clear();
        availableMoves.insert({0, 0});
        return {0, 0};
    }

    // Update internal available moves based on lastMove
    if (lastMove.first != INT_MIN && lastMove.second != INT_MIN) {
        AIUtils::updateAvailableMoves(availableMoves, boardCopy, lastMove.first, lastMove.second);
    } else if (availableMoves.empty()) {
        // First call - compute from board
        auto tempMoves = AIUtils::computeAdjacentMoves(boardCopy);
        availableMoves.insert(tempMoves.begin(), tempMoves.end());
    }

    // Filter out any occupied positions that may have accumulated
    auto it = availableMoves.begin();
    while (it != availableMoves.end()) {
        if (boardCopy.isPositionOccupied(it->first, it->second)) {
            it = availableMoves.erase(it);
        } else {
            ++it;
        }
    }

    // Convert available moves set to vector for iteration
    std::vector<std::pair<int, int>> moves(availableMoves.begin(), availableMoves.end());

    // Store all move scores for verbose output
    std::vector<MoveScore> allMoveScores;

    for (const auto& [i, j] : moves) {
        boardCopy.placeMarkDirect(i, j, playerMark);
        int moveValue = minimax(boardCopy, 0, false, playerMark, humanMark, timeLimitMs, startTime,
                               std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), {i, j});
        boardCopy.removeMarkDirect(i, j);

        // Store score for verbose output
        allMoveScores.push_back({{i, j}, moveValue});

        if (moveValue > bestValue) {
            bestValue = moveValue;
            bestMoves.clear();
            bestMoves.push_back({i, j});
        } else if (moveValue == bestValue) {
            bestMoves.push_back({i, j});
        }
    }

    // Randomly select one of the best moves
    if (bestMoves.empty()) {
        return {0, 0};
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, bestMoves.size() - 1);

    auto chosenMove = bestMoves[dis(gen)];

    // Print verbose output if enabled
    if (verboseMode && !allMoveScores.empty()) {
        printTopMoves(allMoveScores, chosenMove, playerMark);
    }

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

// Evaluate board position for a given player mark
int MinimaxAI::evaluatePosition(const TicTacToeBoard& board, char mark) const
{
    // Use default weights if none provided
    EvaluationWeights defaultWeights;
    const EvaluationWeights& w = weights ? *weights : defaultWeights;

    int score = 0;
    char opponent = (mark == 'X') ? 'O' : 'X';
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> countedWindows;

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

                // Create canonical key (always store min to max)
                std::pair<std::pair<int, int>, std::pair<int, int>> windowKey =
                    {{startX, startY}, {endX, endY}};

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
                    if (openBefore && openAfter) {
                        windowScore = w.four_open;  // Can complete in the gap or extend
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

    return score;
}

// Print top N moves with scores (verbose mode)
void MinimaxAI::printTopMoves(const std::vector<MoveScore>& moveScores, std::pair<int, int> selected, char playerMark) const {
    std::cout << "\n══════════════════════════════════════════════════════\n";
    std::cout << "[MinimaxAI Move Analysis - Player " << playerMark << "]\n";
    std::cout << "══════════════════════════════════════════════════════\n";
    std::cout << "Evaluating " << moveScores.size() << " available moves...\n\n";

    // Sort by score descending
    std::vector<MoveScore> sortedScores = moveScores;
    std::sort(sortedScores.begin(), sortedScores.end(),
        [](const MoveScore& a, const MoveScore& b) {
            return a.score > b.score;
        });

    // Display top 10 (or fewer if less available)
    int displayCount = std::min(10, static_cast<int>(sortedScores.size()));
    std::cout << "Top " << displayCount << " moves (ranked by score):\n";

    // Find max score for bar chart scaling
    int maxScore = sortedScores[0].score;
    int minScore = sortedScores[std::min(9, static_cast<int>(sortedScores.size()) - 1)].score;
    int scoreRange = maxScore - minScore;
    if (scoreRange == 0) scoreRange = 1;  // Avoid division by zero

    for (int i = 0; i < displayCount; ++i) {
        const auto& ms = sortedScores[i];

        // Format move coordinates
        std::cout << std::setw(3) << (i + 1) << ". ("
                  << std::setw(3) << ms.move.first << ","
                  << std::setw(3) << ms.move.second << ")  Score:"
                  << std::setw(6) << ms.score << "  ";

        // Bar chart visualization (20 characters max)
        int barLength = 20;
        if (scoreRange > 0) {
            barLength = static_cast<int>((static_cast<double>(ms.score - minScore) / scoreRange) * 20);
        }
        for (int j = 0; j < barLength; ++j) std::cout << "█";
        for (int j = barLength; j < 20; ++j) std::cout << "░";

        // Mark if tied with previous
        if (i > 0 && ms.score == sortedScores[i-1].score) {
            std::cout << "  [tied]";
        }

        std::cout << "\n";
    }

    // Find the selected move's score
    int selectedScore = 0;
    for (const auto& ms : moveScores) {
        if (ms.move == selected) {
            selectedScore = ms.score;
            break;
        }
    }

    std::cout << "\nSelected: (" << selected.first << ", " << selected.second << ") with score " << selectedScore << "\n";
    std::cout << "══════════════════════════════════════════════════════\n\n";
}
