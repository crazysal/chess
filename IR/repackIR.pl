#!/usr/bin/perl
# File "repackIR.pl", by KWR.  Packs C++ code for IR (back) into 1 file.
# UNLIKE "unpackIR.pl", requires knowing all the individual pieces

if ($ARGV[0]) {
   my $tag = $ARGV[0];

   system("cat IRincludes.h IRutil.h IRfunctions.h Position.h IRmiscGlobal.h TrialSpec.h GameInfo.h TurnInfo.h TupleInfo.h TurnContext.h DeltaScales.h DecisionInfo.h IRmodels.h MoveSelector.h Filters.h PerfData.h PerfData.cpp Shuffle.h Trial.h Trial.cpp TrialFilters.h multimin.h multimin.c Minimizer.h Minimizer.cpp Menus.h Demos.h Ensemble.h Ensemble.cpp Demos.cpp IRmain.cpp > IRall1file$tag.cpp") or die("cat failed: $?\n");

} else {
   die("Please enter two or more initials as a file tag.");
}
