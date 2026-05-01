// BoardItem - QML-renderable infinite game board with pan/zoom/touch
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QQuickPaintedItem>
#include <QMap>
#include <QPair>
#include <QVariantList>
#include <climits>

class BoardItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive)

public:
    explicit BoardItem(QQuickItem* parent = nullptr);

    void paint(QPainter* painter) override;

    bool isInteractive() const { return interactive_; }
    void setInteractive(bool v) { interactive_ = v; }

    Q_INVOKABLE void placeMark(int x, int y, int markAscii);
    Q_INVOKABLE void clearBoard();
    Q_INVOKABLE void highlightCell(int x, int y);
    Q_INVOKABLE void centerOn(int x, int y);
    Q_INVOKABLE void resetView();
    Q_INVOKABLE void rebuildMarks(const QVariantList& history);

signals:
    void cellClicked(int x, int y);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void touchEvent(QTouchEvent* event) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private:
    QPointF cellCenter(int cx, int cy) const;
    QPair<int,int> screenToCell(qreal px, qreal py) const;
    void applyZoom(qreal newZoom, QPointF anchor);

    static constexpr qreal CELL_SIZE = 60.0;
    static constexpr qreal MIN_ZOOM  = 0.1;
    static constexpr qreal MAX_ZOOM  = 5.0;
    static constexpr qreal MARK_MARGIN = 0.15;

    QMap<QPair<int,int>, char> marks_;
    int highlightX_ = INT_MIN;
    int highlightY_ = INT_MIN;

    qreal panX_ = 0.0;
    qreal panY_ = 0.0;
    qreal zoom_ = 1.0;

    // Mouse pan state
    bool isPanning_ = false;
    QPointF lastPanPos_;
    QPointF pressPos_;

    // Touch state
    QPointF touchStartPan_;
    qreal   pinchStartZoom_ = 1.0;
    qreal   pinchStartDist_ = 1.0;
    QPointF pinchStartCenter_;
    QPointF pinchStartPan_;
    bool    touchMoved_     = false;
    int     activeTouchPoints_ = 0;

    bool interactive_ = true;
};
