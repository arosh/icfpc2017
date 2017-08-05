#include "bits/stdc++.h"
using namespace std;
typedef long long ll;
typedef pair<int,int> pii;
#define rep(i,n) for(ll i=0;i<(ll)(n);i++)
#define all(a) (a).begin(),(a).end()
#define pb emplace_back
#define INF (1e9+1)

using Graph = vector<vector<int>>;
using Edge = pair<int,int>;

Graph G;
map<Edge, int> edge2punter;

void setup(int &num_of_punter, int &my_punter_id, vector<int> &sites, map<int,int> &zip, vector<int> &mines, set<Edge> &edges_unused) {
    int S, M, R;
    cin >> num_of_punter >> my_punter_id >> S >> M >> R;
    
    sites.resize(S);
    rep(i, S) {
        cin >> sites[i];
        zip[sites[i]] = i;
    }
    mines.resize(M);
    rep(i, M) {
        cin >> mines[i];
        mines[i] = zip[mines[i]];
    }
    sort(mines.begin(), mines.end());
    G.resize(S);
    rep(i, R) {
        int u, v;
        cin >> u >> v;
        u = zip[u];
        v = zip[v];
        G[u].emplace_back(v);
        G[v].emplace_back(u);
        
        // !! edges_unused 内の edge {u, v} は常に u < v とする !!
        if (u >= v) swap(u, v);
        edges_unused.emplace(u, v);
    }
}

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

pii get_fartherst_set(const vector<vector<int>> &ds, const vector<int> &mines, map<int,int> &zip, const int &upper){
    int max_dist = 0;
    pii max_ind = pii(0,0);
    
    for(const auto &mine:mines){
        if( max_dist>=upper )return max_ind;
        
        vector<int> vec = ds[ zip[mine] ];
        int maxi = -1;
        int pos = -1;
        for(auto e:vec){
            if(vec[e]>=upper || vec[e]>maxi){
                maxi = vec[e];
                pos = e;
                
            }
        }
        
        if(max_dist<maxi){
            max_dist = maxi;
            max_ind = pii(zip[mine],pos);
        }
    }
    return max_ind;
}

int main() {
    vector<int> sites; // 入力の通りの SiteId
    vector<int> mines;
    map<int,int> zip;
    set<Edge> edges_unused;
    int num_of_punter, my_punter_id;
    
    setup(num_of_punter, my_punter_id, sites, zip, mines, edges_unused);
    
    vector<vector<int>> ds(G.size(),vector<int>(0));
    rep(i,G.size()){
        ds[i] = shortestDistanceOne(i);
    }
    
    pii land_set = get_fartherst_set(ds,mines,zip, G.size()/3);
    int S = zip[land_set.first ];
    int T = zip[land_set.second];
    
    int cur = S;
    int dir = ds[S][T];
    
    int punter_id, u, v;
    while (cin >> punter_id >> u >> v, punter_id != -1) {
        // ターンの差分を受け取って更新
        rep(i, num_of_punter) {
            if (i != 0) cin >> punter_id >> u >> v;
            
            assert((u == -1 && v == -1) || (u != -1 && v != -1));
            if (u == -1 && v == -1) continue;
            
            u = zip[u];
            v = zip[v];
            if (u >= v) swap(u, v);
            edge2punter[make_pair(u, v)] = punter_id;
            
            assert(edges_unused.count(make_pair(u, v)) == 1);
            edges_unused.erase(make_pair(u, v));
        }
        
        auto response = [&](int u, int v){cout<<my_punter_id<<" "<<u<<" "<<v<<endl;};
        
        if(dir==0){
            pii res = *prev(edges_unused.end());
            response(res.first, res.second);
            edges_unused.erase( prev(edges_unused.end()) );
        }else{
            for(auto e:G[cur]){
                if(ds[T][e]==T-1){
                    response(min(cur,e), max(cur,e));
                    edges_unused.erase(make_pair(min(cur,e), max(cur,e)));
                    cur = e;
                    dir--;
                    break;
                }
            }
        }
    }
}
