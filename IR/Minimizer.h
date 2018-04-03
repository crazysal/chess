//File-------------------Minimizer.h----------------------------------- 
 
#include <algorithm>

#ifndef __IR_MINIMIZER_H__ 
#define __IR_MINIMIZER_H__ 

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
#include "TurnInfo.h" 
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
 
#ifndef __IR_TRIAL_FILTERS_H__ 
#include "TrialFilters.h" 
#endif 

#ifndef __IR_MULTIMIN_H__
#include "multimin.h"
#endif

#ifndef __IR_MPFIT_H__
#include "mpfit.h"
#endif
 
 
/** A "Swiss Army Knife" front end to multiple minimization packages and allowing:
    () user-weighted combinations of multiple objective functions f_j(X) to minimize;
    () holding any subset of eight total variables constant while fitting the n others;
    () bounded parameter domains (for packages that support them---not imposed here);
    () uniform control over search precision (x, y, and g for "geometry") and step parameters.
    For those components f_j(X) of the form "projected test j value minus actual value in data",
    the squared difference is divided by the (projected) variance to make a squared z-score,
    thus normalizing the units of all the tests, and the sum of these is minimized.  
    (As a back-door, entering a negative weight w_j for f_j gives |w_j|*the raw squared difference.)

    Also implements the relaxation technique of Murrag and Ng (2010) with switch mumul>0:

       minimize f(X) + mumul*g(X)

    where g(X) is a strongly convex function.  User gives initial value mu for mumul and
    a dividor d, then the procedure in Murray-Ng iterates until mumul/d^k < the x precision, 
    whereupon a final pass with mumul=0 occurs.  Initializing mumul=0 bypasses this process.

    Updates to parameters being fitted are managed as a /side effect/ on the global TrialSpec 
    (which is held in the class Trial "callback pointer" held by the Minimizer object).  
    If so needed, updates are copied to/from the fitted-parameter array (typically called "xs")
    passed in the package invocation function (variously called "f" or "fcn" or "Ef" in docs).
    Some packages also desire the f_j(X) values to be held separately so as to form their own
    internal weighted sum-of-squares (and also enforce m == n or m <= n); the mutable "testVals" 
    array in Minimizer itself does this.  Other Minimizer fields satisfy package requirements.

    Because the packages are mostly C code, the passed-in f/fcn/Ef/etc. function must be "static".
    Happily those packages give f a void* environment parameter, which is filled by the owning
    Minimizer object as "this" and cast to "(Minimizer*) miniMe" within f.  This allows invoking
    miniMe->minimand() which does the actual computation of f(X) as configured by the user.
    The body of f/fcn/Ef also updates the TrialSpec and adds mumul*g(X) to f(X) if mumul > 0.
    Updates to mumul are managed by the user-invoked minimize(...) method, which calls the
    workhorse method minimize1(...) for each "epoch" of minimization.  The package choice is
    effected in the body of minimize1 rather than minimize using top-level if/else statements.

    Because the actual function f(X) being minimized is static, it can be exported for use by
    other packages.  In fact, all package routines have been implemented this way.  To add another:
    () Add enumeration constant(s) for the new method(s) to the METHOD enum.
    () Make a new if-else block (or switch block) for each of the new methods in minimize1.
    () If "f", "fcn", or "Ef" does not already match the prototype needed by the routine, code it
       with the needed prototype and an analogous body.
    () Form structs either in the class or in the body of minimize1 as needed to invoke the routine.
    () If calling the new routine(s) from within the IR program, add entries for them to the
       methodMenu object built in the initMenus() routine in class Ensemble.  It may-or-may-not
       be necessary also to add code to the Ensemble::makeMinimizer(...) body.
       [Of course, CSE712 people wishing to do this can get help from me.  If the package's own 
       interface is clear, the process is really quick.]
 */

class Minimizer { 
 public: 
   enum METHOD {
      VWALK, GSL_NM4, GSL_NM6, GSL_NM7, GSL_SIM_ANN, 
      CMINPACK_LMDIF, CMINPACK_LMDIF1, CMINPACK_HYBRD, CMINPACK_HYBRD1, 
      MPFIT_LMDIF, NUM_METHODS
   };
   enum SKED {
      UNIT_SKED, SQRT_SKED, LIN_SKED, QUAD_SKED, INVVAR_SKED, 
      SQRT_ALLWT, LIN_ALLWT, QUAD_ALLWT, NUM_SKEDS
   };
   enum MINIMAND { 
      PERCFIT, ML, BINS,  //PERCFITOLD, PERCFITOLDL1, 
         FALLOFF, FIRST_LINE, SECOND_LINE, THIRD_LINE, ETV, //FOURTH_LINE, FIFTH_LINE,
         INDEX_FIT, INDEX_DIFF, ERROR1, ERROR2,
         //INDEXFITMASS, INDEXDIFFMASS, INDEXFITINVVAR, INDEXFITPROP,
         //FIRSTL1A, ETVL1A, 
         NUM_MINIMANDS
   }; 
 private: 
   const double STOLERANCE; 
   const double CTOLERANCE; 
   typedef double (Minimizer::*Monic)(const double) const; //used only for vWalk
   //typedef double (Trial::*TrialFn)(); 
 
//-------------------------//
   Trial* const theTrial;            //modifying theTrial->theSpec is the main side effect
   const METHOD method;
   size_t numActiveTests;            // = "m" in MINPACK routines
   map<MINIMAND, double> minibook;   //set of tests we are minimizing, with a weight for each
//-------------------------//

   //custom features of some tests
   size_t equalTopIndexLimit;
   const int indexFitStart;     //0-based move index
   const int indexFitEnd;
   const SKED indexFitSked;
   const int indexDiffStart;
   const int indexDiffEnd;
   const SKED indexDiffSked;
   const double error1Low;
   const double error1Hi;
   const SKED error1Sked;
   const double error2Low;
   const double error2Hi;
   const SKED error2Sked;
   vector<MoveSelector*>* mySelectors;

//--------------------------//
   size_t numIndices;
   double mumul;               //"funnel factor" for smoothing, = mu in Miller-Ng paper
   double mudiv;               //amount to divide mumul by in each "epoch"

   size_t whichParams;         //code (s,c,d,v,w,em,ep,a) in binary from 00000000 to 11111111
   const TrialSpec lowSpec;    //used to keep parameter bounds used in fitting
   const TrialSpec hiSpec;     //or in the logarithmic "funnel function".
   TrialSpec pivotSpec;        //map e.g. [lowS,pivotS,hiS] to [0,1/2,1]
//-------------------------//

   //const size_t mjudgmentDepth;
   unsigned int maxIter;  //not size_t to avoid compatibility warning with GSL
   size_t numRetries;
   mutable size_t numCalls;  //temp variable
   mutable size_t callsPerDisplay;   //ditto
   const double xprec; 
   const double yprec;
   const double geomprec;
   const bool walkOut; 
   const double walkOutStep; 
   const size_t bootNum;
   const bool keepSpecInBoot;
   bool verbose; 
   bool finished;
   vector<ostream*>* outs; 

   //Deprecated
   const bool geometricMeans;
   const bool sFirst;

   //CMINPACK quantities
   double epsfcn;           //machine-precision limit constant
   int* nfev;               //actual number of iterations, output variable
   double* testVals;        // "fvec" in MINPACK routines, size m = numActiveTests
   mutable string testValStr;
   mutable map<MINIMAND,string> testNames;

   //Temporary storage used by CMINPACK
   double* diag;            // "diag" in MINPACK routines, size n = numParamsUsed
   int* paramSwap;          // swap used for params in MINPACK routines
   double* lmdifSwap;       // extra swap for "lmdif" routine
   double* fjac;            // simulated Jacobian by CMINPACK lmdif
   double* qtf;
   double* wa1;
   double* wa2;
   double* wa3;
   double* wa4;
   double* hwa;



 public: 
   Minimizer(Trial* trial, METHOD gmethod, map<MINIMAND,double> gmini, size_t etvlimit,
             size_t ifs, size_t ife, SKED ifsked, size_t ids, size_t ide, SKED idsked,
             double e1lo, double e1hi, SKED e1sked, double e2lo, double e2hi, SKED e2sked,
             double mu, double mud, //bool upv,
             const TrialSpec loSpec, const TrialSpec hSpec, const TrialSpec pivSpec,
             unsigned int max_iter, size_t num_retries,
             double precx, double precy, double precg, 
             bool wout = true, double woutstep = 0.02, double epsfn = 1e-08,
             double bn = 0, bool ksib = false, bool vb = false,
             vector<ostream*>* gouts = new vector<ostream*>()) 
      : STOLERANCE(0.00001), CTOLERANCE(0.01), theTrial(trial), method(gmethod), 
        numActiveTests(gmini.size()), minibook(gmini), equalTopIndexLimit(etvlimit),
        indexFitStart(ifs), indexFitEnd(ife), indexFitSked(ifsked),
        indexDiffStart(ids), indexDiffEnd(ide), indexDiffSked(idsked),
        error1Low(e1lo), error1Hi(e1hi), error1Sked(e1sked),
        error2Low(e2lo), error2Hi(e2hi), error2Sked(e2sked),
        mySelectors(new vector<MoveSelector*>()),
        numIndices(max(indexFitEnd,max(indexDiffEnd,5))),
        mumul(mu), mudiv(mud), whichParams(255), 
        lowSpec(loSpec), hiSpec(hSpec), pivotSpec(pivSpec),
        maxIter(max_iter), numRetries(num_retries), numCalls(0), callsPerDisplay(50),
        xprec(precx), yprec(precy), geomprec(precg),
        walkOut(wout), walkOutStep(woutstep), bootNum(bn), keepSpecInBoot(ksib),
        verbose(vb), finished(false), outs(gouts), geometricMeans(false), sFirst(true),
        epsfcn(epsfn), nfev(new int(0)), testValStr(""), testNames(map<MINIMAND,string>())
   { 
      const size_t nat = gmini.size();
      testVals = new double[nat];
      diag = new double[8];
      paramSwap = new int[8];
      lmdifSwap = new double[8*nat + 40 + nat]; //mn+5n+m
      fjac = new double[8*nat];
      qtf = new double[8];
      wa1 = new double[8];
      wa2 = new double[8];
      wa3 = new double[8];
      wa4 = new double[nat];
      hwa = new double[74];
      
      for (int i = 0; i < 8; i++) { diag[i] = 1.0; }
      size_t jd = theTrial->getScale()->getJudgmentDepth();
      map<MINIMAND,double>::const_iterator mapite = gmini.end();
      if (gmini.find(ERROR1) != mapite && e1hi >= e1lo) {
          MoveRawDeltaSelector* mds1lo = new MoveRawDeltaSelector(GEQ,e1lo,jd,false,"mds1lo");
          MoveRawDeltaSelector* mds1hi = new MoveRawDeltaSelector(LEQ,e1hi,jd,false,"mds1hi");
          AndSelector* as1 = new AndSelector(mds1lo,mds1hi,"as1",false);
          mySelectors->push_back(as1);
      }
      if (gmini.find(ERROR2) != mapite && e2hi >= e2lo) {
          MoveRawDeltaSelector* mds2lo = new MoveRawDeltaSelector(GEQ,e2lo,jd,false,"mds2lo");
          MoveRawDeltaSelector* mds2hi = new MoveRawDeltaSelector(LEQ,e2hi,jd,false,"mds2hi");
          AndSelector* as2 = new AndSelector(mds2lo,mds2hi,"as2",false);
          mySelectors->push_back(as2);
      }
      
      if (trial->getS() < STOLERANCE) { 
         cout << "Adjusting s = 0 to " << STOLERANCE 
              << ", hope that's OK." << endl; 
         theTrial->setS(STOLERANCE); 
      } 
      if (trial->getC() < CTOLERANCE && geometricMeans) { 
         cout << "Adjusting c to " << CTOLERANCE 
              << " for geometric means." << endl; 
         theTrial->setC(CTOLERANCE); 
      } 
      testNames[PERCFIT] = "PercFit";
      testNames[ML] = "MaxLikely";
      testNames[BINS] = "Bins";
      testNames[FALLOFF] = "ASD";
      testNames[FIRST_LINE] = "Line1";
      testNames[SECOND_LINE] = "Line2";
      testNames[THIRD_LINE] = "Line3";
      testNames[ETV] = "ETV[" + IRutil::itoa(equalTopIndexLimit) + "]";
      testNames[INDEX_FIT] = "IndexFit["+IRutil::itoa(ifs)+","+IRutil::itoa(ife)+"]";
      testNames[INDEX_DIFF] = "IndexDiff["+IRutil::itoa(ids)+","+IRutil::itoa(ide)+"]";
      testNames[ERROR1] = "Error["+IRutil::ftoa(e1lo,2)+","+IRutil::ftoa(e1hi,2)+"]";
      testNames[ERROR2] = "Error["+IRutil::ftoa(e2lo,2)+","+IRutil::ftoa(e2hi,2)+"]";
   } 

   virtual ~Minimizer() {
      for (size_t i = 0; i < mySelectors->size(); i++) {
         delete(mySelectors->at(i));
      }
      delete(mySelectors);
      delete(nfev);
      delete(testVals);
      delete[](diag);
      delete[](paramSwap);
      delete[](lmdifSwap);
      delete[](hwa);
      delete[](wa1);
      delete[](wa2);
      delete[](wa3);
      delete[](wa4);
    } 
 
   inline bool isSFirst() const { return sFirst; } 
   inline TrialSpec getSpec() const { return theTrial->getSpec(); } 
   inline void setSpec(const TrialSpec& ts) { theTrial->setSpec(ts); }
   inline TrialSpec getLowSpec() const { return lowSpec; }
   inline TrialSpec getPivotSpec() const { return pivotSpec; }
   inline TrialSpec getHighSpec() const { return hiSpec; }
   inline METHOD getMethod() const { return method; }
   inline map<MINIMAND,double> getMinibook() const { return minibook; }
   inline size_t getMaxIter() const { return maxIter; }
   inline size_t getBootNum() const { return bootNum; }
   inline void setSilent() { verbose = false; }
   inline void setVerbose() { verbose = true; }
   inline bool isFinished() const { return finished; }
   inline string getTestValStr() const { return testValStr; }

   inline static double skedVal(Minimizer::SKED sked, double projTally, double projVar, double allWt) {
      switch(sked) {
       case UNIT_SKED:
         return 1.0;
       case SQRT_SKED:
         return sqrt(sqrt(projTally));
       case LIN_SKED:
         return sqrt(projTally);
       case QUAD_SKED:
         return projTally;
       case INVVAR_SKED:
         //sked = 1.0/sqrt(ss.indexMatchesWtd.at(i));
         return 1.0/sqrt(projVar);
       case SQRT_ALLWT:
         return sqrt(sqrt(allWt));
       case LIN_ALLWT:
         return sqrt(allWt);
       case QUAD_ALLWT:
         return allWt;
       default:
         return 1.0;
      }
   }

 
   /** The quantity being minimized---a weighted combination of test statistics.
       Does NOT include the "g" part of funneling f(X) + mu*g(X).
    */
   inline double minimand(bool vb = true) const {   //side effect: writes to testVals and testValStr
      vb = verbose;
      double result = 0.0; 
      size_t onTest = 0;
      size_t ilen, sel;
      double denom = IRutil::MINGTZERO;
      double sked, indexCume, falloffDiffSq, zFalloffSq, evDiffSq, zEVsq, idiff, idiffSq, zi, zisq, zir;
      size_t numIndices = 3;  //always do first-second-third line

      map<MINIMAND,double>::const_iterator mapite = minibook.end();
      if (minibook.find(INDEX_DIFF) != mapite && indexDiffEnd >= numIndices) { numIndices = indexDiffEnd+1; }
      if (minibook.find(INDEX_FIT) != mapite && indexFitEnd >= numIndices) { numIndices = indexFitEnd+1; }

      size_t numSelectors = mySelectors->size();
      testValStr = "";

      //This line evaluates the main test statistics; rest of the code applies only those selected.
      SimpleStats ss = theTrial->perfTestSimple(numIndices,numSelectors,mySelectors,equalTopIndexLimit,
                                                false,true,false);   //gives absolute numbers

      map<MINIMAND,double>::const_iterator citr = minibook.begin();
      while (citr != mapite) {
         MINIMAND md = (*citr).first;
         double mul = (*citr).second;
         denom += fabs(mul);
         double testVal = 0.0;
         //double gadd = (mumul > 0 ? mumul*g() : 0.0);

         switch(md) { 

          case PERCFIT: 
            testVal = mul * theTrial->percFit(new vector<ostream*>()); 
            break; 

          case ML: 
            testVal = mul * theTrial->logSumPlayedMoves(); 
            break; 

          case BINS:
            testVal = mul * theTrial->probabilityBins();
            break;

          case FALLOFF:
            falloffDiffSq = IRutil::sqr(ss.scaledFalloffWtd - ss.scaledFalloffProjWtd);
            if (mul < 0.0) {
               testVal = -mul * falloffDiffSq / IRutil::sqr(ss.accumWeight);
            } else {
               zFalloffSq = falloffDiffSq / ss.scaledFalloffVarianceProjWtd;
               testVal = mul * zFalloffSq;
            }
            break;

          case FIRST_LINE:
            idiffSq = IRutil::sqr(ss.indexMatchesWtd.at(0) - ss.indexMatchesProjWtd.at(0));
            if (mul < 0.0) {
               testVal = -mul * idiffSq / IRutil::sqr(ss.accumWeight);
            } else {
               zisq = (idiffSq / ss.indexMatchesVarianceProjWtd.at(0));
               testVal = mul * zisq;
            }
            break;

          case SECOND_LINE:
            idiffSq = IRutil::sqr(ss.indexMatchesWtd.at(1) - ss.indexMatchesProjWtd.at(1));
            if (mul < 0.0) {
               testVal = -mul * idiffSq / IRutil::sqr(ss.accumWeight);
            } else {
               zisq = (idiffSq / ss.indexMatchesVarianceProjWtd.at(1));
               testVal = mul * zisq;
            }
            break;

          case THIRD_LINE:
            idiffSq = IRutil::sqr(ss.indexMatchesWtd.at(2) - ss.indexMatchesProjWtd.at(2));
            if (mul < 0.0) {
               testVal = -mul * idiffSq / IRutil::sqr(ss.accumWeight);
            } else {
               zisq = (idiffSq / ss.indexMatchesVarianceProjWtd.at(2));
               testVal = mul * zisq;
            }
            break;

          case ETV:
            evDiffSq = IRutil::sqr(ss.equalValueMatchWtd - ss.equalValueMatchProjWtd);
            if (mul < 0.0) {
               testVal = -mul * evDiffSq / IRutil::sqr(ss.accumWeight);
            } else {
               zEVsq = evDiffSq / ss.equalValueMatchVarianceProjWtd;
               testVal = mul * zEVsq;
            }
            break;

          case INDEX_FIT:
            zir = 0.0;
            indexCume = 0.0;
            for (size_t i = indexFitStart; i <= indexFitEnd; i++) {
               sked = skedVal(indexFitSked, ss.indexMatchesProjWtd.at(i),
                              ss.indexMatchesVarianceProjWtd.at(i), ss.accumWeight);
               idiff = (ss.indexMatchesWtd.at(i) - ss.indexMatchesProjWtd.at(i));  // not /ss.accumWeight;
               idiffSq = 10000.0*IRutil::sqr(idiff/ss.accumWeight);
               if (mul < 0.0) {    //use raw index stats
                  indexCume += sked*idiffSq;
               } else {
                  zisq = (idiffSq / ss.indexMatchesVarianceProjWtd.at(i));
                  zir += zisq;
                  indexCume += sked*zisq;
               }
            }
            if (mul < 0.0) {
               testVal = -mul*indexCume/(indexFitEnd - indexFitStart + 1);
            } else {
               testVal = mul*zir/(indexFitEnd - indexFitStart + 1);
            }
            break;

          case INDEX_DIFF:
            zir = 0.0;
            indexCume = 0.0;
            idiff = 0.0;
            for (size_t i = indexDiffStart; i <= indexDiffEnd; i++) {
               sked = skedVal(indexDiffSked, ss.indexMatchesProjWtd.at(i),
                              ss.indexMatchesVarianceProjWtd.at(i), ss.accumWeight);
               idiff += 100.0*(ss.indexMatchesWtd.at(i) - ss.indexMatchesProjWtd.at(i)); 
               if (mul < 0.0) {    //use raw index stats
                  indexCume += sked*idiff/ss.accumWeight;
               } else {
                  zi = idiff / sqrt(ss.indexMatchesVarianceProjWtd.at(i));
                  zir += zi;
                  indexCume += sked*zi;
               }
            }
            if (mul < 0.0) {
               testVal = -mul*IRutil::sqr(indexCume)/(indexFitEnd - indexFitStart + 1);
            } else {
               testVal = mul*zir*zir/(indexFitEnd - indexFitStart + 1);
            }
            break;

          case ERROR1:
            testVal = 0.0;
            for (size_t i = 0; i < mySelectors->size(); i++) {
               if (mySelectors->at(i)->getName() == "as1") {
                  sel = i;
                  break;
               }
            }
            sked = skedVal(error1Sked, ss.selectorMatchesPlayerProjWtd.at(sel),
                           ss.selectorMatchesPlayerProjVarianceWtd.at(sel), ss.accumWeight);

            idiff = ss.selectorMatchesPlayerWtd.at(sel) - ss.selectorMatchesPlayerProjWtd.at(sel);
            if (mul < 0.0) {
               testVal += sked * fabs(mul) * IRutil::sqr(idiff/ss.accumWeight);  //yes?
            } else {
               idiffSq = IRutil::sqr(idiff);  // /ss.accumWeight);
               zisq = idiffSq / ss.selectorMatchesPlayerProjVarianceWtd.at(sel);
               testVal += sked * mul * zisq;
            }
            /*
            if (finished) {
               cout << "ERROR1 Actual: " << ss.selectorMatchesPlayerWtd.at(sel)
                    << ", Proj: " << ss.selectorMatchesPlayerProjWtd.at(sel)
                    << ", Var: " << ss.selectorMatchesPlayerProjVarianceWtd.at(sel)
                    << ", Z: " << sqrt(zisq) << ", sked " << sked << ", testVal: " << testVal << endl;
            }
            */
            break;

          case ERROR2:
            for (size_t i = 0; i < mySelectors->size(); i++) {
               if (mySelectors->at(i)->getName() == "as2") {
                  sel = i;
                  break;
               }
            }
            sked = skedVal(error2Sked, ss.selectorMatchesPlayerProjWtd.at(sel),
                           ss.selectorMatchesPlayerProjVarianceWtd.at(sel), ss.accumWeight);

            idiff = ss.selectorMatchesPlayerWtd.at(sel) - ss.selectorMatchesPlayerProjWtd.at(sel);
            if (mul < 0.0) {
               testVal += sked * fabs(mul) * IRutil::sqr(idiff/ss.accumWeight);
            } else {
               idiffSq = IRutil::sqr(idiff);  // /ss.accumWeight);
               zisq = idiffSq / ss.selectorMatchesPlayerProjVarianceWtd.at(sel);
               testVal += sked * mul * zisq;
            }
            /*
            if (finished) {
               cout << "ERROR2 Actual: " << ss.selectorMatchesPlayerWtd.at(sel)
                    << ", Proj: " << ss.selectorMatchesPlayerProjWtd.at(sel)
                    << ", Var: " << ss.selectorMatchesPlayerProjVarianceWtd.at(sel)
                    << ", Z: " << sqrt(zisq) << ", sked " << sked << ", testVal: " << testVal << endl;
            }
            */
            break;
          default: 
            cerr << "Default on test " << md << ", using percFit." << endl;
            testVal = fabs(mul) * theTrial->percFit(new vector<ostream*>()); 
            break; 
         } 

         //testValStr += IRutil::itoa(1+md) + ": " + IRutil::ftoa(testVal) + "; ";
         testValStr += testNames[md] + ": " + IRutil::ftoa(testVal) + "; ";
         //testVal += gadd/denom;
         result += testVal;
         testVals[onTest] = sqrt(testVal);  //testVal is a square and this will be re-squared by CMINPACK
         onTest++;
         citr++;
      }

      numCalls++;
      if (vb && numCalls % callsPerDisplay == 0) { 
         string s1 = theTrial->strPWV();
         string s2 = (getSpec().v > 0 ? "\nSWV: " + theTrial->strSWV() : "");
         cout << getSpec().paramsPosition();
         if (s1.length() > 10) { cout << endl; } else { cout << "; "; }
         cout << "PWV: " << s1;
	 //if (s2.length() > 10) { cout << endl; } else { cout << "; "; }
	 cout << s2 << endl;
         cout << "Minimand " << result << "/" << denom << " from tests " << testValStr << endl;
      }
      return (result/denom); 
   } 


   
   /** Function f in format expected by GSL multimin.
       Needs to be static rather than member---hence "fparams" is used to
       pass in the needed reference to "this".
    */
   static void f(const size_t n, const double* xs, void* fparams, double* fval){
      Minimizer* miniMe = (Minimizer*)fparams;
      Trial* thisTrial = miniMe->getTrial();   //needed since f is static
      thisTrial->setWhichParams(miniMe->whichParams,n,xs);
      *fval = miniMe->minimand() + (miniMe->mumul > 0 ? miniMe->mumul*miniMe->g() : 0.0);
   }

   /** The "Energy" function f in format for GSL simulated annealing---repeated below.
    *
   static double Ef(void* xp) {
      SimanConfig* scp = (SimanConfig*)xp;  //dynamic_cast<SimanConfig*>(xp);
      return scp->miniMe->minimand();
   }
   */


   /** Function fcn in format needed by MINPACK (C/C++), again static.
       Unlike documentation, happily there really is a void* first parameter.
    */
   static int fcn(void* fparams, int m, int n, const double* xs, double* fvec, int iflag) {
      Minimizer* miniMe = (Minimizer*)fparams;  //passed in as "this", not new
      Trial* thisTrial = miniMe->getTrial();   //needed since f is static
      thisTrial->setWhichParams(miniMe->whichParams,n,xs);

      double res = miniMe->minimand() + (miniMe->mumul > 0 ? miniMe->mumul*miniMe->g() : 0.0);
      fvec = miniMe->testVals;  //set as side-effect of calls to minimand()
      return iflag;
   }

   /** Function header for MINPACK HYBRD where m == n
    */
   static int fcn2(void* fparams, int n, const double* xs, double* fvec, int iflag) {
      return fcn(fparams, n, n, xs, fvec, iflag);
   }

   /** Rearranged form used by MPFIT.  We will use only with the pointer "dvec" set to null,
       so it is ignored.  Since MPFIT manages its own settings of whether parameters are 
       "fixed" or "free", we could dispense with the call to setWhichParams, but we keep it.
       Always fvec is an alias of miniMe->testvals and those values get written.
    */
   static int mpfcn(int m, int n, double* allxs, double* fvec, double** dvec, void* fparams) {
      Minimizer* miniMe = (Minimizer*)fparams;  //passed in as "this", not new
      fvec = miniMe->testVals;
      Trial* thisTrial = miniMe->getTrial();    //needed since f is static
      thisTrial->setWhichParams(255,n,allxs);   //255 because we carry along whole xs matrix
      double res1 = miniMe->minimand();
      double res2 = (miniMe->mumul > 0 ? miniMe->mumul*miniMe->g() : 0.0);
      return int(100000.0 * (res1 + res2));
   }



   /** Helper for funneling---also creates an infinite "penalty" at the boundary.
    */
   static double loginterp(const double& x, const double& lo, const double& mid, const double& hi) {
      const double y2 = (x < mid ? (x - lo)/(mid - lo) : (hi - 2*mid + x)/(hi - x));

      return -(log((y2+IRutil::MINGTZERO)/2.0) + log((2.0-y2+IRutil::MINGTZERO)/2.0));
   }

   /** "Funneling" function---we will minimize f + mu*g.
       As long as mu > 0, also keeps in bounds of lowS..hiS etc.
    */
   inline double g() const {
      double res = 0.0;
      if ((whichParams >> 7) % 2) {
         res += loginterp(theTrial->getS(), lowSpec.s, pivotSpec.s, hiSpec.s);
      }
      if ((whichParams >> 6) % 2) {
         res += loginterp(theTrial->getC(), lowSpec.c, pivotSpec.c, hiSpec.c);
      }
      if ((whichParams >> 5) % 2) {
         res += loginterp(theTrial->getD(), lowSpec.d, pivotSpec.d, hiSpec.d);
      }
      if ((whichParams >> 4) % 2) {
         res += loginterp(theTrial->getV(), lowSpec.v, pivotSpec.v, hiSpec.v);
      }
      if ((whichParams >> 3) % 2) {
         res += loginterp(theTrial->getA(), lowSpec.a, pivotSpec.a, hiSpec.a);
      }
      if ((whichParams >> 2) % 2) {
         res += loginterp(theTrial->getHM(), lowSpec.hm, pivotSpec.hm, hiSpec.hm);
      }
      if ((whichParams >> 1) % 2) {
         res += loginterp(theTrial->getHP(), lowSpec.hp, pivotSpec.hp, hiSpec.hp);
      }
      if ((whichParams) % 2) {
         res += loginterp(theTrial->getB(), lowSpec.b, pivotSpec.b, hiSpec.b);
      }
      return res;
   }



   /** This is the $64,000 function that the client calls. 
       USED ONLY BY void Ensemble::runFit(), and 
       the veeFinder and veeWalker methods are NOT used outside here. 
       --ALSO SETS just ONE of the optimal s and/or c values; runFit() sets
       --the other in a separate call.

       Modified 12/12 to return a TrialSpec, bringing that call in here.
    */ 
   TrialSpec minimize() { 
      finished = false;
      TrialSpec ots = theTrial->getSpec();
      TrialSpec ts = theTrial->getSpec();
      while (mumul > yprec) { 
         double tmp = mumul*pow(yprec,0.25);    //can tune better?
         double myyprec = (tmp > 0.1 ? 0.1 : tmp < yprec ? yprec : tmp);
         double myxprec = myyprec*xprec/yprec;   //preserve original user ratios
         double mygprec = myyprec*geomprec/yprec;
         double wos = walkOutStep * pow(myyprec/yprec, 0.25)/100.0;  //can tune better?
         for (vector<ostream*>::const_iterator it = outs->begin();
              it != outs->end(); it++) {
            (**it) << endl << IRutil::DASHLINE << endl
                   << "Iteration with mu = " << mumul << ", yprec = " << myyprec << ", and spec = " << endl
                   << string(ts) << endl << IRutil::DASHLINE << endl;
         }

         ts = minimize1(myxprec,myyprec,mygprec,wos);
         theTrial->setSpec(ts);
         pivotSpec = ts;
         mumul /= mudiv;
      }
      if (mumul > 0.0) {
         for (vector<ostream*>::const_iterator it = outs->begin();
             it != outs->end(); it++) {
            (**it) << endl << IRutil::DASHLINE << endl
                   << "Final step with prec = " << yprec << ", and spec = " 
                  << string(ts) << endl << IRutil::DASHLINE << endl;
         }
         mumul = 0.0;
      }
      TrialSpec finalTS = minimize1(xprec,yprec,geomprec,walkOutStep); 
      finished = true;
      return finalTS;
   }
      
   TrialSpec minimize1(double myxprec, double myyprec, double mygprec, double mywos) {
      if (verbose) {
         for (vector<ostream*>::const_iterator it = outs->begin(); 
              it != outs->end(); it++) { 
            (**it) << endl << IRutil::DASHLINE << endl << string(*this)  
                   << IRutil::DASHLINE << endl; 
         } 
      } 

      //Create a field that makes this use gradient descent rather than 
      TrialSpec ots = theTrial->getSpec();
      if (method == VWALK) {
         callsPerDisplay *= 1000;   //VWALK code has its own display protocol
         double val = walkOut ? veeWalker() : veeFinder();
         if (isSFirst()) {
            ots.s = val;
            ots.c = bestCFor(ots.s);
         } else {
            ots.c = val;
            //minS = mm->getSpec().s;
            ots.s = bestSFor(ots.c);
         }
         callsPerDisplay /= 1000;

      } else {  //if (method == GSL_MULTIMIN || method == GSL_SIM_ANN || method == CMINPACK_LMDIF1) { 
                //or GSL_NM4, GSL_NM6, GSL_NM7, GSL_SIM_ANN, CMINPACK_LMDIF, CMINPACK_HYBRD

         //from bounds, see if some params are meant to be fixed
         const int numParams = 8;
         const int limi = (mumul > 0 ? 0 : 1);
         mp_par paramSettings[numParams] = {
             {1, {limi, limi}, {lowSpec.s, hiSpec.s}, "s", 0, 0, 0, 0, 0, 0},
             {1, {limi, limi}, {lowSpec.c, hiSpec.c}, "c", 0, 0, 0, 0, 0, 0},
             {1, {limi, limi}, {lowSpec.d, hiSpec.d}, "d", 0, 0, 0, 0, 0, 0},
             {1, {limi, limi}, {lowSpec.v, hiSpec.v}, "v", 0, 0, 0, 0, 0, 0},
             {1, {limi, limi}, {lowSpec.a, hiSpec.a}, "a", 0, 0, 0, 0, 0, 0},
             {1, {limi, limi}, {lowSpec.hm, hiSpec.hm}, "hm", 0, 0, 0, 0, 0, 0},
             {1, {limi, limi}, {lowSpec.hp, hiSpec.hp}, "hp", 0, 0, 0, 0, 0, 0},
             {1, {limi, limi}, {lowSpec.b, hiSpec.b}, "b", 0, 0, 0, 0, 0, 0}
         };
         double params[numParams+1];  //allows room to pass cast of "this" as last arg
         double xmax[numParams];
         double xmin[numParams];
         unsigned mtype[numParams];
         size_t paramsCode = 0; //code is (s,c,d,v,w,em,ep,a) from 00000000 to 11111111 = 255
         size_t numParamsUsed = 0;
         size_t numParamsSeen = 0;

         if (hiSpec.s - lowSpec.s > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.s = min(max(ots.s,lowSpec.s),hiSpec.s);
            paramsCode += 128; 
            params[numParamsUsed] = ots.s;
            xmax[numParamsUsed] = hiSpec.s;
            xmin[numParamsUsed] = lowSpec.s;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.s < hiSpec.s + xprec) {   //adopt and freeze lowSpec.s as the value
            ots.s = lowSpec.s;   
            //leave paramSettings[i].fixed = 1;
         }  //else current value of s is frozen in place
         paramSettings[numParamsSeen].parname = "s";
         numParamsSeen += 1;
            
         if (hiSpec.c - lowSpec.c > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.c = min(max(ots.c,lowSpec.c),hiSpec.c);
            paramsCode += 64;
            params[numParamsUsed] = ots.c;
            xmax[numParamsUsed] = hiSpec.c;
            xmin[numParamsUsed] = lowSpec.c;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.c < hiSpec.c + xprec) {   //adopt and freeze lowSpec.c as the value
            ots.c = lowSpec.c;
         }
         paramSettings[numParamsSeen].parname = "c";
         numParamsSeen += 1;

         if (hiSpec.d - lowSpec.d > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.d = min(max(ots.d,lowSpec.d),hiSpec.d);
            paramsCode += 32;
            params[numParamsUsed] = ots.d;
            xmax[numParamsUsed] = hiSpec.d;
            xmin[numParamsUsed] = lowSpec.d;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.d < hiSpec.d + xprec) {   //adopt and freeze lowSpec.d as the value
            ots.d = lowSpec.d;
         }
         paramSettings[numParamsSeen].parname = "d";
         numParamsSeen += 1;

         if (hiSpec.v - lowSpec.v > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.v = min(max(ots.v,lowSpec.v),hiSpec.v);
            paramsCode += 16;
            params[numParamsUsed] = ots.v;
            xmax[numParamsUsed] = hiSpec.v;
            xmin[numParamsUsed] = lowSpec.v;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.v < hiSpec.v + xprec) {   //adopt and freeze lowSpec.v as the value
            ots.v = lowSpec.v;
         }
         paramSettings[numParamsSeen].parname = "v";
         numParamsSeen += 1;

         if (hiSpec.a - lowSpec.a > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.a = min(max(ots.a,lowSpec.a),hiSpec.a);
            paramsCode += 8;
            params[numParamsUsed] = ots.a;
            xmax[numParamsUsed] = hiSpec.a;
            xmin[numParamsUsed] = lowSpec.a;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.a < hiSpec.a + xprec) {   //adopt and freeze lowSpec.v as the value
            ots.a = lowSpec.a;
         }
         paramSettings[numParamsSeen].parname = "a";
         numParamsSeen += 1;

         if (hiSpec.hm - lowSpec.hm > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.hm = min(max(ots.hm,lowSpec.hm),hiSpec.hm);
            paramsCode += 4;
            params[numParamsUsed] = ots.hm;
            xmax[numParamsUsed] = hiSpec.hm;
            xmin[numParamsUsed] = lowSpec.hm;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.hm < hiSpec.hm + xprec) {   //adopt and freeze lowSpec.v as the value
            ots.hm = lowSpec.hm;
         }
         paramSettings[numParamsSeen].parname = "hm";
         numParamsSeen += 1;

         if (hiSpec.hp - lowSpec.hp > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.hp = min(max(ots.hp,lowSpec.hp),hiSpec.hp);
            paramsCode += 2;
            params[numParamsUsed] = ots.hp;
            xmax[numParamsUsed] = hiSpec.hp;
            xmin[numParamsUsed] = lowSpec.hp;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.hp < hiSpec.hp + xprec) {   //adopt and freeze lowSpec.v as the value
            ots.hp = lowSpec.hp;
         }
         paramSettings[numParamsSeen].parname = "hp";
         numParamsSeen += 1;

         if (hiSpec.b - lowSpec.b > xprec) {
            paramSettings[numParamsSeen].fixed = 0;
            ots.b = min(max(ots.b,lowSpec.b),hiSpec.b);
            paramsCode += 1;
            params[numParamsUsed] = ots.b;
            xmax[numParamsUsed] = hiSpec.b;
            xmin[numParamsUsed] = lowSpec.b;
            mtype[numParamsUsed] = 3;
            numParamsUsed += 1;
         } else if (lowSpec.b < hiSpec.b + xprec) {   //adopt and freeze lowSpec.v as the value
            ots.b = lowSpec.b;
         }
         paramSettings[numParamsSeen].parname = "b";
         numParamsSeen += 1;

         double allxs[numParamsSeen] = {
            ots.s, ots.c, ots.d, ots.v, ots.a, ots.hm, ots.hp, ots.b
         };

         this->whichParams = paramsCode;
         theTrial->setSpec(ots);
         double minVal;

         unsigned int vb = (verbose ? 4 : 0);
         numCalls = 0;

         Minimizer* fparams = this;
         // Type multimin_params imported from GSL
#ifdef OS_UNIX 

         int info = 0;
         gsl_set_error_handler_off();

         if (method == GSL_NM4 || method == GSL_NM6 || method == GSL_NM7) {
            unsigned int alg = (method == GSL_NM4 ? 4 : (method == GSL_NM6 ? 6 : 7));
         
            struct multimin_params optim_par = {
               mywos,         //double step_size
               myxprec,       //double tol  //accuracy of line minimization
               maxIter,       //unsigned maxiter
               myyprec,       //double epsabs---accuracy of the minimization
               mygprec,      //double maxsize---final size of the simplex 
               alg,           //unsigned method---Nelder-Mead simplex flavor
               vb             //verbosity
            };

            if (mumul > 0.0) {
               multimin(numParamsUsed,params,&minVal,NULL,NULL,NULL,
                     &Minimizer::f,&Minimizer::f,NULL,(void*)fparams,optim_par);
            } else {
               multimin(numParamsUsed,params,&minVal,mtype,xmin,xmax,
                     &Minimizer::f,&Minimizer::f,NULL,(void*)fparams,optim_par);
            }

            if (info) {
               cerr << "Multimin failure.  No harm done, can try again." << endl;
            }

         } else if (method == GSL_SIM_ANN) {
            callsPerDisplay *= 4;
            double initialEnergy = minimand();
            //double initTemp = 1.0;  //(prec/initialEnergy);  //or sqrt since least-squares---?
            double initTemp = epsfcn * 10000.0;
            double numCools = size_t(1.0 + maxIter/double(numRetries));
            double dampFactor = pow(initTemp/myyprec, 1.0/double(numCools));

            gsl_siman_params_t gsp;
            gsp.n_tries = numRetries;
            gsp.iters_fixed_T = numRetries;
            gsp.step_size = mywos;
            gsp.k = 1.0;
            gsp.t_initial = initTemp;
            gsp.mu_t = dampFactor;  //1.003;   //magic number like k is
            gsp.t_min = myyprec;
            const gsl_rng_type* T;
            gsl_rng* r;

            gsl_rng_env_setup();

            T = gsl_rng_default;
            r = gsl_rng_alloc(T);
            void* x0_p = new SimanConfig(this, theTrial, theTrial->getSpec(), paramsCode);

            if (verbose) {
               for (vector<ostream*>::const_iterator it = outs->begin();
                    it != outs->end(); it++) {
                  (**it) << endl << "Doing " << maxIter << " evals by SA from temp " << initTemp
                         << " damped by " << IRutil::ftoa(dampFactor, 4) << endl
                         << "giving " << numCools << " cooling stages with " << numRetries
                         << " probes each." << endl;
               }
            }


            gsl_siman_solve(r, x0_p, Ef, takeStep, configDistance, printPosition,
                            //copyConfig, copyConstructConfig, destroyConfig,
                            NULL, NULL, NULL,
                            sizeof(SimanConfig), gsp);

            if (info) {
               cerr << "Simulated annealing failure.  No harm done, can try again." << endl;
            } else {

               SimanConfig* x0 = (SimanConfig*)x0_p;
               theTrial->setSpec(x0->mySpec);
            }
            callsPerDisplay /= 4;

         } else if (method == CMINPACK_LMDIF1) {  //This still has debugging print code

            int lwa = 8*numActiveTests + 40 + numActiveTests;

            cerr << "About to call lmdif1 with m = " << numActiveTests 
                 << " and n = " << numParamsUsed << "..." << endl;

            info = lmdif1(fcn, fparams, numActiveTests, numParamsUsed, params, 
                          testVals, myyprec, paramSwap, lmdifSwap, lwa);

            cerr << "And lmdif1 gave info = " << info << endl;

         } else if (method == CMINPACK_LMDIF) {

            cerr << "About to call lmdif with m = " << numActiveTests 
                 << " and n = " << numParamsUsed << "..." << endl;

            info = lmdif(fcn, fparams, numActiveTests, numParamsUsed, params, testVals,
                             myyprec, myxprec, mygprec, maxIter, epsfcn, diag, 1, mywos, 
                             callsPerDisplay, nfev, fjac, numActiveTests, paramSwap, qtf, wa1, wa2, wa3, wa4);

            cerr << "And lmdif gave info = " << info << endl;

         } else if (method == CMINPACK_HYBRD1) {

            cerr << "About to call hybrd1 with m = " << numActiveTests 
                 << " and n = " << numParamsUsed << "..." << endl;

            if (numParamsUsed == numActiveTests) {
               int lwa = 74;
               info = hybrd1(fcn2, fparams, numParamsUsed, params, testVals, myyprec, hwa, lwa);
            } else {
               cerr << "#active tests must == #active params to use HYBRD" << endl;
            }

            cerr << "And hybrd1 gave info = " << info << endl;

         } else if (method == CMINPACK_HYBRD) {

            cerr << "About to call hybrd with m = " << numActiveTests 
                 << " and n = " << numParamsUsed << "..." << endl;

            if (numParamsUsed == numActiveTests) {
               int lr = numParamsUsed*(numParamsUsed+1)/2;
               info = hybrd(fcn2, fparams, numParamsUsed, params, testVals, myyprec, maxIter, 
                            numParamsUsed, numParamsUsed, epsfcn, diag, 1, mywos,
                            callsPerDisplay, nfev, fjac, numParamsUsed, hwa, lr, qtf, wa1, wa2, wa3, wa4);
            } else {
               cerr << "#active tests must == #active params to use HYBRD" << endl;
            }

            cerr << "And hybrd gave info = " << info << endl;


         } else if (method == MPFIT_LMDIF) {

            mp_config* settings = new mp_config {
               myyprec,            //ftol
               myxprec,            //xtol
               mygprec,            //gtol
               epsfcn,             //epsfcn
               mywos*10000.0,      //stepfactor, common default is 100
               mygprec*0.001,      //covtol
               maxIter,            //maxiter
               0,                  //nfev---no limit on minimand calls
               callsPerDisplay,    //nprint
               0,                  //let variables be scaled internally == MODE in CMINPACK
               0,                  //do not enable check for infinite values
               0                   //mandatory 0 value
            };

            mp_result* results = new mp_result();

            cerr << "About to call mpfit with m = " << numActiveTests << " and n = " 
                 << numParamsSeen << " with " << numParamsUsed << " used and values " << endl;
            for (int i = 0; i < numParamsSeen; i++) {
               cerr << paramSettings[i].parname << ": " << allxs[i] << " in ["
                    << paramSettings[i].limits[0] << ", " << paramSettings[i].limits[1] << "]" << endl;
            }
            int initfv = mpfcn(numActiveTests, numParamsSeen, allxs, testVals, NULL, this);
            cerr << "Initial value " << initfv << " aka. " << minimand() << " from ";
            for (int j = 0; j < numActiveTests; j++) {
               cerr << testVals[j] << " ";
            }
            cerr << endl;

            info = mpfit(&Minimizer::mpfcn, numActiveTests, numParamsSeen, allxs,
                         paramSettings, settings, this, results);

            cerr << "And mpfit gave info = " << info << endl;
            delete(settings);
         }
#endif

         //The calls to f have already set the TrialSpec, so copy it to output.
         ots = theTrial->getSpec();
         
      } 
      //return val;
      //Caller will update the status of the params menu.

      if (verbose) {
         for (vector<ostream*>::const_iterator it = outs->begin();
              it != outs->end(); it++) {
            (**it) << endl << "Total number of minimand calls: " << numCalls << endl;
         }
      }

      return ots;
   } 


   /** Data structures and functions for GSL simulated annealing.
       Have side effect of setting the spec in the Trial.
    */
   struct SimanConfig {
      Minimizer* miniMe; //always points to this
      Trial* thisTrial;  //always points to this->theTrial
      TrialSpec mySpec;  //updated by copy each time.
      size_t paramsCode; //controls which parameters are active

      SimanConfig(Minimizer* m, Trial* t, TrialSpec spec, size_t code) 
       : miniMe(m), thisTrial(t), mySpec(spec), paramsCode(code)
      { }
      void copyp(SimanConfig *rhs) {
         miniMe = rhs->miniMe; 
         thisTrial = rhs->thisTrial;
         mySpec = rhs->mySpec;
         paramsCode = rhs->paramsCode;
      }
   };

   static void copyConfig (void* source, void* dest) {
      SimanConfig* src = (SimanConfig*)source;  //dynamic_cast<SimanConfig*>(source);
      SimanConfig* dst = (SimanConfig*)dest;  //dynamic_cast<SimanConfig*>(dest);
      dst->miniMe = src->miniMe;
      dst->thisTrial = src->thisTrial;
      dst->mySpec = src->mySpec;
      dst->paramsCode = src->paramsCode;
   }

   static void* copyConstructConfig (void* source) {
      SimanConfig* src = (SimanConfig*)source;  //dynamic_cast<SimanConfig*>(source);
      SimanConfig* dst = new SimanConfig(src->miniMe, src->thisTrial, src->mySpec, src->paramsCode);  
      return dst;
   }


   static void destroyConfig(void* xp) { }   //can do nothing


   static double Ef(void* xp) {
      SimanConfig* scp = (SimanConfig*)xp;  //dynamic_cast<SimanConfig*>(xp);
      return scp->miniMe->minimand();
   }
      
   static double configDistance(void* xp, void* yp) {
      SimanConfig* scx = (SimanConfig*)xp;  //dynamic_cast<SimanConfig*>(xp);
      SimanConfig* scy = (SimanConfig*)yp;  //dynamic_cast<SimanConfig*>(yp);
      double sumsq = 0.0;
      sumsq += IRutil::sqr(scx->mySpec.s - scy->mySpec.s);
      sumsq += IRutil::sqr(scx->mySpec.c - scy->mySpec.c);
      sumsq += IRutil::sqr(scx->mySpec.d - scy->mySpec.d);
      sumsq += IRutil::sqr(scx->mySpec.v - scy->mySpec.v);
      sumsq += IRutil::sqr(scx->mySpec.a - scy->mySpec.a);
      sumsq += IRutil::sqr(scx->mySpec.hm - scy->mySpec.hm);
      sumsq += IRutil::sqr(scx->mySpec.hp - scy->mySpec.hp);
      sumsq += IRutil::sqr(scx->mySpec.b - scy->mySpec.b);
      return sqrt(sumsq);
   }

   static void takeStep(const gsl_rng* r, void* xp, double step_size) {
      SimanConfig* scx = (SimanConfig*)xp;  //dynamic_cast<SimanConfig*>(xp);
      double u, inc;
      if ((scx->paramsCode >> 7) % 2) {
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*step_size;
         scx->mySpec.s *= (1.0 + inc);   //relative
         if (scx->mySpec.s < scx->miniMe->lowSpec.s || scx->mySpec.s > scx->miniMe->hiSpec.s) { 
            scx->mySpec.s *= (1.0 - inc)/(1.0 + inc);
         }
      }
      if ((scx->paramsCode >> 6) % 2) {
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*step_size;
         scx->mySpec.c *= (1.0 + inc);   //relative
         if (scx->mySpec.c < scx->miniMe->lowSpec.c || scx->mySpec.c > scx->miniMe->hiSpec.c) {
            scx->mySpec.c *= (1.0 - inc)/(1.0 + inc);
         }
      }
      if ((scx->paramsCode >> 5) % 2) {   //d is treated differently
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*200*step_size;  //200 is a magic number
         scx->mySpec.d += inc;
         if (scx->mySpec.d < scx->miniMe->lowSpec.d || scx->mySpec.d > scx->miniMe->hiSpec.d) { 
            scx->mySpec.d -= 2*inc;
         }
      }
      if ((scx->paramsCode >> 4) % 2) {
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*step_size;
         scx->mySpec.v *= (1.0 + inc);
         if (scx->mySpec.v < scx->miniMe->lowSpec.v || scx->mySpec.v > scx->miniMe->hiSpec.v) { 
            scx->mySpec.v *= (1.0 - inc)/(1.0 + inc);
         }
      }
      if ((scx->paramsCode >> 3) % 2) {
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*step_size;
         scx->mySpec.a *= (1.0 + inc);
         if (scx->mySpec.a < scx->miniMe->lowSpec.a || scx->mySpec.a > scx->miniMe->hiSpec.a) { 
            scx->mySpec.a *= (1.0 - inc)/(1.0 + inc);
         }
      }
      if ((scx->paramsCode >> 2) % 2) {
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*step_size;
         scx->mySpec.hm *= (1.0 + inc);
         if (scx->mySpec.hm < scx->miniMe->lowSpec.hm || scx->mySpec.hm > scx->miniMe->hiSpec.hm) { 
            scx->mySpec.hm *= (1.0 - inc)/(1.0 + inc);
         }
      }
      if ((scx->paramsCode >> 1) % 2) {
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*step_size;
         scx->mySpec.hp *= (1.0 + inc);
         if (scx->mySpec.hp < scx->miniMe->lowSpec.hp || scx->mySpec.hp > scx->miniMe->hiSpec.hp) { 
            scx->mySpec.hp *= (1.0 - inc)/(1.0 + inc);
         }
      }
      if ((scx->paramsCode) % 2) {
         u = gsl_rng_uniform(r);
         inc = (2.0*u - 1.0)*step_size;
         scx->mySpec.b *= (1.0 + inc);
         if (scx->mySpec.b < scx->miniMe->lowSpec.b || scx->mySpec.b > scx->miniMe->hiSpec.b) { 
            scx->mySpec.b *= (1.0 - inc)/(1.0 + inc);
         }
      }
      scx->thisTrial->setSpec(scx->mySpec);
   }

   static void printPosition(void* xp) {
      SimanConfig* scx = (SimanConfig*)xp;
      if (scx->miniMe->numCalls % 10 == 0) { cout << scx->mySpec.paramsPosition(); }
   }


   /** String output on top of what package libraries provide.
    */
   string method2str() const {
      switch(method) {
       case VWALK: 
         return "Two-level bitonic search on s and c only";
       case GSL_NM4: 
         return "GSL Nelder-Mead (old version)";
       case GSL_NM6: 
         return "GSL Nelder-Mead (newer version)";
       case GSL_NM7:
         return "GSL Nelder-Mead (random orientations)";
       case GSL_SIM_ANN:
         return "GSL Simulated Annealing";
       case CMINPACK_LMDIF: 
         return "CMINPACK Levenberg-Marquardt (no derivatives)";
       case CMINPACK_LMDIF1:
         return "CMINPACK Levenberg-Marquardt (simplified call)";
       case CMINPACK_HYBRD:
         return "CMINPACK hybrid Powell method (no derivatives)";
       case CMINPACK_HYBRD1:
         return "CMINPACK hybrid Powell method (simplified call)";
       case MPFIT_LMDIF: 
         return "MPFIT Levenberg-Marquardt (no derivatives)";
       default:
         return "Unknown fitting method";
      }
   };

 
   string sked2str(SKED sked) const {
      switch(sked) {
       case UNIT_SKED:
         return "unitsked";
       case SQRT_SKED:
         return "sqrtsked";
       case LIN_SKED:
         return "linsked";
       case QUAD_SKED:
         return "quadsked";
       case INVVAR_SKED:
         return "invvarsked";
       case SQRT_ALLWT:
         return "sqrtsked";
       case LIN_ALLWT:
         return "linsked";
       case QUAD_ALLWT:
         return "quadsked";
       default:
         return "unknown sked";
      }
   }

 
   inline operator string() const {
      string res = method2str() + " with active tests and weights:\n";
      map<MINIMAND,double>::const_iterator citr = minibook.begin();
      while (citr != minibook.end()) {
         MINIMAND kind = (*citr).first; 
         string mulstr = IRutil::ftoa((*citr).second);
         string skind = "";
         switch(kind) { 
          case PERCFIT: 
            skind = "Percentile fit with L2 distance: " + mulstr + "\n"; 
            break; 
          case ML: 
            skind = "Max-likelihood of played moves: " + mulstr + "\n"; 
            break; 
          case BINS:
            skind = "Fit percentages of hits in bins of probabilities: " + mulstr + "\n";
            break;
          case FALLOFF:
            skind = "Fit of scaled error: " + mulstr + "\n";
            break;
          case FIRST_LINE:
            skind = "Fit of engine first line: " + mulstr + "\n";
            break;
          case SECOND_LINE:
            skind = "Fit of engine second line: " + mulstr + "\n";
	    break;
	  case THIRD_LINE:
            skind = "Fit of engine third line: " + mulstr + "\n";
            break;
          case ETV:
            skind = "Fit of equal-top-value moves to index " + IRutil::itoa(equalTopIndexLimit) + ": " + mulstr + "\n";
            break;
          case INDEX_FIT:
            skind = "Index fit of [" + IRutil::itoa(indexFitStart) + "," + IRutil::itoa(indexFitEnd)
                        + "], " + sked2str(indexFitSked) + ": " + mulstr + "\n";
            break;
          case INDEX_DIFF:
            skind = "Index diff of [" + IRutil::itoa(indexDiffStart) + "," + IRutil::itoa(indexDiffEnd)
                        + "], " + sked2str(indexDiffSked) + ": " + mulstr + "\n";
            break;
          case ERROR1: 
            skind = "Error frequency in [" + IRutil::ftoa(error1Low) + "," + IRutil::ftoa(error1Hi)
                        + "]: " + sked2str(error1Sked) + ": " + mulstr + "\n";
            break;
          case ERROR2:
            skind = "Error frequency in [" + IRutil::ftoa(error2Low) + "," + IRutil::ftoa(error2Hi)
                        + "]: " + sked2str(error2Sked) + ": " + mulstr + "\n";
            break;
          default: 
            skind = "Unknown regression: " + mulstr + "\n"; 
            break; 
         } 
         res += skind;
         citr++;
      }
      
      res += "bounds s in [" + IRutil::ftoa(lowSpec.s) 
                         + ".." + IRutil::ftoa(hiSpec.s) + "], c in ["  
                         + IRutil::ftoa(lowSpec.c) + ".."  
                         + IRutil::ftoa(hiSpec.c) + "],  d in ["
                         + IRutil::ftoa(lowSpec.d) + ".."
                         + IRutil::ftoa(hiSpec.d) + "], v in ["
                         + IRutil::ftoa(lowSpec.v) + ".."
                         + IRutil::ftoa(hiSpec.v) + "], a in ["
                         + IRutil::ftoa(lowSpec.a) + ".."
                         + IRutil::ftoa(hiSpec.a) + "], hm in ["
                         + IRutil::ftoa(lowSpec.hm) + ".."
                         + IRutil::ftoa(hiSpec.hm) + "], hp in ["
                         + IRutil::ftoa(lowSpec.hp) + ".."
                         + IRutil::ftoa(hiSpec.hp) + "], b in ["
                         + IRutil::ftoa(lowSpec.b) + ".."
                         + IRutil::ftoa(hiSpec.b) + "], of\n";

      res += string(*theTrial); 
      return res; 
   } 


   //Rest of code is ONLY used for the "veeFinder" 2-dim. bitonic search routines
 
 private: 
 
   /** Helper function for veeFinders.  Assumes the following true at call: 
       INV: midVal <= f(low), midVal <= f(hi). 
       Returns a local minimum (point not value) of f to approximation 
       (hi - low)/2^(depth+1).  Acts as though Ran(f) is V-shaped. 
       Reports some but not all cases where f has more than one minimum. 
    */ 
   double vfh(Monic f, double low, double mid, double hi, double midVal) const;


 public: 
   Trial* getTrial() const { return theTrial; }

   inline double f1(const double s) const { 
      theTrial->setS(s); 
      return minimand(); 
   } 
   inline double f2(const double c) const { 
      theTrial->setC(c); 
      return minimand(); 
   } 
   inline double vfC(const double s) const {   // find best c for a given s 
      Monic f = &Minimizer::f2; 
      theTrial->setS(s); 
      double midC = geometricMeans ? 2*lowSpec.c*hiSpec.c/(lowSpec.c + hiSpec.c) : (lowSpec.c + hiSpec.c)/2; 
      return vfh(f,lowSpec.c,midC,hiSpec.c, (this->*f)(midC)); 
   } 
   inline double vfCVal(const double s) const { 
      //return value given by best c for s 
      theTrial->setC(vfC(s)); 
      return minimand(); 
   } 
   inline double vfS(const double c) const { 
      // find best s for a given c 
      Monic f = &Minimizer::f1; 
      theTrial->setC(c); 
      double midS = geometricMeans ? 2*lowSpec.s*hiSpec.s/(lowSpec.s+hiSpec.s) 
                                       : (lowSpec.s + hiSpec.s)/2; 
      return vfh(f,lowSpec.s,midS,hiSpec.s, (this->*f)(midS)); 
   } 
   inline double vfSVal(const double c) const { 
      //return value given by best s for c 
      theTrial->setS(vfS(c)); 
      return minimand(); 
   } 
   inline double veeFinder1() const { 
      Monic v = &Minimizer::vfCVal; 
      double midS = geometricMeans ? 2*lowSpec.s*hiSpec.s/(lowSpec.s+hiSpec.s) 
                                       : (lowSpec.s + hiSpec.s)/2; 
      return vfh(v,lowSpec.s,midS,hiSpec.s, (this->*v)(midS)); 
   } 
   inline double veeFinder2() const { 
      Monic v = &Minimizer::vfSVal; 
      double midC = geometricMeans ? 2*lowSpec.c*hiSpec.c/(lowSpec.c + hiSpec.c) : (lowSpec.c + hiSpec.c)/2; 
      return vfh(v,lowSpec.c,midC,hiSpec.c, (this->*v)(midC)); 
   } 
   inline double veeFinder() const { 
      return (sFirst ? veeFinder1() : veeFinder2()); 
   } 
 
   /** Walk out from the trial's initial (s,c) point rather than in from 
       the Minimizer object's overall boundaries.  Otherwise functions are 
       similar to above---longer implementations are outside the class. 
    */ 
   double vWalkC(const double s) const; 
   double vWalkCVal(const double s) const; 
   double vWalkS(const double c) const; 
   double vWalkSVal(const double c) const; 
   double veeWalker1() const; 
   double veeWalker2() const; 
 
   inline double veeWalker() const { 
      return (sFirst ? veeWalker1() : veeWalker2()); 
   } 
 
 
   inline double bestSFor(const double c) const { 
      return (walkOut ? vWalkS(c) : vfS(c)); 
   } 
 
   inline double bestCFor(const double s) const { 
      return (walkOut ? vWalkC(s) : vfC(s)); 
   } 
 
}; //end of class Minimizer, implementation of V-walk methods follows (can ignore)
 
 
#endif    //end of #ifndef __IR_MINIMIZER_H__ 
 
 
 
