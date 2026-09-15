#include <bits/stdc++.h>

using namespace std;

vector<int> approximation(vector<pair<int,int>>& edges,int N,vector<pair<int,int>>& matching){
    unordered_set<int> visited;
    for( auto& edge : edges){
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
int main() {

    int N = 10;

    ifstream input("results.csv");

    if (!input.is_open()) {
        cout << "Could not open results.csv\n";
        return 1;
    }

    ofstream output("results_temp.csv");

    string line;
    getline(input, line);

    output << "N,m,cover_size,time_microseconds,"
           << "matching_size,approximate_cover_size,"
           << "approximation_factor,approx_time_microseconds\n";

    while (getline(input, line)) {

        stringstream ss(line);

        int n, m, bfaCoverSize;
        long long bfaTime;

        char comma;

        ss >> n >> comma
           >> m >> comma
           >> bfaCoverSize >> comma
           >> bfaTime;

        string filename =
            "graph_m" + to_string(m) + ".txt";

        vector<pair<int,int>> edges;

        if (!loadGraph(filename, edges)) {
            cout << "Could not open "
                 << filename << endl;
            continue;
        }

        vector<pair<int,int>> matching;

        auto start =
            chrono::high_resolution_clock::now();

        vector<int> approximateVc =
            approximation(edges, N, matching);

        auto end =
            chrono::high_resolution_clock::now();

        auto duration =
            chrono::duration_cast<chrono::microseconds>(
                end - start
            ).count();

        int matchingSize = matching.size();

        int approximateCoverSize =
            approximateVc.size();

        double approximationFactor =
            (double) approximateCoverSize /
            bfaCoverSize;

        string aproxFilename =
            "aprox_m" + to_string(m) + ".txt";

        ofstream aproxFile(aproxFilename);

        if (!aproxFile.is_open()) {

            cout << "Could not create "<< aproxFilename << endl;

        } else {

            aproxFile << "Matching:\n";

            for (auto edge : matching) {
                aproxFile << edge.first
                          << " "
                          << edge.second
                          << "\n";
            }

            aproxFile << "\nVertexCover:\n";

            for (auto vertex : approximateVc) {
                aproxFile << vertex << " ";
            }

            aproxFile << "\n";

            aproxFile.close();
        }

        output << n << ","
               << m << ","
               << bfaCoverSize << ","
               << bfaTime << ","
               << matchingSize << ","
               << approximateCoverSize << ","
               << fixed << setprecision(2)
               << approximationFactor << ","
               << duration
               << "\n";

    }

    input.close();
    output.close();

    remove("results.csv");
    rename("results_temp.csv", "results.csv");

    cout << "\nresults.csv updated successfully\n";

    system("python aprox_visualization.py");
    return 0;
}