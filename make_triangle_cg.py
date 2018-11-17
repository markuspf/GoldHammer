#!/usr/bin/env python3


import os
from sympy.combinatorics.free_groups import *
from sympy.combinatorics.fp_groups import FpGroup
from sympy.combinatorics.rewritingsystem import *
from matplotlib import pyplot as plt
import networkx as nx


def format_ltx(s):
    r = s.replace('x**-1', 'x') \
         .replace("y**-1", "y^2") \
         .replace('<identity>', '1') \
         .replace('*', '')
    print(s, '->', r)
    return r


def make_cayley_graph(t, radius, lookahead=0):
    cg = nx.DiGraph(directed=True)
    p, q, r = t

    f, x, y = free_group(["x", "y"])
    fp = FpGroup(f, [x ** p, y ** q, (x * y) ** r])
    rws = RewritingSystem(fp)

    seen = [f.identity]
    queue = [[seen[0], 0]]
    depths = [0]

    finished = False
    while not finished and len(queue) > 0:
        print('seen', seen)
        print('queue', queue)
        print('depths', depths)

        g, g_ind = queue[0]
        queue = queue[1:]

        g = rws.reduce_using_automaton(g)

        print('source', format_ltx(str(g)))

        for e in [x, y, y ** 2]:
            h = rws.reduce_using_automaton(g * e)
            print('\t', 'consider', e, '->', format_ltx(str(h)))

            h_ind = 1
            while h_ind < len(seen):
                if h == seen[h_ind]:
                    break
                else:
                    h_ind += 1

            if h_ind == len(seen):
                print('\t\t', 'new element', format_ltx(str(h)))
                cg.add_node(h)
                cg.add_edge(g, h)
                seen += [h]
                queue += [[h, h_ind]]
                depths += [depths[g_ind] + 1]
                if depths[h_ind] > radius + lookahead:
                    finished = True
                    break
            elif h_ind < len(seen) and g_ind != h_ind:
                cg.add_edge(g, h)
                if depths[g_ind] + 1 < depths[h_ind]:
                    depths[h_ind] = depths[g_ind] + 1
        print()
    for i in [ind for ind in range(len(seen)) if depths[ind] > radius]:
        cg.remove_node(seen[i])
    return cg


def make_latex_element(g):
    s = ''
    p, last = 0, None
    for sym in g + '_':
        if sym != last:
            if last != None:
                s += last + '^{' + str(p) + '}'
                p = 0
            last = sym.lower()
        if sym.islower():
            p += 1
        else:
            p -= 1
    if s == '':
        s = '1'
    return '${' + s + '}$'


def plot_digraph(g, title):
    plt.figure(figsize=(24, 24))
    plt.suptitle('cayley graph',
                 size=35,
                 family='monospace',
                 weight='bold')

    ax = plt.subplot(111)

    nx.draw_networkx(g,
                     arrows=True,
                     pos=nx.kamada_kawai_layout(g, dim=2),
                     # labels={nd : make_latex_element(nd) for nd in g.nodes()},
                     labels={nd : '$' + format_ltx(str(nd)) + '$' for nd in g.nodes()},
                     font_size=6,
                     font_family='arial',
                     font_weight='bold',
                     font_color='r',
                     alpha=1.,
                     node_color='#AAFFAA',
                     node_size=300,
                     width=[0.3 for (u, v, d) in g.edges(data=True)],
                     arrowstyle='-|>',
                     arrowsize=12,
                     ax=ax)
    ax.set_title(title, fontsize=40)
    ax.set_axis_off()

    filename = 'cayley_graph.png'
    if os.path.exists(filename):
        os.remove(filename)
    plt.savefig(filename)
    plt.clf()
    plt.cla()
    plt.close()


if __name__ == "__main__":
    t = (2, 3, 7)
    g = make_cayley_graph(t, 12, 3)
    plot_digraph(g, str(t))
