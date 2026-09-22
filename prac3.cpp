#include <bits/stdc++.h>
#include <ctime>
#include <chrono>

using namespace std;

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

void helper(int i, vector<int> curr, vector<vector<int>> &subsets, vector<int>& vertices)
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
	vector<int> curr;
	vector<vector<int>> subsets;

	vector<int> vertices ;

	for (int j = 0; j < N; j++) vertices.push_back(j);

	helper(i, curr, subsets, vertices);

	return subsets;
}

bool checkinsubset(int u, vector<int>& subset)
{
	for (auto i : subset)
		if (u == i)
			return true;

	return false;
}

bool isVc(vector<int>& subset, vector<pair<int, int>>& edgeList)
{
	for (auto edge : edgeList)
	{
		if (!checkinsubset(edge.first, subset) && !checkinsubset(edge.second, subset))
			return false;
	}
	return true;
}
int vertexCover(vector<pair<int, int>>& edgeList, int N, vector<int> &ans)
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
int main(){
    srand(time(0));

    int n = 20;
    int maxM = n*(n-1)/2;
    int m = rand() % maxM;
    m++;

    vector<pair<int,int>> edges = generateEdges(n,m);

    vector<int> bfaVC;

    auto startBfa = chrono::high_resolution_clock::now();
    int bfaVcSize =  vertexCover(edges,n,bfaVC);
    auto endBfa = chrono::high_resolution_clock::now();

	auto durationBfa = chrono::duration_cast<chrono::microseconds>(endBfa - startBfa).count();
    
    vector<pair<int,int>> matching;
	
    auto startAprox = chrono::high_resolution_clock::now();
    vector<int> aproxVc = approximation(edges,n,matching);
	auto endAprox = chrono::high_resolution_clock::now();
	
    int aproxVcSize = aproxVc.size();

	auto durationAprox = chrono::duration_cast<chrono::microseconds>(endAprox - startAprox).count();
    double aproxFactor = (double) aproxVcSize / bfaVcSize;

	string filename = "graph_n" + to_string(n) + ".txt";

    ofstream outFile(filename);

	for (const auto& edge : edges) outFile << edge.first << " " << edge.second << "\n";

	outFile.close();

	filename = "vertex_cover_n" + to_string(n) + ".txt";

	ofstream vcFile(filename);

	vcFile<<"BFA Vertex Cover:"<<endl;

	for(auto& node : bfaVC) vcFile<<node<<" ";
	vcFile<<endl;

	vcFile<<"Aproximation Vertex Cover:"<<endl;

	for(auto& node : aproxVc) vcFile<<node<<" ";

	vcFile.close();

	ofstream csv("p3_results.csv");

	csv << "N,m,cover_size,time_microseconds,"
           << "approximate_cover_size,"
           << "approximation_factor,approx_time_microseconds\n";

	csv << n << ","
               << m << ","
               << bfaVcSize << ","
               << durationBfa << ","
               << aproxVcSize << ","
               << fixed << setprecision(2)
               << aproxFactor << ","
               << durationAprox
               << "\n";
	csv.close();
}