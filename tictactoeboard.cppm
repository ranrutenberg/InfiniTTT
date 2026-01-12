// Infinite Tic-Tac-Toe Board - Module interface
// Defines the board class for an expandable tic-tac-toe game with sparse storage
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

module;

#include <map>
#include <utility>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

export module tictactoeboard;

export class TicTacToeBoard {
private:
    std::map<std::pair<int, int>, char> board;
    char currentPlayer;

    bool checkDirection(int x, int y, int dx, int dy, int length, char mark) const;

public:
    TicTacToeBoard() : currentPlayer('X') {}
    bool placeMark(int x, int y);
    void printBoard(int range = 3) const;
    bool checkWin(int length) const;

    // Helper methods for AI
    const std::map<std::pair<int, int>, char>& getOccupiedPositions() const { return board; }
    bool isPositionOccupied(int x, int y) const { return board.count({x, y}) > 0; }
    void placeMarkDirect(int x, int y, char mark) { board[{x, y}] = mark; }
    void removeMarkDirect(int x, int y) { board.erase({x, y}); }
    char getCurrentPlayer() const { return currentPlayer; }
};

// Check a line in a specific direction for the winning length
bool TicTacToeBoard::checkDirection(int x, int y, int dx, int dy, int length, char mark) const {
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
bool TicTacToeBoard::placeMark(int x, int y) {
    if (board.count({x, y}) > 0) {
        std::cout << "Position already taken.\n";
        return false;
    }
    board[{x, y}] = currentPlayer;
    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
    return true;
}

// Print the board around a specific area
void TicTacToeBoard::printBoard(int range) const {
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
bool TicTacToeBoard::checkWin(int length) const {
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