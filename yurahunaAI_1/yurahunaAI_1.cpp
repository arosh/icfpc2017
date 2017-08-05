// score が 1 でも増える辺を見つけたら即座につなぐ

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cassert>
using namespace std;

#define rep(i,n) for (int i=0;i<(n);i++)

using Graph = vector<vector<int>>;
using Edge = pair<int,int>;

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

int my_punter_id;
int num_punter;
Graph G;
int S, M, R;
vector<int> mines;
map<Edge, int> edge2punter;
set<Edge> edges_unused;
vector<int> sites;              // 入力の通りの SiteId
map<int, int> siteId2idx;       // 入力の SiteId を 0~S-1 の添字に変換する

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

        // !! edges_unused 内の edge {u, v} は常に u < v とする !!
        if (u >= v) swap(u, v);
        edges_unused.emplace(u, v);
    }

}

bool containsMine(UnionFind& uf, int v) {
    for (auto mine : mines) {
        if (uf.same(v, mine)) return true;
    }
    return false;
}

void play(UnionFind& uf) {
    // 「from が鉱山と連結」かつ「to が from と連結でない」→ score が増える
    rep(from, S) {
        if (!containsMine(uf, from)) continue;
        for (auto to : G[from]) {
            if (uf.same(from, to)) continue;
            int u = from, v = to;
            if (u >= v) swap(u, v);
            if (!edges_unused.count(make_pair(u, v)) || edge2punter[make_pair(u, v)] != my_punter_id) {
                // 他のパンターに取られた辺だったら continue
                continue;
            }
            u = sites[u];
            v = sites[v];
            cout << my_punter_id << " " << u << " " << v << endl;
            return ;
        }
    }
    // スコアを増やせる辺がなかったら pass
    cout << my_punter_id << " " << -1 << " " << -1 << endl;
}

int main() {

    setup();
    UnionFind uf(S);

    int punter_id, u, v;
    while (cin >> punter_id >> u >> v, punter_id != -1) {
        // ターンの差分を受け取って更新
        rep(i, num_punter) {
            if (i != 0) {
                cin >> punter_id >> u >> v;
            }
            assert((u == -1 && v == -1) || (u != -1 && v != -1));
            if (u == -1 && v == -1) continue;
            u = siteId2idx[u];
            v = siteId2idx[v];
            if (u >= v) swap(u, v);
            edge2punter[make_pair(u, v)] = punter_id;
            assert(edges_unused.count(make_pair(u, v)) == 1);
            edges_unused.erase(make_pair(u, v));
        }

        play(uf);
    }

}
