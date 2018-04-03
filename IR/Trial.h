//File----------------------Trial.h--------------------------------------- 
 
#ifndef __IR_TRIAL_H__ 
#define __IR_TRIAL_H__ 


#ifndef __IR_INCLUDES_H__
#include "IRincludes.h"
#endif

#ifndef __IR_UTIL_H__ 
#include "IRutil.h" 
#endif                      //end of #ifndef __IR_UTIL_H__ 
 
#ifndef __IR_FUNCTIONS_H__ 
#include "IRfunctions.h" 
#endif                      //end of #ifndef __IR_FUNCTIONS_H__ 
 
#ifndef __IR_MISC_GLOBAL_H__ 
#include "IRmiscGlobal.h" 
#endif	 //end of #ifndef __IR_MISC_GLOBAL_H__ 
 
#ifndef __IR_POSITION_H__
#include "Position.h"
#endif

#ifndef __IR_GAME_INFO_H__
#include "GameInfo.h"
#endif

#ifndef __IR_TRIAL_SPEC_H__ 
#include "TrialSpec.h" 
#endif	  //end of #ifndef __IR_TRIAL_SPEC_H__ 
 
#ifndef __IR_TURN_INFO_H__ 
#include "TurnInfo.h" 
#endif	  //end of #ifndef __IR_TURN_INFO_H__ 
 
#ifndef __IR_TUPLE_INFO_H__
#include "TupleInfo.h"
#endif 

#ifndef __IR_TURN_CONTEXT_H__
#include "TurnContext.h"
#endif

#ifndef __IR_FILTERS_H__ 
#include "Filters.h" 
#endif	  //end of #ifndef __IR_FILTERS_H__ 

#ifndef __IR_DELTA_SCALES_H__
#include "DeltaScales.h"
#endif    //end of #ifndef __IR_DELTA_SCALES_H__

 
#ifndef __IR_PERFDATA_H__ 
#include "PerfData.h" 
#endif	 //end of #ifndef __IR_PERFDATA_H__ 
 
#ifndef __IR_SHUFFLE_H__
#include "Shuffle.h"
#endif

#ifndef __IR_DECISION_INFO_H__
#include "DecisionInfo.h"
#endif
 
#ifndef __IR_MODELS_H__
#include "IRmodels.h"
#endif

#ifndef __IR_MOVESELECTION_H__
#include "MoveSelector.h"
#endif

#ifndef __IR_TRIAL_H__ 
#include "Trial.h" 
#endif	  //end of #ifndef __IR_TRIAL_H__ 
 
 
//----------------------STATISTICAL TRIALS----------------------------- 
 

 
/** Trial class works with a List<TurnInfo*>, and filters it into 
    a List<TupleInfo*>, on which statistical runs are performed. 
 */ 
 
class Trial : public Catalogable { 
   string name; 
   vector<vector<int> >* evalSwap;   //should be large enough for any turn.

   TrialSpec theSpec;                       //IMPT: own copy, settable here. 
   Models::MODEL model; 
   bool usePerceivedEvals;    //temp kludge: will allow setting from fit menu

   list<TurnInfo*>* pTurns;   //must be alias of Ensemble's turns
   list<TurnFilter*> filters; 
   MoveSelector* theSelector;
   vector<MoveSelector*>* theSelectors;
   bool scaleDeltas;
   DeltaScale* theScale;
   //Filters and scale determine next items
   vector<DecisionInfo*>* theDecisions;   //all use lo..hi from theScale
   vector<string> filteredGameIDs;
   //theSpec also determines:
   vector<double> poissonWeights;   //INV: has bounds 0..hi-z from theScale
   vector<double>* pwv;             //dilates Poisson weights according to v,d
                                    //and has bounds 0..hi-lo instead (IMPT)
   vector<double>* swv;             //Swing vector, bounds 0..userSwingHigh-userSwingLow
   bool fixDistortion; 
   bool treatMissAsMoveNN; 
   const SHUFFLES shuffle; 
   const SHUFFLES when; 
   const bool includeMoveNN; 
   const bool isReferenceTrial;
    
   IRfun::Dist dist; 
 
   //---------Private Methods----------------------------------------- 
 
   Trial(const Trial& arg);              //disabled copy constructor 
   Trial& operator=(const Trial& arg);   //disabled assignment 

   void setUPV(bool policy) { usePerceivedEvals = policy; }
 
   static bool deleteTurn(TurnInfo* const& tp) { delete(tp); return true; } 
   //void deleteAllTurns() { pTurns->remove_if(deleteTurn); } 
   void deleteAllTurns() {
      list<TurnInfo*>::iterator it = pTurns->begin();
      list<TurnInfo*>::iterator ite = pTurns->end();
      while (it != ite) {
         TurnInfo* tp = *it++;
         delete(tp);
      }
   }
      
 
   /** Modifies whichV, which is either the "pwv" or "swv" field above.
       REQ: whichV has /already/ been initialized and has size numWeights.
       This enables whichV to be accumulated in successive calls weighted
       by factor.  The accumulated sum is NOT multiplied by "factor"---so
       only the last value in a succession of calls should be tested versus 1.
    */
   double makeSum(size_t g, size_t relDepth, size_t numWeights, 
                  const vector<double>& accWts,
                  vector<double>* whichV, const double factor) {   
      double sum = 0.0;
      double prev = 0.0;
      double next;
      size_t i;
      //double relDepth = theSpec.d - theScale->getLowDepth() + 1;
      //size_t g = size_t(theSpec.d + 0.5);  //rounds
      size_t g0 = size_t(relDepth);
      size_t g1 = g0 + 1;
      double r1 = relDepth - g0;
      double r0 = 1.0 - r1;

      if (theSpec.v > 1.0/IRutil::CVTOLERANCE) {  //treat v as inf
         g = min(g, numWeights-1);
         g1 = min(g1, numWeights-1);  //upper bound is >= 0
         g0 = (g1 == 0 ? 0 : g1 - 1);
         whichV->at(g0) += factor*r0;
         if (g1 != g0) {
            whichV->at(g1) += factor*r1;
         }
         sum += 1.0;
      } else if (g >= numWeights) {  //ignore d in spec
         for (i = 0; i < numWeights; i++) {
            next = numWeights*pow(IRutil::divifgt0((i+1.0),numWeights), theSpec.v);
                                  //IRutil::divifgt0(1.0,theSpec.v));
            whichV->at(i) += factor*IRutil::fracsum(accWts,prev,next);
            prev = next;
            sum += whichV->at(i);  //relevant only in last of series of calls
         }
      } else {
         for (i = 0; i < g; i++) {
            next = (relDepth)*pow(IRutil::divifgt0((i+1.0), relDepth), theSpec.v);
                                  //IRutil::divifgt0(1.0, theSpec.v));
            whichV->at(i) += factor*IRutil::fracsum(accWts,prev,next);
            prev = next;
            sum += whichV->at(i);
         }
         next = pow(IRutil::divifgt0((numWeights - (g + 1.0)), (numWeights - relDepth)),
                    theSpec.v);
                    //IRutil::divifgt0(1.0,theSpec.v));
         next = numWeights - next*(numWeights - relDepth);
         whichV->at(g) += factor*IRutil::fracsum(accWts,prev,next);
         prev = next;
         sum += whichV->at(g);
         for (i = g+1; i < numWeights; i++) {
            next = pow(IRutil::divifgt0((numWeights - (i + 1.0)),(numWeights - relDepth)),
                       theSpec.v);
                       //IRutil::divifgt0(1.0,theSpec.v));
            next = numWeights - next*(numWeights - relDepth);
            whichV->at(i) += factor*IRutil::fracsum(accWts,prev,next);
            prev = next;
            sum += whichV->at(i);
         }
      }
      return sum;
   }

   void makeLWs(double dpeak, double slope, size_t numWeights, //caller ensures slope > 0
                  vector<double>* whichV, const double factor) {
      bool normalize = false;
      int m = numWeights - 1;  //indexed 0 .. m 
      size_t e = (slope < IRutil::CVTOLERANCE ? 0 
                  : max(0, int(1.0 - IRutil::MINGTZERO + dpeak - 1.0/slope)));
      size_t f = (slope < IRutil::CVTOLERANCE ? m : min(m, int(dpeak + 1.0/slope)));
      size_t b = size_t(dpeak);
      if (b > m) {
         normalize = true;
         b = m;
         if (e > b) { e = b; }
      }
      double sum = f - e + 1.0 + slope*dpeak*(f - 2.0*b + e - 1.0)
                     + slope*(b*(b + 1.0) - 0.5*(e*(e - 1.0) + f*(f + 1.0)));
      size_t i;
      if (whichV->size() == 1) {
         whichV->at(0) += 1.0;
         return;
      } //else
      for (i = e; i <= b; i++) {
         whichV->at(i) += (1.0 - slope*(dpeak - i))*factor/sum;
      }
      for (i = b+1; i <= f; i++) {
         whichV->at(i) += (1.0 - slope*(i - dpeak))*factor/sum;
      }
      if (normalize) { IRutil::normalizeP(whichV); }
   }



   void setLWs() {   //sets up linear weights for pwv and swv vectors.
      //size_t numLWs = 1 + theScale->getHighDepth() - theScale->getZeroDepth();
      size_t numLWs = 1 + theScale->getHighDepth() - theScale->getLowDepth();
      size_t numSWs = 1 + theScale->getSwingHigh() - theScale->getSwingLow();

      if (pwv) { delete(pwv); }
      if (swv) { delete(swv); }

      pwv = new vector<double>(numLWs, 0.0);
      double relZDepth = max(0.0, theSpec.d - theScale->getZeroDepth());
      makeLWs(relZDepth, theSpec.v, numLWs, pwv, 1.0);
      if (numSWs < 0.5) {
         swv = new vector<double>();
         return;
      } //else
      swv = new vector<double>(numSWs, 0.0);
      double relDepth = max(0.0, theSpec.d - theScale->getSwingLow());
      makeLWs(relDepth, theSpec.v, numSWs, swv, 1.0);
   }


   void setPWs() {   //must be called after any change to spec d or v

      size_t numPWs = 1 + theScale->getHighDepth() - theScale->getZeroDepth();
      poissonWeights.resize(numPWs);
      double relZDepth = max(0.0, theSpec.d - theScale->getZeroDepth());
      IRutil::genPoissonWeights(relZDepth, numPWs, poissonWeights);

      double sum = 0.0;
      for (size_t i = 0; i < numPWs; i++) {
         sum += poissonWeights.at(i);
      }
      if (fabs(1.0 - sum) > IRutil::SUMPROBSTOLERANCE) {
         cerr << "Bad normalization in poissonWeights vector: sum = " << sum << endl;
         //IRutil::normalize(poissonWeights);
      }

      size_t numWeights = 1 + theScale->getHighDepth() - theScale->getLowDepth();

      vector<double> accWts(numWeights,0.0);
      size_t ii = 0;
      size_t diff = numPWs - numWeights;
      for (ii = 0; ii < diff; ii++) { accWts.at(0) += poissonWeights.at(ii); }
      for (ii = diff; ii < numPWs; ii++) {
         accWts.at(ii - diff) += poissonWeights.at(ii);
      }

/*
cerr << "AccWts: ";
for (int z = 0; z < accWts.size(); z++) {
   cerr << z << "-" << IRutil::ftoa(accWts.at(z), 3) << " ";
}
cerr << endl;
*/

      //pwv->clear();
      //pwv->resize(numWeights,0.0);
      delete(pwv);
      pwv = new vector<double>(numWeights,0.0);
      double relDepth = max(0.0, theSpec.d - theScale->getLowDepth() + 1);
      size_t g0 = size_t(relDepth);
      size_t g1 = g0 + 1;
      double r1 = relDepth - g0;
      double r0 = 1.0 - r1;

      sum = makeSum(g0,relDepth,numWeights,accWts,pwv,r0);  //result discarded
      sum = makeSum(g1,relDepth,numWeights,accWts,pwv,r1);

      //cerr << "First sum: " << sum << endl;
      sum = 0.0;

//cerr << "PWV: " << strPWV() << endl;

      for (int k = 0; k < pwv->size(); k++) {
         sum += pwv->at(k);
         //cerr << k << "-" << IRutil::round(sum,3) << " ";
      }
      //cerr << endl;

      if (fabs(1.0 - sum) > IRutil::SUMPROBSTOLERANCE) {
         cerr << "Bad normalization in PWV vector: sum = " << sum << endl;
         cerr << string(theSpec) << endl;
         for (int m = 0; m < pwv->size(); m++) {
            cerr << m << "-" << IRutil::round(pwv->at(m),4) << " ";
         }
         cerr << endl;
         IRutil::normalizeP(pwv);
      }

      size_t numSWs = 1 + theScale->getSwingHigh() - theScale->getSwingLow();
      relDepth = max(0.0, theSpec.d - theScale->getSwingLow());
      if (numSWs < 0.5) { 
         delete(swv);
         swv = new vector<double>();
         return; 
      }  //else
       
      //poissonWeights.resize(numSWs);
      vector<double> poissonWeights2(numSWs,0.0);
//cerr << "PW2: ";
      IRutil::genPoissonWeights(relDepth, numSWs, poissonWeights2);

      sum = 0.0;
      for (size_t i = 0; i < numSWs; i++) {
         sum += poissonWeights2.at(i);
      }
      if (fabs(1.0 - sum) > IRutil::SUMPROBSTOLERANCE) {
         cerr << "Bad normalization in PW vector for SW: sum = " << sum << endl;
      }  //no need to differentiate accWts and poissonWeights now

      //swv->clear();
      //swv->resize(numSWs,0.0);
      delete(swv);
      swv = new vector<double>(numSWs,0.0);

      g0 = size_t(relDepth);
      g1 = g0 + 1;
      r1 = relDepth - g0;
      r0 = 1.0 - r1;

    
      sum = makeSum(g0,relDepth,numSWs,poissonWeights2,swv,r0);
      sum = makeSum(g1,relDepth,numSWs,poissonWeights2,swv,r1);

//cerr << "SWV: " << strSWV() << endl;

      if (fabs(1.0 - sum) > IRutil::SUMPROBSTOLERANCE) {
         cerr << "Bad normalization in SWV vector: sum = " << sum << endl;
         cerr << string(theSpec) << endl;
         IRutil::normalizeP(swv);
      }
      //delete(poissonWeights2);
   }
 
   bool filterOne(TurnInfo* turn) const { 
      for (list<TurnFilter*>::const_iterator it = filters.begin(); 
           it != filters.end(); it++) { 
         if (!(*it)->apply(turn)) { return false; } 
      } 
      return true; 
   } 
 
   /** Permutation generator, used by translateTurn 
       Assumes arr has correct size, but does not assume it's initialized. 
    */ 
   void makePermutation(vector<int>* arr, const TurnInfo* const pTurn) const { 
      static int tnum; 
      tnum++; 
      int psize = arr->size(); 
      int start; 
      int permEnd = psize; 
      int huffle = shuffle; 
      if (psize == 1 + IRutil::NCUTOFF && (!includeMoveNN)) { 
         permEnd--; 
         arr->at(permEnd) = permEnd; 
      } 
      switch(when) { 
       case APPLY_ALL_TURNS: 
         break; 
       case APPLY_ODD_TURNS: 
         if (tnum % 2 == 0) { return; } 
         break; 
       case APPLY_EVEN_TURNS: 
         if (tnum % 2 == 1) { return; } 
         break; 
       case APPLY_IF_RANDOM: 
         srand(pTurn->srandSeed); 
         if (IRutil::myRand(0,2) % 2 == 0) { return; } 
         break; 
       case APPLY_RANDOM_TWEAK: 
         srand(pTurn->srandSeed); 
         huffle = IRutil::myRand(0,NUM_TWEAKS); 
         break; 
       case FULLY_RANDOMIZE: 
         srand(pTurn->srandSeed); 
         IRutil::randShuffle(arr, 0, permEnd); 
         return;  //nothing more to do 
       default: 
         break; 
      } 
      switch(huffle) { 
       case NO_SHUFFLE: 
         for (int i = 0; i < permEnd; i++) { 
            arr->at(i) = i; 
         } 
         break; 
       case SWAP_FIRST_TWO: 
         if (permEnd >= 2) { 
            arr->at(0) = 1; 
            arr->at(1) = 0; 
         } 
         for (int i = 2; i < permEnd; i++) { 
            arr->at(i) = i; 
         } 
         break; 
       case FLIP: 
         for (int i = 0; i < permEnd; i++) { 
            arr->at(i) = psize - i - 1; 
         } 
         break; 
       case SWAP_HALVES: 
         start = (permEnd + 1) / 2;  // = 6 when permEnd = 11 
         for (int i = 0; i < permEnd; i++) { 
            arr->at(i) = (i + start) % permEnd; 
         } 
         break; 
       case CYCLE_LEFT: 
         for (int i = 0; i < permEnd - 1; i++) { 
            arr->at(i) = i + 1; 
         } 
         arr->at(permEnd - 1) = 0; 
         break; 
       case CYCLE_RIGHT: 
         arr->at(0) = permEnd - 1; 
         for (int i = 1; i < permEnd; i++) { 
            arr->at(i) = i - 1; 
         } 
         break; 
       case CYCLE_BY_TURN_NO: 
         for (int i = 0; i < permEnd; i++) { 
            arr->at(i) = (i + pTurn->turnNumber) % permEnd; 
         } 
         break; 
       case CYCLE_FLIP_TURN_NO: 
         for (int i = 0; i < permEnd; i++) { 
            arr->at(i) = (pTurn->turnNumber % 2 == 0) ? i : permEnd - i - 1; 
         } 
         for (int i = 0; i < permEnd; i ++) { 
            arr->at(i) = (arr->at(i) + (pTurn->turnNumber / 2)) % permEnd; 
         } 
         break; 
       default:  //do nothing 
         break; 
      } 
   } 
 
 public: 
   Trial(const string& gname, list<TurnInfo*>* gturns, TrialSpec spec, 
         Models::MODEL gmodel = Models::POWER_SHARES, bool fd = false, 
         bool adjustDeltas = false, 
         DeltaScale* ds = new UnitScale(false,
            new SwingInfo(map<APPLICATION_POLICIES, double>(), true, false, true,
                          false, false, 0, false, false, 0.0),
            NO_PATCH, IRutil::CENTRALPATCHPOWER, 0.00, map<string,double>(), 
            1, 30, 30, 6, 20, 0, IRutil::DELTACAP),
                                        //!!! todo: elim defaults on these params
         bool tmamNN = true,  
         SHUFFLES gshuffle = NO_SHUFFLE, SHUFFLES gwhen = APPLY_ALL_TURNS, 
         bool imNN = true, bool isRefTrial = false) 
      : theDecisions(new vector<DecisionInfo*>()), 
        filteredGameIDs(vector<string>()), theSpec(spec), 
        evalSwap(new vector<vector<int> >(IRutil::MAXDEPTH,
                 vector<int>(IRutil::MAXNUMLEGALMOVES,0))),
        model(gmodel), usePerceivedEvals(false),
        fixDistortion(fd), 
        dist(IRfun::leastSquares), 
        pTurns(gturns), filters(list<TurnFilter*>(0)),
        theSelector(new EqualTopMoveSelector()),
        theSelectors(new vector<MoveSelector*>()),
        name(gname), scaleDeltas(adjustDeltas), theScale(ds), 
        treatMissAsMoveNN(tmamNN), shuffle(gshuffle), when(gwhen), 
        includeMoveNN(imNN), isReferenceTrial(isRefTrial),
        pwv(NULL), swv(NULL) 
   {  
      reset();  //sets pwv and swv
   } 
 
   virtual ~Trial() { 
      clearFilteredDecisions();
      deleteAllTurns(); 
      //delete(pTurns);
      list<TurnFilter*>::iterator citr = filters.begin();
      list<TurnFilter*>::iterator cite = filters.end();
      while (citr != cite) {
         TurnFilter* tf = *citr++;
         delete(tf);
      }
      //delete(theScale);   //since new in all constructors
      vector<MoveSelector*>::iterator sitr = theSelectors->begin();
      vector<MoveSelector*>::iterator site = theSelectors->end();
      while (sitr != site) {
         MoveSelector* ms = *sitr++;
         delete(ms);
      }
      delete(theSelectors);
      delete(evalSwap);
      delete(pwv);
      delete(swv);
   } 
 
   virtual string getName() const { return name; }  //virtual for Catalogable 
 
   bool scalesDeltas() const { return scaleDeltas; } 
 
   void setDist(IRfun::Dist newDist) { 
      dist = newDist; 
   } 

   string strPWV() const {
      ostringstream oss;
      size_t offset = theScale->getLowDepth();
      for (size_t i = 0; i < pwv->size(); i++) {
         oss << (i+offset) << "-" << IRutil::ftoa(pwv->at(i), 3) << " ";
      }
      return oss.str();
   }

   string strSWV() const {
      ostringstream oss;
      size_t offset = theScale->getSwingLow();
      for (size_t i = 0; i < swv->size(); i++) {
         oss << (i+offset) << "-" << IRutil::ftoa(swv->at(i), 3) << " ";
      }
      return oss.str();
   }

   void setSelector(MoveSelector* selector) {
      theSelector = selector;
   }

   void addSelector(MoveSelector* selector) {
      theSelectors->push_back(selector);
   }

   void clearSelectors() {
      theSelectors->clear();
   }

   size_t numSelectors() const { return theSelectors->size(); }

   vector<MoveSelector*>* exportSelectors() { return theSelectors; }

   void setScale(DeltaScale* ds) {   //Should be followed by reset()
      scaleDeltas = true; 
      theScale = ds; 
cerr << "resetting after setScale..." << endl;
      reset(); 
cerr << "done." << endl;
   } 
   DeltaScale* getScale() const { 
      return theScale; 
   } 

   void setSpec(const TrialSpec& spec) { 
      theSpec = spec; 
      //setPWs();
      setLWs();
      size_t numArgs = (theScale->getSwingPolicy()->separates() ? 1 : 3);
      theSpec.memoize(numArgs);
   } 

   TrialSpec getSpec() const { 
      return theSpec; 
   } 
 
   void setS(double s) { 
      theSpec.s = s; 
      size_t numArgs = (theScale->getSwingPolicy()->separates() ? 1 : 3);
      theSpec.memoize(numArgs); 
   } 
   void setC(double c) { 
      theSpec.c = c; 
      size_t numArgs = (theScale->getSwingPolicy()->separates() ? 1 : 3);
      theSpec.memoize(numArgs); 
   }
   void setD(double d) { theSpec.d = d; setLWs(); } 
   void setV(double v) { theSpec.v = v; setLWs(); }
   void setA(double a) { theSpec.a = a; }
   void setHM(double hm) { theSpec.hm = hm; }
   void setHP(double hp) { theSpec.hp = hp; }
   void setB(double b) { theSpec.b = b; }

   void setParams(double s, double c, double d, double v, double a,
                  double hm, double hp, double b) {
      theSpec.s = s; 
      theSpec.c = c;
      theSpec.d = d;
      theSpec.v = v; 
      theSpec.a = a;
      theSpec.hm = hm;
      theSpec.hp = hp;
      theSpec.b = b;
      //setPWs();
      setLWs();
      size_t numArgs = (theScale->getSwingPolicy()->separates() ? 1 : 3);
      theSpec.memoize(numArgs);
   } 
   void setWhichParams(size_t paramsCode, size_t n, const double* params) {
      //paramsCode is (s,c,d,v,w,em,ep,a) from 00000000 to 11111111 = 255
      bool pwflag = false;
      bool memoflag = false;
      size_t paramUsed = 0;
      if ((paramsCode >> 7) % 2) {
         theSpec.s = params[paramUsed++];
         memoflag = true;
      }
      if ((paramsCode >> 6) % 2) {
         theSpec.c = params[paramUsed++];
         memoflag = true;
      }
      if ((paramsCode >> 5) % 2) {
         theSpec.d = params[paramUsed++];
         pwflag = true;
         memoflag = true;
      }
      if ((paramsCode >> 4) % 2) {
         theSpec.v = params[paramUsed++];
         pwflag = true;
      }
      if ((paramsCode >> 3) % 2) {
         theSpec.a = params[paramUsed++];
         //pwflag = true;
      }
      if ((paramsCode >> 2) % 2) {
         theSpec.hm = params[paramUsed++];
         //pwflag = true;
      }
      if ((paramsCode >> 1) % 2) {
         theSpec.hp = params[paramUsed++];
      }
      if (paramsCode % 2) {
         theSpec.b = params[paramUsed++];
         //pwflag = true;
      }

      if (pwflag) { setLWs(); }   //{ setPWs(); }
      if (memoflag) { 
         size_t numArgs = (theScale->getSwingPolicy()->separates() ? 1 : 3);
         theSpec.memoize(numArgs); 
      }
   }
         
      
   //void setDeltaCap(double newCap) { deltaCap = newCap; } 
 
   double getS() const { return theSpec.s; } 
   double getC() const { return theSpec.c; } 
   double getD() const { return theSpec.d; }
   double getV() const { return theSpec.v; }
   double getA() const { return theSpec.a; }
   double getHM() const { return theSpec.hm; }
   double getHP() const { return theSpec.hp; }
   double getB() const { return theSpec.b; }

   double getDeltaCap() { return theScale->getDeltaCap(); } //{ return deltaCap; } 
   TrialSpec getTrialSpec() const { return theSpec; } 
   //vector<DecisionInfo*>* getDecisions() const { return theDecisions; } 
   size_t numDecisions() const { return theDecisions->size(); }
   list<TurnInfo*>* getTurns() { return pTurns; } //!!! used only by Ensemble ctor
   Models::MODEL getModel() const { return model; } 
   void setModel(Models::MODEL newModel) { model = newModel; }
   bool getFixPolicy() const { return fixDistortion; } 
   void setFixPolicy(bool newFix) { fixDistortion = newFix; }
   bool getMoveNNPolicy() const { return treatMissAsMoveNN; }
   void setMoveNNPolicy(bool newNN) { treatMissAsMoveNN = newNN; }
 
   void attachFilter(TurnFilter* filter) { 
      filters.push_back(filter); 
      //this and detachFilter do not force a reset (yet)---caller should do. 
   } 
 
   void detachFilter(TurnFilter* filter) {  //does not delete filter itself 
      list<TurnFilter*>::iterator it = filters.begin(); 
      while (it != filters.end() && (*it) != filter) { it++; } 
      if (it != filters.end()) { 
         filters.erase(it); 
      } else { 
         cout << "Uncaught attempt to detach unattached filter." << endl; 
      } 
   } 
 
   void clearFilters() { 
      filters.clear(); 
      //filters = list<TurnFilter*>(); 
      reset(); 
   } 
 
   void clearFilteredDecisions() const { 
      vector<DecisionInfo*>::const_iterator it = theDecisions->begin();
      vector<DecisionInfo*>::const_iterator ite = theDecisions->end();
      while (it != ite) {
         DecisionInfo* dp = (*it++);
         delete(dp); 
      } 
      theDecisions->clear(); 
   } 

   void zapBootWeights() {
      int n = numDecisions();
      for (int i = 0; i < n; i++) {
         theDecisions->at(i)->setBootWeight(0.0);
      }
   }
   void resetBootWeights() {
      int n = numDecisions();
      for (int i = 0; i < n; i++) {
         theDecisions->at(i)->setBootWeight(1.0);
      }
   }

   void resample(size_t numItems, bool replace) {
      int n = numDecisions();
      if (replace) {
         zapBootWeights();
         for (int i = 0; i < numItems; i++) {
            int j = IRutil::myRand(0, n);
            theDecisions->at(j)->incBootWeight();
         }
      } else if (numItems > 0.9*double(n)) {
         cout << "Request of " << numItems << " close to " << n
              << " so returning full set." << endl;
         resetBootWeights();
         return;
      } else {
         zapBootWeights();
         int i = 0;
         while (i < numItems) {
            int j = IRutil::myRand(0, n);
            if (theDecisions->at(j)->getBootWeight() < IRutil::MINPROB) {
               theDecisions->at(j)->incBootWeight();
               i++;  //success
            }
         }
      }
   }

   void resampleGames(size_t numGames, bool replace, bool oneSide) {
      int n = numDecisions();
      if (n == 0) { 
         cout << "Please add filtered turns..." << endl;
         return;
      }
      resetGameIDs();
      size_t numFilteredGames = filteredGameIDs.size();
      if (numGames > 0.9*numFilteredGames && (!replace)) {
         cout << "Request of " << numGames << " close to " << numFilteredGames
              << " so returning full set." << endl;
         resetBootWeights();
         return;
      } else {
         zapBootWeights();
         vector<double> gameBootWts(numFilteredGames,0.0);
         int i = 0;
         while (i < numGames) {
            int j = IRutil::myRand(0, numFilteredGames);
            if (gameBootWts.at(j) < IRutil::MINPROB || replace) {
               gameBootWts.at(j) += 1.0;  //no weights for games?
               i++;  //success'
            }
         }
         int n = numDecisions();
         int currGame = 0;
         int currTurn = 0;
         while (currGame < numFilteredGames) {
            if (gameBootWts.at(currGame) >= 1.0) {
               string currGameID = filteredGameIDs.at(currGame);
               string gameID = theDecisions->at(currTurn)->pParent->pGameInfo->gameID;
               while (currTurn < n-1 && gameID != currGameID) {
                  currTurn++;
                  gameID = theDecisions->at(currTurn)->pParent->pGameInfo->gameID;
               }
               bool whichSide = (IRutil::myRand(0,2) == 0);
               while (currTurn < n-1 && gameID == currGameID) {
                  if ((!oneSide) || (whichSide == theDecisions->at(currTurn)->pParent->whiteToMove)) {
                     theDecisions->at(currTurn)->incBootWeight();
                  }
                  currTurn++;
                  gameID = theDecisions->at(currTurn)->pParent->pGameInfo->gameID;
               }
            }
            currGame++;
         }
      }
   }
                  

      


 
   /** Only (non-debugging) place in the program where a DecisionInfo is 
       constructed, and only place where deltas can be adjusted. 
       Note that scaleDeltas is usually assumed to be true. 
        
       In the background is a permutation whose size "psize" is the same as that
       of the  "probs" array that will be computed by the TupleInfo's methods.   
       () if dsize (= pTurn->deltas->size) == NCUTOFF && fixDistortion, 
          then psize = dsize, else psize = NCUTOFF+1. 
       () if (! randomize) then the permutation is the identity 
       () if (randomize && !includeMoveNN && dsize == NCUTOFF) the permutation  
          randomizes 0..NCUTOFF-1 but fixes NCUTOFF, 
       () else, it randomizes 0..psize-1. 
    */ 
   DecisionInfo* translateTurn(const TurnInfo* const pTurn) const { 
      bool pp = (theScale->getPatchPolicy() == MATCH_PROBS); 
      bool ua = (theScale->getPatchPolicy() == BALANCE_TIES); 
      return new DecisionInfo(pTurn,scaleDeltas,theScale,pp,ua,evalSwap);
   } 
 
   void addFilteredTuples() const { 
       //static counters
       IRutil::numResets++;

       for (list<TurnInfo*>::const_iterator tu = pTurns->begin(); 
               tu != pTurns->end(); tu++) { 
           TurnInfo* ti = *tu;
           if (filterOne(ti)) { 
               IRutil::DIcount++;
               DecisionInfo* di = translateTurn(ti);

               theDecisions->push_back(di);
           }
       } 
   }  

   void resetGameIDs() {
      filteredGameIDs.clear();
      string lastGameID = "";
      vector<DecisionInfo*>::const_iterator dit = theDecisions->begin();
      vector<DecisionInfo*>::const_iterator dite = theDecisions->end();
      while (dit != dite) {
         DecisionInfo* dec = *dit++;
         string gameID = dec->pParent->pGameInfo->gameID;
         if (gameID != lastGameID) {
            filteredGameIDs.push_back(gameID);
            lastGameID = gameID;
         }
      }
   }
 
   bool isReady() const { return (!theDecisions->empty()); } 

   void reset(bool isRefTrial = false, bool onExit = false) { 
      clearFilteredDecisions(); 
      //setPWs();
      setLWs();
      if (!onExit) { addFilteredTuples(); }
   } 
 
   string speakFit(const double score, const vector<double>& up, 
                   const vector<double>& scores, const double totalWeight, 
                   const int tupleCount) const; 
   string speakFitNew(const double score,const double result2,const double totalWeight, 
                   const int tupleCount) const; 
 
   double percFit(vector<ostream*>* outs = new vector<ostream*>()); 
 
   double percFitOld(vector<ostream*>* outs = new vector<ostream*>()); 
   double percFitOldL1(vector<ostream*>* outs = new vector<ostream*>()); 
 
   /** Test expected number of first-line matches and top-matches for the 
       moves in "theDecisions", based on the curve specification in "theSpec". 
       Uses spec's weighting only if third argument is "false". 
       Assumes theDecisions have already been filtered. 
       Also reports actual matches *to the MLM data used to compute shares*, 
       but this might not be relevant to tests with another engine. 
       perfTest(uw,tn,suw,sun,numIndices,silent,dump);

       The judgment depth is taken from the TrialSpec and so is not a
       parameter of this method.
    */ 

   PerfData perfTest(bool unitWeights = false, bool tmaNN = true,
                     bool showUnitWeights = false, bool showUnscaled = true,
                     size_t numIndices = 15, size_t equalTopIndexLimit = IRutil::NCUTOFF,
                     bool silent = true, string dump = "", size_t movesLimit = 1000) const; 
 
   /** Return sum_t log(1/p_t), where p_t is the probability of the played 
       move at turn t---as estimated from *this* spec on *these* tuples. 
       Used by "Minimizer" class for Max-Likelihood fitting. 
    */ 
   double logSumPlayedMoves() const; 

   SimpleStats perfTestSimple(size_t numIndices, size_t numSelectors,
                              vector<MoveSelector*>* mySelectors, size_t equalValueIndexLimit,
                              bool unitWeights = false, bool tmaNN = true, bool giveFreq = true) const;


   double firstL1a() { 
      setDist(IRfun::L1a); 
      double value = percFit(NULL); 
      PerfData pf = perfTest(false, fixDistortion); 
      setDist(IRfun::leastSquares); 
      return 0.5*(IRutil::sqr(value/10000.0) 
            + IRutil::sqr(pf.moveMatchWtd.measuredValue - pf.moveMatchWtd.projectedValue)); 
   } 
 
   double topL1a() { 
      setDist(IRfun::L1a); 
      double value = percFit(NULL); 
      PerfData pf = perfTest(false, fixDistortion); 
      setDist(IRfun::leastSquares); 
      return 0.5*(IRutil::sqr(value/10000.0) 
                  + IRutil::sqr(pf.equalValueMatchWtd.measuredValue 
                                - pf.equalValueMatchWtd.projectedValue)); 
   } 
 
   double probabilityBins(); 
 
   string getModelName() const { 
      string modelKind; 
      switch(model) { 
       case Models::SHARES: 
         modelKind = "Shares"; 
         break; 
       case Models::POWER_SHARES: 
         modelKind = "PowerShares"; 
         break; 
       default: 
         modelKind = "Kind?"; 
         break; 
      } 
      return modelKind; 
   } 
 
   virtual string getDescription() const { 
      const int c = pTurns->size(); 
      string res = getModelName() + " trial, " //+ IRutil::itoa(numScores) 
         //+ " pts. in " + IRutil::ftoa(percentilePoints->at(0)) + ".." 
         //+ IRutil::ftoa(percentilePoints->at(numScores-1)) + ",\n      " 
         + IRutil::itoa(c) + "-->" + IRutil::itoa(theDecisions->size())  
         + " turns, " + IRutil::itoa(filters.size()) + " filters, " 
         + (scalesDeltas() ? "modified" : "unit") + " delta scale, cap " 
         + IRutil::ftoa(getScale()->getDeltaCap(),2); 
      //res += "\nSpec: " + string(theSpec); 
      return res; 
   } 
 
   virtual operator string() const { 
      const int c = pTurns->size(); 
      string res = getModelName() + " trial " + getName() + ": " 
         + IRutil::itoa(c) + " turns, " 
         + IRutil::itoa(theDecisions->size()) + " filtered by "  
         + IRutil::itoa(filters.size()) + " filters; delta cap " 
         //+ IRutil::ftoa(deltaCap,2) + "\n"  
         + IRutil::ftoa(getScale()->getDeltaCap(),2) + "\n"
         + "Spec " + string(theSpec) + ";\n" ; 
/*
      if (numScores > 0) { 
         res += "Percentile range " + IRutil::ftoa(percentilePoints->at(0))  
             + ".." + IRutil::ftoa(percentilePoints->at(numScores-1))  
             + ", " + IRutil::itoa(numScores) + " points, "; 
      } 
*/
      string pre = (theScale->getPatchPolicy() == MATCH_PROBS) ? 
             "patching probabilities" : "using raw probabilities"; 
      if (fixDistortion) { pre += ", fixing distortion"; } 
      //res += pre + ", delta cap " + IRutil::ftoa(deltaCap,2) + ";\n"; 
      if (scaleDeltas) { 
         res += string(*theScale) + "\n"; 
      }
      res += "Filters:\n"; 
      for (list<TurnFilter*>::const_iterator it = filters.begin(); 
           it != filters.end(); it++) { 
         res += string(**it) + "\n"; 
      } 
      return res;
   }

   virtual string saySelectors() const {
      string res = "Selector(s):\n";
      for (int m = 0; m < theSelectors->size(); m++) {
         res += theSelectors->at(m)->getName() + ": " + theSelectors->at(m)->getDescription() + "\n";
      }
      //res += theSelector->getName() + ": " + theSelector->getDescription() + "\n";
      return res; 
   } 
 
   string tuplesToString() const { 
      string res = ""; 
      double scaledFalloff = 0.0; 
      double cappedFalloff = 0.0; 
      double uncappedFalloff = 0.0; 
      int i = 1; 

      return res + "Total falloff: scaled " + IRutil::ftoa(scaledFalloff,2) 
                 + ", capped " + IRutil::ftoa(cappedFalloff,2) 
                 + ", uncapped " + IRutil::ftoa(uncappedFalloff,2) + "\n"; 
   } 


   string turnsToString() const {
      string res = "";
      int i = 1;
      for (list<TurnInfo*>::const_iterator it = pTurns->begin();
           it != pTurns->end(); it++) {
         res += IRutil::itoa(i) + ". " + (*it)->gameID + "\n"
             + string(**it) + "\n\n";
         i++;
      }
      return res;
   }

   string decisionsToString() const {
      string res = "";
      int i = 1;
      for (vector<DecisionInfo*>::const_iterator it = theDecisions->begin();
           it != theDecisions->end(); it++) {
         res += IRutil::itoa(i) + ". ";
         res += string((*it)->pParent->gameID) + "\n";
         res += string(**it) + "\n\n";
         i++;
      }
      return res;
   }

   void dumpDeltas(const string& name) const {  //for debugging
      ofstream* file = new ofstream(name.c_str(), ios::app);
      if(!file->is_open()) {
         cerr << "Cannot stat the output file, returning..." << endl;
         return;
      } //else
      for(vector<DecisionInfo*>::const_iterator tu = theDecisions->begin();
             tu != theDecisions->end(); tu++) {
         (*file) << (*tu)->operator string() << endl;
      }
   }


 
}; //end of class Trial. 
 
 
#endif    //end of #ifndef __IR_TRIAL_H__ 
 
 
 
