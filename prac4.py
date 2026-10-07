import csv
import time
import numpy as np
from scipy.optimize import linprog

def read_graph(filename):
    edges = []
    with open(filename, "r") as file:
        for line in file:
            if line.strip():
                u, v = map(int, line.split())
                edges.append((u, v))
    return edges
def read_greedy_results(filename):
    results = {}
    with open(filename, "r", newline="") as file:
        reader = csv.DictReader(file)
        for row in reader:
            m = int(row["m"])
            results[m] = {
                "size": int(row["approximate_cover_size"]),
                "time": int(row["approx_time_microseconds"])
            }
    return results


def solve_lp(n, edges):
    # print(n)
    objective = np.ones(n)
    A = []
    b = []
    for u, v in edges:
        row = [0] * n
        row[u] = -1
        row[v] = -1
        A.append(row)
        b.append(-1)
    start = time.perf_counter()
    result = linprog(
        objective,
        A_ub=A ,
        b_ub=b ,
        bounds=[(0, 1)] * n,
        method="highs"
    )
    lp_optimal = result.fun
    rounded_cover = []
    for v in range(n):
        print(result.x[v],v)
        if result.x[v] >= 0.5:
            rounded_cover.append(v)
    total_time = time.perf_counter() - start
    return lp_optimal, rounded_cover, total_time


def main():
    experiments = [
        (
            10,
            list(range(10, 46, 5)),
            "results.csv",
            "graph_m{}.txt"
        ),
        (
            20,
            list(range(20, 181, 20)) + [190],
            "p3_results.csv",
            "graph_n20_m{}.txt"
        )
    ]
    output = open(
        "practical4_results.csv",
        "w",
        newline=""
    )
    columns = [
        "n",
        "m",
        "Greedy VC Size",
        "Greedy Time (microseconds)",
        "LP Optimal",
        "LP Total Time (microseconds)",
        "LP Rounded VC Size",
        "Approximation Factor"
    ]
    writer = csv.DictWriter(
        output,
        fieldnames=columns
    )

    writer.writeheader()

    for n, m_values, greedy_file, graph_pattern in experiments:

        greedy_results = read_greedy_results(greedy_file)

        for m in m_values:

            graph_file = graph_pattern.format(m)

            edges = read_graph(graph_file)

            greedy_size = greedy_results[m]["size"]
            greedy_time = greedy_results[m]["time"]

            lp_optimal, rounded_cover, lp_time = solve_lp(n,edges)

            rounded_size = len(rounded_cover)

            greedy_factor = (greedy_size / lp_optimal)

            writer.writerow({
                "n": n,
                "m": len(edges),
                "Greedy VC Size": greedy_size,
                "Greedy Time (microseconds)": greedy_time,
                "LP Optimal": round(lp_optimal, 6),
                "LP Total Time (microseconds)": round(
                    lp_time * 1_000_000,
                    2
                ),
                "LP Rounded VC Size": rounded_size,
                "Approximation Factor": round(
                    greedy_factor,
                    4
                )
            })

    output.close()

if __name__ == "__main__":
    main()
