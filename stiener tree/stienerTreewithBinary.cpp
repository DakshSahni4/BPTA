#include <bits/stdc++.h>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

const int INF = 1000000000;
const int MIN_N = 10;            // smallest vertex count the search will try
const int MAX_N = 40;            // largest vertex count the search will try

double TIME_LIMIT_SEC = 10.0;     // "brute force runs" = finishes within this time

struct WEdge
{
	int u, v, w;
};

// state used to stop the brute force when the time limit is hit
chrono::steady_clock::time_point searchStart;
bool timedOut = false;
unsigned long long leafCounter = 0;

bool timeExceeded()
{
	double elapsed = chrono::duration<double>(chrono::steady_clock::now() - searchStart).count();
	return elapsed > TIME_LIMIT_SEC;
}

// ---------------------------------------------------------------
// Subset generation by recursion: every subset of `vertices`.
// Nothing is stored: at each leaf the subset is handed straight to
// `evaluate` (Kruskal) and then dropped, so memory stays tiny.
// ---------------------------------------------------------------
void evaluateSubset(const vector<int> &subset);   // defined after Kruskal

void helper(int i, vector<int> curr, vector<int> &vertices)
{
	if (timedOut) return;

	if (i == (int)vertices.size())
	{
		evaluateSubset(curr);

		if ((++leafCounter & 4095ULL) == 0 && timeExceeded()) timedOut = true;
		return;
	}

	curr.push_back(vertices[i]);
	helper(i + 1, curr, vertices);

	curr.pop_back();
	helper(i + 1, curr, vertices);
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
// Goes through every edge. Returns the MST cost, or INF if the
// induced subgraph is not connected.
int inducedMst(int N, vector<WEdge> &sortedEdges, vector<bool> &inSet, int setSize, vector<WEdge> &used)
{
	vector<int> parent(N);
	for (int i = 0; i < N; i++) parent[i] = i;

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
// Exact solution (brute force).
// Every subset of the non-terminals is tried as the Steiner vertices:
// take the MST of (terminals + subset) and keep the cheapest one.
// Returns 0 = finished, 1 = time limit hit.
// ---------------------------------------------------------------
// state shared with evaluateSubset (called from the recursion)
int gN = 0, gK = 0, gBest = INF;
vector<WEdge> *gSorted = nullptr;
vector<int> *gTerminals = nullptr;
vector<WEdge> gBestEdges;

void evaluateSubset(const vector<int> &subset)
{
	vector<bool> inSet(gN, false);
	for (int t : *gTerminals) inSet[t] = true;
	for (int s : subset) inSet[s] = true;

	int setSize = gK + subset.size();

	vector<WEdge> used;
	int cost = inducedMst(gN, *gSorted, inSet, setSize, used);

	if (cost < gBest)
	{
		gBest = cost;
		gBestEdges = used;
	}
}

int steinerExact(int N, vector<WEdge> &edges, vector<int> &terminals,
                 int &best, vector<WEdge> &bestEdges, unsigned long long &checked)
{
	int K = terminals.size();

	vector<bool> isTerminal(N, false);
	for (int t : terminals) isTerminal[t] = true;

	vector<int> nonTerminals;
	for (int v = 0; v < N; v++)
		if (!isTerminal[v]) nonTerminals.push_back(v);

	vector<WEdge> sortedEdges = edges;
	sort(sortedEdges.begin(), sortedEdges.end(),
	     [](const WEdge &a, const WEdge &b) { return a.w < b.w; });

	gN = N;
	gK = K;
	gBest = INF;
	gSorted = &sortedEdges;
	gTerminals = &terminals;
	gBestEdges.clear();

	searchStart = chrono::steady_clock::now();
	timedOut = false;
	leafCounter = 0;

	vector<int> curr;
	helper(0, curr, nonTerminals);

	best = gBest;
	bestEdges = gBestEdges;
	checked = leafCounter;

	return timedOut ? 1 : 0;
}

// ---------------------------------------------------------------
// Approximation: MST of the metric closure, then merge shortest paths
// ---------------------------------------------------------------
int steinerApprox(int N, vector<WEdge> &edges, vector<int> &terminals, vector<WEdge> &result)
{
	int K = terminals.size();

	vector<vector<int>> d(N, vector<int>(N, INF));
	vector<vector<int>> W(N, vector<int>(N, INF));
	vector<vector<int>> nxt(N, vector<int>(N, -1));

	for (int i = 0; i < N; i++)
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

	for (int k = 0; k < N; k++)
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
				if (d[i][k] < INF && d[k][j] < INF && d[i][k] + d[k][j] < d[i][j])
				{
					d[i][j] = d[i][k] + d[k][j];
					nxt[i][j] = nxt[i][k];
				}

	// Prim on the metric closure; `order` lists terminals as they join,
	// so a parent always comes before its children.
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

	// Add only the part of each shortest path that is not in the tree yet
	vector<bool> inTree(N, false);
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
// Random instance generation
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

void writeEdges(const string &filename, vector<WEdge> &tree)
{
	ofstream out(filename);
	for (const auto &e : tree)
		out << e.u << " " << e.v << " " << e.w << "\n";
	out.close();
}

// ---------------------------------------------------------------
// One probe of the binary search: random m and K for the given N.
// Returns true if the brute force finished within the limits.
// ---------------------------------------------------------------
int probeCounter = 0;

bool runProbe(int N, ofstream &log)
{
	probeCounter++;
	string id = to_string(probeCounter);

	int maxM = N * (N - 1) / 2;
	int m = (N - 1) + rand() % (maxM - (N - 1) + 1);   // random in [N-1, N(N-1)/2]

	int K = max(2, N / 4);                             // terminal count tied to N/4 (never below 2)

	vector<WEdge> edges = generateEdges(N, m);
	vector<int> terminals = generateTerminals(N, K);

	writeEdges("graph_p" + id + ".txt", edges);

	ofstream termFile("terminals_p" + id + ".txt");
	for (int t : terminals) termFile << t << " ";
	termFile << "\n";
	termFile.close();

	// exact
	vector<WEdge> optTree;
	int optCost = INF;
	unsigned long long checked = 0;

	auto start = chrono::high_resolution_clock::now();
	int status = steinerExact(N, edges, terminals, optCost, optTree, checked);
	auto end = chrono::high_resolution_clock::now();
	auto exactTime = chrono::duration_cast<chrono::microseconds>(end - start);

	bool ok = (status == 0);

	// approximation
	vector<WEdge> approxTree;

	start = chrono::high_resolution_clock::now();
	int approxCost = steinerApprox(N, edges, terminals, approxTree);
	end = chrono::high_resolution_clock::now();
	auto approxTime = chrono::duration_cast<chrono::microseconds>(end - start);

	writeEdges("tree_approx_p" + id + ".txt", approxTree);
	if (ok) writeEdges("tree_opt_p" + id + ".txt", optTree);

	unsigned long long totalSubsets = 1ULL << (N - K);

	string statusName = (status == 0) ? "ok" : (status == 1) ? "timeout" : "memory";

	log << id << "," << N << "," << edges.size() << "," << K << ","
	    << statusName << ","
	    << (ok ? optCost : -1) << ","
	    << approxCost << ","
	    << fixed << setprecision(4) << (ok ? (double)approxCost / optCost : -1.0) << ","
	    << exactTime.count() << "," << approxTime.count() << ","
	    << checked << "," << totalSubsets << "\n";
	log.flush();

	printf("Probe %d: N=%d m=%zu K=%d (2^%d subsets) -> ", probeCounter, N, edges.size(), K, N - K);
	if (status == 0)
		printf("finished in %.3f s, opt=%d approx=%d\n", exactTime.count() / 1e6, optCost, approxCost);
	else if (status == 1)
		printf("TIMED OUT after %.1f s (%llu of %llu subsets checked), approx=%d\n",
		       TIME_LIMIT_SEC, checked, totalSubsets, approxCost);
	else
		printf("SKIPPED, approx=%d\n", approxCost);

	return ok;
}

int main(int argc, char **argv)
{
	unsigned seed = (argc >= 2) ? (unsigned)atoi(argv[1]) : (unsigned)time(0);
	if (argc >= 3) TIME_LIMIT_SEC = atof(argv[2]);

	srand(seed);
	printf("seed=%u  time limit=%.1f s  search range N in [%d, %d]  K = max(2, N/4)\n\n",
	       seed, TIME_LIMIT_SEC, MIN_N, MAX_N);

	ofstream log("search_log.csv");
	log << "probe,N,m,K,status,opt_cost,approx_cost,ratio,exact_time_us,approx_time_us,"
	       "subsets_checked,subsets_total\n";

	// Binary search for the largest N whose brute force finishes in time
	int lo = MIN_N, hi = MAX_N, bestN = -1;
	vector<int> mids;

	while (lo <= hi)
	{
		int mid = (lo + hi) / 2;
		mids.push_back(mid);

		printf("[step %zu] lo=%d hi=%d mid=%d\n", mids.size(), lo, hi, mid);

		if (runProbe(mid, log))
		{
			printf("          mid=%d finished -> search higher (lo = %d)\n\n", mid, mid + 1);
			bestN = mid;
			lo = mid + 1;
		}
		else
		{
			printf("          mid=%d failed -> search lower (hi = %d)\n\n", mid, mid - 1);
			hi = mid - 1;
		}
	}

	printf("All mid values tried:");
	for (int m : mids) printf(" %d", m);
	printf("\n");

	log.close();

	if (bestN == -1)
		printf("\nBrute force did not finish even for N=%d.\n", MIN_N);
	else
		printf("\nHighest N for which brute force finished within %.1f s: %d\n", TIME_LIMIT_SEC, bestN);

	system("python visualize_search.py");

	return 0;
}
