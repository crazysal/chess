//File-----------------------DeltaScales.h------------------------------------ 
 
#ifndef __IR_DELTA_SCALES_H__ 
#define __IR_DELTA_SCALES_H__ 
 
//------------------------SCALE MODIFICATION---------------------------------- 
 
 
enum DELTA_SCALES {   //parallel code with Ensemble::scalesMenu 
   NO_SCALING,        //a placeholder, also used for debugging 
   LINEAR_SCALE,      //can differ from NO_SCALING in slope and when pinned 
   MUL_SCALE,         //multiplies delta by 1/(1+cx)^pow
   MIX_SCALE,
   DIVLOG_SCALE, 
   LOG_SCALE,         //implemented as a special case of class FractalScale 
   FRACTAL_SCALE, 
   FULL_FRACTAL_SCALE, 
   NUM_DELTA_SCALES    
}; 
 
enum DELTA_PATCHES { 
   NO_PATCH = NUM_DELTA_SCALES, 
   MATCH_PROBS,       //hardwire 58% fix 
   BALANCE_TIES,      //give playedMoveIndex random place among tied moves 
   UP_TIED_TOP,       //add equalTopFix to all tied deltas 
   PUSH_TIED_TOP,     //add fix progressively to top ties and following deltas 
   PUSH_ALL_TIES,     //add fix progressively to all ties and following deltas 
   NUM_DELTA_PATCHES 
}; 
    
 
/** Translate evaluation differences ("deltas") as reported by chess  
    engines in "raw scale" to scales that might be perceived by players.   
    The motivation is that a tenth-of-a-pawn difference may not matter as 
    much between moves that are already inferior by 1-2 pawns, *and/or* when 
    the position itself favors the stronger side by 1-2 pawns, either now 
    or on the previous turn.   
 
    The re-scaling is intended to depend only on the "facts on the board", 
    not on the skill of the players, so it is applied while filtering 
    TurnContext down to TupleInfo, not while doing fitting.  This also takes 
    it away from the most speed-critical code, so we use function objects. 
    The re-scaling is not absorbed by the regression because it is stepwise, 
    with a "plateau" of evals where unit-scale is applied. 

    This class also handles the four main modes of operation with the deltas,
    depending on whether bool weightEvalsFirst is set true or false, and on
    whether swing is applied.  Here are the combinations:

    1. userLowDepth < userHighDepth && !weightEvalsFirst: The "Rolls Royce" option,
       complete with gas (time) guzzling.  Each depth in that range is treated as
       a separate regression, yielding a probability judgment using each depth's
       data only.  Then the probabilities for each move are combined using weights 
       specified in the class Trial and depending on the "d" and "v" parameters.
    2. userLowDepth < userHighDepth && weightEvalsFirst: The deltas for each
       depth are aggregated into one vector of deltas for a single regression.
       Swing is not used---rather the aggregation of deltas takes care of that
       in-tandem with the "d" and "v" parameters ("v" may always be fixed).
    3. userLowDepth == userHighDepth (== D) && !weightEvalsFirst: This has the same
       effect as just using the one depth D for judgment; d and v are ignored.
    4. userLowDepth == userHighDepth (== D) && weightEvalsFirst: This differs by 
       unlocking "Swing" and the userSwingLow..userSwingHigh range.  Differences
          delta(i,d) - delta(i,D)
       are aggregated using a similar vector of weights from class Trial, which
       in turn are updated with any change to "d" and (optionally) "v".
 */ 
class DeltaScale : public Catalogable { 
   string name; 
   bool weightEvalsFirst;
   //string swingPolicy;
   SwingInfo* swingPolicy;
   //APPLICATION_POLICIES applicationPolicy;
   //bool scaleThere;
   //bool depthDependentSwings;
   DELTA_PATCHES patchPolicy; 
   double patchPower;
   double equalTopFix;
   //double normFactor;
   map<string, double> normFactors;
   int zeroDepth;   //Used for Poisson weights (and the origin of swing?).
   size_t userLowDepth;
   size_t userHighDepth;
   size_t userSwingLow;
   size_t userSwingHigh;
   size_t judgmentDepth;
 protected:
   double deltaCap;

/*
   static APPLICATION_POLICIES parsePolicy(string swp) {
      bool join = (swp.find("join") != string::npos);
      bool link = (swp.find("link") != string::npos);
      bool sep = (swp.find("sep") != string::npos);
      bool proxy = (swp.find("proxy") != string::npos);
      bool rel1 = (swp.find("rel1") != string::npos || swp.find("relative1") != string::npos);
      bool rel2 = (swp.find("rel2") != string::npos || swp.find("relative2") != string::npos);
      bool rel3 = (swp.find("rel3") != string::npos || swp.find("relative3") != string::npos);
      bool rel = (swp.find("rel") != string::npos || swp.find("relative") != string::npos);
      if (join) {
         if (rel1) {
            return JOIN_REL1;
         } else if (rel2) {
            return JOIN_REL2;
         } else if (rel3) {
            return JOIN_REL3;
         } else if (rel) {
            return JOIN_REL1;
         } else {
            return JOIN_ABS;
         }
      } else if (link) {
         if (rel1) {
            return LINK_REL1;
         } else if (rel2) {
            return LINK_REL2;
         } else if (rel3) {
            return LINK_REL3;
         } else if (rel) {
            return LINK_REL1;
         } else {
            return LINK_ABS;
         }
      } else if (sep) {
         if (rel1) {
            return SEPARATE_REL1;
         } else if (rel2) {
            return SEPARATE_REL2;
         } else if (rel3) {
            return SEPARATE_REL3;
         } else if (rel) {
            return SEPARATE_REL1;
         } else {
            return SEPARATE_ABS;
         }
      } else if (proxy) {
         if (rel1) {
            return PROXY_REL1;
         } else if (rel2) {
            return PROXY_REL2;
         } else if (rel3) {
            return PROXY_REL3;
         } else if (rel) {
            return PROXY_REL1;
         } else {
            return PROXY_ABS;
         }
      } else {
         return NO_SWING;
      }
   }
*/

 public: 
   explicit DeltaScale(const string& gname, 
                       bool wef,
                       //string swp = "0(separate,relative2,damped)",
                       SwingInfo* swp,
                       DELTA_PATCHES gpatchPolicy = NO_PATCH,
                       double gppwr = IRutil::CENTRALPATCHPOWER,
                       double etf = 0.0,
                       //double nf = 1.0,
                       map<string,double> nf = map<string,double>(),
                       int zd = 1,
                       size_t ld = 30,
                       size_t hd = 30,
                       size_t swld = 5,
                       size_t swhd = 19,
                       size_t jd = 0,
                       double cap = IRutil::DELTACAP)
    : name(gname), weightEvalsFirst(wef), swingPolicy(swp), 
      //applicationPolicy(parsePolicy(swp)), scaleThere(swp.find("scale") != string::npos),
      //depthDependentSwings(swp.find("relative") != string::npos 
                           //|| swp.find("boxed") != string::npos),
      patchPolicy(gpatchPolicy),  patchPower(gppwr), equalTopFix(etf),
      normFactors(nf), zeroDepth(zd), userLowDepth(ld), userHighDepth(hd),
      userSwingLow(swld), userSwingHigh(swhd), judgmentDepth(jd), deltaCap(cap)
   { } 
   virtual ~DeltaScale() { } 
   virtual double mapDelta(const double delta, const TurnContext* const t) const = 0; 

   bool weightsEvalsFirst() const { return weightEvalsFirst; }
   double getEqualTopFix() const { return equalTopFix; } 
   DELTA_PATCHES getPatchPolicy() const { return patchPolicy; } 
   double getPatchPower() const { return patchPower; }
   double getTransPower() const { return swingPolicy->transPower; }
   map<string,double> getNormFactors() const { return normFactors; }

   string normFactorString() const {
      ostringstream oss;
      map<string, double>::const_iterator mitr = normFactors.begin();
      map<string, double>::const_iterator mite = normFactors.end();
      while (mitr != mite) {
         oss << "(*" << (*mitr).first << "*," << (*mitr).second << ") ";
         ++mitr;
      }
      return oss.str();
   }

   void addNormFactor(const string& key, const double value) { 
      normFactors[key] = value;  //overwrites since map not multimap
   }
   double lookupNF(const TurnContext* const tc, const double def = 1.0) const {
      map<string, double>::const_iterator mitr = normFactors.begin();
      map<string, double>::const_iterator mite = normFactors.end();
      while (mitr != mite) {
         if (tc->engineID.find((*mitr).first) != string::npos) {
            return (*mitr).second;
         }
         ++mitr;
      }
      return def;
   }
      
         

   //void setPatchPolicy(DELTA_PATCHES newPolicy) { patchPolicy = newPolicy; } 
   void setWEF(bool wef) { weightEvalsFirst = wef; }
   void setSwingPolicy(SwingInfo* swp) { swingPolicy = swp; }
   //void setApplicationPolicy(APPLICATION_POLICIES ap) { applicationPolicy = ap; }
   //void setApplicationPolicy(string swp) { applicationPolicy = parsePolicy(swp); }
   //void setScaleThere(string swp) { scaleThere = (swp.find("scale") != string::npos); }
   void setLowDepth(size_t lo) { userLowDepth = lo; }
   void setHighDepth(size_t hi) { userHighDepth = hi; }
   void setSwingLow(size_t swlo) { userSwingLow = swlo; }
   void setSwingHigh(size_t swhi) { userSwingHigh = swhi; }
   void setZeroDepth(int zd) { zeroDepth = zd; }
   void setJudgmentDepth(size_t jd) { judgmentDepth = jd; }
   void setDeltaCap(double newCap) { deltaCap = newCap; }
   void extendName(string ext) { name += ext; }
   
   double getDeltaCap() const { return deltaCap; }
   SwingInfo* getSwingPolicy() const { return swingPolicy; }
   //APPLICATION_POLICIES getApplicationPolicy() const { return applicationPolicy; }
   //bool scalesThere() const { return scaleThere; }
   //bool hasDepthDependentSwings() const { return depthDependentSwings; }
   size_t getLowDepth() const { return userLowDepth; }
   size_t getHighDepth() const { return userHighDepth; }
   size_t getSwingLow() const { return userSwingLow; }
   size_t getSwingHigh() const { return userSwingHigh; }
   int getZeroDepth() const { return zeroDepth; }
   size_t getJudgmentDepth() const { return judgmentDepth; }

   virtual string getName() const { 
      return name;
      //ostringstream oss;
      //oss << name << "depths[" << userLowDepth << ".." << userHighDepth << "]";
      //if (weightEvalsFirst) { oss << "WEF"; }
      //return oss.str();
   } 
   virtual string getDescription() const { 
      string policy; 
      switch(patchPolicy) { 
       case NO_PATCH: 
         policy = "using raw probabilities"; 
         break; 
       case MATCH_PROBS: 
         policy = "patching probabilities"; 
         break; 
       case BALANCE_TIES: 
         policy = "balancing ties"; 
         break; 
       case UP_TIED_TOP: 
         policy = "equal top deltas add " + IRutil::ftoa(getEqualTopFix(),3); 
         break; 
       case PUSH_TIED_TOP: 
         policy = "equal top deltas push " + IRutil::ftoa(getEqualTopFix(),3); 
         break; 
       case PUSH_ALL_TIES: 
         policy = "all equal deltas push " + IRutil::ftoa(getEqualTopFix(),3); 
         break; 
       default: 
         policy = "Unknown policy"; 
      } 
      return "; " + policy + normFactorString(); 
   } 
   virtual operator string() const {  
      return getName() + ":\n" + getDescription(); 
   } 
}; 
 
 
/** Placeholder class---implementations use Boolean option to avoid re-scaling. 
 */ 
class UnitScale : public DeltaScale { 
 public: 
   UnitScale(bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double equalTopFix,
             map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd,
             size_t jd, double cap) 
    : DeltaScale("UnitScale", wef, swp, pp, ppwr, equalTopFix, nf, zd, ld, hd, sld, shd, jd, cap) 
   { } 
   virtual double mapDelta(const double delta, const TurnContext* const turn) const { 
      return delta; 
   } 
/* 
   virtual string getDescription() const {  
      return "always 1.0; equal-top deltas add "  
               + IRutil::ftoa(getEqualTopFix(),3);  
   } 
*/ 
}; 
 

 
/** Abstract base class for defining any scale by differential means. 
    This allows scales that don't have good closed formulas, and is 
    intuitively more convenient even for those that do.  Implements  
    mapDelta(x,turn) but has new abstract function lineElement(x,turn). 
    Note classes TranslateByEval and StraddleEvals, which assumes 
    CLASS INV: the LineElement admits affine translation for positive x  
    and is interpretable as being symmetric for negative x. 
 */ 
class LineScale : public DeltaScale {     
   double epsilon; 
 public: 
   double getEpsilon() const { return epsilon; } 
 
   LineScale(const string& name, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr,
             double equalTopFix, map<string,double> nf, 
             int zd, size_t ld, size_t hd, size_t sld, size_t shd, size_t jd, double eps)  
      : DeltaScale(name, wef, swp, pp, ppwr, equalTopFix, nf, zd, ld, hd, sld, shd, jd)
      ,  epsilon(eps)  
   { } 
 
   /** Formally defined only for x >= 0, but assumed symmetric for x < 0 
    */ 
   virtual double lineElement(const double x, const TurnContext* turn) const = 0; 
 
   /************************************************************************* 
   Integration using the Simpson method with accuracy estimation. 
   Adapted from ALGLIBC code by Sergey Bochkanov, (C) 2005-2007. 
   The integral of function f over [a,b] is calculated with the 
   accuracy of order epsilon.  KWR: lineElement(x, turn) plays the role of f. 
      KWR: Thus far the only info extracted from the turn is the eval e  
   used as an offset, and this is used only as a subtractive translation.   
   Instead of integrating from 0 to delta, we could go from -e to -e + delta. 
   But this integral could straddle 0 and hence need two pieces, and we'd 
   still need e as a second parameter to mapDelta anyway, not a class field. 
   More important, since this is not the most time-critical code, it is 
   better to be clear about how the line-element itself depends on the e. 
   *************************************************************************/ 
   double integralSimpson(const double a, const double b, 
                          const TurnContext* turn) const { 
       double s2 = 1.0; 
       double h = b - a; 
       double s = lineElement(a,turn) + lineElement(b,turn); 
       double s1, s3; 
       double x; 

if (h < 0.0) {
   cerr << "negative case: a = " << a << ", b = " << b << ", turn " 
        << (turn->operator string()) << endl;
   return 0.0;
}
       do { 
           s3 = s2; 
           h = h/2; 
           s1 = 0.0; 
           x = a + h; 
           do { 
               s1 += 2*lineElement(x,turn); 
               x += 2*h; 
           } while (x < b); 
           s += s1; 
           s2 = (s + s1)*h/3.0; 
           x = fabs(s3 - s2)/15.0; 
       } while (x > epsilon); 
       return s2; 
   } 
 
   double mapDelta(const double delta, const TurnContext* const turn) const { 
      return integralSimpson(0.0, lookupNF(turn)*delta, turn); 
      //!! normalization of engines is done only here
   } 
   //still leaves getDescription() abstract 
}; 


/** An example of a direct mapping scale.
    Multiply delta by 1/(1 + gx) if x > 0, 1(1 + g'x) if treatNegSeparately && x < 0.
    The player-to-move eval comes from the TurnContext.
    Implicitly assumes that in the observed relation in positions of value x:

         avgdelta(x)  =  a_E  +  b_E * x,

    the ratio g = b_E/a_E (for x > 0) is independent of the Elo rating E.
    That's not quite true but is a reasonable approximation.
    The constant g (or its reciprocal) is called the "gradient" in the scales menu and below.
    The different g' is called the negGradient---there is observed to be a
    different limit a'_E > a_E as x approaches 0 from below (see "Firewall at Zero" post).
    The eval x is obtained from the TurnContext item.
 */
class MulScale : public LineScale {
   double radius;
   double power;
   double gradient;
   double negGradient;
   bool splitNegative;

 public:
   MulScale(string baseName, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double equalTopFix,
            map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd, size_t jd,
            double r, double pwr, double gr, double ngr, bool split)
    : LineScale(baseName, wef, swp, pp, ppwr, equalTopFix, nf, 
                zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON)
    , radius(r), power(pwr), gradient(gr), negGradient(ngr), splitNegative(split)
   { }
/*
   virtual double mapDelta(const double delta, const TurnContext* turn) const {
      double g = (turn->currEval > 0 ? gradient : negGradient);
      if (delta <= radius) {
         return delta;
      } else {
         return delta/pow(1.0 + fabs(turn->currEval)*g, power);
      }
   }
*/

   virtual double lineElement(const double x, const TurnContext* turn) const {
      double g = ((splitNegative && turn->currEval < 0) ? negGradient : gradient);
      if (x <= radius) {
         return 1.0;
      } else {
         return 1.0/pow(1.0 + fabs(turn->currEval)*g, power);
      }
   }

   virtual double mapDelta(const double delta, const TurnContext* const turnc) const {
      double g = ((splitNegative && turnc->currEval < 0) ? negGradient : gradient);
      double absEval = fabs(turnc->currEval);
      return (absEval <= radius ? delta : delta/pow(1.0 + absEval*g, power));
   }

   virtual string getDescription() const {
      return "Multiplies delta by 1/(1 + g*x)^pow";
   }
};

/** SubScale has a different theory: it applies if in the equation

         avgdelta(x)  =  a_E  +  b_E * x,

    the "b_E" is close to constant.  So this parameter is called beta for x > 0
    and negBeta for x < 0.  Currently (7/29/16), SubScale is not used in the code;
    it is present only to help motivate MixScale.
 */
class SubScale : public LineScale {
   double radius;
   double power;    //keep at 1
   double beta;
   double negBeta;
   bool splitNegative;
 public:
   SubScale(string baseName, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double equalTopFix,
            map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd, size_t jd,
            double r, double pwr, double gb, double ngb, bool split)
    : LineScale(baseName, wef, swp, pp, ppwr, equalTopFix, nf, 
                zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON)
    , radius(r), power(pwr), beta(gb), negBeta(ngb), splitNegative(split)
   { }

   virtual double lineElement(const double x, const TurnContext* turn) const {
      return 0.0;
   }

   virtual double mapDelta(const double delta, const TurnContext* const turnc) const {
      double b = ((splitNegative && turnc->currEval < 0) ? negBeta : beta);
      double absEval = fabs(turnc->currEval);
      return ((absEval <= radius || delta < absEval*b) ? delta : delta - pow(absEval*b, power));
   }

   virtual string getDescription() const {
      return "Subtracts (bx)^power from delta";
   }
};

/** Since neither assumption is shown to hold, MixScale attempts to flatten in a way that
    compensates by sliding a weighting factor "gamma" as the errors delta get bigger:  Let

                    alpha = tanh(gamma*(delta - beta*x))
    
    Then return alpha*SubScale + (1-alpha)*MulScale.  Note that since tanh(0) = 0, for
    delta = beta*x we just get MulScale, and the code ensures continuity by applying only
    MulScale for delta smaller than beta*x too.  Since a_E is close to the same for the
    negative-side relation, we can presume that negBeta/beta ~= negGradient/gradient.
    Thus the strategy is that gradient and negGradient should be taken from one end (say, 2800)
    and beta from the other (say, Elo 1050), and gamma adjusted as a "slider" between them.
 */
class MixScale : public LineScale {
   double radius;
   double power;    //keep at 1
   double gradient;
   double negGradient;
   bool splitNegative;
   double beta;     //the b in the implicit SubScale may differ from "gradient".
   double gamma;    //gauges how to weight the linear combination vis-a-vis delta
 public:
   MixScale(string baseName, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double equalTopFix,
            map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd, size_t jd,
            double r, double pwr, double gr, double ngr, bool split, double gbeta, double ggamma)
    : LineScale(baseName, wef, swp, pp, ppwr, equalTopFix, nf, 
                zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON)
    , radius(r), power(pwr), gradient(gr), negGradient(ngr), splitNegative(split), beta(gbeta), gamma(ggamma)
   { }

   virtual double lineElement(const double x, const TurnContext* turn) const {
      return 0.0;
   }

   virtual double mapDelta(const double delta, const TurnContext* const turnc) const {
      double g = ((splitNegative && turnc->currEval < 0) ? negGradient : gradient);
      double b = ((splitNegative && turnc->currEval < 0) ? beta : beta*negGradient/gradient);
      double absEval = fabs(turnc->currEval);
      double denom = pow(1.0 + absEval*g, power);
      double transDelta = delta - pow(absEval*b, power);
      if (absEval >= radius) {
         if (transDelta < 0) {
            return (delta/denom);
         } else {
            double alpha = tanh(gamma*transDelta);
            return (alpha*transDelta + (1.0-alpha)*delta/denom);
         }
      } else {
         return 1.0;
      }
   }

   virtual string getDescription() const {
      return "Mixes MulScale and subtracting (gx)^power from delta";
   }
};



 
 
class TranslateByEval : public LineScale { 
   LineScale* ls; 
 public: 
   TranslateByEval(LineScale* gls)  
      : LineScale("TranslateByEval [" + gls->getName() + "]",  gls->weightsEvalsFirst(), 
                  gls->getSwingPolicy(), gls->getPatchPolicy(), gls->getPatchPower(), 
                  gls->getEqualTopFix(), gls->getNormFactors(), gls->getZeroDepth(), 
                  gls->getLowDepth(), gls->getHighDepth(), gls->getSwingLow(), 
                  gls->getSwingHigh(), gls->getJudgmentDepth(), gls->getEpsilon()), 
        ls(gls)  
   { } 
   virtual double lineElement(const double x, const TurnContext* pTurn) const { 
      double origin = pTurn->whiteToMove ? pTurn->currEval : -(pTurn->currEval); 
      return ls->lineElement(fabs(x - origin), pTurn); 
   } 
   virtual double mapDelta(const double delta, const TurnContext* const turnc) const {
      return ls->mapDelta(delta, turnc);
   }
   virtual string getDescription() const { 
      return ls->getDescription() + "; scaling by eval"; 
   } 
}; 
 
class StraddleEvals : public LineScale { 
   LineScale* ls; 
 public: 
   StraddleEvals(LineScale* gls) 
      : LineScale("StraddleEvals [" + gls->getName() + "]",  gls->weightsEvalsFirst(), 
                  gls->getSwingPolicy(), gls->getPatchPolicy(), gls->getPatchPower(), 
                  gls->getEqualTopFix(), gls->getNormFactors(), gls->getZeroDepth(), 
                  gls->getLowDepth(), gls->getHighDepth(), gls->getSwingLow(), 
                  gls->getSwingHigh(), gls->getJudgmentDepth(), gls->getEpsilon()),
        ls(gls) { } 
   virtual double lineElement(const double x, const TurnContext* pTurn) const { 
      double origin = 0.0; 
      if (pTurn->currEval < 0.0 && pTurn->prevEval != IRutil::NAVAL && pTurn->prevEval < 0.0) { 
         origin = max(pTurn->currEval, pTurn->prevEval); 
      } else if (pTurn->currEval > 0.0 && pTurn->prevEval > 0.0) {   // uses NAVAL > 0
         origin = min(pTurn->currEval, pTurn->prevEval); 
      } 
      origin = pTurn->whiteToMove ? origin : -origin; 
      return ls->lineElement(fabs(x - origin), pTurn); 
   } 
   virtual string getDescription() const { 
      return ls->getDescription()  //"[" + ls->getName() + "]," 
                 + "; scaling midway between eval and prev-eval"; 
   } 
 
}; 
 
/** Make lineElement decrease to 0 at the cap value deltaCap. 
    Leaves value at x = stepStart (= radius) unchanged. 
    Could, but does not, assume that x <= deltaCap. 
    Squares adjustment to concentrate it at the far end. 
 */ 
class PinToCap : public LineScale { 
   LineScale* ls; 
   double stepStart;  // <= deltaCap 
   double deltaCap; 
 public:  
   PinToCap(LineScale* gls, double gstart, double gcap) 
      : LineScale("PinToCap("+IRutil::ftoa(gcap,2)+")[" + gls->getName() + "]",
                  gls->weightsEvalsFirst(), gls->getSwingPolicy(),
                  gls->getPatchPolicy(), gls->getPatchPower(), 
                  gls->getEqualTopFix(), gls->getNormFactors(), gls->getZeroDepth(), 
                  gls->getLowDepth(), gls->getHighDepth(), gls->getSwingLow(), 
                  gls->getSwingHigh(), gls->getJudgmentDepth(), gls->getEpsilon()),
        ls(gls), stepStart(gstart), deltaCap(gcap) 
   { } 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      double res = ls->lineElement(x,turn); 
      if (x >= deltaCap) { 
         res = 0.0; 
      } else if (x <= stepStart) { 
         //do nothing 
      } else { 
         res -= ls->lineElement(deltaCap,turn) 
                          * IRutil::sqr((x-stepStart)/(deltaCap-stepStart)); 
      } 
      return res; 
   } 
   virtual string getDescription() const { 
      return ls->getDescription() + "; Make differential zero at cap"; 
   } 
}; 
 
//class PureLineScale : public LineScale { 
    
/** Since Ensemble code expects a LineScale, this rather than class UnitScale 
    is used as a singleton class for a no-scaling placeholder. 
 */ 
class UnitLineScale : public LineScale { 
 public: 
   UnitLineScale(const string& name, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double etf,
                 map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd,
                 size_t jd) 
         : LineScale(name, wef, swp, pp, ppwr, etf, nf, zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON) 
   { } 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      return 1.0; 
   } 
   virtual string getDescription() const { 
      return "dx = always 1.0, NormFactors " + normFactorString()
                   + ", equal-top deltas add " 
                   + IRutil::ftoa(getEqualTopFix(),3); 
   } 
}; 
 
class LinearScale : public LineScale { 
   double radius; 
   double scalePower; 
   double gradient; 
 public: 
   LinearScale(const string& name, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double etf,
               map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd,
               size_t jd, double r, double sp, double gr) 
      : LineScale(name, wef, swp, pp, ppwr, etf, nf, zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON),
                  radius(r), scalePower(sp), gradient(gr) 
   { } 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      if (x <= radius) { 
         return 1.0; 
      } else {   //line with value "gradient" at x=radius and value 
                 //scalePower*gradient at x=IRutil::DELTACAP (unless pinned) 
                 //Hence setting gradient = scalePower = 1 yields unit scale. 
         //double width = IRutil::DELTACAP - radius; 
         double width = deltaCap - radius;
         double value = gradient*(1.0 + (scalePower - 1.0)*(x - radius)/width); 
         return max(0.0, value); 
      } 
   } 
   virtual string getDescription() const { 
      //double width = IRutil::DELTACAP - radius; 
      double width = deltaCap - radius;
      string desc; 
      if (gradient == 1.0) { 
         if (scalePower == 1.0) { 
            desc = "Equivalent to unit scale (unless pinned)"; 
         } else { 
            desc = "Linear with slope " + IRutil::ftoa((scalePower - 1)/width)  
                 + " after radius " + IRutil::ftoa(radius,2); 
         } 
      } else { 
         if (scalePower == 1.0) { 
            desc = "Flat with value " + IRutil::ftoa(gradient)  
                 + " after radius " + IRutil::ftoa(radius,2); 
         } else { 
            desc = "Linear with slope "  
                 + IRutil::ftoa(gradient*(scalePower - 1)/width)  
                 + " after value " + IRutil::ftoa(gradient) 
                 + " at radius " + IRutil::ftoa(radius,2); 
         } 
      } 
      desc += ",\nwith normFactors " + normFactorString()
                    + "and equal-top add " + IRutil::ftoa(getEqualTopFix(),3);
      return desc; 
   } 
}; 
 
 
 
 
/** For position values outside radius r of the origin, scale down by the 
    log of the multiple of r involved.  Withing radius r, scaling is 1. 
    If, say, radius = 0.5 pawns, then the difference between delta = 1.00 
    and delta = 1.01 is scaled with division by (1 + log2(1.00/0.5)) = 2. 
    For delta = 2.00, the next hundredth of a pawn is scaled down by 1+2 = 3. 
 
    Integrating this line element gives rise to the famous Li(x) function  
    from analytic number theory, which has no simple closed formula.   
    Since we use mainly for small values of x and have the 
    stepwise plateau feature---and this is called only while filtering, not 
    regressing---we use Simpson rather than x/log(x) to approximate. 
 
    To keep a common menu-interface with FractalScale next, we implement a 
    scale-factor parameter "c", and a gradient parameter "gr", making  
 
                         1/gr*(1 + c*log(x/r))  
 
    the actual line element.  CLASS INV: gr = 1.0 makes the line element 1.0 
    at x = r, making it continuous with the "plateau".  Since C++ "log" is  
    to base, we'd use c = ln(2) to make the log be base-2 as described above. 
    In FractalScale, the "c" plays a much larger role, but in both cases it 
    comes from (x/r)^c, and we encourage analogy to "c" in the regressions. 
 
    Also in these classes, the "turn" parameter is ignored---though it's used 
    when composed with TranslateBy[Prev]Eval.  Whether yet-another abstract 
    parent should remove it, and factor the fields (r,gr,c), is dubious. 
 */ 
class DivLogScale : public LineScale { 
   double radius; 
   double scalePower; 
   double gradient;   
 public: 
   DivLogScale(const string& name, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double etf,
               map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd,
               size_t jd, double r, double sp, double gr)  
      : LineScale(name, wef, swp, pp, ppwr, etf, nf, zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON),
        radius(r), scalePower(sp), gradient(gr) 
   { } 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      if (x <= radius) { 
         return 1.0; 
      } else { 
         return 1.0/(gradient * (1.0 + scalePower*log(x/radius))); 
      } 
   } 
   virtual string getDescription() const { 
      string desc; 
      if (gradient == 1.0) { 
         if (scalePower == 1.0) { 
            desc = "1/(1 + ln(x/r)) with r = " + IRutil::ftoa(radius,2); 
         } else { 
            desc = "1/(1 + c*ln(x/r)) with r = " + IRutil::ftoa(radius,2) 
                      + ", c = " + IRutil::ftoa(scalePower); 
         } 
      } else { 
         if (scalePower == 1.0) { 
            desc = "1/g*(1 + ln(x/r)) with r = " + IRutil::ftoa(radius,2) 
                      + ", g = " + IRutil::ftoa(gradient); 
         } else { 
            desc = "1/g(1 + c*log(x/r)) with r = " + IRutil::ftoa(radius,2)  
                + ", c = " + IRutil::ftoa(scalePower)  
                + ", g = " + IRutil::ftoa(gradient); 
         } 
      } 
      desc += ",\nwith normFactors " + normFactorString()
                    + " and equal-top add " + IRutil::ftoa(getEqualTopFix(),3);
      return desc; 
   } 
}; 
 
/** Integrating a line element of 1/y from y=1 to y=z yields ln(z). 
    The effect is thus more drastic on outlier deltas than with DivLogScale. 
    We generalize this to allow 1/y^c, intended for c <= 1. 
 */ 
class FractalScale : public LineScale { 
   double radius; 
   double scalePower; 
   double gradient; 
 public: 
   FractalScale(const string& name, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double etf,
                map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd,
                size_t jd, double r, double sp, double gr) 
      : LineScale(name, wef, swp, pp, ppwr, etf, nf, zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON),
        radius(r), scalePower(sp), gradient(gr) 
   { } 
 public: 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      if (x <= radius) { 
         return 1.0; 
      } else { 
         return 1.0/(gradient * pow(x/radius, scalePower)); 
      } 
   } 
   virtual string getDescription() const { 
      string desc; 
      if (gradient == 1.0) { 
         if (scalePower == 1.0) { 
            desc = "Log-log scaling of deltas above r = "  
                        + IRutil::ftoa(radius,2); 
         } else { 
            desc = "1/(x/r)^c with r = " + IRutil::ftoa(radius,2) + ", c = " 
                + IRutil::ftoa(scalePower); 
         } 
      } else { 
         if (scalePower == 1.0) { 
            desc = "Log-log scaling of deltas above r = "  
                + IRutil::ftoa(radius,2) + ", g = " + IRutil::ftoa(gradient); 
         } else { 
            desc = "1/g*(x/r)^c) with r = " + IRutil::ftoa(radius,2) + ", c = " 
                + IRutil::ftoa(scalePower) + ", g = " + IRutil::ftoa(gradient); 
         } 
      } 
      desc += ",\nwith normFactors " + normFactorString()
                    + " and equal-top add " + IRutil::ftoa(getEqualTopFix(),3);
      return desc; 
   } 
}; 
 
/** Integrating a line element of 1/y from y=1 to y=z yields ln(z). 
    The effect is thus more drastic on outlier deltas than with DivLogScale. 
    We generalize this to allow 1/y^c, intended for c <= 1. 
    "Full" means radius r == 0; the radius is not polled.
 */ 
class FullFractalScale : public LineScale { 
   double scalePower; 
   double gradient; 
   double negGradient;
 public: 
   FullFractalScale(const string& name, bool wef, SwingInfo* swp, DELTA_PATCHES pp, double ppwr, double etf,
                    map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd,
                    size_t jd, double sp, double gr, double ngr) 
    : LineScale(name, wef, swp, pp, ppwr, etf, nf, zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON)
    , scalePower(sp), gradient(gr), negGradient(ngr) 
   { } 
 public: 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      double gr = (turn->whiteToMove ? gradient : negGradient);
      return 1.0/pow(1 + x*gr, scalePower); 
   } 
   virtual string getDescription() const { 
      string desc; 
      if (gradient == 1.0) { 
         if (scalePower == 1.0) { 
            desc = "Log(1 + ...) scaling"; 
         } else { 
            desc = "1/(1 + x)^c with c = " 
                + IRutil::ftoa(scalePower); 
         } 
      } else { 
         if (scalePower == 1.0) { 
            desc = "Log(1 + ...) scaling graded + by " + IRutil::ftoa(gradient)
            + "; - by " + IRutil::ftoa(negGradient); 
         } else { 
            desc = "1/(1 + gx)^c with c = " 
               + IRutil::ftoa(scalePower) + ", g = " + IRutil::ftoa(gradient)
            + ";" + IRutil::ftoa(negGradient); 
         } 
      } 
      desc += ", with normFactors " + normFactorString()
                    + " and equal-top add " + IRutil::ftoa(getEqualTopFix(),3);
      return desc; 
   } 
}; 
 
/** Version of FullFractalScale that doesn't lend itself to translation by eval
    but handles the eval origin directly.  Overrides mapDelta directly too.
    Activate by setting all translations "off" and choosing FullFractalScale
    in the first part of the scales menu.  TurnContext has eval from White's view.
 */
class SimpleFullFractalScale : public LineScale {
   double scalePower;
   double gradient;
   double negGradient;
   double gradientRatio; 
 public:
   SimpleFullFractalScale(const string& name, bool wef, SwingInfo* swp,
         DELTA_PATCHES pp, double ppwr, double etf,
         map<string,double> nf, int zd, size_t ld, size_t hd, size_t sld, size_t shd,
         size_t jd, double sp, double gr, double ngr)
    : LineScale(name, wef, swp, pp, ppwr, etf, nf, zd, ld, hd, sld, shd, jd, IRutil::LINESCALEEPSILON)
    , scalePower(sp), gradient(gr), negGradient(ngr), 
      gradientRatio(IRutil::divifgt0(gradient,negGradient))
   { }
 public:
   virtual double lineElement(const double x, const TurnContext* turn) const {
      double gr = (turn->whiteToMove ? gradient : negGradient);
      return 1.0/pow(1 + x*gr, scalePower);  //to be ignored
   }
   virtual double mapDelta(const double delta, const TurnContext* const turn) const {
      double evalPTM = (turn->whiteToMove ? turn->currEval : -1*turn->currEval);
      double postEval = evalPTM - delta;
      double term1 = (evalPTM > 0 ? log(1.0 + evalPTM*gradient)/gradient
                                  : -log(1.0 - evalPTM*negGradient)/negGradient);
      double term2 = (postEval > 0 ? -log(1.0 + postEval*gradient)/gradient
                                   : log(1.0 - postEval*negGradient)/negGradient);
      return (term1 + term2);
   }
   virtual string getDescription() const {
      string desc;
      if (gradient == 1.0) {
         if (scalePower == 1.0) {
            desc = "Log(1 + ...) scaling";
         } else {
            desc = "1/(1 + x)^c with c = "
                + IRutil::ftoa(scalePower);
         }
      } else {
         if (scalePower == 1.0) {
            desc = "Log(1 + ...) scaling graded + by " + IRutil::ftoa(gradient)
            + "; - by " + IRutil::ftoa(negGradient);
         } else {
            desc = "1/(1 + gx)^c with c = "
               + IRutil::ftoa(scalePower) + ", g = " + IRutil::ftoa(gradient)
            + ";" + IRutil::ftoa(negGradient);
         }
      }
      desc += ", with normFactors " + normFactorString()
                    + " and equal-top add " + IRutil::ftoa(getEqualTopFix(),3);
      desc += "; scaling from 0";
      return desc;
   }
};

 
 
/** Combine two line metrics by averaging, so that the differential within 
    the plateau stays at 1.0.  If we could assume mapDelta(x,turn) is  
    likewise linear, then we could make this class general for DeltaScale. 
    Assume they both have the same patch policy, use the one from ds1. 
 */ 
class BinaryScale : public LineScale { 
   //LineScale* ds1, ds2;    //Caught this common error before compiling! 
   LineScale* ds1; 
   LineScale* ds2; 
 public: 
   BinaryScale(LineScale* d1, LineScale* d2)  
      : LineScale("Binary [" + d1->getName() + ", " + d2->getName() + "]", 
                   d1->weightsEvalsFirst(),
                   d1->getSwingPolicy(),
                   d1->getPatchPolicy(),
                   d1->getPatchPower(),
                   min(d1->getEqualTopFix(), d2->getEqualTopFix()),  
                   d1->getNormFactors(),  //assumed same for d2
                   max(d1->getZeroDepth(), d2->getZeroDepth()),  //really should be equal
                   max(d1->getLowDepth(), d2->getLowDepth()),
                   min(d1->getHighDepth(), d2->getHighDepth()),  //ditto
                   max(d1->getSwingLow(), d2->getSwingLow()),
                   min(d1->getSwingHigh(), d2->getSwingHigh()),
                   max(d1->getJudgmentDepth(), d2->getJudgmentDepth()),
                   max(d1->getEpsilon(), d2->getEpsilon())), 
        ds1(d1), ds2(d2)  
   { } 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      return (ds1->lineElement(x,turn) + ds2->lineElement(x,turn))/2.0; 
   } 
   virtual string getDescription() const { 
      string desc = ""; 
      desc += "[" + ds1->getName() + ", " + ds2->getName() + "]"; 
      desc += ", equal-top deltas add " + IRutil::ftoa(getEqualTopFix(),3); 
      return desc; 
   } 
}; 
 
class TernaryScale : public LineScale { 
   LineScale* ds1; 
   LineScale* ds2; 
   LineScale* ds3; 
 public: 
   TernaryScale(LineScale* d1, LineScale* d2, LineScale* d3) 
      : LineScale("Ternary [" + d1->getName() + ", " + d2->getName() + ", " 
                      + d3->getName() + "]", 
                   d1->weightsEvalsFirst(),
                   d1->getSwingPolicy(),
                   d1->getPatchPolicy(),
                   d1->getPatchPower(),
                   min(min(d1->getEqualTopFix(), d2->getEqualTopFix()), 
                      d3->getEqualTopFix()), 
                   d1->getNormFactors(),
                   max(max(d1->getZeroDepth(), d2->getZeroDepth()), d3->getZeroDepth()),  
                   max(max(d1->getLowDepth(), d2->getLowDepth()), d3->getHighDepth()),
                   min(min(d1->getHighDepth(), d2->getHighDepth()), d3->getHighDepth()),  //ditto
                   max(max(d1->getSwingLow(), d2->getSwingLow()), d3->getSwingLow()),
                   min(min(d1->getSwingHigh(), d2->getSwingHigh()), d3->getSwingHigh()),
                   max(max(d1->getJudgmentDepth(), d2->getJudgmentDepth()), d3->getJudgmentDepth()),
                   max(max(d1->getEpsilon(), d2->getEpsilon()), 
                      d3->getEpsilon())), 
        ds1(d1), ds2(d2), ds3(d3) 
   { } 
   virtual double lineElement(const double x, const TurnContext* turn) const { 
      return (ds1->lineElement(x,turn) + ds2->lineElement(x,turn)  
                    + ds3->lineElement(x,turn))/3.0; 
   } 
   virtual string getDescription() const { 
      string desc = ""; 
      desc += "[" + ds1->getName() + ", " + ds2->getName()  
                  + ", " + ds3->getName() + "]"; 
      desc += ", equal-top deltas add " + IRutil::ftoa(getEqualTopFix(),3);  
      return desc; 
   } 
 
}; 
 
 
#endif    //end of #ifndef __IR_DELTA_SCALES_H__ 
 
 
