// Game Setup Dialog - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "gamesetupdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>

GameSetupDialog::LastSettings GameSetupDialog::lastSettings_;

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

    player1LevelCombo_ = new QComboBox(this);
    player1LevelCombo_->addItem("Level 0: Pure random");
    player1LevelCombo_->addItem("Level 1: Win detection");
    player1LevelCombo_->addItem("Level 2: Win + Block");
    player1LevelCombo_->addItem("Level 3: Win + Block + Double threat");
    player1LevelCombo_->addItem("Level 4: Win + Block + Fork + Block fork");

    player1Layout->addWidget(new QLabel("Type:", this));
    player1Layout->addWidget(player1TypeCombo_);
    player1Layout->addWidget(new QLabel("AI:", this));
    player1Layout->addWidget(player1AITypeCombo_);
    player1Layout->addWidget(new QLabel("Level:", this));
    player1Layout->addWidget(player1LevelCombo_);

    // Player 2 group
    auto* player2Group = new QGroupBox("Player 2 (O)", this);
    auto* player2Layout = new QHBoxLayout(player2Group);

    player2TypeCombo_ = new QComboBox(this);
    player2TypeCombo_->addItem("Human");
    player2TypeCombo_->addItem("AI");

    player2AITypeCombo_ = new QComboBox(this);
    player2AITypeCombo_->addItem("Smart Random");
    player2AITypeCombo_->addItem("Hybrid Evaluator");
    player2AITypeCombo_->addItem("Hybrid Evaluator v2 (Minimax)");

    player2LevelCombo_ = new QComboBox(this);
    player2LevelCombo_->addItem("Level 0: Pure random");
    player2LevelCombo_->addItem("Level 1: Win detection");
    player2LevelCombo_->addItem("Level 2: Win + Block");
    player2LevelCombo_->addItem("Level 3: Win + Block + Double threat");
    player2LevelCombo_->addItem("Level 4: Win + Block + Fork + Block fork");

    player2Layout->addWidget(new QLabel("Type:", this));
    player2Layout->addWidget(player2TypeCombo_);
    player2Layout->addWidget(new QLabel("AI:", this));
    player2Layout->addWidget(player2AITypeCombo_);
    player2Layout->addWidget(new QLabel("Level:", this));
    player2Layout->addWidget(player2LevelCombo_);

    // Restore last settings
    player1TypeCombo_->setCurrentIndex(lastSettings_.p1Type);
    player1AITypeCombo_->setCurrentIndex(lastSettings_.p1AIType);
    player1LevelCombo_->setCurrentIndex(lastSettings_.p1Level);
    player2TypeCombo_->setCurrentIndex(lastSettings_.p2Type);
    player2AITypeCombo_->setCurrentIndex(lastSettings_.p2AIType);
    player2LevelCombo_->setCurrentIndex(lastSettings_.p2Level);

    bool p1IsAI = (lastSettings_.p1Type == 1);
    player1AITypeCombo_->setEnabled(p1IsAI);
    player1LevelCombo_->setEnabled(p1IsAI && lastSettings_.p1AIType == 0);

    bool p2IsAI = (lastSettings_.p2Type == 1);
    player2AITypeCombo_->setEnabled(p2IsAI);
    player2LevelCombo_->setEnabled(p2IsAI && lastSettings_.p2AIType == 0);

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
    connect(player1AITypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameSetupDialog::onPlayer1AITypeChanged);
    connect(player2AITypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GameSetupDialog::onPlayer2AITypeChanged);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    setMinimumWidth(400);
}

void GameSetupDialog::onPlayer1TypeChanged(int index) {
    bool isAI = (index == 1);
    player1AITypeCombo_->setEnabled(isAI);
    player1LevelCombo_->setEnabled(isAI && player1AITypeCombo_->currentIndex() == 0);
}

void GameSetupDialog::onPlayer2TypeChanged(int index) {
    bool isAI = (index == 1);
    player2AITypeCombo_->setEnabled(isAI);
    player2LevelCombo_->setEnabled(isAI && player2AITypeCombo_->currentIndex() == 0);
}

void GameSetupDialog::onPlayer1AITypeChanged(int index) {
    player1LevelCombo_->setEnabled(player1TypeCombo_->currentIndex() == 1 && index == 0);
}

void GameSetupDialog::onPlayer2AITypeChanged(int index) {
    player2LevelCombo_->setEnabled(player2TypeCombo_->currentIndex() == 1 && index == 0);
}

GameConfig GameSetupDialog::getConfig() const {
    lastSettings_.p1Type   = player1TypeCombo_->currentIndex();
    lastSettings_.p1AIType = player1AITypeCombo_->currentIndex();
    lastSettings_.p1Level  = player1LevelCombo_->currentIndex();
    lastSettings_.p2Type   = player2TypeCombo_->currentIndex();
    lastSettings_.p2AIType = player2AITypeCombo_->currentIndex();
    lastSettings_.p2Level  = player2LevelCombo_->currentIndex();

    GameConfig config;

    config.player1.isHuman = (player1TypeCombo_->currentIndex() == 0);
    config.player1.aiType = static_cast<AIType>(player1AITypeCombo_->currentIndex());
    config.player1.smartRandomLevel = player1LevelCombo_->currentIndex();

    config.player2.isHuman = (player2TypeCombo_->currentIndex() == 0);
    config.player2.aiType = static_cast<AIType>(player2AITypeCombo_->currentIndex());
    config.player2.smartRandomLevel = player2LevelCombo_->currentIndex();

    return config;
}
