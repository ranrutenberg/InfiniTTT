// Hybrid Evaluator AI v2 - Minimax-enhanced version with incremental evaluation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "aiplayer.h"
#include <set>
#include <vector>

class EvaluationWeights;
class TicTacToeBoard;

// Move with its evaluated score
struct MoveScore {
    std::pair<int, int> move;
    int score;
    int ourScore;   // For debugging
    int oppScore;   // For debugging
};

// Hybrid Evaluator AI v2 - Extends v1 with minimax search and incremental evaluation
// Features:
// - Incremental position evaluation (only affected 11x11 area around moves)
// - In-place minimax with undo (no board copies during search)
// - Top-N move pruning for opponent simulation
// - Configurable search depth (default: 2 = our move + opponent response)
//
// Priority system (same as v1):
// 1. Take winning moves
// 2. Block opponent winning moves
// 3. Use minimax to evaluate remaining moves
class HybridEvaluatorAIv2 : public AIPlayer {
private:
    std::set<std::pair<int, int>> availableMoves;  // Maintained internally by AI
    const EvaluationWeights* weights;  // Optional custom weights
    int searchDepth;     // Minimax search depth
    int topN;            // Number of top moves to consider at each depth
    bool useAlphaBeta;   // Enable alpha-beta pruning
    bool debugMode;      // Verify incremental vs full evaluation

    static constexpr int WIN_SCORE = 1000000;  // Score for winning position

    // Helper to check if a move results in a win
    bool isWinningMove(TicTacToeBoard& board, int x, int y, char playerMark, int winLength = 5) const;

    // Full board evaluation (for initialization and debugging)
    int evaluatePositionFull(const TicTacToeBoard& board, char mark) const;

    // Incremental evaluation - returns score for the position considering only
    // windows that include the move position (x, y)
    // This evaluates the score contribution of windows in the 11x11 area around the move
    int evaluatePositionIncremental(const TicTacToeBoard& board, int moveX, int moveY, char evalMark) const;

    // Calculate score delta caused by placing a move
    // Returns: (newScore - oldScore) for the evaluating player
    int calculateScoreDelta(TicTacToeBoard& board, int moveX, int moveY,
                            char moveMark, char evalMark) const;

    // Debug: Compare incremental delta vs full evaluation delta
    bool verifyIncrementalEvaluation(TicTacToeBoard& board, int moveX, int moveY,
                                      char moveMark, char evalMark, int incrementalDelta) const;

    // Minimax with alpha-beta pruning (in-place with undo)
    int minimax(TicTacToeBoard& board, int depth, int alpha, int beta,
                bool isMaximizing, char ourMark, char oppMark,
                std::set<std::pair<int, int>>& currentMoves,
                int currentOurScore, int currentOppScore);

    // Get top N moves sorted by heuristic score
    std::vector<MoveScore> getTopNMoves(TicTacToeBoard& board,
                                         const std::set<std::pair<int, int>>& moves,
                                         char playerMark, int n) const;

    // Available moves management during search
    // Returns vector of positions that were added (for undoing)
    std::vector<std::pair<int, int>> addAdjacentMoves(
        std::set<std::pair<int, int>>& moves,
        const TicTacToeBoard& board, int x, int y) const;

public:
    // Constructor with all configurable parameters
    // weights: Optional evaluation weights (nullptr uses defaults)
    // depth: Search depth (1 = just our move, 2 = our move + opponent response)
    // topN: Number of top moves to consider at each depth level
    // useAlphaBeta: Enable alpha-beta pruning (recommended)
    // debugMode: Enable verification of incremental vs full evaluation
    // verbose: Enable verbose output
    HybridEvaluatorAIv2(const EvaluationWeights* w = nullptr,
                        int depth = 2,
                        int topN = 10,
                        bool useAlphaBeta = true,
                        bool debugMode = false,
                        bool verbose = false)
        : AIPlayer(verbose), weights(w), searchDepth(depth), topN(topN),
          useAlphaBeta(useAlphaBeta), debugMode(debugMode) {}

    std::pair<int, int> findBestMove(const TicTacToeBoard& board, char playerMark,
                                      std::pair<int, int> lastMove = {INT_MIN, INT_MIN}) override;

    // Configuration setters
    void setDepth(int depth) { searchDepth = depth; }
    void setTopN(int n) { topN = n; }
    void setDebugMode(bool debug) { debugMode = debug; }
};
