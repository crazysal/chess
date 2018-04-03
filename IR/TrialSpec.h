//File--------------------TrialSpec.h------------------------------------- 
 
#ifndef __IR_TRIAL_SPEC_H__ 
#define __IR_TRIAL_SPEC_H__ 
 
//------------------------SPECIFYING CURVES-------------------------------- 
 
// Whether to make curves and weights Catalogable, and whether to include 
// the line element in a TrialSpec, are tricky questions.  Curves and 
// weights are accessed continually during the regressions, so max performance 
// is needed, while the line element is applied only while filtering 
// TurnInfo lists down to TupleInfo lists.  Hence the current choices. 

 
class TrialSpec : public Catalogable { 
 public: 
   IRfun::CURVES curveChoice;   //maintained as first entry in curvesBook and used when size==1
   map<IRfun::CURVES, double> curveBook;
   size_t size;

   IRfun::WEIGHTS weightMethodChoice;

   
   //string argFormation;  // "out" or "in" or "proxy"
   string name;
   double s;  //settable, as are c and d too
   double c; 
   double d;  //depth peak, can be Poisson lambda---not necessarily integer.
              //assume DecisionInfo class adjusts depth between engines. 
   double v;  //coefficient of variation for d, defaults to 1.0  (Poisson)
              //value of 0 means only depth-d results used.
   double a;  //special power of negative swing.  (Was w: coefficient of swing.)
   double hm; //special coefficient of negative swing (besides s)
   double hp; //special coefficient of positive swing (besides s).
   double b;  //special power of positive swing---as with a, slack power over c.

   bool slides;  //
   IRfun::CURVES slideCurveChoice;
   double slideLo;
   double slideMid;
   double slideHi;
   double slidePower;
   double slideProp;

   double memoStep;
   double memoCap;
   size_t memoSize;
   vector<double> myMemoTable;  //INV: holds values of curveChoice(delta)

 
   TrialSpec(IRfun::CURVES gcc, map<IRfun::CURVES,double> cb, IRfun::WEIGHTS gwc, const string& gname, 
             double gs, double gc, double gd, double gv, double ga, double ghm, double ghp, double gb, 
             bool slide = false, IRfun::CURVES altCurveChoice = IRfun::FOLDEDLOGISTIC, double glo = 0.0, 
             double gmid = 1.5, double ghi = 3.0, double slp = 2.0, double maxp = 1.0,
             double memStep = 0.01, double memCap = 5.00)
      : curveChoice(gcc), curveBook(cb), size(cb.size()), weightMethodChoice(gwc), name(gname), 
        s(gs), c(gc), d(gd), v(gv), a(ga), hm(ghm), hp(ghp), b(gb), slides(slide), 
        slideCurveChoice(altCurveChoice), slideLo(glo), slideMid(gmid), slideHi(ghi), slidePower(slp),
        slideProp(maxp), memoStep(memStep), memoCap(memCap), memoSize(memoCap/memoStep + 1),
        myMemoTable(vector<double>(memoSize)) {

        if (memCap > 0.0) { memoize(); }  //change to passed-in memoTable pointer?
   } 
   TrialSpec() { } //needed to make "map" happy, but shouldn't be used 
   virtual ~TrialSpec() {  
      //delete(myMemoTable); //Don't delete---is shared/not pointer anymore anyway
   } 
 
   inline double evalCurves1(const double z) const {
      if (size <= 1 && (!slides)) {
         return IRfun::evalCurve1(curveChoice,z);
      } //else
      double val = 0.0;
      double denom = 0.0;
      double mul;
      IRfun::CURVES curve;
      map<IRfun::CURVES,double>::const_iterator cbit = curveBook.begin();
      map<IRfun::CURVES,double>::const_iterator cbite = curveBook.end();
      while (cbit != cbite) {
         curve = cbit->first;
         mul = cbit->second;
         denom += mul;
         val += mul*IRfun::evalCurve1(curve,z);
         ++cbit;
      }
      val = IRutil::divifgt0(val,denom);
      if (!slides) { return val; }
      //else
      double val2 = IRfun::evalCurve1(slideCurveChoice,z);
      double w = (z < slideLo ? 0 : (z < slideMid ? (z - slideLo)/(slideMid - slideLo)
                    : (z < slideHi ? (slideHi + z - 2.0*slideMid)/(slideHi - slideMid) : 2.0)))/2.0;
      double wa = pow(w,slidePower);
      double slider = slideProp*wa/(wa + pow(1.0 - w, slidePower));
      return ((1.0 - slider)*val + slider*val2);
   }

   inline double mevalCurves1(const double z) const {
      if (z < memoCap) {
         double dind = z/memoStep;
         size_t left = size_t(dind);
         return ((left+1.0-dind)*myMemoTable.at(left) + (dind-left)*myMemoTable.at(left+1));
      } //else
      return evalCurves1(z);
   }

   inline double evalCurves2(const double y, const double c) const {
      if (size <= 1 && (!slides)) {
         return IRfun::evalCurve2(curveChoice,y,c);
      } //else
      double val = 0.0;
      double denom = 0.0;
      double mul;
      IRfun::CURVES curve;
      map<IRfun::CURVES,double>::const_iterator cbit = curveBook.begin();
      map<IRfun::CURVES,double>::const_iterator cbite = curveBook.end();
      while (cbit != cbite) {
         curve = cbit->first;
         mul = cbit->second;
         denom += mul;
         val += mul*IRfun::evalCurve2(curve,y,c);
         ++cbit;
      }
      val = IRutil::divifgt0(val,denom);
      if (!slides) { return val; }
      //else
      double val2 = IRfun::evalCurve2(slideCurveChoice,y,c);
      double w = (y < slideLo ? 0 : (y < slideMid ? (y - slideLo)/(slideMid - slideLo)
                    : (y < slideHi ? (slideHi + y - 2.0*slideMid)/(slideHi - slideMid) : 2.0)))/2.0;
      double wa = pow(w,slidePower);
      double slider = slideProp*wa/(wa + pow(1.0 - w, slidePower));
      return ((1.0 - slider)*val + slider*val2);
   }

   inline double mevalCurves2(const double y, const double c) const {
      if (y < memoCap) {
         double dind = y/memoStep;
         size_t left = size_t(dind);
         return ((left+1.0-dind)*myMemoTable.at(left) + (dind-left)*myMemoTable.at(left+1));
      } //else
      return evalCurves2(y,c);
   }

   inline double evalCurves3(const double x, const double s, const double c) const {
      if (size <= 1 && (!slides)) {
         return IRfun::evalCurve3(curveChoice,x,s,c);
      } //else
      double val = 0.0;
      double denom = 0.0;
      double mul;
      IRfun::CURVES curve;
      map<IRfun::CURVES,double>::const_iterator cbit = curveBook.begin();
      map<IRfun::CURVES,double>::const_iterator cbite = curveBook.end();
      while (cbit != cbite) {
         curve = cbit->first;
         mul = cbit->second;
         denom += mul;
         val += mul*IRfun::evalCurve3(curve,x,s,c);
         ++cbit;
      }
      val = IRutil::divifgt0(val,denom);
      if (!slides) { return val; }
      //else
      double val2 = IRfun::evalCurve3(slideCurveChoice,x,s,c);
      double w = (x < slideLo ? 0 : (x < slideMid ? (x - slideLo)/(slideMid - slideLo)
                    : (x < slideHi ? (slideHi + x - 2.0*slideMid)/(slideHi - slideMid) : 2.0)))/2.0;
      double wa = pow(w,slidePower);
      double slider = slideProp*wa/(wa + pow(1.0 - w, slidePower));
      return ((1.0 - slider)*val + slider*val2);
   }

   inline double mevalCurves3(const double x, const double s, const double c) const {
      if (x < memoCap) {
         double dind = x/memoStep;
         size_t left = size_t(dind);
         return ((left+1.0-dind)*myMemoTable.at(left) + (dind-left)*myMemoTable.at(left+1));
      } //else
      return evalCurves3(x,s,c);
   }


   inline void memoize3() {
      for (size_t i = 0; i < memoSize; i++) {
         myMemoTable.at(i) = evalCurves3(i*memoStep,s,c);
      }
   }

   inline void memoize2() {
      for (size_t i = 0; i < memoSize; i++) {
         myMemoTable.at(i) = evalCurves2(i*memoStep,c);
      }
   }

   inline void memoize1() {
      for (size_t i = 0; i < memoSize; i++) {
         myMemoTable.at(i) = evalCurves1(i*memoStep);
      }
   }

   inline void memoize(const size_t numArgs = 3) {
      if (numArgs == 3) {
         memoize3();
      } else if (numArgs == 1) {
         memoize1();
      } else if (numArgs == 2) {   
         memoize2();
      } else {
         memoize3();
      }
   }

 
 
   inline IRfun::WeightMethod selWeightMethod() const { 
      IRfun::WeightMethod weightMethod; 
      switch(weightMethodChoice) { 
       case IRfun::UNITWTS: 
       	 weightMethod = &IRfun::unitWeights; 
         break; 
       case IRfun::SUMWTS: 
       	 weightMethod = &IRfun::sumWeights; 
         break; 
       case IRfun::LOGSUMWTS: 
       	 weightMethod = &IRfun::logSumWeights; 
         break; 
       case IRfun::ENTROPYWTS: 
         weightMethod = &IRfun::entropyWeights; 
         break; 
       case IRfun::FALLOFFWTS: 
         weightMethod = &IRfun::falloffWeights; 
         break; 
       case IRfun::INVVARP: 
       	 weightMethod = &IRfun::invVarP0; 
         break; 
       default: 
       	 cout << "Using Unit Weights, hope that's OK." << endl; 
         weightMethod = &IRfun::unitWeights; 
      } 
      return weightMethod; 
   } 
 
   string showCurvePoints(double step, double cap) const {
      ostringstream o;
      double arg = 0.00;
      double lastRes = 1.0;
      double res;
      string spacer = "";
      while (arg <= cap) {
         res = mevalCurves3(arg,s,c);
         spacer = (res > lastRes ? spacer+" " : "");
         o << fixed << setprecision(3) << setw(5) << right
           << arg << ": " << spacer << setprecision(7) << setw(9) << res << endl;
         arg += step;
         lastRes = res;
      }
      return o.str();
   }

 
   string getName() const { 
      return name; 
   } 
      // + "s" + ftoa(s,3) + "c" + ftoa(c,3); 
   void setName(const string& gname) { name = gname; } 
 
   string getKind() const {
      string st = "";
      map<IRfun::CURVES, double>::const_iterator cbit = curveBook.begin();
      map<IRfun::CURVES, double>::const_iterator cbite = curveBook.end();
      while (cbit != cbite) {
         st += "(" + IRfun::curveNames[IRfun::CURVES(cbit->first)] + ":" 
                   + IRutil::ftoa(cbit->second) + ")";
      //string st = IRfun::curveNames[curveChoice] + ", "
         ++cbit;
      }
      string slstr = (slides ? "" : "not ");
      if (slides) {
         st += " --> " + IRutil::ftoa(slideProp,2) + "*" + IRfun::curveNames[slideCurveChoice] 
                     + " by " + IRutil::ftoa(slidePower,3) + " in [" + IRutil::ftoa(slideLo,2) 
                     + "," + IRutil::ftoa(slideMid,2) + "," +  IRutil::ftoa(slideHi,2) + "]";
      }
      st += ", " + IRfun::weightNames[weightMethodChoice] + " weights";
      return st;
   }

   string paramsPosition(size_t prec = 4) const {
      string str = "s = " + IRutil::ftoa(s,prec) + ", c = " + IRutil::ftoa(c,prec)
                + ", d = " + IRutil::ftoa(d,prec) + ", v = " + IRutil::ftoa(v,prec)
                + ", a = " + IRutil::ftoa(a,prec) + ", hm = " + IRutil::ftoa(hm,prec)
                + ", hp = " + IRutil::ftoa(hp,prec) + ", b = " + IRutil::ftoa(b,prec);
      return str;
   }

   string getDescription() const { 
      string st = (memoCap > 0 ? "; memo(" + IRutil::ftoa(memoStep) + "," + IRutil::ftoa(memoCap) + ")" : "");
      //string st2 = getKind() + " with\n" + paramsPosition();
                // and judgment depth " + IRutil::itoa(judgmentDepth); 
      return getKind() + st + " with\n" + paramsPosition();
   } 

   string oneLine() const {
      return name + ": " + getDescription();
   }
 
   operator string() const { 
      return name + ": " + getDescription();
   } 
}; 
 
#endif    //end of #ifndef __IR_TRIAL_SPEC_H__ 
 
 
 
 
