//File--------------------IRmiscGlobal.h------------------------------------ 
 
#ifndef __IR_MISC_GLOBAL_H__ 
#define __IR_MISC_GLOBAL_H__ 
 
class Catalogable { 
   virtual string getName() const = 0; 
   virtual string getDescription() const = 0; 
 public: 
   virtual ~Catalogable() { } 
}; 
 
#endif   //end of #ifndef __IR_MISC_GLOBAL_H__ 
 
 
 
