#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

const int INF = 1000000000;
const int NODES = 10;          // number of vertices in every generated graph
const int NUM_TERMINALS = 4;   // number of terminals chosen per graph

struct WEdge
{
	int u, v, w;
};

// ---------------------------------------------------------------
// Subset generation (used by the exact brute force)
// ---------------------------------------------------------------
void helper(int i, vector<int> curr, vector<vector<int>> &subsets, vector<int> &vertices)
{
	if (i == (int)vertices.size())
	{
		subsets.push_back(curr);
		return;
	}

	curr.push_back(vertices[i]);
	helper(i + 1, curr, subsets, vertices);

	curr.pop_back();
	helper(i + 1, curr, subsets, vertices);
}

vector<vector<int>> generateSubsets(vector<int> &vertices)
{
	vector<int> curr;
	vector<vector<int>> subsets;

	helper(0, curr, subsets, vertices);

	return subsets;
}

// ---------------------------------------------------------------
// Union-Find for Kruskal
// ---------------------------------------------------------------
int findSet(vector<int> &parent, int x)
{
	if (parent[x] == x) return x;
	return parent[x] = findSet(parent, parent[x]);
}

// Kruskal on the subgraph induced by the vertices marked in inSet.
// Returns the MST cost, or INF if that subgraph is not connected.
int inducedMst(vector<WEdge> &sortedEdges, vector<bool> &inSet, int setSize, vector<WEdge> &used)
{
	vector<int> parent(NODES);
	for (int i = 0; i < NODES; i++) parent[i] = i;

	int cost = 0, count = 0;
	used.clear();

	for (auto e : sortedEdges)
	{
		if (!inSet[e.u] || !inSet[e.v]) continue;

		int a = findSet(parent, e.u);
		int b = findSet(parent, e.v);

		if (a != b)
		{
			parent[a] = b;
			cost += e.w;
			count++;
			used.push_back(e);
		}
	}

	if (count != setSize - 1) return INF;
	return cost;
}

// ---------------------------------------------------------------
// Exact solution: try every subset of non-terminals as the Steiner
// vertices, take the MST of (terminals + subset), keep the cheapest.
// ---------------------------------------------------------------
int steinerExact(vector<WEdge> &edges, vector<int> &terminals, vector<WEdge> &bestEdges)
{
	vector<bool> isTerminal(NODES, false);
	for (int t : terminals) isTerminal[t] = true;

	vector<int> nonTerminals;
	for (int v = 0; v < NODES; v++)
		if (!isTerminal[v]) nonTerminals.push_back(v);

	vector<WEdge> sortedEdges = edges;
	sort(sortedEdges.begin(), sortedEdges.end(),
	     [](const WEdge &a, const WEdge &b) { return a.w < b.w; });

	vector<vector<int>> subsets = generateSubsets(nonTerminals);

	int best = INF;

	for (int i = 0; i < (int)subsets.size(); i++)
	{
		vector<bool> inSet(NODES, false);
		for (int t : terminals) inSet[t] = true;
		for (int s : subsets[i]) inSet[s] = true;

		int setSize = terminals.size() + subsets[i].size();

		vector<WEdge> used;
		int cost = inducedMst(sortedEdges, inSet, setSize, used);

		if (cost < best)
		{
			best = cost;
			bestEdges = used;
		}
	}

	return best;
}

// ---------------------------------------------------------------
// Approximation: MST of the metric closure, then merge shortest paths
// ---------------------------------------------------------------
int steinerApprox(vector<WEdge> &edges, vector<int> &terminals, vector<WEdge> &result)
{
	int K = terminals.size();

	// All-pairs shortest paths (Floyd-Warshall) with path reconstruction
	vector<vector<int>> d(NODES, vector<int>(NODES, INF));
	vector<vector<int>> W(NODES, vector<int>(NODES, INF));
	vector<vector<int>> nxt(NODES, vector<int>(NODES, -1));

	for (int i = 0; i < NODES; i++)
	{
		d[i][i] = 0;
		nxt[i][i] = i;
	}

	for (auto e : edges)
	{
		d[e.u][e.v] = d[e.v][e.u] = e.w;
		W[e.u][e.v] = W[e.v][e.u] = e.w;
		nxt[e.u][e.v] = e.v;
		nxt[e.v][e.u] = e.u;
	}

	for (int k = 0; k < NODES; k++)
		for (int i = 0; i < NODES; i++)
			for (int j = 0; j < NODES; j++)
				if (d[i][k] < INF && d[k][j] < INF && d[i][k] + d[k][j] < d[i][j])
				{
					d[i][j] = d[i][k] + d[k][j];
					nxt[i][j] = nxt[i][k];
				}

	// Prim's MST on the metric closure (terminals only).
	// `order` lists terminals in the order they join, so a parent always
	// comes before its children.
	vector<int> par(K, -1), key(K, INF), order;
	vector<bool> done(K, false);
	key[0] = 0;

	for (int it = 0; it < K; it++)
	{
		int u = -1;
		for (int i = 0; i < K; i++)
			if (!done[i] && (u == -1 || key[i] < key[u])) u = i;

		done[u] = true;
		order.push_back(u);

		for (int v = 0; v < K; v++)
		{
			int dist = d[terminals[u]][terminals[v]];
			if (!done[v] && dist < key[v])
			{
				key[v] = dist;
				par[v] = u;
			}
		}
	}

	// Add the part of each shortest path that is not in the tree yet
	vector<bool> inTree(NODES, false);
	inTree[terminals[0]] = true;

	int cost = 0;
	result.clear();

	for (int idx = 1; idx < K; idx++)
	{
		int c = order[idx];
		int p = par[c];

		vector<int> path;
		int x = terminals[p];
		path.push_back(x);
		while (x != terminals[c])
		{
			x = nxt[x][terminals[c]];
			path.push_back(x);
		}

		int last = 0;
		for (int i = 0; i < (int)path.size(); i++)
			if (inTree[path[i]]) last = i;

		for (int i = last; i + 1 < (int)path.size(); i++)
		{
			int a = path[i], b = path[i + 1];
			result.push_back({a, b, W[a][b]});
			cost += W[a][b];
			inTree[a] = true;
			inTree[b] = true;
		}
	}

	return cost;
}

// ---------------------------------------------------------------
// Dataset generation
// ---------------------------------------------------------------
vector<WEdge> generateEdges(int n, int m)
{
	vector<WEdge> edges;
	set<pair<int, int>> edgesSet;

	m = min(m, n * (n - 1) / 2);

	// random spanning tree first, so the graph is always connected
	for (int i = 1; i < n; i++)
	{
		int randomValue = rand() % i;

		int first = min(i, randomValue);
		int second = max(i, randomValue);

		edgesSet.insert({first, second});
		edges.push_back({first, second, rand() % 20 + 1});
	}

	while ((int)edgesSet.size() != m)
	{
		int r1 = rand() % n;
		int r2 = rand() % n;

		if (r1 != r2)
		{
			int first = min(r1, r2);
			int second = max(r1, r2);

			if (edgesSet.count({first, second}) <= 0)
			{
				edgesSet.insert({first, second});
				edges.push_back({first, second, rand() % 20 + 1});
			}
		}
	}

	return edges;
}

vector<int> generateTerminals(int n, int k)
{
	vector<int> all;
	for (int i = 0; i < n; i++) all.push_back(i);

	for (int i = n - 1; i > 0; i--) swap(all[i], all[rand() % (i + 1)]);

	vector<int> terminals(all.begin(), all.begin() + k);
	sort(terminals.begin(), terminals.end());
	return terminals;
}

void datasetGen()
{
	for (int m = 10; m <= 45; m += 5)
	{
		vector<WEdge> edges = generateEdges(NODES, m);
		vector<int> terminals = generateTerminals(NODES, NUM_TERMINALS);

		ofstream outFile("graph_m" + to_string(m) + ".txt");
		for (const auto &e : edges)
			outFile << e.u << " " << e.v << " " << e.w << "\n";
		outFile.close();

		ofstream termFile("terminals_m" + to_string(m) + ".txt");
		for (int t : terminals) termFile << t << " ";
		termFile << "\n";
		termFile.close();
	}
}

bool loadGraph(const string &filename, vector<WEdge> &edges)
{
	ifstream inFile(filename);
	if (!inFile.is_open()) return false;

	edges.clear();

	int u, v, w;
	while (inFile >> u >> v >> w)
		edges.push_back({u, v, w});

	inFile.close();
	return true;
}

bool loadTerminals(const string &filename, vector<int> &terminals)
{
	ifstream inFile(filename);
	if (!inFile.is_open()) return false;

	terminals.clear();

	int t;
	while (inFile >> t) terminals.push_back(t);

	inFile.close();
	return true;
}

void writeTree(const string &filename, vector<WEdge> &tree)
{
	ofstream out(filename);
	for (const auto &e : tree)
		out << e.u << " " << e.v << " " << e.w << "\n";
	out.close();
}

int main()
{
	srand(time(0));

	// generate the dataset only if it does not exist yet,
	// so repeated runs reuse the same graphs
	ifstream probe("graph_m10.txt");
	if (!probe.is_open()) datasetGen();
	probe.close();

	ofstream csv("results.csv");

	csv << "N,m,K,opt_cost,approx_cost,ratio,exact_time_us,approx_time_us\n";

	for (int m = 10; m <= 45; m += 5)
	{
		string graphFile = "graph_m" + to_string(m) + ".txt";
		string termFile = "terminals_m" + to_string(m) + ".txt";

		vector<WEdge> edges;
		vector<int> terminals;

		if (!loadGraph(graphFile, edges)) continue;
		if (!loadTerminals(termFile, terminals)) continue;

		vector<WEdge> optTree, approxTree;

		auto start = chrono::high_resolution_clock::now();
		int optCost = steinerExact(edges, terminals, optTree);
		auto end = chrono::high_resolution_clock::now();
		auto exactTime = chrono::duration_cast<chrono::microseconds>(end - start);

		start = chrono::high_resolution_clock::now();
		int approxCost = steinerApprox(edges, terminals, approxTree);
		end = chrono::high_resolution_clock::now();
		auto approxTime = chrono::duration_cast<chrono::microseconds>(end - start);

		writeTree("tree_opt_m" + to_string(m) + ".txt", optTree);
		writeTree("tree_approx_m" + to_string(m) + ".txt", approxTree);

		csv << NODES << ","
		    << edges.size() << ","
		    << terminals.size() << ","
		    << optCost << ","
		    << approxCost << ","
		    << fixed << setprecision(4) << (double)approxCost / optCost << ","
		    << exactTime.count() << ","
		    << approxTime.count() << "\n";
	}

	csv.close();

	system("python visualization_stiener.py");

	return 0;
}