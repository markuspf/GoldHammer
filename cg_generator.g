LoadPackage("kbmag");
LoadPackage("digraphs");


TriangleGroup := function(p,q,r)
   local F;
   F := FreeGroup("x", "y");
   return F / [ F.1^p, F.2^q, (F.1*F.2)^r ];
end;

# Adds a generator
FpAddGenerator := function(G, word)
    local n, F, R, F2, gens, gens2;

    F := FreeGroupOfFpGroup(G);
    gens := GeneratorsOfGroup(F);

    # TODO: word has to be 1 in G!
    n := Size(gens);
    F2 := FreeGroup(n + 1);
    gens2 := GeneratorsOfGroup(F2){[1..n]};

    R := List(RelatorsOfFpGroup(G), r -> MappedWord(r, gens, gens2));
    Add(R, F2.(n+1)^-1 * MappedWord(word, gens, gens2));

    return F2 / R;
end;

FpAddRelator := function(G, word)
    local F, R;

    F := FreeGroupOfFpGroup(G);
    R := ShallowCopy(RelatorsOfFpGroup(G));

    # TODO: has to be a word over f and 1 in G
    Add(R, word);

    return F / R;
end;

FpReplaceRelatorsByProduct := function(G, i, j)
    local F, R;

    F := FreeGroupOfFpGroup(G);
    R := ShallowCopy(RelatorsOfFpGroup(G));

    # TODO: has to be a word over f and 1 in G
    Add(R, R[i] * R[j]);
    Remove(R, i);

    return F / R;
end;

FpUglify := function(G)
  local GENERATE, RELATE, REPLACE, i, x, rels, inds;
  GENERATE := 1;
  RELATE := 2;
  REPLACE := 3;
  for i in [1..10] do
    x := RandomList([RELATE]);
    rels := List(RelatorsOfFpGroup(G));
    if x = GENERATE then
      G := FpAddGenerator(G, Product(Shuffle(rels){[1..2]}));
    elif x = RELATE then
      G := FpAddRelator(G, Product(Shuffle(rels){[1..2]}));
    elif x = REPLACE then
      inds := Shuffle([1..Length(rels)]){[1..2]};
      G := FpReplaceRelatorsByProduct(G, inds[1], inds[2]);
    fi;
  od;
  return G;
end;

GenerateCayGraph := function(G, rws, elems)
  local graph, n, e, g_ind, g, h, h_ind, gens;

  graph := Digraph([[], []]);

  g_ind := 1;
  for g in elems do
    graph := DigraphAddVertex(graph, g_ind);
    g_ind := g_ind + 1;
  od;

  g := elems[1];
  # gens := Filtered(elems, x -> Length(String(x)) = Length(String(g)));
  gens := elems{[1..2]};

  g_ind := 1;
  for g in elems do
    h_ind := 1;
    for h in elems do
      for e in gens do
        n := ReducedWord(rws, g * e);
        if n = h then
          Print(n, " == ", h, "\n");
          graph := DigraphAddEdge(graph, [g_ind, h_ind]);
        fi;
      od;
      h_ind := h_ind + 1;
    od;

    g_ind := g_ind + 1;
  od;
  return graph;
end;

# kb := KBMAGRewritingSystem(t);
# KnuthBendix(kb);
# t := FpUglify(t);
# Display(cay);
for i in [8] do
  t := TriangleGroup(2, 3, i);
  t := FpUglify(t);
  rws := KBMAGRewritingSystem(t);
  rc := OptionsRecordOfKBMAGRewritingSystem(rws);
  a := AutomaticStructure(rws);
  r := WordAcceptor(rws);
  PrintTo(Concatenation("cayley_graph_", String(i), ".txt"), r.table.transitions);
  elems := EnumerateReducedWords(rws, 1, 12);
  Append(elems, [ReducedWord(rws, elems[1] * elems[1]^-1)]);
  PrintTo(Concatenation("group_elements_", String(i), ".txt"), List(elems, x -> String(x)));
  cay := GenerateCayGraph(t, rws, elems);
#  Display(DigraphVertices(cay));
  PrintTo(Concatenation("group_edges_", String(i), ".txt"), DigraphEdges(cay));
od;
