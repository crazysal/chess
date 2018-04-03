//File--------------------------TurnContext.h---------------------------------

#ifndef __IR_TURN_CONTEXT_H__
#define __IR_TURN_CONTEXT_H__

/** TurnContext supplies information about the current turn needed to
    compute scaling.  It holds the current evaluation /at a given depth/,
    which may differ from the evaluation at the highest reference depth.
    It also holds the evaluations of the previous and next moves, which 
    are used for filtering and are at the perceivedDepth.
    Intended to be created on-the-fly.
    Pre-scaled evaluations are held as a.bc doubles, not as ints.
    Eval is from White's point of view.
 */

class TurnContext {        //plain old data
 public:
   string TANmove;
   string engineID;
   bool whiteToMove;
   double currEval;  //in pawn .00 not cp units, can be depth-dependent
   double prevEval;
   double nextEval;
   TurnContext(const string& smove, const string& eid, bool gwtm, double gce, double gpe, double gne)
    : TANmove(smove), engineID(eid), whiteToMove(gwtm), currEval(gce), prevEval(gpe), 
      nextEval(gne)
   { }
   operator string() const { 
      ostringstream oss;
      string wtm = whiteToMove ? "wtm" : "btm";
      oss << TANmove << ": " << currEval << ", prev: " << prevEval 
          << ", next: " << nextEval << ", " << wtm;
      return oss.str();
   }
};
   
#endif

 
