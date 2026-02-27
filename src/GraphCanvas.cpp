#include "GraphCanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QTimer>

static constexpr double VERTEX_RADIUS = 10.0;

// Convert index -> label (A, B, C, ...)
static QString indexToLetters(int index) {
    QString s;
    int i = index;
    do {
        int r = i % 26;
        s.prepend(QChar('A' + r));
        i = i / 26 - 1;
    } while (i >= 0);
    return s;
}

GraphCanvas::GraphCanvas(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setAutoFillBackground(true);

    // Timer animation
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() {
        if (animationIndex + 1 < static_cast<int>(routeEdgeOrder.size())) {
            animationIndex++;
            update();
        } else {
            animationTimer->stop();
            animationIndex = -1;
        }
    });
}

/* ============================================================
   SET ROUTE — Euler hoặc Postman
   ============================================================ */
void GraphCanvas::setRoute(const std::vector<int>& edgeOrder) {
    routeEdgeOrder = edgeOrder;
    duplicateEdgeIds.clear();
    originalEdgeCount = 0;
    update();
}

// ✅ Phiên bản mới: có thêm danh sách cạnh duplicated
void GraphCanvas::setRouteWithDuplicates(const std::vector<int>& edgeOrder,
                                         const std::vector<int>& dupIds,
                                         int originalEdgeCount) {
    routeEdgeOrder = edgeOrder;
    duplicateEdgeIds = dupIds;
    this->originalEdgeCount = originalEdgeCount;
    update();
}

void GraphCanvas::clearRoute() {
    routeEdgeOrder.clear();
    duplicateEdgeIds.clear();
    update();
}

/* ============================================================
   XỬ LÝ CHUỘT
   ============================================================ */
int GraphCanvas::hitTestVertex(const QPointF &p) const {
    for (const auto &v : graph.getVertices()) {
        if (QLineF(p, v.position).length() <= VERTEX_RADIUS + 3)
            return v.id;
    }
    return -1;
}

/* ============================================================
   HÀM VẼ CHÍNH
   ============================================================ */
void GraphCanvas::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (!backgroundImage.isNull())
        painter.drawImage(rect(), backgroundImage);

    const auto &edges = graph.getEdges();
    const auto &verts = graph.getVertices();

    // --- Vẽ cạnh nền ---
    QPen basePen(QColor(160, 160, 160));
    basePen.setWidth(2);
    painter.setPen(basePen);
    for (const auto &e : edges) {
        const auto &u = verts[e.u];
        const auto &v = verts[e.v];
        painter.drawLine(u.position, v.position);
    }

    // --- Vẽ route nếu có ---
    if (!routeEdgeOrder.empty()) {
        // 🔵 Cạnh gốc (không duplicated)
        QPen normalPen(QColor(0, 120, 255));
        normalPen.setWidth(4);
        painter.setPen(normalPen);

        for (int eid : routeEdgeOrder) {
            if (eid < 0 || static_cast<size_t>(eid) >= edges.size()) continue;
            const auto &e = edges[eid];
            const auto &u = verts[e.u];
            const auto &v = verts[e.v];
            painter.drawLine(u.position, v.position);
        }

        // 🔴 Cạnh duplicated (theo danh sách ID gốc)
        if (!duplicateEdgeIds.empty()) {
            QPen dupPen(QColor(255, 0, 0));
            dupPen.setWidth(4);
            dupPen.setStyle(Qt::DashDotLine);
            painter.setPen(dupPen);

            for (int eid : duplicateEdgeIds) {
                if (eid >= 0 && static_cast<size_t>(eid) < edges.size()) {
                    const auto &e = edges[eid];
                    const auto &u = verts[e.u];
                    const auto &v = verts[e.v];
                    painter.drawLine(u.position, v.position);
                }
            }
        }
    }

    // --- Vẽ nhãn cạnh ---
    painter.setPen(Qt::black);
    QFont edgeFont = painter.font();
    edgeFont.setPointSizeF(std::max(15.0, edgeFont.pointSizeF() * 1.5));
    painter.setFont(edgeFont);
    for (const auto &e : edges) {
        const auto &u = verts[e.u];
        const auto &v = verts[e.v];
        QPointF mid((u.position.x() + v.position.x()) / 2.0,
                    (u.position.y() + v.position.y()) / 2.0);
        QPointF d = v.position - u.position;
        double len = std::hypot(d.x(), d.y());
        QPointF n = (len > 0.0) ? QPointF(-d.y()/len, d.x()/len) : QPointF(0.0, -1.0);
        QPointF pos = mid + n * 10.0;
        QRectF r(pos.x() - 14, pos.y() - 14, 28, 28);
        painter.drawText(r, Qt::AlignCenter, QString::number(e.id + 1));
    }

    // --- Vẽ đỉnh ---
    QFont vertexFont = painter.font();
    vertexFont.setPointSizeF(16);
    painter.setFont(vertexFont);
    for (const auto &v : verts) {
        painter.setBrush(Qt::white);
        painter.setPen(QPen(Qt::black, 2));
        painter.drawEllipse(v.position, VERTEX_RADIUS, VERTEX_RADIUS);
        QString label = v.name.isEmpty() ? indexToLetters(v.id) : v.name;
        QRectF textRect(v.position.x() - VERTEX_RADIUS,
                        v.position.y() - VERTEX_RADIUS - 28,
                        VERTEX_RADIUS*2, VERTEX_RADIUS*2);
        painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignBottom, label);
    }
}

/* ============================================================
   CHUỘT
   ============================================================ */
void GraphCanvas::mousePressEvent(QMouseEvent *event) {
    if (mode == AddVertex) {
        graph.addVertex(event->pos());
    } else if (mode == AddEdge) {
        if (selectedVertex < 0)
            selectedVertex = hitTestVertex(event->pos());
        else {
            int v2 = hitTestVertex(event->pos());
            if (v2 >= 0 && v2 != selectedVertex)
                graph.addEdge(selectedVertex, v2);
            selectedVertex = -1;
        }
    } else if (mode == Eraser) {
        int vid = hitTestVertex(event->pos());
        if (vid >= 0) graph.removeVertex(vid);
    } else if (mode == MoveVertex) {
        selectedVertex = hitTestVertex(event->pos());
    }
    update();
}

void GraphCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (mode == MoveVertex && selectedVertex >= 0) {
        auto &verts = graph.getVertices();
        if (selectedVertex < static_cast<int>(verts.size()))
            graph.moveVertex(selectedVertex, event->pos());
        update();
    }
}

void GraphCanvas::mouseReleaseEvent(QMouseEvent *event) {
    if (mode == MoveVertex) {
        selectedVertex = -1;
    }
}
