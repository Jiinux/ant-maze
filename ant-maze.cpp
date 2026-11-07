#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <tuple>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <string>
#include <iomanip>
using namespace std;


struct State {
    int r, c, air;
    bool operator<(const State& o) const { return tie(r, c, air) < tie(o.r, o.c, o.air); }
};


struct Node {
    State s;
    int g;
    double h;
    int parent;  
};

struct Result {
    bool found = false;
    int cost = 0;
    long expanded = 0, generated = 0, maxFrontier = 0;
    double ms = 0;
    vector<State> path;
};

class Maze {
public:
    int M, N, A;
    vector<vector<char>> grid;
    State start{0, 0, 0};
    int goalR = -1, goalC = -1;


    Maze(const vector<vector<char>>& g, int maxAir) : M(g.size()), N(g[0].size()), A(maxAir), grid(g) {
        for (int r = 0; r < M; r++)
            for (int c = 0; c < N; c++) {
                if (grid[r][c] == 'S') { start = {r, c, A}; grid[r][c] = 'W'; }  
                if (grid[r][c] == 'X') { goalR = r; goalC = c; grid[r][c] = 'W'; }
            }
    }

    bool isGoal(const State& s) const { return s.r == goalR && s.c == goalC; }

    
    double h(const State& s, int which) const {
        double dx = abs(s.r - goalR), dy = abs(s.c - goalC);
        return which == 1 ? sqrt(dx * dx + dy * dy) : dx + dy;
    }

   
    vector<State> successors(const State& s) const {
        static const int dr[] = {-1, 1, 0, 0, 0}, dc[] = {0, 0, -1, 1, 0};
        vector<State> out;
        for (int k = 0; k < 5; k++) {
            int nr = s.r + dr[k], nc = s.c + dc[k];
            if (nr < 0 || nc < 0 || nr >= M || nc >= N || grid[nr][nc] == 'B') continue;
            int air = s.air;
            if (grid[nr][nc] == 'G') {          
                if (--air < 0) continue;        
            } else air = A;                     
            out.push_back({nr, nc, air});
        }
        return out;
    }
};


Result search(const Maze& m, const string& algo, int which) {
    auto t0 = chrono::steady_clock::now();
    Result res;
    vector<Node> pool;
    
    using Entry = tuple<double, long, int>;
    priority_queue<Entry, vector<Entry>, greater<Entry>> frontier;
    set<State> closed;  
    long order = 0;

    auto f = [&](const Node& n) { return algo == "greedy" ? n.h : n.g + n.h; };
    pool.push_back({m.start, 0, m.h(m.start, which), -1});
    frontier.push({f(pool[0]), order++, 0});
    res.generated = 1;

    while (!frontier.empty()) {
        res.maxFrontier = max<long>(res.maxFrontier, frontier.size());
        int idx = get<2>(frontier.top());
        frontier.pop();
        Node cur = pool[idx];
        if (closed.count(cur.s)) continue;          
        if (m.isGoal(cur.s)) {                      
            res.found = true;
            res.cost = cur.g;
            for (int i = idx; i != -1; i = pool[i].parent) res.path.push_back(pool[i].s);
            reverse(res.path.begin(), res.path.end());
            break;
        }
        closed.insert(cur.s);
        res.expanded++;
        for (const State& ns : m.successors(cur.s)) {
            if (closed.count(ns)) continue;
            pool.push_back({ns, cur.g + 1, m.h(ns, which), idx});
            frontier.push({f(pool.back()), order++, (int)pool.size() - 1});
            res.generated++;
        }
    }
    res.ms = chrono::duration<double, milli>(chrono::steady_clock::now() - t0).count();
    return res;
}

int main(int argc, char** argv) {
    // The professor's maze.
    vector<vector<char>> grid = {
        {'B', 'B', 'B', 'B', 'B', 'B', 'B'},
        {'B', 'W', 'W', 'W', 'W', 'X', 'B'},
        {'B', 'B', 'G', 'B', 'B', 'B', 'B'},
        {'B', 'W', 'G', 'W', 'W', 'W', 'B'},
        {'B', 'B', 'G', 'B', 'W', 'B', 'B'},
        {'B', 'S', 'G', 'W', 'W', 'W', 'B'},
        {'B', 'B', 'B', 'B', 'B', 'B', 'B'}
    };

    int maxAir = argc >= 2 ? atoi(argv[1]) : 3;
    Maze m(grid, maxAir);
    if (argc >= 4) m.start = {atoi(argv[2]), atoi(argv[3]), maxAir};

    cout << "Start=(" << m.start.r << "," << m.start.c << ") Goal=(" << m.goalR << "," << m.goalC
         << ") A=" << m.A << "\n\n";
    cout << left << setw(10) << "Algo" << setw(5) << "h" << setw(8) << "Found" << setw(6) << "Cost"
         << setw(10) << "Expanded" << setw(11) << "Generated" << setw(9) << "MaxFront"
         << "Time(ms)\n";

    for (string algo : {"greedy", "astar"})
        for (int which : {1, 2}) {
            Result r = search(m, algo, which);
            cout << left << setw(10) << algo << setw(5) << ("h" + to_string(which))
                 << setw(8) << (r.found ? "yes" : "NO") << setw(6) << (r.found ? to_string(r.cost) : "-")
                 << setw(10) << r.expanded << setw(11) << r.generated << setw(9) << r.maxFrontier
                 << fixed << setprecision(3) << r.ms << "\n";
            if (r.found) {
                cout << "   path: ";
                for (auto& s : r.path) cout << "(" << s.r << "," << s.c << ",a" << s.air << ") ";
                cout << "\n";
            }
        }
}