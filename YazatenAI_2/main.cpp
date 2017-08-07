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
    mines_deg.resize(V,-1);
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
        
        // !! edges_unused 内の edge {u, v} は常に u < v とする !!
        if (u >= v) swap(u, v);
        edges_unused.emplace(u, v);
    }
    
    for(auto v:mines){
        mines_deg[v] = G[v].size();
    }
    
    return pii(V,E);
}


Edge mp(int u, int v){ return Edge(min(u,v),max(u,v)); }


vector<int> shortestDistanceOne(int s, const set<Edge> &edges_unused) {
    int n = G.size();
    vector<int> d(n, -1);
    d[s] = 0;
    queue<int> que;
    que.push(s);
    while (!que.empty()) {
        int u = que.front(); que.pop();
        for (auto v : G[u]) {
            if( edges_unused.count(mp(u,v))==0 )continue;
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


class UF{
private:
    vector<int> par,rank,elm;
    vector<vector<int>> elmList;    //don't be validated
public:
    int groups;
    
    UF(int __size):par(__size) , rank(__size,0) , elm(__size,1) , groups(__size), elmList(__size){
        rep(i,__size)elmList[i] = vector<int>(1,i);
        for(int i=0;i<__size;i++)par[i]=i;
    }
    
    int getElements(int n){ return elm[find(n)]; }
    vector<int> getElementsList(int n){ return elmList[find(n)]; }
    
    int find(int x){
        if(par[x]==x) {
            return x;
        }else{
            return par[x]=find(par[x]);
        }
    }
    
    void unite(int x,int y){
        x=find(x);
        y=find(y);
        if(x==y) return;
        
        groups--;
        if(rank[x]<rank[y]){
            par[x]=y;
            elm[y]+=elm[x];
            for(auto &e:elmList[x])elmList[y].pb(e);
            
        }else{
            par[y]=x;
            elm[x]+=elm[y];
            for(auto &e:elmList[y])elmList[x].pb(e);
            if(rank[x]==rank[y])rank[x]++;
        }
    }
    
    bool isSame(int x,int y){
        return find(x)==find(y);
    }
};


//ジャッジからの更新情報を受け取る
bool reflesh(const int &num_of_punter, map<Edge, int> &edge2punter, set<Edge> &edges_unused, map<int,int> &zip, vector<int> &online_mines_deg){
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
        
        if( mines.count(u) ) online_mines_deg[u]--;
        if( mines.count(v) ) online_mines_deg[v]--;
        
        assert(edges_unused.count(make_pair(u, v)) == 1);
        edges_unused.erase(make_pair(u, v));
    }
    
    return true;
}



int main() {
    int num_of_punter, my_punter_id;
    
    vector<int> mines_deg;
    
    set<Edge> edges_unused;
    map<Edge, int> edge2punter;
    
    map<int,int> zip, unzip;
    
    
    int V,E;
    tie(V,E) = setup(num_of_punter, my_punter_id, zip, unzip, edges_unused, mines_deg);
    
    UF uf(V);

    vector<bool> visited(V,false);
    vector<vector<int>> ds(V,vector<int>(0));
    
    rep(i,V) ds[i] = shortestDistanceOne(i, edges_unused);
    
    
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
    
    
    vector<int> online_mines_deg = mines_deg;
    vector<bool> get_mines_flag(V,false);
    
    int turn = 0;
    while(1){
        turn++;
        
        //これ以上入力がない or '-1 -1 -1' が返ってきたらおわり
        bool res = reflesh(num_of_punter, edge2punter, edges_unused, zip, online_mines_deg);
        if( res==false )break;
        
        //答えを出力する関数
        auto response = [&](int u, int v){
            assert( edges_unused.count( mp(u,v) ) );
            //            assert( not (visited[u]==true && visited[v]==true) );
            visited[u] = visited[v] = true;
            uf.unite(u,v);
            cout<<my_punter_id<<" "<<unzip[u]<<" "<<unzip[v]<<endl;
        };
        
        
        int turn = 0;
        
        auto decide_edge = [&](){
            turn++;
            
            //そこそこ辺があり、かつ (ターン*play人数) が3以上なら
            if( E>30 && turn*num_of_punter>=6 ){
                for(auto mine:mines){
                    if(mines_deg[mine]==online_mines_deg[mine] && mines_deg[mine]<=3){
                        get_mines_flag[mine] = true;
                    }
                }
                
                rep(i,V){
                    if(get_mines_flag[i]==true){
                        assert(mines.count(i));
                        for(auto v:G[i]){
                            if( edges_unused.count(mp(i,v)) ){
                                response(i,v);
                                return ;
                            }
                        }
                        get_mines_flag[i] = false;
                    }
                }
            }
            
            //目的地に向かう
            while( passed_path.size() ){
                int cur = passed_path.top();
                int cand = -1, mini = INF;
                
                for(auto e:G[cur]){
                    //最短経路 + log(直径)くらいの経路があれば候補を更新する
                    if( mini>ds[T][e] &&  ds[T][e]+max<int>(ceil(log(diameter)),2)>=ds[T][cur]){
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
                    return ;
                    passed_path.push(cand);
                    break;
                }
            }
            
            
            
            auto solve_score = [&](int u,int v){
                int new_vert, old_vert;
                if(visited[u])  new_vert = v, old_vert = u;
                else            new_vert = u, old_vert = v;
                
                int score = 0;
                for(auto mine:mines){
                    if( uf.isSame(mine, old_vert) ){
                        score+=ds[mine][new_vert]*ds[mine][new_vert];
                    }
                }
                return score;
            };
            
            pii cand = pii(-1,-1);
            int score = -INF;;
            for(auto e:edges_unused){
                if( visited[e.first] ^ visited[e.second] ){
                    int res = solve_score(e.first, e.second);
                    if(res>score){
                        res = score;
                        cand = pii(e.first,e.second);
                    }
                }
            }
            if(cand!=pii(-1,-1)){
                assert(edges_unused.size());
                assert(cand!=pii(-1,-1));
                assert( edges_unused.count(mp(cand.first,cand.second)) );
                response(cand.first,cand.second);
                
                return ;
            }else{
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
        };
        
        decide_edge();
    }
}
