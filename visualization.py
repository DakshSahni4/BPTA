import os
import networkx as nx
import matplotlib.pyplot as plt

script_dir = os.path.dirname(os.path.abspath(__file__))


for m in range(10, 46, 5):
    graph_file = os.path.join(script_dir, f"graph_m{m}.txt")
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
    cover_file = os.path.join(
    script_dir,
    f"cover_m{m}.txt"
    )

    with open(cover_file, "r") as f:
        cover_str = f.read().strip()

        if cover_str:
            vertex_cover = set(map(int, cover_str.split()))
        else:
            vertex_cover = set()

    pos = nx.spring_layout(G, seed=42)
    normal = set(G.nodes()) - vertex_cover

    plt.figure(figsize=(8, 6))
    nx.draw_networkx_nodes(G, pos, nodelist=list(normal), node_color="lightblue", node_size=700)
    nx.draw_networkx_nodes(G, pos, nodelist=list(vertex_cover), node_color="red", node_size=700)
    nx.draw_networkx_edges(G, pos, width=2)
    nx.draw_networkx_labels(G, pos, font_size=12, font_weight="bold")

    plt.title(f"Graph m={m} - Vertex Cover (size {len(vertex_cover)})")
    plt.axis("off")
    plt.tight_layout()

    out_file = os.path.join(script_dir, f"graph_m{m}.png")
    plt.savefig(out_file, dpi=300)
    plt.close()

    print(f"Saved {out_file}")

print("All visualizations done.")