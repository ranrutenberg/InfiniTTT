// Infinite Tic-Tac-Toe Board - Implementation
// Implements board operations including placement, display, and win checking
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "tictactoeboard.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <utility>
#include <algorithm>
#include <cmath>

// Check a line in a specific direction for the winning length
bool TicTacToeBoard::checkDirection ( int x, int y, int dx, int dy, int length, char mark ) const
{
    for (int i = 1; i < length; ++i) {
        int nx = x + i * dx;
        int ny = y + i * dy;
        if (board.count({nx, ny}) == 0 || board.at({nx, ny}) != mark) {
            return false;
        }
    }
    return true;
}


    // Place a mark on the board at a given position
bool TicTacToeBoard::placeMark ( int x, int y )
{ if (board.count({x, y}) > 0) {
            std::cout << "Position already taken.\n";
            return false;
        }
        board[{x, y}] = currentPlayer;
        currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
        return true;
}


    // Print the board around a specific area
void TicTacToeBoard::printBoard ( int range ) const
{
    if (board.empty()) {
        std::cout << "The board is empty.\n";
        return;
    }

    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (const auto& [pos, mark] : board) {
        minX = std::min(minX, pos.first);
        maxX = std::max(maxX, pos.first);
        minY = std::min(minY, pos.second);
        maxY = std::max(maxY, pos.second);
    }

    // Adjust range based on the spread of the marks
    minX = std::max(minX - range, minX);
    maxX = std::min(maxX + range, maxX);
    minY = std::max(minY - range, minY);
    maxY = std::min(maxY + range, maxY);

    // Calculate width needed for coordinates (1 for digit + 1 for space)
    int maxCoordWidth = std::max(
        std::to_string(std::max(std::abs(minX), std::abs(maxX))).length(),
        std::to_string(std::max(std::abs(minY), std::abs(maxY))).length()
    );
    int cellWidth = maxCoordWidth + 1;  // Width for each cell

    // Print the board with coordinates
    for (int i = maxY; i >= minY; --i) {
        // Print the row marks
        for (int j = minX; j <= maxX; ++j) {
            if (board.count({j, i}) > 0) {
                std::cout << std::setw(cellWidth) << board.at({j, i});
            } else {
                std::cout << std::setw(cellWidth) << '.';
            }
        }
        // Print the y-coordinate on the right
        std::cout << "  " << i;
        std::cout << "\n";
    }

    // Print the x-coordinates on the bottom
    for (int j = minX; j <= maxX; ++j) {
        std::cout << std::setw(cellWidth) << j;
    }
    std::cout << "\n";
}

    // Check for a winning line of a specified length
bool TicTacToeBoard::checkWin ( int length ) const
{
    for (const auto& [pos, mark] : board) {
        int x = pos.first, y = pos.second;

        if (checkDirection(x, y, 1, 0, length, mark) || // Horizontal
            checkDirection(x, y, 0, 1, length, mark) || // Vertical
            checkDirection(x, y, 1, 1, length, mark) || // Diagonal \ direction
            checkDirection(x, y, 1, -1, length, mark))  // Diagonal /
        {
            std::cout << "Player " << mark << " wins!\n";
            return true;
        }
    }
    return false;
}

    // Check for win without printing (for training)
bool TicTacToeBoard::checkWinQuiet ( int length ) const
{
    for (const auto& [pos, mark] : board) {
        int x = pos.first, y = pos.second;

        if (checkDirection(x, y, 1, 0, length, mark) || // Horizontal
            checkDirection(x, y, 0, 1, length, mark) || // Vertical
            checkDirection(x, y, 1, 1, length, mark) || // Diagonal \ direction
            checkDirection(x, y, 1, -1, length, mark))  // Diagonal /
        {
            return true;
        }
    }
    return false;
}

    // Evaluate board position by counting potential winning sequences
    // Returns a score based on the number and quality of sequences, including gapped patterns
int TicTacToeBoard::evaluatePosition(char mark, const EvaluationWeights* weights) const
{
    // Use default weights if none provided
    EvaluationWeights defaultWeights;
    const EvaluationWeights& w = weights ? *weights : defaultWeights;

    int score = 0;
    char opponent = (mark == 'X') ? 'O' : 'X';
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> countedWindows;

    // Directions: horizontal, vertical, diagonal \, diagonal /
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    // For each occupied position of our mark, examine 5-cell windows in all directions
    for (const auto& [pos, m] : board) {
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

                    if (board.count({cellX, cellY}) == 0) {
                        emptyCount++;
                    } else if (board.at({cellX, cellY}) == mark) {
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

                bool openBefore = (board.count({beforeX, beforeY}) == 0 ||
                                  board.at({beforeX, beforeY}) != opponent);
                bool openAfter = (board.count({afterX, afterY}) == 0 ||
                                 board.at({afterX, afterY}) != opponent);

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
