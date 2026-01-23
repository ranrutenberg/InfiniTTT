// Board View - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "boardview.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QGestureEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QScrollBar>
#include <cmath>

BoardView::BoardView(QWidget* parent)
    : QGraphicsView(parent) {
    setupScene();

    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    // Enable touch events
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::PanGesture);

    setBackgroundBrush(QBrush(QColor(240, 240, 240)));
}

void BoardView::setupScene() {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);

    gridItem_ = new GridGraphicsItem(CELL_SIZE);
    scene_->addItem(gridItem_);

    // Create highlight item (hidden by default)
    highlightItem_ = scene_->addRect(0, 0, CELL_SIZE, CELL_SIZE,
                                      QPen(Qt::transparent),
                                      QBrush(QColor(255, 255, 0, 80)));
    highlightItem_->setVisible(false);
    highlightItem_->setZValue(1);
}

void BoardView::clearBoard() {
    for (auto* item : markItems_) {
        scene_->removeItem(item);
        delete item;
    }
    markItems_.clear();
    highlightItem_->setVisible(false);

    gridItem_->setVisibleRange(-5, 5, -5, 5);
    resetView();
}

void BoardView::placeMark(int x, int y, char mark) {
    if (markItems_.contains({x, y})) {
        return;
    }

    gridItem_->expandToInclude(x, y);
    auto* item = createMarkItem(x, y, mark);
    scene_->addItem(item);
    markItems_[{x, y}] = item;

    highlightLastMove(x, y);
}

QGraphicsItem* BoardView::createMarkItem(int x, int y, char mark) {
    QPointF center = cellToScene(x, y);
    qreal margin = CELL_SIZE * 0.15;
    qreal size = CELL_SIZE - 2 * margin;

    if (mark == 'X') {
        auto* group = new QGraphicsItemGroup();
        QPen pen(QColor(220, 50, 50), 4);

        auto* line1 = new QGraphicsLineItem(
            center.x() - size / 2, center.y() - size / 2,
            center.x() + size / 2, center.y() + size / 2
        );
        line1->setPen(pen);

        auto* line2 = new QGraphicsLineItem(
            center.x() + size / 2, center.y() - size / 2,
            center.x() - size / 2, center.y() + size / 2
        );
        line2->setPen(pen);

        group->addToGroup(line1);
        group->addToGroup(line2);
        group->setZValue(2);
        return group;
    } else {
        auto* ellipse = new QGraphicsEllipseItem(
            center.x() - size / 2, center.y() - size / 2,
            size, size
        );
        ellipse->setPen(QPen(QColor(50, 50, 220), 4));
        ellipse->setBrush(Qt::NoBrush);
        ellipse->setZValue(2);
        return ellipse;
    }
}

void BoardView::highlightLastMove(int x, int y) {
    QPointF topLeft(x * CELL_SIZE, y * CELL_SIZE);
    highlightItem_->setRect(topLeft.x(), topLeft.y(), CELL_SIZE, CELL_SIZE);
    highlightItem_->setVisible(true);
}

void BoardView::centerOnPosition(int x, int y) {
    centerOn(cellToScene(x, y));
}

void BoardView::setInteractionEnabled(bool enabled) {
    interactionEnabled_ = enabled;
}

void BoardView::resetView() {
    zoomFactor_ = 1.0;
    resetTransform();
    centerOn(0, 0);
}

QPointF BoardView::cellToScene(int x, int y) const {
    return QPointF(x * CELL_SIZE + CELL_SIZE / 2, y * CELL_SIZE + CELL_SIZE / 2);
}

std::pair<int, int> BoardView::sceneToCell(const QPointF& pos) const {
    int cellX = static_cast<int>(std::floor(pos.x() / CELL_SIZE));
    int cellY = static_cast<int>(std::floor(pos.y() / CELL_SIZE));
    return {cellX, cellY};
}

void BoardView::zoomToFactor(qreal factor) {
    factor = qBound(MIN_ZOOM, factor, MAX_ZOOM);
    qreal scale = factor / zoomFactor_;
    zoomFactor_ = factor;
    QGraphicsView::scale(scale, scale);
}

void BoardView::wheelEvent(QWheelEvent* event) {
    const qreal scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        zoomToFactor(zoomFactor_ * scaleFactor);
    } else {
        zoomToFactor(zoomFactor_ / scaleFactor);
    }
    event->accept();
}

void BoardView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        isPanning_ = true;
        lastPanPos_ = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else if (event->button() == Qt::LeftButton && interactionEnabled_) {
        QPointF scenePos = mapToScene(event->pos());
        auto [cellX, cellY] = sceneToCell(scenePos);
        emit cellClicked(cellX, cellY);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void BoardView::mouseMoveEvent(QMouseEvent* event) {
    if (isPanning_) {
        QPoint delta = event->pos() - lastPanPos_;
        lastPanPos_ = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void BoardView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        isPanning_ = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void BoardView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
}

bool BoardView::event(QEvent* event) {
    if (event->type() == QEvent::Gesture) {
        auto* gestureEvent = static_cast<QGestureEvent*>(event);

        if (auto* pinch = static_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture))) {
            if (pinch->state() == Qt::GestureStarted) {
                pinchStartZoom_ = zoomFactor_;
            }
            qreal scaleFactor = pinch->totalScaleFactor();
            zoomToFactor(pinchStartZoom_ * scaleFactor);
            return true;
        }

        if (auto* pan = static_cast<QPanGesture*>(gestureEvent->gesture(Qt::PanGesture))) {
            QPointF delta = pan->delta();
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
            verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
            return true;
        }
    }

    return QGraphicsView::event(event);
}
