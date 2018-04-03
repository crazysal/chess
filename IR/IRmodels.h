//File-----------------------IRmodels.h----------------------------------- 

/**
A "Model" is defined by the function f on the left-hand side of the equation

    f(p_i)
    ------   =   g_c(x_i)           where x_i = scalediff(v_i,v_0)/s
    f(p_0)

The case f(p) = p is called "Shares", and gives p_i = p_0*g_c(x_i).
The case f(p) = lop(p) --- actually, f(p) = 1/log(1/p) --- gives
p_i = p_0^{1/g_c(x_i)} and is (hence) called "PowerShares".  

In each case the depth d parameter is reflected by taking a weighted
linear combination over depths in a "window" lo..hi, with weights according
to Poisson distribution.  The variation v parameter affects only the
concentration of the distribution, as its "coefficient of variation":
v = 1 gives pure Poisson, v = 0 puts all weight on the two integer neighbors
of d, and v --> infty creates a uniform (i.e., "flat") distribution on the 
window.  The four fittable parameters come from the TrialSpec, while the
window values lo,hi and also the zero-point z of the Poisson distribution
are set by the DeltaScale in effect.  

The rules for the window are as follows: The data class DecisionInfo 
maintains fields dataLowDepth and dataHighDepth giving the actual range
of depths output in the chess engine analysis.  In particular, dataLowDepth
is the least value k for which some move has a numerical value not "n.a."

(Whereas TurnInfo has range turnLowestDepth..turnHighestDepth including "n.a." columns.)
Note: The code allows for "n.a." columns at either end more than the engine standardly gvies.

The user window userLowDepth..userHighDepth is allowed to extend outside
dataLowDepth...dataHighDepth, with no restriction other than 
0 <= z <= userLowDepth (maybe better 1 <= z <= userLowDepth).
The Poisson weights vector is generated as pw[z..userHighDepth] with pw[userHighDepth]
including the tail to infinity.  Then the values in pw[z..userLowDepth-1]
are added to pw[userLowDepth] to make pw'[userLowDepth..userHighDepth]. 
Finally to make the depth-weights vector dw we do:

lo = max{userLowDepth,dataLowDepth}
hi = min{userHighDepth,dataHighDepth}
dw[lo] = pw[userLowDepth] + ... + pw[lo]
dw[hi] = pw[hi] + ... pw[userHighDepth]
dw[j] = pw[j] for lo < j < hi, which all satisfy dataLowDepth < j < dataHighDepth too.


**Tamal: this is a change from duplicating tuples in DecisionInfo to cover
cases where userLowDepth < dataLowDepth in particular.  The code in processTurnInfo that
overwrites initial "n.a."'s by values at the next-higher depths may still be
necessary, however, as I think there are cases where "n.a." happens at a
depth for which other moves have values, cases where it might not be really
a "PRUN" or "NREC" (?---not sure, but anyway that code can't hurt).
It may be silly to allow z < lo, but anyway I think little is lost by it.

The final issue is when the depth-weighting is done.  It can be applied to
the raw evaluations (even before scaling) to create a "perceived value" 
estimate for each move.  Then delta-differences are taken and scaled, still
using the best-move-at-highest-depth ordering of the DecisionInfo class
itself (hence taking care that the deltas while still nonnegative might not 
be monotonically increasing), to create a TupleInfo object that is treated
as in the old version of the model.  The "official" way (methodologically
sounder, IMO) is to use the TupleInfo for each depth given by DecisionInfo,
compute the probabilities p_{i,k} for each depth k, then 

                     p_i = \sum_k dw[k]p_{i,k}

In this sum, I think it is OK to limit k to lo <= k <= hi.  I had originally
thought I might want to preserve information about every depth in userLowDepth..userHighDepth,
but for userLowDepth <= k < lo there really isn't any new information. 
Bottom line is that the windowing nomenclature should be (IMHO):

1. TurnInfo has fields dataLowDepth, dataHighDepth, while the evals-array has
indexing [0 .. dataHighDepth-dataLowDepth]
2. DecisionInfo has the same fields, with similar indexing for the
"inside-out" evalSwap array.
3. DeltaScale has the fields userLowDepth,userHighDepth,z (remove the first two from TrialSpec).
4. Only the code here computes lo and hi.
 */
 
#ifndef __IR_MODELS_H__ 
#define __IR_MODELS_H__ 
 
#ifndef __IR_TUPLE_INFO_H__
#include "TupleInfo.h"
#endif

#ifndef __IR_DECISION_INFO_H__
#include "DecisionInfo.h"
#endif


namespace Models { 
 
   typedef vector<double> (*ModelProbs) (const TupleInfo* const ti, 
                                         const TrialSpec& ts); 
 
   enum MODEL {  
      SHARES,  
      POWER_SHARES,  
      //ENTROPY_SHARES, 
      NUM_MODELS 
   }; 

   /** Non-POD struct.  Should be kept by pointer and not copied, unlike TrialSpec.
    */
   struct MoveProjection {    //POD but with a pointer that caller takes charge of
      CurveApplication* proxies;
      vector<double>* probs;
      string firstMove;
      string highestProbMove;
      string playedMove;
      size_t indexFirstMove;        //top rank at judgment depth
      size_t indexHighestProbMove;  //initialized via swapIndex in getProbs(...)
      size_t indexPlayedMove;       //index of its prob in probs vector
      size_t rankPlayedMove;        //rank at judgment depth
      size_t equalTopIndexLimit;    //equal-top moves beyond this not counted as matches
      double probFirstMove;
      double probEqualTopMove;
      double probPlayedMove;
      double highestProb;
      double playedMoveDeltaScaled;
      double projFalloffScaled;
      double projFalloffVarianceScaled;
      double playedMoveDeltaUnscaled;
      double projFalloffUnscaled;
      double projFalloffVarianceUnscaled;
      double projLogSumScaled;
      double projLogSumUnscaled;
      double projEntropySumScaled;
      double projEntropySumUnscaled;
      double weight;  //previous fields are NOT already weighted.
      bool deleteSwingsHere;
      const vector<double>* swingsUsed;  //always set from outside; caller deletes(?)

      explicit MoveProjection(bool dds = true) : probs(NULL), proxies(NULL), firstMove("")
       , highestProbMove("") , playedMove(""), indexFirstMove(0), indexHighestProbMove(0), indexPlayedMove(0)
       , equalTopIndexLimit(IRutil::NCUTOFF)    //rankPlayedMove set later, not initialized 
       , probFirstMove(0.0), probEqualTopMove(0.0), probPlayedMove(0.0), highestProb(0.0)
       , playedMoveDeltaScaled(0.0), projFalloffScaled(0.0), projFalloffVarianceScaled(0.0)
       , playedMoveDeltaUnscaled(0.0), projFalloffUnscaled(0.0), projFalloffVarianceUnscaled(0.0)
       , projLogSumScaled(0.0), projLogSumUnscaled(0.0), projEntropySumScaled(0.0)
       , projEntropySumUnscaled(0.0), weight(0.0), deleteSwingsHere(dds), swingsUsed(NULL)
      { }
      virtual ~MoveProjection() { 
         delete(proxies);
         delete(probs); 
         if (deleteSwingsHere) { delete swingsUsed; }
      }  //usages should be pointers or const& 
   };

   inline vector<double>* fudgePatch(const size_t numw, const double patchPower);
   inline void patchProbs(vector<double>* probs, double patchPower);
      

   //Does not require swapping min/max to first index
   //inline vector<double> probsShares(const TupleInfo* const ti,  
   inline MoveProjection* probsShares(const TupleInfo* const ti,
                                      const TrialSpec& ts) {
      MoveProjection* mp = new MoveProjection(ti->swingsWereSetNew);
      mp->proxies = ti->applyCurve(ts);  //treated as own copy
      double pj;
      const size_t numProbs = mp->proxies->values.size();
      //vector<double> probs(numProbs);
      mp->probs = new vector<double>(numProbs);  //not init
      mp->swingsUsed = ti->moveSwings;
      mp->indexHighestProbMove = mp->proxies->swapIndex;  //other fields filled by callers
      double sumShares = 0.0; 
      for (int i = 0; i < numProbs; i++) { 
         sumShares += mp->proxies->values.at(i); 
      } 
 
      //test for fixing distortion removed 
      for (int j = 0; j < numProbs; j++) { 
         pj = mp->proxies->values.at(j) / sumShares;  //can be 1/INF 
         mp->probs->at(j) = (pj > IRutil::MINPROB ? pj : IRutil::MINPROB); 
      } 
      if (ti->patchesProbs) { patchProbs(mp->probs, ti->patchPower); }
      return mp;
   } 

 

   /**
       Requires permuting the applications to have min value in first index.
       Combines three measures to combat "inf" and "NaN" values: 
          () "invert" has a MAXINV cap, 
          () vectorRoot imposes a maximum probability for the first move, 
          () MINPROB puts a nonzero floor for probabilities of other moves. 
    */ 
   inline MoveProjection* probsPowerShares(const TupleInfo* const ti, 
                                           const TrialSpec& ts) {
      //vector<double> inverses = IRutil::invera(ti->applyCurve(ts)); 
      MoveProjection* mp = new MoveProjection(ti->swingsWereSetNew);
      mp->proxies = ti->applyCurve(ts);  //treated as own copy
      double swap = mp->proxies->values.at(mp->proxies->swapIndex);
      mp->proxies->values.at(mp->proxies->swapIndex) =  mp->proxies->values.at(0);
      mp->proxies->values.at(0) = swap;
      vector<double> inverses = IRutil::anvert(mp->proxies->values, ti->swingPolicy->transPower);
      size_t sz = inverses.size(); 
      mp->probs = new vector<double>(sz);
      mp->swingsUsed = ti->moveSwings;
      mp->indexHighestProbMove = mp->proxies->swapIndex;  //other fields filled by callers
      //vector<double> probs(sz); 
      double p0, pj; 
      {  //distortion-fixing test removed 
         p0 = (sz == 1 ? 1.0 : IRutil::vectorRoot(inverses, 1.0)); 
         mp->probs->at(0) = p0; 
         for (int j = 1; j < sz; j++) { 
            pj = pow(p0,inverses.at(j)); 
            mp->probs->at(j) = (pj > IRutil::MINPROB ? pj : IRutil::MINPROB); 
         } 
      } 
      if (mp->indexHighestProbMove > 0) {
         swap = mp->probs->at(mp->proxies->swapIndex);
         mp->probs->at(mp->proxies->swapIndex) = mp->probs->at(0);
         mp->probs->at(0) = swap;
         //swap back proxy values too just for show
         swap = mp->proxies->values.at(mp->proxies->swapIndex);
         mp->proxies->values.at(mp->proxies->swapIndex) =  mp->proxies->values.at(0);
         mp->proxies->values.at(0) = swap;
      }
      double dbg = 0.0;
      for (size_t m = 0; m < mp->probs->size(); m++) { dbg += mp->probs->at(m); }
      double test = fabs(1.0 - dbg);
      if (test > IRutil::SUMPROBSTOLERANCE) {
         if (test > IRutil::SHOUTTOLERANCE) {
            cerr << "Got sum " << dbg << " from p0 = " << p0 << " and inverses:" << endl;
            for (int i = 0; i < inverses.size(); i++) {
               cerr << inverses.at(i) << " ";
               if ((i + 1) % 10 == 0) { cerr << endl; }
            }
            cerr << endl;
         }
         IRutil::normalizeP(mp->probs);
      }
      if (ti->patchesProbs) { patchProbs(mp->probs, ti->patchPower); }
      return mp;
   } 


   /** Scales afterward.  The depthWts are an indirect dependence on TrialSpec, 
       judgmentDepth a possible one.  That is why the routine is here.
       It is possible that the judgmentDepth argument may override the one in theScale.
       Depth arguments in DecisionInfo methods are absolute, but the depthWts and
       SwingWts vectors are indexed from 0.  However, they obey the invariants that
       depthWts.size() = uhd - uld + 1, swingWts.size() = uswhi - uswlo + 1.
       So the code simply iterates over them.  
    */
   inline TupleInfo* perceivedTuple(const DecisionInfo* const di, 
                                    const vector<double>* depthWts,
                                    const vector<double>* swingWts,
                                    size_t judgmentDepth) {
   //REQ: depthWts.size() = hi - lo + 1 in the following.
   //And swingWts.size() = swhi - swlo + 1 likewise.
      int uld = di->theScale->getLowDepth();
      int uhd = di->theScale->getHighDepth();
      int lo = min(max(uld,int(di->dataLowDepth)),int(di->dataHighDepth));
      //"min" part covers cases like z = uld = 2 but move has depth-1 only.
      int hi = min(uhd,int(di->dataHighDepth));

      int numDWs = uhd - uld + 1;
      int numPWs = hi - lo + 1;   //so lo serves as offset to absolute depths

      vector<double> probWts(numPWs,0.0);
      int i,j;
      for (i = 0; i <= lo - uld; i++) {
         probWts.at(0) += depthWts->at(i);
      }
      for (j = 1; j < numPWs; j++) {
         probWts.at(j) = depthWts->at(lo-uld+j);
      }
      for (i = max(hi-uld+1,0); i < numDWs; i++) {
         probWts.at(numPWs-1) += depthWts->at(i);
      }


      size_t ell = di->getNumMovesWithValues();

      vector<double> perceivedEvals(ell,0.0);
      int btm = di->pParent->whiteToMove ? 1 : -1;
      double maxPE = btm*(-IRutil::MATEVAL);  //from White's viewpoint

      for (size_t i = 0; i < ell; i++) {
         for (size_t d = 0; d < numPWs; d++) {
            perceivedEvals.at(i) += di->getEvalP(i,d+lo)*probWts.at(d);//in 0.00 units
         }
         if ((btm*perceivedEvals.at(i)) > (btm*maxPE)) {
            maxPE = perceivedEvals.at(i); //move with max perceived eval need not be first one
         } 
      }

      TurnContext* tc = new TurnContext(di->pParent->movePlayed, 
                                        di->pParent->engineID,
                                        btm == 1,
                                        maxPE,
                                        di->pParent->prevEvalCP/100.0,
                                        di->pParent->nextEvalCP/100.0);
      //TurnContext* tcusw = new TurnContext(*tc);  //default copy constructor

      vector<double>* perceivedDeltas = new vector<double>(ell);
      for (size_t k = 0; k < ell; k++) {
         perceivedDeltas->at(k) = btm*(maxPE - perceivedEvals.at(k));
      }
      SwingInfo* swp = di->theScale->getSwingPolicy();


      TupleInfo* ti = di->makeTuple(perceivedDeltas, tc, true, swp, true); 
      //last arg makes perceived, and so deletes perceivedDeltas & tc



      if (swp->noSwing || (!swp->depthDependentSwings)) {
         return ti; 
      }  //else we use a depth-dependent swing measure


      size_t userSwingLow = di->theScale->getSwingLow();  //not assumed in range of data
      size_t userSwingHigh = di->theScale->getSwingHigh();
      
      size_t jd = (judgmentDepth == 0 ? di->dataHighDepth
                                      : min(judgmentDepth, di->dataHighDepth));
      jd = max(userSwingLow, jd);
      size_t swadd = 1;
      double skipProb = 0.0;
      if (userSwingLow <= jd && jd <= userSwingHigh) {
         skipProb = swingWts->at(jd - userSwingLow);
         swadd = 0;
      }
      //double hieval = di->getBestEvalP(jd);
      //tcusw->currEval = hieval;
      size_t numSWs = userSwingHigh - userSwingLow + swadd;
      
      vector<double>* userSwings = new vector<double>(ell, 0.0); //accumulated not assigned
      for (size_t j = 0; j < ell; j++) {
         double jdel = di->getDeltaScaled(j,jd);
         //double jMinDelta = IRutil::MATEVAL;  //jdel;  //judgment depth is included in the comparisons
         //double jMaxDelta = -IRutil::MATEVAL; //jdel;
         double jMinDelta = jdel;
         double jMaxDelta = jdel;
         double jMeanDelta = 0.0;
         double djdel;  //so is highest-depth delta even if jd is inside.
	 for (size_t d = 0; d < numSWs; d++) {
            size_t absDepth = d + userSwingLow;
            //tcusw->currEval = di->getBestEvalP(absDepth);
            djdel = di->getDeltaScaled(j, absDepth);
            jMeanDelta += djdel;
            if (absDepth >= 5) {
               if (djdel < jMinDelta) { jMinDelta = djdel; }
               if (djdel > jMaxDelta) { jMaxDelta = djdel; }
            }
            userSwings->at(j) += (swingWts->at(d) + skipProb/double(numSWs))*(djdel - jdel); //normalizes
         }
         jMeanDelta /= double(userSwingHigh - userSwingLow + 1);


         //if (jdel < jMinDelta) { jMinDelta = jdel; }
         //if (jdel > jMaxDelta) { jMaxDelta = jdel; }

         //Now we want to scale down according to minDelta and/or jHiDelta, but we also
         //want to keep the swings dimensioned in centipawns, not to become dimensionless.
         //Hence the idea of "boxed".
         
         //if (jMaxDelta > IRutil::DELTACAP) { 
            //jMaxDelta = IRutil::DELTACAP; 
         //}
         //if (jMinDelta > IRutil::DELTACAP) { jMinDelta = IRutil::DELTACAP; }
         //double mul = (userSwings->at(j) > 0.0 ? 1.0 : 1.15);
         const double mul = 1.0;
         const double scaler = 1.0 + di->theScale->getEqualTopFix();
         double factor = 1.0;
         if (jMinDelta > IRutil::MINPROB) {
            //size_t boxStart = swp.find("boxed");
            if (swp->boxed) {
               //char c = swp.at(boxStart+5);
               unsigned int bxk = (unsigned int)swp->boxKind;
               switch(bxk) {
                case 0:
                  factor = scaler;
                  break;
                case 1:
                  factor = (jMaxDelta - jMinDelta)/jMaxDelta;
                  break;
                case 2:
                  factor = (jMeanDelta - jMinDelta)/jMeanDelta;
                  break;
                case 3:
                  factor = 1.0 - jMinDelta/(jMaxDelta + jMinDelta);
                  break;  //note: with 1 in place of kMaxDelta this is same as "damped"
                case 4:
                  factor = 1.0 - jMinDelta/(jMeanDelta + jMinDelta);
                  break;
                case 5:
                  factor = jMaxDelta/(2.0*jMeanDelta);
                  break;
                case 6:
                  //factor = jMaxDelta*jdel/((jdel + jMinDelta)*jMeanDelta);
                  factor = 1.0 - jMinDelta*jMinDelta/((scaler + jMinDelta)*jdel);
                  break;
                case 7:
                  factor = (jMaxDelta - jMinDelta)/(jMaxDelta + jMinDelta);
                  break;
                case 8:
                  factor = IRutil::sqr((jMaxDelta - jMinDelta)/jMaxDelta);
                  break;
                case 9:
                  factor = sqrt((jMaxDelta - jMinDelta)/jMaxDelta);
                  break;
                default:
                  factor = (jMaxDelta - jMinDelta)/jMaxDelta;
               }
               //boxStart += 6;
               //if (boxStart < swp.size() && swp.at(boxStart) == 'a') {
                  //factor *= log(1 + (jdel/jMinDelta));
               //} else if (boxStart < swp.size() && swp.at(boxStart) == 'b') {
                  //factor *= jdel/jMeanDelta;
               //}
            } else {
               double jHiDelta = (jd < userSwingHigh ? djdel : jdel);
               const double damp1 = (swp->prop ? 1.0 : 0.0);
               const double damp2 = (swp->damped ? 1.0 : 0.0);
               factor = scaler/(scaler + damp1*jHiDelta + damp2*jMinDelta);
            }
            userSwings->at(j) *= mul*factor;
            if (!(userSwings->at(j) <= 0.0 || userSwings->at(j) > 0.0)) {
               cerr << "Bad value usw[" << j << "] = " << userSwings->at(j) << " in turn" << endl
                    << string(*di) << endl;
               IRutil::PROBLEM_FENS.insert(di->pParent->getFEN());
               IRutil::PROBLEM_GAMES.insert(di->pParent->gameID + "\n" + di->pParent->getFEN());
            }
         }
      }

      ti->setSwings(userSwings);  //will be deleted when ti is deleted
      return ti;
   }

   
//TrialSpec has "d" in absolute terms; DecisionInfo maintains relative terms
//internally but uses absolute depths as arguments.
 
   inline MoveProjection* getProbsShares(const DecisionInfo* const di,  
                                         const TrialSpec& ts,
                                         const vector<double>* depthWts,
                                         const vector<double>* swingWts,
                                         const double judgmentDepth) {

      if (di->theScale->weightsEvalsFirst()) {   //fff
         TupleInfo* ti = perceivedTuple(di, depthWts, swingWts, judgmentDepth); //news ti
         //vector<double> vee = probsShares(ti,ts);
         MoveProjection* mp = probsShares(ti,ts);
         delete(ti);
         return mp;
      } //else
      MoveProjection* mp = new MoveProjection(false);
      //vector<double> probs(di->getNumMovesWithValues(), 0.0);
      mp->probs = new vector<double>(di->getNumMovesWithValues(), 0.0);
      for (size_t depth = 0; depth < depthWts->size(); depth++) { 
         TupleInfo* ti = di->getWindowInfoScaled(depth); 

         MoveProjection* mpd = probsShares(ti,ts);
         //vector<double> dprobs = probsShares(ti,ts); 

         for (size_t i = 0; i < ti->numDeltas; i++) { 
            //probs.at(i) += depthWts->at(depth)*dprobs.at(i); 
            mp->probs->at(i) += depthWts->at(depth) * mpd->probs->at(i);
         } 
         delete(mpd);
      } 
      double dbg = 0.0;
      for (size_t m = 0; m < mp->probs->size(); m++) { dbg += mp->probs->at(m); }
      if (fabs(1.0 - dbg) > IRutil::SUMPROBSTOLERANCE) {
         cerr << "Bad Poisson-weighting combination" << dbg << endl;
         IRutil::normalizeP(mp->probs);
      }

      return mp; 
   } 
 
   inline MoveProjection* getProbsPowerShares(const DecisionInfo* const di,  
                                              const TrialSpec& ts,
                                              const vector<double>* depthWts,
                                              const vector<double>* swingWts,
                                              const double judgmentDepth) {
   //(OLD) Now assume that depthWts is aligned with di->windowDeltasScaled


      MoveProjection* mp;
      if (di->theScale->weightsEvalsFirst()) {   
         //TupleInfo* ti = perceivedTuple(di, ts, probWts);
         TupleInfo* ti = perceivedTuple(di, depthWts, swingWts, judgmentDepth); //news ti
         mp = probsPowerShares(ti,ts);
         delete(ti);
         return mp;
      }
      //else
      //vector<double> probs(di->pParent->numLegalMoves, 0.0);
      //vector<double> probs(di->getNumMovesWithValues(), 0.0);
      mp = new MoveProjection(false);
      mp->probs = new vector<double>(di->getNumMovesWithValues(), 0.0);
      for (size_t depth = 0; depth < depthWts->size(); depth++) {
         TupleInfo* ti = di->getWindowInfoScaled(depth);   //not new

         MoveProjection* mpd = probsPowerShares(ti,ts);

         for (size_t i = 0; i < ti->numDeltas; i++) { 
            mp->probs->at(i) += depthWts->at(depth) * mpd->probs->at(i);     
         } 
         delete(mpd);
      } 
      double dbg = 0.0;
      for (size_t m = 0; m < mp->probs->size(); m++) { dbg += mp->probs->at(m); }
      if (fabs(1.0 - dbg) > IRutil::SUMPROBSTOLERANCE) {
         cerr << "Bad Poisson-weighting combination" << dbg << endl;
         IRutil::normalizeP(mp->probs);
      }
      return mp; 
   } 
 
   //Now fills in all fields of mp, so that caller doesn't have to.
   inline MoveProjection* getProjection(const DecisionInfo* const di, 
                                        const TrialSpec& ts,  
                                        const vector<double>* depthWts,
                                        const vector<double>* swingWts,
                                        const MODEL model,
                                        const size_t equalTopIndexLimit = IRutil::NCUTOFF) {
      MoveProjection* mp;
      size_t jd = di->theScale->getJudgmentDepth();
      size_t useDepth = ((jd == 0 || jd >= di->dataHighDepth) ? di->dataHighDepth
                             : (jd <= di->dataLowDepth ? di->dataLowDepth : jd));
      switch(model) { 
       case SHARES: 
         mp = getProbsShares(di,ts,depthWts,swingWts,useDepth);
         break;
       case POWER_SHARES: 
         mp = getProbsPowerShares(di,ts,depthWts,swingWts,useDepth);
         break;
       case NUM_MODELS: 
       default: 
         cerr << "Bad model choice in getProjection(...), using PowerShares" << endl; 
         mp = getProbsPowerShares(di,ts,depthWts,swingWts,useDepth);
      } 
      size_t numDeltas = mp->probs->size();
      size_t pmi = di->getPlayedMoveIndex();
      mp->indexPlayedMove = pmi;
      mp->firstMove = di->pParent->engineMove;
      mp->playedMove = di->pParent->movePlayed;
      mp->highestProbMove = di->pParent->consideredMoves->at(mp->indexHighestProbMove);
      
      mp->rankPlayedMove = (useDepth >= di->dataHighDepth ? mp->indexPlayedMove
                            : di->getRankAtDepth(mp->indexPlayedMove,useDepth));
      mp->indexFirstMove = (useDepth >= di->dataHighDepth ? 0
                                : di->getIndexFirstMoveAtDepth(useDepth));

      mp->probFirstMove = mp->probs->at(mp->indexFirstMove);
      mp->probPlayedMove = mp->probs->at(pmi);   //not probs.at(playedRank), keyed to move not index
      //mp.playedMoveIndex = playedRank;
      //mp->topMoveProb = 0.0;
      //
      int i = 1;
      mp->equalTopIndexLimit = min(equalTopIndexLimit,numDeltas-1);
      if (useDepth >= di->dataHighDepth) {   //theScale->getJudgmentDepth() == 0) {
         mp->probEqualTopMove = mp->probFirstMove;
         while (i <= mp->equalTopIndexLimit
             && di->getEvalP(i,di->dataHighDepth) == di->getEvalP(0,di->dataHighDepth)) {
            mp->probEqualTopMove += mp->probs->at(i);
            i++;
         }
      } else {
         for (i = 0; i < numDeltas; i++) {
            if (di->getEvalP(i,useDepth) == di->getBestEvalP(useDepth)
                   && di->getRankAtDepth(i,useDepth) <= mp->equalTopIndexLimit) {
               mp->probEqualTopMove += mp->probs->at(i);
            }
        }
      }


      double pj = mp->probs->at(pmi);
                   // (pmi != IRutil::playedMoveNotInTopN   //pmi, NOT playedRank
                   // || (fixDistortion && treatMissAsMoveNN)) ? mp->probs->at(pmi) : 0.0;
      if (pj < IRutil::MINPROB) { pj = IRutil::MINPROB; }
      mp->probPlayedMove = pj;

      mp->playedMoveDeltaScaled = (useDepth >= di->dataHighDepth //theScale->getJudgmentDepth() == 0
                                  ? di->getHighestDepthMoveDeltaScaled(pmi)
                                  : di->getDeltaScaled(pmi,useDepth)); 
      mp->playedMoveDeltaUnscaled = (useDepth >= di->dataHighDepth //theScale->getJudgmentDepth() == 0
                                     ? di->getHighestDepthMoveDeltaUnscaled(pmi)
                                     : di->getDeltaUnscaled(pmi,useDepth));

      /*  Projection routines for error/falloff.  Convention is that we:
      (*) Use the probabilities computed over all depths according to current depthWeights.
      (*) Use only the error values at the judgment depth.
          Thus we create the TupleInfo object from the judgment depth, and call previous routines.
       */


      const TupleInfo* const ti = (useDepth >= di->dataHighDepth ? di->getDepthInfoScaled(jd) 
                                                                 : di->getHighestTupleScaled());
      const TupleInfo* const tiu = (useDepth >= di->dataHighDepth ? di->getDepthInfoUnscaled(jd) 
                                                                  : di->getHighestTupleUnscaled());
      double ds,du,pri,logpri,sqrds,sqrdu;

      for (size_t i = 0; i < ti->numDeltas; i++) {
         pri = mp->probs->at(i);
         pri = (pri < IRutil::MINPROB ? IRutil::MINPROB: pri);
         ds = ti->pScaledDeltas->at(i);  //cap already applied
         mp->projFalloffScaled += pri * ds;  //cap already applied
         du = tiu->pScaledDeltas->at(i);
         mp->projFalloffUnscaled += pri * du;
         logpri = IRutil::log2(1.0/pri);
         mp->projLogSumScaled += logpri * ds;
         mp->projLogSumUnscaled += logpri * du;
         mp->projEntropySumScaled += pri * logpri * ds;
         mp->projEntropySumUnscaled += pri * logpri * du;
      }
      if (mp->probs->size() == ti->numDeltas + 1) { //estimate falloff from last delta
         pri = mp->probs->at(ti->numDeltas);
         pri = (pri < IRutil::MINPROB ? IRutil::MINPROB: pri);
         mp->projFalloffScaled += pri * ds;
         mp->projFalloffScaled += pri * du;
         logpri = IRutil::log2(1.0/pri);
         mp->projLogSumScaled += logpri * ds;
         mp->projLogSumUnscaled += logpri * du;
         mp->projEntropySumScaled += pri * logpri * ds;
         mp->projEntropySumUnscaled += pri * logpri * du;
      }

      for (size_t i = 0; i < ti->numDeltas; i++) {
         pri = mp->probs->at(i);
         pri = (pri < IRutil::MINPROB ? IRutil::MINPROB: pri);
         sqrds = IRutil::sqr(ti->pScaledDeltas->at(i) - mp->projFalloffScaled);
         sqrdu = IRutil::sqr(tiu->pScaledDeltas->at(i) - mp->projFalloffScaled);
         mp->projFalloffVarianceScaled += pri * sqrds;
         mp->projFalloffVarianceUnscaled += pri * sqrdu;
      }
      if (mp->probs->size() == ti->numDeltas + 1) { //estimate falloff from last delta
         pri = mp->probs->at(ti->numDeltas);
         pri = (pri < IRutil::MINPROB ? IRutil::MINPROB: pri);
         mp->projFalloffVarianceScaled += pri * sqrds;
         mp->projFalloffVarianceUnscaled += pri * sqrdu;
      }

      vector<double>::const_iterator itr, itre, jtr, jtre;
      double p0;
      switch(ts.weightMethodChoice) {
       case IRfun::UNITWTS:
         mp->weight = 1.0;
         break;
       case IRfun::SUMWTS:
         mp->weight = 1.0/mp->probs->at(mp->indexHighestProbMove);
         break;
       case IRfun::LOGSUMWTS:
         mp->weight = log(1.0/mp->probs->at(mp->indexHighestProbMove));
         break;
       case IRfun::ENTROPYWTS:
         itre = mp->probs->end();
         for (itr = mp->probs->begin(); itr != itre; ++itr) {
            mp->weight += (*itr)*log(1.0/(*itr));
         }
         break;
       case IRfun::FALLOFFWTS:
         itr = mp->probs->begin();
         itre = mp->probs->end();
         jtr = ti->pScaledDeltas->begin();
         jtre = ti->pScaledDeltas->end();
         while (itr != itre && jtr != jtre) {
            mp->weight += (*itr++)*(*jtr++);
         }
         mp->weight /= ts.s;  //!!! was in older code too.
         break;
       case IRfun::INVVARP:
         p0 = min(mp->probs->at(mp->indexHighestProbMove), IRutil::P0TOLERANCE);
         mp->weight = min(IRutil::INVVARP0CAP, 1.0/(p0*(1.0 - p0)));
         break;
       default:
         cout << "Using Unit Weights, hope that's OK." << endl;
         mp->weight = 1.0;
      }

      return mp;
   } 
 
 
   /** Gives the (p_t,r_t) pair for a particular move t, but note: it 
       returns only the Endpt for "p"---you need the caller to swap 
       the two fields around to create the Endpt object for "r". 
    */ 
   inline IRutil::Endpt getProbEndpt(const DecisionInfo* const di, 
                             const TrialSpec& ts, 
                             const vector<double>* depthWts,
                             const vector<double>* swingWts,
                             const MODEL model,
                             size_t equalTopIndexLimit = IRutil::NCUTOFF) { 
      //vector<double> probs = getProbs(di,ts,depthWts,swingWts,model); 
      MoveProjection* mp = getProjection(di,ts,depthWts,swingWts,model,equalTopIndexLimit);
      //int pmi = ti->useAltPlayedMoveIndex ? ti->playedMoveIndex 
                                          //: ti->source->altPlayedMoveIndex; 
      int pmi = di->getPlayedMoveIndex(); 
      double cume = 0.0; 
      int i = 0; 
      while (i < pmi) {    //!!! needs change now
         //cume += probs.at(i); 
         cume += mp->probs->at(i);
         i++; 
      } 
      //return IRutil::Endpt(cume,cume+probs.at(i),getDecWeight(di,ts,probs)); 
      IRutil::Endpt endpt(cume, cume+mp->probs->at(i), mp->weight);
      delete(mp);
      return endpt;
   } 

   inline IRutil::Endpt getProbEndpt(MoveProjection* mp, const DecisionInfo* const di) {
      double cume = 0.0;
      int i = 0;
      while (i < mp->indexPlayedMove) {   //!! still needs change using di
         cume += mp->probs->at(i);
         i++;
      }
      return IRutil::Endpt(cume, cume+mp->probs->at(i), mp->weight);
   }


   /** Patched probabilities given by empirical testing with over 600,000 moves. 
    */ 
   inline vector<double>* fudgePatch(const size_t numw, const double patchPower) { 
      vector<double>* patchWeights = new vector<double>(numw); 
      //const double EMPIRICALFREQ = 0.5806; 
      //const double ratio = EMPIRICALFREQ/(1.0 - EMPIRICALFREQ); 
      const double ratio = patchPower/(1.0 - patchPower);
      double sum = 0.0; 
      double item = 1.0; 
      for (size_t i = 0; i < numw; i++) { 
         patchWeights->at(numw - i - 1) = item; 
         sum += item; 
         item = item*ratio; 
         //sum = sum*ratio + 1.0; 
      } 
      double dbg = 0.0; 
      for (size_t k = 0; k < numw; k++) { 
         patchWeights->at(k) = IRutil::divifgt0(patchWeights->at(k), sum); 
         dbg += patchWeights->at(k); 
      } 
      if (fabs(1.0 - dbg) > IRutil::SUMPROBSTOLERANCE) {  
         cerr << "Bad fudge\n"; 
         IRutil::normalizeP(patchWeights);
      } 
      return patchWeights; 
   } 
 
   inline void patchProbs(vector<double>* probs, double patchPower) { 
      const size_t s = probs->size(); 
      size_t i = 0; 
      while (i < s-1) { 
      //while (i < 1) {      //do only for equal-top 
         size_t j = i+1; 
         while (j < s && probs->at(j) >= probs->at(i) - IRutil::MINPROB) { 
            j++; 
         } 
         size_t diff = j - i; 
         if (diff > 1) { 
            double tp = 0.0; 
            size_t k = 0; 
            for (k = i; k < j; k++) { tp += probs->at(k); } 
            vector<double>* patchWeights = fudgePatch(diff, patchPower); 
            for (k = i; k < j; k++) { 
               probs->at(k) = tp * patchWeights->at(k-i); 
            } 
            delete(patchWeights);
            i = j; 
         } else { 
            i++; 
         } 
      } 
      double dbg = 0.0; 
      for (size_t m = 0; m < probs->size(); m++) { dbg += probs->at(m); } 
      if (fabs(1.0 - dbg) > IRutil::SUMPROBSTOLERANCE) { 
         cerr << "Really bad fudge: sum " << dbg << endl; 
         //IRutil::normalize(probs);
      } 
      //return probs; 
   } 
 
};        //end of namespace MODELS 
 
#endif    //end of #ifndef __IR_MODELS_H__ 
 
 

