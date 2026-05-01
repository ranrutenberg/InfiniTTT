// BoardItem - Implementation
// SPDX-FileCopyrightText: 2024 Ran Rutenberg <ran.rutenberg@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "boarditem.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTouchEvent>
#include <cmath>

BoardItem::BoardItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptTouchEvents(true);
    setFlag(ItemHasContents);
    setAntialiasing(true);
}

// ── Coordinate helpers ──────────────────────────────────────────────────────

QPointF BoardItem::cellCenter(int cx, int cy) const {
    qreal cs = CELL_SIZE * zoom_;
    return {
        width()  / 2.0 + panX_ + (cx + 0.5) * cs,
        height() / 2.0 + panY_ - (cy + 0.5) * cs
    };
}

QPair<int,int> BoardItem::screenToCell(qreal px, qreal py) const {
    qreal cs = CELL_SIZE * zoom_;
    int cx = (int)std::floor((px - width()  / 2.0 - panX_) / cs);
    int cy = (int)std::floor((height() / 2.0 + panY_ - py) / cs);
    return {cx, cy};
}

void BoardItem::applyZoom(qreal newZoom, QPointF anchor) {
    newZoom = std::clamp(newZoom, MIN_ZOOM, MAX_ZOOM);
    // Keep the point under the anchor fixed on screen
    qreal ratio = newZoom / zoom_;
    panX_ = anchor.x() - width()  / 2.0 - ratio * (anchor.x() - width()  / 2.0 - panX_);
    panY_ = anchor.y() - height() / 2.0 - ratio * (anchor.y() - height() / 2.0 - panY_);
    zoom_ = newZoom;
    update();
}

// ── Public API ───────────────────────────────────────────────────────────────

void BoardItem::placeMark(int x, int y, int markAscii) {
    marks_[{x, y}] = static_cast<char>(markAscii);
    highlightCell(x, y);
}

void BoardItem::clearBoard() {
    marks_.clear();
    highlightX_ = INT_MIN;
    highlightY_ = INT_MIN;
    resetView();
}

void BoardItem::highlightCell(int x, int y) {
    highlightX_ = x;
    highlightY_ = y;
    update();
}

void BoardItem::centerOn(int x, int y) {
    qreal cs = CELL_SIZE * zoom_;
    panX_ = -(x + 0.5) * cs;
    panY_ =  (y + 0.5) * cs;
    update();
}

void BoardItem::resetView() {
    zoom_ = 1.0;
    panX_ = 0.0;
    panY_ = 0.0;
    update();
}

void BoardItem::rebuildMarks(const QVariantList& history) {
    marks_.clear();
    highlightX_ = INT_MIN;
    highlightY_ = INT_MIN;
    for (const QVariant& v : history) {
        QVariantMap m = v.toMap();
        int x  = m["x"].toInt();
        int y  = m["y"].toInt();
        char mark = static_cast<char>(m["mark"].toInt());
        marks_[{x, y}] = mark;
    }
    if (!history.isEmpty()) {
        QVariantMap last = history.last().toMap();
        highlightX_ = last["x"].toInt();
        highlightY_ = last["y"].toInt();
    }
    update();
}

// ── Painting ─────────────────────────────────────────────────────────────────

void BoardItem::paint(QPainter* painter) {
    qreal w  = width();
    qreal h  = height();
    qreal cs = CELL_SIZE * zoom_;

    painter->setRenderHint(QPainter::Antialiasing);

    // Background
    painter->fillRect(QRectF(0, 0, w, h), QColor(245, 245, 245));

    // Visible cell range
    int minCx = (int)std::floor((-w / 2.0 - panX_) / cs) - 1;
    int maxCx = (int)std::floor(( w / 2.0 - panX_) / cs) + 1;
    int minCy = (int)std::floor((panY_ - h / 2.0)  / cs) - 1;
    int maxCy = (int)std::floor((panY_ + h / 2.0)  / cs) + 1;

    // Grid lines
    painter->setPen(QPen(QColor(210, 210, 210), 1));
    for (int cx = minCx; cx <= maxCx + 1; ++cx) {
        qreal sx = w / 2.0 + panX_ + cx * cs;
        painter->drawLine(QPointF(sx, 0), QPointF(sx, h));
    }
    for (int cy = minCy; cy <= maxCy + 1; ++cy) {
        qreal sy = h / 2.0 + panY_ - cy * cs;
        painter->drawLine(QPointF(0, sy), QPointF(w, sy));
    }

    // Origin axes
    painter->setPen(QPen(QColor(180, 180, 180), 2));
    qreal ox = w / 2.0 + panX_;
    qreal oy = h / 2.0 + panY_;
    painter->drawLine(QPointF(ox, 0), QPointF(ox, h));
    painter->drawLine(QPointF(0, oy), QPointF(w, oy));

    // Coordinate labels (every 5th, only when zoomed enough)
    if (zoom_ >= 0.4) {
        painter->setPen(QColor(160, 160, 160));
        painter->setFont(QFont("Monospace", qMax(7.0, 9.0 * zoom_)));
        for (int cx = minCx; cx <= maxCx; ++cx) {
            if (cx % 5 == 0) {
                qreal sx = w / 2.0 + panX_ + cx * cs + 2;
                painter->drawText(QPointF(sx, oy - 2), QString::number(cx));
            }
        }
        for (int cy = minCy; cy <= maxCy; ++cy) {
            if (cy % 5 == 0 && cy != 0) {
                qreal sy = h / 2.0 + panY_ - cy * cs - 2;
                painter->drawText(QPointF(ox + 2, sy), QString::number(cy));
            }
        }
    }

    // Highlight last move
    if (highlightX_ != INT_MIN) {
        qreal hx = w / 2.0 + panX_ + highlightX_ * cs;
        qreal hy = h / 2.0 + panY_ - (highlightY_ + 1) * cs;
        painter->fillRect(QRectF(hx, hy, cs, cs), QColor(255, 220, 0, 100));
    }

    // Marks
    qreal margin   = MARK_MARGIN * cs;
    qreal penWidth = qMax(1.5, cs * 0.07);

    for (auto it = marks_.constBegin(); it != marks_.constEnd(); ++it) {
        int  cx   = it.key().first;
        int  cy   = it.key().second;
        char mark = it.value();

        qreal left = w / 2.0 + panX_ + cx * cs + margin;
        qreal top  = h / 2.0 + panY_ - (cy + 1) * cs + margin;
        qreal size = cs - 2 * margin;

        if (size <= 0) continue;

        if (mark == 'X') {
            painter->setPen(QPen(QColor(220, 55, 55), penWidth, Qt::SolidLine, Qt::RoundCap));
            painter->drawLine(QPointF(left, top),          QPointF(left + size, top + size));
            painter->drawLine(QPointF(left + size, top),   QPointF(left, top + size));
        } else {
            painter->setPen(QPen(QColor(55, 55, 220), penWidth, Qt::SolidLine, Qt::RoundCap));
            painter->setBrush(Qt::NoBrush);
            painter->drawEllipse(QRectF(left, top, size, size));
        }
    }
}

// ── Mouse events ─────────────────────────────────────────────────────────────

void BoardItem::mousePressEvent(QMouseEvent* event) {
    pressPos_   = event->position();
    isPanning_  = (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton);
    lastPanPos_ = event->position();
    event->accept();
}

void BoardItem::mouseMoveEvent(QMouseEvent* event) {
    QPointF delta = event->position() - lastPanPos_;
    bool movedEnough = (event->position() - pressPos_).manhattanLength() >= 8;

    if (isPanning_ || (movedEnough && (event->buttons() & Qt::LeftButton))) {
        isPanning_ = true;
        panX_ += delta.x();
        panY_ += delta.y();
        lastPanPos_ = event->position();
        update();
    }
    event->accept();
}

void BoardItem::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (!isPanning_ && interactive_) {
            auto [cx, cy] = screenToCell(pressPos_.x(), pressPos_.y());
            emit cellClicked(cx, cy);
        }
        isPanning_ = false;
    } else if (event->button() == Qt::RightButton || event->button() == Qt::MiddleButton) {
        isPanning_ = false;
    }
    event->accept();
}

void BoardItem::wheelEvent(QWheelEvent* event) {
    qreal factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    applyZoom(zoom_ * factor, event->position());
    event->accept();
}

// ── Touch events ──────────────────────────────────────────────────────────────

void BoardItem::touchEvent(QTouchEvent* event) {
    const auto& pts = event->points();

    if (event->type() == QEvent::TouchBegin) {
        touchMoved_        = false;
        activeTouchPoints_ = pts.size();
        if (pts.size() == 1) {
            touchStartPan_ = {panX_, panY_};
        } else if (pts.size() >= 2) {
            QPointF a = pts[0].position(), b = pts[1].position();
            pinchStartDist_   = QLineF(a, b).length();
            pinchStartCenter_ = (a + b) / 2.0;
            pinchStartZoom_   = zoom_;
            pinchStartPan_    = {panX_, panY_};
        }
        event->accept();
        return;
    }

    if (event->type() == QEvent::TouchUpdate) {
        if (pts.size() == 1) {
            QPointF delta = pts[0].position() - pts[0].pressPosition();
            if (delta.manhattanLength() > 6) touchMoved_ = true;
            panX_ = touchStartPan_.x() + delta.x();
            panY_ = touchStartPan_.y() + delta.y();
            update();
        } else if (pts.size() >= 2) {
            touchMoved_ = true;
            QPointF a = pts[0].position(), b = pts[1].position();
            qreal dist   = QLineF(a, b).length();
            QPointF center = (a + b) / 2.0;

            qreal newZoom = std::clamp(pinchStartZoom_ * dist / pinchStartDist_,
                                       MIN_ZOOM, MAX_ZOOM);
            qreal ratio = newZoom / pinchStartZoom_;
            panX_ = pinchStartPan_.x() + center.x() - pinchStartCenter_.x()
                    - (ratio - 1.0) * (pinchStartCenter_.x() - width()  / 2.0);
            panY_ = pinchStartPan_.y() + center.y() - pinchStartCenter_.y()
                    - (ratio - 1.0) * (pinchStartCenter_.y() - height() / 2.0);
            zoom_ = newZoom;
            update();
        }
        event->accept();
        return;
    }

    if (event->type() == QEvent::TouchEnd) {
        if (!touchMoved_ && activeTouchPoints_ == 1 && interactive_) {
            QPointF pos = pts[0].position();
            auto [cx, cy] = screenToCell(pos.x(), pos.y());
            emit cellClicked(cx, cy);
        }
        event->accept();
        return;
    }

    event->accept();
}

void BoardItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) {
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    update();
}
