// Hybrid Evaluator AI v2 - Minimax-enhanced version with incremental evaluation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "hybrid_evaluator_ai_v2.h"
#include "ai_utils.h"
#include "tictactoeboard.h"
#include "evaluationweights.h"
#include <iostream>
#include <random>
#include <algorithm>
#include <limits>

// Helper to check if a move results in a win (modifies board temporarily)
bool HybridEvaluatorAIv2::isWinningMove(TicTacToeBoard& board, int x, int y, char playerMark, int winLength) const {
    board.placeMarkDirect(x, y, playerMark);
    bool wins = board.checkWinQuiet(x, y, winLength);
    board.removeMarkDirect(x, y);
    return wins;
}

// Full board evaluation - same as v1 (for initialization and debugging)
int HybridEvaluatorAIv2::evaluatePositionFull(const TicTacToeBoard& board, char mark) const {
    EvaluationWeights defaultWeights;
    const EvaluationWeights& w = weights ? *weights : defaultWeights;

    int score = 0;
    char opponent = (mark == 'X') ? 'O' : 'X';
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> countedWindows;
    std::set<std::pair<int, int>> winningMoves;

    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    auto occupiedPositions = board.getOccupiedPositions();

    for (const auto& [pos, m] : occupiedPositions) {
        if (m != mark) continue;

        int x = pos.first, y = pos.second;

        for (int d = 0; d < 4; ++d) {
            int dx = directions[d][0];
            int dy = directions[d][1];

            for (int offset = 0; offset < 5; ++offset) {
                int startX = x - offset * dx;
                int startY = y - offset * dy;
                int endX = startX + 4 * dx;
                int endY = startY + 4 * dy;

                auto p1 = std::make_pair(startX, startY);
                auto p2 = std::make_pair(endX, endY);
                std::pair<std::pair<int, int>, std::pair<int, int>> windowKey =
                    (p1 < p2) ? std::make_pair(p1, p2) : std::make_pair(p2, p1);

                if (countedWindows.count(windowKey) > 0) continue;
                countedWindows.insert(windowKey);

                int friendlyCount = 0;
                int opponentCount = 0;
                int emptyCount = 0;

                for (int k = 0; k < 5; ++k) {
                    int cellX = startX + k * dx;
                    int cellY = startY + k * dy;

                    if (!board.isPositionOccupied(cellX, cellY)) {
                        emptyCount++;
                    } else if (occupiedPositions.at({cellX, cellY}) == mark) {
                        friendlyCount++;
                    } else {
                        opponentCount++;
                    }
                }

                if (opponentCount > 0) continue;
                if (friendlyCount < 2) continue;

                int beforeX = startX - dx;
                int beforeY = startY - dy;
                int afterX = endX + dx;
                int afterY = endY + dy;

                bool openBefore = !board.isPositionOccupied(beforeX, beforeY) ||
                                  occupiedPositions.at({beforeX, beforeY}) != opponent;
                bool openAfter = !board.isPositionOccupied(afterX, afterY) ||
                                 occupiedPositions.at({afterX, afterY}) != opponent;

                int windowScore = 0;

                if (friendlyCount == 4) {
                    for (int k = 0; k < 5; ++k) {
                        int cellX = startX + k * dx;
                        int cellY = startY + k * dy;
                        if (!board.isPositionOccupied(cellX, cellY)) {
                            winningMoves.insert({cellX, cellY});
                        }
                    }
                    windowScore = (openBefore && openAfter) ? w.four_open : w.four_blocked;
                } else if (friendlyCount == 3) {
                    if (emptyCount == 2) {
                        windowScore = (openBefore && openAfter) ? w.three_open : w.three_blocked;
                    }
                } else if (friendlyCount == 2) {
                    if (emptyCount == 3 && openBefore && openAfter) {
                        windowScore = w.two_open;
                    }
                }

                score += windowScore;
            }
        }
    }

    if (winningMoves.size() >= 2) {
        score += w.double_threat;
    }

    return score;
}

// Incremental evaluation - only evaluates windows containing the move position
// This is more efficient than full evaluation when we only need to know
// the score contribution of windows in the area around a move
int HybridEvaluatorAIv2::evaluatePositionIncremental(const TicTacToeBoard& board,
                                                       int moveX, int moveY, char evalMark) const {
    EvaluationWeights defaultWeights;
    const EvaluationWeights& w = weights ? *weights : defaultWeights;

    int score = 0;
    char opponent = (evalMark == 'X') ? 'O' : 'X';
    std::set<std::pair<std::pair<int, int>, std::pair<int, int>>> countedWindows;
    std::set<std::pair<int, int>> winningMoves;

    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    auto& occupiedPositions = board.getOccupiedPositions();

    // Only examine windows that include the move position (moveX, moveY)
    // These are windows where the move is at index 0, 1, 2, 3, or 4
    for (int d = 0; d < 4; ++d) {
        int dx = directions[d][0];
        int dy = directions[d][1];

        // For each possible position of moveX,moveY within a 5-cell window
        for (int offset = 0; offset < 5; ++offset) {
            // Calculate window start so that moveX,moveY is at position 'offset'
            int startX = moveX - offset * dx;
            int startY = moveY - offset * dy;
            int endX = startX + 4 * dx;
            int endY = startY + 4 * dy;

            // Create canonical key
            auto p1 = std::make_pair(startX, startY);
            auto p2 = std::make_pair(endX, endY);
            std::pair<std::pair<int, int>, std::pair<int, int>> windowKey =
                (p1 < p2) ? std::make_pair(p1, p2) : std::make_pair(p2, p1);

            if (countedWindows.count(windowKey) > 0) continue;
            countedWindows.insert(windowKey);

            // Analyze this 5-cell window
            int friendlyCount = 0;
            int opponentCount = 0;
            int emptyCount = 0;

            for (int k = 0; k < 5; ++k) {
                int cellX = startX + k * dx;
                int cellY = startY + k * dy;

                if (!board.isPositionOccupied(cellX, cellY)) {
                    emptyCount++;
                } else {
                    char cellMark = occupiedPositions.at({cellX, cellY});
                    if (cellMark == evalMark) {
                        friendlyCount++;
                    } else {
                        opponentCount++;
                    }
                }
            }

            // If opponent has any pieces in this window, it's blocked for evalMark
            if (opponentCount > 0) continue;
            // Need at least 2 friendly pieces for the window to count
            if (friendlyCount < 2) continue;

            // Check if ends are open
            int beforeX = startX - dx;
            int beforeY = startY - dy;
            int afterX = endX + dx;
            int afterY = endY + dy;

            bool openBefore = !board.isPositionOccupied(beforeX, beforeY) ||
                              occupiedPositions.at({beforeX, beforeY}) != opponent;
            bool openAfter = !board.isPositionOccupied(afterX, afterY) ||
                             occupiedPositions.at({afterX, afterY}) != opponent;

            int windowScore = 0;

            if (friendlyCount == 4) {
                // Track winning positions for double threat detection
                for (int k = 0; k < 5; ++k) {
                    int cellX = startX + k * dx;
                    int cellY = startY + k * dy;
                    if (!board.isPositionOccupied(cellX, cellY)) {
                        winningMoves.insert({cellX, cellY});
                    }
                }
                windowScore = (openBefore && openAfter) ? w.four_open : w.four_blocked;
            } else if (friendlyCount == 3) {
                if (emptyCount == 2) {
                    windowScore = (openBefore && openAfter) ? w.three_open : w.three_blocked;
                }
            } else if (friendlyCount == 2) {
                if (emptyCount == 3 && openBefore && openAfter) {
                    windowScore = w.two_open;
                }
            }

            score += windowScore;
        }
    }

    // Note: Double threat bonus is NOT added here in incremental mode
    // because we can't reliably detect full-board double threats
    // from just the local windows. The full evaluation handles this.

    return score;
}

// Calculate score delta caused by placing a move
int HybridEvaluatorAIv2::calculateScoreDelta(TicTacToeBoard& board, int moveX, int moveY,
                                               char moveMark, char evalMark) const {
    // Get score BEFORE the move (in the affected area)
    int scoreBefore = evaluatePositionIncremental(board, moveX, moveY, evalMark);

    // Place the move
    board.placeMarkDirect(moveX, moveY, moveMark);

    // Get score AFTER the move (in the affected area)
    int scoreAfter = evaluatePositionIncremental(board, moveX, moveY, evalMark);

    // Undo the move
    board.removeMarkDirect(moveX, moveY);

    return scoreAfter - scoreBefore;
}

// Debug verification: compare incremental delta with full evaluation delta
bool HybridEvaluatorAIv2::verifyIncrementalEvaluation(TicTacToeBoard& board, int moveX, int moveY,
                                                        char moveMark, char evalMark,
                                                        int incrementalDelta) const {
    // Get full score BEFORE
    int fullBefore = evaluatePositionFull(board, evalMark);

    // Place move
    board.placeMarkDirect(moveX, moveY, moveMark);

    // Get full score AFTER
    int fullAfter = evaluatePositionFull(board, evalMark);

    // Undo
    board.removeMarkDirect(moveX, moveY);

    int fullDelta = fullAfter - fullBefore;

    if (fullDelta != incrementalDelta) {
        std::cerr << "[DEBUG] Evaluation mismatch at (" << moveX << "," << moveY << ") "
                  << "mark=" << moveMark << " eval=" << evalMark << ": "
                  << "incremental=" << incrementalDelta << " full=" << fullDelta << "\n";
        return false;
    }
    return true;
}

// Add adjacent positions to available moves, return list of what was added
std::vector<std::pair<int, int>> HybridEvaluatorAIv2::addAdjacentMoves(
    std::set<std::pair<int, int>>& moves,
    const TicTacToeBoard& board, int x, int y) const {

    std::vector<std::pair<int, int>> added;
    for (int i = x - 1; i <= x + 1; ++i) {
        for (int j = y - 1; j <= y + 1; ++j) {
            if (i == x && j == y) continue;
            if (!board.isPositionOccupied(i, j) && moves.find({i, j}) == moves.end()) {
                moves.insert({i, j});
                added.push_back({i, j});
            }
        }
    }
    return added;
}

// Get top N moves sorted by heuristic score
std::vector<MoveScore> HybridEvaluatorAIv2::getTopNMoves(TicTacToeBoard& board,
                                                          const std::set<std::pair<int, int>>& moves,
                                                          char playerMark, int n) const {
    std::vector<MoveScore> scores;
    char opponent = (playerMark == 'X') ? 'O' : 'X';

    for (const auto& move : moves) {
        int x = move.first;
        int y = move.second;

        // Calculate score delta for this move
        int ourDelta = calculateScoreDelta(board, x, y, playerMark, playerMark);
        int oppDelta = calculateScoreDelta(board, x, y, playerMark, opponent);
        int netScore = ourDelta - oppDelta;

        // Check for immediate win (huge bonus)
        board.placeMarkDirect(x, y, playerMark);
        if (board.checkWinQuiet(x, y, 5)) {
            netScore += WIN_SCORE;
        }
        board.removeMarkDirect(x, y);

        scores.push_back({move, netScore, ourDelta, oppDelta});
    }

    // Sort by score descending
    std::sort(scores.begin(), scores.end(),
              [](const MoveScore& a, const MoveScore& b) { return a.score > b.score; });

    // Return top N
    if (static_cast<int>(scores.size()) > n) {
        scores.resize(n);
    }

    return scores;
}

// Minimax with alpha-beta pruning
int HybridEvaluatorAIv2::minimax(TicTacToeBoard& board, int depth, int alpha, int beta,
                                   bool isMaximizing, char ourMark, char oppMark,
                                   std::set<std::pair<int, int>>& currentMoves,
                                   int currentOurScore, int currentOppScore) {
    // Terminal: depth reached
    if (depth == 0) {
        return currentOurScore - currentOppScore;
    }

    // No moves available
    if (currentMoves.empty()) {
        return currentOurScore - currentOppScore;
    }

    char currentMark = isMaximizing ? ourMark : oppMark;

    // Get top N moves for this depth
    std::vector<MoveScore> topMoves = getTopNMoves(board, currentMoves, currentMark, topN);

    if (isMaximizing) {
        int bestValue = std::numeric_limits<int>::min();

        for (const auto& ms : topMoves) {
            int x = ms.move.first;
            int y = ms.move.second;

            // Make move
            board.placeMarkDirect(x, y, currentMark);

            // Check for win
            if (board.checkWinQuiet(x, y, 5)) {
                board.removeMarkDirect(x, y);
                return WIN_SCORE;  // We win!
            }

            // Update available moves
            currentMoves.erase(ms.move);
            auto addedMoves = addAdjacentMoves(currentMoves, board, x, y);

            // Calculate score deltas
            int ourDelta = calculateScoreDelta(board, x, y, currentMark, ourMark);
            int oppDelta = calculateScoreDelta(board, x, y, currentMark, oppMark);

            // Note: The move is already placed, so deltas are relative to state before this call
            // We need to re-calculate since calculateScoreDelta places/removes internally
            // Actually, the move IS placed now, so we evaluate the position as-is
            int newOurScore = evaluatePositionIncremental(board, x, y, ourMark);
            int newOppScore = evaluatePositionIncremental(board, x, y, oppMark);

            // Debug verification
            if (debugMode) {
                // Verification would need adjustment for already-placed moves
            }

            // Recurse
            int value = minimax(board, depth - 1, alpha, beta, false, ourMark, oppMark,
                               currentMoves, currentOurScore + ourDelta, currentOppScore + oppDelta);

            // Undo move
            board.removeMarkDirect(x, y);
            for (const auto& added : addedMoves) {
                currentMoves.erase(added);
            }
            currentMoves.insert(ms.move);

            bestValue = std::max(bestValue, value);

            if (useAlphaBeta) {
                alpha = std::max(alpha, value);
                if (beta <= alpha) {
                    break;  // Beta cutoff
                }
            }
        }

        return bestValue;
    } else {
        // Minimizing (opponent's turn)
        int bestValue = std::numeric_limits<int>::max();

        for (const auto& ms : topMoves) {
            int x = ms.move.first;
            int y = ms.move.second;

            // Make move
            board.placeMarkDirect(x, y, currentMark);

            // Check for opponent win
            if (board.checkWinQuiet(x, y, 5)) {
                board.removeMarkDirect(x, y);
                return -WIN_SCORE;  // Opponent wins
            }

            // Update available moves
            currentMoves.erase(ms.move);
            auto addedMoves = addAdjacentMoves(currentMoves, board, x, y);

            // Calculate score deltas
            int ourDelta = calculateScoreDelta(board, x, y, currentMark, ourMark);
            int oppDelta = calculateScoreDelta(board, x, y, currentMark, oppMark);

            // Recurse
            int value = minimax(board, depth - 1, alpha, beta, true, ourMark, oppMark,
                               currentMoves, currentOurScore + ourDelta, currentOppScore + oppDelta);

            // Undo move
            board.removeMarkDirect(x, y);
            for (const auto& added : addedMoves) {
                currentMoves.erase(added);
            }
            currentMoves.insert(ms.move);

            bestValue = std::min(bestValue, value);

            if (useAlphaBeta) {
                beta = std::min(beta, value);
                if (beta <= alpha) {
                    break;  // Alpha cutoff
                }
            }
        }

        return bestValue;
    }
}

// Main entry point: find best move
std::pair<int, int> HybridEvaluatorAIv2::findBestMove(const TicTacToeBoard& board, char playerMark,
                                                        std::pair<int, int> lastMove) {
    // If board is empty, start at origin
    if (board.getOccupiedPositions().empty()) {
        availableMoves.clear();
        availableMoves.insert({0, 0});
        return {0, 0};
    }

    // Update internal available moves based on lastMove
    if (lastMove.first != INT_MIN && lastMove.second != INT_MIN) {
        AIUtils::updateAvailableMoves(availableMoves, board, lastMove.first, lastMove.second);
    } else if (availableMoves.empty()) {
        auto tempMoves = AIUtils::computeAdjacentMoves(board);
        availableMoves.insert(tempMoves.begin(), tempMoves.end());
    }

    // Filter out occupied positions
    auto it = availableMoves.begin();
    while (it != availableMoves.end()) {
        if (board.isPositionOccupied(it->first, it->second)) {
            it = availableMoves.erase(it);
        } else {
            ++it;
        }
    }

    if (availableMoves.empty()) {
        return {0, 0};
    }

    if (verboseMode) {
        std::cout << "\n[HybridEvaluatorAIv2 - Player " << playerMark << "]\n";
        std::cout << "Depth: " << searchDepth << ", TopN: " << topN << "\n";
        std::cout << "Evaluating " << availableMoves.size() << " available moves\n";
    }

    // Create mutable copy for win/block checks
    TicTacToeBoard boardCopy = board;

    // PRIORITY 1: Check for winning moves
    std::vector<std::pair<int, int>> winningMoves;
    for (const auto& move : availableMoves) {
        if (isWinningMove(boardCopy, move.first, move.second, playerMark)) {
            winningMoves.push_back(move);
        }
    }

    if (!winningMoves.empty()) {
        if (verboseMode) {
            std::cout << "Priority 1: Winning moves - " << winningMoves.size() << " found\n";
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, winningMoves.size() - 1);
        auto winningMove = winningMoves[dis(gen)];

        if (verboseMode) {
            std::cout << "Selected winning move: (" << winningMove.first << ", " << winningMove.second << ")\n\n";
        }

        availableMoves.erase(winningMove);
        for (int i = winningMove.first - 1; i <= winningMove.first + 1; ++i) {
            for (int j = winningMove.second - 1; j <= winningMove.second + 1; ++j) {
                if (i == winningMove.first && j == winningMove.second) continue;
                availableMoves.insert({i, j});
            }
        }

        return winningMove;
    }

    if (verboseMode) {
        std::cout << "Priority 1: Winning moves - 0 found\n";
    }

    // PRIORITY 2: Block opponent winning moves
    char opponentMark = (playerMark == 'X') ? 'O' : 'X';
    std::vector<std::pair<int, int>> blockingMoves;
    for (const auto& move : availableMoves) {
        if (isWinningMove(boardCopy, move.first, move.second, opponentMark)) {
            blockingMoves.push_back(move);
        }
    }

    if (!blockingMoves.empty()) {
        if (verboseMode) {
            std::cout << "Priority 2: Blocking moves - " << blockingMoves.size() << " found\n";
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, blockingMoves.size() - 1);
        auto blockingMove = blockingMoves[dis(gen)];

        if (verboseMode) {
            std::cout << "Selected blocking move: (" << blockingMove.first << ", " << blockingMove.second << ")\n\n";
        }

        availableMoves.erase(blockingMove);
        for (int i = blockingMove.first - 1; i <= blockingMove.first + 1; ++i) {
            for (int j = blockingMove.second - 1; j <= blockingMove.second + 1; ++j) {
                if (i == blockingMove.first && j == blockingMove.second) continue;
                availableMoves.insert({i, j});
            }
        }

        return blockingMove;
    }

    if (verboseMode) {
        std::cout << "Priority 2: Blocking moves - 0 found\n";
        std::cout << "Priority 3: Minimax evaluation (depth=" << searchDepth << ")\n";
    }

    // PRIORITY 3: Use minimax to evaluate moves
    // Get initial scores
    int initialOurScore = evaluatePositionFull(board, playerMark);
    int initialOppScore = evaluatePositionFull(board, opponentMark);

    // Create a working copy of available moves for minimax
    std::set<std::pair<int, int>> searchMoves = availableMoves;

    // Evaluate all moves using minimax
    struct MinimaxResult {
        std::pair<int, int> move;
        int value;
    };
    std::vector<MinimaxResult> results;

    // Get top N moves to evaluate with minimax
    std::vector<MoveScore> topMoves = getTopNMoves(boardCopy, availableMoves, playerMark, topN);

    for (const auto& ms : topMoves) {
        int x = ms.move.first;
        int y = ms.move.second;

        // Make move
        boardCopy.placeMarkDirect(x, y, playerMark);

        // Update search moves
        searchMoves.erase(ms.move);
        auto addedMoves = addAdjacentMoves(searchMoves, boardCopy, x, y);

        // Calculate deltas for this move
        int ourDelta = ms.ourScore;  // Already calculated in getTopNMoves
        int oppDelta = ms.oppScore;

        // Run minimax from this position
        int value;
        if (searchDepth <= 1) {
            // Depth 1: just use the heuristic score
            value = ms.score;
        } else {
            // Depth > 1: run minimax for opponent's response
            value = minimax(boardCopy, searchDepth - 1,
                           std::numeric_limits<int>::min(),
                           std::numeric_limits<int>::max(),
                           false,  // Opponent's turn (minimizing)
                           playerMark, opponentMark,
                           searchMoves,
                           initialOurScore + ourDelta,
                           initialOppScore + oppDelta);
        }

        // Undo
        boardCopy.removeMarkDirect(x, y);
        for (const auto& added : addedMoves) {
            searchMoves.erase(added);
        }
        searchMoves.insert(ms.move);

        results.push_back({ms.move, value});

        if (verboseMode) {
            std::cout << "  Move (" << x << "," << y << "): minimax value = " << value << "\n";
        }
    }

    // Find best move(s)
    int bestValue = std::numeric_limits<int>::min();
    std::vector<std::pair<int, int>> bestMoves;

    for (const auto& result : results) {
        if (result.value > bestValue) {
            bestValue = result.value;
            bestMoves.clear();
            bestMoves.push_back(result.move);
        } else if (result.value == bestValue) {
            bestMoves.push_back(result.move);
        }
    }

    if (bestMoves.empty()) {
        // Fallback - shouldn't happen
        bestMoves.push_back(*availableMoves.begin());
    }

    // Random tie-breaking
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, bestMoves.size() - 1);
    auto chosenMove = bestMoves[dis(gen)];

    if (verboseMode) {
        std::cout << "Best value: " << bestValue << " (" << bestMoves.size() << " tied)\n";
        std::cout << "Selected: (" << chosenMove.first << ", " << chosenMove.second << ")\n\n";
    }

    // Update internal available moves
    availableMoves.erase(chosenMove);
    for (int i = chosenMove.first - 1; i <= chosenMove.first + 1; ++i) {
        for (int j = chosenMove.second - 1; j <= chosenMove.second + 1; ++j) {
            if (i == chosenMove.first && j == chosenMove.second) continue;
            availableMoves.insert({i, j});
        }
    }

    return chosenMove;
}
