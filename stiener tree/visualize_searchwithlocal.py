import csv
import matplotlib.pyplot as plt

rows = []
with open("search_log.csv") as f:
    for r in csv.DictReader(f):
        rows.append({
            "probe": int(r["probe"]),
            "N": int(r["N"]),
            "m": int(r["m"]),
            "K": int(r["K"]),
            "ok": r["status"] == "ok",
            "opt": int(r["opt_cost"]),
            "approx": int(r["approx_cost"]),
            "heur": int(r["heur_cost"]),
            "approx_ratio": float(r["approx_ratio"]),
            "heur_ratio": float(r["heur_ratio"]),
            "t_exact": int(r["exact_time_us"]),
            "t_approx": int(r["approx_time_us"]),
            "t_heur": int(r["heur_time_us"]),
        })

if not rows:
    raise SystemExit("search_log.csv is empty")

rows.sort(key=lambda r: r["N"])          # probes are logged in binary-search order
done = [r for r in rows if r["ok"]]      # brute force finished
failed = [r for r in rows if not r["ok"]]

N_all = [r["N"] for r in rows]
N_ok = [r["N"] for r in done]

fig, axes = plt.subplots(2, 2, figsize=(13, 9))

# 1. Solution cost (optimal only exists where brute force finished)
ax = axes[0][0]
ax.plot(N_ok, [r["opt"] for r in done], "o-", label="Optimal (brute force)")
ax.plot(N_all, [r["approx"] for r in rows], "s--", label="MST approximation")
ax.plot(N_all, [r["heur"] for r in rows], "^-.", label="Local search heuristic")
ax.set_xlabel("Number of vertices (N)")
ax.set_ylabel("Steiner tree cost")
ax.set_title("Solution cost")
ax.legend()
ax.grid(True, alpha=0.3)

# 2. Ratio to optimal
ax = axes[0][1]
ax.plot(N_ok, [r["approx_ratio"] for r in done], "s--", label="approx / optimal")
ax.plot(N_ok, [r["heur_ratio"] for r in done], "^-.", label="heuristic / optimal")
ax.axhline(1.0, color="gray", linestyle=":")
if done:
    Ns = sorted(set(N_ok))
    bound = {r["N"]: 2 * (1 - 1 / r["K"]) for r in done}
    ax.plot(Ns, [bound[n] for n in Ns], color="red", linestyle="--", label="2(1 - 1/K) bound")
ax.set_xlabel("Number of vertices (N)")
ax.set_ylabel("Ratio")
ax.set_title("Quality relative to optimal")
ax.legend()
ax.grid(True, alpha=0.3)

# 3. Running time (log scale); timed-out brute force marked with a red x
ax = axes[1][0]
ax.plot(N_ok, [r["t_exact"] for r in done], "o-", label="Brute force (finished)")
if failed:
    ax.plot([r["N"] for r in failed], [r["t_exact"] for r in failed], "rx",
            markersize=10, label="Brute force (timed out)")
ax.plot(N_all, [max(r["t_approx"], 1) for r in rows], "s--", label="MST approximation")
ax.plot(N_all, [max(r["t_heur"], 1) for r in rows], "^-.", label="Local search heuristic")
ax.set_yscale("log")
ax.set_xlabel("Number of vertices (N)")
ax.set_ylabel("Time (microseconds, log scale)")
ax.set_title("Running time")
ax.legend()
ax.grid(True, alpha=0.3)

# 4. Binary search path: which N was probed, in order
ax = axes[1][1]
by_probe = sorted(rows, key=lambda r: r["probe"])
steps = [r["probe"] for r in by_probe]
ax.plot(steps, [r["N"] for r in by_probe], "-", color="gray", alpha=0.5)
ax.scatter([r["probe"] for r in by_probe if r["ok"]],
           [r["N"] for r in by_probe if r["ok"]], c="green", s=70, label="finished", zorder=3)
ax.scatter([r["probe"] for r in by_probe if not r["ok"]],
           [r["N"] for r in by_probe if not r["ok"]], c="red", marker="x", s=80,
           label="timed out", zorder=3)
for r in by_probe:
    ax.annotate(str(r["N"]), (r["probe"], r["N"]), textcoords="offset points",
                xytext=(6, 6), fontsize=9)
ax.set_xlabel("Binary search step")
ax.set_ylabel("N probed")
ax.set_title("Binary search path")
ax.set_xticks(steps)
ax.legend()
ax.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig("search_results.png", dpi=150)
plt.show()