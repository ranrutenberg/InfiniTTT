// Grid Graphics Item - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "gridgraphicsitem.h"
#include <QPainter>
#include <algorithm>

GridGraphicsItem::GridGraphicsItem(qreal cellSize)
    : cellSize_(cellSize) {
}

void GridGraphicsItem::setVisibleRange(int minX, int maxX, int minY, int maxY) {
    prepareGeometryChange();
    minX_ = minX;
    maxX_ = maxX;
    minY_ = minY;
    maxY_ = maxY;
}

void GridGraphicsItem::expandToInclude(int x, int y) {
    bool changed = false;
    if (x - 2 < minX_) { minX_ = x - 2; changed = true; }
    if (x + 2 > maxX_) { maxX_ = x + 2; changed = true; }
    if (y - 2 < minY_) { minY_ = y - 2; changed = true; }
    if (y + 2 > maxY_) { maxY_ = y + 2; changed = true; }
    if (changed) {
        prepareGeometryChange();
    }
}

QRectF GridGraphicsItem::boundingRect() const {
    qreal left = minX_ * cellSize_;
    qreal top = minY_ * cellSize_;
    qreal width = (maxX_ - minX_ + 1) * cellSize_;
    qreal height = (maxY_ - minY_ + 1) * cellSize_;
    return QRectF(left, top, width, height);
}

void GridGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setRenderHint(QPainter::Antialiasing, false);

    // Draw grid background
    QRectF rect = boundingRect();
    painter->fillRect(rect, QColor(250, 250, 250));

    // Draw grid lines
    QPen gridPen(QColor(200, 200, 200), 1);
    painter->setPen(gridPen);

    // Vertical lines
    for (int x = minX_; x <= maxX_ + 1; ++x) {
        qreal xPos = x * cellSize_;
        painter->drawLine(QPointF(xPos, rect.top()), QPointF(xPos, rect.bottom()));
    }

    // Horizontal lines
    for (int y = minY_; y <= maxY_ + 1; ++y) {
        qreal yPos = y * cellSize_;
        painter->drawLine(QPointF(rect.left(), yPos), QPointF(rect.right(), yPos));
    }

    // Draw origin marker
    QPen originPen(QColor(180, 180, 180), 2);
    painter->setPen(originPen);

    // Vertical line through origin
    if (minX_ <= 0 && maxX_ >= 0) {
        painter->drawLine(QPointF(0, rect.top()), QPointF(0, rect.bottom()));
    }
    // Horizontal line through origin
    if (minY_ <= 0 && maxY_ >= 0) {
        painter->drawLine(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
    }

    // Draw coordinate labels
    painter->setPen(QColor(150, 150, 150));
    QFont font = painter->font();
    font.setPixelSize(10);
    painter->setFont(font);

    // X-axis labels (top)
    for (int x = minX_; x <= maxX_; ++x) {
        if (x % 5 == 0) {
            qreal xPos = x * cellSize_ + cellSize_ / 2;
            painter->drawText(QRectF(xPos - 15, rect.top() - 15, 30, 15),
                              Qt::AlignCenter, QString::number(x));
        }
    }

    // Y-axis labels (left)
    for (int y = minY_; y <= maxY_; ++y) {
        if (y % 5 == 0) {
            qreal yPos = y * cellSize_ + cellSize_ / 2;
            painter->drawText(QRectF(rect.left() - 25, yPos - 7, 20, 15),
                              Qt::AlignRight | Qt::AlignVCenter, QString::number(y));
        }
    }
}
