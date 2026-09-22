import os
import networkx as nx
import matplotlib.pyplot as plt

script_dir = os.path.dirname(os.path.abspath(__file__))

n = 20
m_values = list(range(20, 181, 20)) + [190]


def parse_cover_file(filepath):
    """Parses a vertex_cover file with both BFA and Approximation sections."""
    bfa_cover = set()
    aprox_cover = set()

    with open(filepath, "r") as f:
        lines = [line.strip() for line in f if line.strip()]

    mode = None
    for line in lines:
        if line.startswith("BFA Vertex Cover"):
            mode = "bfa"
            continue
        elif line.startswith("Aproximation Vertex Cover"):
            mode = "aprox"
            continue

        if mode == "bfa":
            bfa_cover.update(map(int, line.split()))
        elif mode == "aprox":
            aprox_cover.update(map(int, line.split()))

    return bfa_cover, aprox_cover


for m in m_values:
    graph_file = os.path.join(script_dir, f"graph_n{n}_m{m}.txt")
    if not os.path.exists(graph_file):
        print(f"Graph file {graph_file} not found, skipping.")
        continue

    G = nx.Graph()
    with open(graph_file, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            u, v = map(int, line.split())
            G.add_edge(u, v)

    cover_file = os.path.join(script_dir, f"vertex_cover_n{n}_m{m}.txt")
    if not os.path.exists(cover_file):
        print(f"Cover file {cover_file} not found, skipping.")
        continue

    bfa_cover, aprox_cover = parse_cover_file(cover_file)

    pos = nx.spring_layout(G, seed=42)

    fig, axes = plt.subplots(1, 2, figsize=(16, 7))

    # --- BFA subplot ---
    bfa_normal = set(G.nodes()) - bfa_cover
    ax = axes[0]
    nx.draw_networkx_nodes(G, pos, nodelist=list(bfa_normal), node_color="lightblue", node_size=500, ax=ax)
    nx.draw_networkx_nodes(G, pos, nodelist=list(bfa_cover), node_color="red", node_size=500, ax=ax)
    nx.draw_networkx_edges(G, pos, width=1.5, ax=ax)
    nx.draw_networkx_labels(G, pos, font_size=9, font_weight="bold", ax=ax)
    ax.set_title(f"BFA Vertex Cover (size {len(bfa_cover)})")
    ax.axis("off")

    # --- Approximation subplot ---
    aprox_normal = set(G.nodes()) - aprox_cover
    ax = axes[1]
    nx.draw_networkx_nodes(G, pos, nodelist=list(aprox_normal), node_color="lightblue", node_size=500, ax=ax)
    nx.draw_networkx_nodes(G, pos, nodelist=list(aprox_cover), node_color="orange", node_size=500, ax=ax)
    nx.draw_networkx_edges(G, pos, width=1.5, ax=ax)
    nx.draw_networkx_labels(G, pos, font_size=9, font_weight="bold", ax=ax)
    ax.set_title(f"Approximation Vertex Cover (size {len(aprox_cover)})")
    ax.axis("off")

    fig.suptitle(f"n={n}, m={m}", fontsize=14, fontweight="bold")
    plt.tight_layout()

    out_file = os.path.join(script_dir, f"graph_n{n}_m{m}.png")
    plt.savefig(out_file, dpi=300)
    plt.close()

    print(f"Saved {out_file}")

print("All visualizations done.")