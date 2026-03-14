// Game Setup Dialog - Player selection for new game
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef GAMESETUPDIALOG_H
#define GAMESETUPDIALOG_H

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
};

#endif // GAMESETUPDIALOG_H
