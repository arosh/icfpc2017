#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cassert>
#include <cmath>
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
set<int> sites, mines;


//各値の読み込み・前処理
pii setup(int &num_of_punter, int &my_punter_id, map<int,int> &zip, map<int,int> &unzip, set<Edge> &edges_unused, vector<int> &mines_deg) {
    int V, M, E;
    cin >> num_of_punter >> my_punter_id >> V >> M >> E;
    
    vector<int> raw_sites(V), raw_mines(M);
    mines_deg.resize(V,0);
    rep(i, V){
        cin >> raw_sites[i];
        assert( zip.count(raw_sites[i])==0 );
        zip[raw_sites[i]] = i;
        unzip[i] = raw_sites[i];
        
        sites.insert( zip[raw_sites[i]] );
    }
    
    rep(i, M){
        cin >> raw_mines[i];

        int res = zip[raw_mines[i]];
        mines.insert( res );
    }
    
    G.resize(V);
    rep(i, E) {
        int u, v;
        cin >> u >> v;
        u = zip[u];
        v = zip[v];
        G[u].emplace_back(v);
        G[v].emplace_back(u);
        if( mines.count(u) )mines_deg[u]++;
        if( mines.count(v) )mines_deg[v]++;
        
        
        // !! edges_unused 内の edge {u, v} は常に u < v とする !!
        if (u >= v) swap(u, v);
        edges_unused.emplace(u, v);
    }

    return pii(V,E);
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


//グラフ中でそれなりの長さが取れそうなパスの両端の頂点番号のペア(圧縮済)を返す
pii get_fartherst_set(const vector<vector<int>> &ds, const int &upper){
    int max_dist = 0;
    pii max_ind = pii(0,0);
    
    for(const auto &mine:mines){
        if( max_dist>=upper )return max_ind;
        
        vector<int> vec = ds[ mine ];
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
            max_ind = pii(mine,pos);
        }
    }
    return max_ind;
}


//ジャッジからの更新情報を受け取る
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


Edge mp(int u, int v){ return Edge(min(u,v),max(u,v)); }

int main() {
    int num_of_punter, my_punter_id;
    
    vector<int> mines_deg;
    
    set<Edge> edges_unused;
    map<Edge, int> edge2punter;
    
    map<int,int> zip, unzip;
    
    
    int V,E;
    tie(V,E) = setup(num_of_punter, my_punter_id, zip, unzip, edges_unused, mines_deg);
    
    
    vector<bool> visited(V,false);
    vector<vector<int>> ds(V,vector<int>(0));
    
    rep(i,V) ds[i] = shortestDistanceOne(i);
    
    
    int diameter = -1;
    rep(i,V) diameter = max( diameter, *max_element( all(ds[i]) ) );
    
    
    pii site_set = get_fartherst_set(ds, diameter*2.0/3);
    int S = site_set.first;
    int T = site_set.second;
    
    stack<int> passed_path;
    //    stack<int> used;
    //    stack<int> less;
    //    for(auto e:mines)less.push(e);
    passed_path.push(S);
    //    used.push(S);
    
    while(1){
        //これ以上入力がない or '-1 -1 -1' が返ってきたらおわり
        bool res = reflesh(num_of_punter, edge2punter, edges_unused, zip);
        if( res==false )break;
        
        //答えを出力する関数
        auto response = [&](int u, int v){
            assert( edges_unused.count( mp(u,v) ) );
            //            assert( not (visited[u]==true && visited[v]==true) );
            visited[u] = visited[v] = true;
            cout<<my_punter_id<<" "<<unzip[u]<<" "<<unzip[v]<<endl;
        };
        
        
        bool printed = false;
        
        while( passed_path.size() ){
            int cur = passed_path.top();
            int cand = -1, mini = INF;
            
            for(auto e:G[cur]){
                //最短経路 + log(直径)くらいの経路があれば候補を更新する
                if( mini>ds[T][e] &&  ds[T][e]+log(diameter)>=ds[T][cur]){
                    if( edges_unused.count(  mp(e,cur) ) == 0 )continue;
                    if( visited[e] && visited[cur] )continue;
                    cand = e;
                    mini = ds[T][e];
                }
            }
            
            if(cand==-1)passed_path.pop();
            else{
                assert( edges_unused.count(mp(cand,cur)) );
                response(min(cand,cur), max(cand,cur));
                passed_path.push(cand);
                printed = true;
                break;
            }
        }
        if( not printed ){
            [&](){
                for(auto mine:mines){
                    for(auto e:G[mine]){
                        if( edges_unused.count(mp(e,mine))==0 )continue;
                        if( visited[e]==true && visited[mine]==false ){
                            if( edges_unused.count( mp(e,mine) )==0 ){
                                assert( edges_unused.count(mp(e,mine)) );
                                response(e,mine);
                                return ;
                            }
                            
                        }
                    }
                }
                
                pii cand = pii(-1,-1);
                int dist = -1;
                for(auto e:edges_unused){
                    if( visited[e.first] ^ visited[e.second] ){
                        int s,t;
                        tie(s,t) = e;
                        if( visited[s] )swap(s,t);
                        if( ds[S][s]>dist ){
                            dist = ds[S][s];
                            cand = e;
                        }
                    }
                }
                if(cand==pii(-1,-1)){
                    for(auto e:edges_unused){
                        if( !visited[e.first] && !visited[e.second] ){
                            response(e.first,e.second);
                            return ;
                        }
                    }
                    pii tmp = *edges_unused.begin();
                    response(tmp.first,tmp.second);
                    return ;
                }
                assert(edges_unused.size());
                assert(cand!=pii(-1,-1));
                assert( edges_unused.count(mp(cand.first,cand.second)) );
                response(cand.first,cand.second);
            }();
        }
    }
}


/*
 2 0 5 1 8
 1 3 5 7 9
 1
 1 3
 1 5
 1 7
 1 9
 3 5
 5 7
 7 9
 9 3
 0 -1 -1
 1 -1 -1
 
 0 7 9
 1 1 3
 
 0 5 7
 1 1 5
 
 0 3 9
 1 1 9
 
 
 */
