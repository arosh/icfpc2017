#include <algorithm>
#include <iostream>
#include <random>
#include <vector>
#include <set>
#include <map>
#include <ctime>
#include <sstream>
#include "subprocess.hpp"
#define int long long
#define REP(i, n) for (int i = 0; i < (int)(n); ++i)
using namespace std;
using namespace subprocess;
struct Edge {
  int source, target;
  bool operator <(const Edge &other) const {
    return tie(source, target) < tie(other.source, other.target);
  }
  bool operator ==(const Edge &other) const {
    return tie(source, target) == tie(other.source, other.target);
  }
};
struct SetUp {
  int N, P, S, M, R;
  set<int> sites, mines;
  set<Edge> rivers;
  void read() {
    cin >> N >> P >> S >> M >> R;
    REP(i, S) {
      int value;
      cin >> value;
      sites.insert(value);
    }
    REP(i, M) {
      int value;
      cin >> value;
      mines.insert(value);
    }
    REP(i, R) {
      int source, target;
      cin >> source >> target;
      if (source > target) {
        swap(source, target);
      }
      Edge e{ source, target };
      rivers.insert(e);
    }
  }
};
struct Move {
  int punter;
  Edge river;
  void read() {
    cin >> punter >> river.source >> river.target;
  }
  void write() const {
    cout << punter << river.source << river.target;
  }
};
struct GameState {
  map<Edge, int> claims;
  void apply(const Move &move) {
    claims[move.river] = move.punter;
  }
  void apply(const vector<Move> &moves) {
    for(const Move &move : moves) {
      apply(move);
    }
  }
};
SetUp parse_setup() {
  SetUp setup;
  setup.read();
  return setup;
}
GameState playOut(const SetUp &setup, const GameState &gameState, const set<Edge> &avail, const Edge &firstMove, mt19937 &mt) {
  GameState state;
  state.claims.insert(gameState.claims.begin(), gameState.claims.end());
  vector<Edge> availCopy(avail.begin(), avail.end());
  shuffle(availCopy.begin(), availCopy.end(), mt);
  int p = setup.P;
  state.claims[firstMove] = p;
  p++;
  p %= setup.N;
  for (const Edge &e : availCopy) {
    if(e == firstMove) continue;
    state.claims[e] = p;
    p++;
    p %= setup.N;
  }
  return state;
}
double calcScore(const SetUp &setup, const GameState &gameState) {
  ostringstream oss;
  oss << setup.N << ' ' << setup.P << ' ' << setup.S << ' ' << setup.M << ' ' << setup.R << endl;
  for(int site : setup.sites) {
    if(site != *setup.sites.begin()) oss << ' ';
    oss << site;
  }
  oss << endl;
  for(int mine : setup.mines) {
    if(mine != *setup.mines.begin()) oss << ' ';
    oss << mine;
  }
  oss << endl;
  for(const auto &river : setup.rivers) {
    oss << river.source << ' ' << river.target << endl;
  }
  for(const auto &iter : gameState.claims) {
    oss << iter.second << ' ' << iter.first.source << ' ' << iter.first.target << endl;
  }
  auto program = "./calcScore/calcScore";
  auto p = Popen({program}, input{PIPE}, output{PIPE});
  auto msg = oss.str();
  auto res = p.communicate(msg.c_str(), msg.size()).first;
  vector<int> scores;
  istringstream iss(res.buf.data());
  for(int i = 0; i < setup.N; i++) {
    iss >> scores[i];
  }
  return 1.0 * scores[setup.P] / setup.N;
}
Move search(const SetUp &setup, const GameState &gameState, mt19937 &mt) {
  set<Edge> avail(setup.rivers.begin(), setup.rivers.end());
  for (const auto keyval : gameState.claims) {
    avail.erase(keyval.first);
  }
  map<Edge, double> sum, sum2;
  map<Edge, int> size;
  for(const Edge &e : avail) {
    sum[e] = 0;
    sum2[e] = 0;
    size[e] = 0;
  }
  int n = 1;
  clock_t clock_start = clock();
  while(true) {
    clock_t clock_now = clock();
    if (1.0 * (clock_now - clock_start) / CLOCKS_PER_SEC > 0.8) {
      break;
    }
    double maxUcb = -1;
    Edge maxEdge;
    for(const Edge &e : avail) {
      int t = size[e] + 1;
      double mu = (t == 0) ? 0 : (sum[e] / t);
      double mu2 = (t == 0) ? 0 : (sum2[e] / t);
      double var = mu2 - mu*mu;
      double ucb = mu + sqrt(log(n) / t * min(0.25, var + sqrt(2 * log(n) / t)));
      if (ucb > maxUcb) {
        maxUcb = ucb;
        maxEdge = e;
      }
    }
    GameState state = playOut(setup, gameState, avail, maxEdge, mt);
    double score = calcScore(setup, gameState);
    size[maxEdge]++;
    sum[maxEdge] += score;
    sum2[maxEdge] += score*score;
    n++;
  }
  int maxMove = -1;
  Edge bestMove;
  for(const Edge &e : avail) {
    if(size[e] > maxMove) {
      maxMove = size[e];
      bestMove = e;
    }
  }
  Move m;
  m.punter = setup.P;
  m.river = bestMove;
  return m;
}
signed main() {
  std::random_device rnd;
  std::mt19937 mt(rnd());
  const SetUp setup = parse_setup();
  GameState gameState;
  while (true) {
    vector<Move> moves(setup.N);
    moves[0].read();
    if (moves[0].punter == -1) {
      break;
    }
    for(int i = 1; i < setup.N; i++) {
      moves[i].read();
    }
    gameState.apply(moves);
    const Move move = search(setup, gameState, mt);
    move.write();
  }
}
