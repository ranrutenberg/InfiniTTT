// AI Player interface for Tic-Tac-Toe - Header file
// Defines abstract AI player interface
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <utility>
#include <climits>

class TicTacToeBoard;

// Abstract base class for AI players
class AIPlayer {
public:
    virtual ~AIPlayer() = default;

    // Find and return the best move for the current board state
    // lastMove: the last move made by the opponent (x, y coordinates)
    //           Use {INT_MIN, INT_MIN} to indicate no last move (first move of game)
    virtual std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                              std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) = 0;
};
