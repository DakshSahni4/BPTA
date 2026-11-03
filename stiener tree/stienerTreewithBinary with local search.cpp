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
// Heuristic: Local Search
// A tree is represented by its vertex set. Any vertex set is turned
// into a tree by: induced MST (Kruskal) -> prune non-terminal leaves.
// ---------------------------------------------------------------
void allPairsShortestPaths(int N, vector<WEdge> &edges, vector<vector<int>> &d, vector<vector<int>> &nxt)
{
	d.assign(N, vector<int>(N, INF));
	nxt.assign(N, vector<int>(N, -1));

	for (int i = 0; i < N; i++)
	{
		d[i][i] = 0;
		nxt[i][i] = i;
	}
	for (auto e : edges)
	{
		d[e.u][e.v] = d[e.v][e.u] = e.w;
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
}

// vertex set -> valid Steiner tree (induced MST, then prune useless leaves).
// Returns false if the set does not induce a connected subgraph.
bool buildTree(int N, vector<WEdge> &sortedEdges, vector<bool> &isTerm, vector<bool> &inSet,
               vector<WEdge> &tree, int &cost)
{
	int setSize = 0;
	for (int v = 0; v < N; v++) if (inSet[v]) setSize++;

	vector<WEdge> used;
	if (inducedMst(N, sortedEdges, inSet, setSize, used) >= INF) return false;

	int M = used.size();
	vector<vector<int>> inc(N);
	vector<int> deg(N, 0);
	vector<bool> alive(M, true);

	for (int i = 0; i < M; i++)
	{
		inc[used[i].u].push_back(i);
		inc[used[i].v].push_back(i);
		deg[used[i].u]++;
		deg[used[i].v]++;
	}

	vector<int> stk;
	for (int v = 0; v < N; v++)
		if (inSet[v] && !isTerm[v] && deg[v] == 1) stk.push_back(v);

	while (!stk.empty())
	{
		int v = stk.back();
		stk.pop_back();
		if (deg[v] != 1) continue;

		for (int id : inc[v])
		{
			if (!alive[id]) continue;
			alive[id] = false;
			int o = (used[id].u == v) ? used[id].v : used[id].u;
			deg[v]--;
			deg[o]--;
			if (!isTerm[o] && deg[o] == 1) stk.push_back(o);
		}
	}

	tree.clear();
	cost = 0;
	for (int i = 0; i < M; i++)
		if (alive[i])
		{
			tree.push_back(used[i]);
			cost += used[i].w;
		}
	return true;
}

// Connect all components of the kept vertices with shortest paths
// (repeatedly joins the two closest different components).
void reconnect(int N, vector<vector<int>> &d, vector<vector<int>> &nxt,
               vector<bool> &keep, const vector<WEdge> &remEdges)
{
	vector<int> par(N);
	for (int i = 0; i < N; i++) par[i] = i;

	for (auto &e : remEdges)
		par[findSet(par, e.u)] = findSet(par, e.v);

	while (true)
	{
		int ba = -1, bb = -1, bd = INF;
		for (int a = 0; a < N; a++)
		{
			if (!keep[a]) continue;
			for (int b = a + 1; b < N; b++)
			{
				if (!keep[b]) continue;
				if (findSet(par, a) == findSet(par, b)) continue;
				if (d[a][b] < bd)
				{
					bd = d[a][b];
					ba = a;
					bb = b;
				}
			}
		}
		if (ba == -1) break;

		int x = ba;
		while (x != bb)
		{
			int y = nxt[x][bb];
			keep[y] = true;
			par[findSet(par, x)] = findSet(par, y);
			x = y;
		}
	}
}

struct KeyPath
{
	int a, b;                          // key-vertex endpoints
	vector<int> interior;              // degree-2 Steiner vertices between them
	set<pair<int, int>> edgeSet;       // edges of the path (u < v)
};

// Key vertices = terminals and Steiner vertices of degree >= 3.
// Key path = path between two key vertices whose inner vertices are not key.
vector<KeyPath> findKeyPaths(int N, vector<WEdge> &tree, vector<bool> &isTerm, vector<int> &deg)
{
	vector<vector<int>> adj(N);
	for (auto &e : tree)
	{
		adj[e.u].push_back(e.v);
		adj[e.v].push_back(e.u);
	}
	deg.assign(N, 0);
	for (int v = 0; v < N; v++) deg[v] = adj[v].size();

	auto isKey = [&](int v) { return isTerm[v] || deg[v] >= 3; };

	vector<KeyPath> paths;
	for (int v = 0; v < N; v++)
	{
		if (deg[v] == 0 || !isKey(v)) continue;

		for (int nb : adj[v])
		{
			KeyPath p;
			p.a = v;
			int prev = v, cur = nb;
			p.edgeSet.insert({min(prev, cur), max(prev, cur)});

			while (!isKey(cur))
			{
				p.interior.push_back(cur);
				int next = (adj[cur][0] == prev) ? adj[cur][1] : adj[cur][0];
				prev = cur;
				cur = next;
				p.edgeSet.insert({min(prev, cur), max(prev, cur)});
			}
			p.b = cur;

			if (p.a < p.b) paths.push_back(p);   // every path is found from both ends, keep one
		}
	}
	return paths;
}

// Local search: Vertex Insertion, Vertex Elimination, Key-Path Exchange and
// Key-Vertex Elimination. Takes the best valid candidate each round and
// moves to it only if it is strictly cheaper.
int steinerLocalSearch(int N, vector<WEdge> &edges, vector<int> &terminals,
                       vector<WEdge> &result, int &iterations, int maxIterations = 1000)
{
	vector<bool> isTerm(N, false);
	for (int t : terminals) isTerm[t] = true;

	vector<WEdge> sortedEdges = edges;
	sort(sortedEdges.begin(), sortedEdges.end(),
	     [](const WEdge &a, const WEdge &b) { return a.w < b.w; });

	vector<vector<int>> d, nxt;
	allPairsShortestPaths(N, edges, d, nxt);

	// 1. initial tree from shortest paths
	vector<WEdge> init;
	steinerApprox(N, edges, terminals, init);

	vector<bool> inSet(N, false);
	for (auto &e : init) inSet[e.u] = inSet[e.v] = true;
	for (int t : terminals) inSet[t] = true;

	vector<WEdge> T;
	int costT = INF;
	buildTree(N, sortedEdges, isTerm, inSet, T, costT);

	// 2-3. improve until no candidate is better
	iterations = 0;
	while (iterations < maxIterations)
	{
		iterations++;

		vector<bool> inT(N, false);
		for (auto &e : T) inT[e.u] = inT[e.v] = true;
		for (int t : terminals) inT[t] = true;

		vector<WEdge> bestTree;
		int bestCost = INF;

		auto consider = [&](vector<bool> &s)
		{
			vector<WEdge> cand;
			int c;
			if (buildTree(N, sortedEdges, isTerm, s, cand, c) && c < bestCost)
			{
				bestCost = c;
				bestTree = cand;
			}
		};

		// Vertex Insertion
		for (int v = 0; v < N; v++)
		{
			if (inT[v]) continue;
			vector<bool> s = inT;
			s[v] = true;
			consider(s);
		}

		// Vertex Elimination
		for (int v = 0; v < N; v++)
		{
			if (!inT[v] || isTerm[v]) continue;
			vector<bool> s = inT;
			s[v] = false;
			consider(s);
		}

		vector<int> deg;
		vector<KeyPath> kp = findKeyPaths(N, T, isTerm, deg);

		// Key-Path Exchange: drop a key path, reconnect the two halves
		// with the shortest connection between them
		for (auto &p : kp)
		{
			vector<bool> s = inT;
			for (int v : p.interior) s[v] = false;

			vector<WEdge> rem;
			for (auto &e : T)
				if (!p.edgeSet.count({min(e.u, e.v), max(e.u, e.v)})) rem.push_back(e);

			reconnect(N, d, nxt, s, rem);
			consider(s);
		}

		// Key-Vertex Elimination: drop a Steiner key vertex together with all
		// its key paths, reconnect the pieces with shortest paths
		for (int v = 0; v < N; v++)
		{
			if (!inT[v] || isTerm[v] || deg[v] < 3) continue;

			vector<bool> s = inT;
			s[v] = false;
			set<pair<int, int>> removed;

			for (auto &p : kp)
			{
				if (p.a != v && p.b != v) continue;
				for (int x : p.interior) s[x] = false;
				removed.insert(p.edgeSet.begin(), p.edgeSet.end());
			}

			vector<WEdge> rem;
			for (auto &e : T)
				if (!removed.count({min(e.u, e.v), max(e.u, e.v)})) rem.push_back(e);

			reconnect(N, d, nxt, s, rem);
			consider(s);
		}

		// accept only if strictly better, otherwise stop
		if (bestCost < costT)
		{
			T = bestTree;
			costT = bestCost;
		}
		else
			break;
	}

	result = T;
	return costT;
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

	// heuristic (local search, starts from the shortest-path tree)
	vector<WEdge> heurTree;
	int localIters = 0;

	start = chrono::high_resolution_clock::now();
	int heurCost = steinerLocalSearch(N, edges, terminals, heurTree, localIters);
	end = chrono::high_resolution_clock::now();
	auto heurTime = chrono::duration_cast<chrono::microseconds>(end - start);

	writeEdges("tree_approx_p" + id + ".txt", approxTree);
	writeEdges("tree_heur_p" + id + ".txt", heurTree);
	if (ok) writeEdges("tree_opt_p" + id + ".txt", optTree);

	unsigned long long totalSubsets = 1ULL << (N - K);

	string statusName = (status == 0) ? "ok" : (status == 1) ? "timeout" : "memory";

	log << id << "," << N << "," << edges.size() << "," << K << ","
	    << statusName << ","
	    << (ok ? optCost : -1) << ","
	    << approxCost << "," << heurCost << ","
	    << fixed << setprecision(4) << (ok ? (double)approxCost / optCost : -1.0) << ","
	    << (ok ? (double)heurCost / optCost : -1.0) << ","
	    << exactTime.count() << "," << approxTime.count() << "," << heurTime.count() << ","
	    << checked << "," << totalSubsets << "\n";
	log.flush();

	printf("Probe %d: N=%d m=%zu K=%d (2^%d subsets) -> ", probeCounter, N, edges.size(), K, N - K);
	if (status == 0)
		printf("finished in %.3f s, opt=%d approx=%d heur=%d\n", exactTime.count() / 1e6, optCost, approxCost, heurCost);
	else if (status == 1)
		printf("TIMED OUT after %.1f s (%llu of %llu subsets checked), approx=%d heur=%d\n",
		       TIME_LIMIT_SEC, checked, totalSubsets, approxCost, heurCost);
	else
		printf("SKIPPED, approx=%d heur=%d\n", approxCost, heurCost);

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
	log << "probe,N,m,K,status,opt_cost,approx_cost,heur_cost,approx_ratio,heur_ratio,"
	       "exact_time_us,approx_time_us,heur_time_us,"
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

	system("python visualize_searchwithlocal.py");

	return 0;
}