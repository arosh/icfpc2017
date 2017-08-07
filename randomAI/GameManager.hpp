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

class GameManager {
protected:
    int my_punter_id;
    int num_punter;
    Graph G;
    vector<int> mines;
    map<Edge, int> edge2punter;
    set<Edge> edges_unused;
    vector<int> sites; // 入力の通りの SiteId
    map<int, int> siteId2idx; // 入力の SiteId を 0~S-1 の添字に変換する

    std::random_device rnd;
    std::mt19937 mt;

public:
    GameManager() : mt(rnd()) {
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
            edges_unused.emplace(min(u, v), max(u, v));
        }
    }
    bool update() {
        rep(i, num_punter) {
            int punter_id, u, v;
            cin >> punter_id >> u >> v;
            if (punter_id == -1) return false;

            assert((u == -1 && v == -1) || (u != -1 && v != -1));
            if (u == -1 && v == -1) continue;
            u = siteId2idx[u];
            v = siteId2idx[v];
            Edge edge(min(u, v), max(u, v));
            edge2punter[edge] = punter_id;
            assert(edges_unused.count(edge) == 1);
            edges_unused.erase(edge);
        }
        return true;
    }
    virtual void play() = 0;
    void start() {
        while (update()) {
            play();
        }
    }
};
