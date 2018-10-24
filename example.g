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
