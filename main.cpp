// Infinite Tic-Tac-Toe - Main game entry point
// Allows human vs human, human vs AI, and AI vs AI gameplay modes
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <iomanip>
#include "tictactoeboard.h" // Include the TicTacToeBoard class
#include "aiplayer.h"       // Include the AIPlayer class

enum class PlayerType {
    HUMAN,
    AI
};

enum class AIType {
    MINIMAX,
    RANDOM
};

// Function to create AI instance based on type
AIPlayer* createAI(AIType type) {
    switch (type) {
        case AIType::MINIMAX:
            return new MinimaxAI(100, 3);
        case AIType::RANDOM:
            return new RandomAI();
        default:
            return new RandomAI();
    }
}

// Function to get AI type name
const char* getAITypeName(AIType type) {
    switch (type) {
        case AIType::MINIMAX:
            return "Minimax";
        case AIType::RANDOM:
            return "Random";
        default:
            return "Unknown";
    }
}

// Run a single game and return winner ('X', 'O', or 'D' for draw)
char runSingleGame(AIType ai1Type, AIType ai2Type, bool quiet = false) {
    TicTacToeBoard game;
    int x, y;
    const int winningLength = 5;
    const int maxMoves = 1000;  // Prevent infinite games
    int moveCount = 0;

    AIPlayer* ai1 = createAI(ai1Type);
    AIPlayer* ai2 = createAI(ai2Type);

    const char player1Mark = 'X';
    const char player2Mark = 'O';
    bool isPlayer1Turn = true;

    while (moveCount < maxMoves) {
        if (!quiet) {
            game.printBoard();
        }

        char currentMark = isPlayer1Turn ? player1Mark : player2Mark;
        AIPlayer* currentAI = isPlayer1Turn ? ai1 : ai2;

        auto [moveX, moveY] = currentAI->findBestMove(game, currentMark);

        if (!game.placeMark(moveX, moveY)) {
            if (!quiet) {
                std::cout << "No valid moves available.\n";
            }
            break;
        }

        if (!quiet) {
            std::cout << "Player " << currentMark << " played at (" << moveX << ", " << moveY << ")\n";
        }

        if (game.checkWin(winningLength)) {
            if (!quiet) {
                game.printBoard();
            }
            delete ai1;
            delete ai2;
            return currentMark;
        }

        isPlayer1Turn = !isPlayer1Turn;
        moveCount++;
    }

    delete ai1;
    delete ai2;
    return 'D';  // Draw
}

// Get AI type from user selection
AIType selectAIType(const std::string& playerName) {
    std::cout << "\nSelect " << playerName << " AI type:\n";
    std::cout << "1. Minimax (Strategic)\n";
    std::cout << "2. Random\n";
    std::cout << "Enter choice (1-2): ";
    int choice;
    std::cin >> choice;

    return (choice == 2) ? AIType::RANDOM : AIType::MINIMAX;
}

// Structure to hold benchmark statistics
struct BenchmarkStats {
    int xWins = 0;
    int oWins = 0;
    int draws = 0;
    int totalMoves = 0;
    int shortestGame = std::numeric_limits<int>::max();
    int longestGame = 0;

    double getXWinRate() const { return 100.0 * xWins / (xWins + oWins + draws); }
    double getOWinRate() const { return 100.0 * oWins / (xWins + oWins + draws); }
    double getDrawRate() const { return 100.0 * draws / (xWins + oWins + draws); }
    double getAvgMoves() const { return static_cast<double>(totalMoves) / (xWins + oWins + draws); }
};

// Helper to check win without printing
bool checkWinSilent(TicTacToeBoard& game, int winningLength) {
    // Save cout state and redirect to null
    std::streambuf* oldCoutBuf = std::cout.rdbuf();
    std::cout.rdbuf(nullptr);

    bool result = game.checkWin(winningLength);

    // Restore cout
    std::cout.rdbuf(oldCoutBuf);

    return result;
}

// Run a single game and return winner + move count
std::pair<char, int> runSingleGameWithStats(AIType ai1Type, AIType ai2Type) {
    TicTacToeBoard game;
    const int winningLength = 5;
    const int maxMoves = 1000;
    int moveCount = 0;

    AIPlayer* ai1 = createAI(ai1Type);
    AIPlayer* ai2 = createAI(ai2Type);

    const char player1Mark = 'X';
    const char player2Mark = 'O';
    bool isPlayer1Turn = true;

    while (moveCount < maxMoves) {
        char currentMark = isPlayer1Turn ? player1Mark : player2Mark;
        AIPlayer* currentAI = isPlayer1Turn ? ai1 : ai2;

        auto [moveX, moveY] = currentAI->findBestMove(game, currentMark);

        if (!game.placeMark(moveX, moveY)) {
            break;
        }

        moveCount++;

        if (checkWinSilent(game, winningLength)) {
            delete ai1;
            delete ai2;
            return {currentMark, moveCount};
        }

        isPlayer1Turn = !isPlayer1Turn;
    }

    delete ai1;
    delete ai2;
    return {'D', moveCount};
}

// Run benchmark comparing AI types
void runBenchmark(int numGames, bool interactive) {
    std::cout << "\n=== AI Benchmark Mode ===\n";

    AIType ai1Type, ai2Type;

    if (interactive) {
        std::cout << "\nYou can choose which AI types to compare.\n";
        ai1Type = selectAIType("Player X");
        ai2Type = selectAIType("Player O");

        std::cout << "\nHow many games to run? (default 50): ";
        std::string input;
        std::cin >> input;
        if (!input.empty() && input != "\n") {
            numGames = std::atoi(input.c_str());
        }
    } else {
        // Run all matchups
        AIType aiTypes[] = {AIType::MINIMAX, AIType::RANDOM};
        int numTypes = 2;

        std::cout << "Running " << numGames << " games for each matchup...\n\n";

        for (int i = 0; i < numTypes; i++) {
            for (int j = 0; j < numTypes; j++) {
                BenchmarkStats stats;

                std::cout << getAITypeName(aiTypes[i]) << " (X) vs "
                          << getAITypeName(aiTypes[j]) << " (O): ";
                std::cout.flush();

                for (int game = 0; game < numGames; game++) {
                    auto [result, moves] = runSingleGameWithStats(aiTypes[i], aiTypes[j]);

                    if (result == 'X') stats.xWins++;
                    else if (result == 'O') stats.oWins++;
                    else stats.draws++;

                    stats.totalMoves += moves;
                    stats.shortestGame = std::min(stats.shortestGame, moves);
                    stats.longestGame = std::max(stats.longestGame, moves);

                    if ((game + 1) % 10 == 0) {
                        std::cout << "." << std::flush;
                    }
                }

                std::cout << "\n";
                std::cout << "  X wins: " << stats.xWins << " (" << std::fixed << std::setprecision(1)
                          << stats.getXWinRate() << "%)\n";
                std::cout << "  O wins: " << stats.oWins << " (" << std::fixed << std::setprecision(1)
                          << stats.getOWinRate() << "%)\n";
                std::cout << "  Draws:  " << stats.draws << " (" << std::fixed << std::setprecision(1)
                          << stats.getDrawRate() << "%)\n";
                std::cout << "  Avg moves: " << std::fixed << std::setprecision(1) << stats.getAvgMoves() << "\n";
                std::cout << "  Shortest: " << stats.shortestGame << " moves, Longest: " << stats.longestGame << " moves\n\n";
            }
        }
        return;
    }

    // Interactive mode - single matchup
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "Running " << numGames << " games:\n";
    std::cout << "  " << getAITypeName(ai1Type) << " (X) vs " << getAITypeName(ai2Type) << " (O)\n";
    std::cout << std::string(50, '=') << "\n\n";

    BenchmarkStats stats;

    for (int game = 0; game < numGames; game++) {
        auto [result, moves] = runSingleGameWithStats(ai1Type, ai2Type);

        if (result == 'X') stats.xWins++;
        else if (result == 'O') stats.oWins++;
        else stats.draws++;

        stats.totalMoves += moves;
        stats.shortestGame = std::min(stats.shortestGame, moves);
        stats.longestGame = std::max(stats.longestGame, moves);

        // Show progress bar
        if ((game + 1) % 10 == 0 || game == numGames - 1) {
            int percent = ((game + 1) * 100) / numGames;
            std::cout << "\rProgress: [";
            int barWidth = 30;
            int pos = barWidth * (game + 1) / numGames;
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << percent << "% (" << (game + 1) << "/" << numGames << ")";
            std::cout.flush();
        }
    }

    std::cout << "\n\n";
    std::cout << std::string(50, '=') << "\n";
    std::cout << "FINAL STATISTICS\n";
    std::cout << std::string(50, '=') << "\n\n";

    std::cout << "Matchup: " << getAITypeName(ai1Type) << " (X) vs "
              << getAITypeName(ai2Type) << " (O)\n";
    std::cout << "Total games: " << numGames << "\n\n";

    std::cout << "WIN STATISTICS:\n";
    std::cout << "  Player X (" << getAITypeName(ai1Type) << "):\n";
    std::cout << "    Wins: " << stats.xWins << " (" << std::fixed << std::setprecision(1)
              << stats.getXWinRate() << "%)\n";

    std::cout << "  Player O (" << getAITypeName(ai2Type) << "):\n";
    std::cout << "    Wins: " << stats.oWins << " (" << std::fixed << std::setprecision(1)
              << stats.getOWinRate() << "%)\n";

    std::cout << "  Draws: " << stats.draws << " (" << std::fixed << std::setprecision(1)
              << stats.getDrawRate() << "%)\n\n";

    std::cout << "GAME LENGTH STATISTICS:\n";
    std::cout << "  Average moves per game: " << std::fixed << std::setprecision(1)
              << stats.getAvgMoves() << "\n";
    std::cout << "  Shortest game: " << stats.shortestGame << " moves\n";
    std::cout << "  Longest game: " << stats.longestGame << " moves\n\n";

    // Determine winner
    if (stats.xWins > stats.oWins) {
        std::cout << "OVERALL WINNER: Player X (" << getAITypeName(ai1Type) << ")\n";
    } else if (stats.oWins > stats.xWins) {
        std::cout << "OVERALL WINNER: Player O (" << getAITypeName(ai2Type) << ")\n";
    } else {
        std::cout << "RESULT: TIE - Both players performed equally well!\n";
    }

    std::cout << std::string(50, '=') << "\n";
}

// Interactive game mode
void runInteractiveGame() {
    TicTacToeBoard game;
    int x, y;
    const int winningLength = 5;
    bool gameWon = false;

    std::cout << "Welcome to Infinite Tic-Tac-Toe!\n";

    // Configure Player 1 (X)
    std::cout << "\nPlayer 1 (X) - Select type:\n";
    std::cout << "1. Human\n";
    std::cout << "2. AI (Minimax)\n";
    std::cout << "3. AI (Random)\n";
    std::cout << "Enter choice (1-3): ";
    int choice1;
    std::cin >> choice1;

    PlayerType player1Type = (choice1 == 1) ? PlayerType::HUMAN : PlayerType::AI;
    AIType ai1Type = AIType::MINIMAX;  // Default
    if (choice1 == 3) ai1Type = AIType::RANDOM;

    // Configure Player 2 (O)
    std::cout << "\nPlayer 2 (O) - Select type:\n";
    std::cout << "1. Human\n";
    std::cout << "2. AI (Minimax)\n";
    std::cout << "3. AI (Random)\n";
    std::cout << "Enter choice (1-3): ";
    int choice2;
    std::cin >> choice2;

    PlayerType player2Type = (choice2 == 1) ? PlayerType::HUMAN : PlayerType::AI;
    AIType ai2Type = AIType::MINIMAX;  // Default
    if (choice2 == 3) ai2Type = AIType::RANDOM;

    // Create AI instances if needed
    AIPlayer* ai1 = (player1Type == PlayerType::AI) ? createAI(ai1Type) : nullptr;
    AIPlayer* ai2 = (player2Type == PlayerType::AI) ? createAI(ai2Type) : nullptr;

    const char player1Mark = 'X';
    const char player2Mark = 'O';
    bool isPlayer1Turn = true;

    std::cout << "\nGame Configuration:\n";
    std::cout << "Player 1 (X): ";
    if (player1Type == PlayerType::HUMAN) {
        std::cout << "Human\n";
    } else {
        std::cout << "AI (" << getAITypeName(ai1Type) << ")\n";
    }

    std::cout << "Player 2 (O): ";
    if (player2Type == PlayerType::HUMAN) {
        std::cout << "Human\n";
    } else {
        std::cout << "AI (" << getAITypeName(ai2Type) << ")\n";
    }

    std::cout << "The first player to align " << winningLength << " marks in any direction wins.\n";
    std::cout << "Enter coordinates in the format: x y\n\n";

    while (!gameWon) {
        game.printBoard();

        PlayerType currentPlayerType = isPlayer1Turn ? player1Type : player2Type;
        char currentMark = isPlayer1Turn ? player1Mark : player2Mark;
        AIPlayer* currentAI = isPlayer1Turn ? ai1 : ai2;

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
            AIType currentAIType = isPlayer1Turn ? ai1Type : ai2Type;
            std::cout << "Player " << currentMark << " (" << getAITypeName(currentAIType) << ") is making a move...\n";
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
}

int main(int argc, char* argv[]) {
    // Check for benchmark mode
    if (argc > 1 && std::string(argv[1]) == "--benchmark") {
        int numGames = 50;  // Default
        bool interactive = true;  // Default to interactive

        if (argc > 2) {
            if (std::string(argv[2]) == "--all") {
                interactive = false;
            } else {
                numGames = std::atoi(argv[2]);
            }
        }

        if (argc > 3) {
            numGames = std::atoi(argv[3]);
        }

        runBenchmark(numGames, interactive);
    } else {
        runInteractiveGame();
    }

    return 0;
}