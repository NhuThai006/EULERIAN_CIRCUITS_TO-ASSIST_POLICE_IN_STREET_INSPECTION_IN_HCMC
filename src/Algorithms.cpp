#include "Algorithms.h"
#include "ChinesePostman.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <functional>

using namespace std;

// Initialize static storage
namespace Algorithms::detail {
    std::optional<EulerResult> lastEulerResult;
    std::optional<EulerResult> lastPostmanResult;
}

namespace {
struct EdgeUse { int id; bool used{false}; };

bool isEulerianOrSemi(const Graph &g, bool &isCycle, int &startVertex) {
    if (!g.isConnectedUndirected()) return false;
    int oddCount = 0;
    startVertex = -1; // Khởi tạo với giá trị không hợp lệ
    
    // Tìm đỉnh bậc lẻ đầu tiên
    for (const auto &v : g.getVertices()) {
        int d = g.degree(v.id);
        if (d % 2 == 1) {
            oddCount++;
            if (startVertex == -1) { // Chỉ gán đỉnh bậc lẻ đầu tiên
                startVertex = v.id;
            }
        }
    }
    
    if (oddCount == 0) { 
        // Eulerian cycle: có thể bắt đầu từ bất kỳ đỉnh nào có bậc > 0
        isCycle = true; 
        for (const auto &v : g.getVertices()) {
            if (g.degree(v.id) > 0) {
                startVertex = v.id;
                break;
            }
        }
        return true; 
    }
    if (oddCount == 2) { 
        // Eulerian path: phải bắt đầu từ đỉnh bậc lẻ
        isCycle = false; 
        return true; 
    }
    return false;
}
}

optional<EulerResult> Algorithms::findEulerTourHierholzer(const Graph &graph) {
    bool isCycle = false; int start = -1;
    if (!isEulerianOrSemi(graph, isCycle, start)) {
        detail::lastEulerResult = nullopt;
        return nullopt;
    }
    
    // Kiểm tra start vertex hợp lệ
    if (start == -1) {
        detail::lastEulerResult = nullopt;
        return nullopt;
    }

    // Build adjacency list and edge usage tracking
    unordered_map<int, vector<int>> adj = graph.adjacency();
    vector<bool> edgeUsed(graph.getEdges().size(), false);
    vector<int> path; // Final path as edge IDs
    
    // DFS function to find Euler path/cycle
    function<void(int)> dfs = [&](int u) {
        auto it = adj.find(u);
        if (it == adj.end()) return;
        
        // Create a copy of edges to iterate through
        vector<int> edges = it->second;
        for (int eid : edges) {
            if (!edgeUsed[eid]) {
                edgeUsed[eid] = true;
                const Edge &e = graph.getEdges()[eid];
                int v = (e.u == u) ? e.v : e.u;
                dfs(v);
                path.push_back(eid);
            }
        }
    };
    
    // Start DFS from the starting vertex
    dfs(start);
    
    // Reverse to get correct order
    reverse(path.begin(), path.end());
    
    // Verify all edges are used exactly once
    for (int i = 0; i < (int)graph.getEdges().size(); ++i) {
        if (!edgeUsed[i]) {
            return nullopt; // Some edges weren't used
        }
    }
    
    // Additional verification: check that each edge appears exactly once in path
    vector<int> edgeCount(graph.getEdges().size(), 0);
    for (int eid : path) {
        edgeCount[eid]++;
        if (edgeCount[eid] > 1) {
            return nullopt; // Edge used more than once
        }
    }
    
    EulerResult res;
    res.edgeOrder = std::move(path);
    res.isCycle = isCycle;
    
    detail::lastEulerResult = res;
    return detail::lastEulerResult;
}

vector<int> Algorithms::shortestPathVertices(const Graph &graph, int source, int target) {
    const int n = (int)graph.getVertices().size();
    vector<double> dist(n, numeric_limits<double>::infinity());
    vector<int> parent(n, -1);
    using QN = pair<double,int>;
    priority_queue<QN, vector<QN>, greater<QN>> pq;
    dist[source] = 0.0; pq.push({0.0, source});
    unordered_map<int, vector<int>> v2e = graph.adjacency();

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        if (u == target) break;
        auto it = v2e.find(u);
        if (it == v2e.end()) continue;
        for (int eid : it->second) {
            const Edge &e = graph.getEdges()[eid];
            int w = (e.u == u ? e.v : e.u);
            double nd = d + e.weight;
            if (nd < dist[w]) { dist[w] = nd; parent[w] = u; pq.push({nd, w}); }
        }
    }
    vector<int> path;
    if (parent[target] == -1 && source != target) return path;
    for (int v = target; v != -1; v = parent[v]) path.push_back(v);
    reverse(path.begin(), path.end());
    return path;
}

optional<EulerResult> Algorithms::approximateChinesePostman(const Graph &graph) {
    // Sử dụng thuật toán Chinese Postman tối ưu
    ChinesePostmanResult cppResult = ChinesePostmanOptimal::solve(graph);
    
    if (cppResult.edgeOrder.empty()) {
        detail::lastPostmanResult = nullopt;
        return nullopt;
    }
    
    // Chuyển đổi kết quả Chinese Postman thành EulerResult
    EulerResult result;
    result.edgeOrder = cppResult.edgeOrder;
    result.isCycle = cppResult.isCycle;
    
    // Tạo vertex order từ edge order
    result.vertexOrder = getVertexSequence(graph, result.edgeOrder);
    
    detail::lastPostmanResult = result;
    return result;
}

vector<int> Algorithms::getVertexSequence(const Graph& graph, const vector<int>& edgeOrder) {
    vector<int> vertexOrder;
    if (edgeOrder.empty()) return vertexOrder;
    
    // Tìm đỉnh bắt đầu từ cạnh đầu tiên
    const Edge &e0 = graph.getEdges()[edgeOrder[0]];
    int curr = e0.u;
    vertexOrder.push_back(curr);
    
    // Duyệt qua các cạnh để tạo thứ tự đỉnh
    for (int eid : edgeOrder) {
        if (eid < 0 || eid >= static_cast<int>(graph.getEdges().size())) continue;
        const Edge &e = graph.getEdges()[eid];
        int next = (e.u == curr) ? e.v : e.u;
        if (next < 0 || next >= static_cast<int>(graph.getVertices().size())) break;
        vertexOrder.push_back(next);
        curr = next;
    }
    
    return vertexOrder;
}

optional<EulerResult> Algorithms::getEulerSummary() {
    return detail::lastEulerResult;
}

optional<EulerResult> Algorithms::getPostmanSummary() {
    return detail::lastPostmanResult;
}

void Algorithms::clearResults() {
    detail::lastEulerResult = nullopt;
    detail::lastPostmanResult = nullopt;
}
