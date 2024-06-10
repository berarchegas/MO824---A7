#include<bits/stdc++.h>

using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

vector<pair<int, int>> points;
vector<int> demand;

int dist(int i, int j) {
    return floor(sqrt(pow(points[i].first - points[j].first, 2) + pow(points[i].second - points[j].second, 2)));
}

vector<int> tsp(vector<int> bros) {
    int n = bros.size();
    int last = 0;
    vector<int> ans = {0};
    vector<bool> valid(n, 1);
    valid[0] = 0;
    for (int i = 0; i < n - 1; i++) {
        int mn = 1e6, bro = 0;
        for (int j = 1; j < n; j++) {
            if (valid[j] && dist(bros[last], bros[j]) < mn) {
                mn = dist(bros[last], bros[j]);
                bro = j;
            }
        }
        last = bro;
        ans.push_back(bros[last]);
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
    vector<bool> valid(n, true);
    valid[0] = false;
    int ans = 0, qtd = n - 1;
    while (qtd) {
        int start = rng() % n;
        while (!valid[start]) start = rng() % n;
        vector<pair<int, int>> candidates;
        for (int i = 1; i < n; i++) if(valid[i]) candidates.emplace_back(dist(i, start), i);
        sort(candidates.begin(), candidates.end());
        int groupDemand = 0;
        vector<int> group(1, 0);
        for (pair<int, int> x : candidates) {
            if (groupDemand + demand[x.second] <= cap) {
                groupDemand += demand[x.second];
                group.push_back(x.second);
            }
        }
        for (int x : group) cout << x << ' ';
        cout << '\n';
        group = tsp(group);
        for (int x : group) cout << x << ' ';
        cout << '\n';
        qtd -= (int)group.size() - 2;
        for (int i = 0; i < (int)group.size() - 1; i++) {
            valid[group[i]] = false;
            ans += dist(group[i], group[i + 1]);
        }
    }
    cout << ans << '\n';
    return 0;
}