#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

void helper(int i, vector<int> curr, vector<vector<int>> &subsets, vector<int> vertices)
{
	if (i == vertices.size())
	{
		subsets.push_back(curr);
		return;
	}

	curr.push_back(vertices[i]);
	helper(i + 1, curr, subsets, vertices);

	curr.pop_back();
	helper(i + 1, curr, subsets, vertices);
}
vector<vector<int>> genrateSubsets(int N)
{

	int i = 0;
	vector<int> curr = {};
	vector<vector<int>> subsets = {};

	vector<int> vertices = {};

	for (int j = 0; j < N; j++) vertices.push_back(j);

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
		if (isVc(subsets[i], edgeList) && best> subsetSize)
		{
			best = subsetSize;
			ans = subsets[i];
		}
	}

	return best;
}

vector<pair<int,int>> generateEdges(int n,int m) {
	
	vector<pair<int,int>> edges;

	set<pair<int,int>>edgesSet;


	for(int i = 1;i<n;i++) {
		int randomValue = rand() % i;

		int first = min(i,randomValue);
		int second = max(i,randomValue);

		if(edgesSet.count({first,second})<= 0 ) {

			edges.push_back({first,second});
			edgesSet.insert({first,second});
		}

		}

		while(edgesSet.size() != m) {
		    
			int r1 = rand()% n;
			int r2 = rand()% n;

			if(r1 != r2) {
				int first = min(r1,r2);
				int second = max(r2,r1);

				if(edgesSet.count({first,second}) <=0) {
					edgesSet.insert({first,second});
					edges.push_back({first,second});
				}
			}

		}

		return edges;

	}
	void datasetGen(){

		for(int m = 10 ;m<=45;m+=5){
		    
		vector<pair<int,int>> edges = generateEdges(10,m);
		
		string filename = "graph_m" + to_string(m) + ".txt";
        ofstream outFile(filename);
        
            
        for (const auto& edge : edges) {
            outFile << edge.first << " " << edge.second << "\n";
        
        }
        
        outFile.close();
		
		}
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
    srand(time(0));

    // datasetGen();

    ofstream csv("results.csv");

    csv << "N,m,cover_size,cover_set,time_microseconds\n";

    for (int m = 10; m <= 45; m += 5) {

        string filename = "graph_m" + to_string(m) + ".txt";

        vector<pair<int,int>> edges;
        int N = 10;

        if (!loadGraph(filename, edges)) continue;

        vector<int> cover;

        auto start = chrono::high_resolution_clock::now();
        int coverSize = vertexCover(edges, 10, cover);
        auto end = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

        string coverStr;

        for (int i = 0; i < cover.size(); ++i) {
            coverStr += to_string(cover[i]);
            if (i != cover.size() - 1) coverStr += " ";
        }

        csv << N << ","
            << edges.size() << ","
            << coverSize << ","
            << "\"" << coverStr << "\","
            << duration.count() << "\n";
    }

    csv.close();

	system("python visualization.py");
	
    return 0;
}
