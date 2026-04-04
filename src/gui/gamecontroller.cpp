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
    moveHistory_.clear();

    player1AI_.reset();
    player2AI_.reset();
    weights1_.reset();
    weights2_.reset();

    if (!config_.player1.isHuman) {
        weights1_ = loadWeightsForAI(config_.player1.aiType);
        player1AI_ = createAIPlayer(config_.player1, weights1_.get());
    }

    if (!config_.player2.isHuman) {
        weights2_ = loadWeightsForAI(config_.player2.aiType);
        player2AI_ = createAIPlayer(config_.player2, weights2_.get());
    }

    emit turnChanged(currentPlayer_);

    // Always place first move at (0, 0) automatically
    QTimer::singleShot(100, this, &GameController::executeFirstMove);
}

void GameController::executeFirstMove() {
    if (!gameActive_ || moveCount_ > 0) return;

    // Place first move at (0, 0)
    board_.placeMark(0, 0);
    moveHistory_.emplace_back(0, 0, currentPlayer_);
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
    moveHistory_.emplace_back(x, y, currentPlayer_);
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
    moveHistory_.emplace_back(moveX, moveY, currentPlayer_);
    emit moveExecuted(moveX, moveY, currentPlayer_);
    lastMove_ = {moveX, moveY};
    moveCount_++;

    emit aiThinking(false);
    checkGameState(moveX, moveY);
}

void GameController::undoMove() {
    if (moveHistory_.size() <= 1) return;  // keep auto-placed first move

    auto [x, y, mark] = moveHistory_.back();
    moveHistory_.pop_back();
    board_.removeMarkDirect(x, y);
    moveCount_--;
    currentPlayer_ = mark;  // restore to the player who just moved
    gameActive_ = true;

    // update lastMove_ to the new last move
    if (!moveHistory_.empty()) {
        const auto& [lx, ly, lm] = moveHistory_.back();
        lastMove_ = {lx, ly};
    } else {
        lastMove_ = {INT_MIN, INT_MIN};
    }

    emit moveUndone();
    emit turnChanged(currentPlayer_);
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

std::unique_ptr<AIPlayer> GameController::createAIPlayer(const PlayerConfig& playerConfig, const EvaluationWeights* weights) {
    switch (playerConfig.aiType) {
        case AIType::SMART_RANDOM:
            return std::make_unique<SmartRandomAI>(playerConfig.smartRandomLevel, false);
        case AIType::HYBRID_EVALUATOR:
            return std::make_unique<HybridEvaluatorAI>(weights, false);
        case AIType::HYBRID_EVALUATOR_V2:
            return std::make_unique<HybridEvaluatorAIv2>(weights, 2, 10, true, false, false);
        default:
            return std::make_unique<SmartRandomAI>(playerConfig.smartRandomLevel, false);
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
