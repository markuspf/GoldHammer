#!/usr/bin/env python3


import os
import sys
import json
from matplotlib import pyplot as plt
import networkx as nx
import cairo
from subprocess import call


from cayley_graph_utils import *


# def make_latex_element(g):
#     s = ''
#     p, last = 0, None
#     for sym in g + '_':
#         if sym != last:
#             if last != None:
#                 s += last + '^{' + str(p) + '}'
#                 p = 0
#             last = sym.lower()
#         if sym.islower():
#             p += 1
#         else:
#             p -= 1
#     if s == '':
#         s = '1'
#     return '${' + s + '}$'


def plot_digraph(gdata, title, fname, format_f=lambda s: str(s)):
    g, data = gdata
    colors = ['r', 'g', 'b', 'k', 'm', 'c']
    get_color = lambda n: colors[n % len(colors)]
    plt.figure(figsize=(36, 36))
    plt.suptitle('cayley graph',
                 size=35,
                 family='monospace',
                 weight='bold')

    ax = plt.subplot(111)

    nx.draw_networkx(g,
                     arrows=True,
                     # pos=nx.spring_layout(g, dim=2),
                     pos=nx.kamada_kawai_layout(g, dim=2),
                     # labels={nd : make_latex_element(nd) for nd in g.nodes()},
                     labels={nd : format_f(nd) for nd in g.nodes()},
                     font_size=6,
                     # font_family='arial',
                     font_weight='bold',
                     font_color='r',
                     edge_color='k',
                     alpha=1.,
                     node_color='#AAFFAA',
                     node_size=300,
                     width=[0.3 for (u, v, d) in g.edges(data=True)],
                     arrowstyle='-|>',
                     arrowsize=12,
                     ax=ax)
    ax.set_title(title, fontsize=40)
    ax.set_axis_off()

    filename = fname
    if os.path.exists(filename):
        os.remove(filename)
    plt.savefig(filename)
    plt.clf()
    plt.cla()
    plt.close()


def plot_adjacency_matrix(gdata, fname='adjacency_mat.png'):
    g, data = gdata
    colors = [
        (1, 0, 0),
        (0, 1, 0),
        (0, 0, 1),
        (0, 0, 0),
        (1, 0, 1),
        (0, 1, 1)
    ]
    get_color = lambda n: colors[n % len(colors)]
    size = len(g.nodes())
    svg_fname = fname.replace('.png', '.svg')
    with cairo.SVGSurface(svg_fname, size, size) as surface:
        ctx = cairo.Context(surface)
        n = len(g)
        rectsize = float(size) / n
        g_nodes = list(g.nodes())
        for i in range(n):
            for j in range(n):
                edge = (g_nodes[i], g_nodes[j])
                if i == j or edge in g.edges():
                    ctx.rectangle(i * rectsize, j * rectsize, rectsize, rectsize)
                    # color = get_color(g.get_edge_data(*edge)['generator'])
                    color = get_color(data[i][j] if i != j else 3)
                    r, green, b = color
                    ctx.set_source_rgba(r, green, b, 1.)
                    ctx.fill()
            ctx.stroke()

    call(['convert', svg_fname, fname])
    os.remove(svg_fname)


if __name__ == "__main__":
    plt.switch_backend('agg')
    felems, fedges = 'cg_elements.txt', 'cg_edges.txt'
    if len(sys.argv) >= 3:
        felems, fedges = sys.argv[1], sys.argv[2]
    elems = load_group_elements(felems)
    edges = load_group_edges(fedges)
    g, data = make_graph_from_elements(elems, edges)
    for e in edges:
        print(elems[e[0]], '\t->\t', elems[e[1]])
    if len(g.nodes()) < 1500:
        plot_digraph((g, data), 'cg_graph', fedges.replace('cg_edges', 'cg_graph').replace('.txt', '.png'))
    plot_adjacency_matrix((g, data), fedges.replace('cg_edges', 'cg_adjacency').replace('.txt', '.png'))
