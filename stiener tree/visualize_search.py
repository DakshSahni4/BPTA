import csv
import math

import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

TERMINAL_COLOR = "#d62728"   # red
STEINER_COLOR = "#ff9f1c"    # orange: non-terminal used by the tree
OTHER_COLOR = "#ffffff"


def read_edges(path):
    edges = []
    try:
        with open(path) as f:
            for line in f:
                p = line.split()
                if len(p) == 3:
                    edges.append(tuple(int(x) for x in p))
    except FileNotFoundError:
        return None
    return edges


def read_terminals(path):
    with open(path) as f:
        return [int(x) for x in f.read().split()]


def circle_layout(n):
    return {i: (math.cos(2 * math.pi * i / n), math.sin(2 * math.pi * i / n)) for i in range(n)}


def draw(ax, n, pos, all_edges, tree_edges, terminals, title, tree_color, label_all=False):
    # every edge of the graph, light gray
    for u, v, w in all_edges:
        (x1, y1), (x2, y2) = pos[u], pos[v]
        ax.plot([x1, x2], [y1, y2], color="#cccccc", lw=0.6, zorder=1)
        if label_all:
            ax.text((x1 + x2) / 2, (y1 + y2) / 2, str(w), fontsize=6, color="#777777", zorder=2)

    # tree edges, bold, with weights
    tree_nodes = set()
    if tree_edges:
        for u, v, w in tree_edges:
            (x1, y1), (x2, y2) = pos[u], pos[v]
            ax.plot([x1, x2], [y1, y2], color=tree_color, lw=2.6, zorder=3)
            ax.text((x1 + x2) / 2, (y1 + y2) / 2, str(w), fontsize=7, color="black",
                    ha="center", va="center", zorder=5,
                    bbox=dict(boxstyle="round,pad=0.1", fc="white", ec="none", alpha=0.85))
            tree_nodes.update((u, v))

    # vertices
    for v in range(n):
        if v in terminals:
            color = TERMINAL_COLOR
        elif v in tree_nodes:
            color = STEINER_COLOR
        else:
            color = OTHER_COLOR
        x, y = pos[v]
        ax.scatter([x], [y], s=230, c=color, edgecolors="black", linewidths=0.8, zorder=4)
        ax.text(x, y, str(v), fontsize=7, ha="center", va="center", zorder=6)

    ax.set_title(title, fontsize=10)
    ax.set_aspect("equal")
    ax.axis("off")


def main():
    rows = []
    with open("search_log.csv") as f:
        for r in csv.DictReader(f):
            rows.append(r)

    legend_items = [
        Line2D([0], [0], marker="o", color="w", markerfacecolor=TERMINAL_COLOR,
               markeredgecolor="black", markersize=9, label="terminal"),
        Line2D([0], [0], marker="o", color="w", markerfacecolor=STEINER_COLOR,
               markeredgecolor="black", markersize=9, label="Steiner vertex"),
    ]

    for r in rows:
        pid = int(r["probe"])
        n = int(r["N"])
        m = int(r["m"])
        k = int(r["K"])
        status = r["status"]
        ok = status == "ok"

        edges = read_edges(f"graph_p{pid}.txt")
        terminals = set(read_terminals(f"terminals_p{pid}.txt"))
        approx_tree = read_edges(f"tree_approx_p{pid}.txt")
        opt_tree = read_edges(f"tree_opt_p{pid}.txt") if ok else None

        pos = circle_layout(n)

        fig, axes = plt.subplots(1, 3, figsize=(16, 5.6))
        fig.suptitle(
            f"Probe {pid}: N={n}, m={m}, K={k}   "
            f"({ {'ok': 'brute force finished', 'timeout': 'brute force TIMED OUT', 'memory': 'brute force SKIPPED, too many subsets for memory'}[status] })",
            fontsize=12,
        )

        draw(axes[0], n, pos, edges, None, terminals,
             "Generated graph", "black", label_all=(m <= 30))
        axes[0].legend(handles=legend_items, loc="lower left", fontsize=8, frameon=False)

        if ok:
            draw(axes[1], n, pos, edges, opt_tree, terminals,
                 f"Optimal tree (brute force), cost {r['opt_cost']}", "#2ca02c")
        else:
            if status == "timeout":
                msg = f"Brute force timed out\n({r['subsets_checked']} of {r['subsets_total']} subsets checked)"
            else:
                msg = f"Brute force skipped\n({r['subsets_total']} subsets would not fit in memory)"
            axes[1].text(0.5, 0.5, msg, ha="center", va="center", fontsize=11)
            axes[1].set_title("Optimal tree (brute force)", fontsize=10)
            axes[1].axis("off")

        approx_title = f"MST approximation, cost {r['approx_cost']}"
        if ok:
            approx_title += f"  (ratio {float(r['ratio']):.3f})"
        draw(axes[2], n, pos, edges, approx_tree, terminals, approx_title, "#1f77b4")

        plt.tight_layout(rect=[0, 0, 1, 0.94])
        plt.savefig(f"probe_{pid:02d}_N{n}.png", dpi=130)

    # ---- summary of the binary search ----
    fig, ax = plt.subplots(figsize=(8, 4.5))
    solved = [int(r["N"]) for r in rows if r["status"] == "ok"]
    best = max(solved) if solved else None

    ymax = max([int(r["exact_time_us"]) / 1e6 for r in rows] + [1e-3])

    for r in rows:
        t = int(r["exact_time_us"]) / 1e6
        status = r["status"]
        if status == "memory":
            t = ymax   # nothing ran, so park the marker at the top
        marker = "x" if status == "memory" else "o"
        color = "#2ca02c" if status == "ok" else "#d62728"
        ax.scatter(int(r["N"]), max(t, 1e-6), s=70, c=color, marker=marker, zorder=3)
        label = f"#{r['probe']}" if status == "memory" else f"#{r['probe']}  K={r['K']}, m={r['m']}"
        ax.annotate(label, (int(r["N"]), max(t, 1e-6)), textcoords="offset points",
                    xytext=(6, 6), fontsize=8)

    ax.set_yscale("log")
    ax.set_xlabel("Number of vertices N")
    ax.set_ylabel("Brute force time (s, log scale)")
    title = "Binary search on N (green = finished, red = timed out, x = skipped)"
    if best is not None:
        title += f"\nHighest N solved: {best}"
    ax.set_title(title)
    ax.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig("search_summary.png", dpi=130)

    plt.show()


if __name__ == "__main__":
    main()
