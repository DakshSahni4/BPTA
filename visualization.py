import os
import networkx as nx
import matplotlib.pyplot as plt

# Get graph_m10.txt from the same folder as this Python file
filename = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "graph_m35.txt"
)

G = nx.Graph()
vertex_cover = set()

# Read the file
with open(filename, "r") as file:
    reading_cover = False

    for line in file:
        line = line.strip()

        if not line:
            continue

        if line.lower() == "vertex cover":
            reading_cover = True
            continue

        if reading_cover:
            vertex_cover.update(map(int, line.split()))
        else:
            u, v = map(int, line.split())
            G.add_edge(u, v)

# Generate layout
pos = nx.spring_layout(G, seed=42)

# Vertices not in vertex cover
normal_vertices = set(G.nodes()) - vertex_cover

# Draw normal vertices
nx.draw_networkx_nodes(
    G,
    pos,
    nodelist=list(normal_vertices),
    node_color="lightblue",
    node_size=700
)

# Draw vertex cover
nx.draw_networkx_nodes(
    G,
    pos,
    nodelist=list(vertex_cover),
    node_color="red",
    node_size=700
)

# Draw edges
nx.draw_networkx_edges(G, pos, width=2)

# Draw labels
nx.draw_networkx_labels(
    G,
    pos,
    font_size=12,
    font_weight="bold"
)

plt.title("Graph m10 - Vertex Cover Highlighted")
plt.axis("off")
plt.tight_layout()

# Save image in the same folder as the Python file
output_file = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "graph_m10.png"
)

plt.savefig(output_file, dpi=300)
plt.show()

print("Vertex Cover:", sorted(vertex_cover))
print("Visualization saved to:", output_file)