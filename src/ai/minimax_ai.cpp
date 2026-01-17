// Minimax AI - Strategic AI using minimax algorithm with alpha-beta pruning
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "minimax_ai.h"
#include "ai_utils.h"
#include "tictactoeboard.h"
#include "evaluationweights.h"
#include <iostream>
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
        int computerScore = board.evaluatePosition(computerMark, weights);
        int opponentScore = board.evaluatePosition(humanMark, weights);
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

    for (const auto& [i, j] : moves) {
        boardCopy.placeMarkDirect(i, j, playerMark);
        int moveValue = minimax(boardCopy, 0, false, playerMark, humanMark, timeLimitMs, startTime,
                               std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), {i, j});
        boardCopy.removeMarkDirect(i, j);

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
