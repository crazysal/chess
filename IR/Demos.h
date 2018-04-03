//File--------------------Demos.h-------------------------------------- 
 
#ifndef __IR_DEMOS_H__ 
#define __IR_DEMOS_H__ 

#ifndef __IR_INCLUDES_H__
#include "IRincludes.h"
#endif

class Ensemble; 
 
namespace Demos { 
   enum DEMOS { 
      ELISTA_DEMO, 
      SAN_LUIS_MEXICO_DEMO, 
      SWING_DEMO, 
      DEMORALIZATION_DEMO, 
      NUM_DEMOS 
   }; 
   void elistaDemo(Ensemble& ens); 
   void sanLuisMexicoDemo(Ensemble& ens); 
   void swingDemo(Ensemble& ens); 
   void demoralizationDemo(Ensemble& ens); 
}; 
 
#endif    //end of #ifndef __IR_DEMOS_H__ 
 
 
 
