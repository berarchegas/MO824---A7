#include<bits/stdc++.h>

using namespace std;

vector<pair<int, int>> points;
vector<int> demand;
vector<int> pai, sz, groupDemand;

int dist(int i, int j) {
    return floor(sqrt(pow(points[i].first - points[j].first, 2) + pow(points[i].second - points[j].second, 2)));
}

int find(int x) {
    if (pai[x] == x) return x;
    return pai[x] = find(pai[x]);
}

void join(int a, int b) {
    a = find(a), b = find(b);
    if (a == b) return;
    if (sz[a] < sz[b]) swap(a, b);
    pai[b] = a;
    sz[a] += sz[b];
    groupDemand[a] += groupDemand[b];
}

int main() {
    int n, cap;
    cin >> n >> cap;
    for (int i = 0; i < n; i++) {
        int x, y;
        cin >> x >> y;
        points.emplace_back(x, y);
    }
    for (int i = 0; i < n; i++) {
        int q;
        cin >> q;
        demand.push_back(q);
        sz.push_back(1);
        pai.push_back(i);
        groupDemand.push_back(q);
    }
    vector<bool> start(n, true), end(n, true);
    vector<array<int, 3>> savings;
    for (int i = 1; i < n; i++) {
        for (int j = 1; j < n; j++) {
            if (i == j) continue;
            savings.push_back({dist(i, 0) + dist(0, j) - dist(i, j), i, j});
        }
    }
    sort(savings.rbegin(), savings.rend());
    int ans = 0;
    for (int i = 1; i < n; i++) ans += 2 * dist(0, i);
    for (array<int, 3> x : savings) {
        if (start[x[1]] && end[x[2]] && x[0] > 0 && find(x[1]) != find(x[2]) && groupDemand[find(x[1])] + groupDemand[find(x[2])] <= cap) {
            start[x[1]] = false;
            end[x[2]] = false;
            join(x[1], x[2]);
            ans -= x[0];
        }
    }
    cout << ans << '\n';
    return 0;
}