// Game Setup Dialog - Player selection for new game
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QDialog>
#include <QComboBox>
#include "gamecontroller.h"

class GameSetupDialog : public QDialog {
    Q_OBJECT

public:
    explicit GameSetupDialog(QWidget* parent = nullptr);
    GameConfig getConfig() const;

private slots:
    void onPlayer1TypeChanged(int index);
    void onPlayer2TypeChanged(int index);
    void onPlayer1AITypeChanged(int index);
    void onPlayer2AITypeChanged(int index);

private:
    void setupUI();

    QComboBox* player1TypeCombo_;
    QComboBox* player1AITypeCombo_;
    QComboBox* player1LevelCombo_;
    QComboBox* player2TypeCombo_;
    QComboBox* player2AITypeCombo_;
    QComboBox* player2LevelCombo_;

    struct LastSettings {
        int p1Type   = 0;  // 0=Human, 1=AI
        int p1AIType = 0;  // 0=Smart Random, 1=Hybrid, 2=Hybrid v2
        int p1Level  = 2;
        int p2Type   = 1;  // default: AI
        int p2AIType = 1;  // default: Hybrid Evaluator
        int p2Level  = 2;
    };
    static LastSettings lastSettings_;
};
