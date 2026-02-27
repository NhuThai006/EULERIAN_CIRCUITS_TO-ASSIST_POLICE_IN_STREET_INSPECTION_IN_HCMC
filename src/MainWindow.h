#pragma once
#include <QMainWindow>
#include <QToolButton>
#include <QMenu>
#include <QToolBar>
#include <QLabel>
#include <QVBoxLayout>
#include "GraphCanvas.h"
#include "Algorithms.h"
#include "ChinesePostman.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QTextStream>
#include <QDir>         // <-- THÊM MỚI
#include <QInputDialog> // <-- THÊM MỚI


class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    // === Graph Tools ===
    void setAddVertex();
    void setAddEdge();
    void setMoveVertex();
    void setEraser();

    // === Algorithms ===
    void runEuler();
    void runPostman();

    // === File & Map Tools ===
    void exportImage();
    void exportPdf();
    void importBackground();
    void clearBackground();
    void exportMatrix();
    void onAttachFiles();

    // === Summary ===
    void onShowSummary();
    
    // === CÁC SLOT MỚI CHO QUẢN LÝ ĐỊA ĐIỂM ===
    void openLocationDialog();
    void saveLocation();

private:
    GraphCanvas *canvas{nullptr};

    void setupUi();
    void setupToolbar();

    // === HÀM HỖ TRỢ MỚI ===
    void loadLocationFromPath(const QString &dirPath);
};