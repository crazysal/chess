//File-------------------MoveSelector.h--------------------------------------

#ifndef __IR_MOVESELECTION_H__
#define __IR_MOVESELECTION_H__

#include <set>
#include <map>
#include <algorithm>

typedef bool (*MoveSelectorFn)(const TurnInfo* const, const Move&);

enum COMPARE { LEQ, EQ, GEQ };  //NOT < = >, but with <= and >=

class MoveSelector : public Catalogable {
 protected:        //C++: accessible to *immediate* subclasses only
   string name;
   bool excludeTurnIfEmpty;
   bool isSet;
   set<Move>* selectedMoves;  //changes with each turn
   MoveSelector(const string& gname, bool exc) 
    : name(gname), excludeTurnIfEmpty(exc), isSet(false), selectedMoves(new set<Move>()) 
   { }
   MoveSelector() 
    : name("anon"), excludeTurnIfEmpty(false), isSet(false), selectedMoves(new set<Move>()) 
   { }
 public:
   virtual ~MoveSelector() { delete(selectedMoves); }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const = 0;
   void makeExclude() { excludeTurnIfEmpty = true; }
   void makeInclude() { excludeTurnIfEmpty = false; }
   bool excludesTurnIfEmpty() const { return excludeTurnIfEmpty; }
   void declareSet() { isSet = true; }
   void makeSet(const TurnInfo* const pTurn) {
      selectedMoves->clear();
      vector<Move>::const_iterator itr = pTurn->legalMoves->begin();
      while (itr != pTurn->legalMoves->end()) {
         if (apply(pTurn, *itr)) {
            selectedMoves->insert(*itr);
         }
      }
      isSet = true;
   }

   /** Loading of turns is "lazy"---only when user wishes to iterate.  Hence side effect.
    *  Neat thing is that this code is needed only here in the abstract parent class.
    */
   virtual set<Move>::const_iterator begin(const TurnInfo* const pTurn) {
      makeSet(pTurn);
      return selectedMoves->begin();
   }
   virtual set<Move>::const_iterator end() const {
      return selectedMoves->end();
   }

   bool knownEmpty() const { return (isSet && selectedMoves->size() == 0); }

   virtual string getName() const { 
      string x = (excludeTurnIfEmpty ? "X" : "");
      return name + x; 
   }
   virtual string getDescription() const = 0;
   virtual operator string() const {  //works for many subclasses
      return getName() + ": " + getDescription();
   }
};

class FnSelector: public MoveSelector {
   MoveSelectorFn selectorFn;
 public:
   FnSelector(MoveSelectorFn gselfn, const string& name, bool exc)
      : MoveSelector(name, exc), selectorFn(gselfn) { }
   virtual ~FnSelector() { }  //parent destructor called automatically
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      return (*selectorFn)(pTurn, move);
   }
   virtual string getDescription() const {
      return "FnSelector";
   }
};

class NotSelector: public MoveSelector {
   MoveSelector* theSelector;
 public:
   NotSelector(MoveSelector* selector, const string& name, bool exc)
      : MoveSelector(name, exc), theSelector(selector) { }
   virtual ~NotSelector() { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      return !(theSelector->apply(pTurn, move));
   }
   virtual string getDescription() const {
      return "NotSelector [" + theSelector->getName() + "]";
   }
};


class AndSelector : public MoveSelector {
   list<MoveSelector*> conjuncts;
 public:
   AndSelector(const string& name, bool exc)
      : MoveSelector(name, exc), conjuncts(list<MoveSelector*>()) { }
   AndSelector(MoveSelector* lhs, MoveSelector* rhs, const string& name, bool exc)
      : MoveSelector(name, exc), conjuncts(list<MoveSelector*>())
   {
      conjuncts.push_back(lhs);
      conjuncts.push_back(rhs);
   }
   virtual ~AndSelector() { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      bool ans = true;
      for (list<MoveSelector*>::const_iterator it = conjuncts.begin();
           it != conjuncts.end(); it++) {
         if (!(*it)->apply(pTurn, move)) {
            ans = false;
            break;
         }
      }
      return ans;
   }
   void addConjunct(MoveSelector* tf) {
      conjuncts.push_back(tf);
   }

   virtual string getDescription() const {
      string out = "AndSelector [";
      for (list<MoveSelector*>::const_iterator it = conjuncts.begin();
           it != conjuncts.end(); it++) {
         out += (*it)->getName() + ",";
      }
      if (conjuncts.empty()) {
         out += "]";
      } else {
         out[out.size() - 1] = ']';
      }
      return out;
   }

   virtual operator string() const {
      string out = getName() + ": AndSelector [";
      for (list<MoveSelector*>::const_iterator it = conjuncts.begin();
           it != conjuncts.end(); it++) {
         out += string(**it) + ", ";    //string() makes this more verbose
      }
      if (conjuncts.empty()) {
         out += "]";
      } else {
         out[out.size() - 2] = ']';
      }
      return out;
   }
};

class OrSelector : public MoveSelector {
   list<MoveSelector*> disjuncts;
 public:
   OrSelector(const string& name, bool exc)
      : MoveSelector(name, exc), disjuncts(list<MoveSelector*>()) { }
   OrSelector(MoveSelector* lhs, MoveSelector* rhs, const string& name, bool exc)
      : MoveSelector(name, exc), disjuncts(list<MoveSelector*>())
   {
      disjuncts.push_back(lhs);
      disjuncts.push_back(rhs);
   }
   virtual ~OrSelector() { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      bool ans = false;
      for (list<MoveSelector*>::const_iterator it = disjuncts.begin();
           it != disjuncts.end(); it++) {
         if ((*it)->apply(pTurn, move)) {
            ans = true;
            break;
         }
      }
      return ans;
   }
   void addDisjunct(MoveSelector* tf) {
      disjuncts.push_back(tf);
   }

   virtual string getDescription() const {
      string out = "OrSelector [";
      for (list<MoveSelector*>::const_iterator it = disjuncts.begin();
           it != disjuncts.end(); it++) {
         out += (*it)->getName() + ",";
      }
      if (disjuncts.empty()) {
         out += "]";
      } else {
         out[out.size() - 1] = ']';
      }
      return out;
   }

   virtual operator string() const {
      string out = getName() + ": OrSelector [";
      for (list<MoveSelector*>::const_iterator it = disjuncts.begin();
           it != disjuncts.end(); it++) {
         out += (**it).getDescription() + ", " ;
      }
      if (disjuncts.empty()) {
         out += "]";
      } else {
         out[out.size() - 2] = ']';
      }
      return out;
   }

};


class MappedMoveSelector : public MoveSelector {
   map<string, set<Move>*>* fenMap;
 public:
   MappedMoveSelector(const string& name, bool exc)
    : MoveSelector(name, exc), fenMap(new map<string, set<Move>*>())
   { 
      declareSet();
   }
   virtual ~MappedMoveSelector() {
      map<string, set<Move>*>::iterator itr = fenMap->begin();
      while (itr != fenMap->end()) {
         delete(itr->second);
         itr++;
      }
      delete(fenMap);
   }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      map<string, set<Move>*>::iterator mitr = fenMap->find(pTurn->getFEN());
      if (mitr != fenMap->end()) {
         set<Move>::iterator sitr = mitr->second->find(move);
         return (sitr != mitr->second->end());
      } //else
      return false;
   }
   void addTurn(const TurnInfo* const pTurn, const vector<Move>& moves) {
      fenMap->operator[](pTurn->getFEN()) = new set<Move>();
      for (vector<Move>::const_iterator citr = moves.begin(); citr != moves.end(); citr++) {
         fenMap->at(pTurn->getFEN())->insert(*citr);
      }
   }
   size_t mapSize() const { return fenMap->size(); }
   virtual string getDescription() const {
      return "TurnMap";
   }
};
     
class OmniSelector : public MoveSelector {
 public:
   OmniSelector(): MoveSelector("Omni",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      return true;
   }
   virtual string getDescription() const {
      return "always true";
   }
};

class NullSelector : public MoveSelector {
 public:
   NullSelector(): MoveSelector("NullSelector",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      return false;
   }
   virtual string getDescription() const {
      return "always false";
   }
};

class PawnMoveSelector : public MoveSelector {
 public:
   PawnMoveSelector() : MoveSelector("PawnMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      char piece = move.getTAN()[0];
      bool wtm = pTurn->whiteToMove;
      return ((wtm && piece == 'P') || ((!wtm) && piece == 'p'));
   }
   virtual string getDescription() const {
      return "Pawn move";
   }
};

class KnightMoveSelector : public MoveSelector {
 public:
   KnightMoveSelector() : MoveSelector("KnightMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      char piece = move.getTAN()[0];
      bool wtm = pTurn->whiteToMove;
      return ((wtm && piece == 'N') || ((!wtm) && piece == 'n'));
   }
   virtual string getDescription() const {
      return "Knight move";
   }
};

class BishopMoveSelector : public MoveSelector {
 public:
   BishopMoveSelector() : MoveSelector("BishopMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      char piece = move.getTAN()[0];
      bool wtm = pTurn->whiteToMove;
      return ((wtm && piece == 'B') || ((!wtm) && piece == 'b'));
   }
   virtual string getDescription() const {
      return "Bishop move";
   }
};

class RookMoveSelector : public MoveSelector {
 public:
   RookMoveSelector() : MoveSelector("RookMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      char piece = move.getTAN()[0];
      bool wtm = pTurn->whiteToMove;
      return ((wtm && piece == 'R') || ((!wtm) && piece == 'r'));
   }
   virtual string getDescription() const {
      return "Rook move";
   }
};

class QueenMoveSelector : public MoveSelector {
 public:
   QueenMoveSelector() : MoveSelector("QueenMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      char piece = move.getTAN()[0];
      bool wtm = pTurn->whiteToMove;
      return ((wtm && piece == 'Q') || ((!wtm) && piece == 'q'));
   }
   virtual string getDescription() const {
      return "Queen move";
   }
};

class KingMoveSelector : public MoveSelector {
 public:
   KingMoveSelector() : MoveSelector("KingMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      char piece = move.getTAN()[0];
      bool wtm = pTurn->whiteToMove;
      return ((wtm && piece == 'K') || ((!wtm) && piece == 'k'));
   }
   virtual string getDescription() const {
      return "King move";
   }
};

class CastlingSelector : public MoveSelector {
 public:
   CastlingSelector() : MoveSelector("Castling",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      string tanMove = move.getTAN();
      return (tanMove.length() == 7 && tanMove.at(6) == 'c');
   }
   virtual string getDescription() const {
      return "Castling";
   }
};

class CaptureSelector : public MoveSelector {
 public:
   CaptureSelector() : MoveSelector("Capture",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      string tanMove = move.getTAN();
      return (tanMove.at(3) == 'x');
   }
   virtual string getDescription() const {
      return "Capture";
   }
};

class NonCaptureSelector : public MoveSelector {
 public:
   NonCaptureSelector() : MoveSelector("NonCapture",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      string tanMove = move.getTAN();
      return (tanMove.at(3) != 'x');
   }
   virtual string getDescription() const {
      return "Non-capture";
   }
};

class PromotionSelector : public MoveSelector {
 public:
   PromotionSelector() : MoveSelector("Promotion",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      string tanMove = move.getTAN();
      if (tanMove.length() < 7) { return false; }
      char pp = tanMove.at(6);
      bool wtm = pTurn->whiteToMove;
      return ((wtm && (pp == 'Q' || pp == 'N' || pp == 'R' || pp == 'B'))
            || ((!wtm) && (pp == 'q' || pp == 'n' || pp == 'r' || pp == 'b')));
   }
   virtual string getDescription() const {
      return "Promotion";
   }
};

class AdvancingMoveSelector : public MoveSelector {
 public:
   AdvancingMoveSelector() : MoveSelector("AdvancingMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      string tanMove = move.getTAN();
      bool wtm = pTurn->whiteToMove;
      return ((wtm && tanMove.at(2) < tanMove.at(5)) || (!wtm) && tanMove.at(2) > tanMove.at(5));
   }
   virtual string getDescription() const {
      return "Advancing move";
   }
};

class RetreatingMoveSelector : public MoveSelector {
 public:
   RetreatingMoveSelector() : MoveSelector("RetreatingMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      string tanMove = move.getTAN();
      bool wtm = pTurn->whiteToMove;
      return ((wtm && tanMove.at(2) > tanMove.at(5)) || (!wtm) && tanMove.at(2) < tanMove.at(5));
   }
   virtual string getDescription() const {
      return "Retreating move";
   }
};

class SidewaysMoveSelector : public MoveSelector {
 public:
   SidewaysMoveSelector() : MoveSelector("SidewaysMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      string tanMove = move.getTAN();
      bool wtm = pTurn->whiteToMove;
      return (tanMove.at(2) == tanMove.at(5));
   }
   virtual string getDescription() const {
      return "Sideways move";
   }
};

class CheckingMoveSelector : public MoveSelector {
 public:
   CheckingMoveSelector() : MoveSelector("CheckingMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      Position pos(pTurn->getFEN());
      pos.playMove(move);
      return pos.inCheck();
   }
   virtual string getDescription() const {
      return "Checking move";
   }
};

class EngineMoveSelector : public MoveSelector {
 public:
   EngineMoveSelector(): MoveSelector("EngineMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      return (move == pTurn->engineMove);
   }
   virtual string getDescription() const {
      return "Engine's first line";
   }
};

class PlayedMoveSelector : public MoveSelector {
 public:
   PlayedMoveSelector(): MoveSelector("PlayedMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      return (move == pTurn->movePlayed);
   }
   virtual string getDescription() const {
      return "The move played";
   }
};

class SamePieceAsPrevMoveSelector : public MoveSelector {
 public:
   SamePieceAsPrevMoveSelector(): MoveSelector("SamePieceAsPrevMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      TurnInfo* ppTurn = pTurn->prevTurn;
      if (ppTurn == NULL) { return false; }
      ppTurn = pTurn->prevTurn;
      if (ppTurn == NULL) { return false; }
      string tanMove = move.getTAN();
      string prevTAN = ppTurn->movePlayed.getTAN();
      return (prevTAN.at(4) == tanMove.at(1) && prevTAN.at(5) == tanMove.at(5));
   }
   virtual string getDescription() const {
      return "Same piece as player moved last";
   }
};

class EqualTopMoveSelector : public MoveSelector {
 public:
   EqualTopMoveSelector(): MoveSelector("EqualTopMove",false) { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      int cpeval = pTurn->depthEvalsCP->at(0).at(pTurn->turnHighestDepth-1);
      int index = 0;
      while (index < pTurn->numConsideredMoves
                && pTurn->depthEvalsCP->at(index).at(pTurn->turnHighestDepth-1) == cpeval) {
         if (pTurn->consideredMoves->at(index) == move) { return true; }
         index++;
      }
      return false;
   }
   virtual string getDescription() const {
      return "Same value as computer first move";
   }
};

class CompareSelector: public virtual MoveSelector {
   const double rhs;
   const COMPARE comparison;
 protected:
   CompareSelector(COMPARE gc, double r, bool nullExcludes, const string& gname)
         : MoveSelector(gname, nullExcludes), comparison(gc), rhs(r) {
      name = gname;
   }

 public:
   virtual ~CompareSelector() { }

   bool test(double lhs) const {
      bool ans;
      switch(comparison) {
       case LEQ:
         ans = lhs <= rhs;
         break;
       case EQ:
         ans = lhs == rhs;
         break;
       case GEQ:
         ans = lhs >= rhs;
         break;
      }
      return ans;
   }

   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const = 0;

   string relation() const {
      string rel = "";
      switch(comparison) {
       case LEQ:
         rel = "<=";
         break;
       case EQ:
         rel = "==";
         break;
       case GEQ:
         rel = ">=";
         break;
      }
      return rel;
   }

   virtual string getDescription() const {
      return relation() + " " + IRutil::ftoa(rhs);
   }
};

class MoveEvalSelector : public CompareSelector {
   double thresholdEval;   //in pawn not centipawn units, for show in print
   size_t atDepth;         //threshold passed as "geval" to parent
   bool playerView;
 public:
   MoveEvalSelector(const COMPARE comp, const double geval, const size_t gdepth,
                    bool ptm, bool nullExcludes, const string& name)
    : CompareSelector(comp, geval, nullExcludes, name)
    , thresholdEval(geval)
    , atDepth(gdepth)
    , playerView(ptm)
   { }
   virtual ~MoveEvalSelector() { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      size_t i = 0;
      int evalToTest;
      size_t useDepth;
      while (i < pTurn->numConsideredMoves) {
         if (pTurn->consideredMoves->at(i) == move) {
            if (atDepth == 0 || atDepth > pTurn->turnHighestDepth) {
               useDepth = pTurn->turnHighestDepth;
            } else if (atDepth < pTurn->turnLowestDepth) {
               useDepth = pTurn->turnLowestDepth;
            }
            int wtmEval = pTurn->getEvalCP(i,useDepth);
            if (wtmEval != IRutil::PRUNVAL && wtmEval != IRutil::NAVAL
                && wtmEval != IRutil::NRECVAL) {
               evalToTest = (((!playerView) || pTurn->whiteToMove) ? wtmEval : -wtmEval);
               return test(evalToTest/100.0);
            }
            return false;  //move does not have recorded value
         }
         i++;
      }
      return false;  //move not considered
   }

   virtual string getDescription() const {
      string who = (playerView ? "Player" : "White");
      string dstr = (atDepth == 0 ? "max" : IRutil::itoa(atDepth));
      return "Move Eval is " + relation() + " " + IRutil::ftoa(thresholdEval) 
            + " for " + who + " at depth " + dstr;
   }
};

class MoveRawDeltaSelector : public CompareSelector {
   double thresholdDelta;  //for show; tested value passed to parent.  In p.nn pawn units
   size_t atDepth;
 public: 
   MoveRawDeltaSelector(const COMPARE comp, const double gdelta, const size_t gdepth,
                     bool nullExcludes, const string& name)
    : CompareSelector(comp, gdelta, nullExcludes, name)
    , thresholdDelta(gdelta)
    , atDepth(gdepth)
   { }
   virtual ~MoveRawDeltaSelector() { }

   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      size_t i = 0;
      size_t useDepth;
      while (i < pTurn->numConsideredMoves) {
         if (pTurn->consideredMoves->at(i) == move) {
            if (atDepth == 0 || atDepth > pTurn->turnHighestDepth) {
               useDepth = pTurn->turnHighestDepth;
            } else if (atDepth < pTurn->turnLowestDepth) {
               useDepth = pTurn->turnLowestDepth;
            }
            int wtmEval = pTurn->getEvalCP(i,useDepth);
            int bestEval = pTurn->getBestEvalCPAtDepth(useDepth);
            return test((bestEval - wtmEval)/double(100));
         }
         i++;
      }
      return false;
   }

   virtual string getDescription() const {
      string dstr = (atDepth == 0 ? "max" : IRutil::itoa(atDepth));
      return "Move delta is " + relation() + " " + IRutil::ftoa(thresholdDelta)
            + " at depth " + dstr;
   }
};

class MoveRawSwingSelector : public CompareSelector {
   double thresholdSwing;  //can be negative
   size_t targetDepth;     //D in definition of swing, 0 for max
   bool normalize;
 public:
   MoveRawSwingSelector(const COMPARE comp, const double gdelta, const size_t gdepth,
                     bool norm, bool nullExcludes, const string& name)
    : CompareSelector(comp, gdelta, nullExcludes, name)
    , thresholdSwing(gdelta)
    , targetDepth(gdepth)
    , normalize(norm)
   { }
   virtual ~MoveRawSwingSelector() { }

   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      size_t i = 0;
      size_t useDepth = targetDepth;
      while (i < pTurn->numConsideredMoves) {
         if (pTurn->consideredMoves->at(i) == move) {
            if (targetDepth == 0 || targetDepth > pTurn->turnHighestDepth) {
               useDepth = pTurn->turnHighestDepth;
            } else if (targetDepth < pTurn->turnLowestDepth) {
               useDepth = pTurn->turnLowestDepth; //will return 0 for swing
            }
            double swingToTest = pTurn->getRawSwingCPPTM(i, useDepth, normalize);
            return test(swingToTest/double(100));
         }
         i++;
      }
      return false;
   }

   virtual string getDescription() const {
      string dstr = (targetDepth == 0 ? "max" : IRutil::itoa(targetDepth));
      string normstr = (normalize ? "normalized is " : "is ");
      return "Move swing " + normstr + relation() + " " + IRutil::ftoa(thresholdSwing)
            + " targeting depth " + dstr;
   }
};



#endif


 
