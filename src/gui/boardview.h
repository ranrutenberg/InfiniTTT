// Board View - Zoomable, pannable game board visualization
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMap>
#include "gridgraphicsitem.h"

class BoardView : public QGraphicsView {
    Q_OBJECT

public:
    explicit BoardView(QWidget* parent = nullptr);

    void clearBoard();
    void placeMark(int x, int y, char mark);
    void highlightLastMove(int x, int y);
    void centerOnPosition(int x, int y);
    void setInteractionEnabled(bool enabled);
    void resetView();

signals:
    void cellClicked(int x, int y);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    // Touch support for Android
    bool event(QEvent* event) override;

private:
    void setupScene();
    QPointF cellToScene(int x, int y) const;
    std::pair<int, int> sceneToCell(const QPointF& pos) const;
    void zoomToFactor(qreal factor);
    QGraphicsItem* createMarkItem(int x, int y, char mark);

    QGraphicsScene* scene_;
    GridGraphicsItem* gridItem_;
    QMap<std::pair<int, int>, QGraphicsItem*> markItems_;
    QGraphicsRectItem* highlightItem_ = nullptr;

    qreal zoomFactor_ = 1.0;
    static constexpr qreal CELL_SIZE = 60.0;
    static constexpr qreal MIN_ZOOM = 0.1;
    static constexpr qreal MAX_ZOOM = 5.0;

    bool isPanning_ = false;
    QPoint lastPanPos_;
    bool interactionEnabled_ = true;

    // Touch gesture state
    qreal pinchStartZoom_ = 1.0;
};
