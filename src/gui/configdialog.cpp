// Config Dialog - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "configdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFileDialog>

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Configuration");
    setupUI();
}

void ConfigDialog::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Weight files group
    auto* weightsGroup = new QGroupBox("AI Weight Files", this);
    auto* weightsLayout = new QVBoxLayout(weightsGroup);

    // Hybrid Evaluator weights
    auto* hybridLayout = new QHBoxLayout();
    hybridLayout->addWidget(new QLabel("Hybrid Evaluator:", this));
    hybridPathEdit_ = new QLineEdit(this);
    hybridPathEdit_->setPlaceholderText("hybrid_evaluator_weights.txt");
    auto* hybridBrowseBtn = new QPushButton("Browse...", this);
    hybridLayout->addWidget(hybridPathEdit_, 1);
    hybridLayout->addWidget(hybridBrowseBtn);
    weightsLayout->addLayout(hybridLayout);

    // Hybrid Evaluator V2 weights
    auto* hybridV2Layout = new QHBoxLayout();
    hybridV2Layout->addWidget(new QLabel("Hybrid Evaluator V2:", this));
    hybridV2PathEdit_ = new QLineEdit(this);
    hybridV2PathEdit_->setPlaceholderText("hybrid_evaluator_v2_weights.txt");
    auto* hybridV2BrowseBtn = new QPushButton("Browse...", this);
    hybridV2Layout->addWidget(hybridV2PathEdit_, 1);
    hybridV2Layout->addWidget(hybridV2BrowseBtn);
    weightsLayout->addLayout(hybridV2Layout);

    mainLayout->addWidget(weightsGroup);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    auto* okButton = new QPushButton("OK", this);
    auto* cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(hybridBrowseBtn, &QPushButton::clicked, this, &ConfigDialog::browseHybridEvaluator);
    connect(hybridV2BrowseBtn, &QPushButton::clicked, this, &ConfigDialog::browseHybridEvaluatorV2);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    setMinimumWidth(500);
}

void ConfigDialog::setHybridEvaluatorPath(const QString& path) {
    hybridPathEdit_->setText(path);
}

void ConfigDialog::setHybridEvaluatorV2Path(const QString& path) {
    hybridV2PathEdit_->setText(path);
}

QString ConfigDialog::getHybridEvaluatorPath() const {
    return hybridPathEdit_->text();
}

QString ConfigDialog::getHybridEvaluatorV2Path() const {
    return hybridV2PathEdit_->text();
}

void ConfigDialog::browseHybridEvaluator() {
    QString filename = QFileDialog::getOpenFileName(this,
        "Select Hybrid Evaluator Weights File",
        QString(),
        "Text Files (*.txt);;All Files (*)");
    if (!filename.isEmpty()) {
        hybridPathEdit_->setText(filename);
    }
}

void ConfigDialog::browseHybridEvaluatorV2() {
    QString filename = QFileDialog::getOpenFileName(this,
        "Select Hybrid Evaluator V2 Weights File",
        QString(),
        "Text Files (*.txt);;All Files (*)");
    if (!filename.isEmpty()) {
        hybridV2PathEdit_->setText(filename);
    }
}
