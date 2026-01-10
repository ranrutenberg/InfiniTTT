// Infinite Tic-Tac-Toe Board - Header file
// Defines the board class for an expandable tic-tac-toe game with sparse storage
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef TICTACTOEBOARD_H
#define TICTACTOEBOARD_H
#include <map>
#include <utility>

class TicTacToeBoard {
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

#endif // TICTACTOEBOARD_H
