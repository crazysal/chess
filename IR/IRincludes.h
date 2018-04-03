//File-----------------------IRincludes.h----------------------------- 
 
#ifndef __IR_INCLUDES_H__ 
#define __IR_INCLUDES_H__ 
 
/** File "IRmain.cpp" or "IRall1file.cpp"  CREDITS:
- 500-line original by Venkateswaran Yamachandran, Spring 2008
- First full working version by Kenneth W. Regan, August 2008
- Maintained through August 2008 by KWR
- Extended to read Analysis Interchange Format by Tamal Biswas, Fall 2012.
  AIF itself programmed by Biswas
- Extended by Biswas and KWR to implement depth=of-calculation parameter(s).
Copyright (c) Kenneth W. Regan.  Not yet licensed under GPL.

Usage: Compile //with highest optimization//, no makefile needed.   
       Run a.out and follow menus and prompts. 
 
REQUIRES: Data file "RefTurns.aif" in same directory as executable.
Data files of readable analysis have extension .aif and this format:
 
[Event "7th Mikhail Tal Memorial"] 
[Site "Moscow RUS"] 
[Date "2012.06.08"] 
[Round "1"] 
[White "Radjabov,T"] 
[Black "Tomashevsky,E"] 
[Result "1-0"] 
[ECO "C45"] 
[WhiteElo "2784"] 
[BlackElo "2738"] 
[PlyCount "??"] 
[EventDate "2012.06.08"] 
[Source "??"] 
[SourceDate "??"] 
[ply "??"] 
[PlyDepth "13"] 
[EngineID "Rybka 3 1-cpu"] 
 
1. e4 e5 2. Nf3 Nc6 3. d4 exd4 4. Nxd4 Bc5 5. Nb3 Bb6 6. Nc3 d6 7. Qe2 Nge7 8. Be3 O-O 9. O-O-O f5 10. exf5 Bxf5 11. h3 Bd7 12. Qd2 Bxe3 13. Qxe3 Kh8 14. Bd3 Qe8 15. f4 Qf7 16. Rhf1 Rae8 17. Qd2 Nb4 18. Be4 Bc6 19. Rde1 Bxe4 20. Nxe4 Qc4 21. a3 Nbc6 22. Qc3 Qd5 23. Nbd2 Nf5 24. g4 Nfd4 25. Qd3 b5 26. Kb1 b4 27. a4 h6 28. Nb3 Re7 29. Ned2 Rxe1+ 30. Rxe1 g5 31. f5 1-0 
 
[GameID "Radjabov;Tomashevsky;2012.06.08;7th;1"] 
[MoveNo "9-w"] 
[MovePlayed "O-O-O"] 
[EngineMove "O-O-O"] 
[FEN  "r1bq1rk1/ppp1nppp/1bnp4/8/4P3/1NN1B3/PPP1QPPP/R3KB1R w KQ - 4 9"] 
[RepCount "0"] 
[Eval " n.a."] 
[PrevEval " n.a."] 
[NextEval " +004"] 
[LegalMoves "Ra1-b1, Ra1-c1, Ra1-d1, a2-a3, a2-a4, Nb3-a5, Nb3-c5, Nb3-d4, Nb3-d2, Nb3-c1, Nc3-b1, Nc3-a4, Nc3-b5, Nc3-d5, Nc3-d1, Ke1-d2, Ke1-d1, Qe2-d2, Qe2-d1, Qe2-f3, Qe2-g4, Qe2-h5, Qe2-d3, Qe2-c4, Qe2-b5, Qe2-a6, Be3-c1, Be3-d2, Be3-f4, Be3-g5, Be3-h6, Be3-d4, Be3-c5, Be3xb6, e4-e5, f2-f3, f2-f4, g2-g3, g2-g4, Rh1-g1, h2-h3, h2-h4, 0-0-0"] 
[LegalMovesSAN "Rb1, Rc1, Rd1, a3, a4, Na5, Nc5, Nd4, Nd2, Nc1, Nb1, Na4, Nb5, Nd5, Nd1, Kd2, Kd1, Qd2, Qd1, Qf3, Qg4, Qh5, Qd3, Qc4, Qb5, Qa6, Bc1, Bd2, Bf4, Bg5, Bh6, Bd4, Bc5, Bxb6, e5, f3, f4, g3, g4, Rg1, h3, h4, 0-0-0"] 
[NumLegalMoves "43"] 
[FiftyMR "4"] 
[RepLine1 ""] 
[RepLine2 ""] 
 
------------------------------------------------------------------------ 
         1    2    3    4    5    6    7    8    9   10   11   12   13 
------------------------------------------------------------------------ 
0-0-0  n.a. +008 +009 -003 +006 +004 +009 +001 +006 +011 +010 +009 +005 
a3     n.a. -026 -008 -004 -001 -016 -008 -009 -018 -018 -008 -005  000 
f4     n.a. -043 -052 -040 -054 -010 -010 -005 -009 -015 -009 -005  000 
h3     n.a. -006 -001 -013 -013 -004 -008 -008 -009 -009 -009 -004 -005 
h4     n.a. -029 -005 -004 -002 -001 -004 -004 -004 -007 -004 -007 -007 
Qd2    n.a. -032 -020 -005 -005 -005  000 -014 -008 -005 -003 -014 -014 
Rg1    n.a. -040 -011 -034 -031 -031 -033 -029 -018 -021 -021 -016 -016 
f3     n.a. -013 -008 -010 -009 -014 -024 -021 -017 -024 -020 -020 -016 
Bxb6   n.a. -058 -053 -053 -046 -047 -047 -037 -033 -026 -018 -017 -017 
g3     n.a. -064 -039 -029 -019 -029 -028 -029 -029 -021 -026 -026 -022 
Qd3    n.a. -096 -070 -068 -060 -057 -054 -030 -036 -031 -024 -024 -026 
Qh5    n.a. -075 -069 -076 -042 -038 -028 -029 -043 -032 -032 -030 -027 
Qf3    n.a. -065 -052 -047 -030 -037 -033 -028 -025 -027 -048 -032 -032 
Nd2    n.a. -037 -028 -048 -032 -036 -044 -028 -048 -058 -041 -041 -038 
Bg5    n.a. -062 -048 -053 -023 -031 -024 -033 -033 -042 -037 -037 -039 
a4     n.a. -044 -032 -037 -031 -037 -052 -051 -048 -045 -045 -043 -046 
Nb5    n.a. -051 -041 -060 -031 -050 -061 -045 -045 -056 -056 -056 -047 
Bd2    n.a. -042 -047 -051 -038 -036 -037 -038 -042 -044 -045 -044 -048 
Qd1    n.a. -090 -087 -058 -096 -041 -041 -067 -056 -056 -056 -051 -051 
Rd1    n.a. -051 -055 -060 -052 -060 -060 -060 -055 -056 -056 -055 -053 
Na4    n.a. -055 -054 -042 -041 -053 -075 -049 -049 -056 -056 -056 -054 
Nd1    n.a. -089 -091 -096 -082 -077 -070 -052 -067 -063 -068 -055 -055 
Bf4    n.a. -073 -044 -056 -053 -054 -056 -058 -057 -057 -058 -054 -063 
Nb1    n.a. -094 -087 -096 -082 -077 -075 -061 -067 -067 -084 -061 -064 
Bc1    n.a. -086 -078 -070 -067 -071 -056 -063 -067 -076 -072 -064 -064 
g4     n.a. -093 -073 -067 -053 -052 -049 -066 -063 -059 -059 -059 -065 
Qb5    n.a. -095 -080 -090 -084 -073 -070 -088 -077 -062 -062 -062 -066 
Nc1    n.a. -072 -079 -071 -098 -089 -092 -082 -075 -067 -069 -064 -069 
Rc1    n.a. -061 -071 -066 -070 -077 -078 -078 -072 -064 -075 -084 -070 
Rb1    n.a. -061 -071 -078 -082 -082 -076 -076 -073 -090 -079 -079 -073 
Nd5    n.a. -060 -080 -075 -065 -080 -080 -097 -089 -084 -084 -095 -075 
Qc4    n.a. -105 -121 -108 -112 -111 -108 -105 -090 -089 -089 -094 -082 
Kd2    n.a. -087 -092 -116 -111 -332 -340 -343 -343 -341 -342 -342 -342 
e5     n.a. -128 -146 -138 -143 -152 -139 -140 -139 -138 -140 -140 -132 
Kd1    n.a. -087 -127 -118 -121 -139 -133 -131 -144 -143 -121 -121 -135 
Bh6    n.a. -238 -241 -241 -248 -253 -252 -231 -252 -252 -248 -245 -245 
Nd4    n.a. -294 -303 -298 -294 -295 -284 -296 -288 -299 -294 -288 -283 
Nc5    n.a. -286 -291 -283 -288 -294 -280 -275 -282 -284 -298 -298 -288 
Na5    n.a. -290 -288 -298 -297 -286 -293 -304 -312 -320 -319 -310 -308 
Bd4    n.a. -349 -343 -348 -351 -341 -344 -330 -340 -329 -327 -328 -325 
Bc5    n.a. -330 -339 -332 -339 -345 -349 -349 -349 -355 -357 -345 -349 
Qe2    n.a. n.a. n.a. n.a. n.a. n.a. -340 -340 -320 -338 -338 -325 -325 
Ba6    n.a. n.a. n.a. n.a. n.a. -317 PRUN PRUN PRUN PRUN PRUN PRUN PRUN 
Qg4    n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. 
Qa6    n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. n.a. 
 
======================================================================== 
 
[GameID "Radjabov;Tomashevsky;2012.06.08;7th;1"] 
[MoveNo "9-b"] 
[MovePlayed "f7-f5"] 
[EngineMove "f7-f5"] 
[FEN  "r1bq1rk1/ppp1nppp/1bnp4/8/4P3/1NN1B3/PPP1QPPP/2KR1B1R b - - 5 9"] 
[RepCount "0"] 
[Eval " +004"] 
[PrevEval " n.a."] 
[NextEval " +004"] 
[LegalMoves "a7-a6, a7-a5, Ra8-b8, Bb6-a5, Bb6xe3, Bb6-d4, Bb6-c5, Nc6-b4, Nc6-a5, Nc6-b8, Nc6-e5, Nc6-d4, Bc8-h3, Bc8-g4, Bc8-f5, Bc8-e6, Bc8-d7, d6-d5, Qd8-e8, Qd8-d7, Ne7-d5, Ne7-g6, Ne7-f5, f7-f6, f7-f5, Rf8-e8, g7-g6, g7-g5, Kg8-h8, h7-h6, h7-h5"] 
[LegalMovesSAN "a6, a5, Rb8, Ba5, Bxe3, Bd4, Bc5, Nb4, Na5, Nb8, Ne5, Nd4, Bh3, Bg4, Bf5, Be6, Bd7, d5, Qe8, Qd7, Nd5, Ng6, Nf5, f6, f5, Re8, g6, g5, Kh8, h6, h5"] 
[NumLegalMoves "31"] 
[FiftyMR "5"] 
[RepLine1 ""] 
[RepLine2 ""] 
 
------------------------------------------------------------------------ 
         1    2    3    4    5    6    7    8    9   10   11   12   13 
------------------------------------------------------------------------ 
f5     n.a. n.a. n.a. +005 +005 +005 +005 +005 +005 +005 +005 +005 +004 
Be6    n.a. n.a. n.a. +013 +013 +013 +013 +013 +013 +013 +013 +007 +010 
Ng6    n.a. +010 +010 +010 +010 +010 +010 +013 +013 +013 +013 +015 +016 
Re8    n.a. +013 +013 +012 +013 +013 +013 +007 +015 +015 +011 +017 +016 
h6     n.a. +012 +013 +012 +012 +012 +013 +013 +020 +013 +015 +015 +019 
Kh8    n.a. +033 +030 +026 +020 +036 +029 +028 +037 +031 +031 +021 +021 
Bd7    n.a. +020 +016 +023 +017 +025 +024 +024 +023 +021 +017 +021 +022 
Bxe3   n.a. n.a. n.a. +010 +010 +010 +010 +013 +017 +016 +018 +018 +023 
Na5    n.a. +041 +036 +043 +036 +033 +033 +027 +022 +025 +029 +025 +024 
Ne5    n.a. n.a. n.a. +024 +024 +024 +024 +020 +022 +023 +024 +025 +025 
Rb8    n.a. +022 +026 +028 +016 +020 +017 +019 +018 +020 +020 +016 +028 
Qd7    n.a. +029 +015 +013 +016 +028 +026 +020 +026 +016 +024 +026 +028 
Qe8    n.a. +024 +012 +026 +020 +026 +024 +022 +015 +022 +027 +029 +028 
Nb8    n.a. +056 +043 +041 +040 +040 +047 +049 +045 +049 +040 +037 +038 
Nb4    n.a. +042 +042 +035 +036 +038 +031 +038 +036 +039 +038 +034 +040 
f6     n.a. +024 +026 +026 +023 +025 +020 +018 +039 +031 +033 +037 +040 
a6     n.a. +027 +073 +070 +074 +060 +060 +055 +065 +066 +066 +063 +069 
Ba5    n.a. +100 +091 +088 +090 +080 +080 +077 +073 +080 +074 +075 +075 
a5     n.a. +034 +034 +045 +040 +040 +057 +057 +055 +054 +056 +055 +077 
g6     n.a. +041 +037 +047 +035 +045 +036 +080 +088 +082 +094 +084 +086 
d5     n.a. +087 +085 +092 +095 +108 +135 +139 +122 +122 +094 +086 +086 
h5     n.a. +077 +085 +089 +093 +098 +096 +091 +085 +096 +096 +090 +091 
g5     n.a. +135 +130 +136 +141 +150 +160 +153 +173 +180 +184 +184 +191 
Nf5    n.a. +179 +184 +171 +171 +177 +178 +178 +177 +183 +193 +190 +200 
Bf5    n.a. +169 +170 +159 +172 +186 +200 +203 +203 +206 +207 +212 +208 
Nd5    n.a. +281 +288 +288 +279 +277 +272 +273 +275 +269 +266 +274 +270 
Bg4    n.a. +240 +256 +262 +275 +277 +272 +270 +263 +270 +284 +280 +280 
Nd4    n.a. +290 +289 +291 +279 +277 +284 +295 +295 +294 +298 +300 +300 
Bh3    n.a. +293 +284 +297 +293 +294 +295 +299 +302 +301 +296 +302 +303 
Bd4    n.a. +300 +309 +304 +305 +309 +316 +314 +313 +315 +322 +313 +316 
Bc5    n.a. +312 +328 +326 +320 +322 +332 +330 +327 +326 +324 +321 +329 
 
======================================================================== 
 
 
OBJECTS: The Ensemble holds a list of TurnInfo records for each game turn in 
the data, which is further filtered into a list of DecisionInfo giving 
processed data for statistical analysis.  The Ensemble holds Catalogs of: 
 
() user-buildable TurnFilters for selecting subsets of the data; 
() user-definable TrialSpecs each specifying a curve, "s" and "c" 
   parameters, and a statistical weighting method; and 
() user-definable Trials, each linked to a mutable TrialSpec and specifying 
   information for statistical fitting routines. 
 
Statistical regression is done with temporary Minimizer objects, currently 
outfitted only with primitive binary-search minimization ("veeFinder" etc.) 
#Going to use GSL library for that.. 
of various least-squares distances and maximum-likelihood quantities. 
Fit results are stored into PerfData, which can export and import fullReports. 
One Trial and its TrialSpec have "focus" at any one time, and TurnFilters 
are attachable to and detachable from each trial separately.  Users can  
"hide" TrialSpecs (e.g. dummies used before fitting) and TurnFilters to 
keep screen lists tidy.  Demos hide (most) filters and specs they create. 
 
User interaction is governed by a menuing system (classes EnumMenu and 
DynamicMenu, which are composed into Catalogs) that logs all user commands 
by callback to the Ensemble via the Logging interface, to "IRcommandLog.txt". 
Users may select options by name or conveniently by number, while the command 
names are logged to make all sessions *replayable* from the main menu. 
Several suppied Demos illustrate the "mini programming language" of commands. 
Data is writen to screen and to "IRsessionData.txt", with the user able to 
specify other output files and/or close these streams.  Demos can use 
"IRdemos.txt" or another user-designated file, for output then input. 
Constants, limits, and math functions are defined in the IRutil namespace. 
 
 
USAGE:  The text-based menus try to be explanatory.  No help system (as yet). 
Users are automatically set up with one trial and a handful of filters. 
 
 
CODE NOTES:  Near-term to-do list: 
() Fully decide on and implement how to control/mute output to the screen. 
() Resolve whether to keep, improve, or discard "parsePerfData" from files. 
() Revisit public/private, especially for Ensemble and Minimizer classes. 
() Rearrange the Ensemble implementation code and clean up for consistency. 
 
Longer-term: 
() Implement better minimization routines (e.g. "gosset" [NJS] for C++?). 
() Add routines to do automated cheating-testing of specific sets of games. 
() Add more curves and weights and statistical methods and demos! 
() Collect and compile more chess-game data!! 
() Implement the full model with earlier ply depths and "swing"!!! 
 
The code follows Java and C# conventions where it is convenient to do so. 
Objects from classes with virtual functions or that embed arrays are held via 
explciit pointers, rather than references.  Code is (mostly) "const"-correct; 
indeed, modifications are done below (const) pointers by "const" methods. 
Distinctive features of C++ (as oppposed to MatLab, say): 
 
() Standard Template Library containers list and map (and of course vector). 
() Function objects used for TurnFilter and DeltaScale, factoring common code. 
() Extensive use of C++ streams, especially reading from stringstreams. 
() Numerical code needs to be *fast*, with many Newton iterations performed. 
() Inheritance and composition used extensively for menu/catalog system. 
 
*/ 
 
#include <iostream> 
#include <iomanip> 
#include <istream> 
#include <fstream> 
#include <sstream> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string> 
#include <cstring>
#include <cmath> 
#include <vector> 
#include <list> 
#include <set>
//#include <unordered_set>
#include <deque>
#include <map> 
#include <algorithm>  //for sort and max
#include <iterator>
#include <limits> 
#include <ctime> 
#include <glob.h>
#include <sys/param.h>
#include <unistd.h>

#define OS_UNIX 


#ifdef OS_UNIX 
#include <gsl/gsl_multimin.h>    //not redundant, loaded by multimin.c anyway
//#include "multimin.h" 
//#include "multiminBak.c"    //wrapper to nicer interface
//#include "gsl_multimin.c"

//#ifndef IR_MULTIMIN_C__
//#include "multimin.c"
//#endif 

#include <gsl/gsl_siman.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

//#include <cminpack.h>
#include <minpack.h>  // loads cminpack.h anyway

//#ifdef __cplusplus
//}
//#endif

#endif 
 
 
using namespace std; 
 
#endif    //end of #ifndef __IR_INCLUDES_H__ 
 
 
 
