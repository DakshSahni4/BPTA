#include <bits/stdc++.h>



using namespace std;

vector<int> aproximation(vector<pair<int,int>>& edges,int N){
    unordered_set<int> visited;
    for( auto edge : edges){
        int u = edge.first;
        int v = edge.second;

        if(visited.count(u) || visited.count(v)) continue;
        else {
            visited.insert(u);
            visited.insert(v);
        }
    }
    vector<int> aproximateVc;
    for(auto vertex : visited) aproximateVc.push_back(vertex);
    return aproximateVc;
}
bool loadGraph(const string& filename, vector<pair<int,int>>& edges) {

    ifstream inFile(filename);
    if (!inFile.is_open()) return false;

    edges.clear();

    int u, v;

    while (inFile >> u >> v) {
        edges.emplace_back(u, v);
    }

    inFile.close();

    return true;

}
int main(){
    int N = 10;

    string filename = "graph_m" + to_string(10) + ".txt";
    vector<pair<int,int>> edges;
    loadGraph(filename,edges);

    vector<int> aproximateVc = aproximation(edges,N);
    for(auto i : aproximateVc) cout<<i<<" ";
}