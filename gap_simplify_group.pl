#!/usr/bin/env perl


use strict;
use warnings;


sub func_quote {
    return '"'.(shift).'"'
}

<>;
my @gens = ();
while(<>) {
    chomp;
    my $line = $_;
    last if not length $line;
    # print("line '$line'\n");
    push @gens, $line;
}

<>;
my @rels = ();
my $i = 0;
while(<>) {
    chomp;
    my $line = $_;
    last if not length $line;
    push @rels, $line if $i >= 2 * @gens;
    ++$i;
}

my $gens_s = join ', ', map { func_quote $_ } @gens;
my $rels_s = join ', ', map { func_quote $_ } @rels;

print <<EOF
f := FreeGroup($gens_s);
relators_s := [ $rels_s ];
relators := List(relators_s, x -> ParseRelators(f, x)[1]);
Display(relators);
t := f / relators;
Display("UGLIFIED GROUP");
Display(t);
t := SimplifiedFpGroup(t);
Display("SIMPLIFIED GROUP");
Display(t);
EOF
