#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cassert>
#include <random>
using namespace std;

#define rep(i,n) for (int i=0;i<(n);i++)

using Graph = vector<vector<int>>;
using Edge = pair<int,int>;

int my_punter_id;
int num_punter;
Graph G;
vector<int> mines;
map<Edge, int> edge2punter;
set<Edge> edges_unused;
vector<int> sites; // 入力の通りの SiteId
map<int, int> siteId2idx; // 入力の SiteId を 0~S-1 の添字に変換する

void setup() {
    int S, M, R;
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

int main() {

    setup();

    std::random_device rnd;
    std::mt19937 mt(rnd());

    int punter_id, u, v;
    while (cin >> punter_id >> u >> v, punter_id != -1) {
        // ターンの差分を受け取って更新
        rep(i, num_punter) {
            if (i != 0) {
                cin >> punter_id >> u >> v;
            }
            u = siteId2idx[u];
            v = siteId2idx[v];
            if (u >= v) swap(u, v);
            edge2punter[make_pair(u, v)] = punter_id;
            assert(edges_unused.count(make_pair(u, v)) == 1);
            edges_unused.erase(make_pair(u, v));
        }

        // ランダムに辺を選ぶ
        std::uniform_int_distribution<int> uniint(0, (int)edges_unused.size() - 1);
        int edge_id = uniint(mt);
        vector<Edge> edges_unused_vec(edges_unused.begin(), edges_unused.end());
        int u, v;
        tie(u, v) = edges_unused_vec[edge_id];
        u = sites[u];
        v = sites[v];
        cout << my_punter_id << " " << u << " " << v << endl;
    }

}
