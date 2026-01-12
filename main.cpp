// Infinite Tic-Tac-Toe - Main game entry point
// Allows human vs human, human vs AI, and AI vs AI gameplay modes
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>

import tictactoeboard;
import aiplayer;

enum class PlayerType {
    HUMAN,
    AI
};

int main() {
    TicTacToeBoard game;
    // Coordinates for player moves on the infinite board
    // x: horizontal position (column), y: vertical position (row)
    // Can be any integer (positive, negative, or zero) to support infinite board
    // TODO: this basically adds a limit to the "infinite" board based on int range. What should we do if we reach that limit?
    int x, y;
    const int winningLength = 5;
    bool gameWon = false;

    // Player configuration
    std::cout << "Welcome to Infinite Tic-Tac-Toe!\n";

    // Configure Player 1 (X)
    std::cout << "\nPlayer 1 (X) - Select type:\n";
    std::cout << "1. Human\n";
    std::cout << "2. AI\n";
    std::cout << "Enter choice (1-2): ";
    int choice1;
    std::cin >> choice1;
    PlayerType player1Type = (choice1 == 1) ? PlayerType::HUMAN : PlayerType::AI;

    // Configure Player 2 (O)
    std::cout << "\nPlayer 2 (O) - Select type:\n";
    std::cout << "1. Human\n";
    std::cout << "2. AI\n";
    std::cout << "Enter choice (1-2): ";
    int choice2;
    std::cin >> choice2;
    PlayerType player2Type = (choice2 == 1) ? PlayerType::HUMAN : PlayerType::AI;

    // Create AI instances if needed
    MinimaxAI* ai1 = (player1Type == PlayerType::AI) ? new MinimaxAI(100) : nullptr;
    MinimaxAI* ai2 = (player2Type == PlayerType::AI) ? new MinimaxAI(100) : nullptr;

    const char player1Mark = 'X';
    const char player2Mark = 'O';
    bool isPlayer1Turn = true;

    std::cout << "\nGame Configuration:\n";
    std::cout << "Player 1 (X): " << ((player1Type == PlayerType::HUMAN) ? "Human" : "AI") << "\n";
    std::cout << "Player 2 (O): " << ((player2Type == PlayerType::HUMAN) ? "Human" : "AI") << "\n";
    std::cout << "The first player to align " << winningLength << " marks in any direction wins.\n";
    std::cout << "Enter coordinates in the format: x y\n\n";

    while (!gameWon) {
        game.printBoard();

        PlayerType currentPlayerType = isPlayer1Turn ? player1Type : player2Type;
        char currentMark = isPlayer1Turn ? player1Mark : player2Mark;
        MinimaxAI* currentAI = isPlayer1Turn ? ai1 : ai2;

        if (currentPlayerType == PlayerType::HUMAN) {
            // Human player's turn
            std::cout << "Player " << currentMark << " (Human), enter your move (x y): ";
            std::cin >> x >> y;

            // Attempt to place the mark; if invalid, prompt again
            if (game.placeMark(x, y)) {
                gameWon = game.checkWin(winningLength);
                isPlayer1Turn = !isPlayer1Turn;  // Switch players
            } else {
                std::cout << "Invalid move. Try again.\n";
            }
        } else {
            // AI player's turn
            std::cout << "Player " << currentMark << " (AI) is making a move...\n";
            auto [moveX, moveY] = currentAI->findBestMove(game, currentMark);

            if (game.placeMark(moveX, moveY)) {
                std::cout << "AI played at (" << moveX << ", " << moveY << ")\n";
                gameWon = game.checkWin(winningLength);
                isPlayer1Turn = !isPlayer1Turn;  // Switch players
            } else {
                std::cout << "AI has no available moves.\n";
                break;
            }
        }
    }

    game.printBoard();
    std::cout << "Game over! Thanks for playing.\n";

    // Clean up AI instances
    delete ai1;
    delete ai2;

    return 0;
}
