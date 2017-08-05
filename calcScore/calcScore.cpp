#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <cassert>
using namespace std;

#define rep(i,n) for (int i=0;i<(n);i++)

using Graph = vector<vector<int>>;
using Edge = pair<int,int>;

int S, M, R;
int my_punter_id;
int num_punter;
Graph G;
vector<int> mines;
map<Edge, int> edge2punter;
vector<vector<int>> dd;
vector<int> sites; // 入力の通りの SiteId
map<int, int> siteId2idx; // 入力の SiteId を 0~S-1 の添字に変換する

void setup() {
    cin >> num_punter >> my_punter_id >> S >> M >> R;

    sites.resize(S);
    rep(i, S) {
        cin >> sites[i];
        siteId2idx[sites[i]] = i;
    }
    mines.resize(M);
    rep(i, M) {
        cin >> mines[i];
        mines[i] = siteId2idx[mines[i]];
    }
    sort(mines.begin(), mines.end());
    G.resize(S);
    rep(i, R) {
        int u, v;
        cin >> u >> v;
        u = siteId2idx[u];
        v = siteId2idx[v];
        G[u].emplace_back(v);
        G[v].emplace_back(u);
    }

    rep(i, R) {
        int punter_id, u, v;
        cin >> punter_id >> u >> v;
        assert((u == -1 && v == -1) || (u != -1 && v != -1));
        if (u == -1 && v == -1) continue;
        u = siteId2idx[u];
        v = siteId2idx[v];
        assert(edge2punter.count(make_pair(u, v)) == 0);
        edge2punter[make_pair(u, v)] = punter_id;
    }
}

// s から各頂点への最短距離を計算
vector<int> shortestDistanceOne(int s) {
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

void shortestDistanceAll() {
    for (auto mine : mines) {
        dd.emplace_back(shortestDistanceOne(mine));
    }
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
long long calcScoreOne(int punter_id) {
    int n = G.size();
    UnionFind uf(n);
    for (auto p : edge2punter) {
        if (p.second != punter_id) continue;
        uf.unite(p.first.first, p.first.second);
    }

    long long score = 0ll;
    for (int mine_id = 0; mine_id < mines.size(); mine_id++) {
        for (int v = 0; v < n; v++) {
            if (uf.same(mines[mine_id], v)) {
                assert(dd[mine_id][v] != -1);
                score += (long long)dd[mine_id][v] * dd[mine_id][v];
            }
        }
    }
    return score;
}

// 各パンターのスコアを計算し、標準出力にスペース区切りで出力
void calcScoreAll() {

    setup();
    shortestDistanceAll();

    vector<long long> scores(num_punter);
    for (int punter_id = 0; punter_id < num_punter; punter_id++) {
        scores[punter_id] = calcScoreOne(punter_id);
    }
    rep(i, num_punter) {
        cout << (i ? " " : "") << scores[i];
    }
    cout << endl;
}

int main() {
    calcScoreAll();
}
