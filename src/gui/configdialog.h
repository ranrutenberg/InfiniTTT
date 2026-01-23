// Config Dialog - Settings for weight file paths
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QLineEdit>

class ConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = nullptr);

    void setHybridEvaluatorPath(const QString& path);
    void setHybridEvaluatorV2Path(const QString& path);

    QString getHybridEvaluatorPath() const;
    QString getHybridEvaluatorV2Path() const;

private slots:
    void browseHybridEvaluator();
    void browseHybridEvaluatorV2();

private:
    void setupUI();

    QLineEdit* hybridPathEdit_;
    QLineEdit* hybridV2PathEdit_;
};

#endif // CONFIGDIALOG_H
