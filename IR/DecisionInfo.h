//File-----------------------DecisionInfo.h------------------------------------ 
 
 
#ifndef __IR_DECISION_INFO_H__ 
#define __IR_DECISION_INFO_H__ 


/** Class DecisionInfo accesses depth info in absolute int units from TurnInfo,
    but stores them in relative 2-place units ("deltas") from overall eval.

    ** Whereas TurnInfo uses centipawn (int) units, DecisionInfo uses pawn (p.nn) units. **
    ** Evals are from White's view unless marked PTM; swings are always from player view. **

    The class reads data with depths dataLowDepth..dataHighDepth, but stores data
    only in the window userLowDepth..userHighDepth specified by theScale.  
    If userLowDepth < dataLowDepth, then TupleInfo objects for the lowest-depth true info
    are replicated, and similarly for the highest-depth info if userHighDepth > dataHighDepth.
    Note that userLowDepth > dataHighDepth is possible---e.g. when the engine cuts out early
    giving a quick mate---whereupon the top-depth info is replicated.
 */
class DecisionInfo { 
 public:
   const TurnInfo* pParent;
   //const size_t numLegalMoves;   //== pParent->numLegalMoves
   const size_t numMovesWithValues;
   const size_t playedMoveIndex;
   const bool useAltPlayedMoveIndex;
   const double deltaCap;
   const bool patchesProbs;
   const bool scalesDeltas;

   const DeltaScale* theScale;
   const size_t dataLowDepth; //== pParent->turnLowestDepth, least depth with move eval
   const size_t dataHighDepth;//== pParent->turnHighestDepth, greatest with a move eval
                              //Could be >= and <= respectively.  Currently passed in via
                              //Ensemble's readInputFile method, so assume correct here
   const size_t userLowDepth;       //==theScale->userLowDepth
   const size_t userHighDepth;      //==theScale->userHighDepth
   const size_t numTuples;//== hi - lo + 1

   vector <TupleInfo*> depthDeltasScaled;   //ALL CONSTRUCTED, not assigned, own copies
                                            //indexed [0 .. dataHighDepth-dataLowDepth]
   vector <TupleInfo*> depthDeltasUnscaled; //Important function: discrete comparisons for rank.
   vector <double>* decMoveSwings;          //indexed 0 .. numMovesWithValues-1
   vector <double> bestEvals;               //indexed [0 .. dataHighDepth-dataLowDepth]
   vector <double> worstEvals;              //"", both constructed not assigned
   //vector <Move> bestMoveAtDepth;         //duplicated in TurnInfo
   vector<size_t> indexFirstMoveAtDepth;    //the top-depth index/rank of the first
                                            //occurrence of the best value at depth d

   vector <TupleInfo*> windowDeltasScaled;  //indexed [0 .. userHi-userLo], not own copies
   vector <TupleInfo*> windowDeltasUnscaled;
   vector <double> windowBests;
   vector <double> windowWorsts;

   vector <double> unscaledFalloff;       //get from pParent
   vector <double> scaledFalloff;         //range over all depths
   const double turnFalloffCapped;        //TAMAL: do we need unscaledFalloffCapped ?? KWR: No.
   double bootWeight;

   //TupleInfo contains information about const vector<double>* const scaledDeltas;
   //Need to do it for all legal moves 

 public:
   DecisionInfo(const TurnInfo* parent, bool gscaledDeltas, DeltaScale* ds, bool pp, bool ua,
                vector<vector<int> >* evalSwap)  //evalSwap passed as space from Trial
    : 
      pParent(parent)
    , numMovesWithValues(min(parent->numLegalMoves, pParent->numConsideredMoves))   //parent->numLegalMoves)
      //use 1+ to give a "catchall" slot for moves not in top NumConsidered?
    , playedMoveIndex(parent->playedMoveIndex)
    , useAltPlayedMoveIndex(ua)
    , deltaCap(gscaledDeltas ? ds->getDeltaCap() : IRutil::DELTACAP)
    , patchesProbs(pp)
    , scalesDeltas(gscaledDeltas)
    , theScale(ds)
    , dataLowDepth(parent->turnLowestDepth)
    , dataHighDepth(parent->turnHighestDepth)
    , userLowDepth(ds->getLowDepth())
    , userHighDepth(ds->getHighDepth())
    , numTuples(ds->getHighDepth() - ds->getLowDepth() + 1)   //user not data
    , decMoveSwings(new vector<double>(min(parent->numLegalMoves,pParent->numConsideredMoves), 0.0))
    , indexFirstMoveAtDepth(vector<size_t>())
    , turnFalloffCapped(min(parent->turnFalloffCP/100.0, gscaledDeltas ? ds->getDeltaCap() : IRutil::DELTACAP)) 
    , bootWeight(1.0)
   { 
      IRutil::decsMade++;
      if (IRutil::decsMade % IRutil::TURNSMODULUS == 0) {
         cerr << "Created " << IRutil::decsMade << " DecisionInfo objects against "
              << IRutil::decsDeleted << " deleted." << endl;
      }

      if (numMovesWithValues == 0) {
         cerr << endl << "numLegal == " << parent->numLegalMoves << ", numConsidered = "
              << pParent->numConsideredMoves << " in turn " << parent->toString() << endl;
      }
      processTurnInfo(evalSwap); 
      resetWindow();
   } 

   ~DecisionInfo() {
      vector<TupleInfo*>::iterator vitr = depthDeltasScaled.begin();
      vector<TupleInfo*>::iterator vite = depthDeltasScaled.end();
      while (vitr != vite) {
         TupleInfo* tp = *vitr++;
         delete(tp);
      }
      vitr = depthDeltasUnscaled.begin();
      vite = depthDeltasUnscaled.end();
      while (vitr != vite) {
         TupleInfo* tp = *vitr++;
         delete(tp);
      }
      delete(decMoveSwings);
      IRutil::decsDeleted++;
      if (IRutil::decsDeleted % IRutil::TURNSMODULUS == 0) {
         cerr << "Deleted " << IRutil::decsDeleted << " of " << IRutil::decsMade <<
         " DecisionInfo objects." << endl;
      }
   }
 
   /** Get eval in pawn 0.00 units, conversion from centipawns done here.  
       *All depth arguments in the public interface are absolute.*
       Allows d > dataHighDepth as argument.
    */
   double getEvalP(size_t i, size_t d) const { 
      const size_t useDepth = (d > dataHighDepth ? dataHighDepth : (d < dataLowDepth ? dataLowDepth : d));
      int cpeval = pParent->getEvalCP(i,useDepth);    //uses absolute depths
      int btm = pParent->whiteToMove? 1 : -1;
      double eval;
      switch(cpeval) {
       case IRutil::PRUNVAL:
         eval = getBestEvalP(useDepth) - btm*IRutil::PRUNCAP; //PRUNCAP is in p.nn units
         break;
       case IRutil::NRECVAL:
         eval = getWorstEvalP(useDepth);
         break;
       case IRutil::NAVAL:   //fake it, use already-scaled value
         eval = getBestEvalP(useDepth) 
                   - btm*(depthDeltasScaled.at(useDepth - dataLowDepth)->getDelta(i));
         break;
       default:
         eval = cpeval/double(100);
      }
      return eval;
   }

   double getBestEvalP(size_t d) const {  
      const size_t useDepth = (d > dataHighDepth ? dataHighDepth : (d < dataLowDepth ? dataLowDepth : d));
      return bestEvals.at(useDepth - dataLowDepth);
   }
   double getWorstEvalP(size_t d) const { 
      const size_t useDepth = (d > dataHighDepth ? dataHighDepth : (d < dataLowDepth ? dataLowDepth : d));
      return worstEvals.at(useDepth - dataLowDepth);
   }

   double getSwingScaled(size_t i, size_t D, bool normalize) const {  //always from player perspective
      double swing = 0.0;
      size_t targetDepth = (D > dataHighDepth ? dataHighDepth : D);
      TupleInfo* topTuple = getDepthInfoScaled(targetDepth);
      if (i >= topTuple->pScaledDeltas->size()) { 
         cerr << "Swing of non-considered move requested, returning NAVAL..." << endl;
         return IRutil::NAVAL; 
      } //else
      double topDelta = topTuple->pScaledDeltas->at(i);
      size_t userSwingLow = theScale->getSwingLow();
      size_t userSwingHigh = theScale->getSwingHigh();
      if (userSwingHigh < userSwingLow) {
         userSwingHigh = dataHighDepth;
         userSwingLow = dataLowDepth;
      }
      size_t swadd = (userSwingLow <= targetDepth && targetDepth <= userSwingHigh) ? 0 : 1;
      size_t numSWs = userSwingHigh - userSwingLow + swadd;
      for (size_t d = userSwingLow; d <= userSwingHigh; d++) { 
         TupleInfo* dTuple = getDepthInfoScaled(d);
         swing += (dTuple->pScaledDeltas->at(i) - topDelta);
      }

//debug
if (getRankAtDepth(i, D) == 0 && swing < 0) {
   cerr << "Swing bug " << swing << " in turn:" << endl << string(*this) << endl; 
}
      return (normalize ? swing/double(numSWs) : swing);
   }


   double getDecMoveSwing(size_t i) {
      if (i >= decMoveSwings->size()) {
         cerr << "Swing of over-top move requested, returning zero." << endl;
         return 0.0;
      }
      return decMoveSwings->at(i);
   }
      

   double getUnscaledFalloffAtMaxDepth() {
      return unscaledFalloff.at(unscaledFalloff.size()-1);
   }
    
   double getScaledFalloffAtMaxDepth() {
      return scaledFalloff.at(scaledFalloff.size()-1);
   }

   size_t getNumLegalMoves() const { return pParent->numLegalMoves; }
   size_t getNumMovesWithValues() const { return numMovesWithValues; }
    
   size_t getPlayedMoveIndex () const {   //indexed by highest-depth info
      return playedMoveIndex; 
   } 

   /** Call this with i = playedMoveIndex to get rank of played move at any depth.
       Note use of unscaled deltas to ensure comparisons for equality are discrete.
       In figuring ranks of equal-value moves, the assumption is that the engine's
       relative rank of them at depth d is estimated often enough by the rank at
       the data high depth D (which is what defines the given index i).
       At least that should be better than flipping coins...

       Relies on values for the called move i having been filled in at all depths.
       If move i did not have a value at depth d, it will have been given a
       joint last-place value, and the first occurrence of that value will
       become its rank.  Ideally distortion caused by this, such as fetching
       probabilities by those ranks, will be small.
    */
   size_t getRankAtDepth(size_t i, size_t d) const { 
      double numHigher = 0;  // "higher" means >= for j < i, > for j > i
      TupleInfo* tiu = getDepthInfoUnscaled(d);  //not own copy so don't delete
      double di = tiu->pScaledDeltas->at(i);     //should not bomb...
      for (int j = 0; j < i; j++) {
         if (tiu->pScaledDeltas->at(j) <= di) { numHigher++; }
      }
      for (int k = i+1; k < tiu->pScaledDeltas->size(); k++) {  //bound should == numDeltas
         if (tiu->pScaledDeltas->at(k) < di) { numHigher++; }
      }
      return numHigher;
   }

   size_t getIndexFirstMoveAtDepth(size_t depth) const {
      int useDepth = depth - dataLowDepth;
      if (useDepth >= depthDeltasScaled.size()) {
         useDepth = depthDeltasScaled.size()-1;
      } else if (useDepth < 0) {
         useDepth = 0;
      }
      return indexFirstMoveAtDepth.at(useDepth);
   }
   

   TupleInfo* getDepthInfoScaled(const size_t depth) const {
      int useDepth = depth - dataLowDepth;
      if (useDepth >= depthDeltasScaled.size()) {
         useDepth = depthDeltasScaled.size()-1;
      } else if (useDepth < 0) { 
         useDepth = 0; 
      }
      //return depthDeltasScaled.at(depth);  //IMPT: per indexing notes above, this was wrong
      return depthDeltasScaled.at(useDepth);
   }

   TupleInfo* getDepthInfoUnscaled(size_t depth) const {
      int useDepth = depth - dataLowDepth;
      if (useDepth >= depthDeltasUnscaled.size()) {
         useDepth = depthDeltasUnscaled.size()-1;
      } else if (useDepth < 0) { 
         useDepth = 0; 
      }
      return depthDeltasUnscaled.at(useDepth);
   }

   double getDeltaScaled(size_t i, size_t depth) const {
      return getDepthInfoScaled(depth)->pScaledDeltas->at(i);
   }

   double getDeltaUnscaled(size_t i, size_t depth) const {
      return getDepthInfoUnscaled(depth)->pScaledDeltas->at(i);
   }

   /** Currently forgives calls with depth > userHighDepth, returning the data
       above the user window if it exists, so same as above two methods.
       Could alternately return the userHighDepth info---or bomb...
    */
   TupleInfo* getWindowInfoScaled (size_t depth) const {
      int useDepth = depth - userLowDepth;
      if (useDepth >= windowDeltasScaled.size()) {
         useDepth = windowDeltasScaled.size()-1; //which might include higher data
      } else if (useDepth < 0) {
         useDepth = 0;
      }  
      return windowDeltasScaled.at(useDepth);
   }

   TupleInfo* getWindowInfoUnscaled (size_t depth) const {
      int useDepth = depth - userLowDepth;
      if (useDepth >= windowDeltasUnscaled.size()) {
         useDepth = windowDeltasUnscaled.size()-1; //which might include higher data
      } else if (useDepth < 0) {
         useDepth = 0;
      }
      return windowDeltasUnscaled.at(useDepth);
   }

   TupleInfo* getHighestTupleScaled() const {   //irrespective of user window
      return depthDeltasScaled.at(depthDeltasScaled.size()-1); 
   } 

   TupleInfo* getHighestTupleUnscaled() const {
      return depthDeltasUnscaled.at(depthDeltasUnscaled.size()-1);
   }
 
   double getHighestDepthMoveDeltaScaled(int index) const { 
      return getHighestTupleScaled()->getDelta(index); 
   } 

   double getHighestDepthMoveDeltaUnscaled(int index) const {
      return getHighestTupleUnscaled()->getDelta(index);
   }

   TupleInfo* getHighestWindowTupleScaled() const {
      return windowDeltasScaled.at(windowDeltasScaled.size()-1);
   }

   TupleInfo* getHighestWindowTupleUnscaled() const {
      return windowDeltasUnscaled.at(windowDeltasUnscaled.size()-1);
   }

   //double getHighestWindowDepthMoveDelta(int index) const {
      //return getHighestWindowTuple()->getScaledDelta(index);
   //}


   int getNumberOfMovesAnalyzed() const {
      return getHighestTupleScaled()->numDeltas;
   }

   double getSwing(size_t i) const {
      return decMoveSwings->at(i);
   }

   double getBootWeight() const { return bootWeight; }
   void setBootWeight(double wt) { bootWeight = wt; }
   void incBootWeight() { bootWeight += 1.0; }

   /** Create tuple from any deltas vector, not necessarily conforming size
       REQ: deltas are all non-negative, Black to move already cared for.
       REQ: deltas already in 2-place decimal units.
    */
   TupleInfo* makeTuple(const vector<double>* deltas, TurnContext* tc,
                        bool scaleIt, SwingInfo* swingPolicy, bool perceived) const {
      int lastEqPlace = 0;
      double pushAdd = 0.0;
      bool pushing = false;
      vector<double>* adjDeltas = new vector<double>(numMovesWithValues);

      scaleIt &= scalesDeltas;

      for (int k = 0; k < numMovesWithValues; k++) {
         double cappedDelta = min(deltas->at(k), deltaCap);
         if (cappedDelta != deltas->at(lastEqPlace)) {
            lastEqPlace = k;
         }
   
         if (scaleIt)  {
            switch(theScale->getPatchPolicy())  {
             case UP_TIED_TOP:
               if (k > 0 && cappedDelta == 0.0) {
                  cappedDelta += theScale->getEqualTopFix();
                  //cappedDelta += theScale->getEqualTopFix() + IRutil::LINESCALEEPSILON*(i-1);;
               }
               break;
             case PUSH_TIED_TOP:
               pushing = true;
               if (k > 0 && cappedDelta == 0.0) {
                  pushAdd += theScale->getEqualTopFix(); //progresses the fix
               }
               break;
             case PUSH_ALL_TIES:
               pushing = true;
               if (k > lastEqPlace) {
                  pushAdd += theScale->getEqualTopFix(); //progresses the fix
               }
               break;
             default:
               break;
            }
            if (pushing) { 
               cappedDelta += pushAdd; 
            }
   
            adjDeltas->at(k) = theScale->mapDelta(cappedDelta,tc);
   
         } else  {
            adjDeltas->at(k) = cappedDelta;
         }
      }
   
      //vector<double>* swings = (swingPolicy.find("none") == string::npos ? decMoveSwings : NULL);
      //vector<double>* swings = ((swingPolicy->noSwing || (!swingPolicy->depthDependentSwings)) ? decMoveSwings : NULL);
      //may be re-set by caller
      
      double evalPTM = (tc->whiteToMove ? tc->currEval : -tc->currEval);
      TupleInfo* pTupleInfo = new TupleInfo(pParent, adjDeltas, decMoveSwings, swingPolicy, 
                                            //theScale->getApplicationPolicy(),
                                            //theScale->scalesThere(), false,
	                                    evalPTM, playedMoveIndex, useAltPlayedMoveIndex,
                                            patchesProbs, theScale->getPatchPower());

      if (perceived) {
         delete(tc);
         delete(deltas);
      }
      return pTupleInfo;
   }
   
 
   /** Read evals from TurnInfo* pParent which are in centipawns as integers
       indexed first by legal move and then by depth.  Copy into swap as
       ints, with policy that lowest-depth analysis marked "n.a."
       by the special value NAVAL repeats the lowest-depth recorded value for
       the move; values marked PRUN have value (eval - multipv_cp), and values
       marked NREC share the lowest value of any move at that depth.
       Then values are scaled while being read from swap and formed into the
       final TupleInfo objects.  Use of common swap-space avoids copying.
    */
   void processTurnInfo(vector<vector<int> >* evalSwap) { 
      double deltaFallOff; 
 
      if (playedMoveIndex == -1) { 
         cerr << "error: move played not found in legal moves!: " 
              << pParent->movePlayed << ":" << endl; 
         for (vector<Move>::const_iterator it = pParent->legalMoves->begin(); 
              it != pParent->legalMoves->end(); ++it) { cerr << string(*it) << ", "; } 
         cerr << endl; 
      } 
      //TurnInfo has  vector <vector<int> > depthEvalsCP; 
      //inner vector has size (hi-lo+1) 

      //Evals are read as integers in centipawn units, scaled as doubles
      //in "pawn" units, dividing by double(100).

      int numDataDepths = dataHighDepth - dataLowDepth + 1;

      int btm = pParent->whiteToMove? 1 : -1;

      //Only depths with real data are handled

      vector<int> bestEvalsCP(numDataDepths, btm*(-IRutil::MATEVALCP));
      vector<int> worstEvalsCP(numDataDepths, btm*IRutil::MATEVALCP);
      //vector<Move> bestMoveVec(numDataDepths, Move());
      vector<size_t> indexFirstMoveVec(numDataDepths, 0);

      const vector<vector<int> >* rawEvalsCP = pParent->depthEvalsCP;
      //This uses absolute indexing, so use dataLowDepth as offset
      //evalSwap uses relative indexing, so no offset needed

      
      int printOffset = dataLowDepth;
      int offset = printOffset - IRutil::AIFoffset;

      int maxDepth = numDataDepths - 1;

      int cpeval = rawEvalsCP->at(0).at(offset + maxDepth);

      bestEvalsCP.at(maxDepth) = cpeval;
      int dp; //depth probe
 
      for (int i = 0; i < numMovesWithValues; i++) { 
         dp = maxDepth;

         //first compute the highest-depth valid value for this move.
         while (dp >= 0 && rawEvalsCP->at(i).at(dp+offset) == IRutil::NAVAL) {
            dp--; 
         }
         if (dp < 0) {
            cerr << "Move with no analysis read as NA." << endl;
            continue;  //fill with cpeval?
         }
         //else

         int lastVal = rawEvalsCP->at(i).at(dp+offset); //last valid value

         for (int d = maxDepth; d >= 0; d--) {
            cpeval = rawEvalsCP->at(i).at(d+offset);
            if (cpeval == IRutil::NAVAL) {
               cpeval = lastVal;
            } else if (cpeval == IRutil::NRECVAL) {
               cpeval = worstEvalsCP.at(d);
               //May be incorrect when different moves are NREC (e.g. beyond
               //Houdini's 32 at different depths, but judge extra error here
               //minimal compared to the fuzz of the 32-PV limit to begin with
               //Update: With 64-PV now we can ignore this.
               lastVal = cpeval;

            } else if (cpeval == IRutil::PRUNVAL) {
               cpeval = bestEvalsCP.at(d) - btm*int(100*IRutil::PRUNCAP);
               lastVal = cpeval;
            } else { //a real eval
               lastVal = cpeval;
            }

            evalSwap->at(d).at(i) = cpeval;

            if (btm*cpeval > btm*bestEvalsCP.at(d)) {
               bestEvalsCP.at(d) = cpeval;
if (i >= pParent->consideredMoves->size()) { 
cerr << "Trouble in " + string(*pParent);
} else {
               //bestMoveVec.at(d) = pParent->consideredMoves->at(i);
               indexFirstMoveVec.at(d) = i;  //will stay first occurrence
}
            }
            if (btm*cpeval < btm*worstEvalsCP.at(d)) {
               worstEvalsCP.at(d) = cpeval;
            }
         }
      }


      //At this point, no NA, PRUN, or NREC values are left.
      //Following is main place in code where deltas are scaled by "mapDelta"

      double myeval, beval, delta;   //in pawn 0.00 units

      TupleInfo* ptis;  //last value is top-depth deltas
      TupleInfo* ptiu;
      for(int dd = 0; dd < numDataDepths; ++dd) { 
         vector <double>* pDeltas = new vector<double>(); 
         double deltaFalloff; 
 
         for(int j = 0; j < numMovesWithValues; j++) { 
            myeval = (evalSwap->at(dd).at(j))/double(100);

            beval = (bestEvalsCP.at(dd))/double(100);
            delta = btm*(beval - myeval);
            //Should be nonnegative

if (delta < 0.0) {
   cerr << "NegD for j = " << j << ": " << pParent->movePlayed << ": " << delta << " at depth "
        << (dd+printOffset) << ", eval " << myeval << ", bestEval " << beval << endl;
}
 
            pDeltas->push_back(delta); 
         } 

         TurnContext* tc = new TurnContext(pParent->movePlayed,
                                           pParent->engineID,
                                           pParent->whiteToMove,
                                           beval,
                                           (pParent->prevEvalCP)/double(100),
                                           (pParent->nextEvalCP)/double(100));
         //Used by mapDelta.  Note that we use the depth-d current eval, but
         //the highest-depth eval from the previous and/or next turns (if any).
         //(!!! Maybe change that...?)

         bestEvals.push_back(bestEvalsCP.at(dd)/double(100));
         worstEvals.push_back(worstEvalsCP.at(dd)/double(100));
         //bestMoveByDepth.push_back(bestMoveVec.at(dd));
         indexFirstMoveAtDepth.push_back(indexFirstMoveVec.at(dd));

         ptiu = makeTuple(pDeltas,tc,false,theScale->getSwingPolicy(),false); //does not delete pDeltas yet
         ptis = makeTuple(pDeltas,tc,true,theScale->getSwingPolicy(),false);
         scaledFalloff.push_back(ptis->pScaledDeltas->at(playedMoveIndex));
         unscaledFalloff.push_back(ptiu->pScaledDeltas->at(playedMoveIndex));
         depthDeltasScaled.push_back(ptis); 
         depthDeltasUnscaled.push_back(ptiu);

         delete(pDeltas);
         delete(tc);
      } //end for int dd.  **At this point, non-window method calls work.**
        //Now do loop again to compute swing w.r. to theScale->judgmentDepth

      size_t jd = (theScale->getJudgmentDepth() == 0 ? dataHighDepth
                   : max(min(theScale->getJudgmentDepth(), dataHighDepth), dataLowDepth));
      TupleInfo* tijd = getDepthInfoScaled(jd);

      size_t userSwingLow = theScale->getSwingLow();
      size_t userSwingHigh = theScale->getSwingHigh();
      if (userSwingHigh < userSwingLow) {    //swing disabled in scale
         userSwingLow = dataLowDepth;
         userSwingHigh = dataHighDepth;
      }
      //int swingOffset = (theScale->getZeroDepth() <= dataLowDepth ? 0
      //: theScale->getZeroDepth() - dataLowDepth);
      //size_t swingRelLow = (userSwingLow < dataLowDepth ? 0 : userSwingLow - dataLowDepth);
      //size_t swingRelHigh = (userSwingHigh > dataHighDepth ? dataHighDepth - dataLowDepth
      //: userSwingHigh - dataLowDepth);
    
      //int numSwingDepths = numDataDepths - swingOffset;
      //int numSwingDepths = swingRelHigh - swingRelLow + 1;

      size_t swadd = (userSwingLow <= jd && jd <= userSwingHigh) ? 0 : 1;
      size_t numSWs = userSwingHigh - userSwingLow + swadd;  //fill in others from endpoints
      //int numWindowsSwingDepths = (userHighDepth >= swingOffset ? userHighDepth - swingOffset + 1 : 0);

      
/*
      for (size_t ddd = userSwingLow; ddd <= userSwingHigh; ddd++) {
         TupleInfo* tid = getDepthInfoScaled(ddd);
         for (int k = 0; k < numMovesWithValues; k++) {
            decMoveSwings->at(k) += tid->pScaledDeltas->at(k)/double(numSWs);
         }
      }

      for (int k = 0; k < numMovesWithValues; k++) {
         decMoveSwings->at(k) -= tijd->pScaledDeltas->at(k); //normalizes
      }
*/

      SwingInfo* swp = theScale->getSwingPolicy();

      for (size_t k = 0; k < numMovesWithValues; k++) {
         double kjdel = tijd->pScaledDeltas->at(k);  //di->getDeltaScaled(k,jd);
         //judgment depth is included in min/max comparison, so can use to initialize
         double kMinDelta = kjdel;
         double kMaxDelta = kjdel;
         double kMeanDelta = 0.0;
         double djdel;  //outside loop so as to dampen swing by highest-depth delta
         for (size_t ddd = userSwingLow; ddd <= userSwingHigh; ddd++) {
            //tcusw->currEval = di->getBestEvalP(ddd);
            djdel = getDeltaScaled(k, ddd);
            kMeanDelta += djdel;
            if (ddd >= 5) {
               if (djdel < kMinDelta) { kMinDelta = djdel; }
               if (djdel > kMaxDelta) { kMaxDelta = djdel; }
            }
            decMoveSwings->at(k) += (djdel - kjdel)/numSWs; //normalizes
         }
         kMeanDelta /= double(userSwingHigh - userSwingLow + 1);
         //if (kMaxDelta > IRutil::DELTACAP) { 
            //kMaxDelta = IRutil::DELTACAP; 
         //}
         //if (kMinDelta > IRutil::DELTACAP) { kMinDelta = IRutil::DELTACAP; }
         //double mul = (decMoveSwings->at(k) > 0.0 ? 1.0 : 1.15);
         const double mul = 1.0;
         const double scaler = 1.0 + theScale->getEqualTopFix();
         double factor = 1.0;
         if (kMinDelta > IRutil::MINPROB && kMeanDelta > IRutil::MINPROB) {
            //size_t boxStart = swp.find("boxed");
            if (swp->boxed) {
               //char c = swp.at(boxStart+5);
               unsigned int bxk = (unsigned int)swp->boxKind;
               switch(bxk) {
                case 0:
                  factor = scaler;
                  break;
                case 1: 
                  factor = (kMaxDelta - kMinDelta)/kMaxDelta;
                  break;
                case 2:
                  factor = (kMeanDelta - kMinDelta)/kMeanDelta;
                  break;
                case 3: 
                  factor = 1.0 - kMinDelta/(kMaxDelta + kMinDelta);
                  break;  //note: with 1 in place of kMaxDelta this is same as "damped"
                case 4:
                  factor = 1.0 - kMinDelta/(kMeanDelta + kMinDelta);
                  break;
                case 5:
                  factor = kMaxDelta/(2.0*kMeanDelta);
                  break;
                case 6:
                  //factor = kMaxDelta*kjdel/((kjdel + kMinDelta)*kMeanDelta);
                  factor = 1.0 - kMinDelta*kMinDelta/((scaler + kMinDelta)*kjdel);
                  break;
                case 7:
                  factor = (kMaxDelta - kMinDelta)/(kMaxDelta + kMinDelta);
                  break;
                case 8:
                  factor = IRutil::sqr((kMaxDelta - kMinDelta)/kMaxDelta);
                  break;
                case 9:
                  factor = sqrt((kMaxDelta - kMinDelta)/kMaxDelta);
                  break;
                default:
                  factor = (kMaxDelta - kMinDelta)/kMaxDelta;
               }
               //boxStart += 6;
               //if (boxStart < swp.size() && swp.at(boxStart) == 'a') {
                  //factor *= log(1 + (kjdel/kMinDelta));
               //} else if (boxStart < swp.size() && swp.at(boxStart) == 'b') {
                  //factor *= kjdel/kMeanDelta;
               //}
            } else {
               double kHiDelta = (jd < userSwingHigh ? djdel : kjdel);
               const double damp1 = (swp->prop ? 1.0 : 0.0);
               const double damp2 = (swp->damped ? 1.0 : 0.0);
               factor = scaler/(scaler + damp1*kHiDelta + damp2*kMinDelta);
            }
            decMoveSwings->at(k) *= mul*factor;
         }
      }

   } 

   void resetWindow() {   //REQ: processTurnInfo called first

      windowDeltasScaled.clear();
      windowDeltasUnscaled.clear();
      windowBests.clear();
      windowWorsts.clear();
      for (size_t j = userLowDepth; j <= userHighDepth; j++) {
         int k = min(max(j,dataLowDepth), dataHighDepth) - dataLowDepth;
         windowDeltasScaled.push_back(depthDeltasScaled.at(k)); //pushes existing TupleInfo*, not new
         windowDeltasUnscaled.push_back(depthDeltasUnscaled.at(k));
         windowBests.push_back(bestEvals.at(k));
         windowWorsts.push_back(worstEvals.at(k));
      }
   }
   //ENS: window vectors have same indices as weights held by Trial.
         
 
   operator string() const { 
      string out = string(*pParent) + "\n";
      out += string("Cap: deltaCap ") + IRutil::ftoa(deltaCap) + ", falloff: " 
          + IRutil::ftoa(turnFalloffCapped) + ", played move index: " 
          + IRutil::itoa(playedMoveIndex) + "\n";

      for (size_t ii = 0; ii < depthDeltasScaled.size(); ii++) {
         out += IRutil::itoa(ii+dataLowDepth,2) + ": " 
             + string(*(depthDeltasScaled.at(ii))) + "\n";
      }

      return out; 
   }  


 
}; 
 
#endif    //end of #ifndef __IR_DECISION_INFO_H__ 
 


 
