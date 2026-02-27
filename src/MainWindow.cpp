#include "MainWindow.h"
#include "ChinesePostman.h"
#include "Algorithms.h"
#include "GraphCanvas.h"
#include "AnimationWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QPainter>
#include <QStatusBar>
#include <QTextStream>
#include <QToolButton>
#include <QMenu>
#include <QtMath>
#include <QRegularExpression>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}

/* ============================================================
   UI SETUP
   ============================================================ */
void MainWindow::setupUi() {
    canvas = new GraphCanvas(this);
    setCentralWidget(canvas);
    setupToolbar();

    auto *central = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto banner = new QWidget(central);
    auto bannerLayout = new QVBoxLayout(banner);
    bannerLayout->setContentsMargins(0, 8, 0, 8);
    bannerLayout->setSpacing(2);

    auto lblTitle = new QLabel(
        "EULERIAN CIRCUITS TO ASSIST POLICE IN STREET INSPECTION IN HO CHI MINH CITY", banner);
    lblTitle->setAlignment(Qt::AlignCenter);
    lblTitle->setStyleSheet("font-weight:700; font-size:25px; color:#0078FF;");

    auto lblStudentsTitle = new QLabel("<b>Students:</b>", banner);
    lblStudentsTitle->setAlignment(Qt::AlignLeft);
    lblStudentsTitle->setStyleSheet("font-size:16px; color:#000000; margin-top:12px; margin-bottom:6px;");

    QString tableHtml =
        "<table border='1' cellspacing='0' cellpadding='8' style='border-collapse:collapse; font-size:14px; margin-left:auto; margin-right:auto;'>"
        "<tr><td style='min-width:110px; text-align:center'>24110117</td>"
        "<td style='min-width:320px; text-align:center'>Thai Ngoc Tam Nhu (leader)</td></tr>"
        "<tr><td style='text-align:tcenter'>24110148</td>"
        "<td style='text-align:center'>Duong Duy Vinh</td></tr></table>";
    auto lblTable = new QLabel(tableHtml, banner);
    lblTable->setAlignment(Qt::AlignCenter);
    lblTable->setStyleSheet("color:#000000;");

    bannerLayout->addWidget(lblTitle);
    bannerLayout->addStretch();
    bannerLayout->addWidget(lblStudentsTitle);
    bannerLayout->addWidget(lblTable);

    rootLayout->addWidget(banner);
    rootLayout->addWidget(canvas, 1);
    setCentralWidget(central);

    connect(canvas, &GraphCanvas::statusMessage,
            this, [this](const QString &m){ statusBar()->showMessage(m, 3000); });

    statusBar()->showMessage("Ready");
}

/* ============================================================
   TOOLBAR SETUP
   ============================================================ */
void MainWindow::setupToolbar() {
    QToolBar *tb = addToolBar("Main Toolbar");
    tb->setMovable(false);

    // === 📁 File & Export ===
    QToolButton *btnFile = new QToolButton(this);
    btnFile->setText("📁 File & Export");
    btnFile->setPopupMode(QToolButton::InstantPopup);
    QMenu *menuFile = new QMenu(this);

    // <-- THAY ĐỔI TRONG MENU FILE -->
    menuFile->addAction("📂 Open Location...", this, &MainWindow::openLocationDialog);
    menuFile->addAction("💾 Save Location As...", this, &MainWindow::saveLocation);
    menuFile->addSeparator();
    menuFile->addAction("Attach Adjacency Matrix", this, &MainWindow::onAttachFiles); // Đổi tên cho rõ
    menuFile->addSeparator();
    // <-- KẾT THÚC THAY ĐỔI -->

    menuFile->addAction("Export Image", this, &MainWindow::exportImage);
    menuFile->addAction("Export PDF", this, &MainWindow::exportPdf);
    menuFile->addAction("Export Matrix", this, &MainWindow::exportMatrix);
    btnFile->setMenu(menuFile);
    tb->addWidget(btnFile);

    // === 🧩 Graph Tools === (Giữ nguyên)
    QToolButton *btnGraph = new QToolButton(this);
    btnGraph->setText("🧩 Graph Tools");
    btnGraph->setPopupMode(QToolButton::InstantPopup);
    QMenu *menuGraph = new QMenu(this);
    menuGraph->addAction("Add Vertex", this, &MainWindow::setAddVertex);
    menuGraph->addAction("Add Edge", this, &MainWindow::setAddEdge);
    menuGraph->addAction("Move Vertex", this, &MainWindow::setMoveVertex);
    menuGraph->addAction("Erase Edge/Vertex", this, &MainWindow::setEraser);
    menuGraph->addSeparator();
    menuGraph->addAction("Clear All", this, [this]() {
        canvas->model().clear();
        canvas->clearRoute();
        statusBar()->showMessage("Cleared all graph data", 2000);
        canvas->update();
    });
    btnGraph->setMenu(menuGraph);
    tb->addWidget(btnGraph);

    // === ⚙️ Algorithms === (Giữ nguyên)
    QToolButton *btnAlgo = new QToolButton(this);
    btnAlgo->setText("⚙️ Algorithms");
    btnAlgo->setPopupMode(QToolButton::InstantPopup);
    QMenu *menuAlgo = new QMenu(this);
    menuAlgo->addAction("Euler", this, &MainWindow::runEuler);
    menuAlgo->addAction("Postman", this, &MainWindow::runPostman);
    btnAlgo->setMenu(menuAlgo);
    tb->addWidget(btnAlgo);

    // === 🗺 Map Tools === (Giữ nguyên)
    QToolButton *btnMap = new QToolButton(this);
    btnMap->setText("🗺 Map Tools");
    btnMap->setPopupMode(QToolButton::InstantPopup);
    QMenu *menuMap = new QMenu(this);
    menuMap->addAction("Import Map Background", this, &MainWindow::importBackground);
    menuMap->addAction("Clear Map Background", this, &MainWindow::clearBackground);
    btnMap->setMenu(menuMap);
    tb->addWidget(btnMap);

    // === 📘 Summary === (Giữ nguyên)
    QToolButton *btnSummary = new QToolButton(this);
    btnSummary->setText("📘 Summary");
    connect(btnSummary, &QToolButton::clicked, this, &MainWindow::onShowSummary);
    tb->addWidget(btnSummary);
}

/* ============================================================
   GRAPH TOOLS (Giữ nguyên)
   ============================================================ */
void MainWindow::setAddVertex()  { canvas->setMode(GraphCanvas::AddVertex);  statusBar()->showMessage("Mode: Add Vertex"); }
void MainWindow::setAddEdge()    { canvas->setMode(GraphCanvas::AddEdge);    statusBar()->showMessage("Mode: Add Edge"); }
void MainWindow::setMoveVertex() { canvas->setMode(GraphCanvas::MoveVertex); statusBar()->showMessage("Mode: Move Vertex"); }
void MainWindow::setEraser()     { canvas->setMode(GraphCanvas::Eraser);     statusBar()->showMessage("Mode: Eraser"); }

/* ============================================================
   ALGORITHMS (Giữ nguyên)
   ============================================================ */
void MainWindow::runEuler() {
    auto res = Algorithms::findEulerTourHierholzer(canvas->model());
    if (!res) {
        QMessageBox::information(this, "Euler", "No Euler path/cycle exists.");
        return;
    }
    canvas->setRoute(res->edgeOrder);
    statusBar()->showMessage(res->isCycle ? "Euler cycle found" : "Euler path found", 3000);
}

void MainWindow::runPostman() {
    auto res = ChinesePostmanOptimal::solve(canvas->model());
    if (res.edgeOrder.empty()) {
        QMessageBox::warning(this, "Postman", "Failed to compute route.");
        return;
    }
    int originalEdgeCount = static_cast<int>(canvas->model().getEdges().size());
    canvas->setRouteWithDuplicates(res.edgeOrder,
                                   res.duplicateEdgeIds,
                                   originalEdgeCount);
    statusBar()->showMessage("Postman route (optimal) computed", 3000);
}

/* ============================================================
   FILE & MAP TOOLS (Giữ nguyên)
   ============================================================ */
void MainWindow::exportImage() {
    QString file = QFileDialog::getSaveFileName(this, "Export Image", {}, "PNG Image (*.png)");
    if (file.isEmpty()) return;
    QImage img(canvas->size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    canvas->render(&p);
    img.save(file);
    statusBar()->showMessage("Exported image", 2000);
}

void MainWindow::exportPdf() {
    QString file = QFileDialog::getSaveFileName(this, "Export PDF", {}, "PDF (*.pdf)");
    if (file.isEmpty()) return;
    QImage img(canvas->size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::white);
    QPainter p(&img);
    canvas->render(&p);
    QString pngFile = file;
    pngFile.replace(".pdf", ".png");
    img.save(pngFile);
    statusBar()->showMessage("Exported as PNG (PDF not supported here)", 2000);
}

void MainWindow::importBackground() {
    QString file = QFileDialog::getOpenFileName(this, "Import Map Background", {}, "Images (*.png *.jpg *.jpeg *.bmp)");
    if (file.isEmpty()) return;
    QImageReader reader(file);
    reader.setAutoTransform(true);
    QImage img = reader.read();
    if (img.isNull()) {
        QMessageBox::warning(this, "Import", "Cannot read image.");
        return;
    }
    canvas->setBackgroundImage(img);
    statusBar()->showMessage("Map background imported", 2000);
}

void MainWindow::clearBackground() {
    canvas->clearBackgroundImage();
    statusBar()->showMessage("Map background cleared", 2000);
}

void MainWindow::exportMatrix() {
    const Graph &g = canvas->model();
    const auto &verts = g.getVertices();
    const auto &edges = g.getEdges();
    int n = static_cast<int>(verts.size());
    if (n == 0) {
        QMessageBox::information(this, "Export Matrix", "Graph is empty.");
        return;
    }

    QVector<QVector<int>> mat(n, QVector<int>(n, 0));
    for (const auto &e : edges) {
        if (e.u >= 0 && e.u < n && e.v >= 0 && e.v < n) {
            mat[e.u][e.v] = 1;
            mat[e.v][e.u] = 1;
        }
    }

    QString file = QFileDialog::getSaveFileName(this, "Save adjacency matrix", {}, "Text files (*.txt)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Matrix", "Cannot write file.");
        return;
    }

    QTextStream out(&f);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            out << mat[i][j];
            if (j + 1 < n) out << ' ';
        }
        out << '\n';
    }
    statusBar()->showMessage("Adjacency matrix exported", 3000);
}

void MainWindow::onAttachFiles() {
    QString file = QFileDialog::getOpenFileName(this, "Open adjacency matrix", {}, "Text files (*.txt);;All files (*.*)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Attach files", "Cannot open file.");
        return;
    }

    QVector<QVector<int>> mat;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        line.replace(',', ' ');
        QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        QVector<int> row;
        for (const QString &p : parts) row.push_back(p.toInt());
        if (!row.isEmpty()) mat.push_back(row);
    }

    if (mat.isEmpty() || mat.size() != mat[0].size()) {
        QMessageBox::warning(this, "Attach files", "Matrix must be square.");
        return;
    }

    Graph &g = canvas->model();
    g.clear();

    int n = mat.size();
    QSize sz = canvas->size();
    QPointF center(sz.width() / 2.0, sz.height() / 2.0);
    double radius = qMin(sz.width(), sz.height()) * 0.35;
    for (int i = 0; i < n; ++i) {
        double ang = (2 * M_PI * i) / n - M_PI / 2;
        QPointF pos(center.x() + radius * cos(ang), center.y() + radius * sin(ang));
        g.addVertex(pos);
    }
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            if (mat[i][j] != 0) g.addEdge(i, j);

    canvas->clearRoute();
    canvas->update();
    statusBar()->showMessage("Graph imported from adjacency matrix", 3000);
}

/* ============================================================
   SUMMARY + ANIMATION (Giữ nguyên)
   ============================================================ */
void MainWindow::onShowSummary() {
    const Graph &g = canvas->model();
    const auto &verts = g.getVertices();
    const auto &edges = g.getEdges();

    if (verts.empty()) {
        QMessageBox::information(this, "Summary", "The graph is empty.");
        return;
    }

    QString text;
    text += QString("📊 Graph Summary\n\nThis graph has %1 vertices and %2 edges:\n")
                .arg(verts.size()).arg(edges.size());
    for (const auto &e : edges)
        text += QString("• %1-%2 (Edge %3)\n")
                    .arg(verts[e.u].name)
                    .arg(verts[e.v].name)
                    .arg(e.id + 1);

    std::vector<int> route;
    bool isPostman = false;
    std::vector<int> duplicateIds;

    auto eulerRes = Algorithms::findEulerTourHierholzer(g);
    if (eulerRes && !eulerRes->edgeOrder.empty() && g.getEdges().size() == route.size()) {
        route = eulerRes->edgeOrder;
        text += "\n🧭 Euler Route:\n";
    } else {
        auto post = ChinesePostmanOptimal::solve(g);
        route = post.edgeOrder;
        duplicateIds = post.duplicateEdgeIds;
        isPostman = true;
        text += "\n📦 Chinese Postman Route:\n";
    }

    if (route.empty()) {
        QMessageBox::warning(this, "Summary", "No Euler/Postman route found.");
        return;
    }

    std::unordered_map<int, int> dupIdToOriginalId;
    int originalEdgeCount = static_cast<int>(edges.size());

    for (size_t i = 0; i < duplicateIds.size(); ++i) {
        int newId = originalEdgeCount + static_cast<int>(i);
        int originalId = duplicateIds[i];
        dupIdToOriginalId[newId] = originalId;
    }

    for (int eid : route) {
        int displayId = eid + 1;
        if (isPostman && dupIdToOriginalId.count(eid)) {
            int originalId = dupIdToOriginalId[eid];
            text += QString("%1 (dup of %2) ").arg(displayId).arg(originalId + 1);
        } else {
            text += QString::number(displayId) + " ";
        }
    }
    text += "\n\nVertex Degrees:\n";
    QVector<int> deg(verts.size(), 0);
    for (const auto &e : edges) {
        if (e.u >= 0 && e.u < (int)verts.size()) deg[e.u]++;
        if (e.v >= 0 && e.v < (int)verts.size()) deg[e.v]++;
    }
    for (int i = 0; i < deg.size(); ++i)
        text += QString("• %1 = %2\n").arg(verts[i].name).arg(deg[i]);

    std::vector<int> odd;
    for (int i = 0; i < deg.size(); ++i)
        if (deg[i] % 2 == 1) odd.push_back(i);

    text += "\nEulerian Analysis:\n";
    if (odd.empty())
        text += "✅ All vertices have even degree → Eulerian circuit exists.\n";
    else if (odd.size() == 2)
        text += QString("⚠️ Two vertices (%1, %2) are odd → Eulerian path exists.\n")
                    .arg(verts[odd[0]].name, verts[odd[1]].name);
    else
        text += QString("❌ %1 vertices are odd → No Euler path.\n").arg(odd.size());

    if (!duplicateIds.empty()) {
        text += "\n🔴 Duplicated Edges:\n";
        for (int eid : duplicateIds) {
            if (eid >= 0 && static_cast<size_t>(eid) < edges.size()) {
                 text += QString("• %1-%2 (Edge %3)\n")
                        .arg(verts[edges[eid].u].name)
                        .arg(verts[edges[eid].v].name)
                        .arg(eid + 1);
            }
        }
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Summary");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(text);

    QPushButton *btnNext = msgBox.addButton(tr("Next"), QMessageBox::AcceptRole);
    msgBox.exec();

    if (msgBox.clickedButton() == btnNext) {
        auto *anim = new AnimationWindow(g, route, isPostman, duplicateIds, this);
        anim->show();
    }
}


// ===================================================================
// === BẮT ĐẦU CÁC HÀM MỚI CHO VIỆC TẢI VÀ LƯU ĐỊA ĐIỂM ===
// ===================================================================

/**
 * @brief Mở một dialog để người dùng chọn địa điểm có sẵn để tải.
 */
void MainWindow::openLocationDialog() {
    QDir locationsDir(QCoreApplication::applicationDirPath() + "/locations");
    if (!locationsDir.exists()) {
        QMessageBox::warning(this, "Open Location", "The 'locations' directory does not exist.");
        return;
    }

    QStringList locationNames = locationsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    if (locationNames.isEmpty()) {
        QMessageBox::information(this, "Open Location", "No locations found in the 'locations' directory.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Select a Location");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    for (const QString &locName : locationNames) {
        QPushButton *btn = new QPushButton(locName, &dialog);
        layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, this, [this, &dialog, locationsDir, locName]() {
            loadLocationFromPath(locationsDir.filePath(locName));
            dialog.accept();
        });
    }

    dialog.exec();
}

/**
 * @brief Tải dữ liệu đồ thị (ảnh nền, tọa độ, ma trận) từ một đường dẫn thư mục.
 * @param dirPath Đường dẫn đến thư mục của địa điểm.
 */
void MainWindow::loadLocationFromPath(const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        statusBar()->showMessage("Error: Directory does not exist.", 3000);
        return;
    }

    canvas->model().clear();
    canvas->clearRoute();
    canvas->clearBackgroundImage();

    QStringList imageFilters = {"background.png", "background.jpg", "background.jpeg", "background.bmp"};
    for (const QString& filter : imageFilters) {
        QString backgroundPath = dir.filePath(filter);
        if (QFile::exists(backgroundPath)) {
            canvas->setBackgroundImage(QImage(backgroundPath));
            break;
        }
    }

    Graph &g = canvas->model();
    QString coordsPath = dir.filePath("coords.txt");
    QFile coordsFile(coordsPath);
    if (coordsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&coordsFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() == 2) {
                g.addVertex(QPointF(parts[0].toDouble(), parts[1].toDouble()));
            }
        }
        coordsFile.close();
    } else {
         statusBar()->showMessage("Warning: coords.txt not found.", 3000);
    }

    QString matrixPath = dir.filePath("matrix.txt");
    QFile matrixFile(matrixPath);
    if (matrixFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&matrixFile);
        QVector<QVector<int>> mat;
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            QVector<int> row;
            for (const QString &p : parts) row.push_back(p.toInt());
            if (!row.isEmpty()) mat.push_back(row);
        }
        matrixFile.close();

        if (!mat.isEmpty()) {
            int n = g.getVertices().size();
            for (int i = 0; i < n; ++i) {
                for (int j = i + 1; j < n; ++j) {
                    if (i < mat.size() && j < mat[i].size() && mat[i][j] != 0) {
                        g.addEdge(i, j);
                    }
                }
            }
        }
    } else {
        statusBar()->showMessage("Warning: matrix.txt not found.", 3000);
    }

    canvas->update();
    statusBar()->showMessage("Loaded location: " + dir.dirName(), 3000);
}


/**
 * @brief Lưu trạng thái đồ thị hiện tại (ảnh nền, tọa độ, ma trận) ra một thư mục mới.
 */
void MainWindow::saveLocation() {
    bool ok;
    QString locName = QInputDialog::getText(this, "Save Location",
                                            "Enter a name for the new location:", QLineEdit::Normal, "", &ok);
    if (!ok || locName.isEmpty()) {
        return;
    }

    QDir baseDir(QCoreApplication::applicationDirPath());
    baseDir.mkpath("locations");
    QString targetPath = baseDir.filePath("locations/" + locName);
    if (!QDir().mkpath(targetPath)) {
        QMessageBox::critical(this, "Error", "Could not create directory for the location.");
        return;
    }

    const QImage& bg = canvas->getBackgroundImage();
    if (!bg.isNull()) {
        bg.save(targetPath + "/background.png", "PNG");
    }

    const Graph &g = canvas->model();
    QFile coordsFile(targetPath + "/coords.txt");
    if (coordsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&coordsFile);
        for (const auto &vertex : g.getVertices()) {
            out << vertex.position.x() << " " << vertex.position.y() << "\n";
        }
        coordsFile.close();
    } else {
        QMessageBox::warning(this, "Warning", "Could not save coordinates file.");
    }

    QFile matrixFile(targetPath + "/matrix.txt");
    if (matrixFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&matrixFile);
        int n = g.getVertices().size();
        QVector<QVector<int>> mat(n, QVector<int>(n, 0));
        for (const auto &e : g.getEdges()) {
            mat[e.u][e.v] = 1;
            mat[e.v][e.u] = 1;
        }

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                out << mat[i][j] << (j == n - 1 ? "" : " ");
            }
            out << "\n";
        }
        matrixFile.close();
    } else {
        QMessageBox::warning(this, "Warning", "Could not save matrix file.");
    }

    statusBar()->showMessage("Location '" + locName + "' saved successfully!", 3000);
}