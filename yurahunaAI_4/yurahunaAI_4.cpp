#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cassert>
#include <queue>
#include <random>
using namespace std;

#define rep(i,n) for (int i=0;i<(n);i++)

using Graph = vector<vector<int>>;
using Edge = pair<int,int>;

std::random_device rnd;
std::mt19937 mt(rnd());

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
vector<vector<int>> dd;         // dd[i][v] = もとのグラフにおける mines[i] から頂点 v までの最短距離

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

vector<vector<int>> shortestDistanceAll(const Graph& G, const vector<int>& mines) {
    vector<vector<int>> dd;
    for (auto mine : mines) {
        dd.emplace_back(shortestDistanceOne(G, mine));
    }
    return dd;
}

/*
// 離心率最大の頂点を最初の鉱山として選ぶ
int selectStartMine(const vector<vector<int>>& dd) {
    int ma_ecc = -1;
    int arg_m;
    rep(mine_id, dd.size()) {
        int ecc = *max_element(dd[mine_id].begin(), dd[mine_id].end());
        if (ma_ecc < ecc) {
            ma_ecc = ecc;
            arg_m = mine_id;
        }
    }
    assert(ma_ecc != -1);
    return arg_m;
}
*/

bool containsMine(UnionFind& uf, int v) {
    for (auto mine : mines) {
        if (uf.same(v, mine)) return true;
    }
    return false;
}

// 鉱山 mines[mine_id] を始点とする最短経路DAGを返す（他の人に使われていない辺のみ用いる）
Graph shortestDistanceDAG(int mine_id) {
    int n = G.size();
    Graph H(n);
    rep(u, n) {
        for (auto v : G[u]) {
            Edge e(min(u, v), max(u, v));
            if (edges_unused.count(e) || edge2punter[e] == my_punter_id) {
                if (dd[mine_id][u] + 1 == dd[mine_id][v]) {
                    H[u].emplace_back(v);
                }
                else if (dd[mine_id][v] + 1 == dd[mine_id][u]) {
                    H[v].emplace_back(u);
                }
            }
        }
    }
    return H;
}

// DAG H において、頂点 u から到達可能な頂点の数を downwards[u] に格納
int calcDownwardsOne(const Graph& H, int u, vector<int>& downwards_one) {
    downwards_one[u] = 1;
    for (auto v : H[u]) {
        downwards_one[u] += calcDownwardsOne(H, v, downwards_one);
    }
    return downwards_one[u];
}

vector<vector<int>> calcDownwardsAll() {
    vector<vector<int>> downwards;
    rep(mine_id, M) {
        Graph H = shortestDistanceDAG(mine_id);
        vector<int> downwards_one(S);
        calcDownwardsOne(H, mines[mine_id], downwards_one);
        downwards.emplace_back(downwards_one);
    }
    return downwards;
}

void answer(int my_punter_id, int u, int v) {
    u = sites[u];
    v = sites[v];
    cout << my_punter_id << " " << u << " " << v << endl;
}

// 未接続の鉱山のうち、接続する辺が (次数) * EDGE_RATIO 本以上取られているものがあれば、その鉱山に隣接する辺を採用
// なければ、スコアの増分が最大である辺を採用
// 複数ある場合、最短経路DAGにおいて自身から到達可能な頂点数が最大であるような頂点に向かう辺を採用
void play(UnionFind& uf) {
    long long ma_score = 0ll;
    int ma_down = 0;
    int arg_u, arg_v;

    for (auto u : mines) {
        for (auto v : G[u]) {
            Edge e(min(u, v), max(u, v));
            if (edges_unused.count(e)) {
                answer(my_punter_id, e.first, e.second);
                return ;
            }
        }
    }

    auto downwards = calcDownwardsAll();

    rep(from, S) {
        if (!containsMine(uf, from)) continue;
        for (auto to : G[from]) {
            if (uf.same(from, to)) continue;
            int u = from, v = to;
            if (u >= v) swap(u, v);
            if (!edges_unused.count(make_pair(u, v))) {
                // 既に使われている辺は取れない
                continue;
            }

            // 今、from を含む連結成分 C1 と、to を含む連結成分 C2 をつなぐとする
            // 辺 {from, to} をつなぐことによる score の増分は、
            // sum {C1 に属する鉱山と C2 に属する頂点との最短距離の2乗}
            // + sum {C2 に属する鉱山と C1 に属する頂点との最短距離の2乗}
            long long score = 0ll;
            rep(i, M) {
                if (!uf.same(from, mines[i])) continue;
                rep(v, S) {
                    if (!uf.same(to, v)) continue;
                    assert(dd[i][v] != -1);
                    score += (long long)dd[i][v] * dd[i][v];
                }
            }
            rep(i, M) {
                if (!uf.same(to, mines[i])) continue;
                rep(v, S) {
                    if (!uf.same(from, v)) continue;
                    assert(dd[i][v] != -1);
                    score += (long long)dd[i][v] * dd[i][v];
                }
            }

            if (ma_score > score) {
                continue;
            }

            int down = 0;
            rep(mine_id, M) {
                // どこを足すのが良い？？
                if (uf.same(from, mines[mine_id])) {
                    down += downwards[mine_id][to];
                }
            }

            if (ma_score < score || (ma_score == score && ma_down < down)) {
                ma_score = score;
                ma_down = down;
                arg_u = u;
                arg_v = v;
            }
        }
    }

    if (ma_score > 0) {
        arg_u = sites[arg_u];
        arg_v = sites[arg_v];
        cout << my_punter_id << " " << arg_u << " " << arg_v << endl;
        // cerr << my_punter_id << " " << arg_u << " " << arg_v << endl;
        return ;
    }

    // スコアを増やせる辺がなかったら random
    std::uniform_int_distribution<int> uniint(0, (int)edges_unused.size() - 1);
    int edge_id = uniint(mt);
    vector<Edge> edges_unused_vec(edges_unused.begin(), edges_unused.end());
    int u, v;
    tie(u, v) = edges_unused_vec[edge_id];
    u = sites[u];
    v = sites[v];
    cout << my_punter_id << " " << u << " " << v << endl;
    // cerr << my_punter_id << " " << u << " " << v << endl;
    // cout << my_punter_id << " " << -1 << " " << -1 << endl;
    // cerr << my_punter_id << " " << -1 << " " << -1 << endl;
}

int main() {

    setup();
    dd = shortestDistanceAll(G, mines);
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
