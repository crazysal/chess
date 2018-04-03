//File--------------------------TurnInfo.h--------------------------------- 
 
#ifndef __IR_TURN_INFO_H__ 
#define __IR_TURN_INFO_H__ 

#include <algorithm>
 
/** Class TurnInfo represents the raw data extracted by the Perl script 
    from the engine output.  The only *written* change to the raw data is  
    applying a hard DELTACAP of 10.00, because the Perl script translated  
    mates to this value, and engines may use this or a 5.00 cap in searches. 
    It computes actual "falloff" in two basic ways ways: 
    () turnFalloff = eval(this turn) - eval(next turn), signed. 
    () deltaFalloff = deltas[playedMoveIndex] when playedMoveIndex < NCUTOFF, 
       = box(deltas[NCUTOFF - 1], turnFalloff, falloffFix(deltas[NCUTOFF - 1])) 
       otherwise, where box(a,b,c) = if b < a then a else min(b,c). 
    When there is no next turn, the "falloffFix(...)" estimate is used.  Also: 
    () turnFalloffCapped applies the soft Trial::deltaCap value, and 
    () deltaFalloffCapped ditto. 
 
    Class TupleInfo represents scaled data and is used for all "predictions". 
    Scaling is done by class Trial while "filtering".  Deltas may be  
    adjusted both by capping large ones at the soft Trial::deltaCap 
    and by DeltaScale::mapDelta(...).  The cap is applied *before* scaling. 
    The class includes a constant field for one "actual" falloff stat: 
 
    () scaledFalloff = scaledDeltas[playedMoveIndex] if playedMoveIndex < N, 
          deltaScale->mapDelta(box([unscaled Turninfo stats])) otherwise.  So 
       scaledFalloff = deltaScale->mapDelta(source->deltaFalloff) in all cases. 
 
 
    The falloffFix is used much more actively to compute the /predicted/ stats: 
    () scaledFalloff.projectedValue = falloff using the scaled deltas, 
    () unscaledFalloff.projectedValue = applies deltaCap but not the scale, and hence 
       is the "pred" of TurnInfo::deltaFalloffCapped.   
    Thus the soft Trial::deltaCap is regarded as a feature of the data, while 
    the DeltaScale is not.  A contributing reason is that the soft cap value 
    may later be used to make engines more efficient while gathering data. 
    Clear as mud? 
 */ 
     
 
 
 
/** Raw Data pertaining to a game turn.  Not acted on directly---rather, 
    information is "filtered" and scaled when translating to a DecisionInfo 
    object.  NOT Catalogable---these classes want to be non-virtual and fast. 
    Indeed, perhaps I should de-virtualize the destructor and operator string; 
    this class would be "sealed" in C# lingo. 
 
    Constructed ONLY by Ensemble::readInputFile(istream& file). 
    INV: is legal at construction.

    ** Evals are in centipawn (int) units and from White's view unless otherwise said. **

    Most pathological case: some Rybka turns with eval outside -3.00..+3.00
    have just the depth-1 eval, for 1 move, owing to Rybka "jackrabbiting".
    It is AOK on these turns to consider that value as applying to all moves
    at all depths.  The lo..hi "window" setting will fill those values in,
    helped by the "n.a."-->IRutil::NAVAL values read and recorded here.
 */ 

/** TAMAL: Right now, this class is not fast, rather it consists of all the raw information
regarding the move.  It stores all the information of any particular turn for all the depths.
 */ 

class TurnInfo { 
 public: 
   const GameInfo* pGameInfo;    //AIF parent game or parent EPD file
   const int turnLowestDepth, turnHighestDepth; // = range of depths of AIF parent (game).
                       //Might be larger than the data actually read.
                       //Could be different for different turns?  Superseded by "infer"
   TurnInfo* prevTurn;  //do not delete---GameInfo manages
   TurnInfo* nextTurn;  //set from outside
   const string gameID;
   const string engineID;
   const int turnNumber; 
   const bool whiteToMove; 
   Move movePlayed;       //not const since set in ctor body
   Move engineMove;  
   const int currEvalCP;  //current evaluation in centipawns, from White's view
   const int prevEvalCP;  //eval of previous turn in game (opponent's), or NA
   const int nextEvalCP;  //eval of next turn in game, or NA if none
   const int turnFalloffCP;  //from player's view, i.e., nonnegative.
   //const int depth;
   const int nodeCount;
   const string fenCurrentPos;
   const int fiftyMR;
   const int repeatCount;
   const vector<Move>* repMoves;
   const vector<Move>* rep2Moves;
   const string repLine2;
   const size_t numLegalMoves;
   vector<Move>* legalMoves;       

   /** Evaluations read from AIF file in row-major order, in centipawns.
       Inner vector over depths indexed [lowestDepth .. highestDepth]
       Outer vector indexed [0 .. numLegalMoves-1].  All from White's view.
                     */
   vector <vector<int> >* depthEvalsCP; //mutable?, indexed from 0, use AIFoffset
   
   //Derived fields---some given, others adjusted later
   const int playedMoveIndex; 
   bool inRep;    //allows over-arching reason to exclude a move
   const bool firstLineMatch; 
   const bool eqTopMatch; 
   const int rawAD; //stored as 'n.a.' in AIF 
   const int srandSeed;     //fixed at construction, used to make sure the 
                            //"same" permutation is used each time the 
                            //corresponding scaled tuple is rebuilt. 
   //const int minDepthWithValue;  //1-based
   //const int maxDepthWithValue;
   const int numDepths;
   const size_t numConsideredMoves;        // ideally >= min(numPV,numLegalMoves)
   const vector<Move>* consideredMoves;          //ordered by engine at highest depth
   vector<Move>* bestMoveByDepth;  //filled in later
   vector<int>* bestEvalCPByDepth;           //again, from White's view 
   vector<int>* worstRecordedEvalCPByDepth;  //for use in m-PV mode, m = 10 or 20 or more

 
   TurnInfo(const GameInfo* pGmInfo, const int tlo, const int thi,
            const string& gid, const string& eid, const int tNumber, const bool wtm,
            const string& plydMove, const string& engMove, //const string& pmlan, const string& emlan,
            const int curEval, const int preEval, const int nexEval,
            const int d, const int nc,  const string& fenCurrPos, const int fmr,
            const int repCount, const vector<Move>* rm1, const string& rl2, 
            const size_t numLegalMov, const string& legMoves,
            const vector<Move>* cmv, vector<vector<int> >* devals,  //INV: parallel order
            const int playedMoveInd, bool extraRepeatFlag = false) 
    : pGameInfo(pGmInfo)
    , turnLowestDepth(tlo)
    , turnHighestDepth(thi)
    , prevTurn(NULL)
    , nextTurn(NULL)  //set from outside
    , gameID(gid)
    , engineID(eid)
    , turnNumber(tNumber)
    , whiteToMove(wtm)
    , movePlayed(Move())
    , engineMove(Move())
    , currEvalCP(curEval)
    , prevEvalCP(preEval)
    , nextEvalCP(nexEval)
      //turnFalloffCP(wtm ? curEval - nexEval : nexEval - curEval),
    , turnFalloffCP(fabs(curEval - nexEval))
    //, depth(d)
    , nodeCount(nc), fenCurrentPos(fenCurrPos), fiftyMR(fmr)
    , repeatCount(repCount), repMoves(rm1), rep2Moves(new vector<Move>()), repLine2(rl2)
    , numLegalMoves(numLegalMov)
    , legalMoves(new vector<Move>())
    , depthEvalsCP(devals)
    , playedMoveIndex(playedMoveInd)
    , inRep(extraRepeatFlag)
    , firstLineMatch(playedMoveInd == 0)
    , eqTopMatch(playedMoveInd < devals->size() && devals->at(playedMoveInd).at(thi-1) == curEval)  //use hi2 derived depth?
    , rawAD(wtm ? max(curEval-nexEval,0) : max(nexEval-curEval,0))
    , srandSeed(rand())
    //, minDepthWithValue(1), maxDepthWithValue(d)
    , numDepths(d) //can be greater than turnHighestDepth
    , numConsideredMoves(cmv->size())
    , consideredMoves(cmv) //bestMoveByDepth(new vector<Move>()),
    , bestMoveByDepth(new vector<Move>(d,Move()))
    , bestEvalCPByDepth(new vector<int>(d,IRutil::NAVAL))
    , worstRecordedEvalCPByDepth(new vector<int>(d,IRutil::NAVAL))
   { 
//cerr << "@";
      string lm = legMoves;
      while (lm.find(',') != string::npos) {
         lm.replace(lm.find(','), 1, " ");
      }
      istringstream iss(lm);
      string mv;
      Position pos(fenCurrPos);
      while (iss >> mv) {
         legalMoves->push_back(Move(pos.move2TAN(mv)));
      }
      if (legalMoves->size() != numLegalMov) {
         cerr << "Legal Move miscount: " << legalMoves->size() << " of "
              << numLegalMov << " loaded in turn" << endl << fenCurrPos << endl
              << "from " << legMoves << endl;
      }
      movePlayed.setTAN(pos.move2TAN(plydMove));
      engineMove.setTAN(pos.move2TAN(engMove));

      for (size_t d = tlo; d <= numDepths; d++) {  //absolute depths, leave defaults for d < tlo
         int btm = (whiteToMove ? 1 : -1);
         int bestEvalCPPTM = -IRutil::MATEVALCP;  //PTM = from player perspective
         int worstEvalCPPTM = IRutil::MATEVALCP;
         int i = 0;
         size_t useDepth = (d > thi ? thi : d);
         Move bestMove;
         while (i < numConsideredMoves) {
            int wtmEval = getEvalCP(i,useDepth);
            if (wtmEval != IRutil::PRUNVAL && wtmEval != IRutil::NAVAL
                && wtmEval != IRutil::NRECVAL) {
               int ptmEval = btm*wtmEval;
               if (ptmEval > bestEvalCPPTM) {
                  bestEvalCPPTM = ptmEval;
                  bestMove = cmv->at(i);
               } 
               if (ptmEval < worstEvalCPPTM) {
                  worstEvalCPPTM = ptmEval;
               }
            }
            i++;
         }
         if (bestEvalCPPTM == -IRutil::MATEVALCP) { bestEvalCPPTM = IRutil::NAVAL; }
         if (worstEvalCPPTM == IRutil::MATEVALCP) { worstEvalCPPTM = IRutil::NAVAL; }
         bestMoveByDepth->at(d-IRutil::AIFoffset) = bestMove;
         bestEvalCPByDepth->at(d-IRutil::AIFoffset) = btm*bestEvalCPPTM;  //back to White view
         worstRecordedEvalCPByDepth->at(d-IRutil::AIFoffset) = btm*worstEvalCPPTM;
         
      }
   } 
 
   virtual ~TurnInfo() { 
      delete(depthEvalsCP); //OK since value matrix
      delete(legalMoves);
      delete(consideredMoves);
      delete(repMoves);
      delete(rep2Moves);
      delete(bestMoveByDepth);
      delete(bestEvalCPByDepth);
      delete(worstRecordedEvalCPByDepth);
   }

   int getEvalCP(size_t i, size_t d) const {  //REQ indices in range: Caller guards
      return depthEvalsCP->at(i).at(d-IRutil::AIFoffset); //CAN return NAVAL etc.
   } 

   int findEvalCP(size_t i, size_t d) const {  //REQ: i in range, will fudge d, not used
      size_t s = depthEvalsCP->at(i).size();
      if (d > s) { d = s; }
      int evalCP = depthEvalsCP->at(i).at(d-IRutil::AIFoffset);
      while (evalCP >= IRutil::NAVAL && d != 6) {
         d = (d < 6 ? d+1 : d-1);   //!!! 6 is a magic # here
         evalCP = depthEvalsCP->at(i).at(d-IRutil::AIFoffset);
      }
      return evalCP;
   }

   int getBestEvalCPAtDepth(size_t d) const {  //treats d as absolute and sub-data depths as NA
      if (d > turnHighestDepth) { d = turnHighestDepth; }
      if (d < turnLowestDepth) { return IRutil::NAVAL; }
      return bestEvalCPByDepth->at(d-IRutil::AIFoffset);
   }

   double getRawSwingCPPTM(size_t i, size_t D, bool normalize) const {  //always from player perspective
      int swingCP = 0;
      int btm = (whiteToMove ? 1 : -1);
      size_t targetDepth = (D > turnHighestDepth ? turnHighestDepth : D);
      int topDelta = btm*(getBestEvalCPAtDepth(targetDepth) - getEvalCP(i, targetDepth));
      for (size_t d = turnLowestDepth; d < targetDepth; d++) {  //d = D skipped
         swingCP += btm*(getBestEvalCPAtDepth(d) - getEvalCP(i, d)) - topDelta;
      }
      return (normalize ? swingCP/double(targetDepth - turnLowestDepth) : swingCP);
   }
         
   void setRep(bool isRep) { inRep = isRep; }

   string getDepthEvalCPstr() const {
      string depthEvalInfo = "";
      string st;
      for( int i = 0; i< depthEvalsCP->size() ; i++) {
         vector<int> moveEvals = depthEvalsCP->at(i);
         string mv = (i < consideredMoves->size() ? consideredMoves->at(i).TAN2SAN() : "???");
         string pad = "     ";
         depthEvalInfo += mv + pad.substr(min(mv.length(),pad.length()));
         for( int j = 0; j < moveEvals.size() ; j++) {
            if (moveEvals.at(j) < IRutil::NAVAL) {
               st =  IRutil::itoa(moveEvals.at(j),5);
            } else {
               st = "?????";
               st = (moveEvals.at(j) == IRutil::NAVAL) ? " n.a." : st;
               st = (moveEvals.at(j) == IRutil::PRUNVAL) ? " PRUN" : st;
               st = (moveEvals.at(j) == IRutil::NRECVAL) ? " NREC" : st;
            }
            depthEvalInfo += st + " ";
         }
         depthEvalInfo += "\n";
      }
      return depthEvalInfo;
   }

   string getFEN() const { return IRutil::trim(fenCurrentPos); }
 
   bool match() { return (engineMove == movePlayed); }
   bool topMatch() {
      return match() || (whiteToMove ? (turnFalloffCP <= 0) : (turnFalloffCP >= 0)); 
   }

   operator string() const { 
   //note: Black moves already include "..." as part of them 
       
      string line1 = pGameInfo->whitePlayer 
            + " - " + pGameInfo->blackPlayer
            + ", " + pGameInfo->eventName + ", " + string(pGameInfo->date) 
            + " : " + pGameInfo->engineID; 
      string line2 = "Played move: " + string(movePlayed) + ", index " + IRutil::itoa(playedMoveIndex);
      string line3 = "Engine move: " + IRutil::itoa(turnNumber) + ". " 
            + string(engineMove) + ", considered " + IRutil::itoa(numConsideredMoves);
      string line4 = "Eval end-" + IRutil::itoa(turnHighestDepth) + ": " //maxDepthWithValue) + ": "  
            + IRutil::ftoa(currEvalCP/double(100),2) + "  (previously " 
            + IRutil::ftoa(prevEvalCP/double(100),2) + ")"; 
      // 
      string line5 = getDepthEvalCPstr();

      return line1 + "\n" + line2 + "\n" + line3 + "\n" + line4 + "\n" + line5; 
   } 
 
   string toString() const { return string(*this); }

   bool operator<(const TurnInfo& rhs) const { return getFEN() < rhs.getFEN(); }

   
}; 
 
 
 
#endif    //end of #ifndef __IR_TURN_INFO_H__ 
 
 
 
 
