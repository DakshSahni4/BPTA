import os
import networkx as nx
import matplotlib.pyplot as plt

script_dir = os.path.dirname(os.path.abspath(__file__))

for m in range(10, 46, 5):

    graph_file = os.path.join(
        script_dir,
        f"graph_m{m}.txt"
    )

    if not os.path.exists(graph_file):
        print(f"{graph_file} not found, skipping.")
        continue

    G = nx.Graph()

    with open(graph_file, "r") as f:
        for line in f:
            line = line.strip()

            if not line:
                continue

            u, v = map(int, line.split())
            G.add_edge(u, v)

    aprox_file = os.path.join(
        script_dir,
        f"aprox_m{m}.txt"
    )

    if not os.path.exists(aprox_file):
        print(f"{aprox_file} not found, skipping.")
        continue

    matching = []
    vertex_cover = set()

    with open(aprox_file, "r") as f:
        lines = f.readlines()

    reading_matching = False
    reading_cover = False

    for line in lines:

        line = line.strip()

        if line == "Matching:":
            reading_matching = True
            reading_cover = False
            continue

        if line == "VertexCover:":
            reading_matching = False
            reading_cover = True
            continue

        if not line:
            continue

        if reading_matching:
            u, v = map(int, line.split())
            matching.append((u, v))

        elif reading_cover:
            vertex_cover.update(
                map(int, line.split())
            )

    pos = nx.spring_layout(
        G,
        seed=42
    )

    normal = set(G.nodes()) - vertex_cover

    plt.figure(figsize=(8, 6))

    nx.draw_networkx_nodes(
        G,
        pos,
        nodelist=list(normal),
        node_color="lightblue",
        node_size=700
    )

    nx.draw_networkx_nodes(
        G,
        pos,
        nodelist=list(vertex_cover),
        node_color="red",
        node_size=700
    )

    nx.draw_networkx_edges(
        G,
        pos,
        width=1.5
    )

    nx.draw_networkx_edges(
        G,
        pos,
        edgelist=matching,
        width=4
    )

    nx.draw_networkx_labels(
        G,
        pos,
        font_size=12,
        font_weight="bold"
    )

    plt.title(
        f"Graph m={m} - "
        f"Approximate Vertex Cover "
        f"(size {len(vertex_cover)})"
    )

    plt.axis("off")
    plt.tight_layout()

    out_file = os.path.join(
        script_dir,
        f"aprox_graph_m{m}.png"
    )

    plt.savefig(
        out_file,
        dpi=300
    )

    plt.close()

    print(f"Saved {out_file}")

print("All approximation visualizations done.")