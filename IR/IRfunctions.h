//File----------------------IRfunctions.h---------------------------------- 
 
#ifndef __IR_FUNCTIONS_H__ 
#define __IR_FUNCTIONS_H__ 
 
namespace IRfun {          
 
//---------------CURVES AND OTHER FUNCTIONS TO PASS----------------------- 
 
 
   //typedef  
   typedef double (*Curve1) (const double);
   typedef double (*Curve2) (const double, const double);
   typedef double (*Curve3) (const double, const double, const double); 
                           //arg order delta,s,c ,d
   typedef double (*WeightMethod)(const vector<double>&, 
                                  const vector<double>* const); 
   typedef double (*Dist)(const double, const double); 
    
   enum CURVES { 
      INVEXP,  
      INVPOLYA,  
      INVPOLYB,  
      FOLDEDERF,
      FOLDEDLOGISTIC,
      SECHA,  
      SECHB,  
      SECHC, 
      SECHD, 
      INVLOGA,  
      INVLOGB,  
      INVLOGC, 
      INVEXPLIN,
      POWPOW,
      INV2EXPA,  
      INV2EXPB,  
      STUDENTT, 
      IRT, 
      // INV2EXPC,  
      NUM_CURVES 
   }; 
    
   //Curves 
   inline double invExp(const double z) { return exp(-z); }
   inline double invExp(const double y, const double c) {
      return exp(-pow(y,c));
   }
   inline double invExp(const double x, const double s, const double c) { 
      return exp(-pow(x/s, c)); 
   } 
 
   inline double invExpLin(const double z) { return exp(-z)/(1.0 + z); }
   inline double invExpLin(const double y, const double c) {
      const double w = pow(y,c);
      return exp(-w)/(1.0 + w);
   }
   inline double invExpLin(const double x, const double s, const double c) { 
      const double w = pow(x/s, c); 
      //return exp(-w)/(1.0 + x/(10.0*s)); 
      return exp(-w)/(1.0 + w);
   } 
 
   inline double powpow(const double z) { return 1.0/pow(z,z); }
   inline double powpow(const double y, const double c) {
      const double w = pow(y,c);
      return 1.0/pow(w,w);
   }
   inline double powpow(const double x, const double s, const double c) { 
      const double w = pow(x/s, c); 
      return 1.0/pow(x/s, c*w); 
   } 
    
   inline double invPolyA(const double z) { return 1.0/(1.0 + z); }
   inline double invPolyA(const double y, const double c) {
      return 1.0/(1.0 + pow(y,c));
   }
   inline double invPolyA(const double x, const double s, const double c) { 
      return 1.0/(1.0 + pow(x/s,c)); 
   } 
 
   inline double invPolyB(const double z) { return 1.0/(1.0 + z); }  //yes == invPolyA
   inline double invPolyB(const double y, const double c) {
      return 1.0/pow(1.0 + y, c);
   }
   inline double invPolyB(const double x, const double s, const double c) { 
      return 1.0/pow(1.0 + (x/s),c); 
   } 

   inline double ferf(const double z) { return 1.0 + erf(-z); }
   inline double ferf(const double y, const double c) {
      return 1.0 + erf(-pow(y,c));
   }
   inline double ferf(const double x, const double s, const double c) {
      return 1.0 + erf(-pow(x/s,c));
   }

   inline double flogistic(const double z) {
      return 2.0/(1.0 + exp(z));
   }
   inline double flogistic(const double y, const double c) {
      return 2.0/(1.0 + exp(pow(y,c)));
   }
   inline double flogistic(const double x, const double s, const double c) {
      return 2.0/(1.0 + exp(pow(x/s,c)));
   }

    
   inline double sechA(const double z) {
      const double u = exp(z);
      return 2.0/(u + 1.0/u);
   }
   inline double sechA(const double y, const double c) {
      const double u = exp(pow(y,c));
      return 2.0/(u + 1.0/u);
   }
   inline double sechA(const double x, const double s, const double c) { 
      const double u = exp(pow(x/s,c)); 
      return 2.0/(u + 1.0/u); 
   } 

   inline double sechB(const double z) { return sechA(z); }
   inline double sechB(const double y, const double c) {
      const double v = exp(y);
      return pow(2.0/(v + 1.0/v), c);
   }
   inline double sechB(const double x, const double s, const double c) { 
      const double v = exp(x/s); 
      return pow(2.0/(v + 1.0/v), c); 
   } 
   
   inline double sechC(const double z) { return z*sechA(z); }
   inline double sechC(const double y, const double c) {
      const double z = exp(-y);
      return z * pow(2.0/(1.0 + z), c);
   }
   inline double sechC(const double x, const double s, const double c) { 
      const double z = exp(-x/s); 
      return z * pow(2.0/(1.0 + z), c); 
   } 

   //Equivalent to Hubbert curve
   inline double sechD(const double z) { return 4.0 / (z + 2.0 + 1.0/z); }
   inline double sechD(const double y, const double c) {
      const double z = exp(pow(y,c));
      return 4.0 / (z + 2.0 + 1.0/z);
   }
   inline double sechD(const double x, const double s, const double c) { 
      const double z = exp(pow(x/s,c)); 
      return 4.0 / (z + 2.0 + 1.0/z); 
   } 

    
   inline double invLogA(const double z) { return 1.0/(1.0 + log(1.0 + z)); }
   inline double invLogA(const double y, const double c) {
      return 1.0/(1.0 + log(1.0 + pow(y,c)));
   }
   inline double invLogA(const double x, const double s, const double c) { 
      return 1.0/(1.0 + log(1.0 + pow(x/s,c))); 
   } 

   inline double invLogB(const double z) { return invLogA(z); }
   inline double invLogB(const double y, const double c) {
      return 1.0/(1.0 + pow(log(1.0 + y),c));
   }
   inline double invLogB(const double x, const double s, const double c) { 
      return 1.0/(1.0 + pow(log(1.0 + x/s),c)); 
   } 

   inline double invLogC(const double z) { return invLogA(z); }
   inline double invLogC(const double y, const double c) {
      return 1.0/pow(1.0 + log(1.0 + y), c);
   }
   inline double invLogC(const double x, const double s, const double c) { 
      return 1.0/pow(1.0 + log(1.0 + x/s), c); 
   } 
    
   inline double inv2ExpA(const double z) { return 1.0/exp(exp(z) - 1.0); }
   inline double inv2ExpA(const double y, const double c) {
      return 1.0/exp(exp(pow(y, c)) - 1.0);
   }
   inline double inv2ExpA(const double x, const double s, const double c) { 
      return 1.0/exp(exp(pow(x/s, c)) - 1.0); 
   } 

   inline double inv2ExpB(const double z) { return inv2ExpA(z); }
   inline double inv2ExpB(const double y, const double c) {
      return 1.0/exp(pow(exp(y) - 1.0, c));
   }
   inline double inv2ExpB(const double x, const double s, const double c) { 
      return 1.0/exp(pow(exp(x/s) - 1.0, c)); 
   } 

   inline double studentT(const double z) { return (1.0 + z)*exp(-z); }
   inline double studentT(const double y, const double c) {
      return c/pow(c + y*y, (c+1)/2);
   }
   inline double studentT(const double x, const double s, const double c) { 
      return c/pow(c + (x/s)*(x/s), (c+1)/2); 
   } 

   inline double slideIEIPA(const double z) { return (15.0/16.0)*invExp(z) + (1.0/16.0)*invPolyA(z); }
   inline double slideIEIPA(const double y, const double c) { 
      return (15.0/16.0)*invExp(y,c) + (1.0/16.0)*invPolyA(y,c);
    }
   inline double slideIEIPA(const double x, const double s, const double c) { 
      return (15.0/16.0)*invExp(x,s,c) + (1.0/16.0)*invPolyA(x,s,c); 
   } 

   inline double irt(const double z) { return 1.0 - 1.0/(1.0 + exp(-z)); }
   inline double irt(const double y, const double c) {
      return 1.0 - 1.0/(1.0 + exp(-pow(y,c)));
   }
   inline double irt(const double x, const double s, const double c) { 
      return (exp(c*s)/exp(c*(x + s))); 
   } 

   // double inv2ExpC(const double x, const double s, const double c) {
   //    return 1.0/pow(exp(exp(x/s) - 1), c);
   // }

   //Curves commented out cause a "race" between s and c when fitting,
   //with both increasing seemingly unboundedly without fitting well.

       
   inline double evalCurve1(CURVES curve, double z) {
      switch(curve) {
       case INVEXP:
         return invExp(z);
       case INVPOLYA:
         return invPolyA(z);
       case INVPOLYB:
         return invPolyB(z);
       case FOLDEDERF:
         return ferf(z);
       case FOLDEDLOGISTIC:
         return flogistic(z);
       case SECHA:
         return sechA(z);
       case SECHB:
         return sechB(z);
       case SECHC:
         return sechC(z);
       case SECHD:
         return sechD(z);
       case INVLOGA:
         return invLogA(z);
       case INVLOGB:
         return invLogB(z);
       case INVLOGC:
         return invLogC(z);
       case INVEXPLIN:
         return invExpLin(z);
       case POWPOW:
         return powpow(z);
       case INV2EXPA:
         return inv2ExpA(z);
       case INV2EXPB:
         return inv2ExpB(z);
       case STUDENTT:
         return studentT(z);
       case IRT:
         return irt(z);
       default: 
         cerr << "Unknown curve; using InvExp." << endl;
         return invExp(z);
      }
      cerr << "Unknown curve; using InvExp." << endl;
      return invExp(z);
   }
 
   inline double evalCurve2(CURVES curve, double y, double c) {
      switch(curve) {
       case INVEXP:
         return invExp(y,c);
       case INVPOLYA:
         return invPolyA(y,c);
       case INVPOLYB:
         return invPolyB(y,c);
       case FOLDEDERF:
         return ferf(y,c);
       case FOLDEDLOGISTIC:
         return flogistic(y,c);
       case SECHA:
         return sechA(y,c);
       case SECHB:
         return sechB(y,c);
       case SECHC:
         return sechC(y,c);
       case SECHD:
         return sechD(y,c);
       case INVLOGA:
         return invLogA(y,c);
       case INVLOGB:
         return invLogB(y,c);
       case INVLOGC:
         return invLogC(y,c);
       case INVEXPLIN:
         return invExpLin(y,c);
       case POWPOW:
         return powpow(y,c);
       case INV2EXPA:
         return inv2ExpA(y,c);
       case INV2EXPB:
         return inv2ExpB(y,c);
       case STUDENTT:
         return studentT(y,c);
       case IRT:
         return irt(y,c);
       default:
         cerr << "Unknown curve; using InvExp." << endl;
         return invExp(y,c);
      }
      cerr << "Unknown curve; using InvExp." << endl;
      return invExp(y,c);
   }

   inline double evalCurve3(CURVES curve, double x, double s, double c) {
      switch(curve) {
       case INVEXP:
         return invExp(x,s,c);
       case INVPOLYA:
         return invPolyA(x,s,c);
       case INVPOLYB:
         return invPolyB(x,s,c);
       case FOLDEDERF:
         return ferf(x,s,c);
       case FOLDEDLOGISTIC:
         return flogistic(x,s,c);
       case SECHA:
         return sechA(x,s,c);
       case SECHB:
         return sechB(x,s,c);
       case SECHC:
         return sechC(x,s,c);
       case SECHD:
         return sechD(x,s,c);
       case INVLOGA:
         return invLogA(x,s,c);
       case INVLOGB:
         return invLogB(x,s,c);
       case INVLOGC:
         return invLogC(x,s,c);
       case INVEXPLIN:
         return invExpLin(x,s,c);
       case POWPOW:
         return powpow(x,s,c);
       case INV2EXPA:
         return inv2ExpA(x,s,c);
       case INV2EXPB:
         return inv2ExpB(x,s,c);
       case STUDENTT:
         return studentT(x,s,c);
       case IRT:
         return irt(x,s,c);
       default:
         cerr << "Unknown curve; using InvExp." << endl;
         return invExp(x,s,c);
      }
      cerr << "Unknown curve; using InvExp." << endl;
      return invExp(x,s,c);
   }

    
   //Weights 
    
   enum WEIGHTS { 
      UNITWTS, SUMWTS, LOGSUMWTS, ENTROPYWTS, FALLOFFWTS, INVVARP, NUM_WEIGHTS  
   }; 
 
   //Functionsnot inlined since passed as function pointers. 
    
   inline double unitWeights(const vector<double>& probs, 
                             const vector<double>* const unused = NULL) { 
      return 1.0; 
   } 
    
   /** Named because in the "shares" model, 1/p0 is the sum of the shares. 
       Downweights "obvious" move choices. 
    */ 
   inline double sumWeights(const vector<double>& probs, 
                            const vector<double>* const unused = NULL) { 
      return 1.0/probs.at(0); 
   } 
    
   /** Strongly downweights "obvious" move choices. 
    */ 
   inline double logSumWeights(const vector<double>& probs, 
                               const vector<double>* const unused = NULL) { 
      return log(1.0/probs.at(0)); 
   } 
 
   /** Downweights "obvious" move choices. 
    */ 
   inline double entropyWeights(const vector<double>& probs, 
                                const vector<double>* const unused = NULL) { 
      double ent = 0.0; 
      for (vector<double>::const_iterator itr = probs.begin(); 
           itr != probs.end(); ++itr) { 
         ent += (*itr)*log(1.0/(*itr)); 
      } 
      return ent; 
   } 
 
   /** Expected error from a move, given projected probabilities 
    */ 
   inline double falloffWeights(const vector<double>& probs, 
                                const vector<double>* const pScaledDeltas) { 
      double ff = 0.0; 
      vector<double>::const_iterator itr = probs.begin(); 
      vector<double>::const_iterator itr2 = pScaledDeltas->begin(); 
      while (itr != probs.end() && itr2 != pScaledDeltas->end()) { 
         ff += (*itr)*(*itr2); 
         ++itr;  ++itr2; 
      } 
      return ff; 
   } 
 
    
   /** "Inverse-variance" weight on p0 may combat bias in estimating p0 (and 
       hence in predicting first-line matches), but it strongly *upweights* 
       the more-obvious move choices.  Since this is an assignable function 
       pointer, cannot push guard against P0TOLERANCE onto caller (!?). 
       Also needs a cap in cases of clear-choice moves, else an automatic 
       recapture would be weighted thousands more than a normal move choice. 
    */ 
   inline double invVarP0(const vector<double>& probs, 
                          const vector<double>* const unused = NULL) { 
      double p0 = min(probs.at(0), IRutil::P0TOLERANCE); 
      return min(IRutil::INVVARP0CAP, 1.0/(p0*(1.0 - p0))); 
   } 
    
    
   static map<CURVES,string> curveNames;     //could make static
   static map<WEIGHTS,string> weightNames; 
   static map<CURVES,string> curveDescriptions; 
   static map<WEIGHTS,string> weightDescriptions; 
    
   inline void initNames() { 
      curveNames[INVEXP] = "InvExp"; 
      curveNames[INVPOLYA] = "InvPolyA"; 
      curveNames[INVPOLYB] = "InvPolyB"; 
      curveNames[FOLDEDERF] = "FoldedERF";
      curveNames[FOLDEDLOGISTIC] = "FoldedLogistic";
      curveNames[SECHA] = "SechA"; 
      curveNames[SECHB] = "SechB"; 
      curveNames[SECHC] = "SechC"; 
      curveNames[SECHD] = "SechD/Hubbert"; 
      curveNames[INVLOGA] = "InvLogA"; 
      curveNames[INVLOGB] = "InvLogB"; 
      curveNames[INVLOGC] = "InvLogC"; 
      curveNames[INVEXPLIN] = "InvExpLin";
      curveNames[POWPOW] = "PowPow";
      curveNames[INV2EXPA] = "Inv2ExpA"; 
      curveNames[INV2EXPB] = "Inv2ExpB"; 
      curveNames[STUDENTT] = "StudentT"; 
      curveNames[IRT] = "IRT"; 
      // curveNames[INV2EXPC] = "Inv2ExpC";   //causes s-c "race". 
      weightNames[UNITWTS] = "Unit"; 
      weightNames[SUMWTS] = "Sum"; 
      weightNames[LOGSUMWTS] = "Logsum"; 
      weightNames[ENTROPYWTS] = "Entropy"; 
      weightNames[FALLOFFWTS] = "Falloff"; 
      weightNames[INVVARP] = "InvVarP"; 
   curveDescriptions[INVEXP] =   "1/e^{x^c}"; 
   curveDescriptions[INVPOLYA] = "1/(1 + x^c)           (approximates InvExp)"; 
 curveDescriptions[INVPOLYB] = "1/(1 + x)^c           (\"\",~ Fisk for c = 2)"; 
 //outdented since escaped quotes 
   curveDescriptions[FOLDEDERF]="Folded Gaussian error fn";
   curveDescriptions[FOLDEDLOGISTIC]="Folded logistic curve x2";
   curveDescriptions[SECHA] =    "Sech(x^c)               (~ logistic in x^c)"; 
   curveDescriptions[SECHB] =    "Sech(x)^c               ('c-logistic' in x)"; 
   curveDescriptions[SECHC] =    "2^c e^-x/(1+e^-x)^c    (logistic for c = 2)"; 
   curveDescriptions[SECHD] =    "4e^{-x^c}/(1+e^{-x^c})^2  (logistic in x^c)"; 
   curveDescriptions[INVLOGA] =  "1/(1 + ln(1 + x^c))      (approx. InvPolyA)"; 
   curveDescriptions[INVLOGB] =  "1/(1 + ln^c(1 + x))      (approx. InvPolyA)"; 
   curveDescriptions[INVLOGC] =  "1/(1 + ln(1 + x))^c      (approx. InvPolyB)"; 
   curveDescriptions[INVEXPLIN] = "1/e^{x^c}(1+x^c)";
   curveDescriptions[POWPOW] = "(x^c)^(x^c)";
   curveDescriptions[INV2EXPA] = "1/e^{e^{x^c} - 1}"; 
   curveDescriptions[INV2EXPB] = "1/e^{e^x - 1}^c"; 
   curveDescriptions[STUDENTT] = "Student's T with c degrees of freedom"; 
   curveDescriptions[IRT]="e^{cs}/e^{c(delta + s)}"; 
 
   // curveDescriptions[INV2EXPC] = "1/(e^{e^x - 1})^c"; 
   weightDescriptions[UNITWTS] = "all moves treated equally"; 
   weightDescriptions[SUMWTS] = "1/p0,      upweights moves with many choices"; 
   weightDescriptions[LOGSUMWTS] = "log(1/p0), downweights clear-choice moves"; 
   weightDescriptions[ENTROPYWTS] = "sum(p*log(1/p)), downweights clear moves"; 
   weightDescriptions[FALLOFFWTS] = "sum(p(i)*delta(i)), expected falloff"; 
   weightDescriptions[INVVARP] = "1/p0(1-p0), upweights clear-choice moves"; 
   } 
    
   // Dist functions for fitting
   inline double leastSquares(const double a, const double b) { 
      return (a-b)*(a-b); 
   } 
 
   inline double L1(const double a, const double b) { 
      return fabs(a - b); 
   } 
 
   inline double L1a(const double a, const double b) { 
      return (a - b); 
   } 
 
 
   inline double l3(const double a, const double b) { 
      const double c = fabs(a-b); 
      return c*c*c; 
   } 
 
    
   /** Distortion Fix for 167 '05--'07 games, file MasterTurns.txt. 
       Obtained by linear regression (on log scale) from these games. 
       Returns the estimated probability of a 2750-ish player choosing a move 
       that is not among the first 10 lines as judged by Toga II 1.2.1a to 
       depth 15.  As a probability value not a curve value, it applies 
       equally to both the Shares and PowerShares models.  It is used both 
       for turns in which a non-top-10 move was played, and those not. 
       Used only for predictions, not "actual" stats. 
    */ 
   //inline double distortionFix(double deltaTen) { 
   //   double tmp = 5.822*exp(2.89*deltaTen); 
   //   return 1/tmp; 
   //   //return 0;   //was used to de-bug 
   //} 
    
   /** Regressed falloff from cases of played move not in top 10, from 
       MasterTurns.txt.  Used only for prediction and to estimate the 
       mean delta of "Move 11" over all tuples, not just the cases where 
       a non-top-10 move was played.  *And* used for "actual" falloff in 
       only a rare case: when a non-top-10 move is the last move in a game. 
    */ 
   //inline double falloffFix(double deltaTen) { 
   //   return 1.1111*deltaTen + 0.0213; 
   //   //return 1.1115*deltaTen + 0.0382;  //if we kill some goofy outliers 
   //} 
 
 
 
//---------------------------The Big Enchilada!----------------------- 
 

//!!! To be changed.

   inline double ipr(const double ae, const WEIGHTS wmc) { 
      switch(wmc) { 
		  //The current numbers are obtained by linear regression on 2006 to 2009 training data using unit weight and 58-42 split. 
       case UNITWTS: //return (3571.0 - ae*15413.0); 
         return 3474.87 - 13985.7436*ae;
       case SUMWTS:
         return 3603.714 - 16544*ae;
       case LOGSUMWTS:
         return 3646.013  - 17042.486*ae;
       case ENTROPYWTS: //return (3718.7039 - 17120.088*ae);
         return 3605.461 - 15591.517*ae;
       case FALLOFFWTS:
         return 3414 - 11514.1896*ae;
       case INVVARP:
         return 3343.486 - 18739.179*ae;
       default:
         return 3505.917 - 14576.932*ae;
      }
   }

   inline double iprew(const double ae) {
      return 3805.478 - 18210.259*ae;
   }


   inline double iprewall(const double ae) {
      return 3800.8633 - 18137.617*ae;
   }

   inline double ipr2(const double ae) {
      return (3528.0 - ae*14470.7);
   }

   inline double ipr3(const double ae) {
      return (3514.44 - ae*14298.0);
   }

   inline double sOfElo(double elo) {  // Per Aug. 2014 calibration
      return 0.143 - 0.00009*(elo - 2000);
   }

   inline double cOfElo(double elo) {
      return 0.466 + 0.000075*(elo - 2000);
   }

   inline double dOfElo(double elo) {
      return 20;
   }

   inline double vOfElo(double elo) {
      return 0.0;
   }

   inline double aOfElo(double elo) {
      return 0.0;
   }

   inline double hmOfElo(double elo) {
      return 0.0;
   }

   inline double hpOfElo(double elo) {
      return 0.0;
   }

   inline double bOfElo(double elo) {
      return 0.0;
   }


/*
         return 3505.917 - 14576.932*ae; 
       case SUMWTS: 
         return 3603.714 - 16544*ae; 
       case LOGSUMWTS: 
         return 3646.013  - 17042.486*ae; 
       case ENTROPYWTS: //return (3718.7039 - 17120.088*ae); 
         return 3605.461 - 15591.517*ae; 
       case FALLOFFWTS: 
         return 3414 - 11514.1896*ae; 
       case INVVARP: 
         return 3343.486 - 18739.179*ae; 
       default:  
         return 3505.917 - 14576.932*ae; 
      } 
   } 
*/
 
    
};                          //end of IRfun namespace again 
 
#endif                      //end of #ifndef __IR_FUNCTIONS_H__ 
 
 
