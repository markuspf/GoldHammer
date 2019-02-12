#!/usr/bin/env python3


import os
import sys
# import numpy as np
# import matplotlib.pyplot as plt
import sympy
# import imageio
# import skimage
# import skimage.data
# import skimage.color
from subprocess import call


from cayley_graph_utils import *


def get_true_result(image_filename):
    p, q, r = os.path.basename(filename).split('_')[2].split('-')
    return r


def load_image(image_filename):
    return imageio.imread(image_filename)


class CayleyGraph:
    def __init__(self, gdata):
        self.graph, self.gens = gdata
        nodes = list(self.graph.nodes())
        self.dists = list(nx.shortest_path_length(self.graph, source=nodes[0]).values())
        self.generators = list(self.gens[0].values())
        self.max_dist = max(self.dists)
        self.cycles = []

    def add_identity(self, word):
        self.ideitities += [word]

    def check_has_identities(self, word):
        for ident in self.identities:
            if ident in word:
                return True
        return False


class CayleyGraphVisitor:
    def __init__(self, cg):
        self.cg = cg
        self.loc = 0
        self.path = [self.loc]
        self.trans = []

    def step(self, transition, check_seen=True):
        graph, gens = self.cg.graph, self.cg.gens
        new_loc = -1
        if self.loc in self.cg.gens:
            for dst in self.cg.gens[self.loc]:
                if self.cg.gens[self.loc][dst] == transition:
                    new_loc = dst
        if self.loc == -1 or (check_seen and new_loc in self.path[1:]):
            return False
        self.loc = new_loc
        self.path += [self.loc]
        self.trans += [transition]
        return True

    def walk(self, transitions, check_seen=True):
        for tr in transitions:
            if not self.step(tr, check_seen=check_seen):
                return False
        return True

    def is_cycle(self):
        return self.loc == 0 and len(self) > 0

    def distance(self):
        return self.cg.dists[self.loc]

    def __str__(self):
        return str(self.path)

    def __len__(self):
        return len(self.trans)


def is_valid_simple_cycle(cg, length, word):
    graph, gens = cg.graph, cg.gens
    nodes = list(graph.nodes())
    vis = CayleyGraphVisitor(cg)
    for tr in word * length:
        if not vis.step(tr, check_seen=True) or len(vis) + vis.distance() > len(word) * length:
            return False
    if vis.is_cycle():
        return True
    return False


def check_words_of_length(cg, length, word_length, word=[]):
    graph, gens = cg.graph, cg.gens
    if len(word) == word_length:
        res = is_valid_simple_cycle(cg, length=length, word=word)
        # if res:
            # print(str(length) + '-word', word, vis)
        return res
    vis = CayleyGraphVisitor(cg)
    if not vis.walk(word):
        return False
    for g in cg.generators:
        new_word = word + [g]
        if check_words_of_length(cg, length, word_length, new_word):
            return True
    return False


def has_cycle_of_length(cg, length, maxw=-1, word=[]):
    if maxw == -1:
        maxw = len(cg.graph.nodes())
    graph, gens = cg.graph, cg.gens
    nodes = list(graph.nodes())
    for i in range(maxw):
        wlen = i + 1
        print('%s-cycle of word length' % length, wlen)
        if wlen * length > len(cg.graph.nodes()):
            break
        elif check_words_of_length(cg, length, wlen):
            return True
    return False


def make_trans_from_path(cg, path):
    graph, gens = cg.graph, cg.gens
    trans = []
    for i in range(len(path) - 1):
        trans += [gens[path[i]][path[i + 1]]]
    return trans


def find_cycles(cg, function, path=[0]):
    graph, gens = cg.graph, cg.gens
    nodes = list(graph.nodes())
    if len(path) + cg.dists[path[-1]] > cg.max_dist * 2.5:
        return
    for out in graph.neighbors(nodes[path[-1]]):
        id = nodes.index(out)
        if id == 0:
            function(path + [id])
        elif id not in path:
            find_cycles(cg, function, path=path+[id])



divs = []
def process(gdata, y_true):
    graph, gens = gdata
    cg = CayleyGraph(gdata)
    p, q, r, = y_true
    # print('%s-cycle' % p, has_cycle_of_length(cg, p))
    # print('%s-cycle' % q, has_cycle_of_length(cg, q))
    # print('%s-cycle' % r, has_cycle_of_length(cg, r))
    # results = find_cycles(cg)
    def path_function(path):
        global divs
        print('path', path)
        trans = make_trans_from_path(cg, path)
        cur_div = len(trans)
        for d in sympy.divisors(len(trans))[::-1]:
            if trans == trans[:int(len(trans) / d)] * d:
                cur_div = d
                break
        if cur_div not in divs and cur_div > 1:
            divs += [cur_div]
    find_cycles(cg, path_function)
    print(divs)


if __name__ == '__main__':
    # plt.switch_backend('agg')
    dir = './data'
    for filename in os.listdir(dir):
        filename = dir + '/' + filename
        # dataset = {}
        if 'cg_edges' in filename and int(os.path.basename(filename).split('_')[3]) in range(7):
            # adj_filename = filename.replace('edges', 'adjacency').replace('.txt', '.png')
            # print(adj_filename)
            # if os.path.isfile(adj_filename):
            #     img = load_image(adj_filename)
            #     img = skimage.img_as_ubyte(img)
            #     print(img)
            # pass

            p, q, r = os.path.basename(filename).split('_')[2].split('-')
            p, q, r = int(p), int(q), int(r)
            print(filename, p, q, r)
            elems = load_group_elements(filename.replace('edges', 'elements'))
            edges = load_group_edges(filename)
            graph, gens = make_graph_from_elements(elems, edges)
            process((graph, gens), (p, q, r))
            # sys.exit(0)

            # labels = []
            # def label_to_int(label):
            #     global labels
            #     if label in labels:
            #         return labels.index(label)
            #     # 0 for no edge
            #     labels += [label]
            #     return len(labels)
            # sample = min(50, len(gens))
            # matrix = [
            #     [label_to_int(gens[i][j]) if (i in gens and j in gens[i]) else 0 for j in range(sample)]
            #     for i in range(sample)
            # ]
            # vector = []
            # for m in matrix:
            #     vector += m
            # data = np.array(vector)

            # y_true = get_true_result(filename)
            # if y_true not in dataset:
            #     dataset[y_true] = []
            # dataset[y_true] += [data]
            # print(data)
