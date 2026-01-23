// InfiniTTT GUI - Application entry point
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("InfiniTTT");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("InfiniTTT");

    MainWindow window;
    window.show();

    return app.exec();
}
