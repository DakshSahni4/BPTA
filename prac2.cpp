#include <bits/stdc++.h>
#include <iomanip>

using namespace std;

vector<int> approximation(vector<pair<int,int>>& edges,int N,vector<pair<int,int>>& matching){
    unordered_set<int> visited;
    for( auto edge : edges){
        int u = edge.first;
        int v = edge.second;

        if(visited.count(u) || visited.count(v)) continue;
        else {
            matching.push_back({u,v});
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
    
    string filename = "graph_m" + to_string(45) + ".txt";
    vector<pair<int,int>> edges;
    loadGraph(filename,edges);
    
    vector<pair<int,int>> matching;
auto start = chrono::high_resolution_clock::now();

vector<int> approximateVc = approximation(edges, N, matching);

auto end = chrono::high_resolution_clock::now();

auto nanoDuration =
    chrono::duration_cast<chrono::nanoseconds>(end - start);

auto microDuration =
    chrono::duration_cast<chrono::microseconds>(end - start);

double microDouble =
    chrono::duration<double, std::micro>(end - start).count();

cout << "Nanoseconds: " << nanoDuration.count() << endl;
cout << "Microseconds (cast): " << microDuration.count() << endl;
cout << "Microseconds (double): " << microDouble << endl;

    for(auto edge : matching) cout<<edge.first<<" "<<edge.second<<endl;
    for(auto i : approximateVc) cout<<i<<" ";
}