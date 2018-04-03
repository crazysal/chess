//File-----------------------Demos.cpp--------------------------------------- 
 
#ifndef __IR_DEMOS_CPP__ 
#define __IR_DEMOS_CPP__ 

#ifndef __IR_DEMOS_H__
#include "Demos.h"
#endif    //end of #ifndef __IR_DEMOS_H__

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
#endif

#ifndef __IR_MPFIT_H__
#include "mpfit.h"
#endif

 
#ifndef __IR_MINIMIZER_H__ 
#include "Minimizer.h" 
#endif	  //end of #ifndef __IR_MINIMIZER_H__ 
 
#ifndef __IR_MENUS_H__ 
#include "Menus.h" 
#endif	  //end of #ifndef __IR_MENUS_H__ 
 
#ifndef __IR_ENSEMBLE_H__ 
#include "Ensemble.h" 
#endif	  //end of #ifndef __IR_ENSEMBLE_H__ 
 
 
//-------------------------USER-PROGRAMMABLE DEMOS---------------------------- 
 
namespace Demos { 
 
   ofstream* agreeDemoFile(Ensemble& ens, string& fileName) { 
      ofstream* filep; 
      fileName = IRutil::DEMOFILENAME; 
      string ans = ""; 
      cout << "Use " << fileName << " as results file? (y/n) "; 
      ens.loggedRead(ans); 
      if (IRutil::isYes(ans)) { 
         filep = new ofstream(fileName.c_str(), ios::app); 
         if (!filep->is_open()) { 
            cerr << "Something is amiss with the demo file, please " 
                 << "select an alternate." << endl; 
            ans = "no"; 
         } 
      } 
      if (!IRutil::isYes(ans)) {     
            //might have fall-thru from IRutil::isYes(ans) case. 
         bool noFile = true; 
         while(noFile) { 
            cout << "Enter filename---avoid already-attached output files: "; 
            ens.loggedRead(fileName); 
            filep = new ofstream(fileName.c_str(), ios::app); 
            if(!filep->is_open()) { 
               cerr << "Cannot stat results file, please try again..." 
                    << endl; 
            } else { 
               noFile = false; 
            } 
         } 
      } //control here means fileName is name of valid ofstream *filep. 
      return filep; 
   } 
 
   istream& findLineWith(istream& IN, const string& pat, string& line) { 
      while ((!IRutil::delimitedMatch(pat,line)) && (!IN.eof())) { 
         getline(IN,line); 
      } 
      if (IN.eof()) { 
         cerr << "End-of-file reached without finding " << pat << endl; 
      } 
      return IN; 
   } 
 
   /** Extract PerfData information back from a PerfData.fullReport(). 
       Moves the IN pointer as a side-effect.  Per stream conventions, 
       extracts to ref variable and returns reference to modified stream. 
       !! As of 8/16/08, does *not* fill the arrays in the PerfData returned. 
    */ 
   ifstream& parsePerfData(ifstream& IN, const string& fromPat, PerfData& perf) { 
      string line; 
      findLineWith(IN, fromPat, line); 
      findLineWith(IN, "total turns", line); 
      vector<double> ed = IRutil::extractDoubles(line); 
 
      perf.numTurns = int(ed[0]); 
      perf.totalBootItems = int(ed[0]);
      perf.accumWeight = ed[1]; 
 
      findLineWith(IN, "first-move", line); 
      perf.weighted = (line[0] == 'W'); 
 
      ed = IRutil::extractDoubles(line); 
      perf.moveMatchWtd.projectedValue = ed[0]; 
      perf.moveMatchWtd.projectedSigma = ed[1]; 
      perf.moveMatchWtd.projectedFreq = ed[2]; 
 
      findLineWith(IN, "range", line); 
      ed = IRutil::extractDoubles(line); 
      perf.moveMatchWtd.projected2SigmaLeft = ed[0]; 
      perf.moveMatchWtd.projected2SigmaRight = ed[1]; 
      perf.moveMatchWtd.projected2SigmaLeftFreq = ed[2]; 
      perf.moveMatchWtd.projected2SigmaRightFreq = ed[3]; 
 
      findLineWith(IN, "top-move", line); 
      ed = IRutil::extractDoubles(line); 
      perf.equalValueMatchWtd.projectedValue = ed[0]; 
      perf.equalValueMatchWtd.projectedSigma = ed[1]; 
      perf.equalValueMatchWtd.projectedFreq = ed[2]; 
 
      findLineWith(IN, "range", line); 
      ed = IRutil::extractDoubles(line); 
      perf.equalValueMatchWtd.projected2SigmaLeft = ed[0];
      perf.equalValueMatchWtd.projected2SigmaRight = ed[1];
      perf.equalValueMatchWtd.projected2SigmaLeftFreq = ed[2];
      perf.equalValueMatchWtd.projected2SigmaRightFreq = ed[3];
 
      findLineWith(IN, "played-move", line); 
      ed = IRutil::extractDoubles(line); 
      perf.playedMoveMatchWtd.projectedValue = ed[0];  //predPlayedMatchesWtd = ed[0]; 
      perf.playedMoveMatchWtd.projectedSigma = ed[1]; 
      perf.playedMoveMatchWtd.projectedFreq = ed[2]; 
 
      findLineWith(IN, "range", line); 
      ed = IRutil::extractDoubles(line); 
      perf.playedMoveMatchWtd.projected2SigmaLeft = ed[0]; 
      perf.playedMoveMatchWtd.projected2SigmaRight = ed[1]; 
      perf.playedMoveMatchWtd.projected2SigmaLeftFreq = ed[2]; 
      perf.playedMoveMatchWtd.projected2SigmaRightFreq = ed[3]; 
 
 if (perf.weighted) { 
      findLineWith(IN, "Unwtd.", line); 
      ed = IRutil::extractDoubles(line); 
      perf.moveMatch.projectedValue = ed[0]; 
      perf.moveMatch.projectedSigma = ed[1]; 
      perf.moveMatch.projectedFreq = ed[2]; 
 
      findLineWith(IN, "range", line); 
      ed = IRutil::extractDoubles(line); 
      perf.moveMatch.projected2SigmaLeft = ed[0]; 
      perf.moveMatch.projected2SigmaRight = ed[1]; 
      perf.moveMatch.projected2SigmaLeftFreq = ed[2]; 
      perf.moveMatch.projected2SigmaRightFreq = ed[3]; 
 
      findLineWith(IN, "top-move", line); 
      ed = IRutil::extractDoubles(line); 
      perf.equalValueMatch.projectedValue = ed[0]; 
      perf.equalValueMatch.projectedSigma = ed[1]; 
      perf.equalValueMatch.projectedFreq = ed[2]; 
 
      findLineWith(IN, "range", line); 
      ed = IRutil::extractDoubles(line); 
      perf.equalValueMatch.projected2SigmaLeft = ed[0]; 
      perf.equalValueMatch.projected2SigmaRight = ed[1]; 
      perf.equalValueMatch.projected2SigmaLeftFreq = ed[2]; 
      perf.equalValueMatch.projected2SigmaRightFreq = ed[3]; 
 
      findLineWith(IN, "played-move", line); 
      ed = IRutil::extractDoubles(line); 
      perf.playedMoveMatch.projectedValue = ed[0];  //predPlayedMatchesWtd = ed[0];
      perf.playedMoveMatch.projectedSigma = ed[1];
      perf.playedMoveMatch.projectedFreq = ed[2];

      findLineWith(IN, "range", line); 
      ed = IRutil::extractDoubles(line); 
      perf.playedMoveMatch.projected2SigmaLeft = ed[0];
      perf.playedMoveMatch.projected2SigmaRight = ed[1];
      perf.playedMoveMatch.projected2SigmaLeftFreq = ed[2];
      perf.playedMoveMatch.projected2SigmaRightFreq = ed[3];
 } 
      findLineWith(IN, "first-line", line); 
      ed = IRutil::extractDoubles(line); 
      perf.moveMatch.measuredValue = int(ed[0]); 
      perf.moveMatch.measuredFreq = ed[1]; 
      perf.moveMatchWtd.measuredValue = ed[2]; 
      perf.moveMatchWtd.measuredFreq = ed[3]; 
 
      findLineWith(IN, "equal-top", line); 
      ed = IRutil::extractDoubles(line); 
      perf.equalValueMatch.measuredValue = int(ed[0]); 
      perf.equalValueMatch.measuredFreq = ed[1]; 
      perf.equalValueMatchWtd.measuredValue = ed[2]; 
      perf.equalValueMatchWtd.measuredFreq = ed[3]; 
 
      findLineWith(IN, "falloff", line); 
      getline(IN, line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloffWtd.projectedValue = ed[0]; 
      perf.scaledFalloffWtd.projected2SigmaLeft = ed[1]; 
      perf.scaledFalloffWtd.projected2SigmaRight = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloffWtd.measuredValue = ed[0]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloffWtd.projectedValue = ed[0]; 
      perf.unscaledFalloffWtd.projected2SigmaLeft = ed[1]; 
      perf.unscaledFalloffWtd.projected2SigmaRight = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloffWtd.measuredValue = ed[0]; 
 
      findLineWith(IN, "figures", line); 
      getline(IN, line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloffWtd.projectedFreq = ed[0]; 
      perf.scaledFalloffWtd.projected2SigmaLeftFreq = ed[1]; 
      perf.scaledFalloffWtd.projected2SigmaRightFreq = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloffWtd.measuredFreq = ed[0]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloffWtd.projectedFreq = ed[0]; 
      perf.unscaledFalloffWtd.projected2SigmaLeftFreq = ed[1]; 
      perf.unscaledFalloffWtd.projected2SigmaRightFreq = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloffWtd.measuredFreq = ed[0]; 
 
 
      findLineWith(IN, "Unweighted", line); 
      getline(IN, line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloff.projectedValue = ed[0]; 
      perf.scaledFalloff.projected2SigmaLeft = ed[1]; 
      perf.scaledFalloff.projected2SigmaRight = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloff.measuredValue = ed[0]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloff.projectedValue = ed[0]; 
      perf.unscaledFalloff.projected2SigmaLeft = ed[1]; 
      perf.unscaledFalloff.projected2SigmaRight = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloff.measuredValue = ed[0]; 
 
      findLineWith(IN, "figures", line); 
      getline(IN, line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloff.projectedFreq = ed[0]; 
      perf.scaledFalloff.projected2SigmaLeftFreq = ed[1]; 
      perf.scaledFalloff.projected2SigmaRightFreq = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.scaledFalloff.measuredFreq = ed[0]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloff.projectedFreq = ed[0]; 
      perf.unscaledFalloff.projected2SigmaLeftFreq = ed[1]; 
      perf.unscaledFalloff.projected2SigmaRightFreq = ed[2]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloff.measuredFreq = ed[0]; 
 
 
 
       
      findLineWith(IN, "capped", line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloff.measuredValue = ed[0]; 
      perf.unscaledFalloff.measuredFreq = ed[1]; 
      getline(IN,line); 
      ed = IRutil::extractDoubles(line); 
      perf.unscaledFalloffNextTurnCapped.measuredValue = ed[0]; 
      perf.unscaledFalloffNextTurnCapped.measuredFreq = ed[1]; 
 
      return IN; 
   } 
 
 
   /** Initial dialog that can be shared by several demos.  
    */ 
   struct DemoChoices { 
      string groundFilters;  //filters that stay loaded throughout demo run 
      bool   fitThenGo;      //do best-fit on current spec before running demo 
      string matchStat;      //choice of first-line or equal-top matches  
      //double newCap;         //quick-change cap on delta values, max. 10.00 
      bool wtd; 
      bool cancel; 
 
      DemoChoices(const string& gf, bool ftg, const string& ms, bool gwtd = false, 
                  bool ca = false) 
//double nc = IRutil::DELTACAP, 
         : groundFilters(gf), fitThenGo(ftg), matchStat(ms), // newCap(nc), 
           wtd(gwtd), cancel(ca) { } 
   }; 
 
   DemoChoices demoOptions (Ensemble& ens) { 
 
      enum MAIN_DEMO_OPTIONS { 
         MATCH_STAT, 
         WEIGHTED, 
         CLEAR_FILTERS_FIRST, 
         CAP_EVAL, 
         EVAL_CAP, 
         OR_PREV_EVAL, 
//       CHANGE_DELTA_CAP,   //Allow to be part of Scales menu only?  Neither? 
         EXCLUDE_CLEAR_MOVES, 
         CLEAR_MOVE_CAP, 
         FIT_THEN_GO, 
         GO_DEMO, 
         NUM_MAIN_DEMO_OPTIONS 
      }; 
 
      EnumMenu* optionsMenu = new EnumMenu 
            (&ens, NUM_MAIN_DEMO_OPTIONS, 1, false , true,  
             "\nEnter " + IRutil::itoa(1+GO_DEMO) + "/go" 
                  + " to run, " + IRutil::itoa(1+FIT_THEN_GO) + "/fitThenGo" 
                  + " to fit and run, or select to change settings.\n"  
                  + IRutil::DASHES,  
             IRutil::DASHES, true, false, false);  
                               //cancelable, no other options 
      optionsMenu->addEntry(MATCH_STAT, "firstLines", 
         "Match first-line or equal-top moves? ", "first", true, false, false); 
      optionsMenu->addEntry(WEIGHTED, "weighted", 
         "Use weighted stats? ", "no", true, false, false); 
      optionsMenu->addEntry(CLEAR_FILTERS_FIRST, "clearFiltersFirst", 
         "Clear all other filters first?", "no", true, false, false); 
      optionsMenu->addEntry(CAP_EVAL, "capEval", "Skip extreme-eval turns? ", 
         (ens.getFocusTrial()->scalesDeltas() ? "no" : "yes"), 
         true, false, false); 
      optionsMenu->addEntry(EVAL_CAP, "evalCap", "If yes, skip when |eval| >", 
         "3.00", true, false, false); 
      optionsMenu->addEntry(OR_PREV_EVAL, "orPrevEval", 
         "...but include if prev turn OK? ", "yes", true, false, false); 
//    optionsMenu->addEntry(CHANGE_DELTA_CAP, "changeDeltaCap",  
//       "Cap on `Blunder Delta' =", 
//       IRutil::ftoa(ens.getFocusTrial()->getDeltaCap(),2),  
//       true, false, false); 
      optionsMenu->addEntry(EXCLUDE_CLEAR_MOVES, "excludeClearMoves", 
         "Exclude clear-choice moves? ", "no", true, false, false); 
      optionsMenu->addEntry(CLEAR_MOVE_CAP, "clearMoveCap", 
         "If yes, cap on second delta = ", "3.00", true, false, false); 
      optionsMenu->addEntry(FIT_THEN_GO, "fitThenGo", "Run Fit, Then Go.", 
         "", true, false, true); 
      optionsMenu->addEntry(GO_DEMO, "go", "Run With These Settings!", 
         "", true, false, false); 
 
      DemoChoices dc("", false, "first"); 
      bool entering = true; 
      string ans; 
      double doubleItem; 
 
      cout << endl << "Spec " << string(ens.getFocusTrial()->getSpec()) 
           << endl << IRutil::DASHES; 
 
      while (entering) { 
         int choice = optionsMenu->readChoice(); 
         if (optionsMenu->success(choice)) { 
            switch(choice) { 
             case MATCH_STAT: 
               cout << "Match first-line moves only? (y/n) (n -> equal-top) "; 
               ens.loggedRead(ans); 
               optionsMenu->setStatus(choice,  
                                      IRutil::isYes(ans) ? "first" : "top"); 
               break; 
             case WEIGHTED: 
               cout << "Use weighted stats? (y/n) "; 
               ens.loggedRead(ans); 
               optionsMenu->setStatus(choice, 
                                      IRutil::isYes(ans) ? "yes" : "no"); 
               break; 
             case CLEAR_FILTERS_FIRST: 
               cout << "Call clearFilters before attaching these? (y/n) "; 
               ens.loggedRead(ans); 
               optionsMenu->setStatus(choice,  
                                      IRutil::isYes(ans) ? "yes" : "no"); 
               break; 
             case CAP_EVAL: 
               cout << "Skip turns with extreme evals? (y/n) " << endl; 
               ens.loggedRead(ans); 
               optionsMenu->setStatus(choice,  
                                      IRutil::isYes(ans) ? "yes" : "no"); 
               break; 
             case EVAL_CAP: 
               cout << "Enter nonnegative bound for absolute eval value: "; 
               ens.loggedRead(doubleItem); 
               optionsMenu->setStatus(choice, IRutil::ftoa(doubleItem,2)); 
               break; 
             case OR_PREV_EVAL: 
               cout << "Include turns whose previous turn had absolute eval " 
                 << "within " << optionsMenu->getStatus(EVAL_CAP) << "? (y/n) "; 
               ens.loggedRead(ans); 
               optionsMenu->setStatus(choice,  
                                      IRutil::isYes(ans) ? "yes" : "no"); 
               break; 
//           case CHANGE_DELTA_CAP: 
//             cout << "Change delta cap for blunders? (minimum "  
//                  << IRutil::DELTACAP << ") (y/n): "; 
//             ens.loggedRead(doubleItem); 
//             optionsMenu->setStatus(choice, IRutil::ftoa(doubleItem,2)); 
//             break; 
             case EXCLUDE_CLEAR_MOVES: 
               cout << "Exclude moves with a clear choice? (y/n) "; 
               ens.loggedRead(ans); 
               optionsMenu->setStatus(choice,  
                                      IRutil::isYes(ans) ? "yes" : "no"); 
               break; 
             case CLEAR_MOVE_CAP: 
               cout << "Enter cap on delta of next best move: "; 
               ens.loggedRead(doubleItem); 
               optionsMenu->setStatus(choice, IRutil::ftoa(doubleItem,2)); 
               break; 
             case FIT_THEN_GO: 
               dc.fitThenGo = true; 
               entering = false; 
               break; 
             case GO_DEMO: 
             default: 
               entering = false; 
               break; 
            } 
         } else if (choice == EnumMenu::CANCEL_CHOICE) { 
            dc.cancel = true;  //INV: other options valid, so no crash occurs. 
            return dc; 
         } else { 
            cout << "Invalid menu entry, please try again." << endl; 
         } 
      } //control past here means successfully done entering. 
 
      ens.loggedWrite("",true); 
      dc.matchStat = optionsMenu->getStatus(MATCH_STAT); 
      //dc.newCap = atof(optionsMenu->getStatus(DELTA_CAP).c_str()); 
      dc.wtd = IRutil::isYes(optionsMenu->getStatus(WEIGHTED)); 
 
      if (IRutil::isYes(optionsMenu->getStatus(CLEAR_FILTERS_FIRST))) { 
         dc.groundFilters += " clearFilters "; 
      } 
 
      if (IRutil::isYes(optionsMenu->getStatus(CAP_EVAL))) { 
         string cap = optionsMenu->getStatus(EVAL_CAP); 
         dc.groundFilters += " newFilters EvalWithin ew " + cap + " 0 n done "; 
         if (IRutil::isYes(optionsMenu->getStatus(OR_PREV_EVAL))) { 
            dc.groundFilters += " newFilters PrevEvalWithin pw " + cap 
               + " 0 n OrFilter porew ew y pw n done " 
               + " detach ew y pw n "; 
         } 
      } 
 
      if (IRutil::isYes(optionsMenu->getStatus(EXCLUDE_CLEAR_MOVES))) { 
         string cap2 = optionsMenu->getStatus(CLEAR_MOVE_CAP); 
         dc.groundFilters += " newFilters SecondDelta sd leq " + cap2  
                           + " done "; 
      } 
 
      return dc; 
   } 
 
//-------------------------------Elista Demo--------------------------------- 
 
   void elistaDemo(Ensemble& ens) { 
 
      string fileName;   //*vital* to keep this updated! 
      ofstream* filep = agreeDemoFile(ens, fileName);  //fileName mutable 
      string timestamp = IRutil::getRawTime();  //already trimmed 
      string cal = IRutil::getTime(); 
      (*filep) << IRutil::IRCOMMENT << "Elista Test done on " << cal 
               << " (" << timestamp << ")" << endl << endl; 
      filep->close();  //now we give writing control over to "ens" 
      ens.addOutputFile(fileName, false);  //false -> no extra comment 
 
      const DemoChoices dc = demoOptions(ens);  
      if (dc.cancel) { 
         cout << "Canceled---returning." << endl; 
         return; 
      } 
      //else 
 
      double origCap = ens.getFocusTrial()->getDeltaCap(); 
      double newCap = origCap;  //dc.newCap; 
      //ens.getFocusTrial()->setDeltaCap(newCap); 
 
      string SLMpart = " newFilters EventIs SanLuis05 Luis done "; 
      SLMpart += " newFilters EventIs Mexico07 Mexico done "; 
      SLMpart += " newFilters OrFilter SL05+Mex07 SanLuis05 y Mexico07 n done "; 
      SLMpart += " detach Mexico07 y SanLuis05 y SL05+Mex07 n "; 
      SLMpart += " attach SL05+Mex07 n "; 
 
      istringstream* isp = new istringstream(dc.groundFilters + SLMpart); 
      ens.interactFromStream(isp, ""); 
      //leaves control back at main menu and restores input from cin 
 
      if (dc.fitThenGo) { 
         ens.runFit(); 
      } 
 
      string slmPart = " perfTest y n detach SL05+Mex07 n ";  
 
      string elistaFilters = string(" ") 
       + "newFilters PlayerToMove Kr2m Kramnik PlayerToMove To2m Topalov done " 
       + "newFilters DateIs TKg1 eq 2006.09.23 DateIs TKg2 eq 2006.09.24 " 
       + " DateIs TKg3 eq 2006.09.26 DateIs TKg4 eq 2006.09.27 " 
       + " DateIs TKg6 eq 2006.10.02 done " 
       + "newFilters OrFilter TKg1-6 TKg1 y TKg2 y TKg3 y TKg4 y TKg6 n done " 
       + "newFilters MoveNo from11 geq 11 MoveNo from13 geq 13  " 
       + "MoveNo from14 geq 14 MoveNo from15 geq 15 MoveNo from18 geq 18 done " 
       + "newFilters AndFilter Kramnikg1 Kr2m y TKg1 y from13 n done " 
       + "newFilters AndFilter Kramnikg2 Kr2m y TKg2 y from18 n done " 
       + "newFilters AndFilter Kramnikg3 Kr2m y TKg3 y from11 n done " 
       + "newFilters AndFilter Kramnikg4 Kr2m y TKg4 y from15 n done " 
       + "newFilters AndFilter Kramnikg6 Kr2m y TKg6 y from14 n done " 
       + "newFilters OrFilter Kramnikg1-6 Kramnikg1 y Kramnikg2 y Kramnikg3 y " 
       + "Kramnikg4 y Kramnikg6 n done " 
       + "newFilters AndFilter Topalovg1 To2m y TKg1 y from13 n done " 
       + "newFilters AndFilter Topalovg2 To2m y TKg2 y from18 n done " 
       + "newFilters AndFilter Topalovg3 To2m y TKg3 y from11 n done " 
       + "newFilters AndFilter Topalovg4 To2m y TKg4 y from15 n done " 
       + "newFilters AndFilter Topalovg6 To2m y TKg6 y from14 n done " 
       + "newFilters OrFilter Topalovg1-6 Topalovg1 y Topalovg2 y Topalovg3 y " 
       + "Topalovg4 y Topalovg6 n done " 
       + "detach Kr2m y To2m y TKg1 y TKg2 y TKg3 y TKg4 y TKg6 y TKg1-6 n " 
       + "detach from11 y from13 y from14 y from15 y from18 n " 
       + "detach Kramnikg1 y Kramnikg2 y Kramnikg3 y Kramnikg4 y Kramnikg6 n " 
       + "detach Topalovg1 y Topalovg2 y Topalovg3 y Topalovg4 y Topalovg6 n " 
       + "detach Kramnikg1-6 y Topalovg1-6 n "; 
 
      string doElista = ""; 
      doElista += " attach Kramnikg1 n perfTest y n detach Kramnikg1 n "; 
      doElista += " attach Kramnikg2 n perfTest y n detach Kramnikg2 n "; 
      doElista += " attach Kramnikg3 n perfTest y n detach Kramnikg3 n "; 
      doElista += " attach Kramnikg4 n perfTest y n detach Kramnikg4 n "; 
      doElista += " attach Kramnikg6 n perfTest y n detach Kramnikg6 n "; 
      doElista += " attach Kramnikg1-6 n perfTest y n detach Kramnikg1-6 n "; 
      doElista += " attach Topalovg1 n perfTest y n detach Topalovg1 n "; 
      doElista += " attach Topalovg2 n perfTest y n detach Topalovg2 n "; 
      doElista += " attach Topalovg3 n perfTest y n detach Topalovg3 n "; 
      doElista += " attach Topalovg4 n perfTest y n detach Topalovg4 n "; 
      doElista += " attach Topalovg6 n perfTest y n detach Topalovg6 n "; 
      doElista += " attach Topalovg1-6 n perfTest y n detach Topalovg1-6 n "; 
 
      string cleanup = ""; 
      cleanup += " hideFilters Kr2m y To2m y TKg1 y TKg2 y TKg3 y TKg4 y TKg6 "; 
      cleanup += " y TKg1-6 y from11 y from13 y from14 y from15 y from18 "; 
      cleanup += " y Kramnikg1 y Kramnikg2 y Kramnikg3 y Kramnikg4 "; 
      cleanup += " y Kramnikg6 y Kramnikg1-6 y Topalovg1 y Topalovg2 "; 
      cleanup += " y Topalovg3 y Topalovg4 y Topalovg6 y Topalovg1-6 n "; 
      //we leave the San Luis and Mexico filters un-hidden. 
 
      isp = new istringstream(slmPart + elistaFilters + doElista + cleanup); 
 
      ens.interactFromStream(isp, ""); 
 
      ens.closeOutputFile(fileName);  //no more output, removed from "outs" 
 
      ifstream resultsFile(fileName.c_str()); 
 
      PerfData slmPerf(newCap);  
      PerfData kg1(newCap), kg2(newCap), kg3(newCap), kg4(newCap), 
               kg6(newCap), kg1to6(newCap); 
      PerfData tg1(newCap), tg2(newCap), tg3(newCap), tg4(newCap), 
               tg6(newCap), tg1to6(newCap); 
 
      string timeLine;  //used only as placeholder 
      findLineWith(resultsFile,timestamp,timeLine); 
 
      parsePerfData(resultsFile, "SL05+Mex07", slmPerf); 
      parsePerfData(resultsFile, "Kramnikg1", kg1); 
      parsePerfData(resultsFile, "Kramnikg2", kg2); 
      parsePerfData(resultsFile, "Kramnikg3", kg3); 
      parsePerfData(resultsFile, "Kramnikg4", kg4); 
      parsePerfData(resultsFile, "Kramnikg6", kg6); 
      parsePerfData(resultsFile, "Kramnikg1-6", kg1to6); 
      parsePerfData(resultsFile, "Topalovg1", tg1); 
      parsePerfData(resultsFile, "Topalovg2", tg2); 
      parsePerfData(resultsFile, "Topalovg3", tg3); 
      parsePerfData(resultsFile, "Topalovg4", tg4); 
      parsePerfData(resultsFile, "Topalovg6", tg6); 
      parsePerfData(resultsFile, "Topalovg1-6", tg1to6); 
 
      string source = dc.fitThenGo ? "San Luis '05 and Mexico '07"  
                                   : "Your source for the trial-spec"; 
      string weightedAdj = slmPerf.weighted ? "weighted" : "unweighted"; 
      string weightedAdjCap = slmPerf.weighted ? "Weighted" : "Unweighted"; 
 
      cout << endl; 
      cout << "Highlights of the results: " << source << " set the standard " 
           << "used" << endl << "to predict Elista for " << weightedAdj << " " 
           << dc.matchStat << "-move matches, to TogaII 1.2.1a depth 15.\n\n"; 
 
      cout << (slmPerf.matchStats("SL+Mex", dc.matchStat == "first", 7)); 
      cout << endl << endl; 
 
      cout << weightedAdjCap << " " << dc.matchStat << "-move matches for " 
           << "Kramnik's games 1-6, over the moves" << endl 
           << "listed by Danailov " 
           << "(filtered further by options you chose)" << endl; 
      cout << endl; 
 
      cout << kg1.matchStats("Game 1", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << kg2.matchStats("Game 2", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << kg3.matchStats("Game 3", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << kg4.matchStats("Game 4", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << kg6.matchStats("Game 6", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << kg1to6.matchStats("Gms 1-6", dc.matchStat == "first", 6, dc.wtd) << endl; 
 
      string tw = (dc.matchStat == "top") ? "in Game 1 and " : ""; 
      cout << endl << "Thus we see Kramnik matched more " << tw << "especially" 
         << " in Game 2," << endl << "but he was PREDICTED to match more, " 
         << "BECAUSE Topalov faced him with such " << endl 
         << "forcing situations that *anyone* at San Luis + Mexico would do " 
         << "the same!" << endl 
         << "(Stats agree with KWR's posted Fritz results, below " 
         << "Danailov's bare claims.)" << endl; 
      cout << endl << "Hit return to show Topalov's stats."; 
 
      if (ens.usingCin()) { 
         IRutil::finishLine(cin); 
      } 
       
      cout << tg1.matchStats("Game 1", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << tg2.matchStats("Game 2", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << tg3.matchStats("Game 3", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << tg4.matchStats("Game 4", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << tg6.matchStats("Game 6", dc.matchStat == "first", 6, dc.wtd) << endl; 
      cout << tg1to6.matchStats("Gms 1-6", dc.matchStat == "first", 6, dc.wtd) << endl; 
 
      bool sc = ens.getFocusTrial()->scalesDeltas(); 
 
      cout << endl << "Falloff comparisons, expected and actual: " << endl; 
      cout << slmPerf.falloffStats("San Luis '05 and Mexico '07", sc, 8, dc.wtd) 
           << endl; 
      cout << kg1to6.falloffStats("Kramnik, games 1--6", sc, 8, dc.wtd) << endl; 
      cout << tg1to6.falloffStats("Topalov, games 1--6", sc, 8, dc.wtd) << endl; 
 
      cout << endl; 
      cout << "Thus we see Topalov was simply outplayed in games 1-6, " 
           << "with nothing else amiss." << endl; 
      cout << "Hit return to continue exploring (with option to quit)." << endl; 
 
      if (ens.usingCin()) {  
         IRutil::finishLine(cin);  
      } 
 
      //ens.getFocusTrial()->setDeltaCap(origCap); 
      resultsFile.close(); 
   } 
 
 
//--------------------San Luis '05 and Mexico '07 Demo----------------------- 
 
   void sanLuisMexicoDemo(Ensemble& ens) { 
 
      string fileName;   //*vital* to keep this updated! 
      ofstream* filep = agreeDemoFile(ens, fileName);  //fileName mutable 
      string timestamp = IRutil::getRawTime();  //already trimmed 
      string cal = IRutil::getTime(); 
      (*filep) << IRutil::IRCOMMENT  
               << "San Luis '05 + Mexico '07 Demo done on " << cal 
               << " (" << timestamp << ")" << endl << endl; 
      filep->close();  //now we give writing control over to "ens" 
      ens.addOutputFile(fileName, false);  //false -> no extra comment 
 
      cout << endl << "Do fitThenGo to fit San Luis + Mexico with Topalov's " 
           << "San Luis moves excluded." << endl; 
 
      const DemoChoices dc = demoOptions(ens); 
      if (dc.cancel) { 
         cout << "Canceled---returning." << endl; 
         return; 
      } 
      //else 
 
      double origCap = ens.getFocusTrial()->getDeltaCap(); 
      double newCap = origCap;  //dc.newCap; 
      //ens.getFocusTrial()->setDeltaCap(newCap); 
 
      string buildFilters = dc.groundFilters 
         + " newFilters PlayerToMove To2m Topalov PlayerToMove An2m Anand " 
         + " EventIs SanLuis05 Luis EventIs Mexico07 Mexico " 
         + " OrFilter SL05+Mex07 SanLuis05 y Mexico07 n " 
         + " AndFilter To2mSL To2m y SanLuis05 n " 
         + " AndFilter An2mMex An2m y Mexico07 n " 
         + " NotFilter NotTo2mSL To2mSL " 
         + " AndFilter fitBezTo2mSL SL05+Mex07 y NotTo2mSL n " 
         + " OnMoveFacing TopOpp2m Topalov " 
         + " OnMoveFacing AnOpp2m Anand " 
         + " AndFilter TopOpp2mSL TopOpp2m y SanLuis05 n " 
         + " AndFilter AnOpp2mMex AnOpp2m y Mexico07 n done "; 
      string leaveActiveList = " SanLuis05 y Mexico07 y SL05+Mex07 y "; 
      string hideList = string(" To2m y An2m y To2mSL y An2mMex y NotTo2mSL ") 
         + " y fitBezTo2mSL y TopOpp2m y AnOpp2m y TopOpp2mSL y AnOpp2mMex n "; 
      buildFilters += " detach " + leaveActiveList + hideList; 
       
      //now attach filter for-fit *in case* user desired it: 
      buildFilters += " attach fitBezTo2mSL n "; 
 
      istringstream* isp = new istringstream(buildFilters); 
      ens.interactFromStream(isp, ""); 
      // 
      //Leaves control back at main menu with only ground-filters and 
      //"fitBezTo2mSL" attached, and restores input from cin. 
      //This enables user to change other fit settings interactively.  If user 
      //didn't choose to fit, we still do performance test with it attached. 
 
      if (dc.fitThenGo) { 
         ens.runFit(); 
      } 
 
      // Performance tests in order:  
      // (1) SL+Mex without Topalov San Luis, (2) SL+Mex overall, 
      // (3) Topalov San Luis, (4) Topalov's opponents at San Luis, 
      // (5) Anand Mexico, (6) Anand's opponents in Mexico. 
      // For transparency we script user screen action and cull data from the 
      // actual output, rather than call routines directly as with  
      // "ens.runFit" above.  The downside is that (currently) we don't get 
      // named PerfData objects from the performance tests themselves, 
      // but only from the messy after-the-fact parsing of output. 
 
      string perfPart = string(" perfTest y n detach fitBezTo2mSL n ") 
         + " attach SL05+Mex07 n perfTest y n detach SL05+Mex07 n " 
         + " attach To2mSL n perfTest y n detach To2mSL n " 
         + " attach TopOpp2mSL n perfTest y n detach TopOpp2mSL n " 
         + " attach An2mMex n perfTest y n detach An2mMex n " 
         + " attach AnOpp2mMex n perfTest y n detach AnOpp2mMex n "; 
      string cleanup = " hideFilters " + hideList; 
 
      isp = new istringstream(perfPart + cleanup); 
      ens.interactFromStream(isp, ""); 
 
      ens.closeOutputFile(fileName);  //no more output, removed from "outs" 
 
      ifstream resultsFile(fileName.c_str()); 
 
      string timeLine; //used only as placeholder 
      findLineWith(resultsFile,timestamp,timeLine); 
 
      PerfData slmPerfBez(newCap), slmPerf(newCap), topSLPerf(newCap); 
      PerfData topOppSLPerf(newCap), anMexPerf(newCap), anOppMexPerf(newCap); 
 
      parsePerfData(resultsFile, "fitBezTo2mSL", slmPerfBez); 
      parsePerfData(resultsFile, "SL05+Mex07", slmPerf); 
      parsePerfData(resultsFile, "To2mSL", topSLPerf); 
      parsePerfData(resultsFile, "TopOpp2mSL", topOppSLPerf); 
      parsePerfData(resultsFile, "An2mMex", anMexPerf); 
      parsePerfData(resultsFile, "AnOpp2mMex", anOppMexPerf); 
 
      bool sc = ens.getFocusTrial()->scalesDeltas(); 
      string source = dc.fitThenGo ?  
           "San Luis '05 plus Mexico '07 minus Topalov at San Luis" 
         : "Your source for the trial-spec"; 
 
      string matchKind = (dc.matchStat=="first") ? "first-line" : "equal-top"; 
      string weightedAdj = slmPerf.weighted ? "weighted" : "unweighted"; 
      string weightedAdjCap = slmPerf.weighted ? "Weighted" : "Unweighted"; 
 
      cout << endl; 
      cout << "Highlights of results: " << source << endl 
           << "set the standard used to assess performance at San Luis '05 " 
           << "and Mexico '07." << endl 
           << "First we evaluate Topalov at San Luis:" << endl << endl; 
 
      cout << "Overall " + matchKind + " move-matches:" << endl; 
      cout << slmPerf.matchStats("SL+Mex", dc.matchStat == "first", 6, dc.wtd) << endl 
           << slmPerfBez.matchStats("- Top@SL", dc.matchStat == "first", 6, dc.wtd); 
      cout << endl << endl; 
      cout << "Topalov's and Opponents' move-matches at San Luis '05:" << endl; 
      cout << topSLPerf.matchStats("Topalov", dc.matchStat == "first", 6, dc.wtd) 
           << endl 
           << topOppSLPerf.matchStats("Top Opp", dc.matchStat == "first", 6, dc.wtd) 
           << endl << endl; 
 
      cout << "Falloff comparisons, expected and actual: " << endl; 
      cout << slmPerf.falloffStats("San Luis '05 and Mexico '07", sc, 8, dc.wtd) 
           << endl; 
      cout << slmPerfBez.falloffStats("Minus Topalov at San Luis", sc, 8, dc.wtd) 
           << endl; 
      cout << topSLPerf.falloffStats("Topalov at San Luis", sc, 8, dc.wtd)  
           << "  ** !" << endl; 
      cout << topOppSLPerf.falloffStats("Topalov's Opponents at San Luis",sc,8, dc.wtd) 
           << endl; 
 
      cout << endl; 
      cout << "Thus we see Topalov's opponents played pretty much up to par, "  
           << endl 
           << "while Topalov himself played superbly.  This is exactly what " 
           << "one might expect" << endl 
           << "if Topalov were receiving assistance during the games, even " 
           << "only on key moves." << endl 
           << "But before you reach for the phone to have Interpol make an " 
           << "arrest in Bulgaria," << endl 
           << "hit return to see Anand's performance at Mexico '07:" << endl; 
 
      if (ens.usingCin()) { 
         IRutil::finishLine(cin); 
      } 
 
      cout << "Anand's and Opponents' move-matches at Mexico '07:" << endl; 
      cout << anMexPerf.matchStats("Anand", dc.matchStat == "first", 6, dc.wtd) 
           << endl 
           << anOppMexPerf.matchStats("AnandOpp", dc.matchStat == "first", 6, dc.wtd) 
           << endl << endl; 
 
      cout << "Falloff comparisons, expected and actual: " << endl; 
      cout << slmPerf.falloffStats("San Luis '05 and Mexico '07", sc, 8, dc.wtd) 
           << endl; 
      cout << slmPerfBez.falloffStats("Minus Topalov at San Luis", sc, 8, dc.wtd) 
           << endl; 
      cout << anMexPerf.falloffStats("Anand at Mexico '07", sc, 8, dc.wtd)  
           << "  ** !!" << endl; 
      cout << anOppMexPerf.falloffStats("Anand's Opponents at Mexico '07",sc,8, dc.wtd) 
           << endl; 
 
      cout << endl; 
      cout << "Do you see any real difference from Topalov's stats at San Luis?" 
           << endl 
           << "Perhaps in move-matching, but (a) Topalov was predicted " 
           << "to match more" << endl 
           << "(owing to the more-forcing nature of his games, we infer), " 
           << "and (b) Topalov" << endl  
           << "is well within the 2-sigma range.  We see *falloff* rather " 
           << "than move-matches as" << endl 
           << "most important and engine-neutral---note that Anand had " 
	   << "nearly the same ratio!" << endl; 
      cout << "Since Anand's win is uncontroversial, maybe they share a " 
           << "general conclusion:" << endl 
           << endl 
           << "      WINNERS OF TOP EVENTS TEND TO HAVE PLAYED REALLY WELL." 
           << endl << endl; 
      cout << "Hit return to continue exploring (with option to quit)." 
           << endl; 
 
      if (ens.usingCin()) { 
          IRutil::finishLine(cin);  
      } 
 
      //ens.getFocusTrial()->setDeltaCap(origCap); 
      resultsFile.close(); 
   } 
 
 
//------------------------------Swing Demo----------------------------------- 
 
   void swingDemo(Ensemble& ens) { 
 
      string fileName;   //*vital* to keep this updated! 
      ofstream* filep = agreeDemoFile(ens, fileName);  //fileName mutable 
      string timestamp = IRutil::getRawTime();  //already trimmed 
      string cal = IRutil::getTime(); 
      (*filep) << IRutil::IRCOMMENT << " Swing Demo run on " << cal 
               << " (" << timestamp << ")" << endl << endl; 
      filep->close();   //turn control over to "ens" now. 
      ens.addOutputFile(fileName,false);  //false -> no extra comment 
 
      cout << "Include turns with more than 2 or 3 equal-top moves? (y/n) "; 
      string ans; 
      ens.loggedRead(ans); 
 
      if (ens.usingCin()) { 
         IRutil::finishLine(cin);   //finishes line that answered last question. 
      } 
 
      string st = ""; 
      st += " newFilters SecondDelta sdeq0 eq 0 DeltaIs tdeq0 2 eq 0 "; 
      st += " DeltaIs fdeq0 3 eq 0 DeltaIs vdeq0 4 eq 0 FirstLineMatch flm "; 
      st += " PlayedMoveIndex pmieq1 eq 1 PlayedMoveIndex pmieq2 eq 2 "; 
      st += " PlayedMoveIndex pmieq3 eq 3 PlayedMoveIndex pmileq1 leq 1 "; 
      st += " PlayedMoveIndex pmileq2 leq 2 PlayedMoveIndex pmileq3 leq 3 "; 
      st += " PlayedMoveIndex pmieq4 eq 4 PlayedMoveIndex pmileq4 leq 4 "; 
      st += " DeltaIs sdeq01 1 eq 0.01 done "; 
      st += " detach sdeq0 y tdeq0 y fdeq0 y vdeq0 y flm y pmieq1 y pmieq2 y pmieq3 "; 
      st += " y pmieq4 y pmileq1 y pmileq2 y pmileq3 y pmileq4 y sdeq01 n "; 
 
      if (!IRutil::isYes(ans)) { 
         st += " newFilters DeltaIs tdgt0 2 geq 0.005 DeltaIs fdgt0 3 geq "; 
         st += "0.005 DeltaIs vdgt0 4 geq 0.005 DeltaIs xdgt0 5 geq 0.005 done"; 
         st += " detach tdgt0 y fdgt0 y vdgt0 y xdgt0 n "; 
      } 
 
      string td = IRutil::isYes(ans) ? "" : " y tdgt0 "; 
      string fd = IRutil::isYes(ans) ? "" : " y fdgt0 "; 
      string vd = IRutil::isYes(ans) ? "" : " y vdgt0 "; 
      string xd = IRutil::isYes(ans) ? "" : " y xdgt0 "; 
 
      st += " attach pmileq1 y sdeq0 " + td + " n showTrial n "; 
      st += " attach flm n showTrial n "; 
      st += " detach pmileq1 y sdeq0 " + td + " y flm n "; 
 
      st += " attach pmileq2 y tdeq0 " + fd + " n showTrial n "; 
      st += " attach flm n showTrial n detach flm n "; 
      st += " attach pmieq1 n showTrial n detach pmieq1 n "; 
      st += " attach pmieq2 n showTrial n detach pmieq2 n "; 
      st += " detach pmileq2 y tdeq0 " + fd + " n "; 
 
      st += " attach pmileq3 y fdeq0 " + vd + " n showTrial n "; 
      st += " attach flm n showTrial n detach flm n "; 
      st += " attach pmieq1 n showTrial n detach pmieq1 n "; 
      st += " attach pmieq2 n showTrial n detach pmieq2 n "; 
      st += " attach pmieq3 n showTrial n detach pmieq3 n "; 
      st += " detach pmileq3 y fdeq0 " + vd + " n "; 
 
      st += " attach pmileq4 y vdeq0 " + xd + " n showTrial n "; 
      st += " attach flm n showTrial n detach flm n "; 
      st += " attach pmieq1 n showTrial n detach pmieq1 n "; 
      st += " attach pmieq2 n showTrial n detach pmieq2 n "; 
      st += " attach pmieq3 n showTrial n detach pmieq3 n "; 
      st += " attach pmieq4 n showTrial n detach pmieq4 n "; 
      st += " detach pmileq4 y vdeq0 " + xd + " n "; 
 
 
      st += " attach sdeq01 y pmileq1 n showTrial n "; 
      st += " attach flm n showTrial n detach flm y sdeq01 y pmileq1 n "; 
 
      string cleanup = ""; 
      cleanup += " hideFilters sdeq0 y tdeq0 y fdeq0 y flm y pmieq1 y pmieq2 "; 
      cleanup += " y pmieq3 y pmileq1 y pmileq2 y pmileq3 y sdeq01 n "; 
      if (!IRutil::isYes(ans)) { 
         cleanup += " hideFilters tdgt0 y fdgt0 y vdgt0 n "; 
      } 
 
      istringstream* isp = new istringstream(st + cleanup); 
      ens.interactFromStream(isp, ""); 
      ens.closeOutputFile(fileName);  //now file is back in our control 
 
      ifstream resultsFile(fileName.c_str()); 
      string line; 
      findLineWith(resultsFile,timestamp,line); 
 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int twoZeroTurns = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int twoZeroMatches = int(IRutil::extractDoubles(line)[1]); 
 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int threeZeroTurns = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int threeZeroMatches = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int threeZeroSeconds = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int threeZeroThirds = int(IRutil::extractDoubles(line)[1]); 
 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fourZeroTurns = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fourZeroMatches = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fourZeroSeconds = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fourZeroThirds = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fourZeroFourths = int(IRutil::extractDoubles(line)[1]); 
 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fiveZeroTurns = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fiveZeroMatches = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fiveZeroSeconds = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fiveZeroThirds = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fiveZeroFourths = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int fiveZeroFifths = int(IRutil::extractDoubles(line)[1]); 
 
 
 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int sdeq01Turns = int(IRutil::extractDoubles(line)[1]); 
      findLineWith(resultsFile,"trial",line); 
      line = line.substr(line.find_last_of(":")); 
      int sdeq01Matches = int(IRutil::extractDoubles(line)[1]); 
 
 
 
      double sigmaTwo = sqrt(0.5*0.5*twoZeroTurns); 
      double sigmaThree = sqrt(2.0*threeZeroTurns/9.0); 
      double sigmaFour = sqrt(3.0*fourZeroTurns/16.0); 
      double devTwo = (twoZeroMatches - twoZeroTurns/2.0)/sigmaTwo; 
      double devThree = (threeZeroMatches - threeZeroTurns/3.0)/sigmaThree; 
      double devFour = (fourZeroMatches - fourZeroTurns/4.0)/sigmaFour; 
 
      string qualif = IRutil::isYes(ans) ? "" : "exactly "; 
 
      cout << "SUMMARY" << endl << IRutil::DASHES << endl << endl; 
      cout << "Among turns with " << qualif << "two top-eval moves " 
           << "and one of them chosen: " << endl; 
      cout << "# of 2-zero turns: " << setw(5) << right << twoZeroTurns << endl 
           << "Expected #matches: " << setw(5) << int(twoZeroTurns/2.0 + 0.5) 
           << ",  two-sigma range " << setw(8) << setprecision(2) << fixed 
           << twoZeroTurns/2.0 - 2*sigmaTwo << "--" << left 
           << twoZeroTurns/2.0 + 2*sigmaTwo << endl 
           << "First move chosen: " << setw(5) << right << twoZeroMatches 
           << ", " << 100.0*twoZeroMatches/twoZeroTurns << "%" 
           << ",  deviation " << devTwo << " sigmas" << endl << endl; 
 
      if (devTwo > 2.0) { 
         cout << "Of two moves judged equal, the first is chosen " 
              << "significantly more often.  Why?" << endl 
              << "No H-R-S statistical mumbo-jumbo is involved here---we have " 
              << "just the engine" << endl 
              << "(Toga II 1.2.1a) and moves it \"randomly\" judges equal-top " 
              << "at ply-depth 15." << endl 
              << "Why should the first-listed of such an equal pair be played " 
              << "so much more often?" << endl << endl; 
         cout << "A *definite factor* is that the engine's ordering is *not* " 
              << "random." << endl 
              << "Except in (rare?) cases where both moves were equal from " 
              << "the lowest ply-depth," << endl 
              << "the first-listed move was judged *better* at some " 
              << "previous ply-depth." << endl 
              << "Our *swing hypothesis* is that players are sensitive to " 
              << "earlier-ply judgments" << endl 
              << "to degrees not subsumed by the depth-15 results.  Thus our " 
              << "full model intends" << endl 
              << "to apply weights to results from various ply-depths.  " 
              << "However explained, " << endl 
              << "this accounts for all best-fits " 
              << "*over-predicting* the " 
              << "% of equal-top matches." << endl; 
         cout << "Hit return to see more results and ideas:" << endl; 
 
         if (ens.usingCin()) { 
            IRutil::finishLine(cin);  
         } 
      } 
 
      cout << endl; 
      cout << "Among turns with " << qualif << "three top-eval moves " 
           << "and one of them chosen: " << endl; 
      cout << "# of 3-zero turns: " << setw(5) << right << threeZeroTurns<<endl 
           << "Expected #matches: " << setw(5) << int(threeZeroTurns/3.0 + 0.5) 
           << ",  two-sigma range " << setw(8) << setprecision(2) << fixed 
           << threeZeroTurns/3.0 - 2*sigmaThree << "--" << left 
           << threeZeroTurns/3.0 + 2*sigmaThree << endl 
           << "First move chosen: " << setw(5) << right << threeZeroMatches 
           << ", " << 100.0*threeZeroMatches/threeZeroTurns << "%" 
           << ",  deviation " << devThree << " sigmas" << endl 
           << "Second way chosen: " << setw(5) << right << threeZeroSeconds 
           << ", " << 100.0*threeZeroSeconds/threeZeroTurns << "%" 
           << endl 
           << "Third move chosen: " << setw(5) << right << threeZeroThirds 
           << ", " << 100.0*threeZeroThirds/threeZeroTurns << "%" 
           << endl << endl; 
 
      cout << "Among turns with " << qualif << "four top-eval moves " 
           << "and one of them chosen: " << endl; 
      cout << "# of 4-zero turns: " << setw(5) << right << fourZeroTurns << endl 
           << "Expected #matches: " << setw(5) << int(fourZeroTurns/4.0 + 0.5) 
           << ",  two-sigma range " << setw(8) << setprecision(2) << fixed 
           << fourZeroTurns/4.0 - 2*sigmaFour << "--" << left 
           << fourZeroTurns/4.0 + 2*sigmaFour << endl 
           << "First move chosen: " << setw(5) << right << fourZeroMatches 
           << ", " << 100.0*fourZeroMatches/fourZeroTurns << "%" 
           << ",  deviation " << devFour << " sigmas" << endl 
           << "Second way chosen: " << setw(5) << right << fourZeroSeconds 
           << ", " << 100.0*fourZeroSeconds/fourZeroTurns << "%" 
           << endl 
           << "Third move chosen: " << setw(5) << right << fourZeroThirds 
           << ", " << 100.0*fourZeroThirds/fourZeroTurns << "%" 
           << endl 
           << "Fourth way chosen: " << setw(5) << right << fourZeroFourths 
           << ", " << 100.0*fourZeroFourths/fourZeroTurns << "%" 
           << endl << endl; 
 
      cout << "Among turns with " << qualif << "five top-eval moves " 
           << "and one of them chosen: " << endl; 
      cout << "# of 5-zero turns: " << setw(5) << right << fiveZeroTurns << endl 
           << "Expected #matches: " << setw(5) << int(fiveZeroTurns/4.0 + 0.5) 
           << ",  two-sigma range " << setw(8) << setprecision(2) << fixed 
           << fiveZeroTurns/4.0 - 2*sigmaFour << "--" << left 
           << fiveZeroTurns/4.0 + 2*sigmaFour << endl 
           << "First move chosen: " << setw(5) << right << fiveZeroMatches 
           << ", " << 100.0*fiveZeroMatches/fiveZeroTurns << "%" 
           << ",  deviation " << devFour << " sigmas" << endl 
           << "Second way chosen: " << setw(5) << right << fiveZeroSeconds 
           << ", " << 100.0*fiveZeroSeconds/fiveZeroTurns << "%" 
           << endl 
           << "Third move chosen: " << setw(5) << right << fiveZeroThirds 
           << ", " << 100.0*fiveZeroThirds/fiveZeroTurns << "%" 
           << endl 
           << "Fourth way chosen: " << setw(5) << right << fiveZeroFourths 
           << ", " << 100.0*fiveZeroFourths/fiveZeroTurns << "%" 
           << endl 
           << "Fifth move chosen: " << setw(5) << right << fiveZeroFifths 
           << ", " << 100.0*fiveZeroFifths/fiveZeroTurns << "%" 
           << endl << endl; 
 
 
      cout << "Does the engine maintain order rigidly for moves after the " 
           << "first, or vary more?" << endl 
           << "Is a \"swing effect\" prominent only between the top two " 
           << "moves players consider?" << endl 
           << "Our first testable prediction is that among turns where the " 
           << "ply-15 first move " << endl 
           << "was judged inferior at previous ply-depths, first-line " 
           << "matches should run below" << endl 
           << "what ply-15 stats indicate.  I.e. \"swing moves\" are " 
           << "harder for players to find." << endl << endl 
           << "Second, our %-tile best-fits that get the first-move and tail " 
           << "frequencies and " << endl 
           << "falloff correct all over-predict the frequency of the " 
           << "second move.  Unless" << endl 
           << "the equal-top move cases account " 
           << "for all of this deviation, we must predict" << endl 
           << "that the second-best move usually swings toward or catches " 
           << "up with the first," << endl 
           << "rather than the first move breaking away.  " 
           << "Finally, our full model will probe" << endl 
           << "whether players of different strengths are sensitive " 
           << "to evals at different" << endl 
           << "ply-depths.  "; 
      cout << "Hit return to see the final surprising fact." << endl; 
 
      if (ens.usingCin()) {  
         IRutil::finishLine(cin);  
      } 
       
      cout << "Among turns with second move rated exactly 0.01 worse, " << endl 
           << "and one of the first two moves played: " << endl << endl; 
      cout << "# of 0,0.01 turns: " << setw(5) << right << sdeq01Turns << endl 
           << "# 1st move played: " << setw(5) << right << sdeq01Matches 
           << ", " << 100.0*sdeq01Matches / sdeq01Turns << "%" << endl 
           << "# 2nd move played: " << setw(5) << right  
           << sdeq01Turns - sdeq01Matches << endl << endl; 
      cout << "That's right, 0.01 makes almost no difference in frequency! " 
           << endl << "So, a 2nd move rated 0.01 worse is played more often " 
           << "than one rated equal!" << endl 
           << "In particular, this seems to rule out any explanation that " 
           << "the 2nd of two equal" << endl 
           << "moves is expected to be 0.005 worse before any 2-digit " 
           << "rounding of evals." << endl << endl 
           << "Hence the phenomenon may be specific to cases of two or more " 
           << "moves that reach" << endl 
           << "the same end at high depth, but one " 
           << "more quickly or overtly." << endl 
           << "Whether this is part of a larger \"swing\" effect or explains " 
           << "the discrepancy" << endl 
           << "by itself needs closer inspection, and independent data sets "  
           << "would help." << endl; 
 
      cout << "Hit return to continue exploring (with option to quit)." << endl; 
 
      cout.flags(IRutil::URFORMAT); 
      cout << setprecision(6); 
      resultsFile.close(); 
      if (ens.usingCin()) { 
//cerr << endl << "Yes, using cin here." << endl; 
         IRutil::finishLine(cin);  
      } 
   } 
 
//-------------------------Demoralization Demo----------------------------- 
 
   void demoralizationDemo(Ensemble& ens) { 
 
      string fileName;   //*vital* to keep this updated! 
      ofstream* filep = agreeDemoFile(ens, fileName);  //fileName mutable 
      string timestamp = IRutil::getRawTime();  //already trimmed 
      string cal = IRutil::getTime(); 
      (*filep) << IRutil::IRCOMMENT << "Demoralization Demo done on " << cal 
               << " (" << timestamp << ")" << endl << endl; 
      filep->close();  //now we give writing control over to "ens" 
      ens.addOutputFile(fileName, false);  //false -> no extra comment 
 
      double defAdv, bigAdv; 
 
      cout << "Enter value for a definite advantage (+/-), e.g. 1.0 or 0.75: "; 
      ens.loggedRead(defAdv); 
 
      cout << "Enter value for a big advantage (+-), e.g. 2.0 or 1.5: "; 
      ens.loggedRead(bigAdv); 
 
      string da = IRutil::ftoa(defAdv,2); 
      string ba = IRutil::ftoa(bigAdv,2); 
 
      const DemoChoices dc = demoOptions(ens);  
      if (dc.cancel) { 
         cout << "Canceled---returning." << endl; 
         return; 
      } 
      //else 
 
      double origCap = ens.getFocusTrial()->getDeltaCap(); 
      double newCap = origCap;  //dc.newCap; 
//    ens.getFocusTrial()->setDeltaCap(newCap); 
 
      istringstream* isp = new istringstream(dc.groundFilters); 
      ens.interactFromStream(isp, ""); 
      //leaves control back at main menu and restores input from cin 
 
      if (dc.fitThenGo) { 
         ens.runFit(); 
      } 
 
      string st = " newFilters "; 
      st += " EvalWhite ewUpBig geq " + ba + " EvalWhite ewUpDef geq " + da; 
      st += " EvalWhite ewDownBig leq -"+ba + " EvalWhite ewDownDef leq -"+da; 
      st += " WhiteToMove wtm BlackToMove btm "; 
      st += " AndFilter wtmUpBig wtm y ewUpBig n "; 
      st += " AndFilter wtmUp wtm y ewUpDef n "; 
      st += " AndFilter btmUpBig btm y ewDownBig n "; 
      st += " AndFilter btmUp btm y ewDownDef n "; 
      st += " AndFilter wtmDownBig wtm y ewDownBig n "; 
      st += " AndFilter wtmDown wtm y ewDownDef n "; 
      st += " AndFilter btmDownBig btm y ewUpBig n "; 
      st += " AndFilter btmDown btm y ewUpDef n "; 
      st += " OrFilter ptmUpBig wtmUpBig y btmUpBig n "; 
      st += " OrFilter ptmUp wtmUp y btmUp n "; 
      st += " OrFilter ptmDownBig wtmDownBig y btmDownBig n "; 
      st += " OrFilter ptmDown wtmDown y btmDown n done "; 
 
      st += " newFilters NotFilter ptmNotDown ptmDown "; 
      st += " NotFilter ptmNotUp ptmUp "; 
      st += " NotFilter ptmNotDownBig ptmDownBig "; 
      st += " NotFilter ptmNotUpBig ptmUpBig "; 
      st += " AndFilter nearlyEven ptmNotDown y ptmNotUp n "; 
      st += " AndFilter ptmUpDef ptmUp y ptmNotUpBig n "; 
      st += " AndFilter ptmDownDef ptmDown y ptmNotDownBig n done "; 
 
      st += " detach ewUpBig y ewUpDef y ewDownBig y ewDownDef "; 
      st += " y wtm y btm y wtmUpBig y wtmUp y btmUpBig y btmUp "; 
      st += " y wtmDownBig y wtmDown y btmDown y btmDownBig "; 
      st += " y ptmUpBig y ptmUp y ptmDown y ptmDownBig "; 
      st += " y ptmNotDown y ptmNotUp y ptmNotUpBig y ptmNotDownBig "; 
      st += " y nearlyEven y ptmUpDef y ptmDownDef n "; 
 
      st += " perfTest y n ";       //does overall performance stats 
      st += " attach ptmUp n perfTest y n detach ptmUp n "; 
      st += " attach ptmDown n perfTest y n detach ptmDown n "; 
      st += " attach ptmUpBig n perfTest y n detach ptmUpBig n "; 
      st += " attach ptmUpDef n perfTest y n detach ptmUpDef n "; 
      st += " attach nearlyEven n perfTest y n detach nearlyEven n "; 
      st += " attach ptmDownDef n perfTest y n detach ptmDownDef n "; 
      st += " attach ptmDownBig n perfTest y n detach ptmDownBig n "; 
 
      st += " hideFilters ewUpBig y ewUpDef y ewDownBig y ewDownDef "; 
      st += " y wtm y btm y wtmUpBig y wtmUp y btmUpBig y btmUp "; 
      st += " y wtmDownBig y wtmDown y btmDown y btmDownBig "; 
      st += " y ptmUpBig y ptmUp y ptmDown y ptmDownBig "; 
      st += " y ptmNotDown y ptmNotUp y ptmNotUpBig y ptmNotDownBig "; 
      st += " y nearlyEven y ptmUpDef y ptmDownDef n "; 
 
      isp = new istringstream(st); 
      ens.interactFromStream(isp,"");  // "" starts from top of stream 
 
      cout << "(Hid a bunch of filters, so ignore any duplication warnings.)" 
           << endl; 
 
      ens.closeOutputFile(fileName);  //no more output, removed from "outs" 
 
      ifstream resultsFile(fileName.c_str()); 
      string line; 
      findLineWith(resultsFile, timestamp, line); 
      PerfData overallPerf(newCap),  upPerf(newCap),  downPerf(newCap),   
               upBigPerf(newCap),  upDefPerf(newCap); 
      PerfData nearlyEvenPerf(newCap),  downDefPerf(newCap),   
               downBigPerf(newCap); 
 
      parsePerfData(resultsFile, "Test using", overallPerf); 
      parsePerfData(resultsFile, "ptmUp", upPerf); 
      parsePerfData(resultsFile, "ptmDown", downPerf); 
      parsePerfData(resultsFile, "ptmUpBig", upBigPerf); 
      parsePerfData(resultsFile, "ptmUpDef", upDefPerf); 
      parsePerfData(resultsFile, "nearlyEven", nearlyEvenPerf); 
      parsePerfData(resultsFile, "ptmDownDef", downDefPerf); 
      parsePerfData(resultsFile, "ptmDownBig", downBigPerf); 
 
      bool flm = (dc.matchStat == "first"); 
      cout << endl << "SUMMARY of performance when \"Big\" (+-) is >=" << ba 
           << " and \"Def\" (+/-) is " << da << " to " << ba << endl; 
      cout << IRutil::DASHES << endl << PerfData::perfHeader(flm) << endl 
           << IRutil::DASHES << endl 
           << overallPerf.perfLine("Overall", flm) << endl 
           << upPerf.perfLine("Ahead", flm) << endl 
           << downPerf.perfLine("Behind", flm) << endl 
           << endl 
           << upBigPerf.perfLine("Up Big", flm) << endl 
           << upDefPerf.perfLine("Up Def", flm) << endl 
           << nearlyEvenPerf.perfLine("Evenish", flm) << endl 
           << downDefPerf.perfLine("Down Def", flm) << endl 
           << downBigPerf.perfLine("Down Big", flm) << endl 
           << IRutil::DASHES << endl << endl; 
  
      cout << "Either (1) we may infer humans \"play it safe\" or are " 
           << "careless when well ahead," << endl 
           << "and get demoralized when behind, or (2) the model needs to " 
           << "scale \"delta\" values" << endl 
           << "down when the overall evaluation is large.  However, the " 
           << "\"Up Def\" versus " << endl 
           << "\"Down Def\" results argue against (2), " 
           << "and (2) may imply (1) anyway!" << endl 
           << endl 
           << "Either way, this explains why computers are perceived as " 
           << "such tough defenders," << endl 
           << "especially when well behind!  (Caveat: data is small and " 
           << "\"heteroskedastic\".)" << endl; 
      cout << endl << "Hit return to return." << endl; 
 
      resultsFile.close(); 
      //ens.getFocusTrial()->setDeltaCap(origCap); 
      if (ens.usingCin()) { 
         IRutil::finishLine(cin); 
//cerr << endl << "Yes, using cin here." << endl; 
         IRutil::finishLine(cin);      //second call needed to get actual pause. 
      } 
      cout.flags(IRutil::URFORMAT); 
   } 
 
};  //end of Demos namespace 
 
 
#endif    //end of #ifndef __IR_DEMOS_CPP__ 
 
 
 
