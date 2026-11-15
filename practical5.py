import os, time, csv, math
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn.preprocessing import StandardScaler
from sklearn.datasets import load_iris, load_wine, load_breast_cancer, load_digits, load_diabetes

script_dir = os.path.dirname(os.path.abspath(__file__))


def distance(a, b):
    total = 0.0
    for i in range(len(a)):
        diff = a[i] - b[i]
        total += diff * diff
    return math.sqrt(total)


def nearest_center(p, centers):
    best_idx, best_d = 0, float("inf")
    for j in range(len(centers)):
        d = distance(p, centers[j])
        if d < best_d:
            best_d, best_idx = d, j
    return best_idx, best_d


def kcenter_cost(points, centers):
    worst = 0.0
    for p in points:
        _, d = nearest_center(p, centers)
        if d > worst:
            worst = d
    return worst


def library_heuristic(points, k):
    t0 = time.perf_counter()
    km = KMeans(n_clusters=k, n_init=10, random_state=0).fit(points)
    chosen = []
    for c in km.cluster_centers_:
        best_idx, _ = nearest_center(c, points)
        if best_idx not in chosen:
            chosen.append(best_idx)
    centers = points[chosen]
    elapsed = (time.perf_counter() - t0) * 1e6
    return centers, kcenter_cost(points, centers), elapsed


def greedy_kcenter(points, k):
    t0 = time.perf_counter()
    n = len(points)
    chosen = [0]
    dist = [distance(points[i], points[0]) for i in range(n)]
    for _ in range(k - 1):
        far_idx, far_d = 0, -1.0
        for i in range(n):
            if dist[i] > far_d:
                far_d, far_idx = dist[i], i
        chosen.append(far_idx)
        for i in range(n):
            d = distance(points[i], points[far_idx])
            if d < dist[i]:
                dist[i] = d
    elapsed = (time.perf_counter() - t0) * 1e6
    centers = points[chosen]
    return centers, kcenter_cost(points, centers), elapsed


def run_toy():
    pts = np.array([[1, 1], [2, 1], [1, 2], [2, 2],
                    [8, 8], [9, 8], [8, 9], [9, 9],
                    [15, 1]], dtype=float)
    k = 3
    _, c_lib, t_lib = library_heuristic(pts, k)
    g_centers, c_gr, t_gr = greedy_kcenter(pts, k)

    rng = np.random.default_rng(0)
    rp = rng.uniform(0, 10, size=(20, 2))
    _, c1, t1 = library_heuristic(rp, 3)
    _, c2, t2 = greedy_kcenter(rp, 3)

    plt.figure(figsize=(6, 5))
    plt.scatter(pts[:, 0], pts[:, 1], c="gray", s=60)
    plt.scatter(g_centers[:, 0], g_centers[:, 1], c="orange", marker="*", s=300, edgecolors="black")
    for c in g_centers:
        plt.gca().add_patch(plt.Circle(c, c_gr, fill=False, ls="--", color="orange"))
    plt.gca().set_aspect("equal")
    plt.title(f"Toy data, greedy centers (cost {c_gr:.2f})")
    plt.savefig(os.path.join(script_dir, "kcenter_toy.png"), dpi=150)
    plt.close()

    return [["Toy", k, round(c_lib, 3), round(t_lib, 1),
             round(c_gr, 3), round(t_gr, 1), round(c_gr / c_lib, 2)],
            ["Random", 3, round(c1, 3), round(t1, 1),
             round(c2, 3), round(t2, 1), round(c2 / c1, 2)]]


def run_real(rows):
    datasets = {
        "Iris": load_iris(),
        "Wine": load_wine(),
        "BreastCancer": load_breast_cancer(),
        "Digits": load_digits(),
        "Diabetes": load_diabetes(),
    }
    k_values = [3, 5, 10]
    for name, ds in datasets.items():
        X = StandardScaler().fit_transform(ds.data)
        for k in k_values:
            _, c_lib, t_lib = library_heuristic(X, k)
            _, c_gr, t_gr = greedy_kcenter(X, k)
            rows.append([name, k,
                         round(c_lib, 3), round(t_lib, 1),
                         round(c_gr, 3), round(t_gr, 1), round(c_gr / c_lib, 2)])

    with open(os.path.join(script_dir, "p5_results.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["dataset", "k", "kmeans_cost", "kmeans_time_microseconds",
                    "greedy_cost", "greedy_time_microseconds", "greedy_over_kmeans_cost"])
        w.writerows(rows)

    sub = [r for r in rows if r[1] == 5 and r[0] not in ("Toy", "Random")]
    x = np.arange(len(sub))
    plt.figure(figsize=(8, 5))
    plt.bar(x - 0.2, [r[2] for r in sub], 0.4, label="KMeans (library)")
    plt.bar(x + 0.2, [r[4] for r in sub], 0.4, label="Greedy 2-approx")
    plt.xticks(x, [r[0] for r in sub])
    plt.ylabel("k-center cost")
    plt.title("Cost on real datasets, k=5")
    plt.legend()
    plt.savefig(os.path.join(script_dir, "kcenter_real_cost.png"), dpi=150)
    plt.close()


if __name__ == "__main__":
    toy_rows = run_toy()
    run_real(toy_rows)