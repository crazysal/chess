//File-----------------------Trial.cpp------------------------------------ 
 
#ifndef __IR_TRIAL_CPP__ 
#define __IR_TRIAL_CPP__ 

#ifndef __IR_TRIAL_H__
#include "Trial.h"
#endif
 
//---------------IMPLEMENTATION OF CLASS TRIAL-------------------------- 
 
/** Return sum_t log(1/p_t), where p_t is the probability of the played 
    move at turn t---as estimated from *this* spec on *these* tuples. 
    Weights are taken from this spec, while distortion fixing and 
    lumping all moves outside the top N as "move N+1" are /recommended/. 
    Making this a zero-parameter method enables its direct use by the 
    "Minimizer" class---minimizing the sum equates to maximizing the 
    probability of the sequence of played moves, assuming independence. 
 */ 
double Trial::logSumPlayedMoves() const { 
   //if (!isReady()) { reset(); } 
   double logSumPlayed = 0.0; 
   double accumWeight = 0.0; 
   //vector<double> probs; 
   for (vector<DecisionInfo*>::const_iterator it = theDecisions->begin(); 
        it != theDecisions->end(); it++) { 
      DecisionInfo* dec = *it; 
      const double bw = dec->getBootWeight();

      if (bw < IRutil::MINPROB) { continue; } //Else---!!

      //probs = Models::getProbs(dec,theSpec,pwv,swv,model);
      Models::MoveProjection* mp = Models::getProjection(dec,theSpec,pwv,swv,model);
      //double weight = (unitWeights ? 1.0 : Models::getWeight(*it,theSpec,probs));
      //double bweight = bw*weight;
      //double bweight = bw*Models::getDecWeight(*it,theSpec,probs);
      double bweight = bw*mp->weight;

      accumWeight += bweight; 
      int pmi = dec->getPlayedMoveIndex();
      mp->indexPlayedMove = pmi;

      size_t jd = theScale->getJudgmentDepth();
      size_t useDepth = ((jd == 0 || jd >= dec->dataHighDepth) ? dec->dataHighDepth
                             : (jd <= dec->dataLowDepth ? dec->dataLowDepth : jd));
      size_t playedRank = (useDepth >= dec->dataHighDepth ? pmi 
                            : dec->getRankAtDepth(pmi,useDepth));

      //size_t useDepth = (theScale->getJudgmentDepth() == 0 ? dec->dataHighDepth : theScale->getJudgmentDepth());
      //int playedRank = (theScale->getJudgmentDepth() == 0 ? pmi
                           //: dec->getRankAtDepth(pmi,useDepth));
      //size_t indexFirstMove = (theScale->getJudgmentDepth() == 0 ? 0
                           //: dec->getIndexFirstMoveAtDepth(useDepth));

      //double pj = probs.at(dec->getPlayedMoveIndex()); 
      double pj = mp->probs->at(playedRank);
      if (pj < IRutil::MINPROB) { pj = IRutil::MINPROB; }
      logSumPlayed += bweight * IRutil::log2(1/pj); 
      delete(mp);
   } 
   double r = IRutil::divifgt0(logSumPlayed,accumWeight); 
 
   return r; 
} 
 
 
string Trial::speakFitNew(const double score,const double result2, const double totalWeight, 
                   const int tupleCount) const 
{ 
   ostringstream o; 
 
   o << "Overall fit score: " << IRutil::ftoa(score,3) << endl; 
   o << "Without Normalizing: " << IRutil::ftoa(result2,3) << endl; 
   o << "Number of filtered turns: " << tupleCount << endl; 
   o << "Average weight per turn : " 
          << IRutil::ftoa(totalWeight/tupleCount, 3) << endl; 
   o << theSpec.showCurvePoints(0.05,1.00); 
 
   return o.str(); 
} 
string Trial::speakFit(const double score, const vector<double>& up, 
                       const vector<double>& scores, const double totalWeight, 
                       const int tupleCount) const { 
 
   ostringstream o; 
 
/*
   o << "Differences (percentilePoint - result), x 100:" << endl; 
   for (int i = 0; i < numScores; i++) { 
      const double a = percentilePoints->at(i); 
      o << a << ": " << IRutil::ftoa(100.0*(a - up[i]/totalWeight), 3) 
             << endl; 
   } 
*/
   o << "Overall fit score: " << IRutil::ftoa(score,3) << endl; 
   o << "Number of filtered turns: " << tupleCount << endl; 
   o << "Average weight per turn : " 
          << IRutil::ftoa(totalWeight/tupleCount, 3) << endl; 
   o << "Curve points:" << endl; 
   o << theSpec.showCurvePoints(0.05,1.00); 
 
   return o.str(); 
} 
 
/** New Percentile-Fit Model using piecewise integral 
    Code designed by Tamal Biswas, edited by KWR 
*/ 
double Trial::percFit(vector<ostream*>* outs) { 
 
   if (!isReady()) { reset(); } 
   double totalWeight = 0.00;   
   int tupleCount = 0; 
 
   vector<IRutil::Endpt> endpts; 
 
   for(vector<DecisionInfo*>::const_iterator tu = theDecisions->begin(); 
      tu != theDecisions->end(); tu++) { 
         IRutil::Endpt u = Models::getProbEndpt(*tu,theSpec,pwv,swv,model); 
         // Generate second endpt to use as r_i ( end range for current move ) 
         IRutil::Endpt v(u.partner, u.val, u.weight); 
         endpts.push_back(u); 
         endpts.push_back(v); 
         tupleCount++; 
         totalWeight += u.weight; 
   } 
   //Sorting all the tuples 
   std::sort(endpts.begin(), endpts.end(), IRutil::endPtLT); 
   endpts.push_back(IRutil::Endpt(1.0,1.0,0.0));  //target for last piece 
   int T = tupleCount; 
   //Running Sum 
   double a = -totalWeight; 
   double b = 0.0; 
   double il2dist = 0.0;   //accumulated piecewise integral value 
   IRutil::Endpt nextPt = endpts.at(0); 
   if (nextPt.val > 0.0) { 
      il2dist += IRutil::cube(nextPt.val)/3.0; 
   } 
   int numPieces = endpts.size() - 1; 
   for(int i = 0; i < numPieces; i++) { 
      IRutil::Endpt currPt(nextPt.val,nextPt.partner,nextPt.weight); 
      nextPt = endpts.at(i+1); 
      double val = currPt.val; 
      double part = currPt.partner; 
      if (currPt.isP()) { 
         //Updating the Running Sum 
         a += currPt.weight/(part - val);		 
         b -= val*currPt.weight/(part - val); 
      } else { 
         //Here val=r_i and part=p_i 
         a -= currPt.weight/(val - part); 
         // Incrementing b. b stores info about full-hit+ extra. 
         b += currPt.weight + part*currPt.weight/(val - part); 
      } 
      double nextVal = nextPt.val;  //can be 1.0 at the end 
      if(val == nextPt.val) { 
         //skip: We have updated the running sum. That's all we need. 
      } else { 
         /*------------ 
         il2dist += IRutil::sqr(A)*(IRutil::cube(nextVal) - IRutil::cube(val))/3 
                       + A*B*(IRutil::sqr(nextVal) - IRutil::sqr(val)) 
                       + IRutil::sqr(B)*(nextVal - val); 
         ------------*/ 
         //il2dist += IRutil::sqr(a)*(IRutil::cube(nextVal) - IRutil::cube(val))/3 
         il2dist += (nextVal - val)* 
            (    a*(nextVal + val)*(a*nextVal/3.0 + b) 
               + IRutil::sqr(a*val)/3.0 
               + IRutil::sqr(b) 
            ); 
      } 
   } 
   //double result = il2dist/IRutil::sqr(T); 
   //double result = il2dist/T; 
   double result = IRutil::divifgt0(il2dist,totalWeight); 
 
   if (outs && (!outs->empty())) { 
      string st = speakFitNew(result, il2dist, totalWeight, tupleCount); 
      for (vector<ostream*>::iterator it = outs->begin(); 
         it != outs->end(); it++) { 
            (**it) << st; 
      } 
   } 
   return result; 
} 
 
 
double Trial::probabilityBins() { 
   vector<double> allProbs; 
   vector<double> hitProbs; 
   if (!isReady()) { reset(); } 
   double totalWeight = 0.00;  //class field incremented by percOne() 
   int decCount = 0; 
   int probsCount = 0; 
   double score = 0; 
 
   //First read all the derived probabilities 
   for(vector<DecisionInfo*>::const_iterator tu = theDecisions->begin(); 
       tu != theDecisions->end(); tu++) { 
 
      DecisionInfo* dec = *tu; 
      decCount++; 
 
      int numDeltas = dec->getNumberOfMovesAnalyzed(); 
      Models::MoveProjection* mp = Models::getProjection(dec,theSpec,pwv,swv,model);
      //vector<double> probs = Models::getProbs(dec,theSpec,pwv,swv,model); 
      const int numProbs = mp->probs->size(); 
 
      probsCount += numProbs; 
 
      //double weight = Models::getDecWeight(dec,theSpec,probs); 
      //totalWeight += weight; 
      totalWeight += mp->weight;
      int playedMoveIndex = dec->playedMoveIndex; 
      int iplayedMoveIndex = dec->getPlayedMoveIndex(); 
       
      if (playedMoveIndex == IRutil::playedMoveNotInTopN) { 
         if (numDeltas < IRutil::NCUTOFF) { 
            cout << "Data glitch: short tuple with no played move." << endl; 
            decCount--;  //it will already have been incremented 
            continue;  //skips tuple 
         } else { 
            playedMoveIndex = iplayedMoveIndex = numDeltas;//== index last share 
         } 
      } 
      allProbs.insert(allProbs.end(),mp->probs->begin(),mp->probs->end()); 
      hitProbs.push_back(mp->probs->at(iplayedMoveIndex)); 
   } 
 
   //Amazingly, we don't need to associate hits to tuples anymore. 
 
   sort(allProbs.begin(),allProbs.end()); 
   sort(hitProbs.begin(),hitProbs.end()); 
   allProbs.push_back(2.0);  //sentinel 
   hitProbs.push_back(2.0); 
 
   //vector<double> pp = *percentilePoints;  //always low-to-high 
   //Thus we treat blunders first, since sort begins with low values. 
   vector<double> pp;
   for (int ell = 1; ell < 101; ell++) {
      pp.push_back(ell/100.0);
   }
 
   //if (pp.at(pp.size()-1) < 1.0) { pp.push_back(1.0); } 
   size_t binNo = (pp.at(0) > 0.0) ? 0 : 1; 
   double hiProb = 0.0; 
   size_t api = 0; 
   size_t hpi = 0; 
   while (binNo < pp.size()) { 
      double loProb = hiProb; 
      hiProb = pp.at(binNo); 
      //LOOP INV: loProb < allProbs[api] <= hiProb, ditto hitProbs[hpi] 
      size_t abc = 0;  //allProbs bin count 
      size_t hbc = 0; 
      double probSum = 0.0; 
      while (allProbs.at(api) <= hiProb) { 
         abc++; 
         probSum += allProbs.at(api); 
         api++; 
      } 
      double binAvgProb = (abc > 0) ? probSum/abc : 0; 
      while (hitProbs.at(hpi) <= hiProb) { 
         hbc++; 
         hpi++; 
      } 
       
      //note: score is weighted by the bin size. 
      //score += (abc > 0) ? abc*(*dist)(binAvgProb, hbc/double(abc)) : 0.0; 
      score += (abc > 0) ? (*dist)(binAvgProb, hbc/double(abc)) : 0.0; 
      binNo++; 
   } 
 
   return 10000*score/probsCount; 
} 
   
 
    
/** Test expected number of first-line matches and top-matches for the 
    sequence of moves "tuples", based on the curve specification in "spec". 
    If none given, use those associated with the present trial. 
    Uses spec's weighting only if third argument is "false". 
    Assumes tuples have already been filtered. 
    Reports z-scores for various statistical tests on both:
    () the current MLM data used to compute shares, which could be a "panel composite"
    () the moves selected by theSelectors->at(j), which may come from other data.  This data
       need not be composite and can come from other engines.  
    The "silent" argument might be turned into a timesaver...
    The "verbose" argument prints to a MovesOutput file.

    An important procedural understanding is that the projected probabilities
    are not based on the depth used for judgment but on the turn data (as modified
    in DecisionInfo, scale settings, and other parts of the TrialSpec.
    Those probabilities are then applied to the moves and their ranks and values
    at the judgment depth in order to determine the projected quantities.
    The actual quantities on which the projections are judged are also taken
    from the judgment depth.  Thus it is possible to use the values at (say)
    depths 5..15 to predict results at depth 20.  Vice-versa is enabled,
    but its meaning is unclear.
 */ 
PerfData Trial::perfTest(bool unitWeights, bool tmaNN, bool showUnitWeights,
                         bool showUnscaled, size_t numIndices, size_t equalTopIndexLimit,
                         bool silent, string dump, size_t movesLimit) const {

   PerfData pf(theScale->getDeltaCap(), equalTopIndexLimit, theDecisions->size());  //initializes pf.numTurns 
   //if (!isReady()) { reset(); } 
   if (theDecisions->size() == 0) { return pf; } 
   //pf.numTurns = theDecisions->size(); 
   pf.accumWeight = 0.0; 
 
   pf.weighted = (!unitWeights)  
                       && theSpec.weightMethodChoice != IRfun::UNITWTS; 
   string adj = pf.weighted ? "Wtd." : "Unwtd."; 

   size_t numInd = 1 + IRutil::NCUTOFF;     //will use numFilledIndices for each turn later...
   vector<double> indexCumeProj(numInd,0.0);
   vector<double> indexCumeActual(numInd,0.0);

   for (int i = 1; i <= numInd; i++) {      //move indices print as 1-based
      MoveIndexTest mtest(i, 7, 2, 0.0, 0.0, 0.0, 
                          true, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1, 1.0, false, false);
      pf.moveIndexTests.push_back(mtest);
      pf.moveIndexTestsWtd.push_back(mtest);
   }

   size_t numSelectors = theSelectors->size();
   for (size_t m = 0; m < numSelectors; m++) {
      SelectionTest stest(theSelectors->at(m)->getName(), 7, 2, 0.0, 0.0, 0.0,
                          true, 0.0, 0.0, 0.0, 1, 1.0, false, false);
      pf.selectionTests.push_back(stest);
      pf.selectionTestsWtd.push_back(stest);  //a true copy
   }


   //vector<double> probs; 
 
   //pf.turnsData->clear();   //not needed?  !!!

   string dumpFile = (dump == "" ? "movesDump.txt" : dump);
   ofstream MOVESDUMP(dumpFile.c_str(), ios::app);

   for (vector<DecisionInfo*>::const_iterator it = theDecisions->begin(); 
        it != theDecisions->end(); it++) { 

      DecisionInfo* dec = *it; 
      const double bw = dec->getBootWeight();

      if (bw < IRutil::MINPROB) { continue; } //Else---!!

      //probs = Models::getProbs(dec,theSpec,pwv,swv,model); //this should not depend on judgment depth??
      Models::MoveProjection* mp = Models::getProjection(dec,theSpec,pwv,swv,model,equalTopIndexLimit);

      //double weight = (pf.weighted ? Models::getDecWeight(*it,theSpec,probs) : 1.0);
      //double bweight = bw*weight;
      double bweight = bw*mp->weight;
 
      size_t numDeltas = dec->getNumberOfMovesAnalyzed(); 
      //size_t pmi = dec->getPlayedMoveIndex();   //absolute, perhaps build into mp?  !!!
      size_t pmi = mp->indexPlayedMove;

      size_t jd = theScale->getJudgmentDepth();
      size_t useDepth = ((jd == 0 || jd >= dec->dataHighDepth) ? dec->dataHighDepth
                             : (jd <= dec->dataLowDepth ? dec->dataLowDepth : jd));
      size_t playedRank = (useDepth >= dec->dataHighDepth ? pmi 
                            : dec->getRankAtDepth(pmi,useDepth));
      if (playedRank != mp->rankPlayedMove) {
         cerr << "Rank Mismatch!" << endl;
      }

      size_t indexFirstMove = (useDepth >= dec->dataHighDepth ? 0 : dec->getIndexFirstMoveAtDepth(useDepth));
      //size_t useDepth = theScale->getJudgmentDepth() == 0 ? dec->dataHighDepth : theScale->getJudgmentDepth();
      //int playedRank = (theScale->getJudgmentDepth() == 0 ? pmi
                        //: dec->getRankAtDepth(pmi, useDepth));  
      //size_t indexFirstMove = (theScale->getJudgmentDepth() == 0 ? 0
                               //: dec->getIndexFirstMoveAtDepth(useDepth));

/*
      mp.firstMoveProb = mp->probs.at(indexFirstMove);
      mp.playedMoveProb = probs.at(pmi);   //not probs.at(playedRank), keyed to move not index
      mp.playedMoveIndex = playedRank; 
      mp.playedMoveDelta = (useDepth >= dec->dataHighDepth ?   //theScale->getJudgmentDepth() == 0 ?
		               dec->getHighestDepthMoveDeltaScaled(pmi) 
		               //: dec->getDeltaScaled(playedRank, theScale->getJudgmentDepth()));  // *no*
		               : dec->getDeltaScaled(pmi,useDepth)); //deltas use "i"-order
      int i = 1;
      mp.topMoveProb = 0.0;
      if (useDepth >= dec->dataHighDepth) {   //theScale->getJudgmentDepth() == 0) {
         mp.topMoveProb = mp.firstMoveProb;
         while (i < numDeltas  
             && dec->getEvalP(i,dec->dataHighDepth) == dec->getEvalP(0,dec->dataHighDepth)) {
            mp.topMoveProb += probs.at(i); 
            i++; 
         } 
      } else {
         for (i = 0; i < numDeltas; i++) {
            if (dec->getEvalP(i, useDepth) == dec->getBestEvalP(useDepth)) {
               mp.topMoveProb += probs.at(i);
            }
        }
      }
*/
      //pf.turnsData->push_back(mp);   //store pointer, will delete-all later
      //pf.numTurns++;
      pf.totalBootItems += bw;
      pf.accumWeight += bweight;

      const double p0 = mp->probFirstMove;
      const double topProb = mp->probEqualTopMove;
      double pj = (pmi != IRutil::playedMoveNotInTopN   //pmi, NOT playedRank
                   || (fixDistortion && treatMissAsMoveNN)) ? mp->probs->at(pmi) : 0.0;
      if (pj < IRutil::MINPROB) { pj = IRutil::MINPROB; }
      const double psf = mp->projFalloffScaled;  //Models::predScaledFalloff(dec, probs,useDepth);

      const double puf = mp->projFalloffUnscaled;  //Models::predUnscaledFalloff(dec, probs,useDepth);
      const double psfv = mp->projFalloffVarianceScaled;
      const double pufv = mp->projFalloffVarianceUnscaled;
      const double psls = mp->projLogSumScaled;
      const double puls = mp->projLogSumUnscaled;
      const double pses = mp->projEntropySumScaled;
      const double pues = mp->projEntropySumUnscaled;

      const double matchAdd = (playedRank == 0 ? 1.0 : 0.0);
      bool etvMatch = dec->getEvalP(pmi, useDepth) == dec->getBestEvalP(useDepth)
                         && playedRank <= equalTopIndexLimit;
      const double topAdd = (etvMatch ? 1.0 : 0.0);
      const double falloffAdd = mp->playedMoveDeltaScaled;
      const double usfAdd = mp->playedMoveDeltaUnscaled;
                       // = (useDepth >= dec->dataHighDepth ?   //theScale->getJudgmentDepth() == 0 ?
                  //         dec->getHighestDepthMoveDeltaUnscaled(pmi)
                     //      : dec->getDeltaUnscaled(playedRank,useDepth));
      //double topAdd = (dec->getEvalP(pmi,dec->dataHighDepth) == dec->getEvalP(0,dec->dataHighDepth) ? 1.0 : 0.0);

      pf.scaledFalloff.uptick(bw, bw*falloffAdd, bw*psf, bw*psfv,
                              bw*psls, bw*pses, false);
      pf.scaledFalloffWtd.uptick(bweight, bweight*falloffAdd,
                                 bweight*psf, bweight*psfv, bweight*psls, bweight*pses, false);
      pf.unscaledFalloff.uptick(bw, bw*usfAdd, bw*puf, bw*pufv,
                                bw*puls, bw*pues, false);
      pf.unscaledFalloffWtd.uptick(bweight, bweight*usfAdd,
                                   bweight*puf, bweight*pufv, bweight*puls, bweight*pues, false);
      pf.scaledFalloffNextTurnCapped.uptick(bw, bw*dec->turnFalloffCapped, false);
      pf.scaledFalloffNextTurnCappedWtd.uptick(bweight, bweight*dec->turnFalloffCapped, false);

      //pf.unscaledFalloffNextTurnCapped
      //pf.unscaledFalloffNextTurnCappedWtd;

      pf.moveMatch.uptick(bw, bw*matchAdd, bw*p0, bw*p0*(1.0-p0),
                          bw*IRutil::log2(1.0/p0), bw*p0*IRutil::log2(1.0/p0), false);
      pf.moveMatchWtd.uptick(bweight, bweight*matchAdd, bweight*p0, bweight*p0*(1.0-p0),
                             bweight*IRutil::log2(1.0/p0), bweight*p0*IRutil::log2(1.0/p0), false);
      pf.equalValueMatch.uptick(bw, bw*topAdd, bw*topProb, bw*topProb*(1.0-topProb),
                                bw*IRutil::log2(1.0/topProb), bw*topProb*IRutil::log2(1.0/topProb), false);
      pf.equalValueMatchWtd.uptick(bweight, bweight*topAdd, bweight*topProb, bweight*topProb*(1.0-topProb),
                                   bweight*IRutil::log2(1.0/topProb),
                                   bweight*topProb*IRutil::log2(1.0/topProb), false);
      pf.playedMoveMatch.uptick(bw, bw, bw*pj, bw*pj*(1.0-pj),
                                bw*IRutil::log2(1.0/pj), bw*pj*IRutil::log2(1.0/pj), false);
      pf.playedMoveMatchWtd.uptick(bweight, bweight, bweight*pj, bweight*pj*(1.0-pj),
                                   bweight*IRutil::log2(1.0/pj),
                                   bweight*pj*IRutil::log2(1.0/pj), false);

/*
cerr << "pf.moveMatchWtd after adding " << matchAdd << " times " << bw << "*" << weight << endl << string(pf.moveMatchWtd) << endl;
cerr << "pf.scaledFalloffWtd after adding " << falloffAdd << " times " << bweight << endl << string(pf.scaledFalloffWtd) << endl;
cerr << "Predicted falloff add was " << psf << ", sig^2 " << psfv << endl;
*/

      size_t numFilledIndices = (mp->probs->size() >= IRutil::NCUTOFF + 1 ? IRutil::NCUTOFF + 1 
                                                                          : mp->probs->size());

      bool hitFlag = false;
      double hitAdd = 0.0;
      double testProbs = 0.0;
      const TurnInfo* pTurn = dec->pParent;
      double moveSwing, swingUsed, swingDiff;
      Move pm = pTurn->movePlayed;
      Move em = pTurn->engineMove;

      if (dump != "" && theDecisions->size() <= movesLimit && (!silent)) {  
         MOVESDUMP << endl << pTurn->gameID << endl;
         MOVESDUMP << pTurn->getFEN() << "; " << pm.TAN2LAN()
                   << ", engine " << em.TAN2LAN() << endl;
         //MOVESDUMP << pf.moveHeader() << endl << pf.moveLine(mp, true) << endl;
         MOVESDUMP << string(*theScale) << endl;
         MOVESDUMP << setprecision(2) << fixed << "Eval " << (pTurn->currEvalCP/100.0)
                   << " at depth " << useDepth << "; swap index " << (1+mp->proxies->swapIndex)
                   << " and spec " << string(theSpec) << ":" << endl;
         MOVESDUMP << endl;
         MOVESDUMP << " M# Rk   Move  RwDelta  ScDelta  Swing  SwDDep  SwRel   ProxyValue   ProjProb'y" << endl;
      }     //true for first-line match

      //Important to bear in mind again that probs[i] is keyed to the top-depth index i,
      //whereas the moveIndexTests are keys to j at the judgment depth d.
      //Happily j = getRankAtDepth(i, d) is 1-to-1 except on non-analyzed indices, when its
      //projected probability should fall into a "...notInTopN" catchall anyway.
      //
      for (int i = 0; i < numFilledIndices; i++) {

         if (i >= dec->getNumMovesWithValues()) { continue; }

         //size_t j = (theScale->getJudgmentDepth() == 0 ? i : dec->getRankAtDepth(i,useDepth));
         size_t j = (useDepth >= dec->dataHighDepth ? i : dec->getRankAtDepth(i,useDepth));
         double dt = (useDepth >= dec->dataHighDepth ? dec->getHighestDepthMoveDeltaScaled(i)
                                                     : dec->getDeltaScaled(i,useDepth));
         double du = (useDepth >= dec->dataHighDepth ? dec->getHighestDepthMoveDeltaUnscaled(i)
                                                     : dec->getDeltaUnscaled(i,useDepth));
         //double dt = (theScale->getJudgmentDepth() == 0 ? dec->getHighestDepthMoveDeltaScaled(i)
                                                        //: dec->getDeltaScaled(i,useDepth));
         //if (dt > deltaCap) { dt = deltaCap; }  //change from before: not auto-applied to "miss".
         double pri = mp->probs->at(i);
         if (pri < IRutil::MINPROB) { 
            pri = IRutil::MINPROB; 
            //if (i < 4) { cerr << "Yelp! for i:pri = " << i << ":" << pri << endl << string(*dec) << endl; }
         }
         testProbs += pri;
         if (pmi == i) {   //the hit becomes one for test j
            hitAdd = 1.0;
            hitFlag = true;
         }
         //double hitAdd = (pmi == i ? 1.0 : 0.0);
         //Below we use the fixed non-depth-dependent swing values.  They are not tested for, just displayed

         moveSwing = dec->getDecMoveSwing(i);
         swingUsed = mp->swingsUsed->at(i);
         swingDiff = swingUsed - mp->swingsUsed->at(mp->indexFirstMove);

         pf.moveIndexTests.at(j).uptick(bw, bw*hitAdd, bw*pri, bw*pri*(1.0-pri), 
                                        bw*IRutil::log2(1.0/pri), bw*pri*IRutil::log2(1.0/pri), bw*dt, 
                                        bw*moveSwing,
					bw*swingUsed,
					bw*swingDiff,
                                        false);
                                        //bw*(dec->getSwingScaled(i, dec->userHighDepth, true)));
                                        //bw*dt, bw*(dec->getSwing(i, dec->userHighDepth, true)));
         pf.moveIndexTestsWtd.at(j).uptick(bweight, bweight*hitAdd, bweight*pri, bweight*pri*(1.0-pri),
                                           bweight*IRutil::log2(1.0/pri), bweight*pri*IRutil::log2(1.0/pri),
                                           bweight*dt, 
                                           bweight*moveSwing,
                                           bweight*swingUsed,
                                           bweight*swingDiff,
                                           false);
                                           //bweight*(dec->getSwingScaled(i, dec->userHighDepth, true)));
                                           //bweight*dt, bweight*(dec->getSwing(i, dec->userHighDepth, true)));

         if (dump != "" && theDecisions->size() <= movesLimit && (!silent)) {  //true for first-line match
            MOVESDUMP << right << setw(3) << (1+i) << setw(3) << (1+j) 
                      << setw(7) << pTurn->consideredMoves->at(i).TAN2LAN() << ": " 
                      << fixed << setprecision(2) << setw(6) << du
                      << setw(8) << setprecision(3) << dt 
                      << setprecision(3) << setw(8) << moveSwing
                      << setw(8) << swingUsed << setw(8) << swingDiff 
                      << setprecision(8) << setw(13) << mp->proxies->values.at(i)
                      << setw(13) << pri << endl;
         }

         //indexCumeProj.at(j) += bw*pri;
         //indexCumeActual.at(j) += bw*hitAdd;
         indexCumeProj.at(j) += bweight*pri;
         indexCumeActual.at(j) += bweight*hitAdd;
         hitAdd = 0.0;
      }
      if (!hitFlag) { cerr << "Turn not hit: " << string(*dec) << endl; }
      if (fabs(testProbs - 1.0) > IRutil::SUMPROBSTOLERANCE && !silent) {
         cerr << "Bad sum " << testProbs << " in turn " << dec->pParent->turnNumber
              << (dec->pParent->whiteToMove ? "w of " : "b of ") << dec->pParent->pGameInfo->gameID << endl;
         IRutil::PROBLEM_FENS.insert(dec->pParent->getFEN());
         IRutil::PROBLEM_GAMES.insert(dec->pParent->gameID + "\n" + dec->pParent->getFEN());
      }

      for (size_t k = 0; k < pf.selectionTests.size(); k++) {
         double playedMoveAdd = (theSelectors->at(k)->apply(pTurn, pm) ? 1.0 : 0.0);
         double engineMoveAdd = (theSelectors->at(k)->apply(pTurn, em) ? 1.0 : 0.0);
         double selectionProb = 0.0;
         size_t i = 0;
         while (i < numDeltas) {
            Move move = pTurn->consideredMoves->at(i);
            if (theSelectors->at(k)->apply(dec->pParent, move)) {
               selectionProb += mp->probs->at(i);  //no test for near-zero needed here
            }                                      //assumes selectors have any needed depth info already 
            i++;
         }
         if (selectionProb < IRutil::MINPROB) { selectionProb = IRutil::MINPROB; }
         if (selectionProb > 1.0) {
            if (selectionProb > 1.0 + pTurn->numConsideredMoves * IRutil::MINPROB) {
               cerr << "OOB selection prob " << setprecision(10) << selectionProb
                    << " in test " << theSelectors->at(k)->getName()
                    << " at turn " << dec->pParent->getFEN() << endl;
               for (int j = 0; j < numDeltas; j++) {
                  Move move = dec->pParent->consideredMoves->at(j);
                  string sel = (theSelectors->at(k)->apply(dec->pParent, move) ? "*" : " ");
                  cerr << string(move) << sel << " " << mp->probs->at(j) << endl;
               }
            }
            selectionProb = 1.0;
         }


         pf.selectionTests.at(k).uptick(bw, bw*playedMoveAdd, bw*selectionProb, 
                                        bw*selectionProb*(1.0-selectionProb),
                                        bw*IRutil::log2(1.0/selectionProb),
                                        bw*selectionProb*IRutil::log2(1.0/selectionProb),
                                        bw*engineMoveAdd, false);
         pf.selectionTestsWtd.at(k).uptick(bweight, bweight*playedMoveAdd, bweight*selectionProb,
                                           bweight*selectionProb*(1.0-selectionProb),
                                           bweight*IRutil::log2(1.0/selectionProb),
                                           bweight*selectionProb*IRutil::log2(1.0/selectionProb),
                                           bweight*engineMoveAdd, false);
      } 

      if (theDecisions->size() <= 1000) {
         pf.turnsData->push_back(mp);
      } else {
         delete mp;
      }
                                         
   } //end of loop over DecisionInfo. 

   pf.scaledFalloff.update();
   pf.scaledFalloffWtd.update();
   pf.unscaledFalloff.update();
   pf.unscaledFalloffWtd.update();

   pf.scaledFalloffNextTurnCapped.update();
   pf.scaledFalloffNextTurnCappedWtd.update();
   pf.unscaledFalloffNextTurnCapped.update();
   pf.unscaledFalloffNextTurnCappedWtd.update();


   pf.moveMatch.update();   //duplicates moveIndexTests[0]
   pf.equalValueMatch.update();
   pf.playedMoveMatch.update(); //100% on actual but pred is relevant


   pf.moveMatchWtd.update();   //duplicates moveIndexTestsWtd[0]
   pf.equalValueMatchWtd.update();
   pf.playedMoveMatchWtd.update();

   size_t k = 0;
   for (k = 0; k < pf.moveIndexTests.size(); k++) {
      pf.moveIndexTests.at(k).update();
      pf.moveIndexTestsWtd.at(k).update();
   }

   for (k = 0; k < pf.selectionTests.size(); k++) {
      pf.selectionTests.at(k).update();
      pf.selectionTestsWtd.at(k).update();
   }

   //debug
   if (!silent) {
   for (size_t n = 1; n < numInd; n++) {
      indexCumeProj.at(n) += indexCumeProj.at(n-1);
      indexCumeActual.at(n) += indexCumeActual.at(n-1);
   }
   for (size_t m = 0; m < numInd; m += 10) {
      cerr << (m+1) << ": " << indexCumeProj.at(m) << " vs. " << indexCumeActual.at(m) << endl;
   }
   cerr << numInd << ": " << indexCumeProj.at(numInd - 1) << " vs. " << indexCumeActual.at(numInd - 1) << endl;
   }



   //The following use of least-squares may run afoul of heteroskedasticity 
   double indexScore = 0.0; 
   double indexDiff = 0.0;
   double indexMass = 0.0; 
   double indexDiffMass = 0.0;
   double indexScoreInvVar = 0.0; 
   double indexScoreProp = 0.0; 
   double indexScoreWtd = 0.0; 
   double indexDiffWtd = 0.0;
   double indexMassWtd = 0.0; 
   double indexDiffMassWtd = 0.0;
   double indexScoreInvVarWtd = 0.0; 
   double indexScorePropWtd = 0.0; 
   double indexFitPt, indexDiffPt, indexFitPtWtd, indexDiffPtWtd; 
 
   for (int i = 0; i < IRutil::NCUTOFF; i++) { 
      indexFitPt = (*dist)(pf.moveIndexTests.at(i).measuredFreq,
                           pf.moveIndexTests.at(i).projectedFreq);
      indexFitPtWtd = (*dist)(pf.moveIndexTestsWtd.at(i).measuredFreq,
                              pf.moveIndexTestsWtd.at(i).projectedFreq);
      indexDiffPt = pf.moveIndexTests.at(i).measuredFreq - pf.moveIndexTests.at(i).projectedFreq;
      indexDiffPtWtd = pf.moveIndexTestsWtd.at(i).measuredFreq - pf.moveIndexTestsWtd.at(i).projectedFreq;


      indexScore += indexFitPt; 
      indexDiff += indexDiffPt;
      indexMass += indexFitPt*sqrt(pf.moveIndexTests.at(i).measuredFreq);
      indexDiffMass += indexDiffPt*sqrt(pf.moveIndexTests.at(i).measuredFreq * indexDiffPt);
      indexScoreInvVar += IRutil::divifgt0(indexFitPt * pf.totalBootItems,
                                           pf.moveIndexTests.at(i).projectedVariance);
      indexScoreProp += IRutil::divifgt0(indexFitPt * pf.moveIndexTests.at(i).projectedValue,
                                         sqrt(pf.moveIndexTests.at(i).projectedVariance));
      indexScoreWtd += indexFitPtWtd; 
      indexDiffWtd += indexDiffPtWtd;
      indexMassWtd += indexFitPtWtd*sqrt(pf.moveIndexTestsWtd.at(i).measuredFreq);
      indexDiffMassWtd += indexDiffPtWtd;
      indexScoreInvVarWtd += IRutil::divifgt0(indexFitPtWtd * pf.accumWeight, 
                                              pf.moveIndexTestsWtd.at(i).projectedVariance);
      indexScorePropWtd += IRutil::divifgt0(indexFitPtWtd*pf.moveIndexTestsWtd.at(i).projectedValue,
                                            sqrt(pf.moveIndexTestsWtd.at(i).projectedVariance));

   } 

   if (fixDistortion) { 
      indexFitPt = (*dist) 
         (pf.moveIndexTests.at(IRutil::NCUTOFF).projectedFreq,
          pf.moveIndexTests.at(IRutil::NCUTOFF).measuredFreq);
      indexDiffPt = pf.moveIndexTests.at(IRutil::NCUTOFF).measuredFreq
                        - pf.moveIndexTests.at(IRutil::NCUTOFF).projectedFreq;
      indexFitPtWtd = (*dist) 
         (pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).projectedFreq,
          pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).measuredFreq);
      indexDiffPtWtd = pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).measuredFreq
                        - pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).projectedFreq;
      indexScore += indexFitPt; 
      indexDiff += indexDiffPt;
      indexMass += indexFitPt*sqrt(pf.moveIndexTests.at(IRutil::NCUTOFF).measuredFreq);
      indexDiffMass += indexDiffPt*sqrt(pf.moveIndexTests.at(IRutil::NCUTOFF).measuredFreq);
      indexScoreInvVar += IRutil::divifgt0(indexFitPt * pf.totalBootItems, 
                                           pf.moveIndexTests.at(IRutil::NCUTOFF).projectedVariance); 
      indexScoreProp += IRutil::divifgt0(indexFitPt 
                           * pf.moveIndexTests.at(IRutil::NCUTOFF).projectedValue, 
                           sqrt(pf.moveIndexTests.at(IRutil::NCUTOFF).projectedVariance)); 
      indexScoreWtd += indexFitPtWtd; 
      indexDiffWtd += indexDiffPtWtd;
      indexMassWtd += indexFitPtWtd*sqrt(pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).measuredFreq);
      indexDiffMassWtd += indexDiffPtWtd * sqrt(pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).measuredFreq);
      indexScoreInvVarWtd += IRutil::divifgt0(indexFitPtWtd * pf.accumWeight,
                                              pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).projectedVariance);
      indexScorePropWtd += IRutil::divifgt0(indexFitPtWtd
                           * pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).projectedValue,
                             sqrt(pf.moveIndexTestsWtd.at(IRutil::NCUTOFF).projectedVariance));
      pf.indexFit = indexScore / (IRutil::NCUTOFF + 1); 
      pf.indexFitMass = indexMass; 
      pf.indexFitInvVar = indexScoreInvVar / (IRutil::NCUTOFF + 1); 
      pf.indexFitProp = indexScoreProp / (IRutil::NCUTOFF + 1); 
      pf.indexFitWtd = indexScoreWtd / (IRutil::NCUTOFF + 1); 
      pf.indexFitMassWtd = indexMassWtd; 
      pf.indexFitInvVarWtd = indexScoreInvVarWtd / (IRutil::NCUTOFF + 1); 
      pf.indexFitPropWtd = indexScorePropWtd / (IRutil::NCUTOFF + 1); 
   } else { 
      pf.indexFit = indexScore / IRutil::NCUTOFF; 
      pf.indexFitMass = indexMass; 
      pf.indexFitInvVar = indexScoreInvVar / IRutil::NCUTOFF; 
      pf.indexFitProp = indexScoreProp / IRutil::NCUTOFF; 
      pf.indexFitWtd = indexScoreWtd / IRutil::NCUTOFF; 
      pf.indexFitMassWtd = indexMassWtd; 
      pf.indexFitInvVarWtd = indexScoreInvVarWtd / IRutil::NCUTOFF; 
      pf.indexFitPropWtd = indexScorePropWtd / IRutil::NCUTOFF; 
 
   } 
   pf.indexDiff = indexDiff;   //no averaging
   pf.indexDiffMass = indexDiffMass;
   pf.indexDiffWtd = indexDiffWtd;
   pf.indexDiffMassWtd = indexDiffMassWtd;
 
   MOVESDUMP.close();
   return pf; 
} 
 



SimpleStats Trial::perfTestSimple(size_t numIndices, size_t numSelectors, 
                                  vector<MoveSelector*>* mySelectors,
                                  size_t equalTopIndexLimit,
                                  bool unitWeights, bool tmaNN, bool giveFreq) const {
   SimpleStats ss(numIndices, numSelectors);
   if (theDecisions->size() == 0) { return ss; } 
 
   bool weighted = (!unitWeights)  
                       && theSpec.weightMethodChoice != IRfun::UNITWTS; 

   size_t numInd = 1 + IRutil::NCUTOFF;     //will use numIndices later...

   vector<MoveIndexTest> mtv;  //although SimpleStats has indices, variances needed here too
   for (int i = 1; i <= numInd; i++) {      //move indices print as 1-based
      MoveIndexTest mtest(i, 7, 2, 0.0, 0.0, 0.0,    //not giving freq yet
                          true, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1, 1.0, false, false);
      mtv.push_back(mtest);
   }

   //vector<double> probs; 
 
   //Models::MoveProjection mp; 

   for (vector<DecisionInfo*>::const_iterator it = theDecisions->begin(); 
        it != theDecisions->end(); it++) { 

      DecisionInfo* dec = *it; 
      const double bw = dec->getBootWeight();

      if (bw < IRutil::MINPROB) { continue; } //Else---!!

      Models::MoveProjection* mp = Models::getProjection(dec,theSpec,pwv,swv,model,equalTopIndexLimit);
      //probs = Models::getProbs(dec,theSpec,pwv,swv,model); 
      double weight = (weighted ? mp->weight : 1.0);
      double bweight = bw*weight;

//if (bweight < 0.0) { cerr << "Negative weight at turn " << string(*dec) << endl; }
 
      size_t numDeltas = dec->getNumberOfMovesAnalyzed(); 
      size_t pmi = dec->getPlayedMoveIndex(); 

      size_t jd = theScale->getJudgmentDepth();
      size_t useDepth = ((jd == 0 || jd >= dec->dataHighDepth) ? dec->dataHighDepth
                             : (jd <= dec->dataLowDepth ? dec->dataLowDepth : jd));
      ss.accumWeight += bweight;
      const double p0 = mp->probFirstMove;
      const double topProb = mp->probEqualTopMove; 
      //double pj = (pmi != IRutil::playedMoveNotInTopN   //pmi, NOT playedRank
                   //|| (fixDistortion && treatMissAsMoveNN)) ? mp->probs->at(pmi) : 0.0;
      //if (pj < IRutil::MINPROB) { pj = IRutil::MINPROB; }
      const double psf = mp->projFalloffScaled;  //Models::predScaledFalloff(dec, probs,useDepth);

      const double matchAdd = (mp->rankPlayedMove == 0 ? 1.0 : 0.0);
      //const double topAdd = (dec->getEvalP(pmi,useDepth) == dec->getBestEvalP(useDepth) ? 1.0 : 0.0);
      bool etvMatch = dec->getEvalP(pmi, useDepth) == dec->getBestEvalP(useDepth)
                         && mp->rankPlayedMove <= equalTopIndexLimit;
      const double topAdd = (etvMatch ? 1.0 : 0.0);

      ss.scaledFalloffWtd += bweight*mp->playedMoveDeltaScaled;
      ss.scaledFalloffProjWtd += bweight*mp->projFalloffScaled;
      ss.scaledFalloffVarianceProjWtd += bweight*mp->projFalloffVarianceScaled;
      
      //pf.unscaledFalloffWtd.uptick(bweight, bweight*(dec->getUnscaledFalloffAtMaxDepth()), 
                                   //bweight*puf, bweight*pufv, bweight*puls, bweight*pues, false);

      size_t i = 0;
      size_t indexHits = 0;
      while (i < numDeltas && indexHits < numIndices) {
         size_t j = (useDepth >= dec->dataHighDepth ? i : dec->getRankAtDepth(i,useDepth));
                     //theScale->getJudgmentDepth() == 0 ? i : dec->getRankAtDepth(i,useDepth));
         if (j < numIndices) {
            double pri = (i < mp->probs->size() ? mp->probs->at(i) : 0.0);
            ss.indexAccume.at(j) += bweight;
            ss.indexMatchesWtd.at(j) += bweight * (pmi == i ? 1.0 : 0.0);
            ss.indexMatchesProjWtd.at(j) += bweight * pri;
            ss.indexMatchesVarianceProjWtd.at(j) += bweight * pri * (1.0 - pri);
            indexHits++;
         }
         i++;
      }

      ss.equalValueMatchWtd += bweight * topAdd;
      ss.equalValueMatchProjWtd += bweight * topProb;
      ss.equalValueMatchVarianceProjWtd += bweight * topProb * (1.0 - topProb);


      size_t numFilledIndices = (mp->probs->size() >= IRutil::NCUTOFF + 1 ? IRutil::NCUTOFF + 1
                                                                          : mp->probs->size());
      for (size_t k = 0; k < numFilledIndices; k++) {

         if (i >= dec->getNumMovesWithValues()) { continue; }

         double dt = (useDepth >= dec->dataHighDepth   //theScale->getJudgmentDepth() == 0 
                      ? dec->getHighestDepthMoveDeltaScaled(k)
                      : dec->getDeltaScaled(k,useDepth));

         //if (dt > deltaCap) { dt = deltaCap; }  //change from before: not auto-applied to "miss".
         double prk = mp->probs->at(k);
         if (prk < IRutil::MINPROB) { prk = IRutil::MINPROB; }
         double hitAdd = (pmi == k ? 1.0 : 0.0);
         mtv.at(k).uptick(bweight, bweight*hitAdd, bweight*prk, bweight*prk*(1.0-prk),
                          bweight*IRutil::log2(1.0/prk), bweight*prk*IRutil::log2(1.0/prk),
                          bweight*dt, 
                          bweight*dec->getDecMoveSwing(i),
                          bweight*mp->swingsUsed->at(i),
                          bweight*(mp->swingsUsed->at(i) - mp->swingsUsed->at(mp->indexFirstMove)));
                          //bweight*(dec->getSwingScaled(k, dec->userHighDepth, true)));
                          //bweight*dt, bweight*(dec->getSwing(k, dec->userHighDepth, true)));
      }

      const TurnInfo* pTurn = dec->pParent;
      Move pm = pTurn->movePlayed;
      Move em = pTurn->engineMove;
      for (size_t k = 0; k < numSelectors; k++) {
         double playedMoveAdd = (mySelectors->at(k)->apply(pTurn, pm) ? 1.0 : 0.0);
         double engineMoveAdd = (mySelectors->at(k)->apply(pTurn, em) ? 1.0 : 0.0);
         double selectionProb = 0.0;
         size_t i = 0;
         while (i < numDeltas) {
            Move move = pTurn->consideredMoves->at(i);
            if (mySelectors->at(k)->apply(dec->pParent, move)) {
               selectionProb += mp->probs->at(i);  //no test for near-zero needed here
            }
            i++;
         }
         if (selectionProb < IRutil::MINPROB) { selectionProb = IRutil::MINPROB; }
         if (selectionProb > 1.0) {
            if (selectionProb > 1.0 + pTurn->numConsideredMoves * IRutil::MINPROB) {
               cerr << "OOB selection prob " << setprecision(10) << selectionProb
                    << " in test " << mySelectors->at(k)->getName()
                    << " at turn " << dec->pParent->getFEN() << endl;
            }
            selectionProb = 1.0;
         }

         ss.selectorMatchesPlayerWtd.at(k) += bweight * playedMoveAdd;
         ss.selectorMatchesPlayerProjWtd.at(k) += bweight * selectionProb;
         ss.selectorMatchesPlayerProjVarianceWtd.at(k) += bweight * selectionProb * (1.0 - selectionProb);
         ss.selectorMatchesEngineWtd.at(k) += bweight * engineMoveAdd;
      }
      delete(mp);

   } //end of loop over DecisionInfo.  Now update all MoveTests

   size_t k = 0;
   for (k = 0; k < mtv.size(); k++) {
      mtv.at(k).update();
   }

   if (giveFreq) {
      for (size_t j = 0; j < numIndices; j++) {
         ss.indexMatchesWtd.at(j) /= ss.accumWeight;
         ss.indexMatchesProjWtd.at(j) /= ss.accumWeight;
         ss.indexMatchesVarianceProjWtd.at(j) /= (ss.accumWeight * ss.accumWeight);
      }
      ss.equalValueMatchWtd /= ss.accumWeight;
      ss.equalValueMatchProjWtd /= ss.accumWeight;
      ss.equalValueMatchVarianceProjWtd /= (ss.accumWeight * ss.accumWeight);
    
      ss.scaledFalloffWtd /= ss.accumWeight;
      ss.scaledFalloffProjWtd /= ss.accumWeight;
      ss.scaledFalloffVarianceProjWtd /= (ss.accumWeight * ss.accumWeight);
   
      for (k = 0; k < numSelectors; k++) {
         ss.selectorMatchesPlayerWtd.at(k) /= ss.accumWeight;
         ss.selectorMatchesPlayerProjWtd.at(k) /= ss.accumWeight;
         ss.selectorMatchesPlayerProjVarianceWtd.at(k) /= (ss.accumWeight * ss.accumWeight);
         ss.selectorMatchesEngineWtd.at(k) /= ss.accumWeight;
      }
   }


   double indexFitPtWtd; 
   double indexDiffPtWtd;
 
   for (int i = 0; i < IRutil::NCUTOFF; i++) { 
      indexFitPtWtd = (*dist)(mtv.at(i).measuredFreq, mtv.at(i).projectedFreq);
      indexDiffPtWtd = mtv.at(i).measuredFreq - mtv.at(i).projectedFreq;

      ss.indexScoreWtd += indexFitPtWtd; 
      ss.indexDiffWtd += indexDiffPtWtd;
      ss.indexMassWtd += indexFitPtWtd*sqrt(mtv.at(i).measuredFreq);
      ss.indexDiffMassWtd += indexDiffPtWtd*sqrt(mtv.at(i).measuredFreq);
      ss.indexScoreInvVarWtd += IRutil::divifgt0(indexFitPtWtd * ss.accumWeight, 
                                                 mtv.at(i).projectedVariance);
      ss.indexScorePropWtd += IRutil::divifgt0(indexFitPtWtd*mtv.at(i).projectedValue,
                                               sqrt(mtv.at(i).projectedVariance));


   } 

   if (fixDistortion) { 
      indexFitPtWtd = (*dist) 
         (mtv.at(IRutil::NCUTOFF).projectedFreq, mtv.at(IRutil::NCUTOFF).measuredFreq);
      indexDiffPtWtd = mtv.at(IRutil::NCUTOFF).measuredFreq - mtv.at(IRutil::NCUTOFF).projectedFreq;

      ss.indexScoreWtd += indexFitPtWtd; 
      ss.indexDiffWtd += indexDiffPtWtd;
      ss.indexMassWtd += indexFitPtWtd*sqrt(mtv.at(IRutil::NCUTOFF).measuredFreq);
      ss.indexDiffMassWtd += indexDiffPtWtd*sqrt(mtv.at(IRutil::NCUTOFF).measuredFreq);
      ss.indexScoreInvVarWtd += IRutil::divifgt0(indexFitPtWtd * ss.accumWeight,
                                              mtv.at(IRutil::NCUTOFF).projectedVariance);
      ss.indexScorePropWtd += IRutil::divifgt0(indexFitPtWtd
                                 * mtv.at(IRutil::NCUTOFF).projectedValue,
                                 sqrt(mtv.at(IRutil::NCUTOFF).projectedVariance));
      ss.indexFitWtd = ss.indexScoreWtd / (IRutil::NCUTOFF + 1); 
      ss.indexScoreInvVarWtd /= (IRutil::NCUTOFF + 1); 
      ss.indexScorePropWtd /= (IRutil::NCUTOFF + 1);
   } else { 
      ss.indexFitWtd = ss.indexScoreWtd / IRutil::NCUTOFF; 
      ss.indexScoreInvVarWtd /= IRutil::NCUTOFF; 
      ss.indexScorePropWtd /= IRutil::NCUTOFF; 
 
   } 
   
 
   return ss; 
} 

#endif   //end of #ifndef __IR_TRIAL_CPP__ 
 
 
 
