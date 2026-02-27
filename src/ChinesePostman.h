#pragma once
#include <vector>
#include "Graph.h"
#include "Algorithms.h"

struct ChinesePostmanResult {
    std::vector<int> edgeOrder;
    std::vector<int> duplicateEdgeIds;
    std::vector<int> vertexOrder;
    bool isCycle{false};

    // 🔹 Đồ thị có chứa thông tin cạnh duplicate
    Graph graphWithDuplicates;
};

class ChinesePostmanOptimal {
public:
    static ChinesePostmanResult solve(const Graph &g);
};
