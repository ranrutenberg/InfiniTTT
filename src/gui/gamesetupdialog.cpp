// Game Setup Dialog - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "gamesetupdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

GameSetupDialog::GameSetupDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("New Game");
    setupUI();
}

void GameSetupDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Player 1 group
    auto* player1Group = new QGroupBox("Player 1 (X)", this);
    auto* player1Layout = new QHBoxLayout(player1Group);

    player1TypeCombo_ = new QComboBox(this);
    player1TypeCombo_->addItem("Human");
    player1TypeCombo_->addItem("AI");

    player1AITypeCombo_ = new QComboBox(this);
    player1AITypeCombo_->addItem("Smart Random");
    player1AITypeCombo_->addItem("Hybrid Evaluator");
    player1AITypeCombo_->addItem("Hybrid Evaluator v2 (Minimax)");
    player1AITypeCombo_->setEnabled(false);

    player1Layout->addWidget(new QLabel("Type:", this));
    player1Layout->addWidget(player1TypeCombo_);
    player1Layout->addWidget(new QLabel("AI:", this));
    player1Layout->addWidget(player1AITypeCombo_);

    // Player 2 group
    auto* player2Group = new QGroupBox("Player 2 (O)", this);
    auto* player2Layout = new QHBoxLayout(player2Group);

    player2TypeCombo_ = new QComboBox(this);
    player2TypeCombo_->addItem("Human");
    player2TypeCombo_->addItem("AI");
    player2TypeCombo_->setCurrentIndex(1);  // Default to AI

    player2AITypeCombo_ = new QComboBox(this);
    player2AITypeCombo_->addItem("Smart Random");
    player2AITypeCombo_->addItem("Hybrid Evaluator");
    player2AITypeCombo_->addItem("Hybrid Evaluator v2 (Minimax)");
    player2AITypeCombo_->setCurrentIndex(1);  // Default to Hybrid Evaluator

    player2Layout->addWidget(new QLabel("Type:", this));
    player2Layout->addWidget(player2TypeCombo_);
    player2Layout->addWidget(new QLabel("AI:", this));
    player2Layout->addWidget(player2AITypeCombo_);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    auto* okButton = new QPushButton("Start Game", this);
    auto* cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addWidget(player1Group);
    mainLayout->addWidget(player2Group);
    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(player1TypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameSetupDialog::onPlayer1TypeChanged);
    connect(player2TypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameSetupDialog::onPlayer2TypeChanged);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    setMinimumWidth(400);
}

void GameSetupDialog::onPlayer1TypeChanged(int index) {
    player1AITypeCombo_->setEnabled(index == 1);
}

void GameSetupDialog::onPlayer2TypeChanged(int index) {
    player2AITypeCombo_->setEnabled(index == 1);
}

GameConfig GameSetupDialog::getConfig() const {
    GameConfig config;

    config.player1.isHuman = (player1TypeCombo_->currentIndex() == 0);
    config.player1.aiType = static_cast<AIType>(player1AITypeCombo_->currentIndex());

    config.player2.isHuman = (player2TypeCombo_->currentIndex() == 0);
    config.player2.aiType = static_cast<AIType>(player2AITypeCombo_->currentIndex());

    return config;
}
