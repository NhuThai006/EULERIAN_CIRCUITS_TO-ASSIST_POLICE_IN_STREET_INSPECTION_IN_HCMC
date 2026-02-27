#pragma once
#include <QWidget>
#include <QImage>
#include <QTimer>
#include <vector>
#include "Graph.h"

class GraphCanvas : public QWidget {
    Q_OBJECT
public:
    enum Mode {
        None,
        AddVertex,
        AddEdge,
        MoveVertex,
        Eraser
    };

    explicit GraphCanvas(QWidget *parent = nullptr);

    // === Graph model access ===
    Graph& model() { return graph; }
    const Graph& model() const { return graph; }

    // === Route control ===
    void setRoute(const std::vector<int>& edgeOrder);
    void setRouteWithDuplicates(const std::vector<int>& edgeOrder,
                                const std::vector<int>& dupIds,
                                int originalEdgeCount);
    void clearRoute();

    // === Background ===
    void setBackgroundImage(const QImage &img) { backgroundImage = img; update(); }
    void clearBackgroundImage() { backgroundImage = QImage(); update(); }
    const QImage& getBackgroundImage() const { return backgroundImage; }

    // === Mode ===
    void setMode(Mode m) { mode = m; } // <-- SỬA LỖI TẠI ĐÂY: Thêm hàm bị thiếu

signals:
    void statusMessage(const QString &msg);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    Graph graph;
    QImage backgroundImage;
    Mode mode{None};
    int selectedVertex{-1};

    // === Route data ===
    std::vector<int> routeEdgeOrder;
    int originalEdgeCount{0};
    std::vector<int> duplicateEdgeIds;

    // === Animation ===
    QTimer *animationTimer{nullptr};
    int animationIndex{-1};

    // === Utility ===
    int hitTestVertex(const QPointF &p) const;
};