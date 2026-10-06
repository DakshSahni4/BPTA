# Practical 4: Vertex Cover using LP rounding
# Greedy results are read from the CSV files made in Practicals 2 and 3.
# The greedy algorithm is NOT run again.

import csv
import time
from itertools import combinations
import numpy as np
from scipy.optimize import linprog

# Read a graph file: each line contains two vertices, e.g. 0 3
def read_graph(filename):
    edges = []
    with open(filename, "r") as file:
        for line in file:
            if line.strip():
                u, v = map(int, line.split())
                edges.append((u, v))
    return edges


# Read previously saved greedy sizes and times
def read_greedy_results(filename):
    results = {}
    with open(filename, "r", newline="") as file:
        for row in csv.DictReader(file):
            m = int(row["m"])
            results[m] = {
                "size": int(row["approximate_cover_size"]),
                "time": float(row["approx_time_microseconds"])
            }
    return results


# Solve the fractional Vertex Cover LP using scipy.optimize.linprog
def is_vertex_cover(vertices, edges):
    """Return True if every edge has at least one endpoint in vertices."""
    chosen = set(vertices)
    for u, v in edges:
        if u not in chosen and v not in chosen:
            return False
    return True


def solve_lp(n, edges):
    # Minimize x0 + x1 + ... + x(n-1)
    objective = np.ones(n)

    # x[u] + x[v] >= 1 becomes -x[u] - x[v] <= -1
    A = []
    b = []

    for u, v in edges:
        row = [0] * n
        row[u] = -1
        row[v] = -1
        A.append(row)
        b.append(-1)

    # Measure the whole LP process:
    # solve LP, round the solution, then search subsets of the rounded cover.
    start = time.perf_counter()

    result = linprog(
        objective,
        A_ub=A if edges else None,
        b_ub=b if edges else None,
        bounds=[(0, 1)] * n,
        method="highs"
    )

    if not result.success:
        raise Exception("LP solver failed: " + result.message)

    # Round: choose vertices whose LP value is at least 0.5
    lp_rounded_cover = []
    for v in range(n):
        if result.x[v] >= 0.5 - 1e-9:
            lp_rounded_cover.append(v)

    # Find the smallest subset of the rounded vertices that is still a
    # valid vertex cover. Try subsets from smallest to largest.
    lp_cover = set()
    for size in range(len(lp_rounded_cover) + 1):
        found = False
        for subset in combinations(lp_rounded_cover, size):
            if is_vertex_cover(subset, edges):
                lp_cover = set(subset)
                found = True
                break
        if found:
            break

    lp_total_time = time.perf_counter() - start

    # Fractional LP optimum and the minimum cover found among rounded vertices.
    lp_optimal_value = result.fun
    lp_solution = [round(value, 6) for value in result.x]

    return lp_optimal_value, lp_solution, set(lp_rounded_cover), lp_cover, lp_total_time


def main():
    # n=10 uses graph_m*.txt and results.csv from Practical 2.
    # n=20 uses graph_n20_m*.txt and p3_results.csv from Practical 3.
    experiments = [
        (10, list(range(10, 46, 5)), "results.csv", "graph_m{}.txt"),
        (20, list(range(20, 181, 20)) + [190],
         "p3_results.csv", "graph_n20_m{}.txt")
    ]

    output = open("practical4_results.csv", "w", newline="")
    columns = [
        "n",
        "m",
        "Greedy VC Size",
        "Greedy Time (microseconds)",
        "LP Optimal",
        "LP Total Time (microseconds)",
        "LP Rounded VC Size",
        "Approximation Factor",
        "LP Rounding Factor"
    ]
    writer = csv.DictWriter(output, fieldnames=columns)
    writer.writeheader()

    for n, m_values, greedy_file, graph_pattern in experiments:
        greedy_results = read_greedy_results(greedy_file)

        for m in m_values:
            graph_file = graph_pattern.format(m)

            try:
                edges = read_graph(graph_file)
            except FileNotFoundError:
                print("Skipping missing graph:", graph_file)
                continue

            if m not in greedy_results:
                print("No stored greedy result for n =", n, "m =", m)
                continue

            # Reuse the saved greedy result; do not run greedy again.
            greedy_size = greedy_results[m]["size"]
            greedy_time = greedy_results[m]["time"]

            fractional_lp_value, lp_solution, rounded_cover, lp_cover, lp_time = solve_lp(n, edges)

            # Following the requested table, "LP Optimal" is the size of the
            # smallest valid vertex cover found by checking subsets of the
            # threshold-rounded LP vertices. The fractional LP objective is
            # The fractional objective is available internally as fractional_lp_value.
            lp_optimal_size = len(lp_cover)
            greedy_factor = greedy_size / lp_optimal_size if lp_optimal_size else 1
            lp_factor = len(rounded_cover) / lp_optimal_size if lp_optimal_size else 1

            writer.writerow({
                "n": n,
                "m": len(edges),
                "Greedy VC Size": greedy_size,
                "Greedy Time (microseconds)": greedy_time,
                "LP Optimal": lp_optimal_size,
                "LP Total Time (microseconds)": round(lp_time * 1_000_000, 2),
                "LP Rounded VC Size": len(rounded_cover),
                "Approximation Factor": round(greedy_factor, 4),
                "LP Rounding Factor": round(lp_factor, 4)
            })

            print(
                "n =", n, "m =", len(edges),
                "| greedy size =", greedy_size,
                "| LP optimal cover size =", lp_optimal_size,
                "| rounded size =", len(rounded_cover),
                "| greedy factor =", round(greedy_factor, 3)
            )

    output.close()
    print("Finished. Results saved in practical4_results.csv")


if __name__ == "__main__":
    main()
