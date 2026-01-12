// AI Player implementation for Tic-Tac-Toe
// Implements minimax algorithm with alpha-beta pruning, depth limiting, and move generation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "aiplayer.h"
#include "tictactoeboard.h"
#include <iostream>
#include <limits>
#include <chrono>
#include <vector>
#include <random>
#include <set>
#include <algorithm>

// Helper function to get all valid adjacent moves (deduplicated)
static std::vector<std::pair<int, int>> getAdjacentMoves(const TicTacToeBoard& board) {
    std::vector<std::pair<int, int>> moves;
    auto occupiedPositions = board.getOccupiedPositions();

    // Use a set to deduplicate positions
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

// Helper function to check if a player has won (without printing)
static bool checkWinQuiet(const TicTacToeBoard& board, char mark, int length = 5) {
    auto occupiedPositions = board.getOccupiedPositions();

    for (const auto& [pos, m] : occupiedPositions) {
        if (m != mark) continue;

        int x = pos.first, y = pos.second;

        // Check all 4 directions (horizontal, vertical, diagonal \, diagonal /)
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

        for (auto& dir : directions) {
            int count = 1;  // Count the starting position

            // Check in positive direction
            for (int k = 1; k < length; ++k) {
                int nx = x + k * dir[0];
                int ny = y + k * dir[1];
                if (board.isPositionOccupied(nx, ny) &&
                    occupiedPositions.at({nx, ny}) == mark) {
                    count++;
                } else {
                    break;
                }
            }

            // Check in negative direction
            for (int k = 1; k < length; ++k) {
                int nx = x - k * dir[0];
                int ny = y - k * dir[1];
                if (board.isPositionOccupied(nx, ny) &&
                    occupiedPositions.at({nx, ny}) == mark) {
                    count++;
                } else {
                    break;
                }
            }

            if (count >= length) return true;
        }
    }

    return false;
}

// Implementation of minimax algorithm with alpha-beta pruning
int MinimaxAI::minimax(TicTacToeBoard& board, int depth, bool isMaximizing,
                       char computerMark, char humanMark, int timeLimitMs,
                       std::chrono::time_point<std::chrono::high_resolution_clock> startTime,
                       int alpha, int beta) {

    // Check time limit early
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
    if (elapsedTime.count() >= timeLimitMs) {
        return 0;  // Return neutral score if time limit reached
    }

    // Check for terminal states
    if (checkWinQuiet(board, computerMark)) {
        return 1000 - depth;  // Prefer faster wins
    }
    if (checkWinQuiet(board, humanMark)) {
        return -1000 + depth;  // Prefer slower losses
    }

    // Depth limit reached
    if (depth >= maxDepth) {
        return 0;  // Neutral evaluation
    }

    // Get all possible moves
    auto moves = getAdjacentMoves(board);

    if (moves.empty()) {
        return 0;  // Draw - no moves available
    }

    if (isMaximizing) {
        int bestScore = std::numeric_limits<int>::min();

        for (const auto& [i, j] : moves) {
            board.placeMarkDirect(i, j, computerMark);
            int score = minimax(board, depth + 1, false, computerMark, humanMark, timeLimitMs, startTime, alpha, beta);
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
            int score = minimax(board, depth + 1, true, computerMark, humanMark, timeLimitMs, startTime, alpha, beta);
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
std::pair<int, int> MinimaxAI::findBestMove(const TicTacToeBoard& board, char playerMark) {
    // Create a mutable copy for simulation
    TicTacToeBoard boardCopy = board;

    int bestValue = std::numeric_limits<int>::min();
    std::vector<std::pair<int, int>> bestMoves;  // Store all moves with best score

    char humanMark = (playerMark == 'X') ? 'O' : 'X';
    auto startTime = std::chrono::high_resolution_clock::now();

    // If board is empty, make first move at origin
    if (boardCopy.getOccupiedPositions().empty()) {
        return {0, 0};
    }

    // Get all possible moves using the helper function
    auto moves = getAdjacentMoves(boardCopy);

    for (const auto& [i, j] : moves) {
        boardCopy.placeMarkDirect(i, j, playerMark);
        int moveValue = minimax(boardCopy, 0, false, playerMark, humanMark, timeLimitMs, startTime,
                               std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
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

    return bestMoves[dis(gen)];
}

// RandomAI implementation - picks a random adjacent move
std::pair<int, int> RandomAI::findBestMove(const TicTacToeBoard& board, char playerMark) {
    // If board is empty, make first move at origin
    if (board.getOccupiedPositions().empty()) {
        return {0, 0};
    }

    // Get all possible adjacent moves
    auto moves = getAdjacentMoves(board);

    if (moves.empty()) {
        return {0, 0};
    }

    // Randomly select a move
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, moves.size() - 1);

    return moves[dis(gen)];
}
