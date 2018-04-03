//File------------------PerfData.h------------------------------------------- 
 
#ifndef __IR_PERFDATA_H__ 
#define __IR_PERFDATA_H__ 

#ifndef __IR_INCLUDES_H__
#include "IRincludes.h"
#endif
 
//-------------------STATISTICAL QUANTITIES---------------------------------- 
 

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

 
#ifndef __IR_MODELS_H__
#include "IRmodels.h"
#endif


/*
struct MoveProjection { 
   double firstMoveProb; 
   double topMoveProb; 
   double playedMoveProb; 
   int playedMoveIndex; 
   double playedMoveDelta; 
   string firstMove; 
   string playedMove; 
   MoveProjection() : firstMoveProb(0.0), topMoveProb(0.0), playedMoveProb(0.0),
    playedMoveIndex(0), playedMoveDelta(0.0), firstMove(string("")), playedMove(string(""))
   { }
   ~MoveProjection() { }
}; 
*/

/** SimpleStats omits variance information.  //OK, well it has some.  //Indeed it has lots
    For speed in regression and bootstrapping.
 */
struct SimpleStats {   
   double accumWeight;
   vector<double> indexAccume;
   vector<double> indexMatchesWtd;   //includes first-index match
   vector<double> indexMatchesProjWtd;
   vector<double> indexMatchesVarianceProjWtd;
   double equalValueMatchWtd;
   double equalValueMatchProjWtd;
   double equalValueMatchVarianceProjWtd;
   double scaledFalloffWtd;
   double scaledFalloffProjWtd;
   double scaledFalloffVarianceProjWtd;
   double indexScoreWtd;
   double indexMassWtd;
   double indexScoreInvVarWtd;
   double indexScorePropWtd;
   double indexFitWtd;
   double indexDiffWtd;
   double indexDiffMassWtd;
   //vector<MoveSelector*>* mySelectors;   //left implicit
   vector<double> selectorMatchesPlayerWtd;   
   vector<double> selectorMatchesPlayerProjWtd;
   vector<double> selectorMatchesPlayerProjVarianceWtd;
   vector<double> selectorMatchesEngineWtd;
   SimpleStats(size_t numIndices, size_t numSelectors)
               //vector<MoveSelector*>* gselectors = new vector<MoveSelector*>())
    : accumWeight(0.0) //moveMatchWtd(0.0), moveMatchProjWtd(0.0),
    , indexAccume(vector<double>(numIndices,0.0))
    , indexMatchesWtd(vector<double>(numIndices,0.0))
    , indexMatchesProjWtd(vector<double>(numIndices,0.0))
    , indexMatchesVarianceProjWtd(vector<double>(numIndices,0.0))
    , equalValueMatchWtd(0.0), equalValueMatchProjWtd(0.0), equalValueMatchVarianceProjWtd(0.0)
    , scaledFalloffWtd(0.0), scaledFalloffProjWtd(0.0), scaledFalloffVarianceProjWtd(0.0)
    , indexScoreWtd(0.0), indexMassWtd(0.0), indexScoreInvVarWtd(0.0)
    , indexScorePropWtd(0.0), indexFitWtd(0.0), indexDiffWtd(0.0), indexDiffMassWtd(0.0)
    //, mySelectors(gselectors),
    , selectorMatchesPlayerWtd(vector<double>(numSelectors,0.0))
    , selectorMatchesPlayerProjWtd(vector<double>(numSelectors,0.0))
    , selectorMatchesPlayerProjVarianceWtd(vector<double>(numSelectors,0.0))
    , selectorMatchesEngineWtd(vector<double>(numSelectors,0.0))
   { }
   operator string() const {
      ostringstream oss;
      oss << scaledFalloffWtd << ": " << scaledFalloffProjWtd << " +- " << sqrt(scaledFalloffVarianceProjWtd)
          << endl << equalValueMatchWtd << ": " << equalValueMatchProjWtd << " +- " 
          << sqrt(equalValueMatchVarianceProjWtd) << endl;
      for (int j = 0; j < indexMatchesWtd.size(); j++) {
         oss << j << " " << indexMatchesWtd.at(j) << ": " << indexMatchesProjWtd.at(j) << " +- "
             << 2.0*sqrt(indexMatchesVarianceProjWtd.at(j)) << endl;
      }
      oss << "Selectors:" << endl;
      for (int i = 0; i < selectorMatchesPlayerWtd.size(); i++) {
         oss << i << " " << selectorMatchesPlayerWtd.at(i) << ": " << selectorMatchesPlayerProjWtd.at(i) 
             << " +- " << 2.0*sqrt(selectorMatchesPlayerProjVarianceWtd.at(i)) << endl;
      }
      return oss.str();
   }
};


class StatItem {
 public:
   string name;
   size_t maxOrderOfMagnitude;     //if max value is 999999.999 give value 6
   int numDecimals;  //can be negative, e.g. -2 means x100
   double measuredValue;
   double measuredVariance;

   double measuredSigma;
   double left2Sigma;
   double right2Sigma;

   StatItem() : name("???"), maxOrderOfMagnitude(8), numDecimals(2),
         measuredValue(0.0), measuredVariance(0.0) { 
      update();
   }

   StatItem(string gname, size_t maxOM, int numPlaces, double value, 
            double variance)
    : name(gname), maxOrderOfMagnitude(maxOM), numDecimals(numPlaces)
    , measuredValue(value), measuredVariance(variance) {
      update();
   }

   virtual ~StatItem() { }

   virtual void update(bool debug = false) {
      measuredSigma = sqrt(measuredVariance);
      left2Sigma = measuredValue - 2.0*measuredSigma;
      right2Sigma = measuredValue + 2.0*measuredSigma;
   }

   virtual void setName(string newName) { name = newName; }
   virtual void bootName() { setName(string("b-" + name)); }
   virtual void setMeasuredValue(double value) { measuredValue = value; }
   virtual void setMeasuredVariance(double value) { measuredVariance = value; }

   virtual operator string() const {
      ostringstream oss;
      size_t wd = 1 + numDecimals + maxOrderOfMagnitude;
      oss << std::setiosflags(ios::left) << setw(18) << name.substr(0,18);
      oss << resetiosflags(ios::left)
          << setiosflags(ios::right) << setiosflags(ios::fixed) 
          << showpoint << setprecision(numDecimals) 
          << setw(wd) << measuredValue;
      oss << setw(wd) << measuredSigma << ", two-sigma range: " 
          << setw(wd) << left2Sigma << "--" 
          << right2Sigma;
      return oss.str();
   }
};

class TestItem : public virtual StatItem {
 public:
   double projectedValue;
   double projectedVariance;
   int sign;              // +1 or -1
   double adjFactor;      //multiplier for dilated confidence intervals
   bool showAdj;

   double projectedSigma;
   double zScore;
   double projected2SigmaLeft;
   double projected2SigmaRight;
   double projectedSigmaAdj;
   double zScoreAdj;
   double projected2SigmaLeftAdj;
   double projected2SigmaRightAdj;

   TestItem() : StatItem(), projectedValue(0.0), projectedVariance(0.0)
    , sign(+1), adjFactor(1.0), showAdj(false) {
      update();
   }
   
   TestItem(string gname, size_t maxOM, int numPlaces, double value,
            double variance, double projValue, double projVariance,
            int gsign = 1, double af = 1.0, bool sa = false)
    : StatItem(gname, maxOM, numPlaces, value, variance)
    , projectedValue(projValue), projectedVariance(projVariance)
    , sign(gsign), adjFactor(af), showAdj(sa) {
      update();
   }

   virtual ~TestItem() { }

   virtual void update(bool debug = false) {
      StatItem::update();
      projectedSigma = sqrt(projectedVariance);
      zScore = sign*(measuredValue - projectedValue)/projectedSigma; //can be nan
      projected2SigmaLeft = projectedValue - 2.0*projectedSigma;
      projected2SigmaRight = projectedValue + 2.0*projectedSigma;
      projectedSigmaAdj = adjFactor*projectedSigma;
      zScoreAdj = sign*(measuredValue - projectedValue)/projectedSigmaAdj;
      projected2SigmaLeftAdj = projectedValue - 2.0*projectedSigmaAdj;
      projected2SigmaRightAdj = projectedValue + 2.0*projectedSigmaAdj;
   }

   virtual void setProjectedValue(double value) {  //caller must update afterward
      projectedValue = value;
   }
   virtual void setProjectedVariance(double newVariance) {
//cerr << "setting newVariance = " << newVariance << endl;
      projectedVariance = newVariance;
      //update(true);
      //update(false);
   }
      
   virtual operator string() const {
      ostringstream oss;
      double l2p = projected2SigmaLeft;  //(showAdj ? projected2SigmaLeft : projected2SigmaLeftAdj);
      double r2p = projected2SigmaRight; //(showAdj ? projected2SigmaRight : projected2SigmaRightAdj);
      double myz = zScore;   //(showAdj ? zScore : zScoreAdj);
      size_t wd = 1 + numDecimals + maxOrderOfMagnitude;
      oss << std::setiosflags(ios::left) << setw(18) << name.substr(0,18);
      oss << resetiosflags(ios::left) << setiosflags(ios::right)
          << showpoint << setiosflags(ios::fixed) << setprecision(numDecimals);
      oss << setw(wd) << projectedValue << setw(wd-3) << projectedSigma 
          << "; " << setw(wd) << measuredValue;
      oss << ", two-sigma range: " << setw(wd) << l2p << "--" << setw(wd) << r2p
          << ", z = " << showpos << setw(6) << myz;
      if (showAdj) { oss << ", adj " << zScoreAdj; }
      return oss.str();
   }
};

/** StatItem::measuredValue is interpeted as a running sum S_N, and measuredFreq as its mean.
 *  StatItem::measuredVariance however is re-interpreted as V_N = N*sigma_N^2, that is, as the
 *  running total variance.  The recursion for V_N in terms of S_N and the total volume T_N is:
 *  V_N = V_{N-1} + T_{N-1}(S_N/T_n - S_{N-1}/T_{N-1})^2 + (a_N - S_N/T_N)^2,
 *  where the new datum a_N includes any non-unti weight.  The uptick method thus has no add
 *  for the measured variance.  This makes measuredSigmaFreq correct as a percentage.
 *  For example, consider N = 98 with 49 observations of 0, 49 of 1.  Then we have:
 *  sigma = 0.5, sigma^2 = 0.25, N*sigma^2 = 24.5.  Next we observe a99 = 0, a100 = 1.
 *  On the first, uptick adds 98*(49/99 - 49/98)^2 + (49/99)^2 = 0.247474...
 *  On the second, uptick adds 99*(50/100 - 49/99)^2 + (1 - 50/100)^2 = 0.2525252...
 *  The net add is 0.5, bringing Nsigma^2 to 24.5 + 0.5 = 25.  So measuredSigma = 5.
 *  Finally measuredFreq = 0.5 = 50% and measuredSigmaFreq = 0.05 = 5%, as desired.
 */
class AggregateStat : public virtual StatItem {
 protected:
   double sampleVolume;       //for percentages, may be weighted
   //double runningTotal;     // = N*mean, or (total weight)*mean in general
   //double runningVariance;  // = N*sigma^2 etc.
   bool isPct;

 public:
   double measuredFreq;       // = total T divided by N
   double measuredSigmaFreq;  // = sigma/N
   double left2SigmaFreq;
   double right2SigmaFreq;

   AggregateStat() : StatItem(), sampleVolume(0.0), isPct(true) {
      update();
   }

   AggregateStat(string gname, size_t maxOM, int numPlaces, double value,
                 double variance, double volume, bool pct)
    : StatItem(gname, maxOM, numPlaces, value, variance)
    , sampleVolume(volume), isPct(pct) {
      update();
   }

   virtual ~AggregateStat() { }

   virtual void update(bool debug = false) {
      //measuredValue = runningTotal/sampleVolume;
      //measuredVariance = runningVariance/sampleVolume;
      StatItem::update();
      measuredFreq = IRutil::divifgt0(measuredValue,sampleVolume);
      measuredSigmaFreq = IRutil::divifgt0(measuredSigma,sampleVolume);
      left2SigmaFreq = measuredFreq - 2.0*measuredSigmaFreq;
      right2SigmaFreq = measuredFreq + 2.0*measuredSigmaFreq;
      left2SigmaFreq = (left2SigmaFreq < 0.0 ? 0.0 : left2SigmaFreq);
      right2SigmaFreq = (right2SigmaFreq > 99.99 ? 99.99 : right2SigmaFreq);
   }

   double getSampleVolume() const { return sampleVolume; }

   virtual void uptick(double addVolume, double addValue, //double addVariance,
                       bool updateNow = false) {
      double newVol = sampleVolume + addVolume;
      double newSum = measuredValue + addValue;
      measuredVariance += sampleVolume*IRutil::sqr(
            IRutil::divifgt0(newSum,newVol) - IRutil::divifgt0(measuredValue,sampleVolume));
      measuredVariance += IRutil::sqr(addValue - IRutil::divifgt0(newSum,newVol));
      sampleVolume = newVol;
      measuredValue = newSum;
      //measuredVariance += addVariance; //note: perhaps should update via E[(x - E[x])^2]
      if (updateNow) { update(); }
   }

   virtual operator string() const {
      ostringstream oss;
      string div = (isPct ? "%" : "");
      double mul = (isPct ? 100.0 : 1.0);
      double wd = 1 + numDecimals + maxOrderOfMagnitude;
      double prec = (isPct ? 2 : 4);
      oss << std::setiosflags(ios::left) << setw(18) << name.substr(0,18);
      oss << resetiosflags(ios::left) << setiosflags(ios::right)
          << setiosflags(ios::fixed) << showpoint << setprecision(numDecimals);
      oss << setw(wd) << measuredValue << setw(wd-3) << measuredSigma
          << "/" << setw(wd) << sampleVolume << " = ";
      oss << setprecision(prec) << setw(3+prec) << mul*measuredFreq << div << " " 
          << setw(3+prec) << mul*left2SigmaFreq << div << "--"
          << setw(3+prec) << mul*right2SigmaFreq << div;
      return oss.str();
   }
};


class AggregateTest : public TestItem, public AggregateStat {
 protected:
   double logpSum;     //uses projected probabilities
   double entropySum;
   bool showSums;

 public:    //no other "own" protected data
   double projectedFreq;
   double projectedSigmaFreq;
   double projected2SigmaLeftFreq;
   double projected2SigmaRightFreq;
   double projectedSigmaAdjFreq;
   double projected2SigmaLeftAdjFreq;
   double projected2SigmaRightAdjFreq;
   //static const string header;


   AggregateTest() : StatItem(), AggregateStat(), TestItem() 
    , logpSum(0.0), entropySum(0.0), showSums(false) { 
      update();
   }

   AggregateTest(string gname, size_t maxOM, int numPlaces, double value,
                 double variance, double volume, bool pct,
                 double projValue, double projVariance,
                 int gsign = 1, double af = 1.0, bool sa = false, bool ss = false)
    : StatItem(gname, maxOM, numPlaces, value, variance)
    , AggregateStat(gname, maxOM, numPlaces, value, variance, volume, pct)
    , TestItem(gname, maxOM, numPlaces, value, variance,
               projValue, projVariance, gsign, af, sa)
    , logpSum(0.0), entropySum(0.0), showSums(ss) {
      update();
   }

   static const string header() { return string(
"Name                 ProjVal  St.Dev    Actual;   Proj% Actual%  2sigma range   z-score");
   }

   virtual void update(bool debug = false) {
if (debug) { cerr << string(*this) << " is now " << endl; }
      AggregateStat::update();
      TestItem::update();  //re-does base class but that's OK
if (debug) { cerr << string(*this) << endl; }
      projectedFreq = IRutil::divifgt0(projectedValue, sampleVolume);
      projectedSigmaFreq = IRutil::divifgt0(projectedSigma, sampleVolume);
      projected2SigmaLeftFreq = projectedFreq - 2.0*projectedSigmaFreq;
      projected2SigmaRightFreq = projectedFreq + 2.0*projectedSigmaFreq;
      projectedSigmaAdjFreq = projectedSigmaFreq*adjFactor;
      projected2SigmaLeftAdjFreq = projectedFreq - 2.0*projectedSigmaAdjFreq;
      projected2SigmaRightAdjFreq = projectedFreq + 2.0*projectedSigmaAdjFreq;
   }

   virtual void uptick(double addVolume, double addValue, //double addVariance, 
                       double addProjectedValue, double addProjectedVariance,
                       double addLogSum, double addEntropySum, bool updateNow = false) {
      AggregateStat::uptick(addVolume, addValue, false);  //addVariance, false);
      projectedValue += addProjectedValue;
      projectedVariance += addProjectedVariance;
      logpSum += addLogSum;
      entropySum += addEntropySum;  //!!!need projected variance add...
      if (updateNow) { update(); }
   }

   virtual void setProjectedVariance(double newVariance) {
//cerr << "setting newVariance = " << newVariance << endl;
      projectedVariance = newVariance;
      //update(true);
      //update(false);
   }



   virtual operator string() const {
      ostringstream oss;
      double l2f = projected2SigmaLeftFreq;  //(showAdj ? projected2SigmaLeftFreq : projected2SigmaLeftAdjFreq);
      double r2f = projected2SigmaRightFreq; //(showAdj ? projected2SigmaRightFreq : projected2SigmaRightAdjFreq);
      double myz = zScore;  //(showAdj ? zScore : zScoreAdj);
      
      string div = (isPct ? "%" : "");
      double mul = (isPct ? 100.0 : 1.0);
      string divcolon = div+(mul*measuredFreq >= 100.0 ? ":" : ": ");
      double wd = 1 + numDecimals + maxOrderOfMagnitude;
      double prec = (isPct ? 2 : 4);
      oss << std::setiosflags(ios::left) << setw(18) << name.substr(0,18);
      oss << resetiosflags(ios::left) << setiosflags(ios::right) 
          << setiosflags(ios::fixed) << showpoint << setprecision(numDecimals);
      oss << setw(wd) << projectedValue << setw(wd-3) << projectedSigma << ": "
          << setw(wd) << measuredValue << "  ";
      oss << setprecision(prec) << setw(2+prec) << mul*projectedFreq << divcolon 
          << setw(2+prec) << mul*measuredFreq << div << " "
          << setw(2+prec) << mul*l2f << div << "--" << setw(2+prec) << mul*r2f << div
          << ", z = " << showpos << setw(5) << myz;
      if (showAdj) { oss << ", adj " << setw(5) << zScoreAdj; }
      if (showSums) {
         oss << endl << noshowpos << "Log(1/p) sum: " << logpSum << ", p*log(1/p) sum: " << entropySum;
      }
      return oss.str();
   }
};

/*
const string AggregateTest::header = string(
"Name              ProjVal  St.Dev    Actual;   Proj% Actual%  2sigma range   z-score");
*/


class MoveIndexTest : public AggregateTest {
 protected:
   size_t index;
   double deltaSum;
   double swingFixedSum;
   double swingUsedSum;
   double swingRelSum;

 public:
   double meanDelta;
   double meanSwingFixed;
   double meanSwingUsed;
   double meanSwingRel;
   //static const string header;

   MoveIndexTest() : AggregateTest(), index(0), deltaSum(0.0),
                     swingFixedSum(0.0), swingUsedSum(0.0), swingRelSum(0.0) { 
      update();
   }

   MoveIndexTest(int ind, size_t maxOM, int numPlaces, double value,
                 double variance, double volume, bool pct, double projValue,
                 double projVariance, double ds, double sfs, double sus, double srs,
                 int gsign, double af, bool sa, bool ss)
    : StatItem(IRutil::itoa(ind), maxOM, numPlaces, value, variance) 
    , AggregateTest(IRutil::itoa(ind), maxOM, numPlaces, value, variance, volume, pct,
                    projValue, projVariance, gsign, af, sa, ss)
    , index(ind), deltaSum(ds), swingFixedSum(sfs), swingUsedSum(sus), swingRelSum(srs) { 
      update();
   }

   virtual void update(bool debug = false) {
      AggregateTest::update(debug);
      meanDelta = IRutil::divifgt0(deltaSum, sampleVolume);
      meanSwingFixed = IRutil::divifgt0(swingFixedSum, sampleVolume);
      meanSwingUsed = IRutil::divifgt0(swingUsedSum, sampleVolume);
      meanSwingRel = IRutil::divifgt0(swingRelSum, sampleVolume);
   }

   virtual void uptick(double addVolume, double addValue, //double addVariance,
                       double addProjectedValue, double addProjectedVariance,
                       double addLogSum, double addEntropySum, double addDelta,
                       double addSwingFixed, double addSwingUsed, double addSwingRel,
                       bool updateNow = false) {
      AggregateTest::uptick(addVolume, addValue, //addVariance, 
                            addProjectedValue, addProjectedVariance, addLogSum, 
                            addEntropySum, false);
      deltaSum += addDelta;
      swingFixedSum += addSwingFixed;
      swingUsedSum += addSwingUsed;
      swingRelSum += addSwingRel;
      if (updateNow) { update(); }  //updates the parent too
   }

   operator string() const {
      ostringstream oss;
      double l2f = (showAdj ? projected2SigmaLeftFreq : projected2SigmaLeftAdjFreq);
      double r2f = (showAdj ? projected2SigmaRightFreq : projected2SigmaRightAdjFreq);
      double myz = (showAdj ? zScore : zScoreAdj);
      string div = (isPct ? "%" : "");
      double mul = (isPct ? 100.0 : 1.0);
      size_t wd = 1 + numDecimals + maxOrderOfMagnitude;
      double prec = (isPct ? 2 : 4);
      oss << resetiosflags(ios::left) << setiosflags(ios::right) << showpoint;
      oss << setw(2) << setiosflags(ios::fixed) << index
          << setprecision(2) << setw(6) << meanDelta 
          << setprecision(4) << setw(8) << meanSwingFixed 
          << setprecision(4) << setw(8) << meanSwingUsed 
          << setprecision(4) << setw(8) << meanSwingRel;
      oss << setiosflags(ios::fixed) << setprecision(numDecimals);
      oss << setw(wd) << projectedValue << setw(wd-3) << projectedSigma << ": "
          << setw(wd) << measuredValue << "  ";
      oss << setprecision(prec) << setw(3+prec) << mul*projectedFreq << div << ": "
          << setw(3+prec) << mul*measuredFreq << div << " "
          << setw(3+prec) << mul*l2f << div << "--" << setw(3+prec) << mul*r2f << div
          << ", z = " << showpos << setw(6) << myz;
      if (showAdj) { oss << ", adj " << setw(6) << zScoreAdj; }
      if (showSums) {
         oss << endl << noshowpos << "Log(1/p) sum: " << logpSum << ", p*log(1/p) sum: " << entropySum;
      }
      return oss.str();
   }

   static const string header() { return string(
"i  mDelta SwNotDD SwingDD SwRel    ProjVal  Sigma      Actual   Proj% Actual%  2sigma range   z-score");
   }
};

/*
const string MoveIndexTest::header = string(
"i  mDelta SwFixed SwUsed  SwRel    ProjVal  Sigma      Actual   Proj% Actual%  2sigma range   z-score");
*/


class SelectionTest : public AggregateTest {
 public:
   double engineMatches;  //measuredValue refers to played-move matches
   double engineMatchFreq;
   //static const string header;

   SelectionTest() : AggregateTest(), engineMatches(0.0), engineMatchFreq(0.0) {
      update();
   }

   SelectionTest(string gname, size_t maxOM, int numPlaces, double value,
                 double variance, double volume, bool pct, double projValue,
                 double projVariance, double runningEngineMatches,
                 int gsign = 1, double af = 1.0, bool sa = false, bool ss = false)
    : StatItem(gname, maxOM, numPlaces, value, variance)
    , AggregateTest(gname, maxOM, numPlaces, value, variance, volume, pct,
                    projValue, projVariance, gsign, af, sa, ss)
    , engineMatches(runningEngineMatches) {
      update();
   }

   virtual void update(bool debug = false) {
      AggregateTest::update();
      engineMatchFreq = (sampleVolume > 0 ? engineMatches / sampleVolume : 0.0);
   }

   virtual void uptick(double addVolume, double addValue, //double addVariance,
                       double addProjectedValue, double addProjectedVariance,
                       double addLogSum, double addEntropySum, double addEngineMatches,
                       bool updateNow = false) {
      AggregateTest::uptick(addVolume, addValue, //addVariance, 
                            addProjectedValue, addProjectedVariance, 
                            addLogSum, addEntropySum, false);
      engineMatches += addEngineMatches;
      if (updateNow) { update(); }
   }

   virtual operator string() const {
      ostringstream oss;
      double l2f = projected2SigmaLeftFreq;  //(showAdj ? projected2SigmaLeftFreq : projected2SigmaLeftAdjFreq);
      double r2f = projected2SigmaRightFreq; //(showAdj ? projected2SigmaRightFreq : projected2SigmaRightAdjFreq);
      double myz = zScore;  //(showAdj ? zScore : zScoreAdj);
      string div = (isPct ? "%" : " ");
      double mul = (isPct ? 100.0 : 1.0);
      string divcolon = div+(mul*measuredFreq >= 100.0 ? ":" : ": ");
      size_t wd = 1 + numDecimals + maxOrderOfMagnitude;
      double prec = (isPct ? 2 : 4);
      oss << std::setiosflags(ios::left) << setw(18) << name.substr(0,18);
      oss << resetiosflags(ios::left) << setiosflags(ios::right)
          << setiosflags(ios::fixed) << setprecision(numDecimals) << showpoint;
      oss << setw(wd) << projectedValue << setw(wd-3) << projectedSigma << ": "
          << setw(wd) << measuredValue << " ";
      oss << setprecision(prec) << setw(3+prec) << mul*projectedFreq << divcolon
          << setw(3+prec) << mul*measuredFreq << div << " "
          << setw(3+prec) << mul*l2f << div << "--" << setw(3+prec) << mul*r2f << div
          << ", z = " << showpos << setw(6) << myz << noshowpos;
      oss << ", engm% = " << noshowpos << mul*engineMatchFreq;
      if (showSums) {
         oss << endl << "Log(1/p) sum: " << logpSum << ", p*log(1/p) sum: " << entropySum;
      }
      //oss << endl << "Engine matches: " << engineMatches << "/" << sampleVolume
          //<< " = " << mul*engineMatchFreq << div;
      return oss.str();
   }

   static const string header() { return string(
"Selection Test       ProjVal  St.Dev    Actual;  Proj%  Actual%  2sigma range  z-score");
   }

};

/*
const string SelectionTest::header = string(
"Selection Test    ProjVal  St.Dev    Actual;  Proj%  Actual%  2sigma range  z-score");
*/


class BootstrappedItem {  //uses wrapping not templates
 protected:
   StatItem* original;
   StatItem* bootcopy;
   size_t numBootTrials; //optional

 public:
   BootstrappedItem(StatItem* orig, double bootValue, double bootVariance, size_t numTrials)
    : original(orig)
    , bootcopy(new StatItem("boot"+IRutil::itoa(numTrials), original->maxOrderOfMagnitude,
                                 original->numDecimals, bootValue, bootVariance))
    , numBootTrials(numTrials) {
      bootcopy->update();
   }

   virtual ~BootstrappedItem() { delete(bootcopy); }

   operator string() const {
      ostringstream oss;
      oss << string(*original) << endl << string(*bootcopy);
      return oss.str();
   }
};


 
/** Performance data and statistics, including ones for other regressions 
    besides the main "Percentile Fitting" method. 
    [Can use some suggestions for more stats to include, and for how to 
    weight different kinds of fit stats for an "Omni Regression".] 
 */ 
class PerfData { 
 public: 
   bool weighted;   //Now controls whether unweighted items are filled
   double deltaCap; 
 
   //Fields filled in by perfTest(...) from class Trial---now by calls to uptick
   int numTurns;           // actual number of MoveProjection items
   int totalBootItems;     // \sum_i bootWeight(i)
   double accumWeight;     // \sum_i wt(i)*bootWeight(i)  //in case of carryover
   //Otherwise == sampleVolume field on individual AggregateTest items (duplicated)

   //Vector of length totalBootItems, not numTurns
   vector<Models::MoveProjection*>* turnsData;
 
   vector<MoveIndexTest> moveIndexTests;
   vector<MoveIndexTest> moveIndexTestsWtd;

   AggregateTest scaledFalloff;
   AggregateTest scaledFalloffWtd;
   AggregateTest unscaledFalloff;
   AggregateTest unscaledFalloffWtd;

   AggregateStat scaledFalloffNextTurnCapped;
   AggregateStat scaledFalloffNextTurnCappedWtd;
   AggregateStat unscaledFalloffNextTurnCapped;
   AggregateStat unscaledFalloffNextTurnCappedWtd;

   AggregateTest moveMatch;   //duplicates moveIndexTests[0]
   AggregateTest equalValueMatch;
   AggregateTest playedMoveMatch; //100% on actual but pred is relevant
   

   AggregateTest moveMatchWtd;   //duplicates moveIndexTestsWtd[0]
   AggregateTest equalValueMatchWtd;
   AggregateTest playedMoveMatchWtd;

   vector<SelectionTest> selectionTests;
   vector<SelectionTest> selectionTestsWtd;
   

//-----------Other performance statistics---------------------- 
 
   double indexFit; 
   double indexFitMass; 
   double indexFitInvVar; 
   double indexFitProp; 
   double indexDiff;
   double indexDiffMass;
   double indexFitWtd; 
   double indexFitMassWtd; 
   double indexFitInvVarWtd; 
   double indexFitPropWtd; 
   double indexDiffWtd;
   double indexDiffMassWtd;
 
 
//------------------Constructors etc.--------------------------- 
 
   explicit PerfData(double cap, size_t etvl = IRutil::NCUTOFF,
                     int gNumTurns = 0, double gaccumWeight = 0.0) 
    : deltaCap(cap), numTurns(gNumTurns), totalBootItems(0), accumWeight(gaccumWeight)
    , turnsData(new vector<Models::MoveProjection*>())   //dim numDecisions filled by class Trial`
    , moveIndexTests(vector<MoveIndexTest>())   //IRutil::NCUTOFF + 1))  //dim Ncutoff
    , moveIndexTestsWtd(vector<MoveIndexTest>()) //IRutil::NCUTOFF + 1))

    , scaledFalloff(AggregateTest("AvgScaledDiff",7,2,0.0,0.0,0.0,false,0.0,0.0,-1,1.4,true,false))
    , scaledFalloffWtd(AggregateTest("AvgScaledDiffW",7,2,0.0,0.0,0.0,false,0.0,0.0,-1,1.4,true,false))
    , unscaledFalloff(AggregateTest("UnscaledFalloff",7,2,0.0,0.0,0.0,false,0.0,0.0,-1,1.4,true,false))
    , unscaledFalloffWtd(AggregateTest("UnscaledFalloffW",7,2,0.0,0.0,0.0,false,0.0,0.0,-1,1.4,true,false))
    , scaledFalloffNextTurnCapped(AggregateStat("NextTurnFalloff",7,2,0.0,0.0,0.0,false))
    , scaledFalloffNextTurnCappedWtd(AggregateStat("NextTurnFalloffW",7,2,0.0,0.0,0.0,false))
    , unscaledFalloffNextTurnCapped(AggregateStat("NextTurnFalloffU",7,2,0.0,0.0,0.0,false))
    , unscaledFalloffNextTurnCappedWtd(AggregateStat("NextTurnFalloffUW",7,2,0.0,0.0,0.0,false))

    , moveMatch(AggregateTest("MoveMatch", 7, 2, 0.0, 0.0, 0.0, true, 0.0, 0.0, +1, 1.15, true, false))
    , equalValueMatch(AggregateTest("EqValueMatch["+IRutil::itoa(etvl)+"]", 7, 2, 0.0, 0.0, 0.0, true, 0.0, 0.0, +1, 1.2, true, false))
    , playedMoveMatch(AggregateTest("PlayedMoveMatch", 7, 2, 0.0, 0.0, 0.0, true, 0.0, 0.0, +1, 1.0, false, false))
    , moveMatchWtd(AggregateTest("MoveMatchWtd", 7, 2, 0.0, 0.0, 0.0, true, 0.0, 0.0, +1, 1.15, true, false))
    , equalValueMatchWtd(AggregateTest("EqValueMatchW["+IRutil::itoa(etvl)+"]", 7, 2, 0.0, 0.0, 0.0, true, 0.0, 0.0, +1, 1.2, true, false))
    , playedMoveMatchWtd(AggregateTest("PlayedMoveMatchW", 7, 2, 0.0, 0.0, 0.0, true, 0.0, 0.0, +1, 1.0, false, false))

    , selectionTests(vector<SelectionTest>())
    , selectionTestsWtd(vector<SelectionTest>())

    , indexFit(0.0)
    , indexFitMass(0.0)
    , indexFitInvVar(0.0)
    , indexFitProp(0.0)
    , indexDiff(0.0)
    , indexDiffMass(0.0)
    , indexFitWtd(0.0)
    , indexFitMassWtd(0.0)
    , indexFitInvVarWtd(0.0)
    , indexFitPropWtd(0.0)
    , indexDiffWtd(0.0)
    , indexDiffMassWtd(0.0)

   { } 
 
   virtual ~PerfData() { 
      vector<Models::MoveProjection*>::iterator itr = turnsData->begin();
      vector<Models::MoveProjection*>::iterator itre = turnsData->end();
      while (itr != itre) {
         delete(*itr++);
      }
      delete(turnsData);
   }
/*
      delete(playedMoveIndexHistogram);
      delete(playedMoveIndexHistogramWtd);
      delete(predMoveIndices);
      delete(predMoveVariances);
      delete(predMoveIndicesWtd);
      delete(predMoveVariancesWtd);
      delete(meanDeltas);
      delete(meanDeltasWtd);
      IRutil::perfDataDeleted++;
      if (IRutil::perfDataDeleted % IRutil::TURNSMODULUS == 0) {
         cerr << "PerfData made: " << IRutil::perfDataMade
              << ", deleted: " << IRutil::perfDataDeleted << endl;
      }
   }

*/
   static string perfHeader(bool flm) { 
      const string s = flm ? "1stLM %" : "Top-M %"; 
      const string st = 
    "Category  #Turns   Act. - Pred "+ s +"   PM %   Predic. / Actual Falloff"; 
   //cccccccc: dddddd   dd.d - dd.d = +dd.d   dd.d   ddddd.d / ddddd.d = d.ddd 
   //scaled, weighted falloff only here 
      return st; 
   } 

   string perfLine(const string& leader, bool flm, int dw = 7) const { 
      ostringstream ost; 
      double actFreq = (flm ? moveMatch.measuredFreq : equalValueMatch.measuredFreq);
      double predFreq = (flm ? moveMatch.projectedFreq : equalValueMatch.projectedFreq);
      //double predFreq = flm ? predFirstMoveFreq : predTopFreq; 
      ost << fixed << setw(8) << left << leader << ": " 
          << setw(6) << right << int(accumWeight + 0.5) << "   " 
          << setprecision(1) << showpoint 
          << setw(4) << right << actFreq << " - " 
          << setw(4) << left << predFreq << " = " 
          << setw(5) << right << showpos << actFreq-predFreq << "   " 
          << setw(4) << right << noshowpos << playedMoveMatch.projectedFreq << "   " 

          << setw(dw) << right << scaledFalloffWtd.projectedFreq << " / " 
          << setw(dw) << right << scaledFalloffWtd.measuredValue << " = " 
          << setprecision(2) 
          << setw(4) << right << (scaledFalloffWtd.projectedFreq)/(scaledFalloffWtd.measuredValue); 
       return ost.str(); 
   } 
 
   string moveHeader() const { 
      ostringstream ost; 
      ost << fixed << "  " << setw(14) << " Played Move  " << setw(3) 
          << "PMI" << "  " << setw(11) << "Engine Move" 
          << "  " << setprecision(3) << setw(6) << "ProbEM" 
          << "  " << setw(6) << "ProbPM" << "  " 
             << setprecision(2) << setw(5) << "Delta"; 
      return ost.str(); 
   } 
 
   /** playedMove playedMoveIndex firstMove firstMoveProb playedMoveProb delta 
    */ 
   string moveLine(const Models::MoveProjection* const mp, bool flm = true) const { 
      ostringstream ost; 
      string em = (mp->indexPlayedMove == mp->indexFirstMove) ? "   MATCH   " : mp->firstMove; 
      ost << fixed << "  " << setw(14) << left << mp->playedMove  
          << setw(4) << right << mp->indexPlayedMove
          << "  " << setw(11) << left << em 
          << "  " << right << setprecision(3) << setw(6) << mp->probs->at(mp->indexFirstMove); 
      if (em != " MATCH ") { 
         ost << "  " << right << setw(6) << mp->probs->at(mp->indexPlayedMove) << "  " 
             << setprecision(2) << setw(5) << right << mp->playedMoveDeltaScaled; 
      } 
      return ost.str(); 
   } 
 
   string dStats(const string& leader, int dw, bool wtd, 
                 double pred, double predf, double p2sl, double p2sr, 
                 double pf2sl, double pf2sr, double act, double actf) const { 
      ostringstream ost; 
      string adj = (wtd && fabs(accumWeight - numTurns) 
                        > IRutil::MINGTZERO) ? "Wtd." : "Unwtd."; 
      ost << fixed << setprecision(dw-4) << showpoint 
          << setw(7) << left << leader << ": " 
          << setw(dw) << right << numTurns << "  " 
          << adj << " filtered turns, total weight " << accumWeight << endl 
          << setw(7) << left << "Predic." << ": " 
          << setw(dw) << right << pred << ", " << setprecision(2) 
          << setw(5) << right << predf << "%, two-sigma range: " 
          << setprecision(dw-4) << setw(dw) << right << p2sl << "--" 
          << setw(dw) << left << p2sr << ", " 
          << setprecision(2) << setw(5) << right << pf2sl << "--" 
          << setw(5) << left << pf2sr << "%" << endl 
          << setprecision(dw-4) << setw(7) << left << "Actual" << ": " 
          << setw(dw) << right << act << ", " 
          << setprecision(2) << setw(5) << right << actf << "%"; 
      return ost.str(); 
   } 
 
 
 
   string firstMoveMatchStats(const string& leader, int dw, bool wtd = true) const { 
      double pfm = wtd ? moveMatchWtd.projectedValue : moveMatch.projectedValue; 
      double pfmf = wtd ? moveMatchWtd.projectedFreq : moveMatch.projectedFreq; 
      double fm2sl = wtd ? moveMatchWtd.projected2SigmaLeft : moveMatch.projected2SigmaLeft; 
      double fm2sr = wtd ? moveMatchWtd.projected2SigmaRight : moveMatch.projected2SigmaRight; 
      double fm2slf = wtd ? moveMatchWtd.projected2SigmaLeftFreq : moveMatch.projected2SigmaLeftFreq;
      double fm2srf = wtd ? moveMatchWtd.projected2SigmaRightFreq : moveMatch.projected2SigmaRightFreq;
      double fma = wtd ? moveMatchWtd.measuredValue : moveMatch.measuredValue; 
      double fmaf = wtd ? moveMatchWtd.measuredFreq : moveMatch.measuredFreq; 
      return dStats(leader,dw,wtd,pfm,pfmf,fm2sl,fm2sr,fm2slf,fm2srf,fma,fmaf); 
   } 

   string topMatchStats(const string& leader, int dw, bool wtd = true) const { 
      double ptm = wtd ? equalValueMatchWtd.projectedValue : equalValueMatch.projectedValue;
      double ptmf = wtd ? equalValueMatchWtd.projectedFreq : equalValueMatch.projectedFreq;
      double tm2sl = wtd ? equalValueMatchWtd.projected2SigmaLeft : equalValueMatch.projected2SigmaLeft;
      double tm2sr = wtd ? equalValueMatchWtd.projected2SigmaRight : equalValueMatch.projected2SigmaRight;
      double tm2slf = wtd ? equalValueMatchWtd.projected2SigmaLeftFreq : equalValueMatch.projected2SigmaLeftFreq;
      double tm2srf = wtd ? equalValueMatchWtd.projected2SigmaRightFreq : equalValueMatch.projected2SigmaRightFreq;
      double tma = wtd ? equalValueMatchWtd.measuredValue : equalValueMatch.measuredValue;
      double tmaf = wtd ? equalValueMatchWtd.measuredFreq : equalValueMatch.measuredFreq;
      return dStats(leader,dw,wtd,ptm,ptmf,tm2sl,tm2sr,tm2slf,tm2srf,tma,tmaf); 
   }       
        
   string matchStats(const string& leader, bool flm, int dw, bool wtd = true) const { 
      if (flm) { 
         return firstMoveMatchStats(leader,dw,wtd); 
      } else { 
         return topMatchStats(leader,dw,wtd); 
      } 
   //return (flm ? firstMoveMatchStats(leader,dw) : topMatchStats(leader,dw)); 
   //causes SegFault on CC -fast on "nussex" and "castor", not "pegasus"... 
   } 
       
   /** Call with unweighted PerfData object to get Unwtd stats 
 *     to-do (??) !!!
    */ 
   string falloffStats(const string& hl, bool scaled, int dw, bool wtd = true) 
   const { 
      string headline = hl + (scaled ? ", scaled" : ", not scaled"); 
      double pe = scaled ? (wtd ? scaledFalloffWtd.projectedValue : scaledFalloff.projectedValue) 
                         : (wtd ? unscaledFalloffWtd.projectedValue : unscaledFalloff.projectedValue); 
      double pef = scaled ? (wtd ? scaledFalloffWtd.projectedFreq 
                                 : scaledFalloff.projectedFreq) 
                          : (wtd ? unscaledFalloffWtd.projectedFreq  
                                 : unscaledFalloff.projectedFreq); 
      double pe2sl = scaled ? (wtd ? scaledFalloffWtd.projected2SigmaLeft 
                                   : scaledFalloff.projected2SigmaLeft) 
                            : (wtd ? unscaledFalloffWtd.projected2SigmaLeft 
                                   : unscaledFalloff.projected2SigmaLeft); 
      double pe2sr = scaled ? (wtd ? scaledFalloffWtd.projected2SigmaRight 
                                   : scaledFalloff.projected2SigmaRight) 
                            : (wtd ? unscaledFalloffWtd.projected2SigmaRight 
                                   : unscaledFalloff.projected2SigmaRight); 
      double pe2slf= scaled ? (wtd ? scaledFalloffWtd.projected2SigmaLeftFreq 
                                   : scaledFalloff.projected2SigmaLeftFreq) 
                            : (wtd ? unscaledFalloffWtd.projected2SigmaLeftFreq 
                                   : unscaledFalloff.projected2SigmaLeftFreq); 
      double pe2srf= scaled ? (wtd ? scaledFalloffWtd.projected2SigmaRightFreq 
                                   : scaledFalloff.projected2SigmaRightFreq) 
                            : (wtd ? unscaledFalloffWtd.projected2SigmaRightFreq 
                                   : unscaledFalloff.projected2SigmaRightFreq); 
      double ae = scaled ? (wtd ? scaledFalloffWtd.measuredValue : scaledFalloff.measuredValue) 
                         : (wtd ? unscaledFalloffWtd.measuredValue : unscaledFalloff.measuredValue); 
      double aef = scaled ? (wtd ? scaledFalloffWtd.measuredFreq : scaledFalloff.measuredFreq) 
                        : (wtd ? unscaledFalloffWtd.measuredFreq : unscaledFalloff.measuredFreq); 
      return dStats(headline,dw,wtd,pe,pef,pe2sl,pe2sr,pe2slf,pe2srf,ae,aef); 
   } 
 
   string fullReport(size_t numIndices = 15, bool clumpRest = false) const; 
   string fullReportOld() const;
 
   string movesReport(bool flm = true) const { 
      string out = moveHeader() + "\n"; 
      for (int i = 0; i < turnsData->size(); i++) { 
         out += moveLine(turnsData->at(i),flm); 
         out += "\n"; 
      } 
      return out; 
   } 
}; 
 

struct IPRstats {    //Note: IPR uses the /projected/ freq from pfElo
   TrialSpec ts;
   TestItem ipr;    //Use projected variance as measured variance
   TestItem iprAdj;
   TestItem iprAuto;
   TestItem iprAdjByAuto;
   double autoRatio;
   double ownSigma;
   IPRstats(const PerfData& pfElo, const PerfData& pfSigmas, const TrialSpec& gts)
    : ts(gts)
    , ipr(TestItem("IPR", 4, 0, 
            IRfun::ipr(pfElo.scaledFalloffWtd.projectedFreq, gts.weightMethodChoice),
            IRutil::sqr(IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq, gts.weightMethodChoice)
                        - IRfun::ipr(pfSigmas.scaledFalloffWtd.projected2SigmaLeftFreq, gts.weightMethodChoice))/4.0,
            IRfun::ipr(pfElo.scaledFalloffWtd.projectedFreq, gts.weightMethodChoice),
            IRutil::sqr(IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq, gts.weightMethodChoice)
                        - IRfun::ipr(pfSigmas.scaledFalloffWtd.projected2SigmaLeftFreq, gts.weightMethodChoice))/4.0,
      1.0, 1.0, false))
    , iprAdj(TestItem("IPRadj", 4, 0,
            IRfun::ipr(pfElo.scaledFalloffWtd.projectedFreq, gts.weightMethodChoice),
            IRutil::sqr(IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq, gts.weightMethodChoice)
                        - IRfun::ipr(pfSigmas.scaledFalloffWtd.projected2SigmaLeftAdjFreq,gts.weightMethodChoice))/4.0,
            IRfun::ipr(pfElo.scaledFalloffWtd.projectedFreq,gts.weightMethodChoice),
            IRutil::sqr(IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq,gts.weightMethodChoice)
                        - IRfun::ipr(pfSigmas.scaledFalloffWtd.projected2SigmaLeftAdjFreq,gts.weightMethodChoice))/4.0,
      1.0, 1.4, true))

    , iprAuto(TestItem("IPRauto", 4, 0,
            //IRfun::ipr(pfSigmas.scaledFalloffWtd.measuredFreq,gts.weightMethodChoice),
            //Doesn't matter: projected = actual-measured on the person's own games
            IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq,gts.weightMethodChoice),
            IRutil::sqr(IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq,gts.weightMethodChoice)
                        - IRfun::ipr(pfSigmas.scaledFalloffWtd.projected2SigmaLeftFreq,gts.weightMethodChoice))/4.0,
            IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq,gts.weightMethodChoice),
            IRutil::sqr(IRfun::ipr(pfSigmas.scaledFalloffWtd.projectedFreq,gts.weightMethodChoice)
                        - IRfun::ipr(pfSigmas.scaledFalloffWtd.projected2SigmaLeftFreq,gts.weightMethodChoice))/4.0,
      1.0, 1.0, false))

    , autoRatio(IRutil::divifgt0(pfSigmas.scaledFalloffWtd.measuredFreq, pfSigmas.scaledFalloffWtd.projectedFreq))
    , ownSigma(pfSigmas.scaledFalloffWtd.measuredSigmaFreq)
   {
      iprAdjByAuto = TestItem("IPRadjbyauto", 4, 0,
            IRfun::ipr(pfElo.scaledFalloffWtd.projectedFreq*autoRatio,gts.weightMethodChoice),
            iprAuto.measuredVariance,
            IRfun::ipr(pfElo.scaledFalloffWtd.projectedFreq*autoRatio,gts.weightMethodChoice),
            iprAuto.projectedVariance,
      1.0, 1.0, false);
   }

   ~IPRstats() {
      //delete(ipr);
      //delete(iprAdj);
      //delete(iprAuto);
      //delete(iprAdjByAuto);
   }
};


#endif   //end of #ifndef __IR_PERFDATA_H__ 
 
 
 
