//File---------------------PerfData.cpp---------------------------------- 
 
#ifndef __IR_PERFDATA_CPP__ 
#define __IR_PERFDATA_CPP__ 

#ifndef __IR_PERFDATA_H__
#include "PerfData.h"
#endif   //end of #ifndef __IR_PERFDATA_H__


string PerfData::fullReport(size_t numIndices, bool clumpRest) const {

   if (numTurns == 0) { return "No turns---nothing to see."; }
   if (numIndices > moveIndexTestsWtd.size()) { numIndices = moveIndexTestsWtd.size(); }

   ostringstream o;
   string adj = weighted ? "Wtd." : "Unwtd.";
   o << "From " << numTurns << " turns with total weight " << accumWeight 
     << ", move indices first:" << endl << endl;

   o << MoveIndexTest::header() << endl;
   for (int i = 0; i < numIndices; i++) {
      o << string(moveIndexTestsWtd.at(i)) << endl;
   }
   o << "Index fits, x10,000: "
          << setprecision(4) << 10000.0*indexFit << ", wtd. "
          << 10000.0*indexFitWtd << ", diff " << indexDiffWtd << "; mass " << 10000.0*indexFitMass
          << ", wtd. " << 10000.0*indexFitMassWtd << "; diff " << indexDiffMassWtd << endl;

   o << endl << AggregateTest::header() << endl;
   o << string(scaledFalloffWtd) << endl;
   o << string(moveMatchWtd) << endl;
   o << string(equalValueMatchWtd) << endl;
   o << string(playedMoveMatchWtd) << endl;

   size_t numST = selectionTestsWtd.size();
   if (numST > 0) {
      o << endl << SelectionTest::header() << endl;
      for (int j = 0; j < numST; j++) {
         o << string(selectionTests.at(j)) << endl;
      }
   }

   ofstream zscoref("zMM.txt", ios::app);
   zscoref << fixed << showpoint << setprecision(4) << setw(7) << right
           << moveMatchWtd.zScore << endl;
   zscoref.close();

   ofstream zscorev("zEV.txt", ios::app);
   zscorev << fixed << showpoint << setprecision(4) << setw(7) << right
           << equalValueMatchWtd.zScore << endl;
   zscorev.close();

   ofstream zscoread("zASD.txt", ios::app);
   zscoread << fixed << showpoint << setprecision(4) << setw(7) << right
           << scaledFalloffWtd.zScore << endl;
   zscoread.close();


   return(o.str());
}
 
string PerfData::fullReportOld() const { 
 
   if (numTurns == 0) { return "No turns---nothing to see."; } 
 
   ostringstream o; 
   string adj = weighted ? "Wtd." : "Unwtd."; 
 
   //IMPT: "Test using" and "total turns" are searched for in demos. 
 
   o << "From " << numTurns << " total turns (" << fixed << setprecision(2) 
   << accumWeight << " weighted), it is consistent to expect\n" 
   << "the level of players sampled to have the following stats:\n\n" 
   << adj << "  first-move matches: " 
   << fixed << setprecision(2) << right 
   << setw(8) 
   << moveMatchWtd.projectedValue << ", stdev " << moveMatchWtd.projectedSigma << ", " 
   << setw(5) 
   << 100.0*moveMatchWtd.projectedFreq << "%" << endl 
   << "Two-sigma range: " 
   << setw(8) 
   << moveMatchWtd.projected2SigmaLeft << "--" << moveMatchWtd.projected2SigmaRight << ", " 
   << setw(5) 
   << 100*moveMatchWtd.projected2SigmaLeftFreq << "%--" << 100*moveMatchWtd.projected2SigmaRightFreq << "%"  
   << endl << endl 

   << adj << " eq.top-move matches: " 
   << setw(8) 
   << equalValueMatchWtd.projectedValue << ", stdev " << equalValueMatchWtd.projectedSigma << ", " 
   << setw(5) 
   << 100.0*equalValueMatchWtd.projectedFreq << "%" << endl 
   << "Two-sigma range: " 
   << setw(8) 
   << equalValueMatchWtd.projected2SigmaLeft << "--" << equalValueMatchWtd.projected2SigmaRight << ", " 
   << setw(5) 
   << (100.0*equalValueMatchWtd.projected2SigmaLeftFreq) << "%--" << (100.0*equalValueMatchWtd.projected2SigmaRightFreq) << "%" 
   << endl << endl 

   << adj << " played-move matches: " 
   << setw(8) 
   << playedMoveMatchWtd.projectedValue << ", stdev " << playedMoveMatchWtd.projectedSigma << ", " 
   << setw(5) << (100.0*playedMoveMatchWtd.projectedFreq) << "%" << endl 
   << "Two-sigma range: " 
   << setw(8) 
   << playedMoveMatchWtd.projected2SigmaLeft << "--" << playedMoveMatchWtd.projected2SigmaRight << ", " 
   << setw(5) 
   << (100.0*playedMoveMatchWtd.projected2SigmaLeftFreq) << "%--" << (100.0*playedMoveMatchWtd.projected2SigmaRightFreq) << "%"  
   << endl << endl;

 
 if (weighted) { 
   adj = "Unwtd."; 
   o << "From " << numTurns << " total turns (" << fixed << setprecision(2)
   << accumWeight << " weighted), it is consistent to expect\n"
   << "the level of players sampled to have the following stats:\n\n"
   << adj << "  first-move matches: "
   << fixed << setprecision(2) << right
   << setw(8)
   << moveMatch.projectedValue << ", stdev " << moveMatch.projectedSigma << ", "
   << setw(5)
   << 100.0*moveMatch.projectedFreq << "%" << endl
   << "Two-sigma range: "
   << setw(8)
   << moveMatch.projected2SigmaLeft << "--" << moveMatch.projected2SigmaRight << ", "
   << setw(5)
   << 100*moveMatch.projected2SigmaLeftFreq << "%--" << 100*moveMatch.projected2SigmaRightFreq << "%"
   << endl << endl

   << adj << " eq.top-move matches: "
   << setw(8)
   << equalValueMatch.projectedValue << ", stdev " << equalValueMatch.projectedSigma << ", "
   << setw(5)
   << 100.0*equalValueMatch.projectedFreq << "%" << endl
   << "Two-sigma range: "
   << setw(8)
   << equalValueMatch.projected2SigmaLeft << "--" << equalValueMatch.projected2SigmaRight << ", "
   << setw(5)
   << (100.0*equalValueMatch.projected2SigmaLeftFreq) << "%--" << (100.0*equalValueMatch.projected2SigmaRightFreq) << "%"
   << endl << endl

   << adj << " played-move matches: "
   << setw(8)
   << playedMoveMatch.projectedValue << ", stdev " << playedMoveMatch.projectedSigma << ", "
   << setw(5) << (100.0*playedMoveMatch.projectedFreq) << "%" << endl
   << "Two-sigma range: "
   << setw(8)
   << playedMoveMatch.projected2SigmaLeft << "--" << playedMoveMatch.projected2SigmaRight << ", "
   << setw(5)
   << (100.0*playedMoveMatch.projected2SigmaLeftFreq) << "%--" << (100.0*playedMoveMatch.projected2SigmaRightFreq) << "%"
   << endl << endl;

 } //end of parenthetical if (weighted) 
 
   o << endl << "Predicted Unweighted and Weighted"  
          << " played-move index freqs; wtd. 2-sigma range:" << endl; 
   for (int i = 0; i <= IRutil::NCUTOFF; i++) { 
      double expi = moveIndexTests.at(i).projectedValue; // predMoveIndices.at(i); 
      double expiw = moveIndexTestsWtd.at(i).projectedValue; //predMoveIndicesWtd.at(i); 
      double sigi = moveIndexTests.at(i).projectedSigma;  //sqrt(predMoveVariances.at(i)); 
      double sigiw = moveIndexTestsWtd.at(i).projectedSigma;  //sqrt(predMoveVariancesWtd.at(i)); 
      o << setw(2) << right << i << ":" << fixed << setprecision(2) 
             << setw(9) << right << expi << ", " 
	     << setw(5) << right << 100.0*expi/numTurns << "%;" 
             << setw(9) << right << expiw << ", " 
             << setw(5) << right << 100.0*expiw/accumWeight << "%; " 
	     << "2sr:" 
	     << setw(9) << right << expiw-2*sigiw << "--" 
             << setw(9) << left << expiw+2*sigiw << ", " 
             << setw(5) << right << 100.0*(expiw-2*sigiw)/accumWeight 
             << "%--" << setw(5) << right << 100.0*(expiw+2*sigiw)/accumWeight 
             << "%" << endl; 
   } 
   o << "(Last line covers all moves not in the top N.)" << endl; 
 
   o << endl << "Histogram of indexes of played moves, " 
          << "capping blunder deltas at " << deltaCap << ":" << endl; 
   for (int i = 0; i <= IRutil::NCUTOFF; i++) { 
      o << setw(3) << right << i << ": " << fixed << setprecision(2) 
             << setw(5) << right << moveIndexTests.at(i).measuredValue  //playedMoveIndexHistogram.at(i)
             << ", " << setw(5) << right 
             << 100.0*moveIndexTests.at(i).measuredFreq  //playedMoveIndexHistogram.at(i)/numTurns
             << "%,  wtd. " << setw(8) << right 
             << moveIndexTestsWtd.at(i).measuredValue  //playedMoveIndexHistogramWtd.at(i) 
             << ", " << setw(5) << right 
             << 100*moveIndexTestsWtd.at(i).measuredFreq  //playedMoveIndexHistogramWtd.at(i) / accumWeight 
             << "%;  wtd. mean delta = " << setw(5) << right 
             << moveIndexTestsWtd.at(i).meanDelta   
             << endl; 
   } 
   o << "Index fits, x10,000: " 
          << setprecision(4) << 10000.0*indexFit << ", wtd. "  
          << 10000.0*indexFitWtd << "; mass " << 10000.0*indexFitMass 
          << ", wtd. " << 10000.0*indexFitMassWtd << endl << endl 
          << setprecision(2) 
          << "Number of first-line matches: " 
          << moveIndexTests.at(0).measuredValue   //playedMoveIndexHistogram.at(0)
          << ", " << (100.0*moveIndexTests.at(0).measuredFreq)  //playedMoveIndexHistogram.at(0)/numTurns) 
          << "%; wtd. " << moveIndexTestsWtd.at(0).measuredValue //    playedMoveIndexHistogramWtd.at(0) 
          << ", " << (100.0*moveIndexTestsWtd.at(0).measuredFreq) //playedMoveIndexHistogramWtd.at(0)/accumWeight) 
          << "%" << endl 
          << "Number of equal-top matches:  " << equalValueMatch.measuredValue //topMatches 
          << ", " << (100.0*equalValueMatch.measuredFreq)  //topFreq) 
          << "%" << "; wtd. " << equalValueMatchWtd.measuredValue  //topMatchesWtd 
          << ", " << (100.0*equalValueMatchWtd.measuredFreq)   //topFreqWtd) 
          << "%" << endl;

   o << endl << "Selection Tests" << endl << SelectionTest::header() << endl;
   vector<SelectionTest>::const_iterator vsti = selectionTestsWtd.begin();
   vector<SelectionTest>::const_iterator vste = selectionTestsWtd.end();
   while (vsti != vste) {
      o << string(*vsti++) << endl;
   }
 
   o << endl << "Finally, expected and actual falloff, " 
      << "with blunder cap as set at " << deltaCap << ":" << endl 
      << setprecision(2) << setw(8) << right 
      << "Expected scaled weighted:   " << setw(8) << scaledFalloffWtd.projectedValue  
      << ", 2-sigma range: " << scaledFalloffWtd.projected2SigmaLeft 
      << "--"  << scaledFalloffWtd.projected2SigmaRight << endl 
      << "Actual scaled weighted:     " << setw(8) << scaledFalloffWtd.measuredValue << endl 
      << "Expected unscaled weighted: " << setw(8) << unscaledFalloffWtd.projectedValue 
      << ", 2-sigma range: " << unscaledFalloffWtd.projected2SigmaLeft << "--" 
      << unscaledFalloffWtd.projected2SigmaRight << endl 
      << "Actual unscaled weighted:   " << setw(8) << unscaledFalloffWtd.measuredValue << endl 
      << "Per-move figures:" << endl 
      << setprecision(4) << setw(5) << right 
      << "Expected scaled weighted:    " << scaledFalloffWtd.projectedFreq  
      << ",   2-sigma range: " << scaledFalloffWtd.projected2SigmaLeftFreq << "--" 
      << scaledFalloffWtd.projected2SigmaRightFreq << endl 
      << "Actual scaled weighted:       " << scaledFalloffWtd.measuredFreq << endl 
      << "Expected unscaled weighted:  " << unscaledFalloffWtd.projectedFreq  
      << ",   2-sigma range: " << unscaledFalloffWtd.projected2SigmaLeftFreq << "--" 
      << unscaledFalloffWtd.projected2SigmaRightFreq << endl 
      << "Actual unscaled weighted:     " << unscaledFalloffWtd.measuredFreq << endl 
      << endl 
 
      << "Unweighted versions:" << endl 
      << setprecision(2) << setw(8) << right 
      << "Expected scaled:   " << setw(8) << scaledFalloff.projectedValue << ", " 
      << "2-sigma range: " << scaledFalloff.projected2SigmaLeft << "--" 
      << scaledFalloff.projected2SigmaRight << endl 
      << "Actual scaled:     " << setw(8) << scaledFalloffWtd.measuredValue << endl 
      << "Expected unscaled: " << setw(8) << unscaledFalloff.projectedValue << ", " 
      << "2-sigma range: " << unscaledFalloff.projected2SigmaLeft << "--" 
      << unscaledFalloff.projected2SigmaRight << endl 
      << "Actual unscaled:   " << setw(8) << unscaledFalloff.measuredValue << endl 
      << "Per-move figures:" << endl 
      << setprecision(4) << setw(5) << right 
      << "Expected scaled:    " << scaledFalloff.projectedFreq << ",   " 
      << "2-sigma range: " << scaledFalloff.projected2SigmaLeftFreq << "--" 
      << scaledFalloff.projected2SigmaRightFreq << endl 
      << "Actual scaled:      " << scaledFalloff.measuredFreq << endl 
      << "Expected unscaled:  " << unscaledFalloff.projectedFreq << ",   " 
      << "2-sigma range: " << unscaledFalloff.projected2SigmaLeftFreq << "--" 
      << unscaledFalloff.projected2SigmaRightFreq << endl 
      << "Actual unscaled:    " << unscaledFalloff.measuredFreq << endl 
      << endl 
 
      << "------" << endl 
      << setprecision(2) << setw(8) 
      << "Actual capped unweighted:   " << unscaledFalloff.measuredValue << ", " 
      << setprecision(4) << setw(5) 
      << (unscaledFalloff.measuredFreq) << " per move;" << endl 
      << setprecision(2) << setw(8) 
      << "Figured by next turns:      " << scaledFalloffNextTurnCapped.measuredValue << ", " 
      << setprecision(4) << setw(5) 
      << (scaledFalloffNextTurnCapped.measuredFreq) << " per move;" << endl << endl; 
 
   double zscoreFirst = moveMatch.zScore;
   double zscoreTop = equalValueMatch.zScore;
   //double zscoreSelection = (selectionMatches - predSelectionMatches)/selectionSigma;
   double zscoreFalloff = unscaledFalloff.zScore;
   double zscoreScaledFalloff = scaledFalloff.zScore;

   double zscoreFirstWtd = moveMatchWtd.zScore;
   double zscoreTopWtd = equalValueMatchWtd.zScore;
   //double zscoreSelectionWtd = (selectionMatchesWtd - predSelectionMatchesWtd)/selectionSigmaWtd;
   double zscoreFalloffWtd = unscaledFalloffWtd.zScore;
   double zscoreScaledFalloffWtd = scaledFalloffWtd.zScore;

 
 
   o << endl 
      << "Matching Z-scores: firstMM = " << zscoreFirst  
      << ", topMM  = " << zscoreTop << endl  //", selection = " << zscoreSelection << endl 
      << "Falloff Z-scores: unscaled = " << zscoreFalloff 
      << ", scaled = " << zscoreScaledFalloff << endl; 
 
   o //<< endl 
      << "Matching Z-scores weighted: firstMM = " << zscoreFirstWtd 
      << ", topMM  = " << zscoreTopWtd << endl   //", selection = " << zscoreSelectionWtd << endl 
      << "Falloff Z-scores weighted: unscaled = " << zscoreFalloffWtd 
      << ", scaled = " << zscoreScaledFalloffWtd << endl; 
 
   ofstream zscoref("zMM.txt", ios::app); 
   zscoref << fixed << showpoint << setprecision(4) << setw(7) << right 
           << zscoreFirst << endl; 
   zscoref.close(); 

   ofstream zscorev("zEV.txt", ios::app);
   zscorev << fixed << showpoint << setprecision(4) << setw(7) << right
           << zscoreTop << endl;
   zscorev.close();
 
   ofstream zscoread("zASD.txt", ios::app); 
   zscoread << fixed << showpoint << setprecision(4) << setw(7) << right 
           << -zscoreScaledFalloff << endl; 
   zscoread.close(); 
 
 
       
 
   o << endl; 
 
   return o.str(); 
}                
 
#endif    //end of #ifndef __IR_PERFDATA_CPP__ 
 
 
 
