import json
import networkx as nx


def load_group_elements(filename):
    with open(filename, 'r') as f:
        j = json.load(f)
    for i in range(len(j)):
        j[i] = j[i].strip().replace(' ', '*')
    print('len(j)', len(j))
    return j


def load_group_edges(filename):
    with open(filename, 'r') as f:
        edges = json.load(f)
    for i in range(len(edges)):
        u, v, g = edges[i]
        edges[i] = [u - 1, v - 1, g]
        print(u - 1, v - 1)
    return edges


def make_graph_from_elements(elems, edges):
    g = nx.MultiDiGraph()
    data = {}
    for e in edges:
        # print(e, '\t', elems[e[0]], '-->', elems[e[1]])
        u, v, gen = e
        g.add_edge(elems[u], elems[v], generator=int(gen))
        if u not in data:
            data[u] = {}
        data[u][v] = gen
    return (g, data)
