#include <bits/stdc++.h>
#include <algorithm>
using namespace std;

void helper(int i, vector<int> curr, vector<vector<int>> &subsets, vector<int> vertices)
{
    if (i == vertices.size())
    {
        subsets.push_back(curr);
        return;
    }

    curr.push_back(vertices[i]);
    cout << vertices[i] << "\n";
    helper(i + 1, curr, subsets, vertices);
    curr.pop_back();
    helper(i + 1, curr, subsets, vertices);
}
vector<vector<int>> genrateSubsets(int N)
{

    int i = 0;
    vector<int> curr = {};
    vector<vector<int>> subsets = {{}};

    vector<int> vertices = {};

    for (int i = 0; i < N; i++)
    {
        vertices.push_back(i);
    }
    helper(i, curr, subsets, vertices);

    return subsets;
}

bool checkinsubset(int u, vector<int> subset)
{
    for (auto i : subset)
        if (u == i)
            return true;

    return false;
}

bool isVc(vector<int> subset, vector<pair<int, int>> edgeList)
{
    for (auto edge : edgeList)
    {
        if (!checkinsubset(edge.first, subset) && !checkinsubset(edge.second, subset))
            return false;
    }
    return true;
}
int vertexCover(vector<pair<int, int>> edgeList, int N, vector<int> &ans)
{

    vector<vector<int>> subsets;
    int best = N + 1;

    subsets = genrateSubsets(N);

    for (int i = 0; i < subsets.size(); i++)
    {
        int subsetSize = subsets[i].size();
        if (isVc(subsets[i], edgeList))
        {
            best = min(best, subsetSize);
            ans = subsets[i];
        }
    }

    return best;
}

void datasetGenrator(int n = 10, int m = 45){//will maintain it as edge list
    return ;
}

int main()
{
    
    vector<int> ans;
    int N = 7;
    vector<pair<int, int>> edgeList = {
        {0, 1}, {0, 2}, {2, 1}, {1, 3}, {2, 3}, {2, 4}, {3, 4}, {3, 5}, {5, 6}, {6, 1}};
    cout << vertexCover(edgeList, N, ans);
    cout << endl;
    
    for (auto i : ans)
        cout << i;

}
