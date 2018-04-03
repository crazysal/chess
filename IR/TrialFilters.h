//File-------------------TrialFilters.h------------------------------- 
 
#ifndef __IR_TRIAL_FILTERS_H__ 
#define __IR_TRIAL_FILTERS_H__ 
 
 
/** Parent class for all filters that depend on a certain Trial and its spec. 
    Logically abstract, but must have constructor to handle "name" field, 
    since "protected" is 1 level only in C++ and friending would be 
    kludgey here. 
 */ 
class TrialFilter : public virtual TurnFilter { 
 protected: 
   const Trial* theTrial; 
   const bool fixDistortion; 
 public: 
   TrialFilter(const Trial* trial, const int fd, const string& gname) 
         : TurnFilter(gname), theTrial(trial), fixDistortion(fd) { 
      name = gname; 
   } 
   virtual ~TrialFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const = 0; 
   virtual string getDescription() const { 
      return "TrialFilter"; 
   } 
}; 
 
class WeightFilter : public CompareFilter, TrialFilter { 
   const Models::MODEL model; 
 public: 
   WeightFilter(const Trial* trial, const int fd, const Models::MODEL gmodel, 
                const COMPARE comp, const double threshold, const string& name) 
      : TurnFilter(name)
      , CompareFilter(comp, threshold, name)
      , TrialFilter(trial, fd, name)
      , model(gmodel)
     { } 
   virtual ~WeightFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      DecisionInfo* decision = theTrial->translateTurn(pTurn);  //not stored 
      TrialSpec ts = theTrial->getSpec(); 
      double weight=1; 
      //double weight = Models::getWeight(tuple,ts, Models::getProbs(tuple,ts,model)); 
      delete(decision);
      return test(weight); 
   } 
   virtual string getDescription() const { 
      return "Weight " + CompareFilter::getDescription(); 
   } 
}; 
 
class ProbIFilter : public CompareFilter, TrialFilter { 
   const Models::MODEL model; 
   const int index; 
 public: 
   ProbIFilter(const Trial* trial, const int fd, const Models::MODEL gmodel, 
               const COMPARE comp, const double threshold, const int gindex, 
               string name) 
      : TurnFilter(name)
      , CompareFilter(comp, threshold, name)
      , TrialFilter(trial, fd, name)
      , model(gmodel)
      , index(gindex) 
     { } 
   virtual ~ProbIFilter() { } 
   bool apply(const TurnInfo* const pTurn) const { 
      DecisionInfo* decision = theTrial->translateTurn(pTurn);  //not stored 
      TrialSpec ts = theTrial->getSpec(); 
	  //TAMAL: REDO 
      vector<double> probs;  // = Models::getProbs(tuple,ts,model); 
      bool ans; 
      if (probs.size() <= index) { 
         ans = false; 
      } else { 
         ans = test(probs.at(index)); 
      } 
      delete(decision);
      return ans; 
   } 
   virtual string getDescription() const { 
      return "Prob " + IRutil::itoa(index) + " "  
                     + CompareFilter::getDescription(); 
   } 
}; 

class TrialSelector : public virtual MoveSelector {
 protected:
   const Trial* theTrial;
public:
   TrialSelector(const Trial* trial, const bool nullExcludes, const string& gname)
         : MoveSelector(gname,nullExcludes), theTrial(trial) {
      //name = gname;
   }
   virtual ~TrialSelector() { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const = 0;
   virtual string getDescription() const {
      return "TrialSelector";
   }
};



class SwingNegativeSelector : public TrialSelector {
 public:
   SwingNegativeSelector(const Trial* trial, const bool excludeIfNull)
    : MoveSelector("SwingNegative", excludeIfNull)  //any derived of virtual must construct it first
    , TrialSelector(trial, excludeIfNull, "SwingNegative") 
   { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      bool ans = false;
      DecisionInfo* dec = theTrial->translateTurn(pTurn);
      for (int i = 0; i < pTurn->numConsideredMoves; i++) {
         if (pTurn->consideredMoves->at(i) == move) {
            ans = (dec->getSwing(i) < 0);
            break;
         }
      }
      delete(dec);
      return ans;
   }
   virtual string getDescription() const {
      return "Move has negative swing";
   }
};

class SwingNonNegativeSelector : public TrialSelector {
 public:
   SwingNonNegativeSelector(const Trial* trial, const bool excludeIfNull)
    : MoveSelector("SwingNonNegative", excludeIfNull)  //any derived of virtual must construct it first
    , TrialSelector(trial, excludeIfNull, "SwingNonNegative")
   { }
   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      bool ans = false;
      DecisionInfo* dec = theTrial->translateTurn(pTurn);
      for (int i = 0; i < pTurn->numConsideredMoves; i++) {
         if (pTurn->consideredMoves->at(i) == move) {
            ans = (dec->getSwing(i) >= 0);
            break;
         }
      }
      delete(dec);
      return ans;
   }
   virtual string getDescription() const {
      return "Move that has zero or positive swing";
   }
};


class MoveScaledSwingSelector : public CompareSelector, TrialSelector {
   double thresholdSwing;  //can be negative
   size_t targetDepth;     //D in definition of swing, 0 for max
   bool normalize;
 public:
   MoveScaledSwingSelector(const Trial* trial, const COMPARE comp,
                           const double gdelta, const size_t gdepth,
                           const bool norm, bool nullExcludes, const string& name)
    : MoveSelector(name, nullExcludes)
    , CompareSelector(comp, gdelta, nullExcludes, name)
    , TrialSelector(trial, nullExcludes, name)
    , thresholdSwing(gdelta)
    , targetDepth(gdepth)
    , normalize(norm)
   { }
   virtual ~MoveScaledSwingSelector() { }

   virtual bool apply(const TurnInfo* const pTurn, const Move& move) const {
      bool ans = false;
      DecisionInfo* dec = theTrial->translateTurn(pTurn);
      size_t i = 0;
      size_t useDepth = targetDepth;
      while (i < pTurn->numConsideredMoves) {
         if (pTurn->consideredMoves->at(i) == move) {
            if (targetDepth == 0 || targetDepth > pTurn->turnHighestDepth) {
               useDepth = pTurn->turnHighestDepth;
            } else if (targetDepth < pTurn->turnLowestDepth) {
               useDepth = pTurn->turnLowestDepth; //will return 0 for swing
            }
            //double swingToTest = dec->getSwingScaled(i, useDepth, normalize);
            double swingToTest = dec->getDecMoveSwing(i);
            ans = test(swingToTest);
            break;
         }
         i++;
      }
      delete(dec);
      return ans;
   }

   virtual string getDescription() const {
      string dstr = (targetDepth == 0 ? "max" : IRutil::itoa(targetDepth));
      string normstr = (normalize ? "normalized is " : "is ");
      return "Scaled swing " + normstr + relation() + " " + IRutil::ftoa(thresholdSwing)
            + " targeting depth " + dstr;
   }
};

 
#endif    //end of #ifndef __TRIAL_FILTERS_H__ 
 

