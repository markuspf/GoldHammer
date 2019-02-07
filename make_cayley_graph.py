#!/usr/bin/env python3


import os
import sys
import json
from matplotlib import pyplot as plt
import networkx as nx
import cairo
from subprocess import call


from cayley_graph_utils import *


def format_ltx(s):
    r = s.replace('x**-1', 'x') \
         .replace("y**-1", "y^2") \
         .replace('<identity>', '1') \
         .replace('*', '') \
         .replace('Y', 'y^2')
    # print(s, '->', r)
    return r


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


def parse_element_rec(s):
    # s -> xp | yp | | s*s | (s)p
    # p -> ^n | _
    t = s.replace('*', '')
    parsed = ''

    def parse_digit(start_ind):
        digits = ''
        for d in range(start_ind, len(t)):
            d = t[d]
            if d in '-0123456789':
                # print('consider', d)
                digits += d
            else:
                break
        # print("digits", digits)
        return digits

    i = 0
    while i < len(t):
        if t[i] in ['x', 'y']:
            if i == len(t) - 1:
                parsed += t[i]
                break
            elif t[i + 1] == '^':
                digits = parse_digit(i + 2)
                n = int(digits)
                if n < 0:
                    parsed += t[i].upper()
                else:
                    parsed += t[i] * n
                i = i + 2 + len(digits)
            else:
                parsed += t[i]
                i += 1
        elif t[i] == '(':
            k = 1
            for j in range(i + 1, len(t)):
                if t[j] == '(':
                    k += 1
                elif t[j] == ')':
                    k -= 1
                if k == 0:
                    k = j
                    break
            p = parse_element_rec(t[i+1:k])
            j, i = k, k + 1
            if i == len(t):
                parsed += p
                break
            elif t[i] == '^':
                # print('next', t[i:])
                digits = parse_digit(i + 1)
                n = int(digits)
                if n < 0:
                    p_rev = p[::-1]
                    for b in range(len(p_rev)):
                        if p_rev[b].is_lower():
                            p_rev[b] = p_rev[b].upper()
                        else:
                            p_rev[b] = p_rev[b].lower()
                    parsed += p_rev
                    parsed += t[j:k+1] + '^' + str(n)
                else:
                    parsed += p * n
                i += 1 + len(digits)
        else:
            i += 1
    # print(s.replace('*', ''), '->', parsed)
    return parsed


def reduce_element(t, s):
    ss = ''
    p, q, r = t
    while ss != s:
        ss = s
        s = s.replace('X', 'x' * (p - 1))
        s = s.replace('Y', 'y' * (q - 1))
        s = s.replace('x' * p, '')
        s = s.replace('y' * q, '')
        s = s.replace('xy' * r, '')
        s = s.replace('yyx' * r, 'xy')
        for i in range(1, (r // 2) + (r & 1) + 1):
            s = s.replace('yyx' * (r - i), 'xy' * i)
#        s = s.replace('yyx' * 6, 'xy' * 1)
#        s = s.replace('yyx' * 5, 'xy' * 2)
#        s = s.replace('yyx' * 4, 'xy' * 3)
#        s = s.replace('yyx' * 3, 'xy' * 4)
        # s = s.replace('yyx'*2, 'xy'*5)
        # s = s.replace('yyx'*1, 'xy'*7)
    return s


def parse_element(t, s):
    if s == '<identity ...>':
        return ''
    return reduce_element(t, parse_element_rec(s))


def element_ltx(s):
    s = s.replace('^-1', '^{-1}')
    s = s.replace('*', '')
    return s


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
    if len(g.nodes()) < 1000:
        plot_digraph((g, data), 'cg_graph', fedges.replace('cg_edges', 'cg_graph').replace('.txt', '.png'))
    plot_adjacency_matrix((g, data), fedges.replace('cg_edges', 'cg_adjacency').replace('.txt', '.png'))
