// Grid Graphics Item - Dynamic grid rendering for infinite board
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QGraphicsItem>
#include <set>

class GridGraphicsItem : public QGraphicsItem {
public:
    explicit GridGraphicsItem(qreal cellSize = 60.0);

    void setVisibleRange(int minX, int maxX, int minY, int maxY);
    void expandToInclude(int x, int y);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    qreal cellSize() const { return cellSize_; }

private:
    qreal cellSize_;
    int minX_ = -5, maxX_ = 5;
    int minY_ = -5, maxY_ = 5;
};
