#include <iostream>
#include <vector>
#include <queue>
#include <map>
using namespace std;

using Graph = vector<vector<int>>;
using Edge = pair<int,int>;

// s から各頂点への最短距離を計算
vector<int> shortestDistanceOne(const Graph& G, int s) {
    int n = G.size();
    vector<int> d(n, -1);
    d[s] = 0;
    queue<int> que;
    que.push(s);
    while (!que.empty()) {
        int u = que.front(); que.pop();
        for (auto v : G[u]) {
            if (d[v] != -1) continue;
            d[v] = d[u] + 1;
            que.push(v);
        }
    }
    return d;
}

// !! mines: 鉱山の頂点 id が昇順で入っている !!
vector<vector<int>> shortestDistanceAll(const Graph& G, const vector<int>& mines) {
    vector<vector<int>> dd;
    for (auto mine : mines) {
        dd.emplace_back(shortestDistanceOne(G, mine));
    }
    return dd;
}

class UnionFind {
private:
    const int n;
    vector<int> uni;
public:
    UnionFind(int _n) : n(_n), uni(_n, -1) {}
    int root(int x) {
        if (uni[x] < 0) return x;
        return uni[x] = root(uni[x]);
    }
    bool same(int x, int y) {
        return root(x) == root(y);
    }
    bool unite(int x, int y) {
        x = root(x);
        y = root(y);
        if (x == y) return false;
        if (uni[x] > uni[y]) swap(x, y);
        uni[x] += uni[y];
        uni[y] = x;
        return true;
    }
};

// パンター punter_id のスコアを計算
long long calcScoreOne(const Graph& G, const vector<int>& mines, const vector<vector<int>>& dd, const map<Edge, int>& edge2punter, int punter_id) {
    int n = G.size();
    UnionFind uf(n);
    for (auto p : edge2punter) {
        if (p.second != punter_id) continue;
        uf.unite(p.first.first, p.first.second);
    }

    long long score = 0;
    for (int mine_id = 0; mine_id < mines.size(); mine_id++) {
        for (int v = 0; v < n; v++) {
            if (uf.same(mines[mine_id], v)) {
                score += (long long)dd[mine_id][v] * dd[mine_id][v];
            }
        }
    }
    return score;
}

// 各パンターのスコアを計算
vector<long long> calcScoreAll(const Graph& G, const vector<int>& mines, const vector<vector<int>>& dd, const map<Edge, int>& edge2punter, int num_punter) {
    vector<long long> scores(num_punter);
    for (int punter_id = 0; punter_id < num_punter; punter_id++) {
        scores[punter_id] = calcScoreOne(G, mines, dd, edge2punter, punter_id);
    }
    return scores;
}

int main() {
    /*
    int n, m;
    cin >> n >> m;
    Graph G(n);
    for (int i = 0; i < m; i++) {
        int u, v;
        cin >> u >> v;
        G[u].emplace_back(v);
        G[v].emplace_back(u);
    }

    vector<int> mines{0, 4};
    auto dd = shortestDistanceAll(G, mines);
    for (int i = 0; i < dd.size(); i++) {
        for (int j = 0; j < dd[0].size(); j++) {
            cerr << dd[i][j] << " ";
        }
        cerr << endl;
    }

    int num_punter = 2;
    map<Edge, int> edge2punter;
    edge2punter[make_pair(0, 2)] = 0;
    edge2punter[make_pair(2, 3)] = 0;
    edge2punter[make_pair(1, 4)] = 0;
    edge2punter[make_pair(0, 1)] = 1;
    edge2punter[make_pair(1, 2)] = 1;
    edge2punter[make_pair(1, 3)] = 1;
    edge2punter[make_pair(3, 4)] = 1;

    auto scores = calcScoreAll(G, mines, dd, edge2punter, num_punter);
    for (int i = 0; i < scores.size(); i++) {
        cerr << scores[i] << " ";
    }
    cerr << endl;
    */
}
