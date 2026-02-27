#pragma once
#include <QDialog>
#include <QTimer>
#include <QImage>
#include <QPainter>
#include <set>
#include "Graph.h"
#include <unordered_map>
#include <algorithm> // Để dùng std::min, std::max

class AnimationWindow : public QDialog {
    Q_OBJECT
public:
    explicit AnimationWindow(const Graph &g,
                             const std::vector<int> &route,
                             bool isPostman,
                             const std::vector<int> &dupIds = {},
                             QWidget *parent = nullptr)
        : QDialog(parent),
        graph(g),
        route(route),
        isPostman(isPostman),
        duplicateEdgeIds(dupIds)
    {
        // --- Cấu hình cửa sổ ---
        setWindowTitle(isPostman ? "Chinese Postman Animation" : "Euler Path Animation");
        resize(800, 600);
        frame = QImage(size(), QImage::Format_ARGB32_Premultiplied);

        // --- Sửa lỗi ID cho bài toán Postman ---
        if (isPostman) {
            int originalEdgeCount = static_cast<int>(graph.getEdges().size());
            for (size_t i = 0; i < duplicateEdgeIds.size(); ++i) {
                int augmentedId = originalEdgeCount + static_cast<int>(i);
                int originalId = duplicateEdgeIds[i];
                augmentedToOriginalIdMap[augmentedId] = originalId;
            }
        }

        // --- Kết nối timer ---
        connect(&timer, &QTimer::timeout, this, &AnimationWindow::nextStep);
        timer.start(550);
        
        // Vẽ trạng thái ban đầu
        drawFrame();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.drawImage(rect(), frame);
    }
    
    // Cập nhật lại kích thước của buffer `frame` khi cửa sổ thay đổi
    void resizeEvent(QResizeEvent *event) override {
        frame = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        drawFrame();
        QDialog::resizeEvent(event);
    }

private slots:
    void nextStep() {
        drawFrame();
        update();

        if (currentStep >= static_cast<int>(route.size())) {
            timer.stop();
            return;
        }
        currentStep++;
    }

    void drawFrame() {
        frame.fill(Qt::white);
        QPainter painter(&frame);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const auto &verts = graph.getVertices();
        const auto &edges = graph.getEdges();
        if (verts.empty()) return;

        // ===================================================================
        // === LOGIC CĂN GIỮA VÀ THU PHÓNG ĐÃ ĐƯỢC CẬP NHẬT ===
        // ===================================================================
        
        // --- B1: Tìm khung bao (bounding box) của đồ thị ---
        qreal minX = verts[0].position.x(), maxX = minX, minY = verts[0].position.y(), maxY = minY;
        for (const auto &v : verts) {
            minX = std::min(minX, v.position.x()); maxX = std::max(maxX, v.position.x());
            minY = std::min(minY, v.position.y()); maxY = std::max(maxY, v.position.y());
        }
        QRectF graphBounds(minX, minY, maxX - minX, maxY - minY);

        // --- B2: Tính toán tỷ lệ thu phóng (scale) ---
        qreal margin = 40.0;
        QRectF widgetRect = rect().adjusted(margin, margin, -margin, -margin);
        qreal scaleFactor = 1.0;
        if (graphBounds.width() > 1e-6 && graphBounds.height() > 1e-6) {
            qreal scaleX = widgetRect.width() / graphBounds.width();
            qreal scaleY = widgetRect.height() / graphBounds.height();
            scaleFactor = std::min(scaleX, scaleY);
        }

        // --- B3: Áp dụng các phép biến đổi (translate và scale) ---
        painter.save(); // Lưu lại trạng thái của painter
        painter.translate(widgetRect.center()); // Di chuyển gốc tọa độ đến tâm widget
        painter.scale(scaleFactor, scaleFactor); // Áp dụng thu phóng
        painter.translate(-graphBounds.center()); // Di chuyển tâm đồ thị về gốc tọa độ

        // Từ giờ, tất cả các lệnh vẽ sẽ được thực hiện trong hệ tọa độ đã được biến đổi.
        // Điều này tiện lợi hơn nhiều so với việc cộng `offset` vào từng tọa độ.

        // --- Định nghĩa các loại bút vẽ (với độ dày được điều chỉnh theo tỷ lệ) ---
        qreal baseWidth = 2.0 / scaleFactor;
        QPen greyPen(QColor(200, 200, 200), baseWidth);
        QPen bluePen(QColor(0, 120, 255), baseWidth * 2);
        QPen redPen(QColor(255, 0, 0), baseWidth * 2);
        redPen.setStyle(Qt::DashDotLine);
        QPen thickBluePen(QColor(0, 120, 255), baseWidth * 3.5);
        QPen thickRedPen(QColor(255, 0, 0), baseWidth * 3.5);
        thickRedPen.setStyle(Qt::DashDotLine);

        // --- Vẽ tất cả các cạnh làm nền ---
        painter.setPen(greyPen);
        for (const auto &e : edges) {
            painter.drawLine(verts[e.u].position, verts[e.v].position);
        }

        // --- Logic vẽ đè màu theo thứ tự (giữ nguyên) ---
        std::unordered_map<int, int> edgeUsageCount;
        for (int i = 0; i < currentStep; ++i) {
            int augmentedId = route[i];
            int originalId = augmentedToOriginalIdMap.count(augmentedId) ? augmentedToOriginalIdMap.at(augmentedId) : augmentedId;
            if (originalId >= 0 && static_cast<size_t>(originalId) < edges.size()) {
                edgeUsageCount[originalId]++;
            }
        }
        
        for (const auto& pair : edgeUsageCount) {
            int eid = pair.first;
            int count = pair.second;
            const auto& e = edges[eid];
            painter.setPen(bluePen);
            painter.drawLine(verts[e.u].position, verts[e.v].position);
            if (count > 1) {
                painter.setPen(redPen);
                painter.drawLine(verts[e.u].position, verts[e.v].position);
            }
        }
        
        // --- Vẽ tô đậm cạnh hiện tại (giữ nguyên) ---
        if (currentStep < static_cast<int>(route.size())) {
            int augmentedId = route[currentStep];
            int originalId = augmentedToOriginalIdMap.count(augmentedId) ? augmentedToOriginalIdMap.at(augmentedId) : augmentedId;
            if (originalId >= 0 && static_cast<size_t>(originalId) < edges.size()) {
                const auto& e = edges[originalId];
                painter.setPen(edgeUsageCount.count(originalId) ? thickRedPen : thickBluePen);
                painter.drawLine(verts[e.u].position, verts[e.v].position);
            }
        }

        // --- Vẽ các đỉnh và nhãn ---
        qreal vertexRadius = 10.0 / scaleFactor;
        painter.setPen(QPen(Qt::black, baseWidth));
        painter.setBrush(Qt::white);
        QFont f = painter.font(); 
        f.setPointSizeF(13.0 / scaleFactor); // Font chữ cũng được điều chỉnh
        painter.setFont(f);
        
        for (const auto &v : verts) {
            painter.drawEllipse(v.position, vertexRadius, vertexRadius);
            painter.drawText(v.position + QPointF(-5 / scaleFactor, -15 / scaleFactor), v.name);
        }
        
        painter.restore(); // Khôi phục lại trạng thái painter
    }

private:
    Graph graph;
    std::vector<int> route;
    bool isPostman;
    std::vector<int> duplicateEdgeIds; 
    QImage frame;
    QTimer timer;
    int currentStep{0};
    std::unordered_map<int, int> augmentedToOriginalIdMap;
};