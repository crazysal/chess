//File------------------------IRmain.cpp----------------------------------- 
 
/** File "IRmain.cpp", or the end of "IRall1file.cpp", by KWR. 
    See header comment in "IRincludes.h" (or at the top of "IRall1file.cpp") 
    for description and detailed use.  The following organization enables 
    this code to be compiled as one file or via inclusion of 20 files, 
    without using linking or a Makefile.  So just g++ or CC (etc.) this baby! 
    And do it -fast, man!---I mean with highest optimization! 
 */ 
 
#ifndef __IR_INCLUDES_H__ 
#include "IRincludes.h" 
#endif	  //end of #ifndef __IR_INCLUDES_H__ 
 
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
 
#ifndef __IR_TRIAL_FILTERS_H__ 
#include "TrialFilters.h" 
#endif 

#ifndef __IR_MULTIMIN_H__
#include "multimin.h"
#include "multimin.c"
#endif

#ifndef __IR_MPFIT_H__
#include "mpfit.h"
#include "mpfit.c"
#endif

 
#ifndef __IR_MINIMIZER_H__ 
#include "Minimizer.h" 
#endif	  //end of #ifndef __IR_MINIMIZER_H__ 
 
#ifndef __IR_MENUS_H__ 
#include "Menus.h" 
#endif	  //end of #ifndef __IR_MENUS_H__ 
 
#ifndef __IR_DEMOS_H__ 
#include "Demos.h" 
#endif	  //end of #ifndef __IR_DEMOS_H__ 
 
#ifndef __IR_ENSEMBLE_H__ 
#include "Ensemble.h" 
#endif	  //end of #ifndef __IR_ENSEMBLE_H__ 
 
 
void muteElistaSetup(Ensemble& ens) { 
   string strm = ""; 
   istringstream* istrp; 
   strm += " deltaScale y moreOptions FullFractalScale noPatch depthWindow 30 30 ";
   strm += " swingWindow 5 19 zeroDepth 1 judgmentDepth 0 normFactors omodo 1.01 finishScale ";
   strm += " newFilters EvalWithin ew3norm 3 0 y PrevEvalWithin pw3norm 3 0 y NextEvalWithin nw3norm 3 0 y ";
   strm += " EvalWithin ew4norm 4 0 y PrevEvalWithin pw4norm 4 0 y NextEvalWithin nw4norm 4 0 y ";
   strm += " EvalWithin ew5norm 5 0 y PrevEvalWithin pw5norm 5 0 y NextEvalWithin nw5norm 5 0 y ";
   strm += " OrFilter pew3norm ew3norm y pw3norm n OrFilter pnew3norm pw3norm y ew3norm y nw3norm n ";
   strm += " OrFilter pew4norm ew4norm y pw4norm n OrFilter pnew4norm pw4norm y ew4norm y nw4norm n ";
   strm += " OrFilter pew5norm ew5norm y pw5norm n OrFilter pnew5norm pw5norm y ew5norm y nw5norm n ";

   strm += " SecondDelta sdleq4 leq 4 ";
   strm += " EventIs SanLuis05 Luis EventIs Mexico07 Mexico ";
   strm += " OrFilter SL05+Mex07 SanLuis05 y Mexico07 n ";
   strm += " RepCount RC0 eq 0 n ";
   strm += " done ";

   
   strm += " newSelectors MoveEvalSelector EvalGoesToZero n y eq 0.00 0 ";
   strm += " MoveRawDeltaSelector Error050 n geq 0.50 0 ";
   strm += " MoveRawDeltaSelector Error100 n geq 1.00 0 ";
   strm += " MoveRawDeltaSelector Error200 n geq 2.00 0 ";
   strm += " MoveRawDeltaSelector Error400 n geq 4.00 0 ";
   strm += " MoveRawDeltaSelector delgeq01 n geq 0.01 0 ";
   strm += " MoveRawDeltaSelector delleq10 n leq 0.10 0 ";
   strm += " MoveRawDeltaSelector delgeq11 n geq 0.11 0 ";
   strm += " MoveRawDeltaSelector delleq30 n leq 0.30 0 ";
   strm += " MoveRawDeltaSelector delgeq31 n geq 0.31 0 ";
   strm += " MoveRawDeltaSelector delleq70 n leq 0.70 0 ";
   strm += " MoveRawDeltaSelector delgeq71 n geq 0.71 0 ";
   strm += " MoveRawDeltaSelector delleq150 n leq 1.50 0 ";
   strm += " done ";

   strm += " newSelectors AndSelector Delta01-10 no delgeq01 y delleq10 n ";
   strm += " AndSelector Delta11-30 no delgeq11 y delleq30 n ";
   strm += " AndSelector Delta31-70 no delgeq31 y delleq70 n ";
   strm += " AndSelector Delta71-150 no delgeq71 y delleq150 n done ";
   strm += " toggleSelectors delgeq01 delleq10 delgeq11 delleq30 delgeq31 delleq70 delgeq71 delleq150 go ";

   strm += " clearFilters attach RC0 y pnew4norm n ";
   strm += " showTrial n "; 
   strm += " addOutputFile " + IRutil::SESSIONFILENAME; 
   istrp = new istringstream(strm); 
   ens.interactFromStream(istrp, ""); 
} 

void basicSetup(Ensemble& ens) {
   //muteElistaSetup(ens);
   ens.hideFilter(new WhiteToMoveFilter());
   ens.hideFilter(new BlackToMoveFilter());
   ens.hideFilter(new DrawnGameFilter());
   ens.hideFilter(new BlackWinFilter());
   ens.hideFilter(new WhiteWinFilter());
   ens.hideFilter(new PlayerWinFilter());
   ens.hideFilter(new PlayerLossFilter());
   ens.hideFilter(new FirstLineMatchFilter());
   ens.hideFilter(new EqualTopMoveFilter());
   ens.hideFilter(new InCheckFilter());
   ens.hideFilter(new EventTypeFilter("swiss", "Swiss"));
   ens.hideFilter(new EventTypeFilter("tourn", "RR-tourn"));
   ens.hideFilter(new EventTypeFilter("rapid", "Rapid"));
   ens.hideFilter(new EventTypeFilter("blitz", "Blitz"));
   ens.hideFilter(new TeamEventFilter());
   ens.hideFilter(new EventTypeFilter("schev", "Schev"));
   ens.hideFilter(new EventTypeFilter("simul", "Simul"));
   ens.hideFilter(new EventTypeFilter("ko", "Knockout"));
   ens.hideFilter(new EngineFilter("omod", "10", "Komodo 10"));
   ens.hideFilter(new EngineFilter("omod", "", "Komodo-all-versions"));
   ens.hideFilter(new EngineFilter("omod", "9.42", "Komodo-9.42"));
   ens.hideFilter(new EngineFilter("omod", "9.3", "Komodo-9.3.x"));
   ens.hideFilter(new EngineFilter("omod", "9.2", "Komodo-9.2.x"));
   ens.hideFilter(new EngineFilter("ockf", "7", "Stockfish-7"));
   ens.hideFilter(new EngineFilter("ockf", "6", "Stockfish-6"));
   ens.hideFilter(new EngineFilter("ockf", "", "Stockfish-all-versions"));
   ens.hideFilter(new EngineFilter("ybka", "", "Rybka-all-versions"));
   ens.hideFilter(new EngineFilter("oudi", "", "Houdini-all-versions"));

   ens.addSelector(new PawnMoveSelector());
   ens.addSelector(new KnightMoveSelector());
   ens.addSelector(new BishopMoveSelector());
   ens.addSelector(new RookMoveSelector());
   ens.addSelector(new QueenMoveSelector());
   ens.addSelector(new KingMoveSelector());
   ens.addSelector(new CastlingSelector());
   ens.addSelector(new CaptureSelector());
   ens.addSelector(new NonCaptureSelector());
   ens.addSelector(new PromotionSelector());
   ens.addSelector(new AdvancingMoveSelector());
   ens.addSelector(new RetreatingMoveSelector());
   ens.addSelector(new SidewaysMoveSelector());
   ens.addSelector(new CheckingMoveSelector());
   ens.addSelector(new EngineMoveSelector());
   ens.addSelector(new PlayedMoveSelector());
   ens.addSelector(new SamePieceAsPrevMoveSelector());
   //ens.addSelector(new SwingNegativeSelector(ens.getFocusTrial(), false));
   //ens.addSelector(new SwingNonNegativeSelector(ens.getFocusTrial(), false));
   MoveSelector* etv = new EqualTopMoveSelector();
   ens.addSelector(etv);

/*
   ens.addSelector(new MoveRawDeltaSelector(GEQ, 0.01, 0, false, "delgeq01"));
   ens.addSelector(new MoveRawDeltaSelector(LEQ, 0.10, 0, false, "delleq10"));
   ens.addSelector(new MoveRawDeltaSelector(GEQ, 0.11, 0, false, "delgeq11"));
   ens.addSelector(new MoveRawDeltaSelector(LEQ, 0.30, 0, false, "delleq30"));   
   ens.addSelector(new MoveRawDeltaSelector(GEQ, 0.31, 0, false, "delgeq31"));
   ens.addSelector(new MoveRawDeltaSelector(LEQ, 0.70, 0, false, "delleq70"));
   ens.addSelector(new MoveRawDeltaSelector(GEQ, 0.71, 0, false, "delgeq71"));
   ens.addSelector(new MoveRawDeltaSelector(LEQ, 1.50, 0, false, "delleq150"));
*/
   

/*
MoveRawDeltaSelector(const COMPARE comp, const double gdelta, const size_t gdepth,
                     bool nullExcludes, const string& name)
   ens.justAddSelector(new PawnMoveSelector());
   ens.justAddSelector(new KnightMoveSelector());
   ens.justAddSelector(new BishopMoveSelector());
   ens.justAddSelector(new RookMoveSelector());
   ens.justAddSelector(new QueenMoveSelector());
   ens.justAddSelector(new KingMoveSelector());
   ens.justAddSelector(new CastlingSelector());
   ens.justAddSelector(new CaptureSelector());
   ens.justAddSelector(new NonCaptureSelector());
   ens.justAddSelector(new PromotionSelector());
   ens.justAddSelector(new AdvancingMoveSelector());
   ens.justAddSelector(new RetreatingMoveSelector());
   ens.justAddSelector(new SidewaysMoveSelector());
   ens.justAddSelector(new CheckingMoveSelector());
   ens.justAddSelector(new EngineMoveSelector());
   ens.justAddSelector(new PlayedMoveSelector());
   ens.justAddSelector(new SamePieceAsPrevMoveSelector());
   ens.justAddSelector(new SwingNegativeSelector(ens.getFocusTrial(), false));
   ens.justAddSelector(new SwingNonNegativeSelector(ens.getFocusTrial(), false));
   MoveSelector* etv = new EqualTopMoveSelector();
   ens.justAddSelector(etv);
   //ens.setSelector(etv);
*/ 
}

 
int main() { 
   string ans, elistaAns; 
   string dataPath = "/projects/regan/Chess/CSE712/AIF/";
   cout << "Use \"PowerShares\" trial with path " << dataPath 
        << " to file SF7Turns.aif? (y/n) " << endl;
   cin >> ans; 
   ans = IRutil::trim(ans); 
   /*
   cout << "Run Elista Demo? (y/n) " << endl; 
   cin >> elistaAns; 
   elistaAns = IRutil::trim(elistaAns); 
   */
   IRfun::initNames();   //sets up curve and weight-method menus
   initPolicyNames();
   LineScale* ls;
 
   if (IRutil::isYes(ans)) {   // || IRutil::isYes(elistaAns)) { 
      //TrialSpec ts(IRfun::INVEXP, 0.0824614, 0.519043, IRfun::UNITWTS, "SLMfit"); 
      map<IRfun::CURVES, double> curveBook;
      curveBook[IRfun::INVEXP] = 1.0;
      TrialSpec ts(IRfun::INVEXP, curveBook, IRfun::UNITWTS, "start",
                   //0.0214748, 0.532259, 15.4764, 0.0934588, 0.007780, 0.0, 0.0, 0.372179);
                   //0.02, 0.53, 14.5, 0.05, 1.00, 1.61, 1.0, 1.00);
                   0.05, 0.60, 12.5, 0.05, 1.00, 1.00, 1.00, 1.00,
                   false, IRfun::FOLDEDLOGISTIC, 0.00, 2.00, 5.00, 1.0, 1.0, 0.01, 0.00);

      map<string, double> engineInits;
      engineInits[string("omodo")] = 1.01;
      engineInits[string("tockfish")] = 1.0;
      map<APPLICATION_POLICIES, double> appp;
      appp[SEPARATE_REL2] = 1.0;
      //appp[LINK_REL2] = 1;
      SwingInfo* swi = new SwingInfo(appp, true, false, true, false, false, 0, false, false, 0.0);

      //ls = new TranslateByEval(new MulScale("MulScale", false, swi, NO_PATCH, 
                                 //IRutil::CENTRALPATCHPOWER, 0.00, engineInits, 
                                 //5, 30, 30, 5, 19, 0, 1.0, 1.600, 1.200, true));

      //ls = new TranslateByEval(new FullFractalScale("FullFractalScale", true, swi, NO_PATCH,
            //IRutil::CENTRALPATCHPOWER, 0.00, engineInits,
            //1, 30, 30, 5, 20, 0, 1.0, 1.6, 1.2));   //zero radius; 
      
      ls = new SimpleFullFractalScale("SimpleFullFractalScale", true, swi, NO_PATCH,
            IRutil::CENTRALPATCHPOWER, 0.00, engineInits,
            1, 30, 30, 5, 19, 0, 1.0, 1.6, 1.2);   //zero radius;
      ls->setDeltaCap(IRutil::DELTACAP);

      //noPatch, WEF, cap 10.00; (sepRel2:1),damped
      string prefix = ", noPatch, WEF, cap " + IRutil::ftoa(IRutil::DELTACAP,2) + "; " + string(*swi);
      string infix = "[30 .. 30, 5 .. 19; 0]";
      string suffix = "(0, 1.0, 1.6, 1.2, 1.0, 1.0)";
      ls->extendName(prefix + "\n" + infix + suffix);

      //ls will be superseded in muteElistaSetup but is added to the option list

      Trial* ft = new Trial("BasicPowerShares", new list<TurnInfo*>(), ts, 
                            Models::POWER_SHARES, false, true, ls); 
                            //Models::SHARES, false, true, ls);
 
      ofstream* filep = new ofstream(IRutil::LOGFILENAME.c_str(), ios::app); 
      bool check = filep->is_open(); 
      if (!check) { 
         cerr << "Cannot read the input file, using null log..." << endl; 
      }
      ostream* checkp = new IRutil::nullstream(); 
      if (check) { checkp = filep; } 
      cout << "Creating Ensemble now..." << endl;
      Ensemble ens(ft, dataPath, checkp);             //sets timestamp before reading 
      //ens.readInputFile("R3Turns.txt"); 

      cout << "Reading input now...";
      //ens.readInputFile("R3Turns.aif", false, false); 
      ens.readTurnsFromFileGlob("SF7Turns.aif");
      cout << "done." << endl;
 
      muteElistaSetup(ens);
      basicSetup(ens);
      ens.loggedWrite("",true);  //inserts blank line after log of initializations
 
      cout << "Interacting now." << endl;
      ens.interact(); 

   } else { 
      Ensemble ens; 
      cout << "Please enter full default data path: " << endl;
      getline(cin,dataPath);
      dataPath = IRutil::trim(dataPath);
      ens.setDataPath(dataPath);
      ens.interact(); 
   } 

   cerr << endl << "Games and positions with issues:" << endl;
   set<string>::const_iterator itg = IRutil::PROBLEM_GAMES.begin();
   while (itg != IRutil::PROBLEM_GAMES.end()) { cerr << *itg++ << endl; }

   cerr << endl << "Exiting...";
   delete(ls);
   cerr << "done." << endl;
} 
