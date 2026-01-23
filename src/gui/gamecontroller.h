// Game Controller - Connects Qt GUI to game logic
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <memory>
#include "tictactoeboard.h"
#include "ai_types.h"
#include "aiplayer.h"
#include "evaluationweights.h"

struct PlayerConfig {
    bool isHuman = true;
    AIType aiType = AIType::SMART_RANDOM;
};

struct GameConfig {
    PlayerConfig player1;
    PlayerConfig player2;
};

class GameController : public QObject {
    Q_OBJECT

public:
    explicit GameController(QObject* parent = nullptr);
    ~GameController() override;

    void startNewGame(const GameConfig& config);
    void handleCellClick(int x, int y);
    bool isGameActive() const { return gameActive_; }
    char getCurrentPlayer() const { return currentPlayer_; }
    const TicTacToeBoard& getBoard() const { return board_; }

    // Weight file configuration
    void setHybridEvaluatorWeightsPath(const QString& path);
    void setHybridEvaluatorV2WeightsPath(const QString& path);
    QString getHybridEvaluatorWeightsPath() const { return hybridWeightsPath_; }
    QString getHybridEvaluatorV2WeightsPath() const { return hybridV2WeightsPath_; }

signals:
    void moveExecuted(int x, int y, char mark);
    void turnChanged(char newPlayer);
    void gameOver(char winner);
    void aiThinking(bool thinking);
    void invalidMove(int x, int y, const QString& reason);

private slots:
    void executeAIMove();
    void executeFirstMove();

private:
    void checkGameState(int lastX, int lastY);
    void switchTurn();
    std::unique_ptr<AIPlayer> createAIPlayer(AIType type, const EvaluationWeights* weights);
    std::unique_ptr<EvaluationWeights> loadWeightsForAI(AIType type);
    QString getWeightFilename(AIType type) const;

    TicTacToeBoard board_;
    GameConfig config_;

    std::unique_ptr<AIPlayer> player1AI_;
    std::unique_ptr<AIPlayer> player2AI_;
    std::unique_ptr<EvaluationWeights> weights1_;
    std::unique_ptr<EvaluationWeights> weights2_;

    char currentPlayer_ = 'X';
    bool gameActive_ = false;
    std::pair<int, int> lastMove_ = {INT_MIN, INT_MIN};
    int moveCount_ = 0;
    static constexpr int WIN_LENGTH = 5;
    static constexpr int MAX_MOVES = 1000;

    // Configurable weight file paths
    QString hybridWeightsPath_ = "hybrid_evaluator_weights.txt";
    QString hybridV2WeightsPath_ = "hybrid_evaluator_v2_weights.txt";
};

#endif // GAMECONTROLLER_H
