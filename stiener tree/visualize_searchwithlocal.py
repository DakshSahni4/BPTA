import csv
import math
import os
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

LOG_FILE = "search_log.csv"
OUT_DIR = "figures"
SUMMARY_FILE = "results_summary.csv"

TERMINAL_COLOR = "#d62728"
STEINER_COLOR = "#ff9f1c"
OPT_COLOR = "#2ca02c"
APPROX_COLOR = "#1f77b4"
HEUR_COLOR = "#9467bd"
BG_EDGE_COLOR = "#cccccc"


def read_edges(filename):
    if not os.path.exists(filename):
        return None
    edges = []
    with open(filename) as f:
        for line in f:
            parts = line.split()
            if len(parts) == 3:
                edges.append(tuple(int(x) for x in parts))
    return edges


def read_terminals(filename):
    with open(filename) as f:
        return [int(x) for x in f.read().split()]


def to_int(s):
    return int(float(s))


def ratio_or_none(value):
    v = float(value)
    return None if v < 0 else v


def circular_positions(n):
    return {
        i: (math.cos(2 * math.pi * i / n), math.sin(2 * math.pi * i / n))
        for i in range(n)
    }


def draw_panel(ax, n, pos, all_edges, terminals, tree_edges, tree_color, title):
    term_set = set(terminals)

    for u, v, _ in all_edges:
        ax.plot(
            [pos[u][0], pos[v][0]], [pos[u][1], pos[v][1]],
            color=BG_EDGE_COLOR, lw=0.8, zorder=1,
        )

    tree_vertices = set()
    if tree_edges:
        for u, v, w in tree_edges:
            tree_vertices.update((u, v))
            ax.plot(
                [pos[u][0], pos[v][0]], [pos[u][1], pos[v][1]],
                color=tree_color, lw=3.2, zorder=2, solid_capstyle="round",
            )
        for u, v, w in tree_edges:
            mx = (pos[u][0] + pos[v][0]) / 2
            my = (pos[u][1] + pos[v][1]) / 2
            ax.text(
                mx, my, str(w), fontsize=8, ha="center", va="center", zorder=4,
                bbox=dict(boxstyle="round,pad=0.12", fc="white", ec="none"),
            )

    for v in range(n):
        if v in term_set:
            face = TERMINAL_COLOR
        elif v in tree_vertices:
            face = STEINER_COLOR
        else:
            face = "white"
        ax.scatter(
            [pos[v][0]], [pos[v][1]], s=330, c=face,
            edgecolors="black", linewidths=1.0, zorder=5,
        )
        ax.text(pos[v][0], pos[v][1], str(v), fontsize=8, ha="center",
                va="center", zorder=6)

    ax.set_title(title, fontsize=12)
    ax.set_aspect("equal")
    ax.set_xlim(-1.3, 1.3)
    ax.set_ylim(-1.3, 1.3)
    ax.axis("off")


def draw_placeholder(ax, title, text):
    ax.set_title(title, fontsize=12)
    ax.text(0.5, 0.5, text, ha="center", va="center", fontsize=12,
            transform=ax.transAxes, color="gray")
    ax.axis("off")


def make_figure(row, edges, terminals, opt_tree, approx_tree, heur_tree):
    probe, n, m, k = row["probe"], row["N"], row["m"], row["K"]
    finished = row["status"] == "ok"
    pos = circular_positions(n)

    fig, axes = plt.subplots(1, 4, figsize=(28, 7.5))

    draw_panel(axes[0], n, pos, edges, terminals, None, None, "Generated graph")

    if finished and opt_tree is not None:
        draw_panel(axes[1], n, pos, edges, terminals, opt_tree, OPT_COLOR,
                   f"Optimal tree (brute force), cost {row['opt_cost']}")
    else:
        draw_placeholder(axes[1], "Optimal tree (brute force)",
                         "not available\n(brute force timed out)")

    t = f"MST approximation, cost {row['approx_cost']}"
    if row["approx_ratio"] is not None:
        t += f"  (ratio {row['approx_ratio']:.3f})"
    draw_panel(axes[2], n, pos, edges, terminals, approx_tree, APPROX_COLOR, t)

    t = f"Local search, cost {row['heur_cost']}"
    if row["heur_ratio"] is not None:
        t += f"  (ratio {row['heur_ratio']:.3f})"
    draw_panel(axes[3], n, pos, edges, terminals, heur_tree, HEUR_COLOR, t)

    legend_items = [
        Line2D([0], [0], marker="o", color="w", markerfacecolor=TERMINAL_COLOR,
               markeredgecolor="black", markersize=11, label="terminal"),
        Line2D([0], [0], marker="o", color="w", markerfacecolor=STEINER_COLOR,
               markeredgecolor="black", markersize=11, label="Steiner vertex"),
    ]
    axes[0].legend(handles=legend_items, loc="lower left", frameon=False, fontsize=10)

    if finished:
        status = "brute force finished"
    else:
        status = (f"brute force timed out, {row['subsets_checked']} of "
                  f"{row['subsets_total']} subsets checked")
    fig.suptitle(f"Probe {probe}: N={n}, m={m}, K={k}   ({status})", fontsize=15)
    fig.tight_layout(rect=(0, 0, 1, 0.95))

    os.makedirs(OUT_DIR, exist_ok=True)
    path = os.path.join(OUT_DIR, f"probe_{probe:02d}_N{n}.png")
    fig.savefig(path, dpi=100)
    plt.close(fig)
    return path


def load_log():
    rows = []
    with open(LOG_FILE, newline="") as f:
        for r in csv.DictReader(f):
            rows.append({
                "probe": to_int(r["probe"]),
                "N": to_int(r["N"]),
                "m": to_int(r["m"]),
                "K": to_int(r["K"]),
                "status": r["status"],
                "opt_cost": to_int(r["opt_cost"]),
                "approx_cost": to_int(r["approx_cost"]),
                "heur_cost": to_int(r["heur_cost"]),
                "approx_ratio": ratio_or_none(r["approx_ratio"]),
                "heur_ratio": ratio_or_none(r["heur_ratio"]),
                "exact_time_s": to_int(r["exact_time_us"]) / 1e6,
                "approx_time_s": to_int(r["approx_time_us"]) / 1e6,
                "heur_time_s": to_int(r["heur_time_us"]) / 1e6,
                "subsets_checked": to_int(r["subsets_checked"]),
                "subsets_total": to_int(r["subsets_total"]),
            })
    rows.sort(key=lambda r: r["probe"])
    return rows


def fmt(x, digits=4):
    return "" if x is None else f"{x:.{digits}f}"


def main():
    if not os.path.exists(LOG_FILE):
        print(f"{LOG_FILE} not found - run the C++ program first.")
        sys.exit(1)

    rows = load_log()
    summary = []

    for row in rows:
        pid = row["probe"]
        edges = read_edges(f"graph_p{pid}.txt")
        if edges is None:
            print(f"probe {pid}: graph file missing, skipped")
            continue
        terminals = read_terminals(f"terminals_p{pid}.txt")
        opt_tree = read_edges(f"tree_opt_p{pid}.txt")
        approx_tree = read_edges(f"tree_approx_p{pid}.txt")
        heur_tree = read_edges(f"tree_heur_p{pid}.txt")

        path = make_figure(row, edges, terminals, opt_tree, approx_tree, heur_tree)
        print(f"probe {pid}: saved {path}")

        finished = row["status"] == "ok"
        summary.append({
            "probe": pid,
            "N": row["N"],
            "m": row["m"],
            "K": row["K"],
            "status": row["status"],
            "exact": row["opt_cost"] if finished else "",
            "approx": row["approx_cost"],
            "a.ratio": fmt(row["approx_ratio"]),
            "heur": row["heur_cost"],
            "h.ratio": fmt(row["heur_ratio"]),
            "t_exact(s)": fmt(row["exact_time_s"], 6),
            "t_approx(s)": fmt(row["approx_time_s"], 6),
            "t_heur(s)": fmt(row["heur_time_s"], 6),
            "heur_over_approx": fmt(
                row["heur_cost"] / row["approx_cost"] if row["approx_cost"] else None),
        })

    if not summary:
        print("nothing to summarise.")
        return

    with open(SUMMARY_FILE, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(summary[0].keys()))
        writer.writeheader()
        writer.writerows(summary)
    print(f"saved {SUMMARY_FILE}")

    print()
    print(f"{'probe':>5} {'N':>3} {'m':>4} {'K':>3} {'status':>8} {'exact':>6} "
          f"{'approx':>6} {'a.ratio':>8} {'heur':>5} {'h.ratio':>8} "
          f"{'t_exact(s)':>11} {'t_approx(s)':>12} {'t_heur(s)':>10}")
    for s in summary:
        print(f"{s['probe']:>5} {s['N']:>3} {s['m']:>4} {s['K']:>3} {s['status']:>8} "
              f"{str(s['exact']):>6} {s['approx']:>6} {s['a.ratio']:>8} "
              f"{s['heur']:>5} {s['h.ratio']:>8} "
              f"{s['t_exact(s)']:>11} {s['t_approx(s)']:>12} {s['t_heur(s)']:>10}")


if __name__ == "__main__":
    main()