#!/usr/bin/env python3


import os
from sympy.combinatorics.free_groups import *
from sympy.combinatorics.fp_groups import FpGroup
from sympy.combinatorics.rewritingsystem import *
from matplotlib import pyplot as plt
import networkx as nx
import cairo
from subprocess import call


def format_ltx(s):
    r = s.replace('x**-1', 'x') \
         .replace("y**-1", "y^2") \
         .replace('<identity>', '1') \
         .replace('*', '')
    print(s, '->', r)
    return r


def make_cayley_graph(radius, lookahead=0):
    t = (2, 3, 7)
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


def plot_digraph(g, title, fname='cayley_graph.png'):
    plt.figure(figsize=(24, 24))
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
                     labels={nd : '$' + format_ltx(str(nd)) + '$' for nd in g.nodes()},
                     font_size=24,
                     font_family='arial',
                     font_weight='bold',
                     font_color='r',
                     alpha=1.,
                     node_color='#AAFFAA',
                     node_size=1500,
                     width=[0.3 for (u, v, d) in g.edges(data=True)],
                     arrowstyle='-|>',
                     arrowsize=25,
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


def plot_adjacency_matrix(g, fname='adjacency_mat.png'):
    size = 1000
    svg_fname = fname.replace('.png', '.svg')
    with cairo.SVGSurface(svg_fname, size, size) as surface:
        ctx = cairo.Context(surface)
        n = len(g)
        rectsize = float(size) / n
        g_nodes = list(g.nodes())
        for i in range(n):
            for j in range(n):
                val = 0
                if i == j or (g_nodes[i], g_nodes[j]) in g.edges():
                    val = 1
                val = 1 - val
                if val != 1:
                    val = float(val)
                    ctx.rectangle(i * rectsize, j * rectsize, rectsize, rectsize)
                    ctx.set_source_rgba(val, val, val, 1.)
                    ctx.fill()
            ctx.stroke()

    call(['convert', svg_fname, fname])
    os.remove(svg_fname)


def load_cayley_graph(filename):
    s = ''
    with open(filename, 'r') as f:
        for line in f:
            s += line
    fsa_info = eval(s)
    print(fsa_info)
    g = nx.DiGraph()
    i = 1
    for a, b, c in fsa_info:
        if a:
            g.add_edge(i, a)
        if b:
            g.add_edge(i, b)
        if c:
            g.add_edge(i, c)
        i += 1
    return g


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
            j = k
            i = k + 1
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


def parse_element(s):
    r = ''
    while r != s:
        r = s.replace('Xx', '')
        r = s.replace('xX', '')
        r = s.replace('Yy', '')
        r = s.replace('yY', '')
        s = r
    return s


def load_group_elements(filename):
    s = ''
    with open(filename, 'r') as f:
        for line in f:
            s += line
    return eval(s)


def load_group_edges(filename):
    s = ''
    with open(filename, 'r') as f:
        for line in f:
            s += line
    return [[x - y, y - 1] for x, y in eval(s)]


def make_graph_from_elements(elems, edges):
    g = nx.DiGraph()
    for e in edges:
        g.add_edge(elems[e[0]], elems[e[1]])
    return g


if __name__ == "__main__":
    plt.switch_backend('agg')
    # g = make_cayley_graph(9, 2)
    # plot_digraph(g, str((2, 3, 7)), 'cayley_graph.png')
    # plot_adjacency_matrix(g, 'adjacency_mat.png')
    for i in [7]:
        t = (2, 3, i)
        elems = load_group_elements('group_elements_' + str(i) + '.txt')
        edges = load_group_edges('group_edges_' + str(i) + '.txt')
        g = make_graph_from_elements(elems, edges)
        # for s in elems:
        #     p = parse_element(s)
        #     print(s, '->', p)
        # g = load_cayley_graph('cayley_graph_%d.txt' % i)
#        print('graph', g.edges())
        plot_digraph(g, str(t), 'cayley_graph_%d%d%d.png' % t)
        # plot_adjacency_matrix(g, 'adjacency_mat_%d%d%d.png' % t)
