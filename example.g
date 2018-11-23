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
    x := RandomList([GENERATE, RELATE, REPLACE]);
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

# GenerateCayGraph := function(G, radius)
#   # G is finitely presented
#   local
#     graph,
#     rws, opt, reduced_f, F,
#     queue, seen, depths,
#     x_ind, x, g, h, h_ind,
#     break_kb, finished;
#   rws := KBMAGRewritingSystem(G);
#   SetInfoLevel(InfoRWS, 1);
#   opt := OptionsRecordOfKBMAGRewritingSystem(rws);
#   opt.maxstoredlen := 2000 * [1, 1];
#   opt.maxstates := 20000;
#   KnuthBendix(rws);
#
#   F := FreeGroup(Size(GeneratorsOfGroup(G)));
#   reduced_f := function(g_w)
#     local g_gens, f_gens, fw;
#     g_gens := GeneratorsOfGroup(G);
#     f_gens := GeneratorsOfGroup(F);
#     fw := MappedWord(g_w, g_gens, f_gens);
#     return ReducedWord(rws, fw);
#   end;
#
#   graph := Digraph([[1], []]);
#   seen := [Identity(G)];
#   depths := [0];
#   queue := [[Identity(G), 1]];
#
#   finished := false;
#   while not finished and Length(queue) > 0 do
#     x := queue[1][1];
#     x_ind := queue[1][2];
#     queue := Filtered(queue, val -> not reduced_f(val[1]) = reduced_f(x));
#
#     Print("Pop ", x, "\n");
#     Print("BEGIN LOOKAHEAD\n");
#
#     for g in GeneratorsOfGroup(G) do
#       h := x * g;
#
#       Print("Consider ", h, "\n");
#
#       h_ind := 1;
#       break_kb := false;
#       while h_ind <= Length(seen) and not break_kb do
#         Print("h_ind: ", h_ind, "\n");
#         if reduced_f(h) = reduced_f(seen[h_ind]) then
#           break_kb := true;
#         else
#           h_ind := h_ind + 1;
#         fi;
#       od;
#
#       Print("\tNew index: ", h_ind, "\n");
#
#       if h_ind = Length(seen) + 1 and not depths[x_ind] = radius then
#         Print("SEEN: ", seen, "\n");
#         Print("VERTICES: ", DigraphVertices(graph), "\n");
#         Print("NEW EDGE: ", [x_ind, h_ind], "\n");
#         graph := DigraphAddVertex(graph);
#         graph := DigraphAddEdge(graph, [x_ind, h_ind]);
#         Append(seen, [h]);
#         Append(queue, [[h, h_ind]]);
#         Append(depths, [depths[x_ind] + 1]);
#         if depths[h_ind] > radius then
#           finished := true;
#           break;
#         fi;
#       elif h_ind <= Length(seen) and not x_ind = h_ind then
#         Print("EXISTING: ", h_ind, "\n");
#         Print("NEW EDGE (", x_ind, ", ", h_ind, ")\n");
#         graph := DigraphAddEdge(graph, [x_ind, h_ind]);
#         if depths[x_ind] + 1 < depths[h_ind] then
#           depths[h_ind] := depths[x_ind] + 1;
#         fi;
#       fi;
#     od;
#     Print("END LOOKAHEAD\n");
#   od;
#   Print("END FUNCTION\n");
#   return graph;
# end;


GenerateCayGraph := function(G, radius)
  local
    graph,
    rws, opt, reduced_f, F, elems,
    queue, seen, depths,
    g_ind, g, h, h_ind,
    break_kb, finished;
  rws := KBMAGRewritingSystem(G);
  SetInfoLevel(InfoRWS, 1);
  # opt := OptionsRecordOfKBMAGRewritingSystem(rws);
  # opt.maxstoredlen := 2000 * [1, 1];
  # opt.maxstates := 20000;
  AutomaticStructure(rws);
  elems := EnumerateReducedWords(rws, 1, 11);

  F := FreeGroup(Size(GeneratorsOfGroup(G)));
  reduced_f := function(g_w)
#    local g_gens, f_gens, fw;
#    g_gens := GeneratorsOfGroup(G);
#    f_gens := GeneratorsOfGroup(F);
#    fw := MappedWord(g_w, g_gens, f_gens);
#    return ReducedWord(rws, fw);
#    return ReducedWord(rws, g_w);
    return g_w;
  end;

  graph := Digraph([[], []]);

  g_ind := 1;
  for g in elems do
    Display(g);
#    Display(g_ind);
#    Print("Take ", g, "\n");

    for x in [elems[1], elems[2]] do
      h := g * x;
      h_ind := 1;

      break_kb := false;
      while not break_kb and h_ind < g_ind do
#        Print("Consider ", h, "\n");
#        Print("h_ind: ", h_ind, "\n");
        if reduced_f(g) = reduced_f(h) then
          break_kb := true;
        else
          h_ind := h_ind + 1;
        fi;
      od;

      #    Print("\tNew index: ", h_ind, "\n");

      if h_ind = g_ind then
#        Print("ADD EDGE [", g_ind, ", ", h_ind, "]\n");
        graph := DigraphAddVertex(graph);
        graph := DigraphAddEdge(graph, [g_ind, h_ind]);
      elif h_ind < g_ind then
        Print("ADD EDGE [", g_ind, ", ", h_ind, "]\n");
        graph := DigraphAddEdge(graph, [g_ind, h_ind]);
      fi;
    od;

    g_ind := g_ind + 1;
  od;
  return graph;
end;

# kb := KBMAGRewritingSystem(t);
# KnuthBendix(kb);
# t := FpUglify(t);
# Display(cay);
for i in [7] do
  t := TriangleGroup(2, 3, i);
  rws := KBMAGRewritingSystem(t);
  rc := OptionsRecordOfKBMAGRewritingSystem(rws);
  a := AutomaticStructure(rws);
  r := WordAcceptor(rws);
  PrintTo(Concatenation("cayley_graph_", String(i), ".txt"), r.table.transitions);
  elems := EnumerateReducedWords(rws, 1, 15);
  elems := List(elems, x -> String(x));
  PrintTo(Concatenation("group_elements_", String(i), ".txt"), elems);
  cay := GenerateCayGraph(t, elems);
#  Display(DigraphVertices(cay));
  PrintTo(Concatenation("group_edges_", String(i), ".txt"), DigraphEdges(cay));
od;
