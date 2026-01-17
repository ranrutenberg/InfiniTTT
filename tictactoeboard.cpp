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

// Count consecutive marks in a given direction (positive or negative)
int TicTacToeBoard::countConsecutive(int x, int y, int dx, int dy, char mark) const
{
    int count = 0;
    int nx = x + dx;
    int ny = y + dy;

    while (board.count({nx, ny}) > 0 && board.at({nx, ny}) == mark) {
        count++;
        nx += dx;
        ny += dy;
    }

    return count;
}

// Check if a specific position is part of a winning sequence
bool TicTacToeBoard::checkWinFromPosition(int x, int y, int length, char mark) const
{
    // Check all four directions: horizontal, vertical, and two diagonals
    const int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};

    for (const auto& dir : directions) {
        int dx = dir[0];
        int dy = dir[1];

        // Count consecutive marks in both positive and negative directions
        int positiveCount = countConsecutive(x, y, dx, dy, mark);
        int negativeCount = countConsecutive(x, y, -dx, -dy, mark);

        // Total consecutive marks including the current position
        int totalCount = positiveCount + negativeCount + 1;

        if (totalCount >= length) {
            return true;
        }
    }

    return false;
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
bool TicTacToeBoard::checkWin ( int x, int y, int length ) const
{
    // Get the mark at the last move position
    if (board.count({x, y}) == 0) {
        return false;  // No mark at this position
    }

    char mark = board.at({x, y});

    // Check if the last move is part of a winning sequence
    if (checkWinFromPosition(x, y, length, mark)) {
        std::cout << "Player " << mark << " wins!\n";
        return true;
    }

    return false;
}

    // Check for win without printing (for training)
bool TicTacToeBoard::checkWinQuiet ( int x, int y, int length ) const
{
    // Get the mark at the last move position
    if (board.count({x, y}) == 0) {
        return false;  // No mark at this position
    }

    char mark = board.at({x, y});

    // Check if the last move is part of a winning sequence
    return checkWinFromPosition(x, y, length, mark);
}

    // Evaluate board position by counting potential winning sequences
    // Returns a score based on the number and quality of sequences, including gapped patterns
