//File----------------------Shuffle.h------------------------------------- 
 
#ifndef __IR_SHUFFLE_H__
#define __IR_SHUFFLE_H__
#endif

enum SHUFFLES { 
   NO_SHUFFLE, 
   SWAP_FIRST_TWO, 
   FLIP, 
   SWAP_HALVES, 
   CYCLE_LEFT, 
   CYCLE_RIGHT, 
   CYCLE_BY_TURN_NO, 
   CYCLE_FLIP_TURN_NO, 
   NUM_TWEAKS, 
   APPLY_ALL_TURNS, 
   APPLY_ODD_TURNS, 
   APPLY_EVEN_TURNS, 
   APPLY_IF_RANDOM, 
   APPLY_RANDOM_TWEAK, 
   FULLY_RANDOMIZE, 
   NUM_SHUFFLES 
}; 
 
/** Abstract class representing a permutation of the list of moves 
    returned by the engine---more precisely, of /the way that list is read/ 
    during percentile-fitting in particular. 
    N stands for NCUTOFF (== 10 or 20), NN for N+1. 
 
    Design philosophy: Each class (not object) fixes a pre-computed list PP  
    of permutations to use for each possible tuple length 1..N or 1..NN.   
    The abstract base class constructs PP as identity permutations, then 
    derived classes modify them---the redundant computation does not affect 
    the main loop.   
 */ 
class Shuffle : public Catalogable { 
 protected: 
   vector<vector<int>* >* permutations; //one perm for each tuple length 1..NN 
   vector<int>* permNNFixed;  //to use when fixing element NN regardless. 
 private: 
  // int fixDistortion; //if <= 0, permutations go to N only 
   bool includeMoveNN; //can decide 'no' even if fixDistortion is true 
   string name; 
   int tupleNumber; 
   bool adaptsToTurn;  //false => same permutation for all turns, so we need 
                       //only return a pointer to a pre-computed permutation 
    
  /** Return a permutation indexing the probability array that will be  
      derived from the turn's delta array, 1 larger if includeMoveNN. 
      If (adaptsToTurn), then the permutation to return is pre-computed, 
      else we may assume that the permutation is initially the identity. 
 
      Private so that the public method guarantees incrementing tupleNumber. 
      CSE703: Is there a better way to represent this kind of *behavioral* 
      property, a-la my proposal for *static* properties in the type system? 
   */ 
  // virtual vector<int>* makePerm(const TurnInfo* const pTurn) const = 0; 
 
 public: 
   Shuffle(const string& gname, bool imNN, bool a)  
      : name(gname), includeMoveNN(imNN), tupleNumber(0), adaptsToTurn(a), 
        permutations(new vector<vector<int>* >(IRutil::MAXNUMLEGALMOVES)), 
	permNNFixed(new vector<int>(IRutil::MAXNUMLEGALMOVES)) { 
      for (int i = 0; i < IRutil::NCUTOFF + 1; i++) { 
         permutations->at(i) = new vector<int>(i); 
         for (int j = 0; j < i; j++) { 
             permutations->at(i)->at(j) = j; 
         } 
         permNNFixed->at(i) = i; 
      } 
   } 
   virtual ~Shuffle() {
      for (int i = 0; i < IRutil::NCUTOFF + 1; i++) {
         delete(permutations->at(i));
      }
      delete(permutations);
      delete(permNNFixed);
   } 
 
   /** Return a permutation indexing the probability array that will be  
       derived from the turn's delta array.  Static binding, do not shadow! 
    */ 
   //vector<int>* makePermutation(const TurnInfo* const pTurn ) { 
   //   tupleNumber++;  //so we index tuples from 1 not 0 
   //   return makePerm(turn); 
   //} 
   int numProbs(const TurnInfo* const pTurn) { 
	   int s = pTurn->depthEvalsCP->size(); 
      if (s == IRutil::NCUTOFF && includeMoveNN) { s++; } 
      return s; 
   } 
   int getTupleNumber() { return tupleNumber; } 
   void resetTupleNumber() { tupleNumber = 0; } 
   void incTupleNumber() { tupleNumber++; } 
   bool includesMoveNN() { return includeMoveNN; } 
   string getName() const { return name; } 
   virtual string getDescription() const = 0; 
   virtual operator string() const { 
      return getName() + ": " + getDescription(); 
   } 
}; 
 
//class IdentityPerm : public Shuffle { 
// public: 
//   IdentityPerm(bool imNN) : Shuffle("Id", imNN, false) { } 
//   virtual vector<int>* makePerm(const TurnInfo* turn const) const { 
//      return permutations 
//    
// 
//class ApplyOddTurns : public Shuffle { 
//   Shuffle* shuffle; 
//   virtual vector<int>* makePerm(const TurnInfo* turn const) const { 
//      if (getTupleNumber() % 2) { 
//         return  
// public: 
//   ApplyOddTurns 
//    
// 
// 
//   /** Permutation generator, used by translateTurn 
//       Assumes arr has correct size, but does not assume it's initialized. 
//    */ 
//   void makePermutation(vector<int>* arr, const TurnInfo* const pTurn) const { 
//      static int tnum; 
//      tnum++; 
//      int psize = arr->size(); 
//      int start; 
//      int permEnd = psize; 
//      int huffle = shuffle; 
//      if (psize == 1 + IRutil::NCUTOFF && (!includeMoveNN)) { 
//         permEnd--; 
//         arr->at(permEnd) = permEnd; 
//      } 
//      switch(when) { 
//       case APPLY_ALL_TURNS: 
//         break; 
//       case APPLY_ODD_TURNS: 
//         if (tnum % 2 == 0) { return; } 
//         break; 
//       case APPLY_EVEN_TURNS: 
//         if (tnum % 2 == 1) { return; } 
//         break; 
//       case APPLY_IF_RANDOM: 
//         srand(pTurn->srandSeed); 
//         if (IRutil::myRand(0,2) % 2 == 0) { return; } 
//         break; 
//       case APPLY_RANDOM_TWEAK: 
//         srand(pTurn->srandSeed); 
//         huffle = IRutil::myRand(0,NUM_TWEAKS); 
//         break; 
//       case FULLY_RANDOMIZE: 
//         srand(pTurn->srandSeed); 
//         IRutil::randShuffle(arr, 0, permEnd); 
//         return;  //nothing more to do 
//       default: 
//         break; 
//      } 
//      switch(huffle) { 
//       case NO_SHUFFLE: 
//         for (int i = 0; i < permEnd; i++) { 
//            arr->at(i) = i; 
//         } 
//         break; 
//       case SWAP_FIRST_TWO: 
//         if (permEnd >= 2) { 
//            arr->at(0) = 1; 
//            arr->at(1) = 0; 
//         } 
//         for (int i = 2; i < permEnd; i++) { 
//            arr->at(i) = i; 
//         } 
//         break; 
//       case FLIP: 
//         for (int i = 0; i < permEnd; i++) { 
//            arr->at(i) = psize - i - 1; 
//         } 
//         break; 
//       case SWAP_HALVES: 
//         start = (permEnd + 1) / 2;  // = 6 when permEnd = 11 
//         for (int i = 0; i < permEnd; i++) { 
//            arr->at(i) = (i + start) % permEnd; 
//         } 
//         break; 
//       case CYCLE_LEFT: 
//         for (int i = 0; i < permEnd - 1; i++) { 
//            arr->at(i) = i + 1; 
//         } 
//         arr->at(permEnd - 1) = 0; 
//         break; 
//       case CYCLE_RIGHT: 
//         arr->at(0) = permEnd - 1; 
//         for (int i = 1; i < permEnd; i++) { 
//            arr->at(i) = i - 1; 
//         } 
//         break; 
//       case CYCLE_BY_TURN_NO: 
//         for (int i = 0; i < permEnd; i++) { 
//            arr->at(i) = (i + pTurn->turnNumber) % permEnd; 
//         } 
//         break; 
//       case CYCLE_FLIP_TURN_NO: 
//         for (int i = 0; i < permEnd; i++) { 
//            arr->at(i) = (pTurn->turnNumber % 2 == 0) ? i : permEnd - i - 1; 
//         } 
//         for (int i = 0; i < permEnd; i ++) { 
//            arr->at(i) = (arr->at(i) + (pTurn->turnNumber / 2)) % permEnd; 
//         } 
//         break; 
//       default:  //do nothing 
//         break; 
//      } 
//   } 
// 
 
 
 
 
