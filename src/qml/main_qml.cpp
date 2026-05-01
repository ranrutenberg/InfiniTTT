// QML application entry point
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include "boarditem.h"
#include "gamecontroller.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("InfiniTTT");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("InfiniTTT");

    QQuickStyle::setStyle("Material");

    qmlRegisterType<BoardItem>("InfiniTTT", 1, 0, "BoardItem");

    GameController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("gameController", &controller);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
