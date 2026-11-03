import csv
import matplotlib.pyplot as plt

m, K = [], []
opt, approx, ratio = [], [], []
t_exact, t_approx = [], []

with open("results.csv") as f:
    for row in csv.DictReader(f):
        m.append(int(row["m"]))
        K.append(int(row["K"]))
        opt.append(int(row["opt_cost"]))
        approx.append(int(row["approx_cost"]))
        ratio.append(float(row["ratio"]))
        t_exact.append(int(row["exact_time_us"]))
        t_approx.append(int(row["approx_time_us"]))

bound = 2 * (1 - 1 / K[0]) if K else 2

fig, axes = plt.subplots(1, 3, figsize=(15, 4))

# 1. Tree cost
axes[0].plot(m, opt, "o-", label="Optimal (brute force)")
axes[0].plot(m, approx, "s--", label="MST approximation")
axes[0].set_xlabel("Number of edges (m)")
axes[0].set_ylabel("Steiner tree cost")
axes[0].set_title("Solution cost")
axes[0].legend()
axes[0].grid(True, alpha=0.3)

# 2. Approximation ratio
axes[1].plot(m, ratio, "o-", label="approx / optimal")
axes[1].axhline(1.0, color="gray", linestyle=":")
axes[1].axhline(bound, color="red", linestyle="--", label=f"2(1-1/K) = {bound:.2f}")
axes[1].set_xlabel("Number of edges (m)")
axes[1].set_ylabel("Ratio")
axes[1].set_title("Approximation ratio")
axes[1].legend()
axes[1].grid(True, alpha=0.3)

# 3. Running time
axes[2].plot(m, t_exact, "o-", label="Brute force")
axes[2].plot(m, t_approx, "s--", label="MST approximation")
axes[2].set_yscale("log")
axes[2].set_xlabel("Number of edges (m)")
axes[2].set_ylabel("Time (microseconds, log scale)")
axes[2].set_title("Running time")
axes[2].legend()
axes[2].grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("results.png", dpi=150)
plt.show()
