#include<bits/stdc++.h>

using namespace std;

vector<pair<int, int>> points;
vector<int> demand;

int dist(int i, int j) {
    return floor(sqrt(pow(points[i].first - points[j].first, 2) + pow(points[i].second - points[j].second, 2)));
}

vector<int> tsp(int n) {
    int last = 0;
    vector<int> ans = {0};
    vector<bool> valid(n, 1);
    valid[0] = 0;
    for (int i = 0; i < n - 1; i++) {
        int mn = 1e6, bro = 0;
        for (int j = 1; j < n; j++) {
            if (valid[j] && dist(last, j) < mn) {
                mn = dist(last, j);
                bro = j;
            }
        }
        last = bro;
        ans.push_back(last);
        valid[last] = 0;
    }
    ans.push_back(0);
    return ans;
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
    }
    vector<int> cycle = tsp(n);
    int ans = 0, sum = 0;
    for (int i = 0; i < n; i++) {
        if (sum + demand[cycle[i + 1]] > cap) {
            ans += dist(cycle[i], 0);
            ans += dist(0, cycle[i + 1]);
            sum = 0;
        }
        else {
            ans += dist(cycle[i], cycle[i + 1]);
            sum += demand[cycle[i + 1]];
        }
    }
    cout << ans << '\n';
    return 0;
}