// Main Window - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "mainwindow.h"
#include "gamesetupdialog.h"
#include "configdialog.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSettings>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUI();
    setupMenus();
    connectSignals();
    loadSettings();

    setWindowTitle("InfiniTTT - Infinite Tic-Tac-Toe");
    resize(800, 600);

    // Start with new game dialog
    QTimer::singleShot(100, this, &MainWindow::onNewGame);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    boardView_ = new BoardView(this);
    setCentralWidget(boardView_);

    controller_ = new GameController(this);

    // Status bar
    turnLabel_ = new QLabel("Turn: X", this);
    statusLabel_ = new QLabel("Start a new game", this);

    statusBar()->addWidget(turnLabel_);
    statusBar()->addPermanentWidget(statusLabel_);
}

void MainWindow::setupMenus() {
    // Game menu
    QMenu* gameMenu = menuBar()->addMenu("&Game");

    QAction* newGameAction = gameMenu->addAction("&New Game");
    newGameAction->setShortcut(QKeySequence::New);
    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGame);

    gameMenu->addSeparator();

    QAction* resetViewAction = gameMenu->addAction("&Reset View");
    resetViewAction->setShortcut(QKeySequence(Qt::Key_Home));
    connect(resetViewAction, &QAction::triggered, this, &MainWindow::onResetView);

    gameMenu->addSeparator();

    QAction* configAction = gameMenu->addAction("&Configuration...");
    configAction->setShortcut(QKeySequence::Preferences);
    connect(configAction, &QAction::triggered, this, &MainWindow::onConfiguration);

    gameMenu->addSeparator();

    QAction* exitAction = gameMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Help menu
    QMenu* helpMenu = menuBar()->addMenu("&Help");

    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::connectSignals() {
    connect(boardView_, &BoardView::cellClicked,
            controller_, &GameController::handleCellClick);

    connect(controller_, &GameController::moveExecuted,
            this, &MainWindow::onMoveExecuted);

    connect(controller_, &GameController::turnChanged,
            this, &MainWindow::onTurnChanged);

    connect(controller_, &GameController::gameOver,
            this, &MainWindow::onGameOver);

    connect(controller_, &GameController::aiThinking,
            this, &MainWindow::onAIThinking);
}

void MainWindow::loadSettings() {
    QSettings settings("InfiniTTT", "InfiniTTT");

    QString hybridPath = settings.value("weights/hybrid_evaluator",
        "hybrid_evaluator_weights.txt").toString();
    QString hybridV2Path = settings.value("weights/hybrid_evaluator_v2",
        "hybrid_evaluator_v2_weights.txt").toString();

    controller_->setHybridEvaluatorWeightsPath(hybridPath);
    controller_->setHybridEvaluatorV2WeightsPath(hybridV2Path);
}

void MainWindow::saveSettings() {
    QSettings settings("InfiniTTT", "InfiniTTT");

    settings.setValue("weights/hybrid_evaluator",
        controller_->getHybridEvaluatorWeightsPath());
    settings.setValue("weights/hybrid_evaluator_v2",
        controller_->getHybridEvaluatorV2WeightsPath());
}

void MainWindow::onNewGame() {
    GameSetupDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        GameConfig config = dialog.getConfig();
        boardView_->clearBoard();
        controller_->startNewGame(config);
        gameActive_ = true;
        statusLabel_->setText("Game in progress");
        boardView_->setInteractionEnabled(true);
    }
}

void MainWindow::onMoveExecuted(int x, int y, char mark) {
    boardView_->placeMark(x, y, mark);
    boardView_->centerOnPosition(x, y);
}

void MainWindow::onGameOver(char winner) {
    gameActive_ = false;
    boardView_->setInteractionEnabled(false);

    QString message;
    if (winner == 'D') {
        message = "Game ended in a draw!";
        statusLabel_->setText("Draw");
    } else {
        message = QString("Player %1 wins!").arg(winner);
        statusLabel_->setText(QString("%1 wins!").arg(winner));
    }

    QMessageBox::information(this, "Game Over", message);
}

void MainWindow::onTurnChanged(char currentPlayer) {
    currentPlayer_ = currentPlayer;
    turnLabel_->setText(QString("Turn: %1").arg(currentPlayer));
}

void MainWindow::onAIThinking(bool thinking) {
    if (thinking) {
        statusLabel_->setText("AI thinking...");
        boardView_->setInteractionEnabled(false);
    } else {
        statusLabel_->setText("Game in progress");
        boardView_->setInteractionEnabled(gameActive_);
    }
}

void MainWindow::onResetView() {
    boardView_->resetView();
}

void MainWindow::onConfiguration() {
    ConfigDialog dialog(this);
    dialog.setHybridEvaluatorPath(controller_->getHybridEvaluatorWeightsPath());
    dialog.setHybridEvaluatorV2Path(controller_->getHybridEvaluatorV2WeightsPath());

    if (dialog.exec() == QDialog::Accepted) {
        controller_->setHybridEvaluatorWeightsPath(dialog.getHybridEvaluatorPath());
        controller_->setHybridEvaluatorV2WeightsPath(dialog.getHybridEvaluatorV2Path());
        saveSettings();
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About InfiniTTT",
        "<h3>InfiniTTT - Infinite Tic-Tac-Toe</h3>"
        "<p>A tic-tac-toe game with infinite board and AI opponents.</p>"
        "<p><b>Controls:</b></p>"
        "<ul>"
        "<li>Left click: Place mark</li>"
        "<li>Right click + drag: Pan</li>"
        "<li>Mouse wheel: Zoom</li>"
        "<li>Home: Reset view</li>"
        "</ul>"
        "<p>Win by getting 5 in a row!</p>");
}
