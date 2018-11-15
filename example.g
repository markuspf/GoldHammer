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
  for i in [1..100] do
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

GenerateCayGraph := function(G, radius)
  # G is finitely presented
  local
    graph,
    rws, opt, reduced_f, F,
    queue, seen, depths,
    x_ind, x, g, h, h_ind,
    break_kb, finished;
  rws := KBMAGRewritingSystem(G);
  SetInfoLevel(InfoRWS, 1);
  opt := OptionsRecordOfKBMAGRewritingSystem(rws);
  opt.maxstoredlen := 2000 * [1, 1];
  opt.maxstates := 20000;
  KnuthBendix(rws);

  F := FreeGroup(Size(GeneratorsOfGroup(G)));
  reduced_f := function(g_w)
    local g_gens, f_gens, fw;
    g_gens := GeneratorsOfGroup(G);
    f_gens := GeneratorsOfGroup(F);
    fw := MappedWord(g_w, g_gens, f_gens);
    return ReducedWord(rws, fw);
  end;

  graph := Digraph([[1], []]);
  seen := [Identity(G)];
  depths := [0];
  queue := [[Identity(G), 1]];

  finished := false;
  while not finished and Length(queue) > 0 do
    x := queue[1][1];
    x_ind := queue[1][2];
    queue := Filtered(queue, x -> not x = queue[1]);

    Print("Pop ", x, "\n");
    Print("BEGIN LOOKAHEAD\n");

    for g in GeneratorsOfGroup(G) do
      h := x * g;

      Print("Consider ", h, "\n");

      h_ind := 1;
      break_kb := false;
      while h_ind <= Length(seen) and not break_kb do
        Print("h_ind: ", h_ind, "\n");
        if reduced_f(h) = reduced_f(seen[h_ind]) then
          break_kb := true;
        else
          h_ind := h_ind + 1;
        fi;
      od;

      Print("\tNew index: ", h_ind, "\n");

      if h_ind = Length(seen) + 1 and not depths[x_ind] = radius then
        Print("SEEN: ", seen, "\n");
        Print("VERTICES: ", DigraphVertices(graph), "\n");
        Print("NEW EDGE: ", [x_ind, h_ind], "\n");
        graph := DigraphAddVertex(graph);
        graph := DigraphAddEdge(graph, [x_ind, h_ind]);
        Append(seen, [h]);
        Append(queue, [[h, h_ind]]);
        Append(depths, [depths[x_ind] + 1]);
      elif h_ind <= Length(seen) and not x_ind = h_ind then
        Print("EXISTING: ", h_ind, "\n");
        Print("NEW EDGE (", x_ind, ", ", h_ind, ")\n");
        graph := DigraphAddEdge(graph, [x_ind, h_ind]);
        if depths[x_ind] + 1 < depths[h_ind] then
          depths[h_ind] := depths[x_ind] + 1;
        fi;
      fi;

      if depths[h_ind] > radius then
        finished := true;
        break;
      fi;
    od;
    Print("END LOOKAHEAD\n");
  od;
  Print("END FUNCTION\n");
  return graph;
end;

t := TriangleGroup(2, 3, 7);
# kb := KBMAGRewritingSystem(t);
# KnuthBendix(kb);
t := FpUglify(t);
cay := GenerateCayGraph(t, 10);
# Display(cay);
