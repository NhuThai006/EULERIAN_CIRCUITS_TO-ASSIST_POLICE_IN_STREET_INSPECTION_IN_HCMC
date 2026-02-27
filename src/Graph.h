#pragma once
#include <QPointF>
#include <QString>
#include <vector>
#include <unordered_map>

struct Vertex {
    int id;
    QString name;
    QPointF position;
};

struct Edge {
    int id;
    int u, v;
    double weight;
    bool directed{false};
};

class Graph {
private:
    std::vector<Vertex> vertices;
    std::vector<Edge> edges;
    std::vector<int> duplicateEdgeIds;  // ✅ lưu ID các cạnh gốc có duplicate

public:
    Graph() = default;

    // -----------------------------
    // BASIC GRAPH MODIFICATION
    // -----------------------------
    int addVertex(const QPointF &pos, const QString &name = QString()) {
        Vertex v;
        v.id = static_cast<int>(vertices.size());
        v.position = pos;
        v.name = name.isEmpty() ? QString(QChar('A' + v.id)) : name;
        vertices.push_back(v);
        return v.id;
    }

    int addEdge(int u, int v, double weight = 1.0, bool directed = false) {
        Edge e;
        e.id = static_cast<int>(edges.size());
        e.u = u;
        e.v = v;
        e.weight = weight;
        e.directed = directed;
        edges.push_back(e);
        return e.id;
    }

    void removeEdge(int edgeId) {
        if (edgeId < 0 || edgeId >= static_cast<int>(edges.size())) return;
        edges.erase(edges.begin() + edgeId);
    }

    void removeVertex(int vertexId) {
        if (vertexId < 0 || vertexId >= static_cast<int>(vertices.size())) return;
        vertices.erase(vertices.begin() + vertexId);
    }

    void clear() {
        vertices.clear();
        edges.clear();
        duplicateEdgeIds.clear();
    }

    void moveVertex(int id, const QPointF &pos) {
        if (id >= 0 && id < static_cast<int>(vertices.size()))
            vertices[id].position = pos;
    }

    // -----------------------------
    // GETTERS
    // -----------------------------
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<Edge>& getEdges() const { return edges; }
    std::vector<Edge>& getEdges() { return edges; }

    // -----------------------------
    // GRAPH ANALYSIS FUNCTIONS
    // -----------------------------
    std::vector<int> neighbors(int u) const {
        std::vector<int> nb;
        for (const auto &e : edges) {
            if (e.u == u) nb.push_back(e.v);
            else if (!e.directed && e.v == u) nb.push_back(e.u);
        }
        return nb;
    }

    int degree(int u) const {
        int d = 0;
        for (const auto &e : edges) {
            if (e.u == u || (!e.directed && e.v == u))
                d++;
        }
        return d;
    }

    // -----------------------------
    // CONNECTIVITY (DFS)
    // -----------------------------
    void dfs(int start, std::vector<bool> &visited) const {
        visited[start] = true;
        for (int v : neighbors(start))
            if (!visited[v])
                dfs(v, visited);
    }

    bool isConnectedUndirected() const {
        if (vertices.empty()) return true;
        std::vector<bool> visited(vertices.size(), false);

        // tìm 1 đỉnh có bậc > 0 làm điểm bắt đầu
        int start = -1;
        for (size_t i = 0; i < vertices.size(); ++i)
            if (degree(i) > 0) { start = i; break; }

        if (start == -1) return true;  // không có cạnh nào

        dfs(start, visited);

        for (size_t i = 0; i < vertices.size(); ++i)
            if (!visited[i] && degree(i) > 0)
                return false;
        return true;
    }

    // -----------------------------
    // ADJACENCY REPRESENTATION
    // -----------------------------
    std::unordered_map<int, std::vector<int>> adjacency() const {
        std::unordered_map<int, std::vector<int>> adj;
        for (const auto &e : edges) {
            adj[e.u].push_back(e.id);
            adj[e.v].push_back(e.id);
        }
        return adj;
    }

    // -----------------------------
    // DUPLICATE EDGE MANAGEMENT
    // -----------------------------
    void setDuplicateEdgeIds(const std::vector<int>& ids) { duplicateEdgeIds = ids; }
    const std::vector<int>& getDuplicateEdgeIds() const { return duplicateEdgeIds; }
};