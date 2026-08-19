#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>

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
		if (isVc(subsets[i], edgeList) && best> subsetSize)
		{
			best = min(best, subsetSize);
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
			while(randomValue == i);

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

	int main()
	{
		srand(time(0));

		for(int m = 10 ;m<=45;m+=5){
		    
		vector<pair<int,int>> edges = generateEdges(10,m);
		
		string filename = "graph_m" + to_string(m) + ".txt";
        ofstream outFile(filename);
        
            
        for (const auto& edge : edges) {
            outFile << edge.first << " " << edge.second << "\n";
        
        }
        outFile<<"vertex Cover"<<"\n";
        
        vector<int> subsets;
		
		vertexCover(edges,10,subsets);
		
	for (const auto& node:subsets){
            outFile << node<<" ";
        }
        outFile<<"\n";
		
		
        outFile.close();
		
		for(auto edge : edges) cout<<edge.first<<" "<<edge.second<<endl;
		cout<<edges.size();cout<<endl;
		
		}

	}
