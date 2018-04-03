//File-------------------Ensemble.h------------------------------------- 
 
#ifndef __IR_ENSEMBLE_H__ 
#define __IR_ENSEMBLE_H__ 

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
 
#ifndef __IR_DEMOS_H__ 
#include "Demos.h" 
#endif	  //end of #ifndef __IR_DEMOS_H__ 
 
 
//--------------------Ensemble Class and UI----------------------------- 
 
struct CinAgainstConst {    //debugging aid for now 
   bool status; 
   CinAgainstConst(bool st) : status(st) { } 
}; 
 
/** Dictionary for curves and trials and filters user may keep active. 
    Intended as a singleton class.  Includes UI functions. 
 */ 
class Ensemble : public Logging { 
 
   enum MainMenuChoice { 
      NEW_TRIAL, 
      CHANGE_TRIAL, 
      SHOW_TRIAL, 
      LOAD_DELTA_SCALE, 
      ADD_TURNS, 
      CLEAR_TURNS, 
      NEW_FILTER, 
      ATTACH_FILTERS, 
      DETACH_FILTERS, 
      CLEAR_FILTERS, 
      HIDE_FILTERS, 
      NEW_SELECTOR,
      TOGGLE_SELECTORS,
      NEW_TRIAL_SPEC, 
      LOAD_TRIAL_SPEC, 
      HIDE_SPECS, 
      //PERCFIT_TRIAL, 
      RUN_FIT, 
      PERF_TEST, 
      //PERF_TEST_ALL, 
      ADD_OUTPUT_FILE, 
      CLOSE_OUTPUT_FILE, 
      READ_COMMANDS, 
      //RUN_DEMO, 
      QUIT, 
      NUM_MAIN_CHOICES 
   }; 

   enum SelectorKind {   //only the user-built options
      NO_SELECTOR,
      EXISTING_SELECTOR,
      FN_SELECTOR,
      NOT_SELECTOR,
      AND_SELECTOR,
      OR_SELECTOR,
      MAP_SELECTOR,
      MOVE_EVAL_SELECTOR,
      MOVE_RAW_DELTA_SELECTOR,
      MOVE_RAW_SWING_SELECTOR,
      MOVE_SCALED_SWING_SELECTOR,
      NUM_SELECTOR_CHOICES
   };
 
   enum FilterKind { 
      NO_FILTER, 
      EXISTING_FILTER, 
      FN_FILTER, 
      NOT_FILTER, 
      AND_FILTER, 
      OR_FILTER, 
      PLAYER_IS, 
      PLAYER_IS_ONE_OF, 
      PLAYER_IS_ON_MOVE, 
      PLAYER_ON_MOVE_IS_ONE_OF, 
      ON_MOVE_FACING, 
      ON_MOVE_FACING_ONE_OF, 
      WHITE_IS, 
      BLACK_IS, 
      EVENT_IS, 
      DATE_IS, 
      EVENT_DATE_IS,
      YEAR_IS, 
      WHITE_ELO,
      BLACK_ELO,
      PLAYER_ELO,
      ELO_DIFF,
      ELO_DIFF_WITHIN,
      EVAL_WHITE, 
      PREV_EVAL_WHITE, 
      EVAL_WITHIN, 
      PREV_EVAL_WITHIN, 
      NEXT_EVAL_WITHIN,
      TURN_NO, 
      NUM_LEGAL_MOVES, 
      DELTA_I, 
      DELTA_DIFF, 
      SECOND_DELTA, 
      DELTA_N, 
      REP_COUNT,
      PLAYED_MOVE_INDEX, 
      PLAYED_MOVE_IS_SELECTED,
      ENGINE_MOVE_IS_SELECTED,
      EQUAL_TOP_MOVE_IS_SELECTED,
      SOME_MOVE_IS_SELECTED,
      FALLOFF, 
      WEIGHT, 
      PROB_I, 
      NUM_FILTER_CHOICES 
   }; 

   enum SCALE_OPTIONS { 
      WEIGHT_EVALS_FIRST = NUM_DELTA_PATCHES,
      SWING_POLICY,
      DEPTH_WINDOW,
      SWING_WINDOW,
      ZERO_DEPTH,
      JUDGMENT_DEPTH,
      CENTER_DELTA, 
      CENTER_EVAL, 
      STRADDLE_EVALS, 
      RADIUS, 
      SCALE_POWER, 
      GRADIENT, 
      NEG_GRADIENT,
      BETA,
      GAMMA,
      NORM_FACTORS,
      DELTA_CAP, 
      PIN_TO_CAP, 
      EQUAL_TOP_FIX, 
      PATCH_POWER,
      BASE_SCALE_NAME, 
      FINISH_SCALE, 
      NUM_SCALE_OPTIONS 
   }; 

   enum TRIALSPEC_PARAMS {
      GO_SPEC,
      CURVE,
      WEIGHT_FN,
      //ARG_FORMATION,
      RATING,
      SET_FROM_STRING,
      TRIALSPEC_NAME,
      S_VALUE,
      C_VALUE,
      D_VALUE,
      V_VALUE,
      A_VALUE,
      HM_VALUE,
      HP_VALUE,
      B_VALUE,
      //SPEC_JUDGMENT_DEPTH,
      SLIDES,
      SLIDE_CURVE,
      SLIDE_LO,
      SLIDE_MID,
      SLIDE_HI,
      SLIDE_POWER,
      SLIDE_PROP,
      MEMO_STEP,
      MEMO_LIMIT,
      SHOW_STEP,
      SHOW_LIMIT,
      SHOW_CURVE,
      NUM_TRIALSPEC_PARAMS
   };
 
   enum FIT_CHOICES { 
      MIN_METHOD = Minimizer::NUM_MINIMANDS, 
      FUNNEL_FACTORS,
      START_SPEC,
      S_BOUNDS, 
      C_BOUNDS, 
      D_BOUNDS,
      V_BOUNDS,
      A_BOUNDS,
      HM_BOUNDS,
      HP_BOUNDS,
      B_BOUNDS,
      //FIT_JUDGMENT_DEPTH,
      MAX_ITERATIONS,
      NUM_RETRIES,
      PRECISION_X, 
      PRECISION_Y,
      PRECISION_G,
      //WALKOUT,
      WALKOUT_STEP,
      EPSFCN,
      GO,
      BOOT_NUM,
      KEEP_SPEC_IN_BOOT,
      VERBOSE, 
      NUM_FIT_CHOICES 
   }; 

   enum TEST_CHOICES {
      GO_TEST,
      USE_FOCUS_SPEC,
      MISS_IS_MOVE_NPLUSONE,
      FORCE_UNIT_WEIGHTS,
      SHOW_UNWEIGHTED,
      SHOW_UNSCALED,
      NUM_INDICES,
      EQUAL_TOP_INDEX_LIMIT,
      NUM_BOOTSTRAP_TRIALS,
      NUM_RESAMPLE_TRIALS,
      NUM_RESAMPLE_GAMES,
      NUM_RESAMPLE_TURNS,
      ONE_PLAYER_PER_GAME,
      ZMM_FILE,
      ZEV_FILE,
      ZAD_FILE,
      MOVES_FILE,
      MOVES_LIMIT,
      SHOW_INDIVIDUAL_MOVES,
      NUM_TEST_CHOICES
   };
      
       
 
/*//------------------------------------------------------------------------ 
   Data.  Since focusTrial's TrialSpec is *settable* during runs, it is 
   *vital* that the TrialSpec containers use value-semantics, while the 
   trials and filters use reference semantics (via pointers)---indeed, Trial 
   has a disabled copy constructor since it embeds long lists. 
   Because the forward declarations are incomplete, must use pointers here. 
*///------------------------------------------------------------------------ 
   list<GameInfo*>* pGames;
   list<TurnInfo*>* pTurns;  //INV: All trials point to these turns... 
   list<TurnInfo*>* pReferenceTurns; 
   Trial* focusTrial; 
   Trial* referenceTrial;   //...except this one, for Intrinsic Ratings 
   ValueCatalog<TrialSpec>* specs; 
   ValueCatalog<TrialSpec>* hiddenSpecs; 
   RefCatalog<DeltaScale>* scales; 
   RefCatalog<Trial>* trials; 
   RefCatalog<TurnFilter>* filters; 
   RefCatalog<TurnFilter>* hiddenFilters; 
   RefCatalog<MoveSelector>* selectors;

   mutable string dataPath;
   mutable string loadedGlobs;
   mutable vector<string> loadedFiles;

   vector<ostream*>* outs; 
   istream* INP;  //settable---this may cause headaches later... 
   ostream* LOGP; //but at least I localized most code using these to here 
   bool outputToScreen; 
   bool keepGoing; 
   LineScale* theUnitLineScale; 
   CinAgainstConst* const justUsedCin; 

   EnumMenu* modelsMenu; 
   EnumMenu* compareMenu; 
   EnumMenu* mainMenu; 
   EnumMenu* filtersMenu; 
   //GoMenu* selectorsMenu;
   EnumMenu* selectorsMenu;
   EnumMenu* methodMenu;
   EnumMenu* skedMenu;
   EnumMenu* fitMenu; 
   EnumMenu* testMenu;
   EnumMenu* curvesMenu; 
   EnumMenu* weightsMenu; 
   EnumMenu* scalesMenu; 
   EnumMenu* swingMenu;
   EnumMenu* paramsMenu;
   EnumMenu* demosMenu; 
   DynamicMenu* outsMenu; 
 
 friend class EnumMenu; 
 
 //Private methods 
   Ensemble (const Ensemble& rhs);            //disabled copy constructor... 
   Ensemble& operator=(const Ensemble& rhs);  //...and assignment 
   void initMenus(); 
 
   double readS() const; 
   double readC() const; 
   double readD() const;
   double readV() const;
   double readA() const;
   double readHM() const;
   double readHP() const;
   double readB() const;
   pair<int,int> readBounds() const;
   void resetParamsMenu();

   //size_t readJD() const;

   int parseDeltas(string& line, vector<double>* deltas) const; 
   Minimizer::SKED parseSked(string x) const;
 
 public: 
   Ensemble(Trial* initialTrial, string dpath, ostream* logp, istream* IN = &cin); 
   Ensemble(); 
 
   virtual ~Ensemble() { 
      clearFilters();
      clearReferenceTurns();
      clearTurns();   //can make all these the destructor of Trial??
      delete(referenceTrial);   //...except this one, for Intrinsic Ratings
      delete(focusTrial);
      delete(specs);
      delete(hiddenSpecs);
      delete(scales);
      delete(trials);
      delete(filters);
      delete(hiddenFilters);
      delete(selectors);
      delete(pTurns);
      delete(pGames);

      delete(modelsMenu);
      delete(compareMenu);
      delete(mainMenu);
      delete(filtersMenu);
      delete(selectorsMenu);
      delete(methodMenu);
      delete(skedMenu);
      delete(fitMenu);
      delete(testMenu);
      delete(curvesMenu);
      delete(weightsMenu);
      delete(scalesMenu);
      delete(swingMenu);
      delete(paramsMenu);
      delete(demosMenu);
      delete(outsMenu);

      (*LOGP) << endl << endl;    //in case end came in mid-line 
      LOGP->flush(); 
      delete(justUsedCin);
      //delete(INP);
      delete(LOGP);
      delete(outs);
      
   }                              //stream files will close when main exits 
 
   template<class E> 
   void loggedRead(E& item, bool endLine = false, bool write = true) const { 
      (*INP) >> item; 
      if (INP->fail() && !INP->eof()) { 
         cout << "Failed to read item, please select again." << endl; 
         INP->clear(); 
         INP->ignore(numeric_limits<streamsize>::max(),'\n'); 
         loggedWrite("Must edit glitch here before replaying!"); 
      } else { 
         justUsedCin->status = true; 
         if (write) { 
            (*LOGP) << item << (endLine ? "\n" : " "); 
         } 
      } 
   } 
   //These would all be no longer "const" if they modified a "justUsedCin" 
   //field, rather than doing so thru the CinAgainstConst object! 
 
   virtual void loggedReadString(string& item, bool endLine = false, 
                                 bool write = true) const { 
      loggedRead(item,endLine,write); 
   } 
   virtual void loggedReadInt(int& item, bool endLine = false, 
                              bool write = true) const { 
      loggedRead(item,endLine,write); 
   } 
   virtual void loggedReadDouble(double& item, bool endLine = false, 
                                 bool write = true) const { 
      loggedRead(item,endLine,write); 
   } 

   template<class E>
   void loggedReadBounded(E& item, const double lo, const double hi,
                                  bool endLine = false, bool write = true) const {
      bool entering = true;
      while (entering) {
         loggedRead(item,endLine,write);
         if (item >= lo && item <= hi) {
            entering = false;
         } else {
            cout << "Item out of bounds [" << lo << "," << hi << "]; please select again: " << endl;
         }
      }
   }
 
   virtual void loggedWrite(const string& st, bool endLine = false)  const { 
      (*LOGP) << st << (endLine ? "\n" : " "); 
   } 
   virtual string loggedGetline() const { 
      string item; 
      getline(*INP,item); 
      (*LOGP) << item << endl; 
      return item;
   } 
   virtual bool atEOF() const { return (*INP).eof(); } 
   virtual bool usingCin() const { return INP == &cin; } 
 
   void setDataPath(string newPath) { dataPath = newPath; }

//One set of turns is maintained as the reference set for computing IPR's.
//Currently it is R3Turns.txt, and the variable is set true only at pgm start.
//A bit of a kludge---should probably make a separate object rather than
//re-use code here.  Changing to SF7Turns.txt
//Returned pointer is used only by selectionOnly, else Ensemble::turns is set
 

   //list<TurnInfo*>* readInputFile(const string& fileName, bool referenceOnly = false,
                                       //bool selectionOnly = false) const; 

   list<TurnInfo*>* readInputFile(istream& file, bool referenceOnly = false,
                                       bool selectionOnly = false) const; 
   void readTurnsFromFileGlob(const string& fileGlob, bool referenceOnly = false,
                                                      bool selectionOnly = false) const; 
   void readTurns(istream& IN) const; 
      //public readers can use other instreams 
   void printDeltas(const string& name) const; 
   void printTurns(const string& name) const; 
 
   TrialSpec makeTrialSpec() const; 
   //LineScale* makeDeltaScale(double deltaCap) const; 
   LineScale* makeDeltaScale() const;
   SwingInfo* makeSwingInfo() const;
 
   //void addPercentileRange (vector<double>* percentilePoints, 
            //double low, double high, int numPoints, bool includeEnds) const; 
   //void readPercentileRange (vector<double>* percentilePoints) const; 
 
   Trial* getFocusTrial() const { 
      return focusTrial; 
   } 
 
   Trial* getReferenceTrial() const { 
      return referenceTrial; 
   } 
 
   //Trial* getReferenceTrial3() const { 
      //return referenceTrial3; 
   //} 
 
   void clearFilters() const { 
      focusTrial->clearFilters(); 
      filters->clearStars(); 
   } 
 
   void quit() { 
      keepGoing = false; 
   } 
 
   void showActiveTrial(ostream& OUT) const { 
      OUT << string(*focusTrial) << endl; 
   } 
 
   void addOutputStream(ostream* ostrm, bool firstTime = true) const { 
      string cal = IRutil::getTime(); 
      string timestamp = IRutil::getRawTime(); 
      if (firstTime) { 
         (*ostrm) << endl << IRutil::IRCOMMENT << " Session run on " << cal 
                  << " (" << timestamp << ")" << endl << endl; 
      } 
      outs->push_back(ostrm); 
   } 
 
   bool addOutputFile(const string& name, bool firstTime = true) const { 
      if (outsMenu->hasEntry(name)) { 
         cout << "File or stream already active." << endl; 
      } else if (name == "cout") { 
         outs->push_back(&cout); 
         outsMenu->addEntry("cout", ""); 
      } else { 
         ofstream* filep = new ofstream(name.c_str(), ios::app); 
         if(!filep->is_open()) { 
            cerr << "Cannot stat the output file, returning..." << endl; 
            return false; 
         } else { 
            outsMenu->addEntry(name,""); 
            addOutputStream(filep, firstTime); 
         } 
      } 
      return true; 
   } 
 
 
   void closeOutputFile(const string& name) { 
      int index = 0; 
      while (index < outsMenu->size() && outsMenu->nameOf(index) != name) { 
         index++; 
      } 
      if (outsMenu->nameOf(index) == name) { 
         outsMenu->removeEntry(name); 
         vector<ostream*>::iterator it = outs->begin() + index; 
         ofstream* filep = dynamic_cast<ofstream*>(*it); 
         if (filep) { 
            filep->close(); 
         } else { 
            cout << "Muting some output to screen..." << endl; 
            outputToScreen = false; 
         } 
         outs->erase(it); 
      } else { 
         cerr << "File you attempted to close not found." << endl; 
      } 
   } 
 
   void newTrial(); 
   void changeTrial();
 
   void loadNamedTrial(); 
 
   void loadDeltaScale(bool refToo = true); 
 
   void loadNamedTrialSpec(); 
 
   void hideSpecs(); 
 
   Minimizer* makeMinimizer() const; 
 
   void runFit(); 

   void runPerfTest() const;
 
   void addTurns() const {   //does not reassign "turns" pointer... 
      if (usingCin()) {
         readTurns(*INP);       //now logs---previously was not logged 
         reset();               //...but can't be "const" for this reason? 
      } else {
         string fileName;
         (*INP) >> fileName;
         readTurnsFromFileGlob(fileName);  //also logs now
         reset();
      }
   } 

   void clearTurns() const {
      focusTrial->clearFilteredDecisions();  //removes source pointers.
      list<TurnInfo*>::iterator tit = pTurns->begin();
      list<TurnInfo*>::iterator tite = pTurns->end();
      while (tit != tite) {
         TurnInfo* p = *tit++;
         delete(p); //does not invalidate iterator itself, zaps TurnInfo
      }
      pTurns->clear();
      list<GameInfo*>::iterator git = pGames->begin();
      list<GameInfo*>::iterator gite = pGames->end();
      while (git != gite) {
         GameInfo* g = *git++;
         delete(g); //does not invalidate iterator itself, zaps TurnInfo
      }
      pGames->clear();
      loadedGlobs = "";
      loadedFiles = vector<string>();
   }

   void clearReferenceTurns() const {
      referenceTrial->clearFilteredDecisions();  //removes source pointers.
      list<TurnInfo*>::iterator tit = pReferenceTurns->begin();
      list<TurnInfo*>::iterator tite = pReferenceTurns->end();
      while (tit != tite) {
         TurnInfo* p = *tit++;
         delete(p); //does not invalidate iterator itself, zaps TurnInfo
      }
      pReferenceTurns->clear();
   }

   TurnFilter* chooseCreatedFilter(); 
 
   TurnFilter* makeFilter(int choice); 
 
   void addNewFilter(TurnFilter* tf); 
 
   void attachFilters(); 
 
   void detachFilters(); 
 
   void hideFilter(TurnFilter* tf);
   void hideFilters(); 

   void newSelectors();   //not const since it changes status of Trial
   
   MoveSelector* chooseCreatedSelector() const;  //const unlike chooseCreatedFilter since no reset()
   
   MoveSelector* makeSelector(int choice) const;

   MoveSelector* mapSelectorInFile(const string& name, const MoveSelector* ms) const;

   void addSelector(MoveSelector* ms);
   void justAddSelector(MoveSelector* ms);  //does not set
   void setSelector(MoveSelector* ms);

   void toggleOneSelector(); //changes status but could be const in "pseudo" pointer sense
   void toggleSelectors();   //changes status and re-generates focused list.
 
   void reset() const { 
//cerr << "aa";
      focusTrial->reset(); 
//cerr << "bb";
      trials->setDescription(focusTrial->getName(), 
                             focusTrial->getDescription()); 
      //if (focusTrial->getDecisions()->empty() && usingCin()) { 
      if (focusTrial->numDecisions() == 0 && usingCin()) {
        cout << "All tuples filtered out---may see NaNs or worse!" << endl; 
      } 
//cerr << "cc";
   } 
 
   void addTrialSpec(TrialSpec ts); 
 
   void newFilters(); 
 
   void runDemo(int choice); 
 
   void doMainMenuItem(int choice); 
 
   void interactFromStream(istream* istrp, const string& matchString); 
 
   void interact(); 
 
}; //end of class Ensemble, implementation follows menu classes 
 
 
#endif    //end of #ifndef __IR_ENSEMBLE_H__ 
 
 
 
