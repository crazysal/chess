//File--------------------------TupleInfo.h--------------------------------- 
 
#ifndef __IR_TUPLE_INFO_H__ 
#define __IR_TUPLE_INFO_H__ 

static int tuplesMade = 0;
static int tuplesDeleted = 0;

enum APPLICATION_POLICIES {
   NO_SWING,
   JOIN_ABS,   //absolute swing, ditto LINK and SEPARATE and PROXY
   JOIN_REL1,
   JOIN_REL2,
   JOIN_REL3,
   LINK_ABS,
   LINK_REL1,
   LINK_REL2,
   LINK_REL3,
   SEPARATE_ABS,
   SEPARATE_REL1,
   SEPARATE_REL2,
   SEPARATE_REL3,
   PROXY_ABS,
   PROXY_REL1,
   PROXY_REL2,
   PROXY_REL3,
   NUM_APPLICATION_POLICIES
};

static map<APPLICATION_POLICIES,string> policyNames;
static void initPolicyNames() {
   policyNames[NO_SWING] = "noSwing";
   policyNames[JOIN_ABS] = "joinAbs";
   policyNames[JOIN_REL1] = "joinRel1";
   policyNames[JOIN_REL2] = "joinRel2";
   policyNames[JOIN_REL3] = "joinRel3";
   policyNames[LINK_ABS] = "linkAbs";
   policyNames[LINK_REL1] = "linkRel1";
   policyNames[LINK_REL2] = "linkRel2";
   policyNames[LINK_REL3] = "linkRel3";
   policyNames[SEPARATE_ABS] = "sepAbs";
   policyNames[SEPARATE_REL1] = "sepRel1";
   policyNames[SEPARATE_REL2] = "sepRel2";
   policyNames[SEPARATE_REL3] = "sepRel3";
   policyNames[PROXY_ABS] = "proxyAbs";
   policyNames[PROXY_REL1] = "proxyRel1";
   policyNames[PROXY_REL2] = "proxyRel2";
   policyNames[PROXY_REL3] = "proxyRel3";
}


enum SWING_OPTIONS {
   GO_SWING,
   NO_SWING_TERM,
   JOIN_TERMS,
   LINK_TERMS,
   SEPARATE_TERMS,
   APPLY_TO_PROXY,
   ABS_SWING,
   REL1_SWING,
   REL2_SWING,
   REL3_SWING,
   COUPLE_PARAMS,
   DECOUPLE_PARAMS,
   DEPTH_WTD,
   UNSCALED_SWING,
   SCALED_SWING,
   SIMPLE,
   PROPORTIONAL,
   DAMPED,
   BOXED,
   BOXED_OPTION,
   SCALE_HERE,
   TRANS_POWER,       //logistic translation of left-hand side
   NUM_SWING_OPTIONS
};

static APPLICATION_POLICIES combinePolicies(SWING_OPTIONS pol1, SWING_OPTIONS pol2) {
   switch(pol1) {
    case NO_SWING_TERM:
      return NO_SWING;
    case JOIN_TERMS:
      switch(pol2) {
       case ABS_SWING:
         return JOIN_ABS;
       case REL1_SWING:
         return JOIN_REL1;
       case REL2_SWING:
         return JOIN_REL2;
       case REL3_SWING:
         return JOIN_REL3;
       default:
         return JOIN_ABS;
      }
    case LINK_TERMS:
      switch(pol2) {
       case ABS_SWING:
         return LINK_ABS;
       case REL1_SWING:
         return LINK_REL1;
       case REL2_SWING:
         return LINK_REL2;
       case REL3_SWING:
         return LINK_REL3;
       default:
         return LINK_ABS;
      }
    case SEPARATE_TERMS:
      switch(pol2) {
       case ABS_SWING:
         return SEPARATE_ABS;
       case REL1_SWING:
         return SEPARATE_REL1;
       case REL2_SWING:
         return SEPARATE_REL2;
       case REL3_SWING:
         return SEPARATE_REL3;
       default:
         return SEPARATE_ABS;
      }
    case APPLY_TO_PROXY:
      switch(pol2) {
       case ABS_SWING:
         return PROXY_ABS;
       case REL1_SWING:
         return PROXY_REL1;
       case REL2_SWING:
         return PROXY_REL2;
       case REL3_SWING:
         return PROXY_REL3;
       default:
         return PROXY_ABS;
      }
    default:
      return NO_SWING;
   }
   cerr << "Unrecognized outer option, returning noSwing..." << endl;
   return NO_SWING;
}


struct SwingInfo {
   map<APPLICATION_POLICIES,double> swingBook;
   bool noSwing;
   bool depthDependentSwings;
   bool unscaledSwings;
   bool damped;
   bool prop;
   bool boxed;
   size_t boxKind;
   bool decouple;
   bool scaleHere;
   double transPower; //a in power p1^{(1 + a(1 - u_i))/u_i)}

   SwingInfo(map<APPLICATION_POLICIES,double> sb, bool dds, bool uss,
             bool da, bool pr, bool bo, size_t bk, bool dec, bool sch, double tp)
    : swingBook(sb), noSwing(sb.empty()), depthDependentSwings(dds), unscaledSwings(uss),
      damped(da), prop(pr), boxed(bo), boxKind(bk), decouple(dec), scaleHere(sch), transPower(tp)
   { }

   bool separates() const {
      map<APPLICATION_POLICIES,double>::const_iterator swie = swingBook.end();
      return (swingBook.find(SEPARATE_ABS) != swie || swingBook.find(SEPARATE_REL1) != swie
              || swingBook.find(SEPARATE_REL2) != swie || swingBook.find(SEPARATE_REL3) != swie);
   }

   operator string() const { 
      string str = "";
      map<APPLICATION_POLICIES,double>::const_iterator swbb = swingBook.begin();
      map<APPLICATION_POLICIES,double>::const_iterator swbe = swingBook.end();
      while (swbb != swbe) {
         str += "("+policyNames[swbb->first]+":"+IRutil::ftoa(swbb->second)+")";
         ++swbb;
      }
      str += (damped ? ",damped" : (prop ? ",prop" : (boxed ? ",boxed"+IRutil::itoa(boxKind) : ",simple")));
      if (unscaledSwings) { str += ",unscaled"; }
      if (decouple) { str += ";decouple"; }
      if (scaleHere) { str += ",scalesHere"; }
      if (transPower != 0.0) { str += ",tpwr " + IRutil::ftoa(transPower); }
      return str;
   }
      
};
   
   

struct CurveApplication {
   vector<double> values;
   size_t swapIndex;
   explicit CurveApplication(size_t numIndices = 0)
    : values(vector<double>(numIndices,0.0)), swapIndex(0)
   { }
};
   
 
/** A TupleInfo object is constructed ONLY by DecisionInfo::makeTuple.
    Tupleinfo stores the most pertinent information for projecting 
    probabilities.
    This is triggered by the class Trial. In this operation, the "deltas" 
    may be scaled from their "raw" values according to a DeltaScale object, 
    and scaledFalloff may be scaled similarly---but only at construction, 
    as signified by "const" on these fields.  This class needs speed and 
    should not be extended ("final" in Java, "sealed" in C#)---it has a  
    non-virtual destructor even though it *is* deleted via pointers. 
    More simply put, these structs are READ-ONLY---except that  
    TurnInfo::turnFalloff would be coded by /single-assignment/. 

    TODO: split off non-ephemerals into SwingInfo
 */ 
class TupleInfo { 
 
 public: 

   const TurnInfo* pTurn;                      //used only for diagnostics
   const vector<double>* const pScaledDeltas;  //shared? actually not...
   const vector<double>* moveSwings;           //passed to MoveProjection
   SwingInfo* swingPolicy;

   bool swingsWereSetNew;                  //notification not policy, true if reset

   const double evalPTM;                       //overall or perceived, player's perspective
   const int playedMoveIndex; 
   const int numDeltas; 
   const int altPlayedMoveIndex;               //used in event of shuffling equal-top moves
   const bool useAltPlayedMoveIndex; 
   const bool patchesProbs; 
   const double patchPower;

   const int doAltPlayedMoveIndex() {          //random shuffle of equal-top order
      int apmi; 
      int pmiLeft = playedMoveIndex - 1;  //should be on first occurrence 
      int pmiRight = playedMoveIndex + 1; //past-end 
      double pmd = pScaledDeltas->at(playedMoveIndex); 
      while (pmiLeft >= 0 && pScaledDeltas->at(pmiLeft) == pmd) { pmiLeft--; } 
      while (pmiRight < pScaledDeltas->size() 
                   && pScaledDeltas->at(pmiRight) == pmd) {  
         pmiRight++;  
      } 
      pmiLeft++;   //to put back on first tied value 
      apmi = pmiLeft + (IRutil::myRand(0,pmiLeft - pmiRight)); 
      return apmi; 
   } 
 
   TupleInfo(const TurnInfo* pt, const vector<double>* givenDeltas,
             const vector<double>* givenSwings, 
             SwingInfo* swp,
             double givenEval, int givenPlayedMoveIndex, bool ua, bool pp,
             const double ppwr) 
    : pTurn(pt)
    , pScaledDeltas(givenDeltas)
    , moveSwings(givenSwings)     //can be NULL  --?
    , swingPolicy(swp)
    , swingsWereSetNew(false)
    , evalPTM(givenEval)
    , playedMoveIndex(givenPlayedMoveIndex)
    , numDeltas(givenDeltas->size())
    , altPlayedMoveIndex(0)  //(doAltPlayedMoveIndex())
    , useAltPlayedMoveIndex(ua)
    , patchesProbs(pp)
    , patchPower(ppwr)
   { 
      tuplesMade++;
      if (tuplesMade % IRutil::TUPLEMODULUS == 0) {
         cerr << tuplesMade << " tuples made, " << tuplesDeleted
              << " deleted." << endl;
      } 
   }
 
   double getDelta(int index) {      //note: call on an unscaled TupleInfo for unscaled version
      return pScaledDeltas->at(index); 
   } 
   
   void setSwings(const vector<double>* newSwings) { 
      //swingPolicy->depthDependentSwings = true;
      swingsWereSetNew = true;
      moveSwings = newSwings;
   }
 
   ~TupleInfo() {  
      delete(pScaledDeltas);
      //if (swingsWereSetNew) { delete(moveSwings); }
      tuplesDeleted++;
      if (tuplesDeleted % IRutil::TUPLEMODULUS == 0) {
         cerr << tuplesDeleted << " tuples deleted, " << tuplesMade 
              << " tuples made." << endl;
      }
   } 
 
   int getPlayedMoveIndex() const { 
      return useAltPlayedMoveIndex ? altPlayedMoveIndex  
                                   : playedMoveIndex; 
   } 
 
 
   /** The $64,000 place where model parameters are reflected, aside from the
       possible and deferred impact of ts.d and ts.v on the weighting of depths.
       The other parameters behave in the following ways under different policies:

        s ("sensitivity") always divides eval-difference and swing values in centipawns.
        c ("consistency") is the main exponent of eval-difference and swing values.
        a ("slack power") is a supplementary power on the swing(-difference) values alone.
                          It can be tied to c by fixing it to be 1.0.
       hm ("heave minus") gauges extra reactivity when the eval or swing is negative.
                          It can be tied to s by fixing it to be 1.0.
       hp ("heave plus")  gauges extra reactivity when the eval or swing is positive.
                          It can be tied to hm by fixing it to be 1.0.
        b ("heave slack") is additive when heave multiplies delta, and otherwise 
                          supplements a's power on positive swing.  
                          It can be tied to a by fixing it to be 1.0.

       Thus fixing all of a,hm,hp,b = 1.0 makes s and c alone govern swing (aside from d and v).
       Tying hp to hm enables hm to control the reaction to swing in general.
       The swing sw1 of the first move at the judgment depth can never be negative.
       Observations show that the first 8-12 moves have positive swing on average.
       When sw1 is subtracted off to make "relative" swing, however, all moves after the first
       have negative swing on average.  (Moves tied for equal-top value are a notable exception.)
       Hence hm is used as the primary swing multiplier when swing is tied, and the multiplier
       on positive swing is hp*hm*swing not just hp*swing.

       Sign convention: in (hm*swing)^a, hm itself is >= 0, and when swing is < 0, the value
                        is -(hm*|swing|)^a.

       By definition an optimal move at the judgment depth cannot have negative swing, so it can
       never appear "more probable" than the pre-swing indication, only "less probable."
       It is however possible that the -(hm*|swing|)^a term for a move of negative swing can
       make the overall argument negative.  It is also common that positive swing may render
       all of the arguments positive.  The same adjustment is applied in both cases:
       the least or most-negative argument "minArg" is subtracted from every argument.

       Here are the behaviors when either (i) "scale" appears in swingPolicy, or (ii) not.
       
       --------------------------------------------------------------------------------

       Situation (i): This adjustmewnt is intended to be used with NoScaling or with UnitLineScale.  
       It is applied primarily to delta, and can be applied to swing either after the
       (unscaled) swing is joined to the (unscaled) delta, or separately.

       The governing observation is that (depending on the skill/Elo of the sample), there
       are values em,hp,b > 0 such that as a function of the eval x for the player to move,
       the average delta per move over positions of eval x closely obeys these linear relations:

          avgdelta(x) = b + ep*|x|     if x > 0
          avgdelta(x) = b + em*|x|     if x <= 0

       Much like stabilizing a heaving ship, the model behaves better if the ramping up of
       error with x is flattened out by scaling delta->delta' so that (at least approximately)
     
          avgdelta'(x) = b

       regardless of x.  This is done for x > 0 by multiplying delta' = H*delta where
       H is either H' or H" according as whether the eval is negative or positive:

          H'  =  1/(1 + (em/b)x)  =  b/(b + em*x)  =  1 - em*x/(b + em*x),
          H"  =  1/(1 + (ep/b)x)  =  b/(b + ep*x)  =  1 - ep*x/(b + ep*x).

       If ep/b stayed constant, ditto em/b, then this would be something to fix rather than fit
       according to rating/skill level.  Unfortunately(?), it does not stay constant.  Hence we
       fit these quantities: em == hm, and ep == hm*hp.  The latter looks weird but allows tying.
       The formulas by various swing policies---recall the convention on powers of negative numbers:

       Absolute Swing:
       Join:  Form z = (delta + swing^a)/(1 + (hm/b)*eval)     if eval < 0;
                     = (delta + swing^a)                       if eval = 0;
                     = (delta - swing^a)/(1 + (hm*hp/b)*eval)  if eval > 0.
              Then apply Curve(z,s,c).  (Not necessarily dimensionally correct in swing^a)

       Link:  Form z = (delta/s + (swing/s)^a)/(1 + (hm/b)*eval)     if eval < 0;
                     = (delta/s + (swing/s)^a)                       if eval = 0;
                     = (delta/s - (swing/s)^a)/(1 + (hm*hp/b)*eval)  if eval > 0.
              Then apply Curve(z,1,c).
              
       Sep:   Form delta' = delta/(1 + (hm/b)*eval)            if eval < 0;
                          = delta                              if eval = 0;
                          = delta/(1 + (hp*hm/b)*eval)         if eval > 0;
                   swing' = swing/(1 + (hm/b)*eval)            if eval < 0;
                          = swing                              if eval = 0;
                          = swing/(1 + (hp*hm/b)*eval)         if eval > 0;

              Then form z = (delta'/s)^c + (swing'/s)^{ac}
              and apply Curve(z,1,1).

       Proxy: Form delta' and swing' as in Sep and form z = (delta'/s)^c.  
              Apply u = Curve(z,1,1).  
              Then form u' = u + (swing'/s)^{ac}.  Keep track of minProxy over all j,
              and subtract it off for each j to get the final proxies.


       Relative Swing: Replace swing by swing(j) - swing(ifm) in the above, where
       ifm is the index of the first move at the judgment depth.

       ----------------------------------------------------------------------------------

       Description of situation (ii): This assumes delta has already been scaled and that
       swing is using the scaled deltas, so that hm,hp become multipliers of swing and
       b becomes a compounded power over a.  Again the terms hp*hm/s and a*b*c facilitate tying.
       When s and c are outermost the 3-place curve is applied, else Curve(z,1,1).
       
       Absolute Swing:
       Join:     z = delta - (hm*|swing|)^a      if swing < 0;  
                   = delta                       if swing = 0;
                 z = delta + (hp*hm*swing)^{ab}  if swing > 0.
                 Then apply Curve(z,s,c).

       Link:     z = delta/s - (hm*|swing|/s)^a      if swing < 0;
                   = delta/s                         if swing = 0;
                 z = delta/s + (hp*hm*swing/s)^{ab}  if swing > 0.
                 Then apply Curve(z,1,c).

       Separate: z = (delta/s)^c - (hm*|swing|/s)^{ac}    if swing < 0;
                   = (delta/s)^c                          if swing = 0;
                   = (delta/s)^c + (hp*hm*swing/s)^{abc}  if swing > 0.
                 Then apply Curve(z,1,1).

       Proxy:    u  = Curve(delta,s,c) as in the plain (non-swing) model.  Then define
                 u' = u - (hm*|swing|/s)^{ac}       if swing < 0;
                 u' = u                             if swing = 0;
                 u' = u + (hp*hm*swing/s)^{abc}     if swing > 0.
                 As above, subtract off minProxy from each u' = u'(j) so that one is 0.0.

       Relative Swing: 
       For each move j, define swing'(j) = swing(j) - sw1, 
       where sw1 is the swing of the first move at the judgment depth.  As noted above,
       (*) sw1 >= 0 always.
       (*) on average swing'(j) is < 0 for other moves j, but can be > 0 individually.
       Hence believe-it-or-else there is a further breakdown according to whether the
       plus/minus reckoning is applied to swing'(j) on the whole or to its parts, and
       whether this is done before or after powering.  We expand swing' back into its
       parts and omit the easily-inferred swing=0 cases in the cornucopia that follows.
       We also rely more on the power convention for negative arguments.
       Note that sw1 when isolated is always multiplied by hp*hm since it is nonnegative.
       

       Join1:   z = delta + (hm*swing - hm*sw1)^a                     if swing - sw1 <= 0;
                  = delta + (hp*hm*swing - hp*hm*sw1)^{ab}            if swing - sw1 > 0;

       Join2:   z = delta + (hm*swing - hp*hm*sw1)^a                  if swing <= 0;
                  = delta + (hp*hm*swing - hp*hm*sw1)^{ab}            if swing > 0;

       Join3:   z = delta + (hm*swing)^a - (hp*hm*sw1)^{ab}           if swing <= 0;
                  = delta + (hp*hm*swing)^{ab} - (hp*hm*sw1)^{ab}     if swing > 0. 

       Link1:   z = delta/s + ((hm*swing - hm*sw1)/s)^a                     if swing - sw1 <= 0;
                  = delta/s + ((hp*hm*swing - hp*hm*sw1)/s)^{ab}            if swing - sw1 > 0;

       Link2:   z = delta/s + ((hm*swing - hp*hm*sw1)/s)^a                  if swing <= 0;
                  = delta/s + ((hp*hm*swing - hp*hm*sw1)/s)^{ab}            if swing > 0;

       Link3:   z = delta/s + (hm*swing/s)^a - (hp*hm*sw1/s)^{ab}           if swing <= 0;
                  = delta/s + (hp*hm*swing/s)^{ab} - (hp*hm*sw1/s)^{ab}     if swing > 0.

       Sep1:    z = (delta/s)^c + ((hm*swing - hm*sw1)/s)^{ac}           if swing - sw1 <= 0;
                  = (delta/s)^c + ((hp*hm*swing - hp*hm*sw1)/s)^{abc}    if swing - sw1 > 0;

       Sep2:    z = (delta/s)^c + ((hm*swing - hp*hm*sw1)/s)^{ac}              if swing <= 0;
                  = (delta/s)^c + ((hp*hm*swing - hp*hm*sw1)/s)^{abc}          if swing > 0;

       Sep3:    z = (delta/s)^c + (hm*swing/s)^{ac} - (hp*hm*sw1/s)^{abc}      if swing <= 0;
                  = (delta/s)^c + (hp*hm*swing/s)^{abc} - (hp*hm*sw1/s)^{abc}  if swing > 0;

       Proxy1:  u' = u + ((hm*swing - hm*sw1)/s)^{ac}                 if swing - sw1 <= 0;
                   = u + ((hp*hm*swing - hp*hm*sw1)/s)^{abc}          if swing - sw1 > 0;

       Proxy2:  u' = u + ((hm*swing - hp*hm*sw1)/s)^{ac}              if swing <= 0;
                   = u + ((hp*hm*swing - hp*hm*sw1)/s)^{abc}          if swing > 0;

       Proxy3:  u' = u + (hm*swing/s)^{ac} - (hp*hm*sw1/s)^{abc}      if swing <= 0;
                u' = u + (hp*hm*swing/s)^{abc} - (hp*hm*sw1/s)^{abc}  if swing > 0.

    */

   void accumeCurve3(const TrialSpec& ts, const vector<double>& args,
                     const double mul, CurveApplication* cap) const {
      double minArg = IRutil::MATEVAL;
      for (int j = 0; j < numDeltas; j++) {
         if (args.at(j) < minArg) {
            //minIndex = j;
            minArg = args.at(j);
         }
      }
      for (int j = 0; j < numDeltas; j++) {
         double dp = args.at(j) - minArg;
         if (dp >= 0.0) {
            cap->values.at(j) += mul*ts.mevalCurves3(dp,ts.s,ts.c);
         } else {   //means NAN value or etc.
/*
            string fen = pTurn->getFEN();
            if (IRutil::PROBLEM_FENS.find(fen) == IRutil::PROBLEM_FENS.end()) {
               IRutil::PROBLEM_FENS.insert(fen);
               IRutil::PROBLEM_GAMES.insert(pTurn->gameID + "\n" + fen);
               cerr << "Bad tuple value[" << j << "] = " << dp << " in\n" << string(*this) << endl
                    //<< "from turn" << endl << string(*pTurn) << endl;
                    << "args.at(j) = " << args.at(j) << ", minArg = " << minArg << endl
                    << "from game " << pTurn->gameID << endl
                    << "@position " << pTurn->getFEN() << endl;
            }
*/
            cap->values.at(j) += 0.0;   //i.e., do nothing
         } 
      }
   }

   void accumeCurve2(const TrialSpec& ts, const vector<double>& args,
                     const double mul, CurveApplication* cap) const {
      double minArg = IRutil::MATEVAL;
      for (int j = 0; j < numDeltas; j++) {
         if (args.at(j) < minArg) {
            //minIndex = j;
            minArg = args.at(j);
         }
      }
      for (int j = 0; j < numDeltas; j++) {
         double dp = args.at(j) - minArg;
         if (dp >= 0.0) {
            cap->values.at(j) += mul*ts.mevalCurves2(dp,ts.c);
         } else {   //means NAN value or etc.
/*
            string fen = pTurn->getFEN();
            if (IRutil::PROBLEM_FENS.find(fen) == IRutil::PROBLEM_FENS.end()) {
               IRutil::PROBLEM_FENS.insert(fen);
               IRutil::PROBLEM_GAMES.insert(pTurn->gameID + "\n" + fen);
               cerr << "Bad tuple value[" << j << "] = " << dp << " in\n" << string(*this) << endl
                    //<< "from turn" << endl << string(*pTurn) << endl;
                    << "args.at(j) = " << args.at(j) << ", minArg = " << minArg << endl
                    << "from game " << pTurn->gameID << endl
                    << "@position " << pTurn->getFEN() << endl;
            }
*/
            cap->values.at(j) += 0.0;
         }
      }
   }

   void accumeCurve1(const TrialSpec& ts, const vector<double>& args,
                     const double mul, CurveApplication* cap) const {
      double minArg = IRutil::MATEVAL;
      for (int j = 0; j < numDeltas; j++) {
         if (args.at(j) < minArg) {
            //minIndex = j;
            minArg = args.at(j);
         }
      }
      for (int j = 0; j < numDeltas; j++) {
         double dp = args.at(j) - minArg;
         if (dp >= 0.0) {
            cap->values.at(j) += mul*ts.mevalCurves1(dp);
         } else {   //means NAN value or etc.
/*
            string fen = pTurn->getFEN();
            if (IRutil::PROBLEM_FENS.find(fen) == IRutil::PROBLEM_FENS.end()) {
               IRutil::PROBLEM_FENS.insert(fen);
               IRutil::PROBLEM_GAMES.insert(pTurn->gameID + "\n" + fen);
               cerr << "Bad tuple value[" << j << "] = " << dp << " in\n" << string(*this) << endl
                    //<< "from turn" << endl << string(*pTurn) << endl;
                    << "args.at(j) = " << args.at(j) << ", minArg = " << minArg << endl
                    << "from game " << pTurn->gameID << endl
                    << "@position " << pTurn->getFEN() << endl;
            }
*/
            cap->values.at(j) += 0.0;
         }
      }
   }

   void finishProxies(CurveApplication* cap) const { //, const double denom) const {  //sets final minIndex
      double maxProxy = -IRutil::MATEVAL;
      size_t maxIndex = 0;
      for (int j = 0; j < numDeltas; j++) {
         //cap->values.at(j) /= denom;
         if (cap->values.at(j) > maxProxy) {
            maxProxy = cap->values.at(j);    //maxProxy cannot be > denom but can be < denom
            maxIndex = j;
         }
      }
      //if (maxIndex != cap->swapIndex) {
         //cout << "swap index changed from " << cap->swapIndex << " to " << maxIndex << endl;
      //}
      cap->swapIndex = maxIndex;
      for (int j = 0; j < numDeltas; j++) {
         cap->values.at(j) /= maxProxy;
      }
   }

   CurveApplication* applyCurve(const TrialSpec& ts) const {
      CurveApplication* cap = new CurveApplication(numDeltas); //will be accumulated
      vector<double> perceivedArgs(numDeltas);  //entries are replaced, not accumulated
      //next two used only if scaleHere
      const double evalMul = (evalPTM <= 0.0 ? (-ts.hm * evalPTM) : (ts.hm * ts.hp * evalPTM));
      const double deltaMul = 1.0 - IRutil::divifne0(evalMul, ts.b + evalMul);
      //with all these, C++ templates could speed up loops over moves with ts fixed.
      const double sw1 = moveSwings->at(0);
      const double s = (fabs(ts.s) < IRutil::MINPROB ? IRutil::MINPROB : ts.s);
      const double b = (fabs(ts.b) < IRutil::MINPROB ? IRutil::MINPROB : ts.b);
      const double s2 = (swingPolicy->decouple ? 1.0 : s);
      const double exab = (swingPolicy->decouple ? b : ts.a*b);
      const double exca = (swingPolicy->decouple ? ts.a : ts.c*ts.a);
      const double excab = (swingPolicy->decouple ? b : ts.c*ts.a*b);
      const double h2 = (swingPolicy->decouple ? ts.hp : ts.hm*ts.hp);
      double mul, dj, swj, sgnj, term1, term2;
      double denom = 0.0;
      APPLICATION_POLICIES appPolicy;

      if (!(sw1 >= 0)) {
         string fen = pTurn->getFEN();
         if (IRutil::PROBLEM_FENS.find(fen) == IRutil::PROBLEM_FENS.end()) {
            IRutil::PROBLEM_FENS.insert(fen);
            IRutil::PROBLEM_GAMES.insert(pTurn->gameID + "\n" + fen);
            cerr << "Bad sw1 " << sw1 << " in tuple " << string(*this) << endl
                 //<< "from turn" << endl << string(*pTurn) << endl;
                 << "from game " << pTurn->gameID << endl
                 << "@position " << pTurn->getFEN() << endl;
         }
      }

      if (swingPolicy->swingBook.empty()) {
         cerr << "Swing choices empty, using NO_SWING..." << endl;
         swingPolicy->swingBook[NO_SWING] = 1.0;
      }

      map<APPLICATION_POLICIES, double>::const_iterator swbit = swingPolicy->swingBook.begin();
      map<APPLICATION_POLICIES, double>::const_iterator swbite = swingPolicy->swingBook.end();
      while (swbit != swbite) {
         appPolicy = swbit->first;
         mul = swbit->second;
         denom += mul;

         switch (appPolicy) {
          case NO_SWING:
            for (int j = 0; j < numDeltas; j++) {
               perceivedArgs.at(j) = (swingPolicy->scaleHere ? pScaledDeltas->at(j)*deltaMul
                                                             : pScaledDeltas->at(j));
            }
            accumeCurve3(ts, perceivedArgs, mul, cap);
            break;
   
          case JOIN_ABS:
            if (swingPolicy->scaleHere) {
               if (evalPTM <= 0.0) {
                  for (int j = 0; j < numDeltas; j++) {
                     swj = moveSwings->at(j);
                     sgnj = (swj > 0 ? +1.0 : -1.0);
                     perceivedArgs.at(j) = (pScaledDeltas->at(j) + pow(swj*sgnj,ts.a))
                                         /(1.0 - evalPTM*ts.hm/b);
                  }
   	       } else {
                  for (int j = 0; j < numDeltas; j++) {
                     swj = moveSwings->at(j);
   		  sgnj = (swj > 0 ? +1.0 : -1.0);
   		  perceivedArgs.at(j) = (pScaledDeltas->at(j) + pow(swj*sgnj,ts.a))
                                         /(1.0 + evalPTM*h2/b);
                  }
               }
            } else {
               for (int j = 0; j < numDeltas; j++) {
                  swj = moveSwings->at(j);
                  sgnj = (swj > 0 ? +1.0 : -1.0);
                  if (swj <= 0.0) {
                     perceivedArgs.at(j) = pScaledDeltas->at(j) - pow(sgnj*swj*ts.hm, ts.a);
                  } else {
                     perceivedArgs.at(j) = pScaledDeltas->at(j) + pow(swj*h2, exab);
                  }
               }
            }
            accumeCurve3(ts, perceivedArgs, mul, cap);
            break;
   
          case JOIN_REL1:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - sw1 <= 0.0 ? -1.0 : +1.0);
               if (sgnj <= 0.0) {
                  perceivedArgs.at(j) = pScaledDeltas->at(j) - pow(sgnj*(swj-sw1)*ts.hm, ts.a);
               } else {
                  perceivedArgs.at(j) = pScaledDeltas->at(j) + pow((swj-sw1)*h2, exab);
               }
            }
            accumeCurve3(ts, perceivedArgs, mul, cap);
            break;
   
          case JOIN_REL2:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - ts.hp*sw1 <= 0.0 ? -1.0 : +1.0);
               if (swj <= 0.0) {
                  perceivedArgs.at(j) = pScaledDeltas->at(j) + sgnj*pow(sgnj*(ts.hm*swj - h2*sw1), ts.a);
               } else {
                  sgnj = (swj - sw1 <= 0.0 ? -1.0 : +1.0);
                  perceivedArgs.at(j) = pScaledDeltas->at(j) + sgnj*pow(sgnj*(swj-sw1)*h2, exab);
               }
            }
            accumeCurve3(ts, perceivedArgs, mul, cap);
            break;
   
          case JOIN_REL3:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj <= 0 ? -1.0 : +1.0);
               if (swj <= 0.0) {
                  perceivedArgs.at(j) = pScaledDeltas->at(j) 
                                      + sgnj*pow(sgnj*ts.hm*swj, ts.a)
                                      - pow(h2*sw1, exab);
               } else {
                  perceivedArgs.at(j) = pScaledDeltas->at(j)
                                      + sgnj*pow(h2*swj, exab)
                                      - pow(h2*sw1, exab);
               }
            }
            accumeCurve3(ts, perceivedArgs, mul, cap);
            break;
   
          case LINK_ABS:
            if (swingPolicy->scaleHere) {
               if (evalPTM <= 0.0) {
                  for (int j = 0; j < numDeltas; j++) {
                     swj = moveSwings->at(j);
                     sgnj = (swj > 0 ? +1.0 : -1.0);
                     perceivedArgs.at(j) = (pScaledDeltas->at(j)/s + pow(swj*sgnj/s,ts.a))
                                         /(1.0 - evalPTM*ts.hm/b);    //must be s ^ not s2
                  }
               } else {
                  for (int j = 0; j < numDeltas; j++) {
                     swj = moveSwings->at(j);
                     sgnj = (swj > 0 ? +1.0 : -1.0);
                     perceivedArgs.at(j) = (pScaledDeltas->at(j)/s + pow(swj*sgnj/s,ts.a))
                                         /(1.0 + evalPTM*h2/b);       //must be s ^ not s2
                  }
               }
            } else {
               for (int j = 0; j < numDeltas; j++) {
                  swj = moveSwings->at(j);
                  sgnj = (swj > 0 ? +1.0 : -1.0);
                  if (swj <= 0.0) {
                     perceivedArgs.at(j) = pScaledDeltas->at(j)/s - pow(sgnj*swj*ts.hm/s2, ts.a);
                  } else {
                     perceivedArgs.at(j) = pScaledDeltas->at(j)/s + pow(swj*h2/s2, exab);
                  }
               }
            }
            accumeCurve2(ts, perceivedArgs, mul, cap);
            break;
   
          case LINK_REL1:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - sw1 <= 0 ? -1.0 : +1.0);
               if (sgnj <= 0.0) {
                  perceivedArgs.at(j) = pScaledDeltas->at(j)/s - pow(sgnj*(swj-sw1)*ts.hm/s2, ts.a);
               } else {
                  perceivedArgs.at(j) = pScaledDeltas->at(j)/s + pow((swj-sw1)*h2/s2, exab);
               }
            }
            accumeCurve2(ts, perceivedArgs, mul, cap);
            break;
   
          case LINK_REL2:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - ts.hp*sw1 <= 0 ? -1.0 : +1.0);
               if (swj <= 0.0) {
                  perceivedArgs.at(j) = pScaledDeltas->at(j)/s 
                                           + sgnj*pow(sgnj*(ts.hm*swj - h2*sw1)/s2, ts.a);
               } else {
                  sgnj = (swj - sw1 <= 0.0 ? -1.0 : +1.0);
                  perceivedArgs.at(j) = pScaledDeltas->at(j)/s 
                                           + sgnj*pow(sgnj*(swj-sw1)*h2/s2, exab);
               }
            }
            accumeCurve2(ts, perceivedArgs, mul, cap);
            break;
   
          case LINK_REL3:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj <= 0.0 ? -1.0 : +1.0);
               if (sgnj <= 0.0) {
                  perceivedArgs.at(j) = pScaledDeltas->at(j)/s
                                      + sgnj*pow(sgnj*ts.hm*swj/s2, ts.a)
                                      - pow(h2*sw1/s2, exab);
               } else {
                  perceivedArgs.at(j) = pScaledDeltas->at(j)/s
                                      + sgnj*pow(h2*swj/s2, exab)
                                      - pow(h2*sw1/s2, exab);
               }
            }
            accumeCurve2(ts, perceivedArgs, mul, cap);
            break;
   
          case SEPARATE_ABS:
            if (swingPolicy->scaleHere) {
               if (evalPTM <= 0.0) {
                  for (int j = 0; j < numDeltas; j++) {
                     swj = moveSwings->at(j);
                     sgnj = (swj > 0 ? +1.0 : -1.0);
                     term1 = pScaledDeltas->at(j)/(1.0 - evalPTM*ts.hm/b);
                     term2 = sgnj*swj/(1.0 - evalPTM*ts.hm/b);
                     perceivedArgs.at(j) = pow(term1/s, ts.c) + sgnj*pow(term2/s2, exca);
                  }
               } else {
                  for (int j = 0; j < numDeltas; j++) {
                     swj = moveSwings->at(j);
                     sgnj = (swj > 0 ? +1.0 : -1.0);
                     term1 = pScaledDeltas->at(j)/(1.0 - evalPTM*h2/b);
                     term2 = sgnj*swj/(1.0 - evalPTM*h2/b);
                     perceivedArgs.at(j) = pow(term1/s, ts.c) + sgnj*pow(term2/s2, exca);
                  }
               }
            } else {
               for (int j = 0; j < numDeltas; j++) {
                  swj = moveSwings->at(j);
                  sgnj = (swj > 0 ? +1.0 : -1.0);
                  term1 = pScaledDeltas->at(j);
                  if (swj <= 0.0) {
                     term2 = ts.hm*sgnj*swj;
                     perceivedArgs.at(j) = pow(term1/s,ts.c) - pow(term2/s2, exca);
                  } else {
                     term2 = h2*swj;
                     perceivedArgs.at(j) = pow(term1/s,ts.c) + pow(term2/s2, excab);
                  }
               }
            }
            accumeCurve1(ts, perceivedArgs, mul, cap);
            break;
   
          case SEPARATE_REL1:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - sw1 <= 0 ? -1.0 : +1.0);
               term1 = pScaledDeltas->at(j);
               if (sgnj <= 0.0) {
                  term2 = sgnj*ts.hm*(swj - sw1);
                  perceivedArgs.at(j) = pow(term1/s, ts.c) + sgnj*pow(term2/s2, exca);
               } else {
                  term2 = sgnj*h2*(swj - sw1);
                  perceivedArgs.at(j) = pow(term1/s, ts.c) + sgnj*pow(term2/s2, excab);
               }
            }
            accumeCurve1(ts, perceivedArgs, mul, cap);
            break;
   
          case SEPARATE_REL2:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - ts.hp*sw1 <= 0 ? -1.0 : +1.0);
               term1 = pScaledDeltas->at(j);
               if (swj <= 0.0) {
                  term2 = sgnj*(ts.hm*swj - h2*sw1);
                  perceivedArgs.at(j) = pow(term1/s, ts.c) + sgnj*pow(term2/s2, exca);
               } else {
                  sgnj = (swj - sw1 <= 0.0 ? -1.0 : +1.0);
                  term2 = sgnj*h2*(swj - sw1);
                  perceivedArgs.at(j) = pow(term1/s, ts.c) + sgnj*pow(term2/s2, excab);
               }
            }
            accumeCurve1(ts, perceivedArgs, mul, cap);
            break;
   
          case SEPARATE_REL3:   //assume not scaling here
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj <= 0 ? -1.0 : +1.0);
               term1 = pScaledDeltas->at(j);
               if (swj <= 0.0) {
                  perceivedArgs.at(j) = pow(term1/s, ts.c)
                                      + sgnj*pow(sgnj*ts.hm*swj/s2, exca)
                                      - pow(h2*sw1/s2, excab);
               } else {
                  perceivedArgs.at(j) = pow(term1/s, ts.c)
                                      + pow(h2*swj/s2, excab)
                                      - pow(h2*sw1/s2, excab);
               }
            }
            accumeCurve1(ts, perceivedArgs, mul, cap);
            break;
   
          case PROXY_ABS:
            //cap = new CurveApplication(numDeltas);
            if (swingPolicy->scaleHere) {
               if (evalPTM <= 0.0) {
                  for (int j = 0; j < numDeltas; j++) {
                     term1 = pScaledDeltas->at(j)/(1.0 - evalPTM*ts.hm/b);
                     cap->values.at(j) += mul*(ts.evalCurves1(pow(term1/s,ts.c)));
                     swj = moveSwings->at(j);
                     sgnj = (swj > 0 ? +1.0 : -1.0);
                     term2 = sgnj*swj/(1.0 - evalPTM*ts.hm/b);
                     cap->values.at(j) += mul*sgnj*pow(term2/s2, exca);
                  }
               } else {
                  for (int j = 0; j < numDeltas; j++) {
                     term1 = pScaledDeltas->at(j)/(1.0 - evalPTM*h2/b);
                     cap->values.at(j) += mul*(ts.evalCurves1(pow(term1/s,ts.c)));
                     swj = moveSwings->at(j);
                     sgnj = (swj > 0 ? +1.0 : -1.0);
                     term2 = sgnj*swj/(1.0 - evalPTM*h2/b);
                     cap->values.at(j) += mul*sgnj*pow(term2/s2, exca);
                  }
               }
            } else {
               for (int j = 0; j < numDeltas; j++) {
                  swj = moveSwings->at(j);
                  sgnj = (swj > 0 ? +1.0 : -1.0);
                  term1 = pScaledDeltas->at(j);
                  cap->values.at(j) += mul*(ts.evalCurves1(pow(term1/s,ts.c)));
                  if (swj <= 0.0) {
                     term2 = ts.hm*sgnj*swj;
                     cap->values.at(j) -= mul*pow(term2/s2, exca);
                  } else {
                     term2 = h2*swj;
                     cap->values.at(j) += mul*pow(term2/s2, excab);
                  }
               }
            }
            //finishProxies(cap);
            //return cap;
            break;
   
          case PROXY_REL1:   //assume not scaling here
            //cap = new CurveApplication(numDeltas);
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - sw1 <= 0 ? -1.0 : +1.0);
               term1 = pScaledDeltas->at(j);
               cap->values.at(j) += mul*(ts.evalCurves1(pow(term1/s,ts.c)));
               if (sgnj <= 0.0) {
                  term2 = sgnj*ts.hm*(swj - sw1);
                  cap->values.at(j) += mul*sgnj*pow(term2/s2, exca);
               } else {
                  term2 = sgnj*h2*(swj - sw1);
                  cap->values.at(j) += mul*sgnj*pow(term2/s2, excab);
               }
            }
            //finishProxies(cap);
            //return cap;
            break;
   
          case PROXY_REL2:   //assume not scaling here
            //cap = new CurveApplication(numDeltas);
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj - ts.hp*sw1 <= 0 ? -1.0 : +1.0);
               term1 = pScaledDeltas->at(j);
               cap->values.at(j) += mul*(ts.evalCurves1(pow(term1/s,ts.c)));
               if (swj <= 0.0) {
                  term2 = sgnj*(ts.hm*swj - h2*sw1);
                  cap->values.at(j) += mul*sgnj*pow(term2/s2, exca);
               } else {
                  sgnj = (swj - sw1 <= 0.0 ? -1.0 : +1.0);
                  term2 = sgnj*h2*(swj - sw1);
                  cap->values.at(j) += mul*sgnj*pow(term2/s2, excab);
               }
            }
            //finishProxies(cap);
            //return cap;
            break;
   
          case PROXY_REL3:   //assume not scaling here
            //cap = new CurveApplication(numDeltas);
            for (int j = 0; j < numDeltas; j++) {
               swj = moveSwings->at(j);
               sgnj = (swj <= 0 ? -1.0 : +1.0);
               term1 = pScaledDeltas->at(j);
               cap->values.at(j) += mul*(ts.evalCurves1(pow(term1/s,ts.c)));
               if (swj <= 0.0) {
                  cap->values.at(j) += mul*(sgnj*pow(sgnj*ts.hm*swj/s2, exca) - pow(h2*sw1/s2, excab));
               } else {
                  cap->values.at(j) += mul*(pow(h2*swj/s2, excab) - pow(h2*sw1/s2, excab));
               }
            }
            //finishProxies(cap);
            //return cap;
            break;
         }
         swbit++;
      }
      finishProxies(cap);  //, denom);
      return cap;
   }


   operator string() const { 
      string out = "";
      if (pScaledDeltas->size() == 0) {
         cerr << "Empty tuple found---error" << endl;
      } else {
         out += "EvalPTM ";
         out += IRutil::ftoa(evalPTM,2) + string(", deltas and swings used:\n");
         //out += IRutil::ftoa(pScaledDeltas->at(0),2); 
         int j = 0; 
         while (j < pScaledDeltas->size()) { 
            out += IRutil::itoa(j,2) + " ";
            out += IRutil::ftoa(pScaledDeltas->at(j),2); 
            out += " + ";
            if (j < moveSwings->size()) {
               out += IRutil::ftoa(moveSwings->at(j),3);
            } else {
               out += "NoSwing";
            }
            out += "\n";
            j++; 
         }
         //out.at(out.size()-1) = ')';
      }   
      return out;
   }

}; 
 

#endif    //end of #ifndef __IR_TUPLE_INFO_H__ 
 
 
 
