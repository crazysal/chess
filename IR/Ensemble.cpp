//File--------------------Ensemble.cpp----------------------------------- 
 
#ifndef __IR_ENSEMBLE_CPP__ 
#define __IR_ENSEMBLE_CPP__ 


#ifndef __IR_ENSEMBLE_H__
#include "Ensemble.h"
#endif    //end of #ifndef __IR_ENSEMBLE_H__

 
//--------------IMPLEMENTATION OF ENSEMBLE CLASS------------------------- 
 
Ensemble::Ensemble(Trial* initialTrial, string dpath, ostream* logp, istream* IN) 
 : pGames(new list<GameInfo*>()),
   pTurns(initialTrial->getTurns()), //uses trial's pointer
   pReferenceTurns(new list<TurnInfo*>()), 
   //pReferenceTurns(NULL),
   focusTrial(initialTrial), 
   referenceTrial(NULL), 
   //referenceTrial3(NULL), 
             //Catalogs all have the "cancel" option, as enabled by default 
   specs(new ValueCatalog<TrialSpec>(this, 1,  
      IRutil::DASHLINE + "\n" + "Active trial specs---enter # or name",  
      IRutil::DASHLINE, true, true, false)), 
                         // ^^^^ can show more options 
   hiddenSpecs(new ValueCatalog<TrialSpec>(this, 1,  
      IRutil::DASHLINE + "\n" + "Inactive trial specs---enter # or name",  
      IRutil::DASHLINE, true, false, false)), 
                         // ^^^^^ does not 
   scales(new RefCatalog<DeltaScale>(this, 1,  
      "Scales, * shows loaded one\n" + IRutil::DASHLINE,  
      IRutil::DASHLINE + "\nEnter -2/moreOptions to define new scale", 
      true, false, false)), 
   trials(new RefCatalog<Trial>(this, 1,  
      "Trials, * shows active one\n" + IRutil::DASHLINE,  
      IRutil::DASHLINE, true, false, false)), 
   filters(new RefCatalog<TurnFilter>(this, 1,  
      IRutil::DASHLINE + "\n" + "Filters, * shows active one(s).",  
      IRutil::DASHLINE, true, true, false)), 
   hiddenFilters(new RefCatalog<TurnFilter>(this, 1,  
      IRutil::DASHLINE + "\n" + "Hidden Filters---# or name to attach, or " 
                     + "-2/moreOptions to define new", 
      IRutil::DASHLINE, true, true, false)), 
   selectors(new RefCatalog<MoveSelector>(this, 1,
      "Move Selectors, * shows active ones.\nEnter \"go\" or \"OK\" or \"ok\" to submit\n" + IRutil::DASHLINE,
      IRutil::DASHLINE, true, true, false)),

   dataPath(dpath),
   loadedGlobs(""),
   loadedFiles(vector<string>()),

   outs(new vector<ostream*>()), INP(IN), LOGP(logp), 
   outputToScreen(true),  
   keepGoing(true),  
   theUnitLineScale(new UnitLineScale("UnitLineScale", true, 
      new SwingInfo(map<APPLICATION_POLICIES, double>(), true, false, true,
                    false, false, 0, false, false, 0.0),
      NO_PATCH, IRutil::CENTRALPATCHPOWER, 0.00, map<string,double>(), 1, 30, 30, 6, 20, 0)), 
   justUsedCin(new CinAgainstConst(false)), 
   modelsMenu(new EnumMenu(this,Models::NUM_MODELS)), 
   compareMenu(new EnumMenu(this,3,0,false,true)),        //OK to hard-code "3" 
   mainMenu(new EnumMenu(this,NUM_MAIN_CHOICES,1,true)),      //put names after 
   filtersMenu(new EnumMenu(this,NUM_FILTER_CHOICES,0,true)), //ditto, # from 0 
   //selectorsMenu(new GoMenu(this,NUM_SELECTOR_CHOICES,1,true,true,IRutil::DASHLINE,IRutil::DASHLINE,true,true,false)),
   selectorsMenu(new EnumMenu(this,NUM_SELECTOR_CHOICES,0,true,true,IRutil::DASHLINE,IRutil::DASHLINE,true,true,false)),
   methodMenu(new EnumMenu(this, Minimizer::NUM_METHODS,0,false,true)),
   skedMenu(new EnumMenu(this, Minimizer::NUM_SKEDS,0,false,true)),
   fitMenu(new EnumMenu(this,NUM_FIT_CHOICES,1,false,true)),  //pads names 
   testMenu(new EnumMenu(this,NUM_TEST_CHOICES,0,false,true)),
   curvesMenu(new EnumMenu(this, IRfun::NUM_CURVES,1,false,true)), 
   weightsMenu(new EnumMenu(this, IRfun::NUM_WEIGHTS,1,false,true)), 
   scalesMenu(new EnumMenu(this, NUM_SCALE_OPTIONS, 0, false, true)), 
   swingMenu(new EnumMenu(this, NUM_SWING_OPTIONS, 0, false, true)),
   paramsMenu(new EnumMenu(this, NUM_TRIALSPEC_PARAMS, 0, false, true)),
   demosMenu(new EnumMenu(this,Demos::NUM_DEMOS)), 
   outsMenu(new DynamicMenu(this,false,1,false,true,"","",true,false,true)) 
{ 
   initMenus(); 
   TrialSpec ts = initialTrial->getTrialSpec(); 
   trials->addItem(focusTrial,"",false,true);  //CLASS INV: focusTrial starred 
   specs->addItem(ts,"",false,true);           //ditto for its spec, by value 
   if (initialTrial->scalesDeltas()) { 
      scales->addItem(initialTrial->getScale(), "", false, true); 
   } 
   string cal = IRutil::getTime(); 
   string timestamp = IRutil::getRawTime(); 
   (*LOGP) << IRutil::IRCOMMENT << " " << "Session run on " << cal << " (" 
           << timestamp << "):" << endl; 
   outs->push_back(&cout); 
   map<string, double> engineInits;
   engineInits[string("omodo")] = 1.01;
   engineInits[string("tockfish")] = 1.0;
   //LineScale* ls = new TranslateByEval(new MulScale("RefMulScale",
                                       //false, NO_PATCH, IRutil::CENTRALPATCHPOWER, 0.00,
                                       //engineInits, 5, 30, 30, 0.00, 1.0, 1.000, 1.000, true));

   //LineScale* ls = new TranslateByEval(new FullFractalScale("RefFullFractalScale",
   //string swingPolicy = "0(separate,relative2,damped)";
   map<APPLICATION_POLICIES, double> appp;
   appp[SEPARATE_REL2] = 1.0;
   //appp[LINK_REL2] = 1;
   SwingInfo* swi = new SwingInfo(appp, true, false, true, false, false, 0, false, false, 0.0);

   LineScale* ls = new TranslateByEval(new FullFractalScale("RefFullFractalScale", true, swi, NO_PATCH,
                                       IRutil::CENTRALPATCHPOWER, 0.00, engineInits,
                                       1, 30, 30, 5, 20, 0, 1.0, 1.6, 1.2));   //zero radius;
   //LineScale* ls = new SimpleFullFractalScale("RefFullFractalScale", true, swi,
                                       //MATCH_PROBS, IRutil::CENTRALPATCHPOWER, 0.00,
                                       //engineInits, 1, 30, 30, 5, 19, 0, 1.0, 1.6, 1.2);  //);

   //LineScale* ls3 = new TranslateByEval(new FullFractalScale("RefFullFractalScale(1.0,1.0)",0.03,UP_TIED_TOP,1.0,1.0)); 

   ls->setDeltaCap(IRutil::DELTACAP);
   referenceTrial = new Trial("RefBasicPS", pReferenceTurns, ts, //0.00, 1.00, 49, false, 
         Models::POWER_SHARES, false, true, ls); 
/* 
   referenceTrial->attachFilter(new OrFilter(new EvalWithinFilter(3.0,"ew3ref"), 
                                       new PrevEvalWithinFilter(3.0,"pw3ref"), 
                                       "porew3ref") ); 
*/ 
   referenceTrial->attachFilter(new EvalWithinFilter(3.0,"ew3ref",0)); 
   referenceTrial->attachFilter(new RepCountFilter(EQ,0,true,"RC0ref"));
   //referenceTrial3= new Trial("RefBasicPS3", pReferenceTurns, ts, 0.00, 1.00, 49, 
       //false, Models::POWER_SHARES, false, 4.00, true, ls3); 
   //referenceTrial3->attachFilter(new EvalWithinFilter(3.0,"ew3ref3")); 
   cerr << "Reading reference turns now...";
   //readInputFile("R3Turns.aif", true, false);  //sets pReferenceTurns; 
   //readInputFile("tmp.aif", true);
   readTurnsFromFileGlob("SF7Turns.aif", true);  //sets pReferenceTurns;
   cerr << "done." << endl;
   referenceTrial->setSpec(focusTrial->getSpec()); 
   referenceTrial->setScale(focusTrial->getScale()); 
 
   srand(atoi(timestamp.c_str()));  //master "random" seed, all else determ^c! 
}                                   //for purpose of reproducibility 
 
Ensemble::Ensemble() 
 : pGames(new list<GameInfo*>()),
   pTurns(new list<TurnInfo*>()), 
   pReferenceTurns(new list<TurnInfo*>()), 
   focusTrial(NULL), //INV: this pointer is never null after construction 
             //Catalogs all have the "cancel" option, as enabled by default 
   referenceTrial(NULL), 
   //referenceTrial3(NULL), 
   specs(new ValueCatalog<TrialSpec>(this, 1,  
      IRutil::DASHLINE + "\n" + "Active trial specs---enter # or name",  
      IRutil::DASHLINE, true, true, false)), 
                         // ^^^^ can show more options 
   hiddenSpecs(new ValueCatalog<TrialSpec>(this, 1,  
      IRutil::DASHLINE + "\n" + "Inactive trial specs---enter # or name",  
      IRutil::DASHLINE, true, false, false)), 
                         // ^^^^^ does not 
   scales(new RefCatalog<DeltaScale>(this, 1,  
      "Scales, * shows loaded one\n" + IRutil::DASHLINE, 
      IRutil::DASHLINE + "\nEnter -2/moreOptions to define new scale", 
      true, false, false)), 
   trials(new RefCatalog<Trial>(this, 1,  
      "Trials, * shows active one\n" + IRutil::DASHLINE,  
      IRutil::DASHLINE, true, false, false)), 
   filters(new RefCatalog<TurnFilter>(this, 1,  
      IRutil::DASHLINE + "\n" + "Filters, * shows active one(s).",  
      IRutil::DASHLINE, true, true, false)), 
   hiddenFilters(new RefCatalog<TurnFilter>(this, 1,  
      IRutil::DASHLINE + "\n" + "Hidden Filters---# or name to attach, or " 
                     + "-2/moreOptions to define new", 
      IRutil::DASHLINE, true, true, false)), 
   selectors(new RefCatalog<MoveSelector>(this, 1,
      "Move Selectors, * shows active ones.\nEnter \"go\" or \"OK\" or \"ok\" to submit\n" + IRutil::DASHLINE,
      IRutil::DASHLINE, true, true, false)),

   dataPath("/projects/regan/Chess/CSE712/AIF/"),
   loadedGlobs(""),
   loadedFiles(vector<string>()),
   outs(new vector<ostream*>()), INP(&cin), 
   outputToScreen(true),  
   keepGoing(true),  

   theUnitLineScale(new UnitLineScale("UnitLineScale", true,
      new SwingInfo(map<APPLICATION_POLICIES, double>(), true, false, true,
                    false, false, 0, false, false, 0.0),
      NO_PATCH, IRutil::CENTRALPATCHPOWER, 0.00, map<string,double>(), 1, 30, 30, 6, 20, 0)),
   justUsedCin(new CinAgainstConst(false)), 
   modelsMenu(new EnumMenu(this,Models::NUM_MODELS)), 
   compareMenu(new EnumMenu(this,3,0,false,true)),        //OK to hard-code "3" 
   mainMenu(new EnumMenu(this,NUM_MAIN_CHOICES,1,true)),      //put names after 
   filtersMenu(new EnumMenu(this,NUM_FILTER_CHOICES,0,true)), //ditto, # from 0 
   //selectorsMenu(new GoMenu(this,NUM_SELECTOR_CHOICES,1,true,true,IRutil::DASHLINE,IRutil::DASHLINE,true,true,false)),
   selectorsMenu(new EnumMenu(this,NUM_SELECTOR_CHOICES,0,true,true,IRutil::DASHLINE,IRutil::DASHLINE,true,true,false)),
   methodMenu(new EnumMenu(this, Minimizer::NUM_METHODS,0,false,true)),
   skedMenu(new EnumMenu(this, Minimizer::NUM_SKEDS,0,false,true)),
   fitMenu(new EnumMenu(this,NUM_FIT_CHOICES,1,false,true)),  //pads names 
   testMenu(new EnumMenu(this,NUM_TEST_CHOICES,0,false,true)),
   curvesMenu(new EnumMenu(this, IRfun::NUM_CURVES,1,false,true)),
   weightsMenu(new EnumMenu(this, IRfun::NUM_WEIGHTS,1,false,true)),
   scalesMenu(new EnumMenu(this, NUM_SCALE_OPTIONS, 0, false, true)), 
   swingMenu(new EnumMenu(this, NUM_SWING_OPTIONS, 0, false, true)),
   paramsMenu(new EnumMenu(this, NUM_TRIALSPEC_PARAMS, 0, false, true)),
   demosMenu(new EnumMenu(this,Demos::NUM_DEMOS)), 
   outsMenu(new DynamicMenu(this,false,1,false,true,"","",true,false,true)) 
{ 
   initMenus(); 
   ofstream* filep = new ofstream(IRutil::LOGFILENAME.c_str(), ios::app); 
   if(!filep->is_open()) { 
      cerr << "Cannot read the log file, using null log..." << endl; 
      LOGP = new IRutil::nullstream(); 
   } else { 
      LOGP = filep; 
   } 
   string cal = IRutil::getTime(); 
   string timestamp = IRutil::getRawTime(); 
   (*LOGP) << IRutil::IRCOMMENT << " " << "Session run on " << cal << " (" 
           << timestamp << "):" << endl; 
   outs->push_back(&cout); 
   //int dd = int(100*IRutil::EVALMEMOCAP + 1);
   map<IRfun::CURVES, double> curveBook;
   curveBook[IRfun::INVEXP] = 1.0;
   TrialSpec ts(IRfun::INVEXP, curveBook, IRfun::UNITWTS, "startSpec", 
                0.05, 0.60, 12.5, 0.05, 1.0, 1.0, 1.0, 1.0,
                false, IRfun::FOLDEDLOGISTIC, 0.00, 2.00, 5.00, 1.0, 1.0, 0.01, 0.00);
   map<string, double> engineInits;
   engineInits[string("omodo")] = 1.01;
   engineInits[string("tockfish")] = 1.0;
   //LineScale* ls = new TranslateByEval(new MulScale("RefMulScale",
                                       //false, NO_PATCH, IRutil::CENTRALPATCHPOWER, 0.00,
                                       //engineInits, 5, 30, 30, 0.00, 1.0, 1.000, 1.000, true));

   //LineScale* ls = new TranslateByEval(new FullFractalScale("RefFullFractalScale",
   //string swingPolicy = "0(separate,relative2,damped)";
   map<APPLICATION_POLICIES, double> appp;
   appp[SEPARATE_REL2] = 1.0;
   //appp[LINK_REL2] = 1;
   SwingInfo* swi = new SwingInfo(appp, true, false, true, false, false, 0, false, false, 0.0);

   LineScale* ls = new TranslateByEval(new FullFractalScale("RefFullFractalScale", true, swi, NO_PATCH,
            IRutil::CENTRALPATCHPOWER, 0.00, engineInits,
            1, 30, 30, 5, 20, 0, 1.0, 1.6, 1.2));   //zero radius;
   //LineScale* ls = new SimpleFullFractalScale("RefFullFractalScale", true, swi,
                                       //MATCH_PROBS, IRutil::CENTRALPATCHPOWER, 0.00,
                                       //engineInits, 1, 30, 30, 5, 19, 0, 1.0, 1.6, 1.2);  //); 

   ls->setDeltaCap(IRutil::DELTACAP);
   referenceTrial = new Trial("RefBasicPS", pReferenceTurns, ts, //0.00, 1.00, 49, false, 
         Models::POWER_SHARES, false, true, ls); 
/* 
   referenceTrial->attachFilter(new OrFilter(new EvalWithinFilter(3.0,"ew3ref"), 
                                       new PrevEvalWithinFilter(3.0,"pw3ref"), 
                                       "porew3ref") ); 
*/ 
   referenceTrial->attachFilter(new EvalWithinFilter(3.0,"ew3ref",0)); 
   referenceTrial->attachFilter(new RepCountFilter(EQ,0,true,"RC0ref"));
   //readInputFile("R3Turns.aif", true, false);  //sets pReferenceTurns; 
   //readInputFile("tmp.aif", true, false);
   readTurnsFromFileGlob("SF7Turns.aif", true, false);  //sets pReferenceTurns;
   referenceTrial->setSpec(focusTrial->getSpec());
   referenceTrial->setScale(focusTrial->getScale());


 
 
   srand(atoi(timestamp.c_str()));   //master "random" seed, all else determ^c! 
   newTrial(); 
} 
 
 
/** Initialize the non-dynamic menus, just once for this Ensemble.  Parameters: 
    void addEntry(int index, string name, string desc, string status = "", 
                  bool rd = true, bool starred = false, bool novel = false) { 
    The "name" is used for scripting commands, capitalized for class objects. 
    "rd"=true means the default on this entry re-sets between menu invocations. 
    "novel" means that a line of dashes is inserted above the item. 
    Must be called only /after/ focusTrial is defined! 
 */ 
void Ensemble::initMenus() { 
   //IRfun::initNames();   //sets up curve and weight-method menus 
   //initPolicyNames();    //moved to main
   string pre; 
   pre = "Enter curve to fit (normalized to 1 when x = 0); enter 0/go to exit"; 
   curvesMenu->setPreamble(pre); 
   for (int i = 0; i < IRfun::NUM_CURVES; i++) { 
      IRfun::CURVES ci = IRfun::CURVES(i); 
      curvesMenu->addEntry(ci, IRfun::curveNames[ci],  IRfun::curveDescriptions[ci],
                        "0", false, false, false); 
   } 
   curvesMenu->setStatus(IRfun::INVEXP, "1");
   curvesMenu->setStarred(IRfun::INVEXP, true);
   
 
   pre = "Enter the tuple weighting function to use:"; 
   weightsMenu->setPreamble(pre); 
   for (int i = 0; i < IRfun::NUM_WEIGHTS; i++) { 
      IRfun::WEIGHTS wi = IRfun::WEIGHTS(i); 
      weightsMenu->addEntry(i, IRfun::weightNames[wi],  
                            IRfun::weightDescriptions[wi]); 
   } 
 
   pre = "\nChoose scale type, combination of origins, and scale factors:"; 
   scalesMenu->setPreamble(pre); 
   scalesMenu->addEntry(NO_SCALING, "NoScaling", "No scaling, cannot pin.", 
                        "", true, false, true); 
   scalesMenu->addEntry(LINEAR_SCALE, "LinearScale", string("Linear scale, ") 
                        + "can grade and/or pin", 
                        "", true, false, false); 
   scalesMenu->addEntry(MUL_SCALE, "MulScale", "Multiplies delta by 1/(1+gx)^pow",
                        "", true, false, false);
   scalesMenu->addEntry(MIX_SCALE, "MixScale", "Combine MulScale and subtracting g*eval",
                        "", true, false, false);
   scalesMenu->addEntry(DIVLOG_SCALE, "DivLogScale", "1/g*log(y) scaling", 
                        "", true, false, false); 
   scalesMenu->addEntry(LOG_SCALE, "LogScale", "1/(gy) scaling---gives log(gy)",  
                        "", true, false, false); 
   scalesMenu->addEntry(FRACTAL_SCALE, "FractalScale", "1/(g(y^c)) scaling (c < 1)", 
                        "", true, false, false); 
   scalesMenu->addEntry(FULL_FRACTAL_SCALE, "FullFractalScale", 
      "1/(1 + gy)^c scaling", "", true, true, false); 
   scalesMenu->addEntry(NO_PATCH, "noPatch", "No patch for tied evaluations", 
                        "", true, true, true); 
   scalesMenu->addEntry(MATCH_PROBS, "matchProbs", string("Match empirical ") 
                        + "tied probabilities", 
                        "", true, false, false); 
   scalesMenu->addEntry(BALANCE_TIES, "balanceTies",  
                        "Balance played indices of tied moves", 
                        "", true, false, false); 
   scalesMenu->addEntry(UP_TIED_TOP, "upTiedTop", "Add fix to tied top moves", 
                        "", true, false, false); 
   scalesMenu->addEntry(PUSH_TIED_TOP, "pushTiedTop", string("Progress fix ") 
                        + "to top ties and followers", 
                        "", true, false, false); 
   scalesMenu->addEntry(PUSH_ALL_TIES, "pushAllTies",  string("Progress fix ") 
                        + "to all ties and followers", 
                        "", true, false, false); 

   scalesMenu->addEntry(WEIGHT_EVALS_FIRST, "weightEvalsFirst", 
                        "Make evals as-perceived by player?", "yes",
                        true, false, true);

   map<APPLICATION_POLICIES, double> appp;
   appp[SEPARATE_REL2] = 1.0;
   //appp[LINK_REL2] = 1;
   SwingInfo swi(appp, true, false, true, false, false, 0, false, false, 0.0);
   scalesMenu->addEntry(SWING_POLICY, "swingPolicy", 
                        "Equation options for swing term(s)", string(swi), 
                        true, false, false);
   scalesMenu->addEntry(DEPTH_WINDOW, "depthWindow",
                        "Primary depth window:", "30 .. 30",
                        true, false, false);
   scalesMenu->addEntry(SWING_WINDOW, "swingWindow",
                        "Window for swing:", "5 .. 20",
                        true, false, false);
   scalesMenu->addEntry(ZERO_DEPTH, "zeroDepth",
                        "Left edge for weights over depths", "5",
                        true, false, false);
   scalesMenu->addEntry(JUDGMENT_DEPTH, "judgmentDepth",
                        "Depth to predict (0 for turn max)", "0",
                        true, false, false);

   scalesMenu->addEntry(CENTER_DELTA, "centerDelta", "Scale regardless of eval:", 
                        "off", true, false, true); 
   scalesMenu->addEntry(CENTER_EVAL, "centerEval", "Scale from overall eval:", 
                        "on", true, true, false); 
   scalesMenu->addEntry(STRADDLE_EVALS, "straddleEvals", string("Scale by ") 
                        + "eval and previous eval:", "off", true, false, false); 
   scalesMenu->addEntry(RADIUS,"radius","Highest delta treated at face value =", 
                        "0.00", true, false, true); 
   scalesMenu->addEntry(SCALE_POWER, "scalePower", "c in fractal scales =", 
                        "1.0", true, false, false); 
   scalesMenu->addEntry(GRADIENT, "gradient", "Gradient at plateau end =", 
                        "1.600", true, false, false); 
   scalesMenu->addEntry(NEG_GRADIENT, "negGradient", "Gradient at negative end =",
                        "1.200", true, false, false);
   scalesMenu->addEntry(BETA, "beta", "Slope mul for (delta-bx) subtraction",
                        "1.000", true, false, false);
   scalesMenu->addEntry(GAMMA, "gamma", "Comb. factor for MulScale & subtraction",
                        "1.000", true, false, false);
   scalesMenu->addEntry(NORM_FACTORS, "normFactors", 
                        //"Normalization factors (stockfish-7 = 1.0)",
                        "", 
                        "(*omodo*,0.96688)(*tockfish*,1.00000)",
                        true, false, false);
   scalesMenu->addEntry(DELTA_CAP, "deltaCap", "Cap on blunder deltas = ", 
                        //IRutil::ftoa(focusTrial->getDeltaCap(), 2), 
                        IRutil::ftoa(IRutil::DELTACAP, 2),
                        true, false, true); 
   scalesMenu->addEntry(PIN_TO_CAP, "pinToCap", "Zero differential at cap?", 
                        "no", true, false, false); 
   scalesMenu->addEntry(EQUAL_TOP_FIX, "equalTopFix", string("Delta for ") 
                        + "equal-top moves = ", "0.00", true, false, false); 
   scalesMenu->addEntry(PATCH_POWER, "patchPower", "Power for equal-top moves",
                        IRutil::ftoa(IRutil::CENTRALPATCHPOWER), false, false, false);
   string infix = "[" + scalesMenu->getStatus(DEPTH_WINDOW) + "," + scalesMenu->getStatus(SWING_WINDOW)
                      + "; " + scalesMenu->getStatus(JUDGMENT_DEPTH) + "]";
   string suffix = "(" + scalesMenu->getStatus(RADIUS) + ","  
                       + scalesMenu->getStatus(SCALE_POWER) + "," 
                       + scalesMenu->getStatus(GRADIENT) + ","
                       + scalesMenu->getStatus(NEG_GRADIENT) + ","
                       + scalesMenu->getStatus(BETA) + ","
                       + scalesMenu->getStatus(GAMMA) + ")";  //";"
                       //+ scalesMenu->getStatus(NORM_FACTORS) + ")";
   scalesMenu->addEntry(BASE_SCALE_NAME, "baseScaleName", "", 
                        "SimpleFullFractalScale" + infix + suffix, true, false, true); 
   scalesMenu->addEntry(FINISH_SCALE, "finishScale", string("Save Settings") 
                        + " and Finish", "", true, false, false); 
   scalesMenu->setPostamble(IRutil::DASHLINE); 
 
   pre = "Enter factors for equation options; condition swing; couple/decouple params";
   swingMenu->setPreamble(pre);
   swingMenu->addEntry(GO_SWING, "goSwing", "Submit options", "", true, false, true);
   swingMenu->addEntry(NO_SWING_TERM, "noSwing", "Zero out swing term", 
                       "0", false, false, true);
   swingMenu->addEntry(JOIN_TERMS, "joinTerms", "Swing with power a added to delta, then /s and ^c", 
                       "0", false, false, false);
   swingMenu->addEntry(LINK_TERMS, "linkTerms", "Swing/s with power a added to delta/s, then ^c",
                       "0", false, false, false);
   swingMenu->addEntry(SEPARATE_TERMS, "separateTerms", "Swing with power ac (or a) is separate term",
                       "1", false, true, false);
   swingMenu->addEntry(APPLY_TO_PROXY, "applyProxy", "Apply swing to proxy value",
                       "0", false, false, false);
   swingMenu->addEntry(ABS_SWING, "absoluteSwing", "Do not subtract first move's swing sw1", 
                       "0", false, false, true);
   swingMenu->addEntry(REL1_SWING, "rel1Swing", "Swing subtracts sw1 before test",
                       "0", false, false, false);
   swingMenu->addEntry(REL2_SWING, "rel2Swing", "Swing subtracts sw1 after test",
                       "1", false, true, false);
   swingMenu->addEntry(REL3_SWING, "rel3Swing", "Subtraction term has separate power",
                       "0", false, false, false);
   swingMenu->addEntry(COUPLE_PARAMS, "coupleParams", "Use s,c,a,hm in multiple terms",
                       "", false, true, true);
   swingMenu->addEntry(DECOUPLE_PARAMS, "decoupleParams", "Each term has own params",
                       "", false, false, false);
   swingMenu->addEntry(DEPTH_WTD, "depthWeighted", "Swing weights depend on depth",
                       "yes", false, false, true);
   swingMenu->addEntry(UNSCALED_SWING, "unscaledSwing", "Use swing from raw deltas",
                       "", false, false, true);
   swingMenu->addEntry(SCALED_SWING, "scaledSwing", "Use swing from scaled deltas",
                       "", false, true, false);
   swingMenu->addEntry(SIMPLE, "simpleSwing", "Swing values not divided by delta",
                       "", false, false, true);
   swingMenu->addEntry(PROPORTIONAL, "proportionalSwing", "Swing values divided by delta",
                       "", false, false, false);
   swingMenu->addEntry(DAMPED, "dampedSwing", "Swing values 'damped' at large deltas",
                       "", false, true, false);
   swingMenu->addEntry(BOXED, "boxedSwing", "Swing values damped inside delta box",
                       "", false, false, false);
   swingMenu->addEntry(BOXED_OPTION, "boxedOption", "Options 0-9 for boxing swing",
                       "0", false, false, true);
   swingMenu->addEntry(SCALE_HERE, "scaleHere", "Scale deltas (also) when applying curves",
                       "no", false, false, false);
   swingMenu->addEntry(TRANS_POWER, "transPower", "a in power (1 + a(1 - u))/u",
                       "0.0", false, false, false);
   swingMenu->setPostamble(IRutil::DASHLINE);

   pre = "Edit any parameter(s), or enter Elo rating to set central-fit parameters from it.";
   paramsMenu->setPreamble(pre);
   paramsMenu->addEntry(GO_SPEC, "goSpec", "Save with these values",
                        "", false, false, true);
   paramsMenu->addEntry(CURVE, "curves", "Curve(s)", curvesMenu->nameOf(IRfun::INVEXP) + ": "
                        + curvesMenu->getDescription(IRfun::INVEXP), false, false, true);
   paramsMenu->addEntry(WEIGHT_FN, "weightFn", "Weight function", weightsMenu->nameOf(IRfun::UNITWTS) + ": "
                        + weightsMenu->getDescription(IRfun::UNITWTS), false, false, false);
   //paramsMenu->addEntry(ARG_FORMATION, "argFormation", "Power c out, in, or proxy?", "in",
                        //false, false, false);
   paramsMenu->addEntry(RATING, "rating", "(Set from) Intrinsic Rating", "0000", false, false, true);
   paramsMenu->addEntry(SET_FROM_STRING, "setFromString", "Set parameters from string",
                        "", false, false, false);
   paramsMenu->addEntry(TRIALSPEC_NAME, "specName", "TrialSpec name",
                        "E0000", false, false, false);
   paramsMenu->addEntry(S_VALUE, "sValue", "Sensitivity s value", "0.050", false, false, true);
   paramsMenu->addEntry(C_VALUE, "cValue", "Consistency c value", "0.600", false, false, false);
   paramsMenu->addEntry(D_VALUE, "dValue", "Depth d value", "12.5", false, false, false);
   paramsMenu->addEntry(V_VALUE, "vValue", "Peak-variation v value", "0.050", false, false, false);
   paramsMenu->addEntry(A_VALUE, "aValue", "Swing-minus power a", "1.000", false, false, false);
   paramsMenu->addEntry(HM_VALUE, "hmValue", "Eval/swing-minus hm scaler", "1.000", false, false, false);
   paramsMenu->addEntry(HP_VALUE, "hpValue", "Eval/swing-plus hp scaler", "1.000", false, false, false);
   paramsMenu->addEntry(B_VALUE, "bValue", "Swing-plus power b in ab", "1.000", false, false, false);
   paramsMenu->addEntry(SLIDES, "slides", "Slide by another curve?", "no", false, false, true);
   paramsMenu->addEntry(SLIDE_CURVE, "slideCurve", "Curve to slide into at tail",
                        curvesMenu->nameOf(IRfun::FOLDEDLOGISTIC), false, false, false);
   paramsMenu->addEntry(SLIDE_LO, "slideLo", "Point where sliding starts", "0.00", false, false, false);
   paramsMenu->addEntry(SLIDE_MID, "slideMid", "Point where curves are equal", "2.00", false, false, false);
   paramsMenu->addEntry(SLIDE_HI, "slideHi", "Point where slider takes over", "5.00", false, false, false);
   paramsMenu->addEntry(SLIDE_POWER, "slidePower", "1 for linear, > 1 for sigmoid", "1.0", false, false, false);
   paramsMenu->addEntry(SLIDE_PROP, "slideProp", "Maximum proportion of slide curve", "1.0", false, false, false);
   paramsMenu->addEntry(MEMO_STEP, "memoStep",
                      "Memoize curve values every p.nn",
                      "0.010", false, false, true);
   paramsMenu->addEntry(MEMO_LIMIT, "memoLimit",
                      "Memoize values up to p.nn",
                      "0.00", false, false, false);
   paramsMenu->addEntry(SHOW_STEP, "showStep",
                      "Display curve values every p.nn",
                      "0.01", false, false, true);
   paramsMenu->addEntry(SHOW_LIMIT, "showLimit",
                      "Display values up to p.nn",
                      "3.00", false, false, false);
   paramsMenu->addEntry(SHOW_CURVE, "showCurve",
                      "Show values on screen",
                      "", false, false, false);



   //paramsMenu->addEntry(SPEC_JUDGMENT_DEPTH, "specJudgmentDepth", "Judgment depth (0 = turn max)",
                        //"0", false, false, true);
   pre = string("Change setting or enter ")
            + IRutil::itoa(GO_SPEC) + "/" + paramsMenu->nameOf(GO_SPEC)
            + " to save.";

   paramsMenu->setPostamble(IRutil::DASHLINE + string("\n") + pre);

   pre = "Choose model for first-line probability p and curve value a_i:"; 
   modelsMenu->setPreamble(pre); 
   modelsMenu->addEntry(Models::SHARES, "Shares", "Pr[move i] = p*a_i"); 
   modelsMenu->addEntry(Models::POWER_SHARES, "PowerShares", "Pr[move i] = p^{1/a_i}"); 
 
   outsMenu->setPreamble("Active output streams and files."); 
   outsMenu->addEntry("cout", ""); 
 
   compareMenu->setPreamble("Choose comparison:"); 
   compareMenu->addEntry(LEQ, "leq", "Turn value <= #"); 
   compareMenu->addEntry(EQ, "eq", "Turn value == #"); 
   compareMenu->addEntry(GEQ, "geq", "Turn value >= #"); 
 
   mainMenu->addEntry(NEW_TRIAL, "newTrial", "Define New Trial", 
                      "", false, false, true);   //true puts dashes above 
   mainMenu->addEntry(CHANGE_TRIAL, "changeTrial", 
                      "Change (equation model of) Focus Trial"); 
   mainMenu->addEntry(SHOW_TRIAL, "showTrial", 
                      "Show Active Trial and Filters"); 
   mainMenu->addEntry(LOAD_DELTA_SCALE, "deltaScale", string("Delta Scale---") 
                      + "Load or Define New"); 
   mainMenu->addEntry(ADD_TURNS, "addTurns", "Add More Game Turns"); 
   mainMenu->addEntry(CLEAR_TURNS, "clearTurns", 
                      "Clear Turns And Filtered Tuples"); 
 
   mainMenu->addEntry(NEW_FILTER, "newFilters", "Define New Move Filter", 
                      "", false, false, true);  //novel, so dashes above 
   mainMenu->addEntry(ATTACH_FILTERS, "attach", 
                      "Attach Filter(s) to Focus Trial"); 
   mainMenu->addEntry(DETACH_FILTERS, "detach", 
                      "Detach Filter(s) from Focus Trial"); 
   mainMenu->addEntry(CLEAR_FILTERS, "clearFilters", 
                      "Clear Filters from Focus Trial"); 
   mainMenu->addEntry(HIDE_FILTERS, "hideFilters", 
                      "Hide Filters"); 
   mainMenu->addEntry(NEW_SELECTOR, "newSelectors", "Define New Move Selector",
                      "", false, false, true);  //novel, so dashes above
   mainMenu->addEntry(TOGGLE_SELECTORS, "toggleSelectors", "Toggle Move Selector");
 
   mainMenu->addEntry(NEW_TRIAL_SPEC, "newTrialSpec", 
                      "Define and Load New Trial Spec", 
                      "", false, false, true); 
   mainMenu->addEntry(LOAD_TRIAL_SPEC, "loadTrialSpec", 
                      "Show Active Trial Specs and Load One"); 
   mainMenu->addEntry(HIDE_SPECS, "hideSpecs", 
                      "Hide Trial Specs"); 
 
   //mainMenu->addEntry(PERCFIT_TRIAL, "percfitTrial", 
                      //"Run Percentile Fit on Focus Trial", 
                      //"", false, false, true); 
   mainMenu->addEntry(RUN_FIT, "runFit", "Run Fit to Find Best (s,c,...)",
                      "", false, false, true); 
   mainMenu->addEntry(PERF_TEST, "perfTest", 
                      "Run Performance Test on Focus Trial"); 
   //mainMenu->addEntry(PERF_TEST_ALL, "perfTestAll", 
                      //"Run Perf. Test on All Active TrialSpecs"); 
 
   mainMenu->addEntry(ADD_OUTPUT_FILE, "addOutputFile", "Attach Output File", 
                      "", false, false, true); 
   mainMenu->addEntry(CLOSE_OUTPUT_FILE, "closeOutputFile", 
                      "Close Output File"); 
   mainMenu->addEntry(READ_COMMANDS, "readCommands", 
                      "Process Commands from File"); 
   //mainMenu->addEntry(RUN_DEMO, "runDemo", "Run Demo"); 
   mainMenu->addEntry(QUIT, "quit", "Quit"); 
 
   pre = "Choose one of the following filter options:"; 
   filtersMenu->setPreamble(pre); 
   filtersMenu->addEntry(NO_FILTER, "done", 
                         "Cancel, or stop adding filter disjuncts."); 
   filtersMenu->addEntry(EXISTING_FILTER, "existingFilter", 
                         "Load an already-defined filter."); 
   filtersMenu->addEntry(FN_FILTER, "FnFilter", 
                         "Filter with pre-written function"); 
   filtersMenu->addEntry(NOT_FILTER, "NotFilter", 
                         "NOT of another filter (load or define new)"); 
   filtersMenu->addEntry(AND_FILTER, "AndFilter", 
                         "AND of other filters (load or define new)"); 
   filtersMenu->addEntry(OR_FILTER, "OrFilter", 
                         "OR of other filters (load or define new)"); 
 
   filtersMenu->addEntry(PLAYER_IS, "PlayerIs", 
                         "Games involving player (both sides' moves)", 
                         "", false, false, true); 
   filtersMenu->addEntry(PLAYER_IS_ONE_OF, "PlayerIsOneOf", 
                         "Games involving players (both sides' moves)"); 
   filtersMenu->addEntry(PLAYER_IS_ON_MOVE, "PlayerToMove", 
                         "Player on move is..."); 
   filtersMenu->addEntry(PLAYER_ON_MOVE_IS_ONE_OF, "PlayersToMove", 
                         "Player on move is one of..."); 
   filtersMenu->addEntry(ON_MOVE_FACING, "OnMoveFacing", 
                         "Game turns facing player..."); 
   filtersMenu->addEntry(ON_MOVE_FACING_ONE_OF, "OnMoveFacingOneOf", 
                         "Game turns facing one of..."); 
   filtersMenu->addEntry(WHITE_IS, "WhiteIs", 
                         "White is... (both sides' moves)"); 
   filtersMenu->addEntry(BLACK_IS, "BlackIs", 
                         "Black is... (both sides' moves)"); 
 
   filtersMenu->addEntry(EVENT_IS, "EventIs", "Event is...", "", 
                         false, false, true); 
   filtersMenu->addEntry(DATE_IS, "DateIs", "Date is (Arena format)..."); 
   filtersMenu->addEntry(EVENT_DATE_IS, "EventDateIs", "Event date is (Arena format)...");
   filtersMenu->addEntry(YEAR_IS, "YearIs", "Year is (4-digits)..."); 
 
   filtersMenu->addEntry(WHITE_ELO, "WhiteElo", "White Elo is at least / at most...", "",
                         false, false, true);
   filtersMenu->addEntry(BLACK_ELO, "BlackElo", "Black Elo is at least / at most...");
   filtersMenu->addEntry(PLAYER_ELO, "PlayerElo", 
                         "Elo of player to move is at least / at most...");
   filtersMenu->addEntry(ELO_DIFF, "EloDiff",
                         "Player Elo - opponent Elo is at least / at most...");
   filtersMenu->addEntry(ELO_DIFF_WITHIN, "EloDiffWithin", "Difference of Elos is within...");

   filtersMenu->addEntry(EVAL_WHITE, "EvalWhite", 
                         "Eval to White is...", "", false, false, true); 
   filtersMenu->addEntry(PREV_EVAL_WHITE, "PrevEvalWhite", 
                         "Previous eval to White is..."); 
   filtersMenu->addEntry(EVAL_WITHIN, "EvalWithin", 
                         "Evaluation is within..."); 
   filtersMenu->addEntry(PREV_EVAL_WITHIN, "PrevEvalWithin", 
                         "Eval of previous turn is within..."); 
   filtersMenu->addEntry(NEXT_EVAL_WITHIN, "NextEvalWithin",
                         "Eval of next turn is within...");
   filtersMenu->addEntry(TURN_NO, "MoveNo", 
                         "Move number is at least / at most..."); 
   filtersMenu->addEntry(NUM_LEGAL_MOVES, "NumLegalMoves", 
                         "Number of legal moves is at least / at most..."); 
   filtersMenu->addEntry(DELTA_I, "DeltaIs", 
                         "Delta[i...] is at least / at most..."); 
   filtersMenu->addEntry(DELTA_DIFF, "DeltaDiff", 
                         "Delta[i] - Delta[j] is at least / at most..."); 
   filtersMenu->addEntry(SECOND_DELTA, "SecondDelta", 
                         "Next-best move is inferior by at least / at most..."); 
   filtersMenu->addEntry(DELTA_N, "LastDeltaIs", 
                         "Nth-best move is inferior by at least / at most..."); 
 
   filtersMenu->addEntry(REP_COUNT, "RepCount", "Turn not in repeating sequence",
                         "", false, false, true);

   filtersMenu->addEntry(PLAYED_MOVE_INDEX, "PlayedMoveIndex", 
                         "Played Move Index is at least / at most..."); 
   filtersMenu->addEntry(PLAYED_MOVE_IS_SELECTED, "PlayedMoveSelected",
                         "Played move is selected by given MoveSelector");
   filtersMenu->addEntry(ENGINE_MOVE_IS_SELECTED, "EngineMoveSelected",
                         "Engine move is selected by given MoveSelector");
   filtersMenu->addEntry(EQUAL_TOP_MOVE_IS_SELECTED, "EqualTopMoveSelected",
                         "Equal top move is selected by given MoveSelector");
   filtersMenu->addEntry(SOME_MOVE_IS_SELECTED, "SomeLegalMoveSelected",
                         "Some move is selected by given MoveSelector");
   filtersMenu->addEntry(FALLOFF, "Falloff",  
                         "Delta of Played Move is at least / at most..."); 
   filtersMenu->addEntry(WEIGHT, "WeightIs", 
                         "Turn weight is at least / at most..."); 
   filtersMenu->addEntry(PROB_I, "ProbIs", 
                         "Est. prob. of move[i...] is at least / at most..."); 

   pre = "Choose one of the following move-selector options:";
   selectorsMenu->setPreamble(pre);
   selectorsMenu->addEntry(NO_SELECTOR, "done",
                         "Cancel, or stop adding selector disjuncts.");
   selectorsMenu->addEntry(EXISTING_SELECTOR, "existingSelector",
                         "Load an already-defined selector.");
   selectorsMenu->addEntry(FN_SELECTOR, "FnSelector",
                         "Selector with pre-written function");
   selectorsMenu->addEntry(NOT_SELECTOR, "NotSelector",
                         "NOT of another selector (load or define new)");
   selectorsMenu->addEntry(AND_SELECTOR, "AndSelector",
                         "AND of other selectors (load or define new)");
   selectorsMenu->addEntry(OR_SELECTOR, "OrSelector",
                         "OR of other selectors (load or define new)");
   selectorsMenu->addEntry(MAP_SELECTOR, "MappedMoveSelector",
                         "Map of turns to moves from other selector");
   selectorsMenu->addEntry(MOVE_EVAL_SELECTOR, "MoveEvalSelector",
                         "Move eval at depth d is <=> threshold");
   selectorsMenu->addEntry(MOVE_RAW_DELTA_SELECTOR, "MoveRawDeltaSelector",
                         "Move delta at depth d is <=> threshold");
   selectorsMenu->addEntry(MOVE_RAW_SWING_SELECTOR, "MoveRawSwingSelector",
                         "Move raw swing up to depth D is <=> threshold");
   selectorsMenu->addEntry(MOVE_SCALED_SWING_SELECTOR, "MoveScaledSwingSelector",
                         "Move scaled swing up to depth D is <=> threshold");

/*
   enum METHOD {
      VWALK, GSL_NM4, GSL_NM6, GSL_NM7, GSL_SIM_ANN, CMINPACK_LMDIF, CMINPACK_HYBRD, NUM_METHODS
   };
   enum SKED {
      UNIT_SKED, SQRT_SKED, LIN_SKED, QUAD_SKED, INVVAR_SKED, NUM_SKEDS
   };
*/
   methodMenu->setPreamble("Choose fitting method, HYBRD only for exact solutions.");
   methodMenu->addEntry(Minimizer::VWALK, "vwalk", "Two-dimensional bitonic search");
   methodMenu->addEntry(Minimizer::GSL_NM4, "GSLNMold", "GSL Nelder-Mead, old version");
   methodMenu->addEntry(Minimizer::GSL_NM6, "GSLNM", "GSL Nelder-Mead, newer version");
   methodMenu->addEntry(Minimizer::GSL_NM7, "GSLNMrand", "GSL Nelder-Mead with random orientations");
   methodMenu->addEntry(Minimizer::GSL_SIM_ANN, "GSLSA", "GSL Simulated Annealing");
   methodMenu->addEntry(Minimizer::CMINPACK_LMDIF, "LMDIF", "Levenberg-Marquardt difference method (not yet working)");
   methodMenu->addEntry(Minimizer::CMINPACK_LMDIF1, "LMDIF1", "Levenberg-Marquardt, simplified call (not yet working)");
   methodMenu->addEntry(Minimizer::CMINPACK_HYBRD, "HYBRD", "Hybrid Powell difference method (not yet working)");
   methodMenu->addEntry(Minimizer::CMINPACK_HYBRD1, "HYBRD1", "Hybrid Powell, simplified call (not yet working)");
   methodMenu->addEntry(Minimizer::MPFIT_LMDIF, "MPFITLM", "Levenberg-Marquardt, MPFIT version (not yet working)");

   skedMenu->setPreamble("Choose weighting scheme for index values");
   skedMenu->addEntry(Minimizer::UNIT_SKED, "unitsked", "Unit weight for each index");
   skedMenu->addEntry(Minimizer::SQRT_SKED, "sqrtsked", "Square-root weights after squaring");
   skedMenu->addEntry(Minimizer::LIN_SKED, "linsked", "Linear weights after squaring (sqrt before)");
   skedMenu->addEntry(Minimizer::QUAD_SKED, "quadsked", "Linear weights before squaring");
   skedMenu->addEntry(Minimizer::INVVAR_SKED, "invvarsked", "Weighted by inverse variance");
   skedMenu->addEntry(Minimizer::SQRT_ALLWT, "sqrtallwt", "Square-root all-weight after squaring");
   skedMenu->addEntry(Minimizer::LIN_ALLWT, "linallwt", "Linear all-weight after squaring (sqrt before)");
   skedMenu->addEntry(Minimizer::QUAD_ALLWT, "quadallwt", "Linear all-weight before squaring");



   fitMenu->addEntry(Minimizer::PERCFIT, "percFit", 
                     "Fit focus trial's percentiles", "0.00", false, false, true); 
   fitMenu->addEntry(Minimizer::ML, "maxLikely", 
                     "Maximum likelihood of played moves", 
                     "0.00", false, false, false);   //not starred, no dashes above 
   fitMenu->addEntry(Minimizer::BINS, "bins",
                     "Fit frequency in percentile bins", "0.00",
                     false, false, false);
   fitMenu->addEntry(Minimizer::FALLOFF, "falloff",
                     "Fit scaled falloff",
                     "1.00", false, true, true);
   fitMenu->addEntry(Minimizer::FIRST_LINE, "firstLine",
                     "Fit first index",
                     "1.00", false, true, false);
   fitMenu->addEntry(Minimizer::SECOND_LINE, "secondLine",
                     "Fit second index",
                     "1.00", false, true, false);
   fitMenu->addEntry(Minimizer::THIRD_LINE, "thirdLine",
                     "Fit third index",
                     "0.00", false, false, false);
   fitMenu->addEntry(Minimizer::ETV, "equalTopValue",
                     "Fit equal-top value moves to index",
                     IRutil::itoa(IRutil::NCUTOFF) + ": 1.00", false, true, false);
   fitMenu->addEntry(Minimizer::INDEX_FIT, "indexFit",
                     "Fit indices [m thru n] by sked",
                     "[1,4] sqrtsked: 0.00", false, false, false);
   fitMenu->addEntry(Minimizer::INDEX_DIFF, "indexDiff",
                     "Fit +- diff in [m thru n] by sked",
                     "[1,4] sqrtsked: 0.0", false, false, false);
   fitMenu->addEntry(Minimizer::ERROR1, "error1",
                     "Fit error X.mm to Y.nn",
                     "[0.51, 1.00] sqrtallwt: 0", false, false, false);
   fitMenu->addEntry(Minimizer::ERROR2, "error2",
                     "Fit error X.mm to Y.nn",
                     "[1.01, 999.99] sqrtallwt: 0", false, false, false);


   fitMenu->addEntry(MIN_METHOD, "minMethod", "Minimization method",
                     methodMenu->nameOf(Minimizer::GSL_NM6), true, false, true);
   fitMenu->addEntry(FUNNEL_FACTORS, "funnelFactors", "mu in f + mu*g, map mu to mu/div",
                     "0.0 div by 2.0", true, false, false);
   fitMenu->addEntry(START_SPEC, "startSpec", "", "", false, false, false);


   fitMenu->addEntry(S_BOUNDS, "sBounds", "Bounds for s are", 
                     "0.0001 .. 1.024", false, true, true); 
   fitMenu->addEntry(C_BOUNDS, "cBounds", "Bounds for c are",
                     "0.0000 .. 10.0000", false, true, false); 
   fitMenu->addEntry(D_BOUNDS, "dBounds", "Bounds for d are",
                     //"0.000 .. 30.0", true, false, false);
                     //"-20.5 .. 50.5", false, false, false);
                     "12.5 .. 12.5", false, false, false);
   fitMenu->addEntry(V_BOUNDS, "vBounds", "Bounds for v are",
                     //"0.000 .. 10.0", true, false, false);
                     //"-1.0000 .. 1.0000", false, false, false);
                     "0.05 .. 0.05", false, false, false);
   fitMenu->addEntry(A_BOUNDS, "aBounds", "Bounds for a are",
                     //"0.0000 .. 10.0000", false, false, false);
                     "0.0000 .. 10.0000", false, true, false);
   fitMenu->addEntry(HM_BOUNDS, "hmBounds", "Bounds for hm are",
                     "0.0000 .. 100.0000", false, true, false);
   fitMenu->addEntry(HP_BOUNDS, "hpBounds", "Bounds for hp are",
                     "1.0000 .. 1.0000", false, false, false);
   fitMenu->addEntry(B_BOUNDS, "bBounds", "Bounds for b are",
                     "1.0000 .. 1.0000", false, false, false);

   //fitMenu->addEntry(FIT_JUDGMENT_DEPTH, "fitJudgmentDepth", "(0 for turn max depth)",
                     //"0", false, false, false);
   fitMenu->addEntry(MAX_ITERATIONS, "maxIterations", "max # of iterations",
                     "2000", false, false, true);
   fitMenu->addEntry(NUM_RETRIES, "numRetries", "= # tries at same temp in SA",
                     "50", false, false, false);
   fitMenu->addEntry(PRECISION_X, "xprec", "Precision on x-axes",
                     "0.0001", false, false, false);
   fitMenu->addEntry(PRECISION_Y, "yprec", "Precision of function values",
                     "0.0001", false, false, false);
   fitMenu->addEntry(PRECISION_G, "gprec", "Precision of other geometry",
                     "0.00001", false, false, false);
   //fitMenu->addEntry(WALKOUT, "walkOut", "Walk out from spec point?",
                     //"yes", true, false, false);
   fitMenu->addEntry(WALKOUT_STEP, "walkOutStep", "Initial walkout step",
                     "0.01", true, false, false);
   fitMenu->addEntry(EPSFCN, "epsfcn", "Minpack epsfcn, or initemp/10000 in SA",
                     "0.001", true, false, false);
   fitMenu->addEntry(GO, "go", "Run Fit With These Settings!", "",
                     false, false, true);

   fitMenu->addEntry(BOOT_NUM, "bootNum", "Number of Bootstrap Trials", "0",
                     true, false, false);
   fitMenu->addEntry(KEEP_SPEC_IN_BOOT, "keepSpecInBoot", "Keep regressed spec in bootstrap?", "no",
                     false, false, false);
   fitMenu->addEntry(VERBOSE, "verbose", "Verbose output?", 
                     "yes", true, false, false); 

   pre = "Enter number to change setting, " 
            + IRutil::itoa(GO + fitMenu->getFDI()) 
            + "/" + fitMenu->nameOf(GO) + " to run fit on focus trial."; 
   fitMenu->setPreamble(pre); 



   testMenu->addEntry(GO_TEST, "goTest", "Run test with these settings", 
                      "", true, false, true);
   testMenu->addEntry(USE_FOCUS_SPEC, "useFocusSpec", "Use current TrialSpec?",
                      "yes", true, false, true);
   testMenu->addEntry(MISS_IS_MOVE_NPLUSONE, "missIsNP1",
                      "Treat miss as Move N+1?", "no", true, false, false);
   testMenu->addEntry(FORCE_UNIT_WEIGHTS, "forceUnitWeights", 
                      "Override spec to use unit weights?",
                      "no", true, false, false);
   testMenu->addEntry(SHOW_UNWEIGHTED, "showUnweighted",
                      "Duplicate output to show unit weights?",
                      "no", false, false, true);
   testMenu->addEntry(SHOW_UNSCALED, "showUnscaled",
                      "Duplicate output with unscaled deltas?",
                      "no", false, false, false);
   testMenu->addEntry(NUM_INDICES, "numIndices", 
                      "Number of Kth-best lines to show",
                      "15", true, false, true);
   testMenu->addEntry(EQUAL_TOP_INDEX_LIMIT, "equalTopIndexLimit",
                      "Index limit on equal-top-value moves",
                      IRutil::itoa(IRutil::NCUTOFF), false, false, false);
   testMenu->addEntry(NUM_BOOTSTRAP_TRIALS, "numBootTests",
                      "Number of bootstrap trials", 
                      "0", true, false, true);
   testMenu->addEntry(NUM_RESAMPLE_TRIALS, "numResampleTrials",
                      "Number of resampling trials",
                      "0", true, false, true);
   testMenu->addEntry(NUM_RESAMPLE_GAMES, "numResampleGames",
                      "Number of games to resample",
                      "9", false, false, false);
   testMenu->addEntry(NUM_RESAMPLE_TURNS, "numResampleTurns",
                      "Number of turns to resample",
                      "0", false, false, false);
   testMenu->addEntry(ONE_PLAYER_PER_GAME, "onePlayerPerGame",
                      "Choose one side of each game?",
                      "yes", false, false, false);
   testMenu->addEntry(ZMM_FILE, "zmmFile", "File for move-match z-scores",
                      "zmm.txt", false, false, true);
   testMenu->addEntry(ZEV_FILE, "zevFile", "File for equal-value match z-scores",
                      "zev.txt", false, false, false);
   testMenu->addEntry(ZAD_FILE, "zadFile", "File for average-difference z-scores",
                      "zad.txt", false, false, false);
   testMenu->addEntry(MOVES_FILE, "movesFile", "File for probability data on moves",
                      "movesDump.txt", false, false, false);
   testMenu->addEntry(MOVES_LIMIT, "movesLimit", "Limit on # of positions written",
                      "1000", false, false, false);
   testMenu->addEntry(SHOW_INDIVIDUAL_MOVES, "showMoves",
                      "List each move and its probability?",
                      "no", false, false, false);

   pre = string("Enter number to change setting, ")
            + IRutil::itoa(GO_TEST) + "/" + testMenu->nameOf(GO_TEST)
            + " to run statistical tests.";
   testMenu->setPreamble(pre);
   testMenu->setPostamble(IRutil::DASHES);

 
   pre = "Choose demo to run, or " + IRutil::itoa(EnumMenu::CANCEL_CHOICE) 
            + "/" + demosMenu->CANCEL_NAME + " to cancel."; 
   demosMenu->setPreamble(pre); 
   demosMenu->addEntry(Demos::ELISTA_DEMO, "elistaDemo", 
                       "Test Elista 2006 games 1-6 with focus trial-spec."); 
   demosMenu->addEntry(Demos::SAN_LUIS_MEXICO_DEMO, "sanLuisMexicoDemo", 
                       "Topalov at San Luis '05 vs. Anand at Mexico '07."); 
   demosMenu->addEntry(Demos::SWING_DEMO, "swingDemo", 
                       "Test 'swing' effects of unprobed earlier ply depths."); 
   demosMenu->addEntry(Demos::DEMORALIZATION_DEMO, "demoralizationDemo", 
                       "Do we play better when ahead, worse when behind?"); 
} 
 
 
void Ensemble::interactFromStream(istream* istrp, const string& matchString = "") { 
                                  //bool suppressQuit) { 
   if (justUsedCin->status && istrp != &cin) { 
      //IRutil::finishLine(cin); 
      justUsedCin->status = false; 
   } 
   string line; 
   INP = istrp; 
   if (matchString != "") { 
      getline(*INP,line);   //not logged 
      while ((!IRutil::delimitedMatch(matchString,line)) && (!atEOF())) { 
         getline(*INP,line); 
      } 
   } 
   //last line we got should be a comment line, so we can throw it away 
   string cal = IRutil::getTime(); 
   // loggedWrite(IRutil::IRCOMMENT + "Reading commands at time " + cal + " (" 
   //                       + IRutil::getRawTime() + ")\n"); 
 
   bool interacting = true; 
   while (interacting) { 
      int choice = mainMenu->readChoice(); 
      if (mainMenu->success(choice) && (choice != QUIT)) { 
		  //TAMAL
         //try { 
            doMainMenuItem(choice); 
        // } catch (exception &e) { 
       //     cerr << "Error " << e.what() << endl; 
      //   } 
      } else if (choice == QUIT || choice == EnumMenu::STREAM_END_CHOICE) { 
         interacting = false; 
      } else { 
         cout << "Choose " + IRutil::itoa(QUIT + mainMenu->getFDI()) + "/" 
                           + mainMenu->nameOf(QUIT) 
                           + " if you wish to quit." << endl; 
      } 
   } 
   (*INP).clear(); 
   (*INP).ignore(numeric_limits<streamsize>::max(),'\n'); 
   INP = &cin; 
} 
 
 
void Ensemble::interact() { 
   interactFromStream(INP, ""); 
} 
 
 
 
double Ensemble::readS() const { 
   double s; 
   if (usingCin()) { 
      cout << "Enter sensitivity s or enter Elo rating: "; 
   } 
   loggedRead(s,false); 
   return s; 
} 
 
double Ensemble::readC() const { 
   double c; 
   if (usingCin()) { 
      cout << "Enter consistency c: "; 
   } 
   loggedRead(c,false); 
   return c; 
} 

double Ensemble::readD() const {
   double d;
   if (usingCin()) {
      cout << "Enter (central) depth d: ";
   }
   loggedRead(d,false);
   return d;
}

double Ensemble::readV() const {
   double v;
   if (usingCin()) {
      cout << "Enter var^n coeff v: ";
   }
   loggedRead(v,false);
   return v;
}

double Ensemble::readA() const {
   double a;
   if (usingCin()) {
      cout << "Enter (negative) swing exponent a: ";
   }
   loggedRead(a,false);
   return a;
}

double Ensemble::readHM() const {
   double hm;
   if (usingCin()) {
      cout << "Enter minus-eval/swing multiplier hm: ";
   }
   loggedRead(hm,false);
   return hm;
}

double Ensemble::readHP() const {
   double hp;
   if (usingCin()) {
      cout << "Enter plus-eval/swing multiplier hp: ";
   }
   loggedRead(hp,false);
   return hp;
}

double Ensemble::readB() const {
   double b;
   if (usingCin()) {
      cout << "Enter (positive) swing/slack power/term a: ";
   }
   loggedRead(b,false);
   return b;
}

/*
size_t Ensemble::readJD() const {
   size_t jd;
   if (usingCin()) {
      cout << "Enter data depth to use for judgments, 0 for turn max: ";
   }
   loggedRead(jd,false);
   return jd;
}
*/




pair<int,int> Ensemble::readBounds() const {
   int lo,hi;
   if (usingCin()) {
      cout << "Enter lo and hi bounds for depth, as ints sep by space: ";
   }
   loggedRead(lo,false);
   loggedRead(hi,false);
   return pair<int,int>(lo,hi);
}

void Ensemble::resetParamsMenu() {
   paramsMenu->setStatus(TRIALSPEC_NAME, focusTrial->getSpec().name);
   paramsMenu->setStatus(S_VALUE, IRutil::ftoa(focusTrial->getSpec().s));
   paramsMenu->setStatus(C_VALUE, IRutil::ftoa(focusTrial->getSpec().c));
   paramsMenu->setStatus(D_VALUE, IRutil::ftoa(focusTrial->getSpec().d));
   paramsMenu->setStatus(V_VALUE, IRutil::ftoa(focusTrial->getSpec().v));
   paramsMenu->setStatus(A_VALUE, IRutil::ftoa(focusTrial->getSpec().a));
   paramsMenu->setStatus(HM_VALUE, IRutil::ftoa(focusTrial->getSpec().hm));
   paramsMenu->setStatus(HP_VALUE, IRutil::ftoa(focusTrial->getSpec().hp));
   paramsMenu->setStatus(B_VALUE, IRutil::ftoa(focusTrial->getSpec().b));
   //paramsMenu->setStatus(SPEC_JUDGMENT_DEPTH, IRutil::ftoa(focusTrial->getSpec().judgmentDepth));
}

 
 
TrialSpec Ensemble::makeTrialSpec() const { 
   IRfun::CURVES curveChoice = IRfun::CURVES(IRfun::INVEXP); 
   IRfun::WEIGHTS weightChoice = IRfun::WEIGHTS(IRfun::UNITWTS); 
   map<IRfun::CURVES,double> curveBook;
   string curvesStr = "";
   for (int i = 0; i < IRfun::CURVES(IRfun::NUM_CURVES); i++) {
      double mul = IRutil::extractDoubles(curvesMenu->getStatus(IRfun::CURVES(i)))[0];
      IRfun::CURVES curve = IRfun::CURVES(i);
      if (mul > 0) {
         curveBook[curve] = mul;
         curvesStr += "(" + IRfun::curveNames[curve] + ":" + IRutil::ftoa(mul) + ")";
      }
   }
   map<IRfun::CURVES,double>::const_iterator cbit = curveBook.begin();
   map<IRfun::CURVES,double>::const_iterator cbite = curveBook.end();
   IRfun::CURVES curve;
   double mul;
   paramsMenu->setStatus(CURVE, curvesStr);
   double floatItem;
   double s,c,d,v,a,hm,hp,b;
   vector<double> vals;
   vector<double>::const_iterator vit, vite;
   bool slide = IRutil::isYes(paramsMenu->getStatus(SLIDES));
   IRfun::CURVES slideCurveChoice = IRfun::CURVES(curvesMenu->indexOf(paramsMenu->getStatus(SLIDE_CURVE)));
   double slideLo = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_LO))[0];
   double slideMid = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_MID))[0];
   double slideHi = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_HI))[0];
   double slidePower = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_POWER))[0];
   double slideProp = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_PROP))[0];
   double memoStep = IRutil::extractDoubles(paramsMenu->getStatus(MEMO_STEP))[0];
   double memoLimit = IRutil::extractDoubles(paramsMenu->getStatus(MEMO_LIMIT))[0];
   double showStep = IRutil::extractDoubles(paramsMenu->getStatus(SHOW_STEP))[0];
   double showLimit = IRutil::extractDoubles(paramsMenu->getStatus(SHOW_LIMIT))[0];
   double deltaCap = focusTrial->getScale()->getDeltaCap();
   TrialSpec ts;

   IRutil::nullstream nullstrm;
   ostream& ccout = (usingCin()) ? cout : nullstrm;
   int rating; //could be negative...
   string argFormation, name, ans;
   string curveName;
   string weightsName;
   bool setFromRating = false;
   int choice, choice2, intItem;
   bool entering = true;
   bool innerEntering;
   while (entering) {
      choice = paramsMenu->readChoice();
      switch(choice) {
       case GO_SPEC:
         s = atof(paramsMenu->getStatus(S_VALUE).c_str());
         c = atof(paramsMenu->getStatus(C_VALUE).c_str());
         d = atof(paramsMenu->getStatus(D_VALUE).c_str());
         v = atof(paramsMenu->getStatus(V_VALUE).c_str());
         a = atof(paramsMenu->getStatus(A_VALUE).c_str());
         hm = atof(paramsMenu->getStatus(HM_VALUE).c_str());
         hp = atof(paramsMenu->getStatus(HP_VALUE).c_str());
         b = atof(paramsMenu->getStatus(B_VALUE).c_str());
         name = IRutil::trim(paramsMenu->getStatus(TRIALSPEC_NAME));
         //judgmentDepth = atoi(paramsMenu->getStatus(SPEC_JUDGMENT_DEPTH).c_str());
         //argFormation = paramsMenu->getStatus(ARG_FORMATION);
         curveName = paramsMenu->getStatus(CURVE);
         //curveName = IRutil::trim(curveName.substr(0,curveName.find(':')));
         //curveChoice = IRfun::CURVES(curvesMenu->indexOf(curveName));
         curveChoice = cbit->first;
         weightsName = paramsMenu->getStatus(WEIGHT_FN);
         weightsName = IRutil::trim(weightsName.substr(0,weightsName.find(':')));
         weightChoice = IRfun::WEIGHTS(weightsMenu->indexOf(weightsName));
         slide = IRutil::isYes(paramsMenu->getStatus(SLIDES));
         slideCurveChoice = IRfun::CURVES(curvesMenu->indexOf(paramsMenu->getStatus(SLIDE_CURVE)));
         slideLo = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_LO))[0];
         slideMid = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_MID))[0];
         slideHi = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_HI))[0];
         slidePower = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_POWER))[0];
         slideProp = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_PROP))[0];
         memoStep = atof(paramsMenu->getStatus(MEMO_STEP).c_str());
         memoLimit = atof(paramsMenu->getStatus(MEMO_LIMIT).c_str());

         entering = false;
         //TrialSpec ts(curveChoice,s,c,d,v,w,em,ep,a,weightChoice,judgmentDepth,name);
         //return ts;
         break;
       case CURVE:
         choice2 = -1;
         innerEntering = true;
         while (innerEntering && choice2 != -5) {
            choice2 = curvesMenu->readChoice(true, true);  //invokes specialGo in readChoice
            if ((choice2 == -6 || IRfun::INVEXP <= choice2) && choice2 < IRfun::NUM_CURVES) {
               if (usingCin()) {
                  cout << "Enter float weight (0 to exclude): ";
               }
               loggedRead(floatItem, false);
               if (usingCin()) { cout << endl; }
               intItem = (choice2 == -6 ? 0 : choice2);
               curvesMenu->setStatus(intItem, IRutil::ftoa(floatItem));
               if (floatItem != 0) {
                  curvesMenu->setStarred(intItem,true);
               } else {
                  curvesMenu->setStarred(intItem,false);
               }
            }
            innerEntering = (choice2 == -6 || choice2 > 0);  
         }
         curveChoice = IRfun::CURVES(choice2);
         //paramsMenu->setStatus(CURVE, curvesMenu->nameOf(choice2) + ": "
                                    //+ curvesMenu->getDescription(choice2));
         curveBook.clear();
         curvesStr = "";
         for (int i = 0; i < IRfun::CURVES(IRfun::NUM_CURVES); i++) {
            mul = IRutil::extractDoubles(curvesMenu->getStatus(IRfun::CURVES(i)))[0];
            curve = IRfun::CURVES(i);
            if (mul > 0) {
               curveBook[curve] = mul;
            }
         }
         if (curveBook.size() == 0) {
            cerr << "No curve selected, using pure INVEXP." << endl;
            curveBook[IRfun::INVEXP] = 1.0;
         }
         cbite = curveBook.end();
         for (cbit = curveBook.begin(); cbit != cbite; ++cbit) {
            curve = IRfun::CURVES(cbit->first);
            mul = cbit->second;
            curvesStr += "(" + IRfun::curveNames[curve] + ":" + IRutil::ftoa(mul) + ")";
         }
         cbit = curveBook.begin();
         curveChoice = cbit->first;
         paramsMenu->setStatus(CURVE, curvesStr);
         break;
       case WEIGHT_FN:
         choice2 = weightsMenu->readChoice();
         weightChoice = IRfun::WEIGHTS(choice2);
         paramsMenu->setStatus(WEIGHT_FN, weightsMenu->nameOf(choice2) + ": " 
                                        + weightsMenu->getDescription(choice2));
         break;

       case SLIDES:
         if (usingCin()) {
            cout << "Slide to another curve toward the tail? ";
         }
         loggedRead(ans,false);
         ans = IRutil::trim(ans);
         paramsMenu->setStatus(SLIDES,ans);
         break;
       case SLIDE_CURVE:
         if (usingCin()) {
            cout << "Choose slide curve---one-shot choice only: " << endl;
         }
         choice2 = -1;
         while (IRfun::INVEXP > choice2 || choice2 >= IRfun::NUM_CURVES) {
            choice2 = curvesMenu->readChoice(true, true);
         }
         slideCurveChoice = IRfun::CURVES(choice2);
         paramsMenu->setStatus(SLIDE_CURVE, curvesMenu->nameOf(slideCurveChoice));
         break;
       case SLIDE_LO:
         if (usingCin()) {
            cout << "Enter starting point of slide: ";
         }
         loggedReadBounded(slideLo,0,deltaCap,false);
         paramsMenu->setStatus(SLIDE_LO, IRutil::ftoa(slideLo));
         break;
       case SLIDE_MID:
         if (usingCin()) {
            cout << "Enter midpoint of slide: ";
         }
         loggedReadBounded(slideMid,0,deltaCap,false);
         paramsMenu->setStatus(SLIDE_MID, IRutil::ftoa(slideMid));
         break;
       case SLIDE_HI:
         if (usingCin()) {
            cout << "Enter point at which tail curve takes over: ";
         }
         loggedReadBounded(slideHi,slideMid,deltaCap,false);
         paramsMenu->setStatus(SLIDE_HI, IRutil::ftoa(slideHi));
         break;
       case SLIDE_POWER:
         if (usingCin()) {
            cout << "Enter 1 for straight line, > 1 for sigmoid: ";
         }
         loggedRead(slidePower,false);
         paramsMenu->setStatus(SLIDE_POWER, IRutil::ftoa(slidePower));
         break;
       case SLIDE_PROP:
         if (usingCin()) {
            cout << "Enter maximum proportion up to 1 for the slide spec: ";
         }
         loggedReadBounded(slideProp,0,1,false);
         paramsMenu->setStatus(SLIDE_PROP, IRutil::ftoa(slideProp));
         break;

/*
       case ARG_FORMATION:
         if (usingCin()) {
            cout << "Form args as (.+.)^c, (.^c,.^c), or proxy? Enter out, in, or proxy: ";
         }
         loggedRead(argFormation);
         paramsMenu->setStatus(ARG_FORMATION, argFormation);
         break;
*/
       case RATING:
         if (usingCin()) {
            cout << "Enter Elo rating; parameters will match it: ";
         }
         loggedRead(rating);
         paramsMenu->setStatus(RATING, IRutil::itoa(rating));
         s = IRfun::sOfElo(rating);
         c = IRfun::cOfElo(rating);
         d = IRfun::dOfElo(rating);
         v = IRfun::vOfElo(rating);
         a = IRfun::aOfElo(rating);
         hm = IRfun::hmOfElo(rating);
         hp = IRfun::hpOfElo(rating);
         b = IRfun::bOfElo(rating);
         name = "E" + IRutil::itoa(IRutil::round(rating));
         paramsMenu->setStatus(S_VALUE, IRutil::ftoa(s));
         paramsMenu->setStatus(C_VALUE, IRutil::ftoa(c));
         paramsMenu->setStatus(D_VALUE, IRutil::ftoa(d));
         paramsMenu->setStatus(V_VALUE, IRutil::ftoa(v));
         paramsMenu->setStatus(A_VALUE, IRutil::ftoa(a));
         paramsMenu->setStatus(HM_VALUE, IRutil::ftoa(hm));
         paramsMenu->setStatus(HP_VALUE, IRutil::ftoa(hp));
         paramsMenu->setStatus(B_VALUE, IRutil::ftoa(b));
         paramsMenu->setStatus(TRIALSPEC_NAME, name);
         break;
       case SET_FROM_STRING:
         if (usingCin()) {
            cout << "Paste any string with up to 8 numbers: ";
         }
         ans = loggedGetline();
         if (IRutil::trim(ans) == "") { ans = loggedGetline(); }
         vals = IRutil::extractDoubles(ans);
         vit = vals.begin();
         vite = vals.end();
         s = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(S_VALUE).c_str()));
         c = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(C_VALUE).c_str()));
         d = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(D_VALUE).c_str()));
         v = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(V_VALUE).c_str()));
         a = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(A_VALUE).c_str()));
         hm = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(HM_VALUE).c_str()));
         hp = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(HP_VALUE).c_str()));
         b = (vit != vite ? *vit++ : atof(paramsMenu->getStatus(B_VALUE).c_str()));
         paramsMenu->setStatus(S_VALUE, IRutil::ftoa(s));
         paramsMenu->setStatus(C_VALUE, IRutil::ftoa(c));
         paramsMenu->setStatus(D_VALUE, IRutil::ftoa(d));
         paramsMenu->setStatus(V_VALUE, IRutil::ftoa(v));
         paramsMenu->setStatus(A_VALUE, IRutil::ftoa(a));
         paramsMenu->setStatus(HM_VALUE, IRutil::ftoa(hm));
         paramsMenu->setStatus(HP_VALUE, IRutil::ftoa(hp));
         paramsMenu->setStatus(B_VALUE, IRutil::ftoa(b));
         break;
       case TRIALSPEC_NAME:
         if (usingCin()) {
            cout << "Enter descriptive name for TrialSpec: ";
         }
         loggedRead(name, true);  //end line with entered name
         name = IRutil::trim(name);
         paramsMenu->setStatus(TRIALSPEC_NAME, name);
         break;
       case S_VALUE:
         s = readS();
         paramsMenu->setStatus(S_VALUE, IRutil::ftoa(s));
         break;
       case C_VALUE:
         c = readC();
         paramsMenu->setStatus(C_VALUE, IRutil::ftoa(c));
         break;
       case D_VALUE:
         d = readD();
         paramsMenu->setStatus(D_VALUE, IRutil::ftoa(d));
         break;
       case V_VALUE:
         v = readV();
         paramsMenu->setStatus(V_VALUE, IRutil::ftoa(v));
         break;
       case A_VALUE:
         a = readA();
         paramsMenu->setStatus(A_VALUE, IRutil::ftoa(a));
         break;
       case HM_VALUE:
         hm = readHM();
         paramsMenu->setStatus(HM_VALUE, IRutil::ftoa(hm));
         break;
       case HP_VALUE:
         hp = readHP();
         paramsMenu->setStatus(HP_VALUE, IRutil::ftoa(hp));
         break;
       case B_VALUE:
         b = readB();
         paramsMenu->setStatus(B_VALUE, IRutil::ftoa(b));
         break;
       //case SPEC_JUDGMENT_DEPTH:
         //judgmentDepth = readJD();
         //paramsMenu->setStatus(SPEC_JUDGMENT_DEPTH, IRutil::itoa(judgmentDepth));
         //break;
       case MEMO_STEP:
         ccout << "Enter memo interval for curve in p.nn units: ";
         loggedReadBounded(memoStep,0.001,1.0,false);
         paramsMenu->setStatus(choice, IRutil::ftoa(memoStep));
         break;
       case MEMO_LIMIT:
         ccout << "Show curve values up to p.nn: ";
         loggedReadBounded(memoLimit,0.00,10.00,false);
         paramsMenu->setStatus(choice, IRutil::ftoa(memoLimit));
         break;
       case SHOW_STEP:
         ccout << "Enter memo interval for curve in p.nn units: ";
         loggedReadBounded(showStep,0.001,1.0,false);
         paramsMenu->setStatus(choice, IRutil::ftoa(showStep));
         break;
       case SHOW_LIMIT:
         ccout << "Show curve values up to p.nn: ";
         loggedReadBounded(showLimit,0.00,10.00,false);
         paramsMenu->setStatus(choice, IRutil::ftoa(showLimit));
         break;

       case SHOW_CURVE:
         s = atof(paramsMenu->getStatus(S_VALUE).c_str());
         c = atof(paramsMenu->getStatus(C_VALUE).c_str());
         d = atof(paramsMenu->getStatus(D_VALUE).c_str());
         v = atof(paramsMenu->getStatus(V_VALUE).c_str());
         a = atof(paramsMenu->getStatus(A_VALUE).c_str());
         hm = atof(paramsMenu->getStatus(HM_VALUE).c_str());
         hp = atof(paramsMenu->getStatus(HP_VALUE).c_str());
         b = atof(paramsMenu->getStatus(B_VALUE).c_str());
         name = IRutil::trim(paramsMenu->getStatus(TRIALSPEC_NAME));
         curveName = paramsMenu->getStatus(CURVE);
         curveChoice = cbit->first;
         weightsName = paramsMenu->getStatus(WEIGHT_FN);
         weightsName = IRutil::trim(weightsName.substr(0,weightsName.find(':')));
         weightChoice = IRfun::WEIGHTS(weightsMenu->indexOf(weightsName));
         slide = IRutil::isYes(paramsMenu->getStatus(SLIDES));
         slideCurveChoice = IRfun::CURVES(curvesMenu->indexOf(paramsMenu->getStatus(SLIDE_CURVE)));
         slideLo = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_LO))[0];
         slideMid = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_MID))[0];
         slideHi = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_HI))[0];
         slidePower = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_POWER))[0];
         slideProp = IRutil::extractDoubles(paramsMenu->getStatus(SLIDE_PROP))[0];
         memoStep = atof(paramsMenu->getStatus(MEMO_STEP).c_str());
         //memoLimit = atof(paramsMenu->getStatus(MEMO_LIMIT).c_str());
         showStep = atof(paramsMenu->getStatus(SHOW_STEP).c_str());
         showLimit = atof(paramsMenu->getStatus(SHOW_LIMIT).c_str());

         ts = TrialSpec(curveChoice,curveBook,weightChoice,name,s,c,d,v,a,hm,hp,b, //judgmentDepth,
                        slide, slideCurveChoice, slideLo, slideMid, slideHi, slidePower, slideProp,
                        memoStep, 0.0);
         cout << ts.showCurvePoints(showStep,showLimit) << endl;
         break;
       default:
         cout << "Invalid entry; please try again: ";
      }
   }
   //curveBook has been maintained on the fly
   ts = TrialSpec(curveChoice,curveBook,weightChoice,name,s,c,d,v,a,hm,hp,b, //judgmentDepth,
		  slide, slideCurveChoice, slideLo, slideMid, slideHi, slidePower, slideProp,
                  memoStep, memoLimit);
   return ts;
} 
 
 
LineScale* Ensemble::makeDeltaScale() const { 
   LineScale* ls;   //base scale constructed from menu, always a LineScale 
   LineScale* df;   //final scale returned 
   map<APPLICATION_POLICIES, double> appp;
   //appp[LINK_REL2] = 1;   //matches default in main()
   appp[SEPARATE_REL2] = 1.0;   //matches default in main()
   SwingInfo* swp = new SwingInfo(appp, true, false, true, false, false, 0, false, false, 0.0);
   IRutil::nullstream nullstrm; 
   ostream& ccout = (usingCin()) ? cout : nullstrm; 
   string ans, desc, status, prompt; 
   size_t pos,pos2,pos3;
   string capstr = scalesMenu->getStatus(DELTA_CAP);
   double deltaCap = atof(capstr.c_str());
   double radius, scalePower, gradient, negGradient, equalTopFix, patchPower, beta, gamma;
   double floatItem; 
   map<string,double> normFactors;
   DELTA_PATCHES pp; 
   bool wef;     //weight evals first
   string wefstr = "";
   //string swp;   //swing policy
   size_t userLowDepth, userHighDepth, userSwingLow, userSwingHigh, zeroDepth, judgmentDepth;
 
   // The most complicated menu interaction in the program, and I prefer 
   // while(entering) to while(choice != "go")... 
   // To share code updating the scale name, I implement fall-thru: 
   bool fallThru = false; 
   string baseName, patchPolicy, infix, suffix; 
   int baseClass, policyChoice; 
 
   //scalesMenu->resetDefaults(); 
   int choice; 
   bool entering = true; 
   while (entering) { 
      choice = scalesMenu->readChoice(); 
      switch(choice) { 
       case NO_SCALING: 
       case LINEAR_SCALE: 
       case MUL_SCALE:
       case MIX_SCALE:
       case DIVLOG_SCALE: 
       case LOG_SCALE: 
       case FRACTAL_SCALE: 
       case FULL_FRACTAL_SCALE: 
         for (int j = NO_SCALING; j < NUM_DELTA_SCALES; j++) { 
            scalesMenu->setStarred(j, false); 
         } 
         scalesMenu->setStarred(choice,true); 
         if (choice == NO_SCALING) { 
            scalesMenu->setStatus(BASE_SCALE_NAME, "NoScaling"); 
         } 
         if (choice == LOG_SCALE) { 
            scalesMenu->setStatus(SCALE_POWER, "1.0"); 
            ccout << "LogScale is FractalScale with scalePower = 1.0." << endl; 
         } 
         fallThru = true;  //!! to re-set the base-scale name. 
 
       case NO_PATCH: 
       case MATCH_PROBS: 
       case BALANCE_TIES: 
       case UP_TIED_TOP: 
       case PUSH_TIED_TOP: 
       case PUSH_ALL_TIES: 
         if (!fallThru) { 
            for (int j = NO_PATCH; j < NUM_DELTA_PATCHES; j++) { 
               scalesMenu->setStarred(j, false); 
            } 
         } 
         scalesMenu->setStarred(choice, true); 
         fallThru = true; 
        
       case CENTER_DELTA: 
       case CENTER_EVAL: 
       case STRADDLE_EVALS: 
         if (!fallThru) { 
            prompt = ((choice == CENTER_DELTA) ? 
                           "Include best move as center, regardless of eval?" 
                        : ((choice == CENTER_EVAL) ? 
                              "Include center on eval of current position?" 
                            : "Include center on eval of previous position?")); 
            ccout << prompt << " (y/n) "; 
            loggedRead(ans,false); 
            if (IRutil::isYes(ans) || IRutil::trim(ans) == "on") { 
               scalesMenu->setStarred(choice,true); 
               scalesMenu->setStatus(choice,"on"); 
            } else { 
               scalesMenu->setStarred(choice,false); 
               scalesMenu->setStatus(choice,"off"); 
            } 
            break; 
         } 
       case WEIGHT_EVALS_FIRST:
         if (!fallThru) {
            ccout << "Weight evals by player depth first? (y/n) ";
            loggedRead(ans,false);
            scalesMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no");
            fallThru = true;
        }
       case SWING_POLICY:
         if (!fallThru) {
            //ccout << "Can be \"none\" or any string of scale+fixed+relative[123]+join/link/sep/proxy+boxed/damped/prop: ";
            //loggedRead(ans,false);
            //ans = IRutil::trim(ans);
            //scalesMenu->setStatus(choice, ans);
            delete(swp);
            swp = makeSwingInfo();
            scalesMenu->setStatus(choice, string(*swp));
            fallThru = true;
         }
       case DEPTH_WINDOW:
         if (!fallThru) {
            ccout << "Enter lowest and highest depths considered: " << endl;
            loggedReadBounded(userLowDepth,0,100,false);
            loggedReadBounded(userHighDepth,userLowDepth,100,false);
            scalesMenu->setStatus(choice, IRutil::itoa(userLowDepth) + " .. "
                                     + IRutil::itoa(userHighDepth));
            fallThru = true;
         }
       case SWING_WINDOW:
         if (!fallThru) {
            ccout << "Enter lowest and highest depths for swing: " << endl;
            loggedReadBounded(userSwingLow,0,100,false);
            loggedReadBounded(userSwingHigh,userSwingLow,100,false);
            scalesMenu->setStatus(choice, IRutil::itoa(userSwingLow) + " .. "
                                     + IRutil::itoa(userSwingHigh));
            fallThru = true;
         }
       case ZERO_DEPTH:
         if (!fallThru) {
            ccout << "Enter depth for zero point of Poisson distribution: " << endl;
            loggedReadBounded(zeroDepth,0,100,false);
            scalesMenu->setStatus(choice, IRutil::itoa(zeroDepth));
            fallThru = true;
         }
       case JUDGMENT_DEPTH:
         if (!fallThru) {
            ccout << "Enter depth to predict (0 for turn max): " << endl;
            loggedReadBounded(judgmentDepth,0,100,false);
            scalesMenu->setStatus(choice, IRutil::itoa(judgmentDepth));
            fallThru = true;
         }
       case RADIUS: 
         if (!fallThru) { 
            ccout << "Enter max margin considered part of plateau, " 
                  << "e.g. 0.75 pawns: "; 
            loggedReadBounded(radius,0,deltaCap,false); 
            scalesMenu->setStatus(choice, IRutil::ftoa(radius,2)); 
            fallThru = true; 
         }  
       case SCALE_POWER: 
         if (!fallThru) { 
            if (scalesMenu->isStarred(LOG_SCALE)) { 
               ccout << "Can't change scale power for log scale, sorry." 
                     << endl; 
            } else { 
               ccout << "Enter new c for 1/y^c or 1/log(y^c) scaling: "; 
               loggedReadBounded(scalePower,0,100,false); 
               scalesMenu->setStatus(choice, IRutil::ftoa(scalePower)); 
            } 
            fallThru = true; 
         } 
       case GRADIENT: 
         if (!fallThru) { 
            ccout << "Enter steepness at end of plateau " 
                  << "(> 1.0 means steeper): "; 
            loggedReadBounded(gradient,0,100,false); 
            scalesMenu->setStatus(choice, IRutil::ftoa(gradient)); 
            fallThru = true;
         } 
       case NEG_GRADIENT:
         if (!fallThru) {
            ccout << "Enter steepness at negative end "
                  << "(> 1.0 means steeper): ";
            loggedReadBounded(negGradient,0,100,false);
            scalesMenu->setStatus(choice, IRutil::ftoa(negGradient));
            fallThru = true;
         }
       case BETA:
         if (!fallThru) {
            ccout << "Enter b for delta - bx scaling"
                  << "(> 1.0 means steeper): ";
            loggedReadBounded(beta,0,100,false);
            scalesMenu->setStatus(choice, IRutil::ftoa(beta));
            fallThru = true;
         }
       case GAMMA:
         if (!fallThru) {
            ccout << "Enter combination factor for MulScale: ";
            loggedReadBounded(gamma,0,100,false);
            scalesMenu->setStatus(choice, IRutil::ftoa(gamma));
            fallThru = true;
         }
       case DELTA_CAP:
         if (!fallThru) {
            ccout << "Enter new cap on blunder deltas (max "
                  << IRutil::ftoa(IRutil::DELTACAP) << "): ";
            loggedRead(deltaCap,false);
            scalesMenu->setStatus(choice, IRutil::ftoa(deltaCap, 2));
         }

         //Now we must recall which base-name is starred:
         baseClass = NO_SCALING;
         while (!scalesMenu->isStarred(baseClass)) { baseClass++; }
         policyChoice = NO_PATCH;
         while (!scalesMenu->isStarred(policyChoice)) { policyChoice++; }
         baseName = scalesMenu->nameOf(baseClass);   //note: is capitalized
         patchPolicy = scalesMenu->nameOf(policyChoice);
         wefstr = (IRutil::isYes(scalesMenu->getStatus(WEIGHT_EVALS_FIRST)) ? ", WEF" : "");
         capstr = scalesMenu->getStatus(DELTA_CAP);
         //swp = scalesMenu->getStatus(SWING_POLICY);
         infix = "[" + scalesMenu->getStatus(DEPTH_WINDOW) + ", " 
                     + scalesMenu->getStatus(SWING_WINDOW)
                     + "; " + scalesMenu->getStatus(JUDGMENT_DEPTH) + "]";
         suffix = "(" + scalesMenu->getStatus(RADIUS) + ","
                      + scalesMenu->getStatus(SCALE_POWER) + ","
                      + scalesMenu->getStatus(GRADIENT) + ","
                      + scalesMenu->getStatus(NEG_GRADIENT) + ","
                      + scalesMenu->getStatus(BETA) + ","
                      + scalesMenu->getStatus(GAMMA) + ")";
         scalesMenu->setStatus(BASE_SCALE_NAME, baseName + ", " + patchPolicy
                               + wefstr + ", cap " + capstr + "; " + string(*swp) + "\n" + infix + suffix);
         fallThru = false;
         break;

       case NORM_FACTORS:
         ccout << "Enter identifying substring for engine: ";
         ans = "";
         while (ans == "") {
            loggedRead(ans,false);
            ans = IRutil::trim(ans);
         }
         ccout << "Enter normalization value as multiplier: ";
         loggedRead(floatItem,false);
         desc = scalesMenu->getDescription(choice);
         if (desc.find("Normalization") != string::npos) {
            scalesMenu->setDescription(choice,""); 
         }
         status = scalesMenu->getStatus(choice);
         pos = status.find(ans);
         if (pos != string::npos) {  //changing existing entry
            pos2 = status.find(",", pos);
            pos3 = status.find(")", pos2);
            status.replace(pos2+1,pos3-pos2-1,IRutil::ftoa(floatItem));
            scalesMenu->setStatus(choice,status);
         } else {
            scalesMenu->setStatus(choice, status + "(*" + ans + "*," + IRutil::ftoa(floatItem) + ") ");
         }
         break;
       case PIN_TO_CAP: 
         ccout << "Make differential approach 0 at delta-cap? (y/n) "; 
         loggedRead(ans,false); 
         scalesMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no"); 
         break; 
       case EQUAL_TOP_FIX: 
         ccout << "Enter new value for equal-top fudge factor, e.g. 0.02: "; 
         loggedRead(equalTopFix,true);  //break line here in log file 
         scalesMenu->setStatus(choice, IRutil::ftoa(equalTopFix, 3)); 
         break; 
       case PATCH_POWER:
         ccout << "Enter new value for patch power, e.g. 0.58: ";
         loggedRead(patchPower,true);  //break line here in log file
         scalesMenu->setStatus(choice, IRutil::ftoa(patchPower, 4));
         break;
       case BASE_SCALE_NAME: 
         ccout << "Enter name for base scale (do again if you " 
               << "change parameters): "; 
         loggedRead(ans,true); //inserts newline in log 
         scalesMenu->setStatus(choice, IRutil::trim(ans)); 
         break; 
       case FINISH_SCALE: 
         entering = false; 
         break; 
       default: 
         ccout << "No other options, must make valid choice, " 
               << FINISH_SCALE << "/" << scalesMenu->nameOf(FINISH_SCALE) 
               << " to end." << endl; 
         break; 
      } 
   } //end of while loop 
 
   deltaCap = atof((scalesMenu->getStatus(DELTA_CAP)).c_str()); 
 
   if (scalesMenu->isStarred(NO_SCALING)) { 
      //scalesMenu->resetDefaults(); 
      theUnitLineScale->setDeltaCap(deltaCap);
      return theUnitLineScale; 
   } 
   //else 
   int ppi = NO_PATCH; 
   while (ppi < NUM_DELTA_PATCHES && !scalesMenu->isStarred(ppi)) { ppi++; } 
   pp = (ppi == NUM_DELTA_PATCHES) ? NO_PATCH : DELTA_PATCHES(ppi); 
   wef = IRutil::isYes(scalesMenu->getStatus(WEIGHT_EVALS_FIRST));
   //swp = scalesMenu->getStatus(SWING_POLICY);
 
   radius = atof((scalesMenu->getStatus(RADIUS)).c_str()); 
   scalePower = atof((scalesMenu->getStatus(SCALE_POWER)).c_str()); 
   gradient = atof((scalesMenu->getStatus(GRADIENT)).c_str()); 
   negGradient = atof((scalesMenu->getStatus(NEG_GRADIENT)).c_str());
   normFactors = IRutil::parseStarredPairs(scalesMenu->getStatus(NORM_FACTORS));
   equalTopFix = atof((scalesMenu->getStatus(EQUAL_TOP_FIX)).c_str()); 
   patchPower = atof((scalesMenu->getStatus(PATCH_POWER)).c_str());
   baseName = scalesMenu->getStatus(BASE_SCALE_NAME); 
   zeroDepth = atoi((scalesMenu->getStatus(ZERO_DEPTH)).c_str());
   judgmentDepth = atoi((scalesMenu->getStatus(JUDGMENT_DEPTH)).c_str());
   userLowDepth = size_t(IRutil::extractDoubles(scalesMenu->getStatus(DEPTH_WINDOW))[0]);
   userHighDepth = size_t(IRutil::extractDoubles(scalesMenu->getStatus(DEPTH_WINDOW))[1]);
   userSwingLow = size_t(IRutil::extractDoubles(scalesMenu->getStatus(SWING_WINDOW))[0]);
   userSwingHigh = size_t(IRutil::extractDoubles(scalesMenu->getStatus(SWING_WINDOW))[1]);
   beta = atof((scalesMenu->getStatus(BETA)).c_str());
   gamma = atof((scalesMenu->getStatus(GAMMA)).c_str());
   
cerr << "Zero depth: " << zeroDepth << "; judgment depth " << judgmentDepth << "; ";
cerr << "Depth window: " << scalesMenu->getStatus(DEPTH_WINDOW) << "; Swing window "
<< userSwingLow << ".." << userSwingHigh << endl;
 
   if (scalesMenu->isStarred(LINEAR_SCALE)) { 
      ls = new LinearScale(baseName, wef, swp, pp, patchPower, equalTopFix, normFactors,
                           zeroDepth, userLowDepth, userHighDepth, userSwingLow, 
                           userSwingHigh, judgmentDepth, radius, scalePower, gradient); 
   } else if (scalesMenu->isStarred(MUL_SCALE)) {
      ls = new MulScale(baseName, wef, swp, pp, patchPower, equalTopFix, normFactors,
                        zeroDepth, userLowDepth, userHighDepth, userSwingLow, userSwingHigh,
                        judgmentDepth, radius, scalePower, gradient, negGradient,
                        (negGradient != gradient));
   } else if (scalesMenu->isStarred(MIX_SCALE)) {
      ls = new MixScale(baseName, wef, swp, pp, patchPower, equalTopFix, normFactors,
                        zeroDepth, userLowDepth, userHighDepth, userSwingLow, userSwingHigh,
                        judgmentDepth, radius, scalePower, gradient, negGradient,
                        (negGradient != gradient), beta, gamma);
   } else if (scalesMenu->isStarred(DIVLOG_SCALE)) { 
      ls = new DivLogScale(baseName, wef, swp, pp, patchPower, equalTopFix, normFactors,
                           zeroDepth, userLowDepth, userHighDepth, userSwingLow,
                           userSwingHigh, judgmentDepth, radius, scalePower, gradient); 
   } else if (scalesMenu->isStarred(LOG_SCALE)) { 
      ls = new FractalScale(baseName, wef, swp, pp, patchPower, equalTopFix, normFactors,
                            zeroDepth, userLowDepth, userHighDepth, userSwingLow, 
                            userSwingHigh, judgmentDepth, radius, scalePower, gradient); 
      //repeating code with next line to make changes easier 
   } else if (scalesMenu->isStarred(FRACTAL_SCALE)) { 
      ls = new FractalScale(baseName, wef, swp, pp, patchPower, equalTopFix, normFactors,
                            zeroDepth, userLowDepth, userHighDepth, userSwingLow,
                            userSwingHigh, judgmentDepth, radius, scalePower, gradient); 
   } else if (scalesMenu->isStarred(FULL_FRACTAL_SCALE)) { 
      ls = new FullFractalScale(baseName, wef, swp, pp, patchPower, equalTopFix, normFactors,
                                zeroDepth, userLowDepth, userHighDepth, userSwingLow,
                                userSwingHigh, judgmentDepth, scalePower, gradient, negGradient); 
   } else { 
      cerr << "Bug in Scales menu." << endl; 
   } 
   if (scalesMenu->isStarred(CENTER_DELTA)) { 
      if (scalesMenu->isStarred(CENTER_EVAL)) { 
         if (scalesMenu->isStarred(STRADDLE_EVALS)) { 
            df = new TernaryScale(ls, new TranslateByEval(ls), 
                                  new StraddleEvals(ls)); 
         } else { 
            df = new BinaryScale(ls, new TranslateByEval(ls)); 
         } 
      } else { 
         if (scalesMenu->isStarred(STRADDLE_EVALS)) { 
            df = new BinaryScale(ls, new StraddleEvals(ls)); 
         } else { 
            df = ls; 
         } 
      } 
   } else { 
      if (scalesMenu->isStarred(CENTER_EVAL)) { 
         if (scalesMenu->isStarred(STRADDLE_EVALS)) { 
            df = new BinaryScale(new TranslateByEval(ls), 
                                 new StraddleEvals(ls)); 
         } else { 
            df = new TranslateByEval(ls); 
         } 
      } else { 
         if (scalesMenu->isStarred(STRADDLE_EVALS)) { 
            df = new StraddleEvals(ls); 
         } else { 
            df = new SimpleFullFractalScale(string("Simple")+baseName,
                     ls->weightsEvalsFirst(), swp, pp, patchPower, equalTopFix,
                     normFactors, ls->getZeroDepth(), ls->getLowDepth(),
                     ls->getHighDepth(), ls->getSwingLow(), ls->getSwingHigh(),
                     ls->getJudgmentDepth(), 1.0, gradient, negGradient);

/*
            df = new UnitLineScale(baseName, ls->weightsEvalsFirst(), pp, patchPower,
                    //atof((scalesMenu->getStatus(EQUAL_TOP_FIX)).c_str()),
                    equalTopFix, normFactors, ls->getZeroDepth(), ls->getLowDepth(), 
                    ls->getHighDepth(), ls->getSwingLow(), ls->getSwingHigh(),
                    ls->getJudgmentDepth());
*/
         } 
      } 
   } 
 
   if (IRutil::isYes(scalesMenu->getStatus(PIN_TO_CAP))) { 
      df = new PinToCap(df, radius, deltaCap); 
   } 
 
   df->setWEF(wef);
   df->setSwingPolicy(swp);
   df->setLowDepth(userLowDepth);
   df->setHighDepth(userHighDepth);
   df->setSwingLow(userSwingLow);
   df->setSwingHigh(userSwingHigh);
   df->setZeroDepth(zeroDepth);
   df->setJudgmentDepth(judgmentDepth);
   df->setDeltaCap(deltaCap);
   //scalesMenu->resetDefaults();
   return df; 
} 



SwingInfo* Ensemble::makeSwingInfo() const {
   IRutil::nullstream nullstrm;
   ostream& ccout = (usingCin()) ? cout : nullstrm;
   int choice = -1;
   map<APPLICATION_POLICIES,double> swingBook;
   SwingInfo* swp;
   swingMenu->resetDefaults();
   APPLICATION_POLICIES ap;
   SWING_OPTIONS option1, option2;
   bool depthDependentSwings = IRutil::isYes(swingMenu->getStatus(DEPTH_WTD));
   bool unscaledSwings = swingMenu->isStarred(UNSCALED_SWING);
   bool damped = swingMenu->isStarred(DAMPED); //IRutil::isYes(swingMenu->getStatus(DAMPED));
   bool prop = swingMenu->isStarred(PROPORTIONAL); //IRutil::isYes(swingMenu->getStatus(PROPORTIONAL));
   bool boxed = swingMenu->isStarred(BOXED); //IRutil::isYes(swingMenu->getStatus(BOXED));
   size_t boxKind = atoi(swingMenu->getStatus(BOXED_OPTION).c_str());
   bool decouple = swingMenu->isStarred(DECOUPLE_PARAMS);
   bool scaleHere = IRutil::isYes(swingMenu->getStatus(SCALE_HERE));
   double transpower = IRutil::extractDoubles(swingMenu->getStatus(TRANS_POWER))[0];
   int intItem;
   double floatItem;
   string ans;
   double mul, mul1, mul2;
   int i,j;
   bool entering = true;

   while (entering) {
      choice = swingMenu->readChoice();
      switch(choice) {
       case GO_SWING:
         depthDependentSwings = IRutil::isYes(swingMenu->getStatus(DEPTH_WTD));
         unscaledSwings = swingMenu->isStarred(UNSCALED_SWING);
         damped = swingMenu->isStarred(DAMPED);
         prop = swingMenu->isStarred(PROPORTIONAL);
         boxed = swingMenu->isStarred(BOXED);
         boxKind = atoi(swingMenu->getStatus(BOXED_OPTION).c_str());
         decouple = swingMenu->isStarred(DECOUPLE_PARAMS);
         scaleHere = IRutil::isYes(swingMenu->getStatus(SCALE_HERE));
         transpower = IRutil::extractDoubles(swingMenu->getStatus(TRANS_POWER))[0];
         mul = IRutil::extractDoubles(swingMenu->getStatus(NO_SWING_TERM))[0];
         if (mul > 0.0) {
            swingBook[NO_SWING] = mul;
         }
         for (i = JOIN_TERMS; i <= APPLY_TO_PROXY; i++) {
            for (j = ABS_SWING; j <= REL3_SWING; j++) {
               option1 = SWING_OPTIONS(i);
               option2 = SWING_OPTIONS(j);
               ap = combinePolicies(option1,option2);
               mul1 = IRutil::extractDoubles(swingMenu->getStatus(option1))[0];
               mul2 = IRutil::extractDoubles(swingMenu->getStatus(option2))[0];
               mul = mul1*mul2;
               if (mul > 0.0) {
                  swingBook[ap] = mul;
               }
            }
         }
         if (swingBook.size() == 0) {
            cerr << "No swing policy selected, please try again." << endl;
            break;
         }

         swp = new SwingInfo(swingBook, depthDependentSwings, unscaledSwings,
                             damped, prop, boxed, boxKind, decouple, scaleHere, transpower);
         entering = false;
         break;
       case NO_SWING_TERM:
       case JOIN_TERMS:
       case LINK_TERMS:
       case SEPARATE_TERMS:
       case APPLY_TO_PROXY:
       case ABS_SWING:
       case REL1_SWING:
       case REL2_SWING:
       case REL3_SWING:
         ccout << "Enter float weight (0 to exclude): ";
         loggedRead(floatItem, false);
         swingMenu->setStatus(choice, IRutil::ftoa(floatItem));
         if (floatItem == 0.0) {   //OK comparison
            swingMenu->setStarred(choice,false);
         } else {
            swingMenu->setStarred(choice,true);
         }
         break;

       case COUPLE_PARAMS:
         swingMenu->setStarred(COUPLE_PARAMS, true);
         swingMenu->setStarred(DECOUPLE_PARAMS, false);
         break;
       case DECOUPLE_PARAMS:
         swingMenu->setStarred(DECOUPLE_PARAMS, true);
         swingMenu->setStarred(COUPLE_PARAMS, false);
         break;

       case DEPTH_WTD:
         ccout << "Weight swings differently across depths via d and v? ";
         loggedRead(ans , false);
         swingMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no");
         break;

       case UNSCALED_SWING:
         swingMenu->setStarred(UNSCALED_SWING, true);
         swingMenu->setStarred(SCALED_SWING, false);
         break;
       case SCALED_SWING:
         swingMenu->setStarred(SCALED_SWING, true);
         swingMenu->setStarred(UNSCALED_SWING, false);
         break;

       case SIMPLE:
         swingMenu->setStarred(SIMPLE, true);
         swingMenu->setStarred(PROPORTIONAL, false);
         swingMenu->setStarred(DAMPED, false);
         swingMenu->setStarred(BOXED, false);
         break;
       case PROPORTIONAL:
         swingMenu->setStarred(SIMPLE, false);
         swingMenu->setStarred(PROPORTIONAL, true);
         swingMenu->setStarred(DAMPED, false);
         swingMenu->setStarred(BOXED, false);
         break;
       case DAMPED:
         swingMenu->setStarred(SIMPLE, false);
         swingMenu->setStarred(PROPORTIONAL, false);
         swingMenu->setStarred(DAMPED, true);
         swingMenu->setStarred(BOXED, false);
         break;
       case BOXED:
         swingMenu->setStarred(SIMPLE, false);
         swingMenu->setStarred(PROPORTIONAL, false);
         swingMenu->setStarred(DAMPED, false);
         swingMenu->setStarred(BOXED, true);
	 break;

       case BOXED_OPTION:
         ccout << "Enter option 0--9 (\"see manual\"): ";
         loggedReadBounded(intItem,0,9,false);
         swingMenu->setStatus(choice, IRutil::itoa(intItem));
         break;
       case SCALE_HERE:
         cout << "Scale deltas here, instead/on-top of other scaling? ";
         loggedRead(ans);
         swingMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no");
         break;
       case TRANS_POWER:
         ccout << "Enter a for logistic translation by (a + 1)/(a + exp(-u)): ";
         loggedReadBounded(floatItem,0,100,false);
         swingMenu->setStatus(choice, IRutil::ftoa(floatItem));
         break;
       default:
         ccout << "Invalid option, please try again..." << endl;
      }
   }
   return swp;
}

 


/** Parse lines matching this pattern: 
          Delta = (d.dd,d.dd, ...  ,*d.dd, ... ,d.dd) or 
          Delta = (d.dd,d.dd, ...  ,d.dd, ... ,d.dd)* 
    Ignores tag "Delta". 
    Hard-writes the capping of deltas by IRutil::DELTACAP (= 10.00). 
*/ 
int Ensemble::parseDeltas(string& line, vector<double>* deltas) const { 
   int playedMoveIndex = IRutil::playedMoveNotInTopN; 
   int startPos = line.find("("); 
   int endPos = line.find(")"); 
   line = line.substr(startPos+1,endPos-startPos-1); 
   char* pch = strtok((char*)line.c_str(),","); 
   int deltaCount = 0; 
   while (pch != NULL) {  // && deltaCount < IRutil::NCUTOFF) 
      if(pch[0] == '*') { 
         playedMoveIndex = deltaCount; 
         deltas->push_back(min(atof(pch+1), IRutil::DELTACAP)); 
      } else { 
         deltas->push_back(min(atof(pch), IRutil::DELTACAP)); 
      } 
      deltaCount++; 
      pch = strtok(NULL, ","); 
   } 
   return playedMoveIndex; 
} 

void Ensemble::readTurnsFromFileGlob(const string& fileGlobInput,
                                     bool referenceOnly, bool selectionOnly) const{
   string fileGlob = IRutil::trim(fileGlobInput);
   glob_t globbuf;
   //GLOB_TILDE expands "~" in the pattern to the home directory
   //If fileGlob has a '/' in it then figure this is only seek user wants.
   //If fileGlob begins with "../" then we log path up to "/"
   if (fileGlob.size() >= 3 && fileGlob.substr(0,3) == "../") {
      string cwd = IRutil::getmycwd();
      size_t pathEnd = cwd.find_last_of('/');
      string fullpath = cwd.substr(0,pathEnd);
      fileGlob = fullpath + fileGlob.substr(2); //includes the "/"
   }
   loggedWrite(fileGlob, true);
   vector<string> refFiles;   //checked for dupes only in this shot
         
   string lastpath = "";
   bool seeking = true;
   bool foundFiles = false;
   while (seeking) {
      glob(fileGlob.c_str(), GLOB_TILDE, NULL, &globbuf);
      for (int i = 0; i < globbuf.gl_pathc; ++i) {
         string str(globbuf.gl_pathv[i]);
         size_t pathEnd = str.find_last_of('/');  //!!! change for Windows
         string newpath = (pathEnd == string::npos ? "" : str.substr(0,pathEnd+1));
         string name = (pathEnd == string::npos ? str : str.substr(pathEnd+1));
         if (newpath != lastpath) {
            cout << "Files matched on new path " << newpath << ":" << endl;
            cout << name << endl;
         } else {
            cout << name << endl;
         }
         lastpath = newpath;
      }
      for (int i = 0; i < globbuf.gl_pathc; ++i) {
         string fileName = globbuf.gl_pathv[i];
         size_t pathEnd = fileName.find_last_of('/');
         string rawName = (pathEnd == string::npos ? fileName : fileName.substr(pathEnd+1));
         ifstream file(globbuf.gl_pathv[i]);
         vector<string>::iterator fbegin = (referenceOnly ? refFiles.begin() : loadedFiles.begin());
         vector<string>::iterator fend = (referenceOnly ? refFiles.end() : loadedFiles.end());
         if (!file.is_open()) {
            cout << "Cannot open glob match " << fileName << endl;
         } else if (find(fbegin,fend,rawName) != fend) {
            string rds = (rawName == "SF7Turns.aif" ? "un-needed occurrence of "
                                                    : "duplicate file ");
            cout << "Skipping " << rds << rawName << endl;
         } else {
            string refstr = (referenceOnly ? " for reference trial only..." : "");
            cout << "Reading file " << rawName << refstr << endl;
            readInputFile(file, referenceOnly, selectionOnly);
            foundFiles = true;
            if (referenceOnly) {
               refFiles.push_back(rawName);
            } else {
               loadedFiles.push_back(rawName);
            }
         }
      }
      if (foundFiles && (!referenceOnly)) {
         loadedGlobs = (loadedGlobs == "" ? fileGlob : loadedGlobs + ";" + fileGlob);
      }
      seeking = (fileGlob.find_last_of('/') == string::npos);
      fileGlob = dataPath + fileGlob; 
      foundFiles = false;
   }
   globfree( &globbuf );
}

void Ensemble::readTurns(istream& IN) const {   //outside the INP limitation 
   string fileGlob; 
   cout << "Enter name of file or glob." << endl; 
   IN >> fileGlob;
   //loggedWrite(fileGlob,true);  //now done in callee after substituting for "../" if present
   readTurnsFromFileGlob(fileGlob,false,false);
} 
 
/*
list<TurnInfo*>* Ensemble::readInputFile(const string& fileName, bool referenceOnly,
                                              bool selectionOnly) const { 
   ifstream file; 
   file.open((const char*)fileName.c_str(), ios::in); 
   if (!file.is_open()) { 
      cout << "Cannot read the input file." << endl; 
      return new list<TurnInfo*>(); 
   } 
   return readInputFile(file, referenceOnly, selectionOnly); 
} 
*/
 
/** Process AIF file and produced TurnInfo and GameInfo objects.
    TurnInfo stores 'raw' information of all turns in the file 
    GameInfo stores information related to games, from mainly PGN file.
    KWR: Changed to store evals as ints in centipawns.  Also changed to
    assume moves are in game sequence, and to look back in current game
    for first occurrence of repeating position, marking all intervening
    moves as being "inRep".
 */ 
list<TurnInfo*>* Ensemble::readInputFile(istream& infile, bool referenceOnly,
                                              bool selectionOnly) const { 
 
   //next is semi-debugging but should be preserved. 
   string line = ""; 
   int lineNumber = 0; 
   int turnNumber = 0; 
   //int prevTurnNumber = 1000;   //??
   int AIFoffset = 1;  //AIF format numbers depths from 1
   
   double currEval = 0.0; 
   double prevEval = 0.0; 
   double deltaFalloff, deltaFalloffCapped; 
   TurnInfo* lastTurn = NULL; 
   //vector<Move>* consideredMovesVec = new vector<Move>(); //has moves in column order, passed by value
   //vector<int> evalsCP;     //pushed back as a copy
   vector <TurnInfo> gameTurns; 
   list<TurnInfo*>* returnTurns = new list<TurnInfo*>();
   list<TurnInfo*>* fillTurns = (selectionOnly ? returnTurns : pTurns);
   GameInfo* pGameInfo = NULL; 
      
   while(getline(infile,line)) {   //outer loop over AIF *file* (or files?)
      line = IRutil::trim(line);
      lineNumber++; 
      size_t gameBegin = line.find("[GameID ");//Beginning of PGN-header part
      bool legacy = false;
      if (gameBegin == string::npos) {
         gameBegin = line.find("[Event ");  //Legacy beginning
         legacy = (gameBegin != string::npos);
      }
      size_t turnBegin = line.find("[GID "); 

      int numPV;
      int minHighDepth; 
      int maxHighDepth; //used for moves as well

      if ((gameBegin == string::npos) && (turnBegin == string::npos)) {   
         //skip---not game or turn beginning 
         continue; 
      } 
      else if (gameBegin != string::npos) {   //Game Info, commence inner loop 
//cerr << "Game ";

         string gameID;
         string engineID;
         string platform;
         int numThreads = 1;
         int hash = 512; 
         int plyDepth = 0;  //legacy
         string egt = "";
         string modestr = "??";
         string eventName = "??"; 
         string site = "??"; 
         IRutil::SimpleDate date(0,0,0); 
         int round = 1; 
         string whitePlayer = ""; 
         string blackPlayer = ""; 
         string result = "*";   //TAMAL: convert to enum? 
         string eco = "??"; 
         int whiteElo = 0; 
         int blackElo = 0; 
         int plyCount = 0; 
         IRutil::SimpleDate eventDate(0,0,0); 
         string eventType = "?!?";
         int eventRounds = 0;
         string eventCountry = "??";
         string eventCategory = "??";
         string source = ""; 
         IRutil::SimpleDate sourceDate(0,0,0); 
         string timeControl = "??";  //e.g. [TimeControl "40/4800:20/2400:1200"]
         string whiteTeam = "??";
         string blackTeam = "??";
         string whiteTeamCountry = "??";
         string blackTeamCountry = "??";
         string whiteFIDEID = "??";
         string blackFIDEID = "??";

         string tmp = "";
         //process GameHeader here 
         //Use switch case 
         
         if (!legacy) {
            gameID = IRutil::getBetweenDelm(line,'\"','\"'); 

         //PGN spec requires a blank line between header and moves.
         //It seems that for PGN export, the blank line must have no spaces.
         //KWR: No---a blank line is ^\s*$.  Spaces must be tolerated.  So we trim.
         //
         //IMPT: Does not enforce any ordering of PGN/AIF fields.
         //Unlike perl/Python script, does not load any field anonymously.

            getline(infile,line); 
            line = IRutil::trim(line);
            lineNumber++; 
         }
//cerr << gameID << endl;
         while(!line.empty()) {   //stops on blank line
            if (line.find("[EngineID ") != string::npos) {
               engineID = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Platform ") != string::npos) {
               platform = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Threads ") != string::npos) {
               tmp = IRutil::getBetweenDelm(line,'\"','\"');
               numThreads = atoi(tmp.c_str());
            }
            else if (line.find("[Hash ") != string::npos) {
               tmp = IRutil::getBetweenDelm(line,'\"','\"');
               hash = atoi(tmp.c_str());
            }
            else if (line.find("[MultiPV ") != string::npos) {
               tmp = IRutil::getBetweenDelm(line,'\"','\"');
               numPV = atoi(tmp.c_str());
            }
            else if (line.find("[DepthRange ") != string::npos) {
               tmp = IRutil::getBetweenDelm(line,'\"',':');
               minHighDepth = atoi(tmp.c_str());
               tmp = IRutil::getBetweenDelm(line,':','\"');
               maxHighDepth = atoi(tmp.c_str());
            }
            else if (line.find("[PlyDepth ") != string::npos) {  //legacy
               string plyDepthStr = IRutil::getBetweenDelm(line,'\"','\"');
               if (plyDepthStr.compare("??") == 0) {
                  plyDepth = -1;
               } else {
                  plyDepth = atoi(plyDepthStr.c_str());
               }
               minHighDepth = plyDepth;
               maxHighDepth = plyDepth;
            }
            else if (line.find("[EGT ") != string::npos) {
               egt = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Mode ") != string::npos) {
               modestr = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Event ") != string::npos) {
               eventName = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Site ") != string::npos) { 
               site = IRutil::getBetweenDelm(line,'\"','\"'); 
            }
            else if (line.find("[Date ") != string::npos) { 
               date = IRutil::SimpleDate 
                  (IRutil::getBetweenDelm(line,'\"','\"'), 
                  IRutil::ARENA_FORMAT); 
            }
            else if (line.find("[Round ") != string::npos) { 
               string roundStr = IRutil::getBetweenDelm(line,'\"','\"'); 
               if (roundStr.compare("??") == 0) {
                  round = -1; 
               } else {
                  round = atoi(roundStr.c_str()); 
               }
            }
            else if (line.find("[White ") != string::npos) {
               whitePlayer = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Black ") != string::npos) {
               blackPlayer = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Result ") != string::npos) { 
               result = IRutil::getBetweenDelm(line,'\"','\"'); 
            } 
            else if (line.find("[ECO ") != string::npos) { 
               eco = IRutil::getBetweenDelm(line,'\"','\"'); 
            } 
            else if (line.find("[WhiteElo ") != string::npos) { 
               string whiteEloStr = IRutil::getBetweenDelm(line,'\"','\"'); 
               if (whiteEloStr.compare("??") == 0) {
                  whiteElo = -1; 
               } else {
                  whiteElo = atoi(whiteEloStr.c_str()); 
               }
            } 
            else if (line.find("[BlackElo ") != string::npos) { 
               string blackEloStr = IRutil::getBetweenDelm(line,'\"','\"'); 
               if (blackEloStr.compare("??") == 0) {
                  blackElo = -1; 
               } else {
                  blackElo = atoi(blackEloStr.c_str()); 
               }
            } 
            else if (line.find("[PlyCount ") != string::npos) { 
               plyCount = atoi((IRutil::getBetweenDelm(line,'\"','\"')).c_str()); 
            } 
            else if (line.find("[EventDate ") != string::npos) { 
               eventDate = IRutil::SimpleDate 
                  (IRutil::getBetweenDelm(line,'\"','\"'), 
                  IRutil::ARENA_FORMAT); 
            } 
            else if (line.find("[EventType ") != string::npos) {
               eventType = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[EventRounds ") != string::npos) {
               string er = IRutil::getBetweenDelm(line,'\"','\"');
               eventRounds = atoi(er.c_str());
            }
            else if (line.find("[EventCountry ") != string::npos) {
               eventCountry = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[EventCategory ") != string::npos) {
               eventCategory = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[Source ") != string::npos) {
               source = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[SourceDate ") != string::npos) { 
               sourceDate = IRutil::SimpleDate 
                  (IRutil::getBetweenDelm(line,'\"','\"'), 
                  IRutil::ARENA_FORMAT); 
            }
            else if (line.find("[TimeControl ") != string::npos) {
	       timeControl = IRutil::getBetweenDelm(line,'\"','\"');
	    }
            else if (line.find("[WhiteTeam ") != string::npos) {
               whiteTeam = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[BlackTeam ") != string::npos) {
               blackTeam = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[WhiteTeamCountry ") != string::npos) {
               whiteTeamCountry = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[BlackTeamCountry ") != string::npos) {
               blackTeamCountry = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[WhiteFideId ") != string::npos) {
               whiteFIDEID = IRutil::getBetweenDelm(line,'\"','\"');
            }
            else if (line.find("[BlackFideId ") != string::npos) {
               blackFIDEID = IRutil::getBetweenDelm(line,'\"','\"');
            } else { 
               //unknown header---choose to ignore rather than load anonymously  !!!
            } 
            getline(infile,line); 
            line = IRutil::trim(line);
            lineNumber++; 
            if (lineNumber % IRutil::LINESMODULUS == 0) {
               cerr << lineNumber << " lines read" << endl;
            }
         } 

         if (legacy && numPV == 0) {    //!!!
cerr << "Setting numPV = 50 for this game" << endl;
            numPV = 50;
         }

         pGameInfo = new GameInfo(gameID,engineID,platform,numThreads,hash,numPV,
            minHighDepth,maxHighDepth,egt,modestr,eventName,site,date,round,
            whitePlayer,blackPlayer,result,eco,whiteElo,blackElo,plyCount,
            eventDate,eventType,eventRounds,eventCountry,eventCategory,
            source,sourceDate,timeControl,whiteTeam,blackTeam,whiteTeamCountry,
            blackTeamCountry,whiteFIDEID,blackFIDEID);
         pGames->push_back(pGameInfo);
            
         continue; // We don't want to create a TurnInfo exactly now.  

      }    //end of interior loop over lines 
      else // We are processing the TurnInfo part of an AIF file
           // Since we control all of AIF, we dictate a strict line order for
           // greater efficiency.
           //
/*
[GID "AUT-chT 1314;Austria;2014.01.30;4.6;Baramidze, David;Kuljasevic, Davorin;1-0"]
[EID "Komodo 9.3 64-bit "]
[Turn "9-w"]
[MovePlayed "h3"]
[EngineMove "e5"]
[Eval "-009"]
[PrevEval "n.a."]
[NextEval "-015"]
[Depth "17"]
[NodeCount "7964964"]
[FEN "rn1q1rk1/pb2ppbp/1p1p1np1/2p5/2P1P3/2NP1NP1/PP3PBP/R1BQ1RK1 w - - 0 9"]
[FiftyMR "1"]
[RepCount "0"]
[RepLine1 ""]
[RepLine2 ""]
[NumLegalMoves "36"]
[LegalMoves ".."

[GameID "Anand;Gelfand;2007.09.13;World;1"]
[MoveNo "9-b"]
[MovePlayed "O-O"]
[MovePlayedSAN "0-0"]
[EngineMove "Ne5xf3"]
[EngineMoveSAN "Nxf3"]
[FEN  "r1bqk2r/ppp1bppp/3p4/4n3/8/2P1BN2/PPPQ1PPP/2KR1B1R b kq - 6 9"]
[RepCount "0"]
[Eval " +009"]
[PrevEval " +019"]
[NextEval " +019"]
[LegalMoves "..."]
[LegalMovesSAN "..."]
[NumLegalMoves "36"]
[FiftyMR "6"]
[RepLine1 ""]
[RepLine2 ""]

*/
      { 
//cerr << "Mv";
         while (line.find("[GID ") == string::npos
                      && line.find("[GameID ") == string::npos) { //legacy
            if (getline(infile,line)) {
               lineNumber++;
               if (lineNumber % IRutil::LINESMODULUS == 0) {
                  cerr << lineNumber << " lines read" << endl;
               }
            } else {
               break;
            }
         }
         bool bgID = (line.find("[GID ") != string::npos
                      || line.find("[GameID ") != string::npos); //legacy
         if (!bgID) { break; }  //kludge for end-of-file
         string gID = (bgID ? IRutil::getBetweenDelm(line,'\"','\"') : "??"); 
         getline(infile,line); 
         lineNumber++; 
         bool beID = (line.find("[EID ") != string::npos);
         bool legacy = !beID; //legacy has no EID for individual moves
         string eID = (beID ? IRutil::getBetweenDelm(line,'\"','\"') : "??");
         if (beID) { getline(infile,line); lineNumber++; }
         string turnNumberStr = "";
         int turnNumber = 1;
         string whoMoves = "";
         bool whiteMove = false;
         bool bmn = (line.find("[Turn ") != string::npos 
                        || line.find("[MoveNo ") != string::npos);
         if (bmn) {
            turnNumberStr = IRutil::getBetweenDelm(line,'\"','-'); 
            turnNumber = atoi(turnNumberStr.c_str());
            string whoMoves = IRutil::getBetweenDelm(line,'-','\"');
            whiteMove = (whoMoves[0] == 'w');
            getline(infile,line); 
            lineNumber++; 
         }
         //Legacy AIF has LAN first then SAN; use latter to overwrite.
         bool bmp = (line.find("[MovePlayed ") != string::npos);
         string movePlayed = (bmp ? IRutil::trim(IRutil::getBetweenDelm(line,'\"','\"')) : "??"); 
         if (bmp) { getline(infile,line); lineNumber++; }
         bmp = (line.find("[MovePlayedSAN ") != string::npos);
	 if (bmp) {
	    movePlayed = IRutil::trim(IRutil::getBetweenDelm(line,'\"','\"'));
            getline(infile,line);
            lineNumber++;
         }
         if (movePlayed == "0-0") { movePlayed == "O-O"; }
         if (movePlayed == "0-0-0") { movePlayed == "O-O-O"; }
         bool bem = (line.find("[EngineMove ") != string::npos);
         string engineMove = (bem ? IRutil::trim(IRutil::getBetweenDelm(line,'\"','\"')) : "??"); 
         if (bem) { getline(infile,line); lineNumber++; }
         bem = (line.find("[EngineMoveSAN ") != string::npos);
         if (bem) {
            engineMove = IRutil::trim(IRutil::getBetweenDelm(line,'\"','\"'));
            getline(infile,line);
            lineNumber++;
         }
         if (engineMove == "0-0") { engineMove == "O-O"; }
         if (engineMove == "0-0-0") { engineMove == "O-O-O"; }
         string movePlayedLAN = "";
         string engineMoveLAN = "";
         bmp = (line.find("[MovePlayedLAN ") != string::npos);
         if (bmp) {
            movePlayedLAN = IRutil::trim(IRutil::getBetweenDelm(line,'\"','\"'));
            getline(infile,line);
            lineNumber++;
         }
         bmp = (line.find("[EngineMoveLAN ") != string::npos);
         if (bmp) {
            engineMoveLAN = IRutil::trim(IRutil::getBetweenDelm(line,'\"','\"'));
            getline(infile,line);
            lineNumber++;
         }

         bool bfen = (line.find("[FEN ") != string::npos);
         string fenCurrPos = (bfen ? IRutil::getBetweenDelm(line,'\"','\"') : "??");
         if (bfen) { getline(infile,line); lineNumber++; }
         bool brc = (line.find("[RepCount ") != string::npos);
         string repCountString = "";
         int repCount = 0;
         if (brc) {
            repCountString = IRutil::getBetweenDelm(line,'\"','\"');
            repCount = atoi(repCountString.c_str());
            getline(infile,line);
            lineNumber++;
         } //now files synch'ed again on eval part
         bool beval = (line.find("[Eval ") != string::npos);
         string evalStr = "";
         int eval = 0.0;
	 string prevEvalStr = "";
	 int prevEval = 0.0;
         string nextEvalStr = "";
         int nextEval = 0.0;
         if (beval) {
            evalStr = IRutil::getBetweenDelm(line,'\"','\"');
            eval = atoi(evalStr.c_str());
            getline(infile,line);
            lineNumber++;
         }
         bool bpeval = (line.find("[PrevEval ") != string::npos);
         if (bpeval) {
            prevEvalStr = IRutil::getBetweenDelm(line,'\"','\"');
            prevEval = (prevEvalStr != "n.a.") ? atoi(prevEvalStr.c_str()) : IRutil::NAVAL;
            getline(infile,line);
            lineNumber++;
         }
         bool bneval = (line.find("[NextEval ") != string::npos);
         if (bneval) {
            nextEvalStr = IRutil::getBetweenDelm(line,'\"','\"');
            nextEval = (nextEvalStr != "n.a.") ? atoi(nextEvalStr.c_str()) : IRutil::NAVAL;
            getline(infile,line);
            lineNumber++;
         } //now legacy AIF has no Depth field
         bool bdepth = (line.find("[Depth ") != string::npos);
         int depth;
         if (bdepth) {
            string depthStr = IRutil::getBetweenDelm(line,'\"','\"');
            depth = atoi(depthStr.c_str());
            getline(infile,line);
            lineNumber++;
         } else {
            depth = minHighDepth;  //defined from GameID section and uniform
         }
         bool bnc = (line.find("[NodeCount ") != string::npos);
         int nodeCount = 0;
         if (bnc) {
            string nodeCountStr = IRutil::getBetweenDelm(line,'\"','\"');
            nodeCount = atoi(evalStr.c_str());
            getline(infile,line);
            lineNumber++;
         } //FEN line might come after this too
         bfen = (line.find("[FEN ") != string::npos);
         if (bfen) {
            fenCurrPos = IRutil::getBetweenDelm(line,'\"','\"'); 
            getline(infile,line); 
            lineNumber++; 
         }
         if (fenCurrPos == "??") { cerr << "FEN not found!" << endl; }
         //Remaining fields treated as pretty random
         int fiftyMR,numLegalMoves;
         string repLine1,repLine2;
         string legalMoves; //want in SAN hence may assign twice
         while(line.find(IRutil::DASHES) == string::npos) {
            if (line.find("[RepCount ") != string::npos) {
               repCountString = IRutil::getBetweenDelm(line,'\"','\"');
               repCount = atoi(repCountString.c_str());
            } else if (line.find("[FiftyMR ") != string::npos) {
               string fiftyMRStr = IRutil::getBetweenDelm(line,'\"','\"');
               fiftyMR = atoi(fiftyMRStr.c_str());
            } else if (line.find("[RepLine1 ") != string::npos) {
               repLine1 = IRutil::getBetweenDelm(line,'\"','\"');
            } else if (line.find("[RepLine2 ") != string::npos) {
               repLine2 = IRutil::getBetweenDelm(line,'\"','\"');
            } else if (line.find("[NumLegalMoves ") != string::npos) {
               string nlmstr = IRutil::getBetweenDelm(line,'\"','\"');
               numLegalMoves = atoi(nlmstr.c_str());
            } else if (line.find("[LegalMoves ") != string::npos) {
               legalMoves = IRutil::getBetweenDelm(line,'\"','\"'); 
            } else if (line.find("[LegalMovesSAN ") != string::npos) {
               legalMoves = IRutil::getBetweenDelm(line,'\"','\"');
            }
            getline(infile,line); 
            lineNumber++; 
            if (lineNumber % IRutil::LINESMODULUS == 0) {
               cerr << lineNumber << " lines read" << endl;
            }

         }
//cerr << "a";

         getline(infile,line);// This line should contain the depth information 
         lineNumber++; 

         size_t lowDepth = 30;  //to be minimized, value not material
         size_t highDepth = 0;
         //char* pch;
         int mlo,mhi;   //lowest and highest depth for the move

         line = IRutil::trim(line);
         istringstream depthsLine(line);
         string item;
         if (depthsLine >> item) {
            mlo = atoi(item.c_str());
            mhi = mlo;
         } else {
            cerr << "Line with no depths encountered." << endl;
         }
         //char* testing = (char*)line.c_str(); 
         //pch = strtok(testing," "); 
         //if (pch != NULL) { 
            //lo = atoi(pch); 
            //pch = strtok(NULL," ");    //uses previous saved pointer
            //hi = lo;// For handling issues of only single depth. 
         //} 
         while (depthsLine >> item) {
            mhi = atoi(item.c_str());
            //hi = atoi(pch); 
            //pch = strtok(NULL," "); 
         } 
 
         getline(infile,line);// second line containing -------------- 
         lineNumber++; 
         if (lineNumber % IRutil::LINESMODULUS == 0) {
            cerr << lineNumber << " lines read" << endl;
         }

         vector<int> maxEvalPerDepth;  //must min for Black moves.
 
         int playedMoveIndex = -1; 
         vector<Move>* consideredMovesVec = new vector<Move>();

         vector<vector<int> >* depthEvalsCP = new vector<vector<int> >();

         string movestr = "";
         Position pos(fenCurrPos);
         Move mov;
         Move mpl;
         string smv;
         while (repLine1.find(',') != string::npos) {
            repLine1.replace(repLine1.find(','), 1, " ");
         }
         istringstream iss(repLine1);
         vector<Move>* repMoves = new vector<Move>();
         while (iss >> smv) {
            if (smv == "1." || smv == "2." || smv == "3." || smv == "4.") { 
               continue; //legacy
            }
            repMoves->push_back(Move(pos.move2TAN(smv)));
         }
//cerr << "b";
         while(1)   //loop broken by break for double-dashes?
         { 
            vector<int> evalsCP;
            string evalstr = "";
            getline(infile,line); 
            line = IRutil::trim(line);
            lineNumber++; 
            if (lineNumber % IRutil::LINESMODULUS == 0) { 
               cerr << lineNumber << " lines read" << endl; 
            }
            if (line.empty() || line.find("========") != string::npos) 
               break; 
 
            istringstream ess(line);
            string mv;
//cerr << ".";
            if (ess >> mv) {
               
               mov.setTAN(pos.move2TAN(mv));
               mpl.setTAN(pos.move2TAN(movePlayed));
               if (mov == mpl) {   //so Re1 matches Re1+
                  playedMoveIndex = consideredMovesVec->size(); //current not final size
               }
               consideredMovesVec->push_back(mov);
               movestr += mv + ":";
            } //remaining entries are evals
            
            int currDepth = 0; 
            int naActive = -1; // we got na, looking for next meaningful value 
            string eitem = "";
            string pt = "";
            int b;
            int multiplier = 1;    // X = 10, C = 100, x = -10, c = -100
            while (ess >> eitem && (!(IRutil::trim(eitem).empty()))) {
               if (eitem == "n.a.") {
                  b = IRutil::NAVAL;// "n.a." identifier 
               } else if (eitem == "PRUN") {                
                  b = IRutil::PRUNVAL; //" PRUN " identifier 
               } else if (eitem == "NREC") { 
                  b = IRutil::NRECVAL; //" NREC " identifier 
               } else { //real value but may still have leading M,-M or trailing XxCc
                  lowDepth = min(currDepth,int(lowDepth));
                  highDepth = max(currDepth, int(highDepth));
                  if (eitem[3] == 'X') {
                     multiplier = 10;
                     pt = eitem.substr(0,3);  //3 means 3 chars not position 3
                     b = atoi(pt.c_str());
                  } else if (eitem[3] == 'x') {
                     multiplier = -10;
                     pt = eitem.substr(0,3);  
                     b = atoi(pt.c_str());
                  } else if (eitem[3] == 'C') {
                     multiplier = 100;
                     pt = eitem.substr(0,3); 
                     b = atoi(pt.c_str());
                  } else if (eitem[3] == 'c') {
                     multiplier = -100;
                     pt = eitem.substr(0,3);
                     b = atoi(pt.c_str());
                  } else if (eitem[3] == 'M') {
                     multiplier = 1000;
                     pt = eitem.substr(0,3);
                     b = 99;   //will give sub-mate val of 99,000
                  } else if (eitem[3] == 'm') {
                     multiplier = -1000;
                     pt = eitem.substr(0,3);
                     b = 99;
                  } else if (eitem.substr(0,2) == "+M") {  
                     pt = eitem.substr(2);    //might be longer than 2 more chars
                     b = IRutil::MATEVALCP - atoi(pt.c_str());
                  } else if (eitem.substr(0,2) == "-M") {
                     pt = eitem.substr(2);
                     b = atoi(pt.c_str()) - IRutil::MATEVALCP;
                  } else {   //simple value!
                     b = atoi(eitem.c_str());
                  }
                  b *= multiplier;
               }  
               evalsCP.push_back(b); 
               multiplier = 1; 
               currDepth++; 
            } 
 
            //Update maxEvalPerDepth Now  
            depthEvalsCP->push_back(evalsCP); 
         } 
         if (playedMoveIndex == -1) { 
             //cerr << "played Move Index not found " << endl; 
             //cerr << "Current Line Number: " << lineNumber<<endl; 
             //cerr << "FEN: " << fenCurrPos << endl;
             //cerr << "Move played: " << movePlayed << " in " << movestr << endl;
             //exit(1); 
         } 

         getline(infile,line);  //should be blank line in old & new format
         lineNumber++;
 
         //At this point, we have information for a particular move. 
         // We will push that to turns vector.   !!!

         lowDepth += AIFoffset;
         highDepth += AIFoffset;
 
/*
   TurnInfo(const GameInfo* pGmInfo, const int lo, const int hi,
            const string& gid, const string& eid, const int tNumber, const bool wtm,
            const string& plydMove, const string& engMove,
            const int curEval, const int preEval, const int nexEval,
            const int d, const int nc,  const string& fenCurrPos, const int fmr,
            const int repCount, const string& rl1, const string& rl2,
            const size_t numLegalMov, const string& legMoves,
            vector<vector<int> >* devals,
            const int playedMoveInd, bool extraRepeatFlag = false)
*/
			 
         TurnInfo* pSingleTurn = new TurnInfo( 
            pGameInfo, mlo, mhi, gID, eID, turnNumber, whiteMove,
            movePlayed, engineMove, 
            eval, prevEval, nextEval, depth, nodeCount, fenCurrPos, fiftyMR,
            repCount, repMoves, repLine2, numLegalMoves, legalMoves, 
            consideredMovesVec, depthEvalsCP, playedMoveIndex, false);
            
         if (referenceOnly) {   //no need to link up turns
            pReferenceTurns->push_back(pSingleTurn);
         } else {
            if (prevEval == IRutil::NAVAL) {   //first turn in game
                pGameInfo->gameTurns->push_back(pSingleTurn);
                fillTurns->push_back(pSingleTurn);
            } else {  //lastTurn must be assigned from same game
               if (lastTurn != NULL) {
                  lastTurn->nextTurn = pSingleTurn;
                  pSingleTurn->prevTurn = lastTurn;
                  fillTurns->push_back(pSingleTurn); 
               } else {
                  cerr << "Saved a segfault on turn " << string(*pSingleTurn) << endl;
               }
            }
         } 
         lastTurn = pSingleTurn;

         //depthEvalsCP->clear(); 

         //Now mark repetitions.  Definition is all moves from the first
         //occurrence of a position, up to---but not including---its last
         //occurrence, are marked as repetitions (if not already marked).
         //Last occurrence is marked only if it is last move of game.
         //This implies that only the first move that breaks a repeating
         //sequence is unmarked.

         if (legacy && repCount > 0) {
            list<TurnInfo*>* ptr = referenceOnly ? pReferenceTurns : fillTurns;
            string fenpos = fenCurrPos.substr(0,fenCurrPos.find(" -"));
            string fenpos2;
            list<TurnInfo*>::iterator itr = --(ptr->end());
            if (nextEvalStr == " n.a.") { 
               (*itr)->setRep(true); //only automatic on last move of game
            }
            while (itr != ptr->begin()) {
               const GameInfo* gi = (*(--itr))->pGameInfo;
		  fenpos2 = (*itr)->fenCurrentPos.substr(0, (*itr)->fenCurrentPos.find(" -"));
               if (gi != pGameInfo || fenpos == fenpos2) {
                  break;
               } else {
                  (*itr)->setRep(true);
               }
            }
            fenpos2 = (*itr)->fenCurrentPos.substr(0, (*itr)->fenCurrentPos.find(" -"));
            if (fenpos == fenpos2) {
               (*itr)->setRep(true);
            } else {
               cerr << "Repetition bound not found for " << endl
                    << string(*pSingleTurn) << endl << endl
                    << string(**itr) << endl;
            }
         }
      }
   }
cerr << "Done reading " << lineNumber << " lines." << endl;
   return fillTurns;
}

 
 
/** Used to check that the input is read correctly. 
 */ 
void Ensemble::printDeltas(const string& name) const { 
   focusTrial->dumpDeltas(name);
} 
 
void Ensemble::printTurns(const string& name) const { 
   ofstream* file = new ofstream(name.c_str(), ios::app); 
   if(!file->is_open()) { 
      cerr << "Cannot stat the output file, returning..." << endl; 
      return; 
   } //else 
   for(list<TurnInfo*>::const_iterator tu = pTurns->begin(); 
      tu != pTurns->end(); tu++) { 
 
      (*file) << string(**tu) << endl << endl; 
   } 
} 
 
 
void Ensemble::newTrial() { 
   string trialName; 
   string ans; 
   double doubleAns; 
   int intAns; 
   bool scaleDeltas; 
   Models::MODEL model = Models::MODEL(modelsMenu->readChoice()); 
   cout << "Enter spec (curve, weighting, initial point) for trial---\n"; 
   TrialSpec ts = makeTrialSpec(); 
   specs->clearStars(); 
   specs->addItem(ts,"",false,true); 
   cout << "Impose distortion fix? (y/n) "; 
   loggedRead(ans,false); 
   ans = IRutil::trim(ans); 
   bool fixDistortion = IRutil::isYes(ans); 
 
   cout << "Adjust delta scale? (y/n): "; 
   loggedRead(ans,false); 
   scaleDeltas = IRutil::isYes(IRutil::trim(ans)); 
   DeltaScale* ds; 
   scales->clearStars(); 
   SwingInfo* swi = new SwingInfo(map<APPLICATION_POLICIES,double>(), true, false, true,
                                  false, false, 0, false, false, 0.0);
   if (scaleDeltas) { 
      //ds = makeDeltaScale(doubleAns);  //must pass in cap if trial not made 
      ds = makeDeltaScale();
      ds->setDeltaCap(doubleAns);
   } else { 
      ds = new UnitScale(false, swi, NO_PATCH, IRutil::CENTRALPATCHPOWER, 0.00,
                         map<string,double>(), 1, 30, 30, 6, 20, 0, doubleAns);  
   } 
   scales->addItem(ds,"",false,true); 
 
   cout << "Enter name for trial: "; 
   loggedRead(trialName,true);   //end line defining trial 
   trialName = IRutil::trim(trialName); 
   focusTrial = new Trial(trialName,pTurns,ts, //percs,
                          model,fixDistortion,scaleDeltas,ds); 
   //focusTrial->addFilteredTuples(); 
   filters->clearStars(); 
   trials->clearStars(); 
   trials->addItem(focusTrial,"",false,true);  //adds when name is new, *s it 
   if (pTurns->size() == 0) { 
      readTurns(*INP);  //difference from addTurns() is latter filters too. 
   } 
} 
 
void Ensemble::changeTrial() {
   Models::MODEL model = Models::MODEL(modelsMenu->readChoice());
   focusTrial->setModel(model);
   referenceTrial->setModel(model);
   string ans = (focusTrial->getFixPolicy() ? "y" : "n");
   cout << "Impose distortion fix? (y/n) (currently " + ans + ") ";
   loggedRead(ans,false);
   ans = IRutil::trim(ans);
   bool fixDistortion = IRutil::isYes(ans);
   focusTrial->setFixPolicy(fixDistortion);
   referenceTrial->setFixPolicy(fixDistortion);
   ans = (focusTrial->getMoveNNPolicy() ? "y" : "n");
   cout << "Treat miss as Move NN? (y/n) (currently " + ans + ") ";
   loggedRead(ans,false);
   ans = IRutil::trim(ans);
   bool moveNNPolicy = IRutil::isYes(ans);
   focusTrial->setMoveNNPolicy(moveNNPolicy);
   referenceTrial->setMoveNNPolicy(moveNNPolicy);
}

void Ensemble::loadNamedTrial() { 
   if (trials->empty()) { 
      cout << "No trials to choose from, sorry." << endl; 
      return; 
   } //else 
   int i = trials->readChoice(); 
   if (trials->success(i)) { 
      string name = trials->nameOf(i); 
      focusTrial = (*trials)[name]; 
      trials->clearStars(); 
      trials->setStarred(name,true); 
   } else { 
      while (i == EnumMenu::INVALID_CHOICE) { 
         cout << "Invalid choice---please try again: " << endl; 
         i = trials->readChoice(false);  //so menu doesn't display again 
      } 
   } 
   //in cancel/more-options/abort cases, do nothing... 
} 
 
void Ensemble::loadDeltaScale(bool refToo) { 
   int choice = scales->readChoice(); 
   DeltaScale* ds; 
   if (scales->success(choice)) { 
      string name = scales->nameOf(choice); 
      ds = (*scales)[name]; 
      focusTrial->setScale(ds); 
      if (refToo) { 
         cout << "Changing reference trial scale too...";
         referenceTrial->setScale(ds); 
      }
      scales->clearStars(); 
      scales->setStarred(name,true); 
   } else if (choice == EnumMenu::MORE_CHOICE) { 
      //ds = makeDeltaScale(focusTrial->getDeltaCap()); 
      ds = makeDeltaScale();
      focusTrial->setScale(ds); 
      if (refToo) {
         cout << "Changing reference trial to new scale too...";
         referenceTrial->setScale(ds);
      }
      scales->clearStars(); 
      scales->addItem(ds,"",false,true); 
   } else { 
      //do nothing 
   } 
   //reset();   //unnecessary?
} 
 
 
void Ensemble::loadNamedTrialSpec() { 
   int choice; 
   if (specs->empty()) { 
      cout << "No active trial specs to choose from, showing inactive ones..." 
           << endl; 
      choice = EnumMenu::MORE_CHOICE; 
   } else { 
      choice = specs->readChoice(); 
   } 
   if (specs->success(choice)) { 
      string name = specs->nameOf(choice); 
      TrialSpec ts = (*specs)[name]; 
      focusTrial->setSpec(ts); 
      referenceTrial->setSpec(ts);
      resetParamsMenu();
      specs->clearStars(); 
      specs->setStarred(name,true); 
   } else if (choice == EnumMenu::MORE_CHOICE && !hiddenSpecs->empty()) { 
      choice = hiddenSpecs->readChoice(); 
      if (hiddenSpecs->success(choice)) { 
         string name = hiddenSpecs->nameOf(choice); 
         TrialSpec ts = (*hiddenSpecs)[name]; 
         focusTrial->setSpec(ts); 
         referenceTrial->setSpec(ts);
         resetParamsMenu();
         specs->clearStars(); 
         specs->addItem(ts,"",false,true);       //makes it active and stars it 
         hiddenSpecs->removeItem(name); 
      } else { 
         cout << "Invalid choice, canceling." << endl; 
         //do nothing. 
      } 
   } else if (hiddenSpecs->empty()) { 
      cout << "No inactive specs to choose from, returning..." << endl; 
   } else { 
      //do nothing 
   } 
} 
 
 
void Ensemble::hideSpecs() { 
   string ans = "yes"; 
   bool cancel = false; 
   int choice; 
   while (IRutil::isYes(ans) && specs->size() > 1) { 
      choice = specs->readChoice(); 
      if (specs->success(choice)) { 
         if (specs->isStarred(choice)) {     // (specs->nameOf(choice))) { 
            cout << "Can't inactivate the current spec, sorry." << endl; 
         } else { 
            string name = specs->nameOf(choice); 
            hiddenSpecs->addItem((*specs)[name], "", false, false); 
            specs->removeItem(name); 
         } 
      } else { 
         cout << "Invalid choice, returning..."; 
         cancel = true; 
      } 
      if (usingCin() && !cancel) { cout << "Inactivate another? "; } 
      loggedRead(ans); 
   } 
   if ((cancel || specs->size() == 1) && usingCin()) { 
      cout << "Had to leave the loaded spec active." << endl; 
   } 
} 
 
/*
   Minimizer(Trial* trial, METHOD gmethod, map<MINIMAND,double> gmini,
             size_t ifs, size_t ife, SKED ifsked, size_t ids, size_t ide, SKED idsked,
             double e1lo, double e1hi, double e2lo, double e2hi,
             double mu, double mud, bool upv,
             const TrialSpec loSpec, const TrialSpec hSpec, const TrialSpec pivSpec,
             unsigned int max_iter, size_t num_retries,
             double precx, double precy, double precg,
             bool wout = false, double woutstep = 0.02,
             double bn = 0, bool ksib = false, bool vb = false,
             vector<ostream*>* gouts = new vector<ostream*>())

   enum METHOD {
      VWALK, GSL_NM4, GSL_NM6, GSL_NM7, GSL_SIM_ANN, CMINPACK_LMDIF, CMINPACK_HYBRD, NUM_METHODS
   };
   enum SKED {
      UNIT_SKED, SQRT_SKED, LIN_SKED, QUAD_SKED, INVVAR_SKED,
      SQRT_ALLWT, LIN_ALLWT, QUAD_ALLWT, NUM_SKEDS
   };
   enum MINIMAND {
      PERCFIT, ML,  //PERCFITOLD, PERCFITOLDL1,
         FALLOFF, FIRST_LINE, ETV, SECOND_LINE, THIRD_LINE, FOURTH_LINE, FIFTH_LINE,
         INDEXFIT, INDEXDIFF, ERROR1, ERROR2,
         //INDEXFITMASS, INDEXDIFFMASS, INDEXFITINVVAR, INDEXFITPROP,
         FIRSTL1A, ETVL1A, BINS, NUM_MINIMANDS
   };
*/

Minimizer::SKED Ensemble::parseSked(string x) const {
   for (int i = Minimizer::UNIT_SKED; i < Minimizer::NUM_SKEDS; i++) {
      if (x.find(skedMenu->nameOf(Minimizer::SKED(i))) != string::npos) {
         return Minimizer::SKED(i);
      }
   }
   return Minimizer::UNIT_SKED;
}
 
Minimizer* Ensemble::makeMinimizer() const { 
   IRutil::nullstream nullstrm; 
   ostream& ccout = (usingCin()) ? cout : nullstrm; 
   string ans; 
   int choice = 0; 
   int choice2 = 0;
   Minimizer::METHOD method = Minimizer::METHOD(methodMenu->indexOf(fitMenu->getStatus(MIN_METHOD)));
   size_t equalTopIndexLimit = IRutil::extractDoubles(fitMenu->getStatus(Minimizer::ETV))[0];
   Minimizer::SKED ifsked = parseSked(fitMenu->getStatus(Minimizer::INDEX_FIT));
   Minimizer::SKED idsked = parseSked(fitMenu->getStatus(Minimizer::INDEX_DIFF));
   Minimizer::SKED e1sked = parseSked(fitMenu->getStatus(Minimizer::ERROR1));
   Minimizer::SKED e2sked = parseSked(fitMenu->getStatus(Minimizer::ERROR2));
                                           //Minimizer::GSL_MULTIMIN;
   //Minimizer::MINIMAND mini;
   map<Minimizer::MINIMAND, double> minimap; //constructed here
   Minimizer* mm; 
   double loS, hiS, loC, hiC, loD, hiD, loV, hiV, loA, hiA, loHM, hiHM, loHP, hiHP, loB, hiB;
   size_t ifs, ife, ids, ide;
   double e1lo, e1hi, e2lo, e2hi;
   IRfun::CURVES slideCurve;
   double slideLo, slideMid, slideHi, slidePower, slideProp;
   double mumul, mudiv, precx, precy, precg, step, epsfcn;
   bool slides, gm, wo, si, vb, ul, upv, ksib;
   bool entering = true;
   bool innerEntering;
   size_t bootNum = 0;
   //size_t fjd;   //0 means turn maximum depth used for judgment
   size_t maxIter = 2000;
   size_t numRetries = 50;
   int intItem, intItem2, intItem3;
   double floatItem, floatItem2, floatItem3;
   TrialSpec ts = focusTrial->getSpec();
   fitMenu->resetDefaults(); 
   string tss = string(ts);
   TrialSpec loSpec, hiSpec, pivSpec;
   map<IRfun::CURVES, double> cb;

   string strparams = ts.getDescription();
   fitMenu->setStatus(START_SPEC, ts.getName() + ": " + strparams);
   
   while (entering) { 
      choice = fitMenu->readChoice(); 
      switch(choice) { 
       case Minimizer::PERCFIT: 
       //case Minimizer::PERCFITOLD: 
       //case Minimizer::PERCFITOLDL1: 
       case Minimizer::ML: 
       case Minimizer::BINS:
/*
       case Minimizer::INDEXFITMASS: 
       case Minimizer::INDEXDIFFMASS:
       case Minimizer::INDEXFITINVVAR: 
       case Minimizer::INDEXFITPROP: 
*/
       case Minimizer::FALLOFF:
       case Minimizer::FIRST_LINE:
       case Minimizer::SECOND_LINE:
       case Minimizer::THIRD_LINE:
       //case Minimizer::FOURTH_LINE:
       //case Minimizer::FIFTH_LINE:

       //case Minimizer::FIRSTL1A: 
       //case Minimizer::ETVL1A: 
         //fitMenu->clearStars(); 
         ccout << "Enter float weight, 0 to exclude: ";
         loggedRead(floatItem);
         fitMenu->setStatus(choice, IRutil::ftoa(floatItem));
         if (floatItem != 0) {   //OK comparison
            //minimap[choice] = floatItem;
            fitMenu->setStarred(choice,true);
         } else {
            fitMenu->setStarred(choice,false);
         }
         break; 
   
       case Minimizer::ETV:
         ccout << "Enter 0-based inclusive index limit: ";
         loggedReadBounded(intItem, 0, IRutil::NCUTOFF,false);
         ccout << "Enter float weight, 0 to exclude: ";
         loggedRead(floatItem);
         fitMenu->setStatus(choice, IRutil::itoa(intItem) + ": " + IRutil::ftoa(floatItem));
         if (floatItem != 0) {   //OK comparison
            //minimap[choice] = floatItem;
            fitMenu->setStarred(choice,true);
         } else {
            fitMenu->setStarred(choice,false);
         }
         break;

       case Minimizer::INDEX_FIT:
         ccout << "Enter first index, 0-based: ";
         loggedReadBounded(intItem, 0, IRutil::NCUTOFF,false);
         ccout << "Enter end index, 0-based: ";
         loggedReadBounded(intItem2, intItem,IRutil::NCUTOFF,false);
         innerEntering = true;
         while (innerEntering) {
            choice2 = skedMenu->readChoice();
            if (0 <= choice2 && choice2 < Minimizer::NUM_SKEDS) {
               innerEntering = false;
            } else {
               ccout << "Invalid choice; please try again: " << endl;
            }
         }
         ccout << "Enter float weight, 0 to exclude: ";
         loggedRead(floatItem);
         ifsked = Minimizer::SKED(choice2);
         fitMenu->setStatus(choice, "[" + IRutil::itoa(intItem) + ", " + IRutil::itoa(intItem2)
                               + "] " + skedMenu->nameOf(choice2) + ": " + IRutil::ftoa(floatItem));
         if (floatItem != 0) {   //OK comparison
            //minimap[choice] = floatItem;
            fitMenu->setStarred(choice,true);
         } else {
            fitMenu->setStarred(choice,false);
         }
         break;
       case Minimizer::INDEX_DIFF:
         ccout << "Enter first index, 0-based: ";
         loggedReadBounded(intItem, 0, IRutil::NCUTOFF,false);
         ccout << "Enter end index, 0-based: ";
         loggedReadBounded(intItem2, intItem,IRutil::NCUTOFF,false);
         innerEntering = true;
         while (innerEntering) {
            choice2 = skedMenu->readChoice();
            if (0 <= choice2 && choice2 < Minimizer::NUM_SKEDS) {
               innerEntering = false;
            } else {
               ccout << "Invalid choice; please try again: " << endl;
            }
         }
         ccout << "Enter float weight, 0 to exclude: ";
         loggedRead(floatItem);
         fitMenu->setStatus(choice, "[" + IRutil::itoa(intItem) + ", " + IRutil::itoa(intItem2)
                               + "] " + skedMenu->nameOf(choice2) + ": " + IRutil::ftoa(floatItem));
         if (floatItem != 0) {   //OK comparison
            //minimap[choice] = floatItem;
            fitMenu->setStarred(choice,true);
         } else {
            fitMenu->setStarred(choice,false);
         }
         break;
       case Minimizer::ERROR1:
         ccout << "Enter min. value p.nn for error: ";
         loggedReadBounded(floatItem2, 0.00, 2*IRutil::MATEVAL, false);
         ccout << "Enter max. value for error, " << (2*IRutil::MATEVAL) << " for none: ";
         loggedReadBounded(floatItem3, floatItem2, 2*IRutil::MATEVAL, false);
         innerEntering = true;
         while (innerEntering) {
            choice2 = skedMenu->readChoice();
            if (0 <= choice2 && choice2 < Minimizer::NUM_SKEDS) {
               innerEntering = false;
            } else {
               ccout << "Invalid choice; please try again: " << endl;
            }
         }
         e1sked = Minimizer::SKED(choice2);
         ccout << "Enter float weight, 0 to exclude: ";
         loggedRead(floatItem);
         fitMenu->setStatus(choice, "[" + IRutil::ftoa(floatItem2) + ", " + IRutil::ftoa(floatItem3)
                               + "] " + skedMenu->nameOf(choice2) + ": " + IRutil::ftoa(floatItem));
         if (floatItem != 0) {   //OK comparison
            //minimap[choice] = floatItem;
            fitMenu->setStarred(choice,true);
         } else {
            fitMenu->setStarred(choice,false);
         }
         break;

       case Minimizer::ERROR2:
         ccout << "Enter min. value p.nn for error: ";
         loggedReadBounded(floatItem2, 0.00, 2*IRutil::MATEVAL, false);
         ccout << "Enter max. value for error, " << (2*IRutil::MATEVAL) << " for none: ";
         loggedReadBounded(floatItem3, floatItem2, 2*IRutil::MATEVAL, false);
         innerEntering = true;
         while (innerEntering) {
            choice2 = skedMenu->readChoice();
            if (0 <= choice2 && choice2 < Minimizer::NUM_SKEDS) {
               innerEntering = false;
            } else {
               ccout << "Invalid choice; please try again: " << endl;
            }
         }
         e2sked = Minimizer::SKED(choice2);   //only difference from previous block
         ccout << "Enter float weight, 0 to exclude: ";
         loggedRead(floatItem);
         fitMenu->setStatus(choice, "[" + IRutil::ftoa(floatItem2) + ", " + IRutil::ftoa(floatItem3)
                               + "] " + skedMenu->nameOf(choice2) + ": " + IRutil::ftoa(floatItem));
         if (floatItem != 0) {   //OK comparison
            //minimap[choice] = floatItem;
            fitMenu->setStarred(choice,true);
         } else {
            fitMenu->setStarred(choice,false);
         }
         break;
       case FUNNEL_FACTORS:
         ccout << "Enter positive starting \"mu\" multiplier or 0 to exclude: ";
         loggedRead(floatItem);
         fitMenu->setStatus(choice, IRutil::ftoa(floatItem));
         if (floatItem != 0) {   //OK comparison
            ccout << "Enter divisor for update: ";
            loggedRead(floatItem2);
            fitMenu->setStatus(choice, IRutil::ftoa(floatItem) + ", update dividing by " + IRutil::ftoa(floatItem2));
            fitMenu->setStarred(choice,true);
         } else {
            fitMenu->setStarred(choice,false);
         }
         break;

       case GO: 
         LOGP->flush();

         loS = IRutil::extractDoubles(fitMenu->getStatus(S_BOUNDS))[0]; 
         hiS = IRutil::extractDoubles(fitMenu->getStatus(S_BOUNDS))[1]; 
         loC = IRutil::extractDoubles(fitMenu->getStatus(C_BOUNDS))[0]; 
         hiC = IRutil::extractDoubles(fitMenu->getStatus(C_BOUNDS))[1]; 
         loD = IRutil::extractDoubles(fitMenu->getStatus(D_BOUNDS))[0];
         hiD = IRutil::extractDoubles(fitMenu->getStatus(D_BOUNDS))[1];
         loV = IRutil::extractDoubles(fitMenu->getStatus(V_BOUNDS))[0];
         hiV = IRutil::extractDoubles(fitMenu->getStatus(V_BOUNDS))[1];
         loA = IRutil::extractDoubles(fitMenu->getStatus(A_BOUNDS))[0];
         hiA = IRutil::extractDoubles(fitMenu->getStatus(A_BOUNDS))[1];
         loHM = IRutil::extractDoubles(fitMenu->getStatus(HM_BOUNDS))[0];
         hiHM = IRutil::extractDoubles(fitMenu->getStatus(HM_BOUNDS))[1];
         loHP = IRutil::extractDoubles(fitMenu->getStatus(HP_BOUNDS))[0];
         hiHP = IRutil::extractDoubles(fitMenu->getStatus(HP_BOUNDS))[1];
         loB = IRutil::extractDoubles(fitMenu->getStatus(B_BOUNDS))[0];
         hiB = IRutil::extractDoubles(fitMenu->getStatus(B_BOUNDS))[1];

         precx = atof(fitMenu->getStatus(PRECISION_X).c_str());

         if (hiS - loS <= precx && hiC - loC <= precx && hiD - loD <= precx && hiV - loV <= precx
             && hiA - loA <= precx && hiHM - loHM <= precx && hiHP - loHP <= precx && hiB - loB <= precx) {
            cerr << "Please unleash at least one of the variables." << endl;
            break;
         }

         //fjd = atoi((fitMenu->getStatus(FIT_JUDGMENT_DEPTH)).c_str());
         ifs = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::INDEX_FIT)).c_str())[0];
         ife = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::INDEX_FIT)).c_str())[1];
         //SKED choices maintained inductively
         ids = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::INDEX_DIFF)).c_str())[0];
         ide = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::INDEX_DIFF)).c_str())[1];

         e1lo = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::ERROR1)).c_str())[0];
         e1hi = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::ERROR1)).c_str())[1];
         e2lo = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::ERROR2)).c_str())[0];
         e2hi = IRutil::extractDoubles((fitMenu->getStatus(Minimizer::ERROR2)).c_str())[1];

         mumul = IRutil::extractDoubles((fitMenu->getStatus(FUNNEL_FACTORS)).c_str())[0];
         mudiv = IRutil::extractDoubles((fitMenu->getStatus(FUNNEL_FACTORS)).c_str())[1];
         maxIter = atoi((fitMenu->getStatus(MAX_ITERATIONS)).c_str());
         numRetries = atoi((fitMenu->getStatus(NUM_RETRIES)).c_str());
         //prec = atof((fitMenu->getStatus(PRECISION)).c_str()); 
         //precx = atof(fitMenu->getStatus(PRECISION_X).c_str());
	 precy = atof(fitMenu->getStatus(PRECISION_Y).c_str());
	 precg = atof(fitMenu->getStatus(PRECISION_G).c_str());
         step = atof((fitMenu->getStatus(WALKOUT_STEP)).c_str()); 
         epsfcn = atof((fitMenu->getStatus(EPSFCN)).c_str());
         //gm = IRutil::isYes(fitMenu->getStatus(GEOMETRIC_MEANS)); 
         //wo = IRutil::isYes(fitMenu->getStatus(WALKOUT)); 
         //si = IRutil::isYes(fitMenu->getStatus(S_INNER)); 
         vb = IRutil::isYes(fitMenu->getStatus(VERBOSE)); 
         ul = IRutil::isYes(fitMenu->getStatus(MIN_METHOD));
         bootNum = atoi(fitMenu->getStatus(BOOT_NUM).c_str());
         ksib = IRutil::isYes(fitMenu->getStatus(KEEP_SPEC_IN_BOOT));

         cb = ts.curveBook;
         loSpec = TrialSpec(ts.curveChoice,cb,ts.weightMethodChoice,"loSpec",
                            loS,loC,loD,loV,loA,loHM,loHP,loB, ts.slides,
                            ts.slideCurveChoice, ts.slideLo, ts.slideMid,
                            ts.slideHi, ts.slidePower, ts.slideProp, ts.memoStep, 0.0); //don't memoize
         hiSpec = TrialSpec(ts.curveChoice,cb,ts.weightMethodChoice,"hiSpec",
                            hiS,hiC,hiD,hiV,hiA,hiHM,hiHP,hiB, ts.slides,
                            ts.slideCurveChoice, ts.slideLo, ts.slideMid,
                            ts.slideHi, ts.slidePower, ts.slideProp, ts.memoStep, 0.0);
         pivSpec = TrialSpec(ts.curveChoice,cb,ts.weightMethodChoice,ts.getName(),
                             ts.s,ts.c,ts.d,ts.v,ts.a,ts.hm,ts.hp,ts.b, ts.slides,
                             ts.slideCurveChoice, ts.slideLo, ts.slideMid,
                             ts.slideHi, ts.slidePower, ts.slideProp, ts.memoStep, ts.memoCap);
   

         for (size_t ch = Minimizer::PERCFIT; ch <= Minimizer::THIRD_LINE; ch++) {
            float mul = atof((fitMenu->getStatus(Minimizer::MINIMAND(ch))).c_str());
            if (mul != 0) {   //OK comparison
               minimap[Minimizer::MINIMAND(ch)] = mul;
            }
         }

         equalTopIndexLimit = IRutil::extractDoubles(fitMenu->getStatus(Minimizer::ETV))[0];
         floatItem = IRutil::extractDoubles(fitMenu->getStatus(Minimizer::ETV))[1];
         if (floatItem != 0) {   //OK comparison
            minimap[Minimizer::ETV] = floatItem;
         }
         for (size_t ch = Minimizer::INDEX_FIT; ch <= Minimizer::ERROR2; ch++) {
            float mul = IRutil::extractDoubles(fitMenu->getStatus(Minimizer::MINIMAND(ch)))[2];
            if (mul != 0) {   //OK comparison
               minimap[Minimizer::MINIMAND(ch)] = mul;
            }
         }

         if (minimap.size() == 0) {
            cout << "No test selected.  Please give at least one test a nonzero weight." << endl;
            break;
         }

       
         //choice = 1;
         //while (!(fitMenu->isStarred(choice))) { choice++; }
         //mini = Minimizer::MINIMAND(choice);   //!!! todo: makew this more robust
 
         //only place a Minimizer is created
         focusTrial->setSpec(ts);
         referenceTrial->setSpec(ts);
         //resetParamsMenu();
         mm = new Minimizer(focusTrial, method, minimap, equalTopIndexLimit,
                            ifs, ife, ifsked, ids, ide, idsked, 
                            e1lo, e1hi, e1sked, e2lo, e2hi, e2sked,
                            mumul, mudiv, loSpec, hiSpec, pivSpec,
                            maxIter, numRetries, precx, precy, precg,
                            step > 0.0, step, epsfcn, bootNum, ksib, vb, outs);
         entering = false;
         break; 

       case MIN_METHOD:
         choice2 = methodMenu->readChoice();
         fitMenu->setStatus(choice, methodMenu->nameOf(choice2));
         method = Minimizer::METHOD(choice2);
         break;
       //case GEOMETRIC_MEANS: 
       //case WALKOUT: 
       //case S_INNER: 
       case VERBOSE: 
         ccout << "Yes or No? (y/n) "; 
         loggedRead(ans); 
         ans = IRutil::trim(ans); 
         fitMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no"); 
         break; 
       case S_BOUNDS: 
         ccout<<"Enter lower and upper bounds for `s' on next line, or 1 0 to freeze spec point value:"<<endl; 
         loggedRead(loS,false); 
         loggedRead(hiS,false); 
         if (loS < hiS) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loS > hiS) {
               loS = hiS = ts.s;
            } else {
               hiS = loS;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loS) + " .. "
                                  + IRutil::ftoa(hiS));
         break; 
       case C_BOUNDS: 
         ccout << "Enter lower and upper bounds for `c' on next line, or 1 0 to freeze spec point value:" << endl; 
         loggedRead(loC,false); 
         loggedRead(hiC,false); 
         if (loC < hiC) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loC > hiC) {
               loC = hiC = ts.c;
            } else {
               hiC = loC;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loC) + " .. "
                                  + IRutil::ftoa(hiC));
         break; 
       case D_BOUNDS:
         ccout << "Enter lower and upper bounds for `d' on next line, or 1 0 to freeze spec point value:" << endl;
         loggedRead(loD,false);
         loggedRead(hiD,false);
         if (loD < hiD) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loD > hiD) {
               loD = hiD = ts.d;
            } else {
               hiD = loD;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loD) + " .. "
                                  + IRutil::ftoa(hiD));
         break;
       case V_BOUNDS:
         ccout << "Enter lower and upper bounds for `v' on next line, or 1 0 to freeze spec point value:" << endl;
         loggedRead(loV,false);
         loggedRead(hiV,false);
         if (loV < hiV) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loV > hiV) {
               loV = hiV = ts.v;
            } else {
               hiV = loV;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loV) + " .. "
                                  + IRutil::ftoa(hiV));
         break;
       case A_BOUNDS:
         ccout << "Enter lower and upper bounds for `a' on next line, or 1 0 to freeze spec point value:" << endl;
         loggedRead(loA,false);
         loggedRead(hiA,false);
         if (loA < hiA) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loA > hiA) {
               loA = hiA = ts.a;
            } else {
               hiA = loA;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loA) + " .. "
                                  + IRutil::ftoa(hiA));
         break;
       case HM_BOUNDS:
         ccout << "Enter lower and upper bounds for `hm' on next line, or 1 0 to freeze spec point value:" << endl;
         loggedRead(loHM,false);
         loggedRead(hiHM,false);
         if (loHM < hiHM) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loHM > hiHM) {
               loHM = hiHM = ts.hm;
            } else {
               hiHM = loHM;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loHM) + " .. "
                                  + IRutil::ftoa(hiHM));
         break;
       case HP_BOUNDS:
         ccout << "Enter lower and upper bounds for `hp' on next line, or 1 0 to freeze spec point value:" << endl;
         loggedRead(loHP,false);
         loggedRead(hiHP,false);
         if (loHP < hiHP) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loHP > hiHP) {
               loHP = hiHP = ts.hp;
            } else {
               hiHP = loHP;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loHP) + " .. "
                                  + IRutil::ftoa(hiHP));
         break;
       case B_BOUNDS:
         ccout << "Enter lower and upper bounds for `b' on next line, or 1 0 to freeze spec point value:" << endl;
         loggedRead(loB,false);
         loggedRead(hiB,false);
         if (loB < hiB) {
            fitMenu->setStarred(choice,true);
         } else {
            if (loB > hiB) {
               loB = hiB = ts.b;
            } else {
               hiB = loB;
            }
            fitMenu->setStarred(choice,false);
         }
         fitMenu->setStatus(choice, IRutil::ftoa(loB) + " .. "
                                  + IRutil::ftoa(hiB));
         break;
       case START_SPEC:
         ts = makeTrialSpec();
         tss = string(ts);
         strparams = ts.getDescription();
         fitMenu->setStatus(START_SPEC, ts.getName() + ": " + strparams);
         //fitMenu->setStatus(FIT_JUDGMENT_DEPTH, IRutil::itoa(ts.judgmentDepth));
         focusTrial->setSpec(ts);  //done also in "GO..."
         referenceTrial->setSpec(ts);
         //resetParamsMenu();
         break;
       //case FIT_JUDGMENT_DEPTH:
         //ccout << "Enter depth for judgment (0 for turn max): ";
         //loggedRead(fjd, false);
         //fitMenu->setStatus(choice, IRutil::itoa(fjd));
         //break;
       case MAX_ITERATIONS:
         ccout << "Enter limit on number of iterations: ";
         loggedRead(maxIter, false);
         fitMenu->setStatus(choice, IRutil::itoa(maxIter));
         break;
       case NUM_RETRIES:
         ccout << "Enter number of retries (per temp level in SA): ";
         loggedRead(numRetries, false);
         fitMenu->setStatus(choice, IRutil::itoa(numRetries));
         break;
       case PRECISION_X: 
         ccout << "Precision for 'x' lines: "; 
         loggedRead(precx,false); 
         fitMenu->setStatus(choice, IRutil::ftoa(precx));
         break;
       case PRECISION_Y:
         ccout << "Precision for 'y' values: ";
         loggedRead(precy,false);
         fitMenu->setStatus(choice, IRutil::ftoa(precy));
         break;
       case PRECISION_G:
         ccout << "Precision for geometry: " << endl;
         loggedRead(precg,false);
         fitMenu->setStatus(choice, IRutil::ftoa(precg));
         break; 
       case WALKOUT_STEP:
         ccout << "Enter new initial walkOut step: " << endl;
         loggedRead(step,false);
         fitMenu->setStatus(choice, IRutil::ftoa(step));
         break;
       case EPSFCN:
         ccout << "Enter epsfcn: " << endl;
         loggedRead(epsfcn,false);
         fitMenu->setStatus(choice, IRutil::ftoa(epsfcn));
         break;
       case BOOT_NUM:
         ccout << "Enter number of bootstrap trials: " << endl;
         loggedRead(bootNum,false);
         fitMenu->setStatus(choice, IRutil::itoa(bootNum));
         break;
       case KEEP_SPEC_IN_BOOT:
         ccout << "Keep regressed parameters in bootstrap trials?" << endl;
         loggedRead(ans,false);
         ans = IRutil::trim(ans);
         fitMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no");
         break;
       //case COPY_SCALE_TO_REFERENCE:
         //ccout << "Copy current scale to reference? (y/n) ";
         //loggedRead(ans);
         //ans = IRutil::trim(ans);
         //fitMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no");
       case EnumMenu::CANCEL_CHOICE: 
         cout << "Can't cancel, sorry---please try again." << endl; 
         break; 
       default: 
         cout << "Invalid choice, please try again." << endl;  //not ccout 
         break; 
      } 
      ccout << endl; 
   } 
   fitMenu->resetDefaults(); 
   return mm; 
} 
 
void Ensemble::runFit() {   
//Conceptualize as updating the TrialSpec.
   LOGP->flush();
   IRutil::nullstream nullstr; 
   ostream& ccout = usingCin() ? cout : nullstr; 
   string ans; 
   if (focusTrial->numDecisions() == 0) { 
      cout << "Empty data: please change filters and/or add game turns." << endl;  //not ccout 
   } else { 

      //reset static values 
      IRutil::numNewtons = 0; IRutil::maxNewtons = 0; IRutil::numVRCalls = 0; 
 
      TrialSpec ts0 = focusTrial->getSpec();  //used for all trials

      Minimizer* mm = makeMinimizer();  //only place Minimizer is used
                                        //includes the bootstrap number in mm.

      if (mm->getMaxIter() <= 0) {     //user only wants to print one value
         double val = mm->minimand();
         for (vector<ostream*>::iterator it = outs->begin();
              it != outs->end(); it++) {
            (**it) << "Minimand value: " << val << " at " << string(focusTrial->getSpec()) << endl;
         }
         return;
      } //else


      for (vector<ostream*>::iterator it = outs->begin();
           it != outs->end(); it++) {
         (**it) << endl << IRutil::DASHES << endl
                << "Fit of aggregate " << loadedGlobs << " giving files" << endl;
         for (size_t f = 0; f < loadedFiles.size(); f++) {
            (**it) << loadedFiles.at(f) << endl;
         }
         (**it) << IRutil::DASHES << endl << endl;
      }

      TrialSpec tsMM = mm->getPivotSpec();   //focusTrial->getSpec();
      focusTrial->setSpec(tsMM);

/*---------------
      TrialSpec ts = focusTrial->getSpec();  //new copy 
      if (mm->isSFirst()) { 
         minS = mm->minimize1(); 
         //minC = mm->getSpec().c;     //why doesn't this work? Side-effects... 
         minC = mm->bestCFor(minS); 
      } else { 
         minC = mm->minimize1(); 
         //minS = mm->getSpec().s; 
         minS = mm->bestSFor(minC); 
      } 
----------------*/


      TrialSpec ts = mm->minimize();         //new copy

      paramsMenu->setStatus(S_VALUE, IRutil::ftoa(ts.s));
      paramsMenu->setStatus(C_VALUE, IRutil::ftoa(ts.c));
      paramsMenu->setStatus(D_VALUE, IRutil::ftoa(ts.d));
      paramsMenu->setStatus(V_VALUE, IRutil::ftoa(ts.v));
      paramsMenu->setStatus(A_VALUE, IRutil::ftoa(ts.a));
      paramsMenu->setStatus(HM_VALUE, IRutil::ftoa(ts.hm));
      paramsMenu->setStatus(HP_VALUE, IRutil::ftoa(ts.hp));
      paramsMenu->setStatus(B_VALUE, IRutil::ftoa(ts.b));
      //paramsMenu->setStatus(SPEC_JUDGMENT_DEPTH, IRutil::ftoa(ts.judgmentDepth));



      if (focusTrial->getModel() == Models::POWER_SHARES) { 
         cout << endl << "#vectorRoot calls = " << IRutil::numVRCalls  
              << ", #Newtons = " << IRutil::numNewtons  
              << ", max #Newtons in any call = " << IRutil::maxNewtons 
           << endl << "Average #Newtons per call = " 
           << ((1.0 * IRutil::numNewtons) / double(IRutil::numVRCalls)) 
           << ",  Max value of root = " << IRutil::maxRoot << endl; 
            //not ccout? not for-loop over all "out"s? 
      } 
 
      IRfun::WEIGHTS wmc = ts.weightMethodChoice; 
      ccout << "Please name the computed best-fit spec: "; 
      loggedRead(ans,true); 
      ans = IRutil::trim(ans); 
      ccout << ans << endl;
      ts.setName(ans); 
      specs->clearStars(); 
      specs->addItem(ts,"",false,true);  //stars it 
      focusTrial->setSpec(ts); 
      referenceTrial->setSpec(ts); //vital to compute IPR
      resetParamsMenu();
      //Next line ought not to be an issue once equations are settled.
      //Workaround: re-load current DeltaScale with copy to reference enabled
      //if (refToo) { referenceTrial->setScale(focusTrial->getScale()); }
      string scalestr = "(x10,000) "; //(mm->getKind() == Minimizer::ML) ? "" : "(x10,000) "; 
      //note: Spec and scale on reference trial not really important
      //PerfData pfElo = (focusTrial->getFixPolicy() < 0) ?  
          //referenceTrial->perfTest(false,false) 
        //: referenceTrial3->perfTest(false,false); 

      for (vector<ostream*>::iterator it = outs->begin();
           it != outs->end(); it++) {
         (**it) << endl << IRutil::DASHES << endl
                << "Minimization"  // by " << fitMenu->nameOf(mm->getKind())
            << " converted the first " << tsMM.getKind() //curvesMenu->nameOf(tsMM.curveChoice)
            << " TrialSpec into the second:" << endl
            << tsMM.paramsPosition() << endl << ts.paramsPosition() << endl;
         (**it) << "with minimand value " << mm->minimand(false);
         (**it) <<  " from tests " << mm->getTestValStr() << endl << IRutil::DASHES << endl;
      }

      PerfData pfElo = referenceTrial->perfTest(false,false);  //gives sw1 error on a few turns
      PerfData pfSigmas = focusTrial->perfTest(false,false); 

      IPRstats is(pfElo,pfSigmas,ts);

      ccout << endl << "Basic Results: " << endl << setprecision(4) 
           << ts.getName() << " & & " << is.ipr.measuredValue << " & "
           << is.ipr.projected2SigmaLeft << "--" << is.ipr.projected2SigmaRight << " & "
           << is.ipr.projected2SigmaLeftAdj << "--" << is.ipr.projected2SigmaRightAdj << " & & "
           << focusTrial->numDecisions() << "\\\\  % IPRauto: "
           << is.iprAuto.measuredValue << " +- " << is.iprAuto.projectedSigma << "  / "
           << is.iprAdjByAuto.measuredValue << endl;




/*
                << is.irdpr2sd << "--" << is.irdpr2su << " & "
                << is.irdpr28sd << "--" << is.irdpr28su << " & & "
                << focusTrial->numDecisions() << "\\\\  % "
                << is.irpr << " +- " << (is.ira2su - is.ira2sd)/4 << "  / "
                << is.irj << endl;

*/
//Now do bootstrap runs.  

      size_t bootNum = mm->getBootNum();
      mm->setSilent();  //turns off verbose if on
      vector<IPRstats> iprv;
      cout << "Doing " << bootNum << " bootstrap trials..." << endl;
      for (int t = 0; t < bootNum; t++) {
         focusTrial->resample(focusTrial->numDecisions(), true);  //with replacement
         //focusTrial->setSpec(ts0);  //makes walkOut from original point
         focusTrial->setSpec(ts);  //makes walkOut from result point
         referenceTrial->setSpec(ts);
         TrialSpec tst = mm->minimize();
         focusTrial->setSpec(tst);
         referenceTrial->setSpec(tst); //vital to compute IPR
         PerfData pfElot = referenceTrial->perfTest(false,false);
         PerfData pfSigmast = focusTrial->perfTest(false,false);
         IPRstats ist(pfElot,pfSigmast,tst);
         iprv.push_back(ist);
         if ((t+1) % 10 == 0) { cout << "Finished trial " << (t+1) << endl; } //minimand calls print #
      }
      cerr << endl;
      focusTrial->resetBootWeights();  //does *not* re-filter
      focusTrial->setSpec(ts);
      referenceTrial->setSpec(ts);
      resetParamsMenu();
      double bootIPRsum = 0.0;
      double bootSumS = 0.0;
      double bootSumC = 0.0;
      for (int i = 0; i < iprv.size(); i++) {
         bootIPRsum += iprv.at(i).ipr.measuredValue;
         bootSumS += iprv.at(i).ts.s;
         bootSumC += iprv.at(i).ts.c;
      }
      double bootIPR = (bootNum > 0 ? bootIPRsum/double(iprv.size()) : is.ipr.measuredValue);
      double bootS = (bootNum > 0 ? bootSumS/double(iprv.size()) : is.ts.s);
      double bootC = (bootNum > 0 ? bootSumC/double(iprv.size()) : is.ts.c);
      double bootIPRvarxn = 0.0;
      double bootSvarxn = 0.0;
      double bootCvarxn = 0.0;
      for (int i = 0; i < iprv.size(); i++) {
         bootIPRvarxn += IRutil::sqr(iprv.at(i).ipr.measuredValue - bootIPR);
         bootSvarxn += IRutil::sqr(iprv.at(i).ts.s - bootS);
         bootCvarxn += IRutil::sqr(iprv.at(i).ts.c - bootC);
      }
      double bootIPRsigma = (iprv.size() > 0 ? sqrt(bootIPRvarxn/double(iprv.size())) : 0.0);
      double bootSsigma = (iprv.size() > 0 ? sqrt(bootSvarxn/double(iprv.size())) : 0.0);
      double bootCsigma = (iprv.size() > 0 ? sqrt(bootCvarxn/double(iprv.size())) : 0.0);



      iprv.clear();

      for (vector<ostream*>::const_iterator it = outs->begin();
           it != outs->end(); it++) {
         (**it) << "Best-fit score " << scalestr << "and spec: " << mm->minimand()
                << endl << ts.oneLine() << endl << IRutil::DASHES << endl
                << "IPR: " << setprecision(4) << is.ipr.measuredValue
                << " from " << pfElo.scaledFalloffWtd.projectedFreq
                << ", 2-sigma range [" << is.ipr.projected2SigmaLeft << "," << is.ipr.projected2SigmaRight << "]"
                << ", 2.8-sigma [" << is.iprAdj.projected2SigmaLeftAdj << "," << is.iprAdj.projected2SigmaRightAdj 
                << "]" << endl
                << "IPR if " << focusTrial->numDecisions()
                << " positions faced were test suite: " << is.iprAuto.measuredValue
                << ", st. dev. " << setprecision(2) << (is.iprAuto.projected2SigmaRight - is.iprAuto.projected2SigmaLeft)/4
                << endl
                << "AdjIPR: " << setprecision(4) << is.iprAdjByAuto.measuredValue << " via " << setprecision(6)
                << pfSigmas.scaledFalloffWtd.measuredFreq << "/"
                << pfSigmas.scaledFalloffWtd.projectedFreq << " = " << is.autoRatio
                << setprecision(2)
                << ": " << is.iprAdjByAuto.projected2SigmaLeft << "--" << is.iprAdjByAuto.projected2SigmaRight << "; 2.8s: "
                << is.iprAdjByAuto.projected2SigmaLeftAdj << "--" << is.iprAdjByAuto.projected2SigmaRightAdj << endl << setprecision(6)
                << "Adj. AE/turn: " << pfElo.scaledFalloffWtd.projectedFreq * is.autoRatio
                << " stdev. " << is.ownSigma << ", index "
                << is.ownSigma/sqrt(double(pfSigmas.accumWeight))
                << endl;

         (**it) << endl << "Line for paper: " << endl << setprecision(4)
           << ts.getName() << " & & " << is.ipr.measuredValue << " & "
           << is.ipr.projected2SigmaLeft << "--" << is.ipr.projected2SigmaRight << " & "
           //<< is.ipr.projected2SigmaLeftAdj << "--" << is.ipr.projected2SigmaRightAdj << " & & "
           << is.iprAdj.projected2SigmaLeft << "--" << is.iprAdj.projected2SigmaRight << " & & "
           << focusTrial->numDecisions() << "\\\\  % IPRauto: "
           << is.iprAuto.measuredValue << " +- " << is.iprAuto.projectedSigma << "  / "
           << is.iprAdjByAuto.measuredValue << endl;
/*
                << ts.getName() << " & & " << is.ir << " & "
                << is.irdpr2sd << "--" << is.irdpr2su << " & "
                << is.irdpr28sd << "--" << is.irdpr28su << " & & "
                << focusTrial->numDecisions() << "\\\\  % "
                << is.irpr << " +- " << (is.ira2su - is.ira2sd)/4 << "  / "
                << is.irj << endl;
*/
         if (bootNum > 0) {
            (**it) << setprecision(5) << "Bootstrap IPR: " << setprecision(4) << bootIPR
                   << ", stdev " << bootIPRsigma << endl
                   << "Bootstrap s: " << bootS << ", stdev " << bootSsigma << endl
                   << "Bootstrap c: " << bootC << ", stdev " << bootCsigma << endl;
         }

      } 
      delete(mm);
   } 
} 

void Ensemble::runPerfTest() const {   //!!!
   IRutil::nullstream nullstrm;
   ostream& ccout = (usingCin()) ? cout : nullstrm;
   string ans;
   int intItem;
   int choice = -1;
   bool uf, uw, tn, mr, suw, sun, showMoves;
   string zmmFile, zevFile, zadFile, movesFile;
   size_t numBootstrapTrials = 0;
   size_t numResampleTrials = 0;
   size_t numResampleGames = 0;
   size_t numResampleTurns = 0;
   size_t movesLimit = 1000;
   bool oneSidePerGame = true;
   size_t numIndices = 15; //should agree with defaults
   size_t equalTopIndexLimit = atoi(testMenu->getStatus(EQUAL_TOP_INDEX_LIMIT).c_str());
   size_t numSelectors = focusTrial->numSelectors();
   testMenu->resetDefaults();
   while (choice != GO_TEST) {
      choice = testMenu->readChoice();  
      switch(choice) {
       case USE_FOCUS_SPEC:
       case MISS_IS_MOVE_NPLUSONE:
       case FORCE_UNIT_WEIGHTS:
       case SHOW_UNWEIGHTED:
       case SHOW_UNSCALED:
       case SHOW_INDIVIDUAL_MOVES:
         ccout << "Yes or No? (y/n) ";
         loggedRead(ans);
         ans = IRutil::trim(ans);
         testMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no");
         break;
       case NUM_INDICES:
         ccout << "Enter K to display top K moves: ";
         loggedRead(numIndices,false);
         testMenu->setStatus(choice, IRutil::itoa(numIndices));
         break;
       case EQUAL_TOP_INDEX_LIMIT:
         ccout << "Enter 0-based limit on equal-top value moves: ";
         loggedRead(equalTopIndexLimit,false);
         testMenu->setStatus(choice, IRutil::itoa(equalTopIndexLimit));
         break;
       case NUM_BOOTSTRAP_TRIALS: 
         ccout << "Enter number of bootstrap trials: ";
         loggedRead(numBootstrapTrials,false);
         testMenu->setStatus(choice, IRutil::itoa(numBootstrapTrials));
         break;
       case NUM_RESAMPLE_TRIALS:
         ccout << "Enter number of non-bootstrap resample trials: ";
         loggedRead(numResampleTrials,false);
         testMenu->setStatus(choice, IRutil::itoa(numResampleTrials));
         break;
       case NUM_RESAMPLE_GAMES:
         ccout << "Enter number of games to resample: ";
         loggedRead(numResampleGames,false);
         testMenu->setStatus(choice, IRutil::itoa(numResampleGames));
         break;
       case NUM_RESAMPLE_TURNS:
         ccout << "Enter number of turns to resample: ";
         loggedRead(numResampleTurns,false);
         testMenu->setStatus(choice, IRutil::itoa(numResampleTurns));
         break;
       case ONE_PLAYER_PER_GAME:
         ccout << "Randomly select one side in each game? (y/n) ";
         loggedRead(ans,false);
         ans = IRutil::trim(ans);
         testMenu->setStatus(choice, IRutil::isYes(ans) ? "yes" : "no");
         break;
       case ZMM_FILE:
         ccout << "File for move-match z-scores? "; 
         loggedRead(ans,false);
         ans = IRutil::trim(ans);
         testMenu->setStatus(choice, ans);
         break;
       case ZEV_FILE:
         ccout << "File for equal-value-match z-scores? ";
         loggedRead(ans,false);
         ans = IRutil::trim(ans);
         testMenu->setStatus(choice, ans);
         break;
       case ZAD_FILE:
         ccout << "File for average-(scaled-)difference z-scores? ";
         loggedRead(ans,false);
         ans = IRutil::trim(ans);
         testMenu->setStatus(choice, ans);
         break;
       case MOVES_FILE:
         ccout << "File for dumping probability data on each move? ";
         loggedRead(ans,false);
         ans = IRutil::trim(ans);
         testMenu->setStatus(choice, ans);
         break;
       case MOVES_LIMIT:
         ccout << "Enter limit on number of positions written: ";
         loggedRead(intItem,false);
         testMenu->setStatus(choice, IRutil::itoa(intItem));
         break;

       case GO_TEST:
         numBootstrapTrials = atoi(testMenu->getStatus(NUM_BOOTSTRAP_TRIALS).c_str());
         numResampleTrials = atoi(testMenu->getStatus(NUM_RESAMPLE_TRIALS).c_str());
         numResampleGames = atoi(testMenu->getStatus(NUM_RESAMPLE_GAMES).c_str());
         numResampleTurns = atoi(testMenu->getStatus(NUM_RESAMPLE_TURNS).c_str());
         oneSidePerGame = IRutil::isYes(testMenu->getStatus(ONE_PLAYER_PER_GAME));
         zmmFile = testMenu->getStatus(ZMM_FILE);
         zevFile = testMenu->getStatus(ZEV_FILE);
         zadFile = testMenu->getStatus(ZAD_FILE);
         movesFile = testMenu->getStatus(MOVES_FILE);
         movesLimit = atoi(testMenu->getStatus(MOVES_LIMIT).c_str());

         numIndices = atoi(testMenu->getStatus(NUM_INDICES).c_str());
         equalTopIndexLimit = atoi(testMenu->getStatus(EQUAL_TOP_INDEX_LIMIT).c_str());
         uf = IRutil::isYes(testMenu->getStatus(USE_FOCUS_SPEC));
         tn = IRutil::isYes(testMenu->getStatus(MISS_IS_MOVE_NPLUSONE));
         uw = IRutil::isYes(testMenu->getStatus(FORCE_UNIT_WEIGHTS));
         suw = IRutil::isYes(testMenu->getStatus(SHOW_UNWEIGHTED));
         sun = IRutil::isYes(testMenu->getStatus(SHOW_UNSCALED));
         showMoves = IRutil::isYes(testMenu->getStatus(SHOW_INDIVIDUAL_MOVES));
         break;  //exits menu into main body
       default:
         ccout << "Could not understand answer, please try again: ";
      }
   } //end of while(choice...); alternative to uf not implemented 

   bool silent = false;

   PerfData pf = focusTrial->perfTest(uw,tn,suw,sun,numIndices,equalTopIndexLimit,silent,movesFile,movesLimit);

//cerr << "Completed perf test." << endl;

   string rept = pf.fullReport(numIndices,tn);
   if (showMoves) {
      //rept += string("\n\nMoves:\n") + pf.movesReport();
      ofstream movesDump(movesFile.c_str(), ios::app);
      movesDump << pf.movesReport() << endl;
      movesDump.close();
   }

   for (vector<ostream*>::iterator it = outs->begin();
        it != outs->end(); it++) {
      (**it) << endl << IRutil::DASHES << endl
             << "Test of aggregate " << loadedGlobs << " giving files" << endl;
      for (size_t f = 0; f < loadedFiles.size(); f++) {
         (**it) << loadedFiles.at(f) << endl;
      }
      (**it) << "using " << string(*focusTrial) << endl
             << IRutil::DASHES << endl << endl;
      (**it) << rept << endl;
      //if (interval > 0.0) {
         //(**it) << "Curve points:" << endl;
         //(**it) << theSpec->showCurvePoints(interval, curveLimit);
      //}
      (**it) << IRutil::DASHES << endl << endl;
   }

   string bootrept = "";
   if (numBootstrapTrials > 0) {
      int t;
      silent = true;
      vector<SimpleStats> ssv;
      cout << "Doing " << numBootstrapTrials << " bootstrap trials..." << endl;
      for (t = 0; t < numBootstrapTrials; t++) {
         focusTrial->resample(focusTrial->numDecisions(), true);  //with replacement
         SimpleStats ss = focusTrial->perfTestSimple(numIndices,numSelectors,
                                         focusTrial->exportSelectors(),uw,tn,false); //don't aggregate
         ssv.push_back(ss);
         //cout << "." << (t+1);
         if ((t+1) % 100 == 0) { cout << "Finished " << (t+1) << " trials." << endl; }
      }
      cerr << endl;
      focusTrial->resetBootWeights();  //does *not* re-filter
      SimpleStats bootSums(numIndices,numSelectors);
      SimpleStats bootVarSums(numIndices,numSelectors);
      SimpleStats bootSigmas(numIndices,numSelectors);
      for (t = 0; t < numBootstrapTrials; t++) {
         bootSums.accumWeight += ssv.at(t).accumWeight;  //meaningless?
         bootSums.equalValueMatchWtd += ssv.at(t).equalValueMatchWtd; //sum of averages--beware
         bootSums.equalValueMatchProjWtd += ssv.at(t).equalValueMatchProjWtd;
         bootSums.scaledFalloffWtd += ssv.at(t).scaledFalloffWtd;
         bootSums.scaledFalloffProjWtd += ssv.at(t).scaledFalloffProjWtd;
         bootSums.indexScoreWtd += ssv.at(t).indexScoreWtd;
         bootSums.indexMassWtd += ssv.at(t).indexMassWtd;
         bootSums.indexScoreInvVarWtd += ssv.at(t).indexScoreInvVarWtd;
         bootSums.indexScorePropWtd += ssv.at(t).indexScorePropWtd;
         bootSums.indexFitWtd += ssv.at(t).indexFitWtd;
         for (int i = 0; i < numIndices; i++) {
            bootSums.indexMatchesWtd.at(i) += ssv.at(t).indexMatchesWtd.at(i);
            bootSums.indexMatchesProjWtd.at(i) += ssv.at(t).indexMatchesProjWtd.at(i);
         }
         for (int k = 0; k < numSelectors; k++) {
            bootSums.selectorMatchesPlayerWtd.at(k) += ssv.at(t).selectorMatchesPlayerWtd.at(k);
            bootSums.selectorMatchesPlayerProjWtd.at(k) += ssv.at(t).selectorMatchesPlayerProjWtd.at(k);
            bootSums.selectorMatchesEngineWtd.at(k) += ssv.at(t).selectorMatchesEngineWtd.at(k);
         }
      }
      double nbt = numBootstrapTrials;
      for (t = 0; t < numBootstrapTrials; t++) {
         bootVarSums.accumWeight += IRutil::sqr(ssv.at(t).accumWeight - bootSums.accumWeight/nbt);
         bootVarSums.equalValueMatchWtd += IRutil::sqr(ssv.at(t).equalValueMatchWtd 
                                                       - bootSums.equalValueMatchWtd/nbt);
         bootVarSums.equalValueMatchProjWtd += IRutil::sqr(ssv.at(t).equalValueMatchProjWtd
                                                       - bootSums.equalValueMatchProjWtd/nbt);
         bootVarSums.scaledFalloffWtd += IRutil::sqr(ssv.at(t).scaledFalloffWtd
                                                       - bootSums.scaledFalloffWtd/nbt);
         bootVarSums.scaledFalloffProjWtd += IRutil::sqr(ssv.at(t).scaledFalloffProjWtd
                                                       - bootSums.scaledFalloffProjWtd/nbt);
         bootVarSums.indexScoreWtd += IRutil::sqr(ssv.at(t).indexScoreWtd
                                                       - bootSums.indexScoreWtd/nbt);
         bootVarSums.indexMassWtd += IRutil::sqr(ssv.at(t).indexMassWtd
                                                       - bootSums.indexMassWtd/nbt);
         bootVarSums.indexScoreInvVarWtd += IRutil::sqr(ssv.at(t).indexScoreInvVarWtd
                                                       - bootSums.indexScoreInvVarWtd/nbt);
         bootVarSums.indexScorePropWtd += IRutil::sqr(ssv.at(t).indexScorePropWtd
                                                       - bootSums.indexScorePropWtd/nbt);
         bootVarSums.indexFitWtd += IRutil::sqr(ssv.at(t).indexFitWtd
                                                       - bootSums.indexFitWtd/nbt);
         for (int i = 0; i < numIndices; i++) {
            bootVarSums.indexMatchesWtd.at(i) += IRutil::sqr(ssv.at(t).indexMatchesWtd.at(i)
                                                       - bootSums.indexMatchesWtd.at(i)/nbt);
            bootVarSums.indexMatchesProjWtd.at(i) += IRutil::sqr(ssv.at(t).indexMatchesProjWtd.at(i)
                                                       - bootSums.indexMatchesProjWtd.at(i)/nbt);
         }
         for (int k = 0; k < numSelectors; k++) {
            bootVarSums.selectorMatchesPlayerWtd.at(k) += IRutil::sqr(ssv.at(t).selectorMatchesPlayerWtd.at(k)
                                                       - bootSums.selectorMatchesPlayerWtd.at(k)/nbt);
            bootVarSums.selectorMatchesPlayerProjWtd.at(k) += IRutil::sqr(ssv.at(t).selectorMatchesPlayerProjWtd.at(k)
                                                       - bootSums.selectorMatchesPlayerProjWtd.at(k)/nbt);
            bootVarSums.selectorMatchesEngineWtd.at(k) += IRutil::sqr(ssv.at(t).selectorMatchesEngineWtd.at(k)
                                                       - bootSums.selectorMatchesEngineWtd.at(k)/nbt);
         }
      }
      bootSigmas.accumWeight = sqrt(bootVarSums.accumWeight/nbt);
      bootSigmas.equalValueMatchWtd = sqrt(bootVarSums.equalValueMatchWtd/nbt);
      bootSigmas.equalValueMatchProjWtd = sqrt(bootVarSums.equalValueMatchProjWtd/nbt);
      bootSigmas.scaledFalloffWtd = sqrt(bootVarSums.scaledFalloffWtd/nbt);
      bootSigmas.scaledFalloffProjWtd = sqrt(bootVarSums.scaledFalloffProjWtd/nbt);
      bootSigmas.indexScoreWtd = sqrt(bootVarSums.indexScoreWtd/nbt);
      bootSigmas.indexMassWtd = sqrt(bootVarSums.indexMassWtd/nbt);
      bootSigmas.indexScoreInvVarWtd = sqrt(bootVarSums.indexScoreInvVarWtd/nbt);
      bootSigmas.indexScorePropWtd = sqrt(bootVarSums.indexScorePropWtd/nbt);
      bootSigmas.indexFitWtd = sqrt(bootVarSums.indexFitWtd/nbt);
      for (int i = 0; i < numIndices; i++) {
         bootSigmas.indexMatchesWtd.at(i) = sqrt(bootVarSums.indexMatchesWtd.at(i)/nbt);
         bootSigmas.indexMatchesProjWtd.at(i) = sqrt(bootVarSums.indexMatchesProjWtd.at(i)/nbt);
      }
      for (int k = 0; k < numSelectors; k++) {
         bootSigmas.selectorMatchesPlayerWtd.at(k) = sqrt(bootVarSums.selectorMatchesPlayerWtd.at(k)/nbt);
         bootSigmas.selectorMatchesPlayerProjWtd.at(k) = sqrt(bootVarSums.selectorMatchesPlayerProjWtd.at(k)/nbt);
         bootSigmas.selectorMatchesEngineWtd.at(k) = sqrt(bootVarSums.selectorMatchesEngineWtd.at(k)/nbt);
      }

      focusTrial->resetBootWeights();
      PerfData pfc = focusTrial->perfTest(uw,tn,suw,sun,numIndices,equalTopIndexLimit,silent);
      pfc.moveMatchWtd.setProjectedValue(bootSums.indexMatchesProjWtd.at(0)/nbt);
      pfc.moveMatchWtd.setMeasuredValue(bootSums.indexMatchesWtd.at(0)/nbt);
      pfc.moveMatchWtd.setProjectedVariance(IRutil::sqr(bootSigmas.indexMatchesProjWtd.at(0)));
      pfc.moveMatchWtd.setMeasuredVariance(IRutil::sqr(bootSigmas.indexMatchesWtd.at(0)));
      pfc.moveMatchWtd.update();
      pfc.moveMatchWtd.bootName();
      pfc.equalValueMatchWtd.setProjectedValue(bootSums.equalValueMatchProjWtd/nbt);
      pfc.equalValueMatchWtd.setMeasuredValue(bootSums.equalValueMatchWtd/nbt);
      pfc.equalValueMatchWtd.setProjectedVariance(IRutil::sqr(bootSigmas.equalValueMatchProjWtd));
      pfc.equalValueMatchWtd.setMeasuredVariance(IRutil::sqr(bootSigmas.equalValueMatchWtd));
      pfc.equalValueMatchWtd.update();
      pfc.equalValueMatchWtd.bootName();
      pfc.scaledFalloffWtd.setProjectedValue(bootSums.scaledFalloffProjWtd/nbt);
      pfc.scaledFalloffWtd.setMeasuredValue(bootSums.scaledFalloffWtd/nbt);
      pfc.scaledFalloffWtd.setProjectedVariance(IRutil::sqr(bootSigmas.scaledFalloffProjWtd));
      pfc.scaledFalloffWtd.setMeasuredVariance(IRutil::sqr(bootSigmas.scaledFalloffWtd));
      pfc.scaledFalloffWtd.update();
      pfc.scaledFalloffWtd.bootName();

      bootrept += string("Original tests and tests with bootstrapped sigmas:\n");
      bootrept += string("\n") + AggregateTest::header() + string("\n");
      bootrept += string(pf.scaledFalloffWtd) + string("\n") + string(pfc.scaledFalloffWtd) + string("\n");
      bootrept += string(pf.moveMatchWtd) + string("\n") + string(pfc.moveMatchWtd) + string("\n");
      bootrept += string(pf.equalValueMatchWtd) + string("\n") + string(pfc.equalValueMatchWtd) + string("\n");

      bootrept += string("Bootstrapped selector matches:\n");
      for (int k = 0; k < numSelectors; k++) {
         pfc.selectionTestsWtd.at(k).setProjectedValue(bootSums.selectorMatchesPlayerProjWtd.at(k));
         pfc.selectionTestsWtd.at(k).setMeasuredValue(bootSums.selectorMatchesPlayerWtd.at(k));
         pfc.selectionTestsWtd.at(k).setProjectedVariance(IRutil::sqr(bootSigmas.selectorMatchesPlayerProjWtd.at(k)));
         pfc.selectionTestsWtd.at(k).setMeasuredVariance(IRutil::sqr(bootSigmas.selectorMatchesPlayerWtd.at(k)));
         pfc.selectionTestsWtd.at(k).update();
         pfc.selectionTestsWtd.at(k).bootName();
         bootrept += string(pf.selectionTestsWtd.at(k)) + "\n" + string(pfc.selectionTestsWtd.at(k)) + "\n";
      }
      bootrept += string("\n");



/*---------------Not the best way to show bootstrapped items?----------------------------------
  
      StatItem* pscaledFalloffWtd = new StatItem(pf.scaledFalloffWtd.name,
            pf.scaledFalloffWtd.maxOrderOfMagnitude, 
            pf.scaledFalloffWtd.numDecimals,
            pf.scaledFalloffWtd.measuredFreq,
            IRutil::sqr(pf.scaledFalloffWtd.measuredSigmaFreq));  //not sure of this !!!
      BootstrappedItem bscaledFalloffWtd(pscaledFalloffWtd,
                                            bootSums.scaledFalloffWtd/nbt, 
                                            bootVarSums.scaledFalloffWtd/nbt,
                                            numBootstrapTrials);
      StatItem* pscaledFalloffProjWtd = new StatItem("scaledFalloffProj",
            pf.scaledFalloffWtd.maxOrderOfMagnitude,
            pf.scaledFalloffWtd.numDecimals,
            pf.scaledFalloffWtd.projectedFreq,
            IRutil::sqr(pf.scaledFalloffWtd.projectedSigmaFreq));  //not sure of this !!!
      BootstrappedItem bscaledFalloffProjWtd(pscaledFalloffProjWtd,
                                            bootSums.scaledFalloffProjWtd/nbt,
                                            bootVarSums.scaledFalloffProjWtd/nbt,
                                            numBootstrapTrials);
      StatItem* pequalValueMatchWtd = new StatItem(pf.equalValueMatchWtd.name,
            pf.equalValueMatchWtd.maxOrderOfMagnitude,
            pf.equalValueMatchWtd.numDecimals,
            pf.equalValueMatchWtd.measuredFreq,
            IRutil::sqr(pf.equalValueMatchWtd.measuredSigmaFreq));  //not sure of this !!!
      BootstrappedItem bequalValueMatchWtd(pequalValueMatchWtd,
                                            bootSums.equalValueMatchWtd/nbt,
                                            bootVarSums.equalValueMatchWtd/nbt,
                                            numBootstrapTrials);
      StatItem* pequalValueMatchProjWtd = new StatItem("equalValueMatchProj",
            pf.equalValueMatchWtd.maxOrderOfMagnitude,
            pf.equalValueMatchWtd.numDecimals,
            pf.equalValueMatchWtd.projectedFreq,
            IRutil::sqr(pf.equalValueMatchWtd.projectedSigmaFreq));  //not sure of this !!!
      BootstrappedItem bequalValueMatchProjWtd(pequalValueMatchProjWtd,
                                            bootSums.equalValueMatchProjWtd/nbt,
                                            bootVarSums.equalValueMatchProjWtd/nbt,
                                            numBootstrapTrials);
      StatItem* pindexMassWtd = new StatItem("indexMassWtd",1,4,pf.indexFitMassWtd,0.0);
      BootstrappedItem bindexMassWtd(pindexMassWtd,
                                            bootSums.indexMassWtd/nbt,
                                            bootVarSums.indexMassWtd/nbt,
                                            numBootstrapTrials);
      StatItem* pindexInvVarWtd = new StatItem("indexInvVarWtd",1,4,pf.indexFitInvVarWtd,0.0);
      BootstrappedItem bindexInvVarWtd(pindexInvVarWtd,
                                            bootSums.indexScoreInvVarWtd/nbt,
                                            bootVarSums.indexScoreInvVarWtd/nbt,
                                            numBootstrapTrials);
      StatItem* pindexPropWtd = new StatItem("indexPropWtd",1,4,pf.indexFitPropWtd,0.0);
      BootstrappedItem bindexPropWtd(pindexPropWtd,
                                            bootSums.indexScorePropWtd/nbt,
                                            bootVarSums.indexScorePropWtd/nbt,
                                            numBootstrapTrials);
      StatItem* pindexFitWtd = new StatItem("indexFitWtd",1,4,pf.indexFitWtd,0.0);
      BootstrappedItem bindexFitWtd(pindexFitWtd,
                                            bootSums.indexFitWtd/nbt,
                                            bootVarSums.indexFitWtd/nbt,
                                            numBootstrapTrials);

      for (int i = 0; i < numIndices; i++) {
         StatItem* pindexMatchesWtd = new StatItem(pf.moveIndexTests.at(i).name,
               pf.moveIndexTests.at(i).maxOrderOfMagnitude,
               pf.moveIndexTests.at(i).numDecimals,
               pf.moveIndexTests.at(i).measuredFreq,
               IRutil::sqr(pf.moveIndexTests.at(i).measuredSigmaFreq));
         BootstrappedItem bindexMatchesWtd(pindexMatchesWtd,
                                            bootSums.indexMatchesWtd.at(i)/nbt,
                                            bootVarSums.indexMatchesWtd.at(i)/nbt,
                                            numBootstrapTrials);
         StatItem* pindexMatchesProjWtd = new StatItem(pf.moveIndexTests.at(i).name,
               pf.moveIndexTests.at(i).maxOrderOfMagnitude,
               pf.moveIndexTests.at(i).numDecimals,
               pf.moveIndexTests.at(i).projectedFreq,
               IRutil::sqr(pf.moveIndexTests.at(i).projectedSigmaFreq));
         BootstrappedItem bindexMatchesProjWtd(pindexMatchesProjWtd,
                                            bootSums.indexMatchesProjWtd.at(i)/nbt,
                                            bootVarSums.indexMatchesProjWtd.at(i)/nbt,
                                            numBootstrapTrials);
         bootrept += string(bindexMatchesProjWtd) + string("\n");
         bootrept += string(bindexMatchesWtd) + string("\n");
         delete(pindexMatchesWtd);
         delete(pindexMatchesProjWtd);
      }

      bootrept += string("\n") + string(bscaledFalloffProjWtd) + string("\n")
                            + string(bscaledFalloffWtd) + string("\n");
      bootrept += string("\n") + string(bequalValueMatchProjWtd) + string("\n")
                            + string(bequalValueMatchWtd) + string("\n");

      bootrept += string("\n") + string(bindexFitWtd) + string("\n")
                            + string(bindexMassWtd) + string("\n")
                            + string(bindexInvVarWtd) + string("\n")
                            + string(bindexPropWtd) + string("\n");
      delete(pindexMassWtd);
      delete(pindexInvVarWtd);
      delete(pindexPropWtd);
      delete(pindexFitWtd);

      for (int k = 0; k < numSelectors; k++) {
         StatItem* pselectorMatchWtd  = new StatItem(pf.selectionTests.at(k).name,
               pf.selectionTests.at(k).maxOrderOfMagnitude,
               pf.selectionTests.at(k).numDecimals,
               pf.selectionTests.at(k).measuredFreq,
               IRutil::sqr(pf.selectionTests.at(k).measuredSigmaFreq));
         BootstrappedItem bselectorMatchWtd(pselectorMatchWtd,
                                            bootSums.selectorMatchesPlayerWtd.at(k)/nbt,
                                            bootVarSums.selectorMatchesPlayerWtd.at(k)/nbt,
                                            numBootstrapTrials);
         StatItem* pselectorMatchProjWtd = new StatItem(pf.selectionTests.at(k).name,
               pf.selectionTests.at(k).maxOrderOfMagnitude,
               pf.selectionTests.at(k).numDecimals,
               pf.selectionTests.at(k).projectedFreq,
               IRutil::sqr(pf.selectionTests.at(k).projectedSigmaFreq));
         BootstrappedItem bselectorMatchProjWtd(pselectorMatchProjWtd,
                                            bootSums.selectorMatchesPlayerProjWtd.at(k)/nbt,
                                            bootVarSums.selectorMatchesPlayerProjWtd.at(k)/nbt,
                                            numBootstrapTrials);
         StatItem* pengineMatchWtd  = new StatItem(pf.selectionTests.at(k).name,
               pf.selectionTests.at(k).maxOrderOfMagnitude,
               pf.selectionTests.at(k).numDecimals,
               pf.selectionTests.at(k).engineMatchFreq,
               0.0);
         BootstrappedItem bengineMatchWtd(pengineMatchWtd,
                                            bootSums.selectorMatchesEngineWtd.at(k)/nbt,
                                            bootVarSums.selectorMatchesEngineWtd.at(k)/nbt,
                                            numBootstrapTrials);

         bootrept += string("\n") + string(bselectorMatchProjWtd) + string("\n")
                               + string(bselectorMatchWtd) + string("\n")
                               + string(bengineMatchWtd) + string("\n");
         delete(pselectorMatchWtd);
         delete(pselectorMatchProjWtd);
         delete(pengineMatchWtd);
      }
*///-----------------------------------------------------------------------------------

         

   } //end of if (numBootstrapTrials > 0)


   if (bootrept != "") {
      for (vector<ostream*>::iterator it = outs->begin();
           it != outs->end(); it++) {
         (**it) << bootrept << endl;
         (**it) << IRutil::DASHES << endl << endl;
      }
   }

   vector<PerfData> perfTests;
   if (numResampleTrials > 0) {
      cout << "Doing " << numResampleTrials << " resample trials." << endl;
      if (numResampleGames > 0) {
         for (int t = 0; t < numResampleTrials; t++) {
            focusTrial->resampleGames(numResampleGames, false, oneSidePerGame);
            PerfData pft = focusTrial->perfTest(uw,tn,suw,sun,numIndices,equalTopIndexLimit,true); //always silent
            perfTests.push_back(pft);
            if ((t+1) % 100 == 0) { cout << "Finished " << (t+1) << " trials." << endl; }
         }
      } else if (numResampleTurns > 0) {
         for (int t = 0; t < numResampleTrials; t++) {
            focusTrial->resample(numResampleTurns, false);  //gives a set
            PerfData pft = focusTrial->perfTest(uw,tn,suw,sun,numIndices,equalTopIndexLimit,true); //always silent
            perfTests.push_back(pft);
            if ((t+1) % 100 == 0) { cout << "Finished " << t << " trials." << endl; }
         }
      } 
      focusTrial->resetBootWeights();
      ofstream zscoreMM(zmmFile.c_str(), ios::app);
      ofstream zscoreEV(zevFile.c_str(), ios::app);
      ofstream zscoreAD(zadFile.c_str(), ios::app);
      int nt = perfTests.size();
      zscoreMM << "Next test of " << nt << " trials:" << endl;
      zscoreEV << "Next test of " << nt << " trials:" << endl;
      zscoreAD << "Next test of " << nt << " trials:" << endl;
      
      for (int t = 0; t < nt; t++) {
         zscoreMM << perfTests.at(t).moveMatchWtd.zScore
                  << "   from " << perfTests.at(t).moveMatchWtd.getSampleVolume() << endl;
         zscoreEV << perfTests.at(t).equalValueMatchWtd.zScore 
                  << "   from " << perfTests.at(t).equalValueMatchWtd.getSampleVolume() << endl;
         zscoreAD << perfTests.at(t).scaledFalloffWtd.zScore
                  << "   from " << perfTests.at(t).scaledFalloffWtd.getSampleVolume() << endl;
      }
      zscoreMM.close();
      zscoreEV.close();
      zscoreAD.close();
   }

}



 
 
TurnFilter* Ensemble::chooseCreatedFilter() { 
   int i = filters->readChoice(); 
   while (!(filters->success(i) || i == EnumMenu::CANCEL_CHOICE 
            || i == EnumMenu::MORE_CHOICE)) { 
      cerr << "Need valid filter choice, or " << EnumMenu::MORE_CHOICE 
           << "/" << filters->info->MORE_NAME << " to define new, " 
           << EnumMenu::CANCEL_CHOICE 
           << "/" << filters->info->CANCEL_NAME << " to cancel." << endl; 
      i = filters->readChoice(); 
   } 
   if (filters->success(i)){ 
      string name = filters->nameOf(i); 
      //filters->setStarred(name,true); 
      return (*filters)[name]; 
   } else if (i == EnumMenu::MORE_CHOICE) { 
      return makeFilter(filtersMenu->readChoice()); 
   } else { 
      cout << "Cancel---returning trivial-true filter..." << endl; 
      FnFilter* outf = new FnFilter(&allowAll,"triv"); 
      return outf; 
   } 
} 
 
TurnFilter* Ensemble::makeFilter(int choice) { 
   IRutil::nullstream nullstr; 
   ostream& ccout = usingCin() ? cout : nullstr; 
   COMPARE comp; 
   string name; 
   string item,ans; 
   int intItem, intItem2, enumItem; 
   double floatItem; 
   bool entering; 
   TurnFilter* outf; 
   AndFilter* andf; 
   OrFilter* orf; 
   PlayerOrFilter* porf; 
   MoveSelector* ms;
   map<string,double> nf;
 
   if (choice >= 2) { 
      ccout << "Name the new filter itself: "; 
      loggedRead(name,false); 
      name = IRutil::trim(name); 
   } 
 
   switch(choice) { 
    case NO_FILTER: 
      outf = new FnFilter(&allowAll,name); 
      break; 
    case EXISTING_FILTER: 
      outf = chooseCreatedFilter(); 
      break; 
    case FN_FILTER: 
      cout << "Only one implemented so far is trivial-true, sorry." << endl; 
      outf = new FnFilter(&allowAll,name); 
      break; 
   case NOT_FILTER: 
      ccout << "Choose another filter, or " << EnumMenu::MORE_CHOICE 
            << " to define new:" << endl; 
      //outf = new NotFilter(makeFilter(filtersMenu->readChoice()),name); 
      outf = new NotFilter(chooseCreatedFilter(), name); 
      break; 
   case AND_FILTER: 
      andf = new AndFilter(name); 
      ans = "yes"; 
      ccout << "Define or load filter conjuncts, or " << EnumMenu::MORE_CHOICE 
            << " to define new:" << endl; 
      while (IRutil::isYes(ans)) { 
         andf->addConjunct(chooseCreatedFilter()); 
         ccout << "Add another conjunct? (y/n) "; 
         loggedRead(ans); 
         ans = IRutil::trim(ans); 
      } 
      outf = andf; 
      break; 
    case OR_FILTER: 
      orf = new OrFilter(name); 
      ans = "yes"; 
      ccout << "Define or load filter disjuncts, " << EnumMenu::MORE_CHOICE 
            << " to define new:" << endl; 
      while (IRutil::isYes(ans)) { 
         orf->addDisjunct(chooseCreatedFilter()); 
         ccout << "Add another disjunct? (y/n) "; 
         loggedRead(ans); 
         ans = IRutil::trim(ans); 
      } 
      outf = orf; 
      break; 
    case PLAYER_IS: 
      ccout << "Enter player surname (build AndFilter to match separate " 
            << "parts):\n"; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      outf = new PlayerFilter(item,name); 
      break; 
    case PLAYER_IS_ONE_OF: 
      porf = new PlayerOrFilter(name); 
      ccout << "Enter players, followed by . :" << endl; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      while (item.size() > 0 && item[0] != '.') { 
         porf->addPlayer(item); 
         loggedRead(item,false); 
         item = IRutil::trim(item); 
      } 
      outf = porf; 
      break; 
    case PLAYER_IS_ON_MOVE: 
      ccout << "Enter player surname (build AndFilter to match separate " 
            << "parts):\n"; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      outf = new PlayerToMoveFilter(item,name); 
      break; 
    case PLAYER_ON_MOVE_IS_ONE_OF: 
      porf = new PlayerOrToMoveFilter(name); 
      ccout << "Enter players, followed by . :" << endl; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      while (item.size() > 0 && item[0] != '.') { 
         porf->addPlayer(item); 
         loggedRead(item,false); 
         item = IRutil::trim(item); 
      } 
      outf = porf; 
      break; 
    case ON_MOVE_FACING: 
      ccout << "Enter player surname (build AndFilter to match separate " 
            << "parts):\n"; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      outf = new OpponentToMoveFilter(item,name); 
      break; 
    case ON_MOVE_FACING_ONE_OF: 
      porf = new OpponentOrToMoveFilter(name); 
      ccout << "Enter players, followed by . :" << endl; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      while (item.size() > 0 && item[0] != '.') { 
         porf->addPlayer(item); 
         loggedRead(item,false); 
         item = IRutil::trim(item); 
      } 
      outf = porf; 
      break; 
    case WHITE_IS: 
      ccout << "Enter player surname (build AndFilter to match separate " 
            << "parts):\n"; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      outf = new PlayerWhiteFilter(item,name); 
      break; 
    case BLACK_IS: 
      ccout << "Enter player surname (build AndFilter to match separate " 
            << "parts):\n"; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      outf = new PlayerBlackFilter(item,name); 
      break; 
    case EVENT_IS: 
      ccout << "Enter word in event name (build AndFilter to match separate " 
            << "parts):\n"; 
      loggedRead(item,false); 
      item = IRutil::trim(item); 
      outf = new EventFilter(item,name); 
      break; 
    case DATE_IS: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter date in exact Arena (Euro) format yyyy.mm.dd: "; 
         loggedRead(item,false); 
         item = IRutil::trim(item); 
         entering = !compareMenu->success(enumItem); 
         if (entering) { 
            cout << "Invalid DateFilter entry, please try again.\n"; 
         } 
      } 
      outf = new DateFilter(COMPARE(enumItem), 
                            IRutil::SimpleDate(item, IRutil::ARENA_FORMAT), 
                            name); 
      break; 
    case EVENT_DATE_IS:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter date in exact Arena (Euro) format yyyy.mm.dd: ";
         loggedRead(item,false);
         item = IRutil::trim(item);
         entering = !compareMenu->success(enumItem);
         if (entering) {
            cout << "Invalid EventDateFilter entry, please try again.\n";
         }
      }
      outf = new EventDateFilter(COMPARE(enumItem),
                                 IRutil::SimpleDate(item, IRutil::ARENA_FORMAT),
                                 name);
      break;
    case YEAR_IS: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter year: "; 
         loggedRead(item,false); 
         item = IRutil::trim(item); 
         entering = !compareMenu->success(enumItem); 
         if (entering) { 
            cout << "Invalid YearFilter entry, please try again.\n"; 
         } 
      } 
      outf = new YearFilter(COMPARE(enumItem), atoi(item.c_str()), name); 
      break; 
    case WHITE_ELO:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal Elo for White: ";
         loggedRead(intItem,false);
         entering = !compareMenu->success(enumItem);
         if (entering) {
            cout << "Invalid EloWhite entry, please try again.\n";
         }
      }
      outf = new WhiteEloFilter(COMPARE(enumItem),intItem,name);
      break;
    case BLACK_ELO:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal Elo for Black: ";
         loggedRead(intItem,false);
         entering = !compareMenu->success(enumItem);
         if (entering) {
            cout << "Invalid EloBlack entry, please try again.\n";
         }
      }
      outf = new BlackEloFilter(COMPARE(enumItem),intItem,name);
      break;
    case PLAYER_ELO:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal Elo for player to move: ";
         loggedRead(intItem,false);
         entering = !compareMenu->success(enumItem);
         if (entering) {
            cout << "Invalid PlayerElo entry, please try again.\n";
         }
      }
      outf = new PlayerEloFilter(COMPARE(enumItem),intItem,name);
      break;
    case ELO_DIFF:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal value of Elo minus opponent's Elo: ";
         loggedRead(intItem,false);
         entering = !compareMenu->success(enumItem);
         if (entering) {
            cout << "Invalid EloDiff entry, please try again.\n";
         }
      }
      outf = new EloDiffFilter(COMPARE(enumItem),intItem,name);
      break;
    case ELO_DIFF_WITHIN:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal absolute value of Elo difference: ";
         loggedRead(intItem,false);
         entering = !compareMenu->success(enumItem);
         if (entering) {
            cout << "Invalid EloDiffWithin entry, please try again.\n";
         }
      }
      outf = new EloDiffWithinFilter(COMPARE(enumItem),intItem,name);
      break;
    case EVAL_WHITE: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter pivotal eval to White: "; 
         loggedRead(floatItem,false); 
         entering = !compareMenu->success(enumItem); 
         if (entering) { 
            cout << "Invalid EvalWhite entry, please try again.\n"; 
         } 
      } 
      outf = new EvalWhiteFilter(COMPARE(enumItem),floatItem,name); 
      break; 
    case PREV_EVAL_WHITE: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter pivotal eval to White: "; 
         loggedRead(floatItem,false); 
         entering = !compareMenu->success(enumItem); 
         if (entering) { 
            cout << "Invalid PrevEvalWhite entry, please try again.\n"; 
         } 
      } 
      outf = new PrevEvalWhiteFilter(COMPARE(enumItem),floatItem,name); 
      break; 
    case EVAL_WITHIN: 
      ccout << "Enter nonnegative bound for absolute eval: "; 
      loggedRead(floatItem,false); 
      ccout << "Enter applicable depth, 0 for turn max: ";
      loggedRead(intItem,false);
      ccout << "Import norm factors from focus trial'sscale? ";
      loggedRead(ans);
      ans = IRutil::trim(ans);
      if (IRutil::isYes(ans)) {
         nf = focusTrial->getScale()->getNormFactors();
      }
      outf = new EvalWithinFilter(floatItem,name,intItem,nf); 
      break; 
    case PREV_EVAL_WITHIN: 
      ccout << "Enter nonnegative bound for absolute eval: "; 
      loggedRead(floatItem,false); 
      ccout << "Enter applicable depth, 0 for turn max: ";
      loggedRead(intItem,false);
      ccout << "Import norm factors from focus trial'sscale? ";
      loggedRead(ans);
      ans = IRutil::trim(ans);
      if (IRutil::isYes(ans)) {
         nf = focusTrial->getScale()->getNormFactors();
      }
      outf = new PrevEvalWithinFilter(floatItem,name,intItem,nf); 
      break; 
    case NEXT_EVAL_WITHIN:
      ccout << "Enter nonnegative bound for absolute eval: ";
      loggedRead(floatItem,false);
      ccout << "Enter applicable depth, 0 for turn max: ";
      loggedRead(intItem,false);
      ccout << "Import norm factors from focus trial'sscale? ";
      loggedRead(ans);
      ans = IRutil::trim(ans);
      if (IRutil::isYes(ans)) {
         nf = focusTrial->getScale()->getNormFactors();
      }
      outf = new NextEvalWithinFilter(floatItem,name,intItem,nf);
      break;
    case TURN_NO: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter move number: "; 
         loggedRead(intItem,false); 
         entering = !(compareMenu->success(enumItem) && intItem >= 1); 
         if (entering) { 
            cout << "Invalid MoveNo entry, please try again.\n"; 
         } 
      } 
      outf = new TurnNoFilter(COMPARE(enumItem),intItem,name); 
      break; 
    case NUM_LEGAL_MOVES: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter number of legal moves: "; 
         loggedRead(intItem,false); 
         entering = !(compareMenu->success(enumItem) && intItem >= 1); 
         if (entering) { 
            cout << "Invalid NumLegalMoves entry, please try again.\n"; 
         } 
      } 
      outf = new NumLegalMovesFilter(COMPARE(enumItem),intItem,name); 
      break; 
    case DELTA_I: 
      entering = true; 
      while (entering) { 
         ccout << "Enter index in " << 0 << ".." << IRutil::NCUTOFF - 1 << ": "; 
         loggedRead(intItem,false); 
         enumItem = compareMenu->readChoice(); 
         entering = !(compareMenu->success(enumItem) && intItem >= 0 
                      && intItem <= IRutil::NCUTOFF - 1); 
         if (entering) { 
            cout << "Invalid DeltaIs entry, please try again.\n"; 
         } 
      } 
      ccout << "Enter minimum desired magnitude of delta["<<intItem<<"]: "; 
      loggedRead(floatItem,false); 
      outf = new DeltaIFilter(COMPARE(enumItem),floatItem,intItem,name); 
      break; 
    case DELTA_DIFF: 
      entering = true; 
      while (entering) { 
         ccout << "Enter index1 in " << 0 << ".." << IRutil::NCUTOFF-1 << ": "; 
         loggedRead(intItem,false); 
         ccout << "Enter index2 in " << 0 << ".." << IRutil::NCUTOFF-1 
               << ": "; 
         loggedRead(intItem2,false); 
         enumItem = compareMenu->readChoice(); 
         entering = !(compareMenu->success(enumItem) && intItem >= 0 
                      && intItem2 >= 0 && intItem <= IRutil::NCUTOFF-1 
                      && intItem2 <= IRutil::NCUTOFF-1); 
         if (entering) { 
            cout << "Invalid DeltaDiff entry, please try again.\n"; 
         } 
      } 
      ccout << "Enter minimum desired magnitude of delta(" << intItem 
               << ") - delta(" << intItem2 << "): "; 
      loggedRead(floatItem,false); 
      outf = new DeltaDiffFilter(COMPARE(enumItem),floatItem,intItem,intItem2, 
                                 name); 
      break; 
    case SECOND_DELTA: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter pivotal magnitude of delta[1]: "; 
         loggedRead(floatItem,false); 
         entering = !(compareMenu->success(enumItem) && floatItem >= 0.0); 
         if (entering) { 
            cout << "Invalid SecondDelta entry, please try again.\n"; 
         } 
      } 
      outf = new SecondDeltaFilter(COMPARE(enumItem),floatItem,name); 
      break; 
    case DELTA_N: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter pivotal magnitude of delta[" << IRutil::NCUTOFF  
               << "]: "; 
         loggedRead(floatItem,false); 
         entering = !(compareMenu->success(enumItem) && floatItem >= 0); 
         if (entering) { 
            cout << "Invalid LastDeltaIs entry, please try again.\n"; 
         } 
      } 
      outf = new DeltaNFilter(COMPARE(enumItem),floatItem,name); 
      break; 
    case REP_COUNT:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter count to keep: ";
         loggedRead(intItem,false);
         ccout << "Use RepLines? (y/n) ";
         ans = "0";
         loggedRead(ans);
         ans = IRutil::trim(ans);
         entering = (!(compareMenu->success(enumItem) && ans != "0"));
         if (entering) {
            cout << "Invalid RepCount entry, please try again.\n";
         }
      }
      outf = new RepCountFilter(COMPARE(enumItem), intItem, IRutil::isYes(ans), name);
      break;
    case PLAYED_MOVE_INDEX: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter 0-based pivotal index: "; 
         loggedRead(intItem, false); 
         entering = !(compareMenu->success(enumItem) && intItem >= 0.0 
                      && intItem <= IRutil::NCUTOFF); 
         if (entering) { 
            cout << "Invalid PlayedMoveIndex entry, please try again.\n"; 
         } 
      } 
      outf = new PlayedMoveIndexFilter(COMPARE(enumItem),intItem,name); 
      break; 
    case PLAYED_MOVE_IS_SELECTED:
      ms = chooseCreatedSelector(); //handles all I/O
      outf = new PlayedMoveSelectedFilter(ms);
      break;
    case ENGINE_MOVE_IS_SELECTED:
      ms = chooseCreatedSelector(); //handles all I/O
      outf = new EngineMoveSelectedFilter(ms);
      break;
    case EQUAL_TOP_MOVE_IS_SELECTED:
      ms = chooseCreatedSelector(); //handles all I/O
      outf = new EqualTopMoveSelectedFilter(ms);
      break;
    case SOME_MOVE_IS_SELECTED:
      ms = chooseCreatedSelector(); //handles all I/O
      outf = new SelectionNonemptyFilter(ms);
      break;
    case FALLOFF: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter threshold delta d.dd: "; 
         loggedRead(floatItem, false); 
         entering = !(compareMenu->success(enumItem) && floatItem >= 0.0); 
         if (entering) { 
            cout << "Invalid Falloff delta, please try again.\n"; 
         } 
      } 
      outf = new FalloffFilter(COMPARE(enumItem),floatItem,name); 
      break; 
    case WEIGHT: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter pivotal weight under current spec: "; 
         loggedRead(floatItem,false); 
         entering = !(compareMenu->success(enumItem) && floatItem >= 0); 
         if (entering) { 
            cout << "Invalid WeightFilter entry, please try again.\n"; 
         } 
      } 
      outf = new WeightFilter(focusTrial,true, 
                              focusTrial->getModel(),COMPARE(enumItem), 
                              floatItem,name); 
      break; 
    case PROB_I: 
      entering = true; 
      while (entering) { 
         enumItem = compareMenu->readChoice(); 
         ccout << "Enter index in " << 0 << ".." << IRutil::NCUTOFF << ": "; 
         loggedRead(intItem,false); 
         entering = !(compareMenu->success(enumItem) && intItem >= 0.0 
                      && intItem <= IRutil::NCUTOFF); 
         if (entering) { 
            cout << "Invalid ProbIs entry, please try again.\n"; 
         } 
      } 
      ccout << "Enter pivotal probability of selected move: "; 
      loggedRead(floatItem,false); 
      outf = new ProbIFilter(focusTrial,true, 
                             focusTrial->getModel(),COMPARE(enumItem), 
                             floatItem,intItem,name); 
      break; 
    default: 
      ccout << "Choice out of bounds or undecodable, clearing input" << endl; 
      outf = new FnFilter(&allowAll,name); 
      (*INP).clear(); 
      (*INP).ignore(numeric_limits<streamsize>::max(),'\n'); 
      break; 
   } 
   return outf; 
} 
 
void Ensemble::newFilters() { 
   bool entering = true; 
   int choice = filtersMenu->readChoice(); 
   while (entering) { 
      if (filtersMenu->success(choice) && choice != NO_FILTER) { 
         TurnFilter* tf = makeFilter(choice); 
         string name = tf->getName(); 
         if (filters->hasEntry(name)) { 
            if (filters->isStarred(name)) { 
               cout << "Filter already attached, skipping..." << endl; 
            } else { 
               cout << "Filter already exists, attaching it..." << endl; 
               filters->setStarred(name, true); 
               focusTrial->attachFilter((*filters)[name]); 
            } 
         } else { 
            addNewFilter(tf); 
            cout << "Added " + string(*tf) << endl; 
         } 
         if (usingCin()) { 
            cout << "Make another, or " << NO_FILTER << "/" 
                 << filtersMenu->nameOf(NO_FILTER) << " to stop and filter: "; 
         } 
         //loggedRead(choice,true);  //begin line for next choice/action 
         choice = filtersMenu->readChoice(false);  //does not re-display menu 
      } else if (choice == NO_FILTER) { 
         entering = false; 
//cerr << "Abt2reset...";
         reset(); 
//cerr << "didit." << endl;
      } else { 
         cout << "Invalid entry in newFilters, please try again..." << endl; 
         choice = filtersMenu->readChoice();   //re-displays menu 
      } 
   } 
   loggedWrite("",true);  //ends the line. 
} 
 
 
void Ensemble::addNewFilter(TurnFilter* tf) { 
   filters->addItem(tf,"",false,true); //CLASS INV: all loaded filters starred 
   focusTrial->attachFilter(tf); 
} 
 
void Ensemble::addTrialSpec(TrialSpec ts) { 
   //does not overwrite focus trial's spec.  NOT on main menu, not logged 
   specs->addItem(ts,"",false,false);    //hence not starred 
} 

void Ensemble::attachFilters() { 
   bool entering = true; 
   bool showMenu = true; 
   string ans, name; 
   while (entering) { 
      int choice = filters->readChoice(showMenu); 
      if (filters->success(choice)) { 
         name = filters->nameOf(choice); 
         if (!filters->isStarred(choice)) { 
            filters->setStarred(name,true); 
            focusTrial->attachFilter((*filters)[name]); 
         } else { 
            cout << "Filter already attached." << endl; 
         } 
         showMenu = false; 
      } else if (choice == EnumMenu::MORE_CHOICE) { 
         if (hiddenFilters->empty()) { 
            cout << "No hidden filters to choose from." << endl; 
         } 
         int choice2 = hiddenFilters->readChoice(); 
         if (hiddenFilters->success(choice2)) { 
            name = hiddenFilters->nameOf(choice2); 
            TurnFilter* tf = (*hiddenFilters)[name]; 
            hiddenFilters->removeItem(name); 
            addNewFilter(tf); 
         } else if (choice2 == EnumMenu::CANCEL_CHOICE) { 
            cout << "Canceling load of hidden filter..." << endl; 
         } else if (choice2 == EnumMenu::MORE_CHOICE) { 
            newFilters(); 
         } else { 
            cout << "Invalid choice or hidden filter not found." << endl; 
            //pop back to loading active filters, not total cancel 
            //this is consistent with the case of no hidden filters. 
         } 
         showMenu = true;  //need to show active filters again 
      } else if (choice == EnumMenu::CANCEL_CHOICE) { 
         entering = false; 
      } else {  
         cout << "Invalid choice in attachFilters, please try again." << endl; 
         showMenu = false; 
      } 
      if (entering) { 
         if (usingCin()) { 
            cout << "Attach another? (y/n) "; 
            if (choice == EnumMenu::MORE_CHOICE) { 
               cout << "(will show active filters first again) "; 
            } 
         } 
         loggedRead(ans); 
         entering = IRutil::isYes(ans); 
      } 
   } 
   reset(); 
   loggedWrite("",true);  //logs a newline 
} 
 
 
void Ensemble::detachFilters() { 
   int choice = filters->readChoice(); 
   string ans = "yes"; 
   bool entering = true; 
   while (entering) { 
      if (filters->success(choice)) { 
         if (filters->isStarred(choice)) { 
            string name = filters->nameOf(choice); 
            filters->setStarred(name,false); 
            focusTrial->detachFilter((*filters)[name]); 
         } else { 
            cout << "Filter already detached." << endl; 
         } 
      } else if (choice == EnumMenu::CANCEL_CHOICE) { 
         break;  //jumps while-loop 
      } else { 
         cout << "Invalid choice." << endl; 
      } 
      if (usingCin()) { 
         cout << "Detach another? (y/n) "; 
      } 
      loggedRead(ans); 
      entering = IRutil::isYes(ans); 
      if (entering) { choice = filters->readChoice(false); } //suppress menu 
   } 
   reset(); 
   loggedWrite("",true); 
} 

void Ensemble::hideFilter(TurnFilter* tf) {
   hiddenFilters->addItem(tf, "", false, false);
}
 
void Ensemble::hideFilters() { 
   string ans = "yes"; 
   bool cancel = false; 
   int choice; 
   while (IRutil::isYes(ans) && filters->size() > 0) { 
      choice = filters->readChoice(); 
      if (filters->success(choice)) { 
         if (filters->isStarred(choice)) {     // (specs->nameOf(choice))) { 
            cout << "Can't hide active filter---please detach it first." 
                 << endl; 
         } else { 
            string name = filters->nameOf(choice); 
            hiddenFilters->addItem((*filters)[name], "", false, false); 
            filters->removeItem(name); 
         } 
      } else if (choice == 0) { 
         choice = 1; 
         while (filters->success(choice)) { 
            string name = filters->nameOf(choice); 
            hideFilter((*filters)[name]); 
            filters->removeItem(name); 
         } 
      } else { 
         cout << "Invalid choice, returning..."; 
         cancel = true; 
      }   
      if (usingCin() && !cancel) {  
         cout << "Hide another? (y/n) ";  
      } 
      loggedRead(ans); 
   }                         
   if ((cancel || filters->size() == 0) && usingCin()) { 
      cout << "No more filters to hide!" << endl; 
   }    
} 

/*-------------
   enumSelectorsMenu {
       NEW_SELECTOR,
       TOGGLE_SELECTOR,
       NO_SELECTOR,
       EXISTING_SELECTOR,
       FN_SELECTOR,
       NOT_SELECTOR,
       AND_SELECTOR,
       OR_SELECTOR,
       MAP_SELECTOR,
       NUM_SELECTOR_CHOICES
   }
   void newSelectors();

   void addSelector(const MoveSelector* ms);
   void setSelector(MoveSelector* ms);

   void toggleOneSelector() const;
   
   MoveSelector* chooseCreatedSelector() const;

   MoveSelector* makeSelector(int choice) const;

   MoveSelector* mapSelectorInFile(const MoveSelector* ms) const;
*/
void Ensemble::newSelectors() {
   bool entering = true; 
   int choice = selectorsMenu->readChoice();  //since reading from enumMenu
   while (entering) { 
      if (selectorsMenu->success(choice) && choice != NO_SELECTOR) { 
         //TurnFilter* tf = makeFilter(choice);
         MoveSelector* ms = makeSelector(choice); 
         string name = ms->getName(); 
         if (selectors->hasEntry(name)) { 
            if (selectors->isStarred(name)) { 
               cout << "Selector already active, skipping..." << endl; 
            } else { 
               cout << "Selector already exists, activating it..." << endl; 
               selectors->setStarred(name, true);  //!!!fix to toggle for now, add later
               focusTrial->setSelector((*selectors)[name]); 
            } 
         } else { 
            justAddSelector(ms); //Does not star it
            selectors->setStarred(ms->getName(), true);
            focusTrial->addSelector(ms);
            cout << "Added " + string(*ms) << endl; 
         } 
         if (usingCin()) { 
            cout << "Make another, or " << NO_SELECTOR << "/" 
                 << selectorsMenu->nameOf(NO_SELECTOR) << " to stop: "; 
         } 
         //loggedRead(choice,true);  //begin line for next choice/action 
         choice = selectorsMenu->readChoice(false);  //does not re-display menu 
      } else if (choice == NO_SELECTOR) { 
         entering = false; 
      } else { 
         cout << "Invalid entry " << choice << " in newSelectors, please try again..." << endl; 
         choice = selectorsMenu->readChoice();   //re-displays menu 
      } 
   } 
   loggedWrite("",true);  //ends the line. 
} 

void Ensemble::addSelector(MoveSelector* ms) {
   selectors->addItem(ms, "", false, true);  //stars it
   focusTrial->addSelector(ms);
}

void Ensemble::justAddSelector(MoveSelector* ms) {
   selectors->addItem(ms, "", false, false);  //not starred or even set
}

void Ensemble::setSelector(MoveSelector* ms) {
   //focusTrial->setSelector(ms);
   focusTrial->addSelector(ms);
   string name = ms->getName();
   //selectors->clearStars();
   selectors->setStarred(name, true);
}

void Ensemble::toggleOneSelector() { 
   bool entering = true; 
   bool showMenu = true; 
   string ans, name; 
   while (entering) { 
      int choice = selectors->readChoice(showMenu); 
      if (selectors->success(choice)) { 
         name = selectors->nameOf(choice); 
         if (!selectors->isStarred(choice)) { 
            selectors->clearStars();
            selectors->setStarred(name,true); 
            //focusTrial->setSelector((*selectors)[name]); 
            focusTrial->clearSelectors();
            focusTrial->addSelector((*selectors)[name]);
         } else { 
            cout << "Selector already active." << endl; 
         } 
         showMenu = false; 
         entering = false;
      } else if (choice == EnumMenu::MORE_CHOICE) { 
         newSelectors();   //does set the selector made
         showMenu = true;  //need to show active selectors again 
      } else if (choice == EnumMenu::CANCEL_CHOICE) { 
         entering = false; 
      } else {  
         cout << "Invalid choice in toggleOneSelector, please try again." << endl; 
         showMenu = false; 
      } 
   } 
   //reset(); 
   loggedWrite("",true);  //logs a newline 
} 

void Ensemble::toggleSelectors() {
   bool entering = true;
   bool showMenu = true;
   string ans, name;
   while (entering) {
      int choice = selectors->readChoice(showMenu, true);
      if (choice == GoMenu::GO_CHOICE) {
         focusTrial->clearSelectors();
         for (int i = 0; i < selectors->size(); i++) {
            if (selectors->isStarred(i)) {
               focusTrial->addSelector((*selectors)[selectors->nameOf(i)]);
            }
         }
         entering = false;
      } else if (selectors->success(choice)) {
         name = selectors->nameOf(choice);
         if (selectors->isStarred(choice)) {
            selectors->setStarred(name,false);
         } else {
            selectors->setStarred(name,true);
         }
      } else if (choice == EnumMenu::MORE_CHOICE) {
         newSelectors();   //does star the selector made
         showMenu = true;  //need to show active selectors again
      } else if (choice == EnumMenu::CANCEL_CHOICE) {
         entering = false;
      } else {
         cout << "Invalid choice in toggleSelectors, please try again." << endl;
         showMenu = false;
      }
   }
   loggedWrite("",true);  //logs a newline
}

MoveSelector* Ensemble::chooseCreatedSelector() const {
   int i = selectors->readChoice();
   while (!(selectors->success(i) || i == EnumMenu::CANCEL_CHOICE
            || i == EnumMenu::MORE_CHOICE)) {
      cerr << "Need valid selector choice, or " << EnumMenu::MORE_CHOICE
           << "/" << selectors->info->MORE_NAME << " to define new, "
           << EnumMenu::CANCEL_CHOICE
           << "/" << selectors->info->CANCEL_NAME << " to cancel." << endl;
      i = selectors->readChoice();
   }
   if (selectors->success(i)){
      string name = selectors->nameOf(i); //selectors->setStarred(name,true);
      return (*selectors)[name];
   } else if (i == EnumMenu::MORE_CHOICE) {
      return makeSelector(selectorsMenu->readChoice());
   } else {
      cout << "Cancel---returning trivial-true selector..." << endl;
      MoveSelector* outf = new OmniSelector();
      return outf;
   }
}

MoveSelector* Ensemble::makeSelector(int choice) const {
   IRutil::nullstream nullstr;
   ostream& ccout = usingCin() ? cout : nullstr;
   COMPARE comp;
   string name;
   string item,ans;
   int intItem, intItem2, enumItem;
   double floatItem;
   bool entering;
   MoveSelector* outs;
   MoveSelector* ms;
   AndSelector* ands;
   OrSelector* ors;
   bool exc,playerView,normalize;

   if (choice >= 2) {
      ccout << "Name the new selector itself: ";
      loggedRead(name,false);
      name = IRutil::trim(name);
      ccout << "Exclude from sample in case of null move-set? (y/n): ";
      loggedRead(ans,false);
      ans = IRutil::trim(ans);
      exc = IRutil::isYes(ans);
   }

   switch(choice) {
    case NO_SELECTOR:
      outs = new OmniSelector();
      break;
    case EXISTING_SELECTOR:
      outs = chooseCreatedSelector();
      break;
    case FN_SELECTOR:
      cout << "Only one implemented so far is trivial-true, sorry." << endl;
      outs = new OmniSelector();
      break;
   case NOT_SELECTOR:
      ccout << "Choose another selector, or " << EnumMenu::MORE_CHOICE
            << " to define new:" << endl;
      outs = new NotSelector(chooseCreatedSelector(), name, exc);
      break;
   case AND_SELECTOR:
      ands = new AndSelector(name, exc);
      ans = "yes";
      ccout << "Define or load selector conjuncts, or " << EnumMenu::MORE_CHOICE
            << " to define new:" << endl;
      while (IRutil::isYes(ans)) {
         ands->addConjunct(chooseCreatedSelector());
         ccout << "Add another conjunct? (y/n) ";
         loggedRead(ans);
         ans = IRutil::trim(ans);
      }
      outs = ands;
      break;
    case OR_SELECTOR:
      ors = new OrSelector(name, exc);
      ans = "yes";
      ccout << "Define or load selector disjuncts, " << EnumMenu::MORE_CHOICE
            << " to define new:" << endl;
      while (IRutil::isYes(ans)) {
         ors->addDisjunct(chooseCreatedSelector());
         ccout << "Add another disjunct? (y/n) ";
         loggedRead(ans);
         ans = IRutil::trim(ans);
      }
      outs = ors;
      break;
    case MAP_SELECTOR:
      ccout << "Choose selector to apply, or " << EnumMenu::MORE_CHOICE
            << " to define new:" << endl;
      ms = chooseCreatedSelector();
      outs = mapSelectorInFile(name, ms);
      break;
    case MOVE_EVAL_SELECTOR:
      ccout << "Define eval from Player's not White's view? (y/n): ";
      loggedRead(ans,false);
      playerView = IRutil::isYes(ans);
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal eval of selected move: ";
         loggedRead(floatItem, false);
         ccout << "Enter depth, 0 for turn max: ";
         loggedRead(intItem,false);
         entering = !(compareMenu->success(enumItem) && intItem >= 0);
         if (entering) {
            cout << "Invalid depth entry, please try again.\n";
         }
      }
      outs = new MoveEvalSelector(COMPARE(enumItem), floatItem, intItem, playerView, exc, name);
      break;

    case MOVE_RAW_DELTA_SELECTOR:
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal delta of selected move in p.nn units: ";
         loggedRead(floatItem, false);
         ccout << "Enter depth, 0 for turn max: ";
         loggedRead(intItem,false);
         entering = !(compareMenu->success(enumItem) && intItem >= 0);
         if (entering) {
            cout << "Invalid depth entry, please try again.\n";
         }
      }
      outs = new MoveRawDeltaSelector(COMPARE(enumItem), floatItem, intItem, exc, name);
      break;
    case MOVE_RAW_SWING_SELECTOR:
      ccout << "Normalize swing by number of depths? (y/n): ";
      loggedRead(ans,false);
      normalize = IRutil::isYes(ans);
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal swing of selected move: ";
         loggedRead(floatItem, false);
         ccout << "Enter depth, 0 for turn max: ";
         loggedRead(intItem,false);
         entering = !(compareMenu->success(enumItem) && intItem >= 0);
         if (entering) {
            cout << "Invalid depth entry, please try again.\n";
         }
      }
      outs = new MoveRawSwingSelector(COMPARE(enumItem), floatItem, intItem, normalize, exc, name);
      break;
    case MOVE_SCALED_SWING_SELECTOR:
      ccout << "Normalize swing by number of depths? (y/n): ";
      loggedRead(ans,false);
      ans = IRutil::trim(ans);
      normalize = IRutil::isYes(ans);
      entering = true;
      while (entering) {
         enumItem = compareMenu->readChoice();
         ccout << "Enter pivotal swing of selected move: ";
         loggedRead(floatItem, false);
         ccout << "Enter depth, 0 for turn max: ";
         loggedRead(intItem,false);
         entering = !(compareMenu->success(enumItem) && intItem >= 0);
         if (entering) {
            cout << "Invalid depth entry, please try again.\n";
         }
      }
      outs = new MoveScaledSwingSelector(focusTrial, COMPARE(enumItem), floatItem, intItem, 
                                         normalize, exc, name);
      break;

    default:
      ccout << "Choice out of bounds or undecodable, clearing input" << endl;
      outs = new OmniSelector();
      (*INP).clear();
      (*INP).ignore(numeric_limits<streamsize>::max(),'\n');
      break;
   }
   return outs;
}


MoveSelector* Ensemble::mapSelectorInFile(const string& name, const MoveSelector* ms) const {
   MappedMoveSelector* outs;
   IRutil::nullstream nullstr;
   ostream& ccout = usingCin() ? cout : nullstr;
   bool entering = true;
   int count = 0;
   string fileName;
   ccout << "Enter file name: ";
   while (entering) {
      loggedRead(fileName, true); //ends line
      fileName = IRutil::trim(fileName);
      ifstream file(fileName.c_str());
      if (!file.is_open()) {
         ccout << "Cannot read the input file, please try again: ";
      } else {
         list<TurnInfo*>* myTurns = readInputFile(file, false, true);
         outs = new MappedMoveSelector(name+":"+fileName, ms->excludesTurnIfEmpty());
         list<TurnInfo*>::const_iterator citr = myTurns->begin();
         list<TurnInfo*>::const_iterator citre = myTurns->end();
         while (citr != citre) {
            TurnInfo* turn = *citr++;
            vector<Move> moves;
            vector<Move>::const_iterator cmit = turn->legalMoves->begin();
            while (cmit != turn->legalMoves->end()) {
               Move mv = *cmit++;
               if (ms->apply(turn, mv)) {
                  moves.push_back(mv);
               }
            }
            outs->addTurn(turn, moves);
            count++;
            if (count % 1000 == 0) {
               ccout << "Mapped " << count << " turns." << endl;
            }
         }
	 entering = false;
         citr = myTurns->begin();
         citre = myTurns->end();
         while (citr != citre) {
            TurnInfo* t = *citr++;
            delete(t);
         }
         delete(myTurns);
      }
   }
   ccout << "Created map of size " << outs->mapSize() << endl;
   return outs;
}


 
 
void Ensemble::runDemo(int choice) { 
   cout << "If you *re-run* a demo, you may ignore \"duplicate entry\" " 
        << "messages." << endl << endl; 
   switch(choice) { 
    case Demos::ELISTA_DEMO: 
      Demos::elistaDemo(*this); 
      break; 
    case Demos::SAN_LUIS_MEXICO_DEMO: 
      Demos::sanLuisMexicoDemo(*this); 
      break; 
    case Demos::SWING_DEMO: 
      Demos::swingDemo(*this); 
      break; 
    case Demos::DEMORALIZATION_DEMO: 
      Demos::demoralizationDemo(*this); 
      break; 
    default: 
      Demos::elistaDemo(*this); 
      break; 
   } 
} 
 
 
void Ensemble::doMainMenuItem(int choice) { //INV: focusTrial is never null 
   IRutil::nullstream nullstr; 
   ostream& ccout = usingCin() ? cout : nullstr; 
   bool entering = true; 
   string ans = "";
   string fileName = "";
   string matchString = ""; 
   ifstream myfile; 
   int subChoice = 0; 
   //list<TurnInfo*>::const_iterator tit;
   //list<TurnInfo*>::const_iterator tite;
   //list<GameInfo*>::const_iterator git;
   //list<GameInfo*>::const_iterator gite;

 
   ofstream zscoref("zscores.txt", ios::app); 
   ofstream zscoread("zASD.txt", ios::app);
   ofstream zscoreEV("zscoreEV.txt", ios::app);
   ofstream zscoreSel("zscoreSel.txt", ios::app);   
 
   switch(choice) { 
    case NEW_TRIAL: 
      //newTrial(); 
      //changeTrial();
      //ccout << "Added and loaded " << string(*focusTrial) << endl; 
      ccout << "Feature disabled: please use 2 Change Trial instead." << endl;
      break; 
    case CHANGE_TRIAL: 
      //loadNamedTrial(); 
      changeTrial();
      ccout << "Trial now " << string(*focusTrial) << endl;
      break; 
    case SHOW_TRIAL: 
      for (vector<ostream*>::const_iterator it = outs->begin(); 
           it != outs->end(); it++) { 
         showActiveTrial(**it); 
      } 
      ccout << "Show filtered tuples? (screen only if <= 1000) (y/n) "; 
      loggedRead(ans,false); 
      if (IRutil::isYes(ans)) { 
         for (vector<ostream*>::const_iterator it = outs->begin(); 
              it != outs->end(); it++) { 
            if ((*it) != &cout || focusTrial->numDecisions() <= 1000) { 
               //(**it) << "Tuples:" << endl << focusTrial->tuplesToString()  
               (**it) << "Turns:" << endl << focusTrial->decisionsToString()
                      << endl; 
            } 
         } 
      } else {
         ccout << endl;
      } 
      break; 
    case LOAD_DELTA_SCALE: 
      ccout << "Apply to Reference Trial too? (y/n) ";
      loggedRead(ans,false);
      loadDeltaScale(IRutil::isYes(ans)); 
      break; 
    case ADD_TURNS: 
      addTurns(); 
      ccout << "Current trial now---" << string(*focusTrial) << endl; 
      break; 
    case CLEAR_TURNS: 
      clearTurns();
      break; 
    case NEW_FILTER: 
      newFilters();  //includes a reset 
      break; 
    case ATTACH_FILTERS: 
      attachFilters(); 
      break; 
    case DETACH_FILTERS: 
      detachFilters(); 
      break; 
    case CLEAR_FILTERS: 
      clearFilters();  //includes a reset of filtered tuples 
      break; 
    case HIDE_FILTERS: //allows hiding attached ones and so does reset too 
      hideFilters(); 
      break; 
    case NEW_SELECTOR:
      newSelectors();
      break;
    case TOGGLE_SELECTORS:
      toggleSelectors();
      break;
    case NEW_TRIAL_SPEC: 
      { 
         TrialSpec ts = makeTrialSpec(); 
         IRfun::WEIGHTS wmc = ts.weightMethodChoice; 
         specs->clearStars(); 
         specs->addItem(ts,"",false,true);  //*s it 
         focusTrial->setSpec(ts); 
         referenceTrial->setSpec(ts); 
cerr << "Doing perftest on reference trial...";
         PerfData rpf = referenceTrial->perfTest(false,false); 
         double asd = rpf.scaledFalloffWtd.projectedFreq;
         double ird = IRfun::ipr(asd,wmc); 
cerr << "done." << endl;
         int ir = (int)(ird + 0.5); 
         ccout << "Created " << string(focusTrial->getSpec()) << endl 
               << "with Intrinsic Rating " << ir << " from ASD = " << asd << endl; 
      } 
      break; 
    case LOAD_TRIAL_SPEC: 
      { 
         loadNamedTrialSpec(); 
         TrialSpec ts = focusTrial->getSpec(); 
         IRfun::WEIGHTS wmc = ts.weightMethodChoice; 
         referenceTrial->setSpec(ts); 
cerr << "Doing perftest on reference trial...";
	 PerfData rpf = referenceTrial->perfTest(false,false); 
	 double asd = rpf.scaledFalloffWtd.projectedFreq;
         double ird = IRfun::ipr(asd,wmc);
cerr << "done." << endl;
         int ir = (int)(ird + 0.5); 
         ccout << "Loaded " << string(focusTrial->getSpec()) << endl 
               << "with Intrinsic Rating " << ir << " from ASD = " << asd << endl; 
      } 
      break; 
    case HIDE_SPECS: 
      hideSpecs(); 
      break; 
/*
    case PERCFIT_TRIAL: 
      if (pTurns->empty()) { 
         cout << "Empty data: please add game turns." << endl; 
      } else { 
         focusTrial->percFit(outs); 
      } 
      break; 
*/
    case RUN_FIT: 
      runFit(); 
      break; 
    case PERF_TEST: 
      if (pTurns->empty()) { 
         cout << "Empty data: please add game turns." << endl;  //not ccout 
      } else { 
         runPerfTest();
      }
      break;

/*
    case PERF_TEST_ALL: 
      if (pTurns->empty()) { 
         cout << "Empty data: please add game turns." << endl;   //not ccout 
      } else { 
         bool uw = false; 
         bool tn = true; 
         ccout << "Use focus spec and treat miss as Move " 
               << IRutil::NCUTOFF + 1 << "? (y/n) "; 
         loggedRead(ans,false); 
         ans = IRutil::trim(ans); 
         if (!IRutil::isYes(ans)) { 
            ccout<<"Override weighting function to use unit weights? (y/n) "; 
            loggedRead(ans,false); 
            ans = IRutil::trim(ans); 
            uw = IRutil::isYes(ans); 
            ccout << "Treat played move not in top " << IRutil::NCUTOFF  
                  << " as Move " << (IRutil::NCUTOFF+1) << "? (y/n) "; 
            loggedRead(ans,true); 
            ans = IRutil::trim(ans); 
            tn = IRutil::isYes(ans); 
         } 
         TrialSpec currentSpec = focusTrial->getSpec(); 
         for (int i = 0; i < specs->size(); i++) { 
            TrialSpec ts = (*specs)[specs->nameOf(i)]; 
            focusTrial->setSpec(ts); 
            referenceTrial->setSpec(ts); 
            PerfData pf = focusTrial->perfTest(uw,tn); 
            string rept = pf.fullReport(); 
            for (vector<ostream*>::iterator it = outs->begin(); 
                 it != outs->end(); it++) { 
               (**it) << endl << IRutil::DASHES << endl 
                      << "Test using " << string(*focusTrial) << endl 
                      << IRutil::DASHES << endl << endl; 
               (**it) << rept; 
               (**it) << "Curve points:" << endl; 
               (**it) << focusTrial->showCurvePoints(0.25, 1.00); 
               (**it) << IRutil::DASHES << endl << endl; 
            } 
         } 
         focusTrial->setSpec(currentSpec); 
         referenceTrial->setSpec(currentSpec); 
         resetParamsMenu();
      } 
      break; 
*/
    case ADD_OUTPUT_FILE: 
      entering = true; 
      while (entering) { 
         if (usingCin()) { 
            outsMenu->showMenu(); 
            cout << "Name of new output file to append onto: "; 
         } 
         loggedRead(fileName,true);  //ends line 
         entering = !addOutputFile(fileName); 
      } 
      break; 
    case CLOSE_OUTPUT_FILE: 
      entering = true; 
      while (entering) { 
         subChoice = outsMenu->readChoice(); 
         if (outsMenu->success(subChoice)) { 
            entering = false; 
         } else if (subChoice == EnumMenu::CANCEL_CHOICE) { 
            break; 
         } else { 
            cout << "Invalid entry, please try again." << endl; 
         } 
      } 
      fileName = outsMenu->nameOf(choice); 
      closeOutputFile(fileName); 
      break; 
    case READ_COMMANDS: 
      matchString = ""; 
      cout << "Name of input file with commands: ";    //NOT ccout 
      loggedRead(fileName,false);  //does not end line, writes 
      fileName = IRutil::trim(fileName); 
      myfile.open((const char*)fileName.c_str(),ios::in); 
      if (!myfile.is_open()) { 
         cout << "File not found, can re-select and try again." << endl; 
         return; 
      } //else 
      if (!usingCin()) { 
         cout << "Start from top of file? (y/n) "; 
         loggedReadString(ans,false,false); 
         if (!IRutil::isYes(ans)) { 
            cout << "Enter string to match in COMMENT line, no whitespace.\n"; 
            loggedReadString(matchString,true,true); 
         } 
      } 
      loggedWrite("",true);   //inserts endline 
      zscoref << "Next Test: " << endl; 
      zscoread << "Next AD Test: " << endl; 
      zscoreEV << "Next EV Test: " << endl;
      zscoreSel << "Next Selection Test: " << endl;
      zscoref.close(); 
      zscoread.close(); 
      zscoreEV.close();
      zscoreSel.close();
 
cerr << "Running commands now..." << endl;
      interactFromStream(&myfile,matchString); 
      break; 
/*
    case RUN_DEMO: 
      cout << "Demos splash output on your screen, and draw summaries from " 
           << "this output *only*." << endl << endl; 
      subChoice = demosMenu->readChoice(); 
      while (!(demosMenu->success(subChoice) 
               || subChoice == EnumMenu::CANCEL_CHOICE)) { 
         cout << "Please try again. " << endl; 
         subChoice = demosMenu->readChoice(); 
      } 
      if (demosMenu->success(subChoice)) { 
         runDemo(subChoice); 
      } 
      break; 
*/
    case QUIT: 
      keepGoing = false; 
      break; 
/*---------------------------------------------- 
    case EnumMenu::INVALID_CHOICE: 
      if (usingCin()) { 
         cout << "Invalid choice, please try again." << endl; 
      } else { 
         cerr << "Bad top-level menu choice, trying to recover..." << endl; 
      } 
      break; 
    case EnumMenu::CANCEL_CHOICE: 
      if (usingCin()) { 
         cout << "Cancel propagated to top level, did you meant to quit?\n"; 
      } else { 
         cerr << "Bad top-level menu choice, trying to recover..." << endl; 
      } 
      break; 
    case EnumMenu::STREAM_END_CHOICE: 
*///-------------------------------------------- 
    default: //shouldn't happen either---handled by caller interactFromStream() 
      if (choice != EnumMenu::STREAM_END_CHOICE) { 
         cout << "Could not execute your choice; please try again." << endl; 
      } 
      INP->clear(); 
      INP->ignore(numeric_limits<streamsize>::max(),'\n'); 
      break; 
   } 
} //end of implementation of class Ensemble 
 
 
#endif    //end of #ifndef __IR_ENSEMBLE_CPP__ 
 
 
 
