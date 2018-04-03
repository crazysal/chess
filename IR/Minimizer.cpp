//File-------------------Minimizer.cpp---------------------------------- 
 
#ifndef __IR_MINIMIZER_CPP__ 
#define __IR_MINIMIZER_CPP__ 

#ifndef __IR_MINIMIZER_H__
#include "Minimizer.h"
#endif
 
/** Helper function for veeFinders.  Assumes the following true at call: 
    INV: midVal <= f(low), midVal <= f(hi). 
    Returns a local minimum (point not value) of f to approximation 
    (hi - low)/2^(depth+1).  Acts as though Ran(f) is V-shaped. 
    Reports some but not all cases where f has more than one minimum. 
 */ 
double Minimizer::vfh(Monic f, double low, double mid, double hi, double midVal) const { 
   while (hi - low > yprec) { 
      if (verbose) { 
         cout << "."; 
         flush(cout); 
      } 
      double q1 = geometricMeans ? 2.0*low*mid/(low + mid) : (low + mid)/2.0; 
      double q3 = geometricMeans ? 2.0*mid*hi/(mid + hi)   : (mid + hi)/2.0; 
      double q1Val = (this->*f)(q1); 
      double q3Val = (this->*f)(q3); 
 
      if (q1Val < midVal) {        //so by INV, q1Val < lowVal too. 
         if (q3Val < midVal) { 
            if (verbose) { 
               for (vector<ostream*>::const_iterator it = outs->begin(); 
                    it != outs->end(); it++) { 
 
                  (**it) << "Found a W: f(" << q1 << ") = " << q1Val 
                         << ", f(" << mid << ") = " << midVal 
                         << ", f(" << q3 << ") = " << q3Val << endl; 
               } 
            } 
            if (q1Val < q3Val) { 
               hi = mid; mid = q1; midVal = q1Val; 
            } else {               //so q3Val <= q1Val <= midVal <= hiVal 
               low = mid; mid = q3; midVal = q3Val; 
            } 
         } else { 
            hi = mid; mid = q1; midVal = q1Val; 
         } 
      } else { 
         if (q3Val < midVal) { 
            low = mid; mid = q3; midVal = q3Val; 
         } else {                  //midVal is <= both q1Val,q3Val 
            low = q1; hi = q3; 
         } 
      } 
   } 
 
   if (verbose) { 
      cout << "Done, with value "; 
      for (vector<ostream*>::const_iterator it = outs->begin(); 
           it != outs->end(); it++) { 
         (**it) << fixed << setprecision(6) 
                << "f(" << mid << ") = " << midVal << endl; 
      } 
   } 
   return mid; 
} 


/** Walk out from the trial's initial (s,c) point rather than in from 
 the Minimizer object's overall boundaries. 
 */ 
double Minimizer::vWalkC(const double s) const { 
Monic f = &Minimizer::f2; 
theTrial->setS(s);      //s is unchanged for this run 
double c1 = theTrial->getC(); 
double c1Val = minimand();    //uses c1, == f(c1). 
double c2 = c1 + walkOutStep; 
double c0 = c1 - walkOutStep; 
double c2Val = (this->*f)(c2); 
double c0Val = (this->*f)(c0); 
if (c1Val < c2Val && c1Val < c0Val) { 
    return vfh(f,c0,c1,c2,c1Val); 
   } 
   if (c1Val > c2Val && c1Val > c0Val) { cout << "Found an immediate W\n"; } 
   int sg = c0Val < c2Val ? -1 : +1; 
   int mul = 2; 
   double cNew = c1 + mul*sg*walkOutStep; 
   double cNewVal; 
   double cMid = c1 + sg*walkOutStep;  //is c0 or c2 
   double cMidVal = (sg == -1 ? c0Val : c2Val); 
   double cBack = c1; 
   double cBackVal = c1Val;  //INV: cMidVal <= cBackVal 
   double cBound = (sg == -1 ? lowSpec.c : hiSpec.c); 
   while (sg*cNew <= sg*cBound && (cNewVal = (this->*f)(cNew)) < cMidVal) { 
      cBack = cMid; 
      cBackVal = cMidVal; 
      cMid = cNew; 
      cMidVal = cNewVal; 
      mul *= 2; 
      cNew = c1 + mul*sg*walkOutStep; 
   } 
   if (cNew < lowSpec.c) { 
      cout << "LowC boundary hit on inner walk for s = " << s 
           << " from c = " << c1 << endl; 
      cNew = lowSpec.c; 
      cNewVal = (this->*f)(lowSpec.c); //no need to check if > cMidVal... 
   } else if (cNew > hiSpec.c) { 
      cout << "HiC boundary hit on inner walk for s = " << s 
           << " from c = " << c1 << endl; 
      cNew = hiSpec.c; 
      cNewVal = (this->*f)(hiSpec.c);  //i.e. we assume boundaries are not minima 
   } else { 
      //do nothing 
   } 
   return vfh(f,cBack,cMid,cNew,cMidVal); 
} 
 
double Minimizer::vWalkCVal(const double s) const { 
   //return value given by best c for s 
   theTrial->setC(vWalkC(s)); 
   return minimand(); 
} 
 
double Minimizer::vWalkS(const double c) const { 
   Monic f = &Minimizer::f1; 
   theTrial->setC(c);      //c is unchanged for this run 
   double s1 = theTrial->getS(); 
   double s1Val = minimand();    //uses s1, == f(s1). 
   double s2 = s1 + walkOutStep; 
   double s0 = s1 - walkOutStep; 
   double s2Val = (this->*f)(s2); 
   double s0Val = (this->*f)(s0); 
   if (s1Val < s2Val && s1Val < s0Val) { 
       return vfh(f,s0,s1,s2,s1Val); 
   } 
   if (s1Val > s2Val && s1Val > s0Val) { 
      cout << "Found an immediate W\n"; 
   } 
   int sg = s0Val < s2Val ? -1 : +1; 
   int mul = 2; 
   double sNew = s1 + mul*sg*walkOutStep; 
   double sNewVal; 
   double sMid = s1 + sg*walkOutStep;  //is s0 or s2 
   double sMidVal = (sg == -1 ? s0Val : s2Val); 
   double sBack = s1; 
   double sBackVal = s1Val;  //INV: sMidVal <= sBackVal 
   double sBound = (sg == -1 ? lowSpec.s : hiSpec.s); 
   while (sg*sNew <= sg*sBound 
             && (sNewVal = (this->*f)(sNew)) < sMidVal) { 
      sBack = sMid; 
      sBackVal = sMidVal; 
      sMid = sNew; 
      sMidVal = sNewVal; 
      mul *= 2; 
      sNew = s1 + mul*sg*walkOutStep; 
   } 
   if (sNew < lowSpec.s) { 
      cout << "LowS boundary hit on inner walk for c = " << c 
           << " from s = " << s1 << endl; 
      sNew = lowSpec.s; 
      sNewVal = (this->*f)(lowSpec.s); //need not check > sMidVal... 
   } else if (sNew > hiSpec.s) { 
      cout << "HiS boundary hit on inner walk for c = " << c 
           << " from s = " << s1 << endl; 
      sNew = hiSpec.s; 
      sNewVal = (this->*f)(hiSpec.s);  //i.e. assume bounds not minima 
   } else { 
      //do nothing 
   } 
   return vfh(f,sBack,sMid,sNew, sMidVal); 
} 
 
double Minimizer::vWalkSVal(const double c) const { 
   //return value given by best s for c 
   theTrial->setS(vWalkS(c)); 
   return minimand(); 
} 
 
double Minimizer::veeWalker1() const { 
   double c = theTrial->getC(); 
   Monic w = &Minimizer::vWalkCVal; 
   double s1 = theTrial->getS(); 
   double s1Val = (this->*w)(s1); 
   double s2 = s1 + walkOutStep; 
   double s0 = s1 - walkOutStep; 
   double s2Val = (this->*w)(s2); 
   double s0Val = (this->*w)(s0); 
   if (s1Val < s2Val && s1Val < s0Val) { 
       return vfh(w,s0,s1,s2,s1Val); 
   } 
   if (s1Val > s2Val && s1Val > s0Val) { 
      cout << "Found an immediate W\n"; 
   } 
   int sg = s0Val < s2Val ? -1 : +1; 
   int mul = 2; 
   double sNew = s1 + mul*sg*walkOutStep; 
   double sNewVal; 
   double sMid = s1 + sg*walkOutStep;  //is s0 or s2 
   double sMidVal = (sg == -1 ? s0Val : s2Val); 
   double sBack = s1; 
   double sBackVal = s1Val;  //INV: sMidVal <= sBackVal 
   double sBound = (sg == -1 ? lowSpec.s : hiSpec.s); 
   while (sg*sNew <= sg*sBound 
             && (sNewVal = (this->*w)(sNew)) < sMidVal) { 
      sBack = sMid; 
      sBackVal = sMidVal; 
      sMid = sNew; 
      sMidVal = sNewVal; 
      mul *= 2; 
      sNew = s1 + mul*sg*walkOutStep; 
   } 
   if (sNew < lowSpec.s) { 
      cout << "LowS boundary hit on outer walk for c = " << c 
           << " from s = " << s1 << endl; 
      sNew = lowSpec.s; 
      sNewVal = (this->*w)(lowSpec.s); //need not check > sMidVal... 
   } else if (sNew > hiSpec.s) { 
      cout << "HiS boundary hit on outer walk for c = " << c 
           << " from s = " << s1 << endl; 
      sNew = hiSpec.s; 
      sNewVal = (this->*w)(hiSpec.s);  //i.e. assume bounds not minima 
   } else { 
      //do nothing 
   } 
   return vfh(w,sBack,sMid,sNew, sMidVal); 
} 
 
 
double Minimizer::veeWalker2() const { 
   double s = theTrial->getS(); 
   Monic w = &Minimizer::vWalkSVal; 
   double c1 = theTrial->getC(); 
   double c1Val = (this->*w)(c1); 
   double c2 = c1 + walkOutStep; 
   double c0 = c1 - walkOutStep; 
   double c2Val = (this->*w)(c2); 
   double c0Val = (this->*w)(c0); 
   if (c1Val < c2Val && c1Val < c0Val) { 
       return vfh(w,c0,c1,c2,c1Val); 
   } 
   if (c1Val > c2Val && c1Val > c0Val) { cout << "Found an immediate W\n"; } 
   int sg = c0Val < c2Val ? -1 : +1; 
   int mul = 2; 
   double cNew = c1 + mul*sg*walkOutStep; 
   double cNewVal; 
   double cMid = c1 + sg*walkOutStep;  //is c0 or c2 
   double cMidVal = (sg == -1 ? c0Val : c2Val); 
   double cBack = c1; 
   double cBackVal = c1Val;  //INV: cMidVal <= cBackVal 
   double cBound = (sg == -1 ? lowSpec.c : hiSpec.c); 
   while (sg*cNew <= sg*cBound && (cNewVal = (this->*w)(cNew)) < cMidVal) { 
      cBack = cMid; 
      cBackVal = cMidVal; 
      cMid = cNew; 
      cMidVal = cNewVal; 
      mul *= 2; 
      cNew = c1 + mul*sg*walkOutStep; 
   } 
   if (cNew < lowSpec.c) { 
      cout << "LowC boundary hit on outer walk for s = " << s 
           << " from c = " << c1 << endl; 
      cNew = lowSpec.c; 
      cNewVal = (this->*w)(lowSpec.c); //no need to check if > cMidVal... 
   } else if (cNew > hiSpec.c) { 
      cout << "HiC boundary hit on outer walk for s = " << s 
           << " from c = " << c1 << endl; 
      cNew = hiSpec.c; 
      cNewVal = (this->*w)(hiSpec.c);  //i.e. we assume boundaries are not minima 
   } else { 
      //do nothing 
   } 
   return vfh(w,cBack,cMid,cNew,cMidVal); 
} 
 
 
#endif  //end of #ifndef __IR_MINIMIZER_CPP__ 
 
 
 
