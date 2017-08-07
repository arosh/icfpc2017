#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cassert>
#include <queue>
#include <stack>

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


void setup(int &num_of_punter, int &my_punter_id, vector<int> &raw_sites, map<int,int> &zip, map<int,int> &unzip, vector<int> &raw_mines, set<Edge> &edges_unused) {
    int S, M, R;
    cin >> num_of_punter >> my_punter_id >> S >> M >> R;

    raw_sites.resize(S);
    raw_mines.resize(M);
    rep(i, S) cin >> raw_sites[i];
    rep(i, M) cin >> raw_mines[i];
    rep(i, S) {
        zip[raw_sites[i]] = i;
        unzip[i] = raw_sites[i];
    }

    sort(raw_mines.begin(), raw_mines.end());

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


pii get_fartherst_set(const vector<vector<int>> &ds, const vector<int> &raw_mines, map<int,int> &zip, const int &upper){
    int max_dist = 0;
    pii max_ind = pii(0,0);

    for(const auto &raw_mine:raw_mines){
        if( max_dist>=upper )return max_ind;

        vector<int> vec = ds[ zip[raw_mine] ];
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
            max_ind = pii(zip[raw_mine],pos);
        }
    }
    return max_ind;
}


bool reflesh(const int &num_of_punter, map<Edge, int> &edge2punter, set<Edge> &edges_unused, map<int,int> &zip){
    int punter_id, u, v;
    if( !(cin>>punter_id>>u>>v) || punter_id==-1 )return false;

    rep(i,num_of_punter){
        if(i)cin>>punter_id>>u>>v;

        assert((u == -1 && v == -1) || (u != -1 && v != -1));
        if (u == -1 && v == -1) continue;

        u = zip[u];
        v = zip[v];
        if (u >= v) swap(u, v);
        edge2punter[make_pair(u, v)] = punter_id;

        assert(edges_unused.count(make_pair(u, v)) == 1);
        edges_unused.erase(make_pair(u, v));
    }

    return true;
}


int main() {
    vector<int> raw_sites, raw_mines;
    map<int,int> zip, unzip;
    set<Edge> edges_unused;
    map<Edge, int> edge2punter;
    int num_of_punter, my_punter_id;

    setup(num_of_punter, my_punter_id, raw_sites, zip, unzip, raw_mines, edges_unused);

    vector<vector<int>> ds(G.size(),vector<int>(0));
    rep(i,G.size()){
        ds[i] = shortestDistanceOne(i);
    }

    pii site_set = get_fartherst_set(ds,raw_mines,zip, G.size()/3);
    int S = site_set.first;
    int T = site_set.second;

    stack<int> went;
    went.push(S);

    while(1){
        bool res = reflesh(num_of_punter, edge2punter, edges_unused, zip);

        if( res==false )break;


        auto response = [&](int u, int v){cout<<my_punter_id<<" "<<unzip[u]<<" "<<unzip[v]<<endl;};

        bool printed = false;
        while( went.size() ){
            int cur = went.top();
            int cand = -1;
            int mini = INF;
            for(auto e:G[cur]){
                if( mini>ds[T][e] &&  ds[T][e]<=ds[T][cur]+3){
                    if( edges_unused.count(  make_pair(min(e,cur), max(e,cur)) ) == 0 )continue;
                    cand = e;
                    mini = ds[T][e];
                }
            }

            if(cand==-1) went.pop();
            else {
                response(min(cand,cur), max(cand,cur));
                went.push(cand);
                printed = true;
                break;
            }
        }
        if( not printed ){
            assert(edges_unused.size()>0);
            pii res = *prev(edges_unused.end());
            response(res.first, res.second);
        }
    }
}
