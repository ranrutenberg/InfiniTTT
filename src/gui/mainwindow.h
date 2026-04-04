// Main Window - Application main window with menu and status
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QMainWindow>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include "boardview.h"
#include "gamecontroller.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNewGame();
    void onMoveExecuted(int x, int y, char mark);
    void onGameOver(char winner);
    void onTurnChanged(char currentPlayer);
    void onAIThinking(bool thinking);
    void onResetView();
    void onConfiguration();
    void onAbout();
    void onUndoMove();
    void onMoveUndone();

private:
    void setupUI();
    void setupMenus();
    void connectSignals();
    void loadSettings();
    void saveSettings();

    BoardView* boardView_;
    GameController* controller_;
    QLabel* turnLabel_;
    QLabel* moveCountLabel_;
    QLabel* statusLabel_;
    QToolBar* undoToolBar_;
    QAction* undoAction_;

    char currentPlayer_ = 'X';
    bool gameActive_ = false;
    int moveCount_ = 0;
};
