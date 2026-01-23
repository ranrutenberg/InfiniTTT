// Game Controller - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "gamecontroller.h"
#include "smart_random_ai.h"
#include "hybrid_evaluator_ai.h"
#include "hybrid_evaluator_ai_v2.h"

GameController::GameController(QObject* parent)
    : QObject(parent) {
}

GameController::~GameController() = default;

void GameController::setHybridEvaluatorWeightsPath(const QString& path) {
    hybridWeightsPath_ = path;
}

void GameController::setHybridEvaluatorV2WeightsPath(const QString& path) {
    hybridV2WeightsPath_ = path;
}

void GameController::startNewGame(const GameConfig& config) {
    config_ = config;
    board_ = TicTacToeBoard();
    currentPlayer_ = 'X';
    gameActive_ = true;
    lastMove_ = {INT_MIN, INT_MIN};
    moveCount_ = 0;

    player1AI_.reset();
    player2AI_.reset();
    weights1_.reset();
    weights2_.reset();

    if (!config_.player1.isHuman) {
        weights1_ = loadWeightsForAI(config_.player1.aiType);
        player1AI_ = createAIPlayer(config_.player1.aiType, weights1_.get());
    }

    if (!config_.player2.isHuman) {
        weights2_ = loadWeightsForAI(config_.player2.aiType);
        player2AI_ = createAIPlayer(config_.player2.aiType, weights2_.get());
    }

    emit turnChanged(currentPlayer_);

    // Always place first move at (0, 0) automatically
    QTimer::singleShot(100, this, &GameController::executeFirstMove);
}

void GameController::executeFirstMove() {
    if (!gameActive_ || moveCount_ > 0) return;

    // Place first move at (0, 0)
    board_.placeMark(0, 0);
    emit moveExecuted(0, 0, currentPlayer_);
    lastMove_ = {0, 0};
    moveCount_++;

    checkGameState(0, 0);
}

void GameController::handleCellClick(int x, int y) {
    if (!gameActive_) return;

    bool isCurrentPlayerHuman = (currentPlayer_ == 'X')
        ? config_.player1.isHuman
        : config_.player2.isHuman;

    if (!isCurrentPlayerHuman) return;

    if (board_.isPositionOccupied(x, y)) {
        emit invalidMove(x, y, "Position already occupied");
        return;
    }

    board_.placeMark(x, y);
    emit moveExecuted(x, y, currentPlayer_);
    lastMove_ = {x, y};
    moveCount_++;

    checkGameState(x, y);
}

void GameController::executeAIMove() {
    if (!gameActive_) return;

    emit aiThinking(true);

    AIPlayer* currentAI = (currentPlayer_ == 'X') ? player1AI_.get() : player2AI_.get();
    if (!currentAI) {
        emit aiThinking(false);
        return;
    }

    auto [moveX, moveY] = currentAI->findBestMove(board_, currentPlayer_, lastMove_);

    board_.placeMark(moveX, moveY);
    emit moveExecuted(moveX, moveY, currentPlayer_);
    lastMove_ = {moveX, moveY};
    moveCount_++;

    emit aiThinking(false);
    checkGameState(moveX, moveY);
}

void GameController::checkGameState(int lastX, int lastY) {
    if (board_.checkWinQuiet(lastX, lastY, WIN_LENGTH)) {
        gameActive_ = false;
        emit gameOver(currentPlayer_);
        return;
    }

    if (moveCount_ >= MAX_MOVES) {
        gameActive_ = false;
        emit gameOver('D');
        return;
    }

    switchTurn();
}

void GameController::switchTurn() {
    currentPlayer_ = (currentPlayer_ == 'X') ? 'O' : 'X';
    emit turnChanged(currentPlayer_);

    bool isNextPlayerAI = (currentPlayer_ == 'X')
        ? !config_.player1.isHuman
        : !config_.player2.isHuman;

    if (isNextPlayerAI && gameActive_) {
        QTimer::singleShot(300, this, &GameController::executeAIMove);
    }
}

std::unique_ptr<AIPlayer> GameController::createAIPlayer(AIType type, const EvaluationWeights* weights) {
    switch (type) {
        case AIType::SMART_RANDOM:
            return std::make_unique<SmartRandomAI>(2, false);
        case AIType::HYBRID_EVALUATOR:
            return std::make_unique<HybridEvaluatorAI>(weights, false);
        case AIType::HYBRID_EVALUATOR_V2:
            return std::make_unique<HybridEvaluatorAIv2>(weights, 2, 10, true, false, false);
        default:
            return std::make_unique<SmartRandomAI>(2, false);
    }
}

QString GameController::getWeightFilename(AIType type) const {
    switch (type) {
        case AIType::HYBRID_EVALUATOR:
            return hybridWeightsPath_;
        case AIType::HYBRID_EVALUATOR_V2:
            return hybridV2WeightsPath_;
        default:
            return QString();
    }
}

std::unique_ptr<EvaluationWeights> GameController::loadWeightsForAI(AIType type) {
    QString filename = getWeightFilename(type);
    if (filename.isEmpty()) {
        return nullptr;
    }

    auto weights = std::make_unique<EvaluationWeights>();
    if (weights->loadFromFile(filename.toStdString())) {
        return weights;
    }

    return nullptr;
}
