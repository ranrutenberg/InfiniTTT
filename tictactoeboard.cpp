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

    // Evaluate board position by counting potential winning sequences
    // Returns a score based on the number and quality of sequences
int TicTacToeBoard::evaluatePosition(char mark) const
{
    int score = 0;
    char opponent = (mark == 'X') ? 'O' : 'X';
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> countedSequences;

    // Directions: horizontal, vertical, diagonal \, diagonal /
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    for (const auto& [pos, m] : board) {
        if (m != mark) continue;

        int x = pos.first, y = pos.second;

        for (int d = 0; d < 4; ++d) {
            int dx = directions[d][0];
            int dy = directions[d][1];

            // Count consecutive pieces in both directions from this position
            int count = 1;  // Count this position
            int minX = x, minY = y, maxX = x, maxY = y;

            // Count in positive direction
            for (int k = 1; k < 5; ++k) {
                int nx = x + k * dx;
                int ny = y + k * dy;
                if (board.count({nx, ny}) > 0 && board.at({nx, ny}) == mark) {
                    count++;
                    maxX = nx;
                    maxY = ny;
                } else {
                    break;
                }
            }

            // Count in negative direction
            for (int k = 1; k < 5; ++k) {
                int nx = x - k * dx;
                int ny = y - k * dy;
                if (board.count({nx, ny}) > 0 && board.at({nx, ny}) == mark) {
                    count++;
                    minX = nx;
                    minY = ny;
                } else {
                    break;
                }
            }

            // Skip if less than 3 in a row
            if (count < 3) continue;

            // Create a canonical representation of this sequence to avoid double counting
            std::pair<std::pair<int, int>, std::pair<int, int>> sequenceKey =
                {{minX, minY}, {maxX, maxY}};

            // Skip if we've already counted this sequence
            if (countedSequences.count(sequenceKey) > 0) continue;
            countedSequences.insert(sequenceKey);

            // Check openness - can this sequence extend to 5?
            bool openStart = true, openEnd = true;

            // Check if start is blocked
            int startX = minX - dx;
            int startY = minY - dy;
            if (board.count({startX, startY}) > 0 && board.at({startX, startY}) == opponent) {
                openStart = false;
            }

            // Check if end is blocked
            int endX = maxX + dx;
            int endY = maxY + dy;
            if (board.count({endX, endY}) > 0 && board.at({endX, endY}) == opponent) {
                openEnd = false;
            }

            // Calculate spaces needed to win
            int spacesNeeded = 5 - count;

            // If blocked on both ends and can't reach 5, skip
            if (!openStart && !openEnd) continue;
            if (openStart && !openEnd && spacesNeeded > 1) continue;  // Can only extend 1 in one direction
            if (!openStart && openEnd && spacesNeeded > 1) continue;

            // Calculate score for this sequence based on length and openness
            int sequenceScore = 0;

            if (count == 4) {
                // 4 in a row - very valuable (one move from winning)
                if (openStart && openEnd) {
                    sequenceScore = 500;  // Can win on either end
                } else if (openStart || openEnd) {
                    sequenceScore = 200;  // Can win on one end
                }
            } else if (count == 3) {
                // 3 in a row - valuable but less urgent
                if (openStart && openEnd) {
                    sequenceScore = 50;  // Can extend in both directions
                } else if (openStart || openEnd) {
                    sequenceScore = 10;  // Can extend in one direction
                }
            }

            score += sequenceScore;
        }
    }

    return score;
}
