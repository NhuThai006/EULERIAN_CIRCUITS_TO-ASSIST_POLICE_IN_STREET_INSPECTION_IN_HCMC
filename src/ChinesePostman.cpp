#include "ChinesePostman.h"
#include "Algorithms.h"
#include <unordered_map>
#include <set>
#include <queue>
#include <limits>
#include <QDebug>

/* ============================================================
   Hàm solve() — Chinese Postman Problem cho đồ thị vô hướng
   ============================================================ */
ChinesePostmanResult ChinesePostmanOptimal::solve(const Graph &g) {
    ChinesePostmanResult result;
    const auto &verts = g.getVertices();
    const auto &edges = g.getEdges();
    if (verts.empty() || edges.empty()) return result;

    // --- B1. Tính bậc các đỉnh ---
    std::vector<int> degree(verts.size(), 0);
    for (const auto &e : edges) {
        if (e.u >= 0 && e.v >= 0 &&
            e.u < (int)verts.size() && e.v < (int)verts.size()) {
            degree[e.u]++;
            degree[e.v]++;
        }
    }

    // --- B2. Tìm các đỉnh bậc lẻ ---
    std::vector<int> oddVertices;
    for (size_t i = 0; i < degree.size(); ++i)
        if (degree[i] % 2 != 0)
            oddVertices.push_back((int)i);

    // Nếu không có đỉnh bậc lẻ → Eulerian circuit
    if (oddVertices.empty()) {
        auto euler = Algorithms::findEulerTourHierholzer(g);
        if (euler) result.edgeOrder = euler->edgeOrder;
        return result;
    }

    // --- B3. Tính khoảng cách ngắn nhất giữa các đỉnh lẻ ---
    int n = oddVertices.size();
    std::vector<std::vector<double>> dist(n, std::vector<double>(n, 1e9));
    std::vector<std::vector<std::vector<int>>> path(n, std::vector<std::vector<int>>(n));

    for (int i = 0; i < n; ++i) {
        int start = oddVertices[i];
        // Dijkstra
        std::vector<double> d(verts.size(), 1e9);
        std::vector<int> parent(verts.size(), -1);
        d[start] = 0;
        using P = std::pair<double, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> pq;
        pq.push({0, start});
        while (!pq.empty()) {
            auto [du, u] = pq.top(); pq.pop();
            if (du != d[u]) continue;
            for (const auto &e : edges) {
                int v = (e.u == u ? e.v : (e.v == u ? e.u : -1));
                if (v == -1) continue;
                double w = e.weight;
                if (d[v] > d[u] + w) {
                    d[v] = d[u] + w;
                    parent[v] = u;
                    pq.push({d[v], v});
                }
            }
        }
        for (int j = 0; j < n; ++j) {
            int end = oddVertices[j];
            dist[i][j] = d[end];
            if (d[end] < 1e9) {
                // reconstruct path
                int v = end;
                std::vector<int> rev;
                while (v != -1) {
                    rev.push_back(v);
                    v = parent[v];
                }
                std::reverse(rev.begin(), rev.end());
                path[i][j] = rev;
            }
        }
    }

    // --- B4. Tìm ghép đôi tối ưu (min weight matching) ---
    double bestCost = 1e9;
    std::vector<int> bestMatch;
    std::vector<int> perm(n);
    for (int i = 0; i < n; ++i) perm[i] = i;

    do {
        double cost = 0;
        for (int i = 0; i < n; i += 2)
            cost += dist[perm[i]][perm[i + 1]];
        if (cost < bestCost) {
            bestCost = cost;
            bestMatch = perm;
        }
    } while (std::next_permutation(perm.begin(), perm.end()));

    // --- B5. Tạo đồ thị mới có thêm cạnh duplicated ---
    Graph augmented = g;
    for (int i = 0; i < n; i += 2) {
        int uIdx = bestMatch[i];
        int vIdx = bestMatch[i + 1];
        const auto &p = path[uIdx][vIdx];
        for (size_t k = 0; k + 1 < p.size(); ++k) {
            int u = p[k];
            int v = p[k + 1];
            // tìm cạnh gốc giữa u-v
            for (const auto &e : edges) {
                if ((e.u == u && e.v == v) || (e.u == v && e.v == u)) {
                    augmented.addEdge(u, v, e.weight);
                    result.duplicateEdgeIds.push_back(e.id); // ✅ lưu id cạnh gốc bị duplicate
                    break;
                }
            }
        }
    }

    // --- B6. Tìm chu trình Euler trên đồ thị augmented ---
    auto euler = Algorithms::findEulerTourHierholzer(augmented);
    if (euler)
        result.edgeOrder = euler->edgeOrder;

    return result;
}
