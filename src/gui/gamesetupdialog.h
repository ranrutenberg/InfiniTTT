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

private:
    void setupUI();

    QComboBox* player1TypeCombo_;
    QComboBox* player1AITypeCombo_;
    QComboBox* player2TypeCombo_;
    QComboBox* player2AITypeCombo_;
};
