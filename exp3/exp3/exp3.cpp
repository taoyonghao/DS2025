#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <climits>
#include <algorithm>
#include <string>
#include <set>
#include <stack>
#include <utility>

using namespace std;

const int INF = 0x3f3f3f3f;  // 表示无穷大（无直接边）

// 边结构体（双连通分量专用）
struct Edge {
    string u;
    string v;
    Edge(string u_, string v_) : u(u_), v(v_) {}
    // 重载==，用于边去重
    bool operator==(const Edge& other) const {
        return (u == other.u && v == other.v) || (u == other.v && v == other.u);
    }
};

// 图类
class Graph {
private:
    vector<string> vertices;                 // 顶点列表
    map<string, int> vIndex;                 // 顶点到索引的映射
    vector<vector<int>> adjMatrix;           // 邻接矩阵
    int vertexNum;                           // 顶点数量
    bool isDirected;                         // 是否为有向图
    stack<Edge> edgeStack;                   // 双连通分量边栈
    vector<vector<Edge>> biconnectedComps;   // 双连通分量集合

    // Tarjan辅助函数（双连通分量+关节点，修复边重复问题）
    void tarjanDFS(int u, vector<bool>& visited, vector<int>& disc, vector<int>& low,
        vector<int>& parent, set<string>& articulationPoints, int& time) {
        int children = 0;
        visited[u] = true;
        disc[u] = low[u] = ++time;

        for (int v = 0; v < vertexNum; v++) {
            if (adjMatrix[u][v] == INF || u == v) {
                continue;  // 无边或自环，跳过
            }
            Edge e(vertices[u], vertices[v]);

            if (!visited[v]) {
                edgeStack.push(e);
                parent[v] = u;
                children++;
                tarjanDFS(v, visited, disc, low, parent, articulationPoints, time);

                // 更新low值
                low[u] = min(low[u], low[v]);

                // 触发双连通分量的条件
                bool isArticulation = false;
                if (parent[u] == -1 && children > 1) {
                    isArticulation = true;
                    articulationPoints.insert(vertices[u]);
                }
                if (parent[u] != -1 && low[v] >= disc[u]) {
                    isArticulation = true;
                    articulationPoints.insert(vertices[u]);
                }

                // 提取双连通分量
                if (isArticulation) {
                    vector<Edge> comp;
                    while (!edgeStack.empty()) {
                        Edge top = edgeStack.top();
                        edgeStack.pop();
                        comp.push_back(top);
                        if ((top.u == e.u && top.v == e.v) || (top.u == e.v && top.v == e.u)) {
                            break;
                        }
                    }
                    biconnectedComps.push_back(comp);
                }
            }
            // 回边（且不是父节点，且未被处理过）
            else if (v != parent[u] && disc[v] < disc[u]) {
                edgeStack.push(e);
                low[u] = min(low[u], disc[v]);
            }
        }
    }

    // 去重双连通分量中的重复边
    vector<vector<Edge>> removeDuplicateEdges(const vector<vector<Edge>>& comps) {
        vector<vector<Edge>> res;
        for (const auto& comp : comps) {
            vector<Edge> uniqueComp;
            for (const auto& e : comp) {
                bool exists = false;
                for (const auto& ue : uniqueComp) {
                    if (ue == e) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    uniqueComp.push_back(e);
                }
            }
            if (!uniqueComp.empty()) {
                res.push_back(uniqueComp);
            }
        }
        return res;
    }

public:
    // 构造函数（兼容C++11，移除结构化绑定依赖）
    Graph(const vector<string>& vs, bool directed = false) {
        vertices = vs;
        isDirected = directed;
        vertexNum = vs.size();
        // 初始化顶点索引
        for (int i = 0; i < vertexNum; ++i) {
            vIndex[vertices[i]] = i;
        }
        // 初始化邻接矩阵
        adjMatrix.resize(vertexNum, vector<int>(vertexNum, INF));
        for (int i = 0; i < vertexNum; ++i) {
            adjMatrix[i][i] = 0;  // 自身到自身权值为0
        }
    }

    // 添加边（增加合法性校验）
    void addEdge(const string& v1, const string& v2, int weight = 1) {
        if (vIndex.find(v1) == vIndex.end() || vIndex.find(v2) == vIndex.end()) {
            cerr << "错误：顶点" << (vIndex.count(v1) == 0 ? v1 : v2) << "不存在！" << endl;
            return;
        }
        if (weight < 0) {
            cerr << "错误：边权不能为负数！" << endl;
            return;
        }
        int i = vIndex[v1];
        int j = vIndex[v2];
        adjMatrix[i][j] = weight;
        if (!isDirected) {
            adjMatrix[j][i] = weight;
        }
    }

    // 输出邻接矩阵（优化格式）
    void showAdjMatrix() {
        cout << "图的邻接矩阵（INF表示无直接连接）：" << endl;
        // 打印表头
        cout << "    ";
        for (size_t i = 0; i < vertices.size(); ++i) {
            cout << vertices[i] << "   ";
        }
        cout << endl;
        // 打印每行
        for (int i = 0; i < vertexNum; ++i) {
            cout << vertices[i] << "   ";
            for (int j = 0; j < vertexNum; ++j) {
                if (adjMatrix[i][j] == INF) {
                    cout << "INF ";
                }
                else {
                    printf("%-3d", adjMatrix[i][j]);  // 对齐输出
                }
            }
            cout << endl;
        }
    }

    // 广度优先搜索BFS（修复访问逻辑）
    vector<string> BFS(const string& start) {
        vector<string> res;
        if (vIndex.find(start) == vIndex.end()) {
            cerr << "错误：起始顶点" << start << "不存在！" << endl;
            return res;
        }
        int startIdx = vIndex[start];
        vector<bool> visited(vertexNum, false);
        queue<int> q;

        q.push(startIdx);
        visited[startIdx] = true;

        while (!q.empty()) {
            int cur = q.front();
            q.pop();
            res.push_back(vertices[cur]);

            // 遍历所有邻接顶点（按顶点顺序，保证结果可复现）
            for (int i = 0; i < vertexNum; ++i) {
                if (adjMatrix[cur][i] != INF && !visited[i] && cur != i) {
                    visited[i] = true;
                    q.push(i);
                }
            }
        }
        return res;
    }

    // DFS递归辅助
    void DFSHelper(int cur, vector<bool>& visited, vector<string>& res) {
        visited[cur] = true;
        res.push_back(vertices[cur]);
        // 按顶点顺序遍历，保证结果稳定
        for (int i = 0; i < vertexNum; ++i) {
            if (adjMatrix[cur][i] != INF && !visited[i] && cur != i) {
                DFSHelper(i, visited, res);
            }
        }
    }

    // 深度优先搜索DFS
    vector<string> DFS(const string& start) {
        vector<string> res;
        if (vIndex.find(start) == vIndex.end()) {
            cerr << "错误：起始顶点" << start << "不存在！" << endl;
            return res;
        }
        int startIdx = vIndex[start];
        vector<bool> visited(vertexNum, false);
        DFSHelper(startIdx, visited, res);
        return res;
    }

    // Dijkstra最短路径（修复不可达判断）
    map<string, int> dijkstra(const string& start) {
        map<string, int> res;
        if (vIndex.find(start) == vIndex.end()) {
            cerr << "错误：起始顶点" << start << "不存在！" << endl;
            return res;
        }
        int startIdx = vIndex[start];
        vector<int> dist(vertexNum, INF);
        vector<bool> visited(vertexNum, false);

        dist[startIdx] = 0;

        for (int i = 0; i < vertexNum; ++i) {
            // 找未访问的最小距离顶点
            int minDist = INF;
            int u = -1;
            for (int j = 0; j < vertexNum; ++j) {
                if (!visited[j] && dist[j] < minDist) {
                    minDist = dist[j];
                    u = j;
                }
            }
            if (u == -1) {
                break;  // 无更多可达顶点
            }
            visited[u] = true;

            // 更新邻接顶点距离
            for (int v = 0; v < vertexNum; ++v) {
                if (!visited[v] && adjMatrix[u][v] != INF) {
                    if (dist[u] != INF && dist[u] + adjMatrix[u][v] < dist[v]) {
                        dist[v] = dist[u] + adjMatrix[u][v];
                    }
                }
            }
        }

        // 封装结果
        for (int i = 0; i < vertexNum; ++i) {
            if (dist[i] == INF) {
                res[vertices[i]] = -1;  // -1表示不可达
            }
            else {
                res[vertices[i]] = dist[i];
            }
        }
        return res;
    }

    // Prim最小生成树（兼容C++11，替换结构化绑定）
    pair<vector<pair<pair<string, string>, int>>, int> prim(const string& start) {
        vector<pair<pair<string, string>, int>> mstEdges;
        int totalWeight = 0;
        if (vIndex.find(start) == vIndex.end()) {
            cerr << "错误：起始顶点" << start << "不存在！" << endl;
            return make_pair(mstEdges, -1);
        }
        int startIdx = vIndex[start];
        vector<int> key(vertexNum, INF);
        vector<bool> inMST(vertexNum, false);
        vector<int> parent(vertexNum, -1);

        key[startIdx] = 0;

        for (int i = 0; i < vertexNum; ++i) {
            // 找最小key的顶点
            int minKey = INF;
            int u = -1;
            for (int j = 0; j < vertexNum; ++j) {
                if (!inMST[j] && key[j] < minKey) {
                    minKey = key[j];
                    u = j;
                }
            }
            if (u == -1) {
                break;
            }
            inMST[u] = true;
            totalWeight += key[u];

            // 更新邻接顶点key
            for (int v = 0; v < vertexNum; ++v) {
                if (!inMST[v] && adjMatrix[u][v] != INF && adjMatrix[u][v] < key[v]) {
                    key[v] = adjMatrix[u][v];
                    parent[v] = u;
                }
            }
        }

        // 构建生成树边
        for (int i = 0; i < vertexNum; ++i) {
            if (parent[i] != -1) {
                string v1 = vertices[parent[i]];
                string v2 = vertices[i];
                int w = adjMatrix[parent[i]][i];
                mstEdges.push_back(make_pair(make_pair(v1, v2), w));
            }
        }
        return make_pair(mstEdges, totalWeight);
    }

    // 重置双连通分量存储
    void resetBCC() {
        biconnectedComps.clear();
        while (!edgeStack.empty()) {
            edgeStack.pop();
        }
    }

    // 查找双连通分量和关节点（修复连通分量遗漏问题）
    pair<vector<vector<Edge>>, set<string>> findBCCAndArticulation(const string& start) {
        resetBCC();
        set<string> articulationPoints;
        if (vIndex.find(start) == vIndex.end()) {
            cerr << "错误：起始顶点" << start << "不存在！" << endl;
            return make_pair(biconnectedComps, articulationPoints);
        }
        int startIdx = vIndex[start];
        vector<bool> visited(vertexNum, false);
        vector<int> disc(vertexNum, 0);
        vector<int> low(vertexNum, 0);
        vector<int> parent(vertexNum, -1);
        int time = 0;

        // 处理起始连通分量
        tarjanDFS(startIdx, visited, disc, low, parent, articulationPoints, time);
        // 处理栈中剩余边
        if (!edgeStack.empty()) {
            vector<Edge> comp;
            while (!edgeStack.empty()) {
                Edge top = edgeStack.top();
                edgeStack.pop();
                comp.push_back(top);
            }
            if (!comp.empty()) {
                biconnectedComps.push_back(comp);
            }
        }
        // 处理其他连通分量
        for (int i = 0; i < vertexNum; ++i) {
            if (!visited[i]) {
                tarjanDFS(i, visited, disc, low, parent, articulationPoints, time);
                // 弹出剩余边
                vector<Edge> comp;
                while (!edgeStack.empty()) {
                    Edge top = edgeStack.top();
                    edgeStack.pop();
                    comp.push_back(top);
                }
                if (!comp.empty()) {
                    biconnectedComps.push_back(comp);
                }
            }
        }
        // 去重
        vector<vector<Edge>> uniqueComps = removeDuplicateEdges(biconnectedComps);
        return make_pair(uniqueComps, articulationPoints);
    }

    // 输出双连通分量（优化格式）
    void printBCC(const vector<vector<Edge>>& comps) {
        cout << "双连通分量总数：" << comps.size() << endl;
        for (size_t i = 0; i < comps.size(); ++i) {
            cout << "分量" << (i + 1) << "：";
            for (size_t j = 0; j < comps[i].size(); ++j) {
                if (j > 0) {
                    cout << ", ";
                }
                cout << "(" << comps[i][j].u << "," << comps[i][j].v << ")";
            }
            cout << endl;
        }
    }
};

// 辅助函数：输出遍历顺序
void printTraversal(const vector<string>& order, const string& name) {
    cout << name << "遍历顺序：";
    for (size_t i = 0; i < order.size(); ++i) {
        if (i > 0) {
            cout << " -> ";
        }
        cout << order[i];
    }
    cout << endl;
}

int main() {
    // ========== （1）图1的邻接矩阵（严格匹配题目拓扑） ==========
    cout << "===== 任务1：输出图1邻接矩阵 =====" << endl;
    // 图1顶点（根据题目图修正：A/B/D/E/G/H，补充缺失顶点）
    vector<string> v1 = { "A", "B", "D", "E", "G", "H" };
    Graph g1(v1);
    // 严格对应题目图1的边权（数字均来自文档）
    g1.addEdge("A", "B", 2);   // A-B权2
    g1.addEdge("A", "D", 7);   // A-D权7
    g1.addEdge("B", "E", 9);   // B-E权9
    g1.addEdge("B", "H", 3);   // B-H权3
    g1.addEdge("D", "E", 13);  // D-E权13
    g1.addEdge("D", "G", 6);   // D-G权6
    g1.addEdge("E", "G", 11);  // E-G权11
    g1.addEdge("E", "H", 1);   // E-H权1
    g1.addEdge("G", "H", 2);   // G-H权2
    g1.showAdjMatrix();
    cout << endl;

    // ========== （2）图1的BFS和DFS（A为起点） ==========
    cout << "===== 任务2：图1从A出发的BFS和DFS =====" << endl;
    vector<string> bfs1 = g1.BFS("A");
    printTraversal(bfs1, "BFS");
    vector<string> dfs1 = g1.DFS("A");
    printTraversal(dfs1, "DFS");
    cout << endl;

    // ========== （3）图1的最短路径和最小生成树（A为起点） ==========
    cout << "===== 任务3：图1从A出发的最短路径和最小生成树 =====" << endl;
    // 最短路径
    map<string, int> dist1 = g1.dijkstra("A");
    cout << "Dijkstra最短路径（-1=不可达）：" << endl;
    for (auto& p : dist1) {
        cout << "A -> " << p.first << "：";
        if (p.second == -1) {
            cout << "不可达";
        }
        else {
            cout << p.second;
        }
        cout << endl;
    }
    // 最小生成树
    pair<vector<pair<pair<string, string>, int>>, int> mstRes = g1.prim("A");
    vector<pair<pair<string, string>, int>> mst1 = mstRes.first;
    int totalW = mstRes.second;
    cout << "\nPrim最小生成树的边：" << endl;
    for (auto& e : mst1) {
        cout << e.first.first << " - " << e.first.second << "（权值：" << e.second << "）" << endl;
    }
    cout << "最小生成树总权值：" << totalW << endl;
    cout << endl;

    // ========== （4）图2的双连通分量和关节点（不同起点验证） ==========
    cout << "===== 任务4：图2不同起点的双连通分量和关节点 =====" << endl;
    // 图2顶点（A-L共12个）
    vector<string> v2 = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L" };
    Graph g2(v2);
    // 图2的边（按题目拓扑连接）
    g2.addEdge("A", "B", 1);
    g2.addEdge("B", "C", 1);
    g2.addEdge("B", "D", 1);
    g2.addEdge("B", "E", 1);
    g2.addEdge("E", "F", 1);
    g2.addEdge("E", "G", 1);
    g2.addEdge("G", "H", 1);
    g2.addEdge("E", "H", 1);
    g2.addEdge("I", "J", 1);
    g2.addEdge("J", "K", 1);
    g2.addEdge("K", "L", 1);
    g2.addEdge("J", "L", 1);

    // 测试3个不同起点
    vector<string> starts = { "A", "B", "E" };
    for (string s : starts) {
        cout << "\n--- 以" << s << "为起点 ---" << endl;
        pair<vector<vector<Edge>>, set<string>> bccRes = g2.findBCCAndArticulation(s);
        vector<vector<Edge>> bcc = bccRes.first;
        set<string> artPoints = bccRes.second;

        // 输出双连通分量
        g2.printBCC(bcc);
        // 输出关节点
        cout << "关节点：";
        if (artPoints.empty()) {
            cout << "无";
        }
        else {
            for (string p : artPoints) {
                cout << p << " ";
            }
        }
        cout << endl;
    }

    return 0;
}