F := FreeGroup("x", "y");
T1 := F / [ F.1^2, F.2^3, (F.1*F.2)^7 ];

TriangleGroup := function(p,q,r)
   local F;
   F := FreeGroup("x", "y");
   return F / [ F.1^p, F.2^q, (F.1*F.2)^r ];
end;
 
