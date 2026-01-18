// Infinite Tic-Tac-Toe - Main game entry point
// Allows human vs human, human vs AI, and AI vs AI gameplay modes
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include <iomanip>
#include <climits>
#include <limits>
#include <memory>
#include <map>
#include "tictactoeboard.h" // Include the TicTacToeBoard class
#include "ai_types.h"              // Include AIType enum
#include "src/ai/aiplayer.h"       // Include the AIPlayer class
#include "src/ai/smart_random_ai.h" // Include SmartRandomAI
#include "src/ai/hybrid_evaluator_ai.h" // Include HybridEvaluatorAI
#include "src/ai/hybrid_evaluator_ai_v2.h" // Include HybridEvaluatorAIv2 (Minimax)
#include "weighttrainer.h"  // Include the weight training system
#include "evaluationweights.h"  // Include evaluation weights

enum class PlayerType {
    HUMAN,
    AI
};

// Function to create AI instance based on type
// Pass weights pointer to enable trained weights (nullptr for default weights)
// For HYBRID_EVALUATOR_V2: depth and topN parameters can be customized
std::unique_ptr<AIPlayer> createAI(AIType type, const EvaluationWeights* weights = nullptr, bool verbose = false,
                                    int depth = 2, int topN = 10) {
    switch (type) {
        case AIType::SMART_RANDOM:
            return std::make_unique<SmartRandomAI>(2, verbose);  // Level 2 optimization
        case AIType::HYBRID_EVALUATOR:
            return std::make_unique<HybridEvaluatorAI>(weights, verbose);
        case AIType::HYBRID_EVALUATOR_V2:
            return std::make_unique<HybridEvaluatorAIv2>(weights, depth, topN, true, false, verbose);
        default:
            return std::make_unique<SmartRandomAI>(2, verbose);  // Default to Smart Random
    }
}

// Function to get AI type name
const char* getAITypeName(AIType type) {
    switch (type) {
        case AIType::SMART_RANDOM:
            return "Smart Random";
        case AIType::HYBRID_EVALUATOR:
            return "Hybrid Evaluator";
        case AIType::HYBRID_EVALUATOR_V2:
            return "Hybrid Evaluator v2 (Minimax)";
        default:
            return "Unknown";
    }
}

// Function to get weight filename for AI type
const char* getWeightFilename(AIType type) {
    switch (type) {
        case AIType::HYBRID_EVALUATOR:
            return "hybrid_evaluator_weights.txt";
        case AIType::HYBRID_EVALUATOR_V2:
            return "hybrid_evaluator_v2_weights.txt";
        default:
            return nullptr;  // AI type doesn't support weights
    }
}

// Load trained weights for AI type (returns nullptr if not available)
std::unique_ptr<EvaluationWeights> loadWeightsForAI(AIType type) {
    const char* filename = getWeightFilename(type);
    if (!filename) {
        return nullptr;  // AI type doesn't support weights
    }

    auto weights = std::make_unique<EvaluationWeights>();
    if (weights->loadFromFile(filename)) {
        std::cout << "Loaded trained weights for " << getAITypeName(type)
                  << " from " << filename << "\n";
        return weights;
    }

    return nullptr;  // No trained weights available
}

// Run a single game and return winner ('X', 'O', or 'D' for draw)
char runSingleGame(AIType ai1Type, AIType ai2Type, bool quiet = false, bool verbose = false) {
    TicTacToeBoard game;
    int x, y;
    const int winningLength = 5;
    const int maxMoves = 1000;  // Prevent infinite games
    int moveCount = 0;

    auto ai1 = createAI(ai1Type, nullptr, verbose);
    auto ai2 = createAI(ai2Type, nullptr, verbose);

    const char player1Mark = 'X';
    const char player2Mark = 'O';
    bool isPlayer1Turn = true;
    std::pair<int, int> lastMove = {INT_MIN, INT_MIN};  // Track opponent's last move

    while (moveCount < maxMoves) {
        if (!quiet) {
            game.printBoard();
        }

        char currentMark = isPlayer1Turn ? player1Mark : player2Mark;
        AIPlayer* currentAI = isPlayer1Turn ? ai1.get() : ai2.get();

        auto [moveX, moveY] = currentAI->findBestMove(game, currentMark, lastMove);

        if (!game.placeMark(moveX, moveY)) {
            if (!quiet) {
                std::cout << "No valid moves available.\n";
            }
            break;
        }

        if (!quiet) {
            std::cout << "Player " << currentMark << " played at (" << moveX << ", " << moveY << ")\n";
        }

        lastMove = {moveX, moveY};  // Update last move for next player

        if (game.checkWin(moveX, moveY, winningLength)) {
            if (!quiet) {
                game.printBoard();
            }
            return currentMark;
        }

        isPlayer1Turn = !isPlayer1Turn;
        moveCount++;
    }

    return 'D';  // Draw
}

// Get AI type from user selection
AIType selectAIType(const std::string& playerName) {
    std::cout << "\nSelect " << playerName << " AI type:\n";
    std::cout << "1. Smart Random (Random + Win Detection)\n";
    std::cout << "2. Hybrid Evaluator (Tactical + Strategic)\n";
    std::cout << "3. Hybrid Evaluator v2 (Minimax-enhanced)\n";
    std::cout << "Enter choice (1-3): ";
    int choice;
    std::cin >> choice;

    switch (choice) {
        case 1:
            return AIType::SMART_RANDOM;
        case 2:
            return AIType::HYBRID_EVALUATOR;
        case 3:
            return AIType::HYBRID_EVALUATOR_V2;
        default:
            std::cout << "Invalid choice. Defaulting to Smart Random.\n";
            return AIType::SMART_RANDOM;
    }
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
bool checkWinSilent(TicTacToeBoard& game, int x, int y, int winningLength) {
    // Save cout state and redirect to null
    std::streambuf* oldCoutBuf = std::cout.rdbuf();
    std::cout.rdbuf(nullptr);

    bool result = game.checkWin(x, y, winningLength);

    // Restore cout
    std::cout.rdbuf(oldCoutBuf);

    return result;
}

// Run a single game and return winner + move count
std::pair<char, int> runSingleGameWithStats(AIType ai1Type, AIType ai2Type, bool verbose = false,
                                             const EvaluationWeights* ai1Weights = nullptr,
                                             const EvaluationWeights* ai2Weights = nullptr) {
    TicTacToeBoard game;
    const int winningLength = 5;
    const int maxMoves = 1000;
    int moveCount = 0;

    auto ai1 = createAI(ai1Type, ai1Weights, verbose);
    auto ai2 = createAI(ai2Type, ai2Weights, verbose);

    const char player1Mark = 'X';
    const char player2Mark = 'O';
    bool isPlayer1Turn = true;
    std::pair<int, int> lastMove = {INT_MIN, INT_MIN};  // Track opponent's last move

    while (moveCount < maxMoves) {
        char currentMark = isPlayer1Turn ? player1Mark : player2Mark;
        AIPlayer* currentAI = isPlayer1Turn ? ai1.get() : ai2.get();

        auto [moveX, moveY] = currentAI->findBestMove(game, currentMark, lastMove);

        if (!game.placeMark(moveX, moveY)) {
            break;
        }

        lastMove = {moveX, moveY};  // Update last move for next player
        moveCount++;

        if (checkWinSilent(game, moveX, moveY, winningLength)) {
            return {currentMark, moveCount};
        }

        isPlayer1Turn = !isPlayer1Turn;
    }

    return {'D', moveCount};
}

// Run benchmark comparing AI types
void runBenchmark(int numGames, bool interactive, bool verbose = false, bool useTrainedWeights = false) {
    std::cout << "\n=== AI Benchmark Mode ===\n";

    // Load weights for all weight-aware AIs if requested
    std::map<AIType, std::unique_ptr<EvaluationWeights>> aiWeights;
    if (useTrainedWeights) {
        std::cout << "Loading trained weights...\n";
        aiWeights[AIType::HYBRID_EVALUATOR] = loadWeightsForAI(AIType::HYBRID_EVALUATOR);
        aiWeights[AIType::HYBRID_EVALUATOR_V2] = loadWeightsForAI(AIType::HYBRID_EVALUATOR_V2);
        std::cout << "\n";
    }

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
        AIType aiTypes[] = {AIType::SMART_RANDOM, AIType::HYBRID_EVALUATOR, AIType::HYBRID_EVALUATOR_V2};
        int numTypes = 3;

        std::cout << "\nRunning comprehensive benchmark (all AI matchups)...\n";
        std::cout << "Running " << numGames << " games for each matchup...\n\n";

        for (int i = 0; i < numTypes; i++) {
            for (int j = 0; j < numTypes; j++) {
                BenchmarkStats stats;

                std::cout << getAITypeName(aiTypes[i]) << " (X) vs "
                          << getAITypeName(aiTypes[j]) << " (O): ";
                std::cout.flush();

                for (int game = 0; game < numGames; game++) {
                    auto [result, moves] = runSingleGameWithStats(aiTypes[i], aiTypes[j], verbose,
                                                                   aiWeights[aiTypes[i]].get(),
                                                                   aiWeights[aiTypes[j]].get());

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
        auto [result, moves] = runSingleGameWithStats(ai1Type, ai2Type, verbose,
                                                       aiWeights[ai1Type].get(),
                                                       aiWeights[ai2Type].get());

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
void runInteractiveGame(bool verbose = false, bool useTrainedWeights = false) {
    TicTacToeBoard game;
    int x, y;
    const int winningLength = 5;
    bool gameWon = false;

    std::cout << "Welcome to Infinite Tic-Tac-Toe!\n";

    // Configure Player 1 (X)
    std::cout << "\nPlayer 1 (X) type:\n";
    std::cout << "1. Human\n";
    std::cout << "2. Smart Random AI\n";
    std::cout << "3. Hybrid Evaluator AI\n";
    std::cout << "4. Hybrid Evaluator v2 AI (Minimax)\n";
    std::cout << "Enter choice (1-4): ";
    int p1Choice;
    std::cin >> p1Choice;

    PlayerType player1Type;
    AIType ai1Type = AIType::SMART_RANDOM;  // Default

    switch (p1Choice) {
        case 1:
            player1Type = PlayerType::HUMAN;
            break;
        case 2:
            player1Type = PlayerType::AI;
            ai1Type = AIType::SMART_RANDOM;
            break;
        case 3:
            player1Type = PlayerType::AI;
            ai1Type = AIType::HYBRID_EVALUATOR;
            break;
        case 4:
            player1Type = PlayerType::AI;
            ai1Type = AIType::HYBRID_EVALUATOR_V2;
            break;
        default:
            std::cout << "Invalid choice. Defaulting to Human.\n";
            player1Type = PlayerType::HUMAN;
    }

    // Configure Player 2 (O)
    std::cout << "\nPlayer 2 (O) type:\n";
    std::cout << "1. Human\n";
    std::cout << "2. Smart Random AI\n";
    std::cout << "3. Hybrid Evaluator AI\n";
    std::cout << "4. Hybrid Evaluator v2 AI (Minimax)\n";
    std::cout << "Enter choice (1-4): ";
    int p2Choice;
    std::cin >> p2Choice;

    PlayerType player2Type;
    AIType ai2Type = AIType::SMART_RANDOM;  // Default

    switch (p2Choice) {
        case 1:
            player2Type = PlayerType::HUMAN;
            break;
        case 2:
            player2Type = PlayerType::AI;
            ai2Type = AIType::SMART_RANDOM;
            break;
        case 3:
            player2Type = PlayerType::AI;
            ai2Type = AIType::HYBRID_EVALUATOR;
            break;
        case 4:
            player2Type = PlayerType::AI;
            ai2Type = AIType::HYBRID_EVALUATOR_V2;
            break;
        default:
            std::cout << "Invalid choice. Defaulting to Human.\n";
            player2Type = PlayerType::HUMAN;
    }

    // Load weights if requested
    std::unique_ptr<EvaluationWeights> ai1Weights = nullptr;
    std::unique_ptr<EvaluationWeights> ai2Weights = nullptr;

    if (useTrainedWeights) {
        if (player1Type == PlayerType::AI) {
            ai1Weights = loadWeightsForAI(ai1Type);
        }
        if (player2Type == PlayerType::AI) {
            ai2Weights = loadWeightsForAI(ai2Type);
        }
    }

    // Create AI instances if needed
    auto ai1 = (player1Type == PlayerType::AI) ? createAI(ai1Type, ai1Weights.get(), verbose) : nullptr;
    auto ai2 = (player2Type == PlayerType::AI) ? createAI(ai2Type, ai2Weights.get(), verbose) : nullptr;

    const char player1Mark = 'X';
    const char player2Mark = 'O';
    bool isPlayer1Turn = true;
    std::pair<int, int> lastMove = {INT_MIN, INT_MIN};  // Track opponent's last move

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

    // Automatically make first move at (0,0) if human is playing first
    if (player1Type == PlayerType::HUMAN) {
        game.placeMark(0, 0);
        lastMove = {0, 0};
        isPlayer1Turn = !isPlayer1Turn;  // Switch to player 2
        std::cout << "First move automatically placed at (0, 0)\n\n";
    }

    while (!gameWon) {
        game.printBoard();

        PlayerType currentPlayerType = isPlayer1Turn ? player1Type : player2Type;
        char currentMark = isPlayer1Turn ? player1Mark : player2Mark;
        AIPlayer* currentAI = isPlayer1Turn ? ai1.get() : ai2.get();

        if (currentPlayerType == PlayerType::HUMAN) {
            // Human player's turn
            std::cout << "Player " << currentMark << " (Human), enter your move (x y): ";
            std::cin >> x >> y;

            // Attempt to place the mark; if invalid, prompt again
            if (game.placeMark(x, y)) {
                lastMove = {x, y};  // Update last move for AI
                gameWon = game.checkWin(x, y, winningLength);
                isPlayer1Turn = !isPlayer1Turn;  // Switch players
            } else {
                std::cout << "Invalid move. Try again.\n";
            }
        } else {
            // AI player's turn
            AIType currentAIType = isPlayer1Turn ? ai1Type : ai2Type;
            std::cout << "Player " << currentMark << " (" << getAITypeName(currentAIType) << ") is making a move...\n";
            auto [moveX, moveY] = currentAI->findBestMove(game, currentMark, lastMove);

            if (game.placeMark(moveX, moveY)) {
                std::cout << "AI played at (" << moveX << ", " << moveY << ")\n";
                lastMove = {moveX, moveY};  // Update last move for next player
                gameWon = game.checkWin(moveX, moveY, winningLength);
                isPlayer1Turn = !isPlayer1Turn;  // Switch players
            } else {
                std::cout << "AI has no available moves.\n";
                break;
            }
        }
    }

    game.printBoard();
    std::cout << "Game over! Thanks for playing.\n";
}

// Run weight training mode
void runTraining(int generations, int populationSize, int gamesPerMatchup) {
    std::cout << "=== AI Weight Training Mode ===\n";
    std::cout << "Training: Hybrid Evaluator\n\n";
    std::cout << "Configuration:\n";
    std::cout << "  Generations: " << generations << "\n";
    std::cout << "  Population size: " << populationSize << "\n";
    std::cout << "  Games per matchup: " << gamesPerMatchup << "\n";
    std::cout << "  Estimated total games: " << (populationSize * (populationSize - 1) / 2 * gamesPerMatchup * generations) << "\n\n";

    AIType aiType = AIType::HYBRID_EVALUATOR;
    const char* weightFilename = getWeightFilename(aiType);

    // Try to load existing weights as starting point
    EvaluationWeights startingWeights;
    if (startingWeights.loadFromFile(weightFilename)) {
        std::cout << "Loaded existing weights from " << weightFilename << "\n";
        startingWeights.print();
    } else {
        std::cout << "Using default weights as starting point\n";
        startingWeights.print();
    }
    std::cout << "\n";

    // Create trainer and run evolution
    WeightTrainer trainer(aiType, populationSize, gamesPerMatchup, 100, 0.15);
    EvaluationWeights bestWeights = trainer.train(generations, startingWeights);

    // Save best weights
    if (bestWeights.saveToFile(weightFilename)) {
        std::cout << "\nBest weights saved to " << weightFilename << "\n";
    } else {
        std::cout << "\nError: Could not save weights to " << weightFilename << "\n";
    }

    std::cout << "\nUse with: ./InfiniTTT --use-trained-weights\n";
}

int main(int argc, char* argv[]) {
    // Scan for --verbose flag
    bool verboseAI = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--verbose") {
            verboseAI = true;
            break;
        }
    }

    // Scan for --use-trained-weights flag
    bool useTrainedWeights = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--use-trained-weights") {
            useTrainedWeights = true;
            break;
        }
    }

    // Check for help
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        std::cout << "Infinite Tic-Tac-Toe - Usage:\n\n";
        std::cout << "Interactive mode (default):\n";
        std::cout << "  ./InfiniTTT\n\n";
        std::cout << "Training mode:\n";
        std::cout << "  ./InfiniTTT --train [generations] [population] [games_per_matchup]\n";
        std::cout << "  Trains the Hybrid Evaluator AI using genetic algorithms\n";
        std::cout << "  Arguments:\n";
        std::cout << "    generations        - Number of evolution cycles (default: 10)\n";
        std::cout << "    population         - Number of weight candidates (default: 20)\n";
        std::cout << "    games_per_matchup  - Games each pair plays (default: 6)\n";
        std::cout << "  Example:\n";
        std::cout << "    ./InfiniTTT --train 10 20 6\n";
        std::cout << "  Note: More games per matchup = more stable results but slower\n\n";
        std::cout << "Using trained weights:\n";
        std::cout << "  ./InfiniTTT --use-trained-weights\n";
        std::cout << "  ./InfiniTTT --benchmark --use-trained-weights --all 50\n";
        std::cout << "  Note: Automatically loads trained weights for supported AIs\n\n";
        std::cout << "Benchmark mode:\n";
        std::cout << "  ./InfiniTTT --benchmark [num_games]\n";
        std::cout << "  ./InfiniTTT --benchmark --all [num_games]\n\n";
        std::cout << "Verbose mode (show AI move scores):\n";
        std::cout << "  ./InfiniTTT --verbose\n";
        std::cout << "  ./InfiniTTT --benchmark --verbose 50\n\n";
        return 0;
    }

    // Check for training mode
    if (argc > 1 && std::string(argv[1]) == "--train") {
        int generations = 10;  // Default
        int populationSize = 20;  // Default
        int gamesPerMatchup = 6;  // Default - higher than 2 to reduce random effects

        // Parse numeric arguments starting at argv[2]
        if (argc > 2) generations = std::atoi(argv[2]);
        if (argc > 3) populationSize = std::atoi(argv[3]);
        if (argc > 4) gamesPerMatchup = std::atoi(argv[4]);

        // Validate parameters
        if (generations < 1 || populationSize < 2 || gamesPerMatchup < 1) {
            std::cerr << "Error: Invalid parameters. Use --help for usage.\n";
            return 1;
        }

        runTraining(generations, populationSize, gamesPerMatchup);
        return 0;
    }

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

        runBenchmark(numGames, interactive, verboseAI, useTrainedWeights);
    } else {
        runInteractiveGame(verboseAI, useTrainedWeights);
    }

    return 0;
}