F := FreeGroup("x", "y");
T1 := F / [ F.1^2, F.2^3, (F.1*F.2)^7 ];

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


