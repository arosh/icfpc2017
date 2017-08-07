#include "GameManager.hpp"
using namespace std;

class randomAI : public GameManager {
public:
    void play() {
        // ランダムに辺を選ぶ
        std::uniform_int_distribution<int> uniint(0, (int)edges_unused.size() - 1);
        int edge_id = uniint(mt);
        int u, v;
        tie(u, v) = *next(edges_unused.begin(), edge_id);
        u = sites[u];
        v = sites[v];
        cout << my_punter_id << " " << u << " " << v << endl;
        // cerr << my_punter_id << " " << u << " " << v << endl;
    }
};

int main() {
    randomAI random_ai;
    random_ai.start();
}
