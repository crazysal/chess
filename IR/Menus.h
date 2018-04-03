//File-----------------------Menus.h--------------------------------------- 
 
#ifndef __IR_MENUS_H__ 
#define __IR_MENUS_H__ 
 
//---------UNIFORM MENU SYSTEM, WITH AUTOMATIC LOGGING OF CHOICES----------- 
 
class Logging { 
 public: 
   virtual void loggedReadString(string& item, bool endLine = false, 
                                 bool write = true) const = 0; 
   virtual void loggedReadInt(int& item, bool endLine = false, 
                              bool write = true) const = 0; 
   virtual void loggedReadDouble(double& item, bool endLine = false, 
                                 bool write = true) const = 0; 

   //can't write a virtual template function in C++! 
   virtual void loggedWrite(const string& st, bool endLine = false)  const = 0; 
   virtual string loggedGetline() const = 0; 
   virtual bool atEOF() const = 0; 
   virtual bool usingCin() const = 0; 
   virtual ~Logging() { } 
}; 
 
 
/** Menu class allowing user selection by number *or* by name---importantly, 
    it logs the name via callback to the held "Logging" (->Ensemble) object. 
    Designed to be used in-tandem with an enum---the caller writes a switch 
    with the int returned by readChoice() over the enum. 
    Assumes the enum begins at 0, and otherwise knows nothing about it. 
    Employs special constants with int values < 0 for other status info. 
    Allows displayed menu to begin with "1" or larger indices, but keeps 
    the returned index zero-based.  Provides several menu-display options. 
    *Does not encourage toggling*---toggling makes the logged commands 
    harder to read because meaning depends on non-local context. 
    It might be a good idea to allow int entry only when ensp->isUsingCin(). 
 
    This base class governs display and iteration by the enum's indices, not 
    by the map's sorting (which is alphabetical by the menu option's name). 
    Intended---perhaps dubiously---for derivation to classes that can change 
    the indexing to alphabetical, and build other behavior on top of this. 
    The main dubious aspect is that alpha-indexing runs apart from the idea of 
    using an enum, and mandates returning the user choice as a string not int. 
 */ 
class EnumMenu {   //holds a Logging object for callbacks to loggedRead* 
 protected: 
   struct Entry { 
      int index;          //settable by DynamicMenu etc. 
      const string name; 
      string description; //ditto 
      string status; 
      bool resetToDefault; 
      bool starred; 
      const bool novel; 
      //derived field 
      const string defaultStatus; 
      const bool defaultStarred; 
 
      Entry(int i, const string& n, const string& desc, const string& s,
            bool rtd, bool st, bool nv) 
         : index(i), name(IRutil::trim(n)), description(IRutil::trim(desc)),  
           status(IRutil::trim(s)), resetToDefault(rtd), starred(st), novel(nv), 
           defaultStatus(IRutil::trim(s)), defaultStarred(st) 
      { } 
      ~Entry() { } 
   }; 
 
   Logging* ensp; 
   map<string,Entry*> entries; 
   int numItems;            //given but not fixed at creation 
   vector<string> names;    //governs mapping of indices to names 
   int firstDisplayedIndex; //not const since mutatable by descendants. 
   bool namesAfter; 
   bool padNames;           //relevant only if !namesAfter 
   bool padDescriptions;
   string preamble; 
   string postamble; 
   bool cancelable; 
   bool speakMoreOptions; 
   bool screenAnyway; 
   int maxNameLength; 
   int maxDescriptionLength;
 
 public: 
   static const int INVALID_CHOICE = -1; 
   static const int MORE_CHOICE = -2; 
   static const int CANCEL_CHOICE = -3; 
   static const int STREAM_END_CHOICE = -4; 
   const string INVALID_NAME;   // = "INVALID_NAME";  //not logged 
   const string MORE_NAME;      // = string("moreOptions").c_str();     //logged 
   const string CANCEL_NAME;    // = "cancelChoice";   //logged 
   const string STREAM_END_NAME;//= "STREAM_END";    //not logged 
 
 private: 
   /** Displays entries with "firstDisplayedIndex" offset added to "true" 
       indices, and with names before or after menu-item descriptions. 
    */ 
   void displayEntry(const Entry& e, int offset = 0) const { 
      int indexWidth = (numItems <= 10 - firstDisplayedIndex - offset ? 1 : 2); 
                                   //presume numItems <= 99 
      int numDigits = (e.index <= 9 - firstDisplayedIndex - offset ? 1 : 2); 
      if (e.novel) { cout << IRutil::DASHLINE << endl; } 
      string sep = e.starred ? "]*" : "] "; 
      string pre = (numDigits == 1 && indexWidth == 2) ? " " : ""; 
      bool hasDesc = e.description.size() > 0 && e.status.size() > 0;
      cout << pre << "[" << setw(numDigits) << right 
           << (e.index + firstDisplayedIndex) << sep; 
      if (namesAfter) { 
         cout << e.description << " <" << e.name << "> " 
              << e.status << endl; 
      } else { 
         cout << left << setw(padNames ? maxNameLength : 1) << e.name << ": " 
              << setw((hasDesc && padDescriptions) ? maxDescriptionLength : 0)
              << e.description << (hasDesc ? ": " : "") << e.status << endl; 
      } 
   } 
 
 
 public: 
   EnumMenu(Logging* enp, int gsize, int fdi = 1, bool na = false, 
            bool pa = false, const string& pre = "", const string& post = "", 
            bool ca = false, bool smo = false, bool sa = false) 
      : ensp(enp), entries(map<string,Entry*>()), numItems(gsize), 
        names(vector<string>(gsize)), firstDisplayedIndex(fdi), 
        namesAfter(na), padNames(pa), padDescriptions(pa),
        preamble(pre), postamble(post), cancelable(ca), speakMoreOptions(smo), 
        screenAnyway(sa), maxNameLength(0), maxDescriptionLength(0),
        INVALID_NAME("INVALID_NAME"), MORE_NAME("moreOptions"),
        CANCEL_NAME("cancelChoice"), STREAM_END_NAME("STREAM_END")

   { } 
 
   virtual ~EnumMenu() { }  //will null not delete the ensp pointer 
 
   int size() const { return numItems; } 
 
   bool success(int i) const { 
      return i >= 0 && i < numItems;   //zero-basedness of true index 
   } 
 
   string nameOf(int i) {   //checked 
      if (i < numItems) { 
         return names.at(i); 
      } else { 
         cerr << "Invalid index, returning empty string ..." << endl; 
         return ""; 
      } 
   } 

   int indexOf(string name) {
      for (int i = 0; i < numItems; i++) {
         if (names.at(i) == name) {
            return i;
         }
      }
      cerr << "Name " << name << " not found." << endl;
      return -1;
   }
 
   void addEntry(int index, const string& nm, const string& desc,
                 const string& status = "",
                 bool rd = true, bool starred = false, bool novel = false) { 
      string name = IRutil::trim(nm); //assumes will not be a duplicate! 
      if (index < numItems) { 
         names.at(index) = name; 
      } else if (index == numItems) { 
         names.push_back(name); 
         numItems++; 
      } else { 
         cout << "Invalid entry index " << index << ", sorry." << endl; 
         return; 
      } //so control past here means success.  OK not to delete next
      entries[name] = new Entry(index,name,desc,status,rd,starred,novel); 
      if (name.size() > maxNameLength) { 
         maxNameLength = name.size(); 
      } 
      if (desc.size() > maxDescriptionLength) {
         maxDescriptionLength = desc.size();
      }
   } 
 
   virtual void addEntry(const string& name, const string& desc,
                         const string& status = "", 
                bool rd = true, bool starred = false, bool novel = false) { 
      addEntry(numItems, name, desc, status, rd, starred, novel); 
   } 
 
   void resetDefaults() { 
      for (map<string,Entry*>::iterator it = entries.begin(); 
           it != entries.end(); it++) { 
         if (it->second->resetToDefault) { 
            it->second->status = it->second->defaultStatus; 
            it->second->starred = it->second->defaultStarred; 
         } 
      } 
   } 
 
 
   virtual void showMenu(int offset = 0) { 
      if (preamble != "") { cout << preamble << endl; } 
      for (int i = 0; i < numItems; i++) { 
         displayEntry(*(entries[names.at(i)]));  //not const since map lookup 
      } 
      if (postamble != "") { cout << postamble << endl; } 
      if (cancelable || speakMoreOptions) { 
         cout << "Enter "; 
         if (cancelable) { 
            cout << CANCEL_CHOICE << "/" << CANCEL_NAME << " to cancel"; 
            if (speakMoreOptions) { 
               cout << "; " << MORE_CHOICE << "/" << MORE_NAME 
                    << " for more options."; 
            } else { 
               cout << "."; 
            } 
         } else { 
            cout << MORE_CHOICE << "/" << MORE_NAME << " for more options."; 
         } 
         cout << endl; 
      } 
   } 
 
   bool isStarred(const string& name) {                 //unchecked! 
      bool st = entries[name]->starred; 
      return entries[name]->starred; 
   } 
   bool isStarred(int i) { 
      return success(i) && isStarred(names.at(i)); 
   } 
   void setStarred(const string& name, bool starIt) {   //unchecked! 
      entries[name]->starred = starIt; 
   } 
   void setStarred(int i, bool starIt) {         //checked 
      if (success(i)) { 
         setStarred(names.at(i),starIt); 
      } else { 
         cout << "Invalid entry index " << i << "in setStarred, sorry." << endl; 
      } 
   } 
   void clearStars() { 
      for (map<string,Entry*>::iterator it = entries.begin(); 
           it != entries.end(); it++) { 
         it->second->starred = false; 
      } 
   } 

   void setDescription(const string& name, const string& newDescription) {
      entries[name]->description = IRutil::trim(newDescription);
   }
   void setDescription(int i, const string& newDescription) {
      if (i < numItems) {
         setDescription(names.at(i), newDescription);
      } else {
         cout << "Invalid entry index " << i << "in setDescription, sorry." << endl;
      }
   }

   string getDescription(const string& name) const {
      for (map<string,Entry*>::const_iterator it = entries.begin();
           it != entries.end(); it++) {
         if ((*it).first == name) { return ((*it).second->description); }
      }
      return "";
   }
   string getDescription(int i) const {
      return getDescription(names.at(i));
   }

   void setStatus(const string& name, const string& newStatus) { 
      entries[name]->status = IRutil::trim(newStatus); 
   } 
   void setStatus(int i, const string& newStatus) { 
      if (i < numItems) { 
         setStatus(names.at(i), newStatus); 
      } else { 
         cout << "Invalid entry index " << i << "in setStatus, sorry." << endl; 
      } 
   } 
   string getStatus(const string& name) { 
      return entries[name]->status; 
   } 
   string getStatus(int i) { 
      if (i < numItems) { 
         return getStatus(names.at(i)); 
      } else { 
         cout << "Invalid entry index " << i << "in getStatus, returning empty string..." << endl; 
         return ""; 
      } 
   } 
 
   int getFDI() const { return firstDisplayedIndex; } 
   void setPreamble(const string& pre) { preamble = pre; } 
   void setPostamble(const string& post) { postamble = post; } 
   void setScreen(bool sa) { screenAnyway = sa; } 
 
   /** Single master reader and logger of all menu and catalog choices. 
       Skips over lines beginning with the IR-comment string. 
       Admittedly a kludge, clean up by putting more responsibility on caller? 
       Allows use by another routine when other element(s) are starred.
    */ 
   virtual int readChoice(bool displayMenu = true, bool specialGo = false) { 
      int choice; 
      string st; 
      if (displayMenu && (ensp->usingCin() || screenAnyway)) { 
         showMenu(); 
      } 
      ensp->loggedReadString(st,false,false);   //suppresses writing to the log 
      st = IRutil::trim(st); 
      while (IRutil::isIRComment(st) && !ensp->atEOF()) { 
         ensp->loggedWrite(st,false); 
         ensp->loggedGetline();      //consumes trailing newline too 
         ensp->loggedWrite(st,true); //append newline back 
         if (!ensp->atEOF()) { 
            ensp->loggedReadString(st,false,true); //OK to write any whitespace 
            //(**INPP) >> st; 
            st = IRutil::trim(st); 
         } else { 
            st = ""; 
         } 
      } 
      if (st == "") {           //stream has failed and caller didn't catch it 
         return STREAM_END_CHOICE;   //should clean stream and try to continue 
      } 
      if (firstDisplayedIndex <= 0 && st == "0") {  //user might have said 0 
         ensp->loggedWrite(nameOf(0),false); 
         return 0; 
      } //else---so purposeful int i is nonzero, might be "cancel" or "more..." 
 
      int i = atoi(st.c_str());         //returns 0 if st does not parse to int 
      if (i == 0) {                     //figure user entered a name 
         ensp->loggedWrite(st,false);   //we log the name whether found or not 
 
         map<string,Entry*>::iterator it = entries.find(st); 
         if (it != entries.end()) { 
            choice = it->second->index;  //true enum index, not "+ 1" 
         } else if (st == MORE_NAME) { 
            return MORE_CHOICE; 
         } else if (st == CANCEL_NAME) { 
            return CANCEL_CHOICE; 
         } else if (specialGo && (st == "0" || st == "go" || st == "OK" || st == "ok")) {
            return -5;   //kludge: == GoMenu::GO_CHOICE;
         } else { 
            cerr << "Invalid name, returning " << INVALID_CHOICE << "." << endl; 
            return INVALID_CHOICE; 
         } 
      } else if (success(i - firstDisplayedIndex)) { 
         choice = i - firstDisplayedIndex; 
         ensp->loggedWrite(nameOf(choice),false); 
         if (i == 1 && specialGo) { return -6; } //double kludge
      } else if (i == MORE_CHOICE) { 
         ensp->loggedWrite(MORE_NAME,false);  //don't start new log line 
         choice = i; 
      } else if (i == CANCEL_CHOICE) { 
         ensp->loggedWrite(CANCEL_NAME,true); //start new line since cancel 
         choice = i; 
      } else { 
         cerr << "Invalid choice, returning " << INVALID_CHOICE << "." << endl; 
         choice = INVALID_CHOICE; 
         ensp->loggedWrite(IRutil::itoa(choice),false); //yes, log the "-1" too 
      } 
      return choice; 
   } 
}; 
 
/*
const string EnumMenu::INVALID_NAME = "INVALID_NAME";  //not logged 
const string EnumMenu::MORE_NAME = "moreOptions";      //logged 
const string EnumMenu::CANCEL_NAME = "cancelChoice";   //logged 
const string EnumMenu::STREAM_END_NAME = "STREAM_END"; //not logged 
*/
 
/** Legislates that 0 is used to mean "OK" or "go" or "submit"
   EnumMenu(Logging* enp, int gsize, int fdi = 1, bool na = false,
            bool pa = false, const string& pre = "", const string& post = "",
            bool ca = false, bool smo = false, bool sa = false)
      : ensp(enp), entries(map<string,Entry*>()), numItems(gsize),
        names(vector<string>(gsize)), firstDisplayedIndex(fdi),
        namesAfter(na), padNames(pa), preamble(pre), postamble(post),
        cancelable(ca), speakMoreOptions(smo), screenAnyway(sa),
        maxNameLength(0)

 */
class GoMenu : public EnumMenu {
 public: 
   static const int GO_CHOICE = -5;
   const string GO_NAME;

   GoMenu(Logging* enp, int gsize, int fdi = 1, bool na = false,
         bool pa = false, const string& pre = "", const string& post = "",
         bool ca = false, bool smo = false, bool sa = false)
    : EnumMenu(enp,gsize,fdi,na,pa,pre,post,ca,smo,sa), GO_NAME("go")
   {
      //setPostamble("Enter 0 or go or OK or ok to submit.");
   }

   //virtual void showMenu(int offset = 0) {
      //setPostamble("Enter 0 or go or OK or ok to submit.");
      //EnumMenu::showMenu(offset);
      //cout << "Enter " << GO_CHOICE << " or " << GO_NAME << " to submit." << endl;
   //}

   virtual int readChoice(bool displayMenu = true) {
      return EnumMenu::readChoice(displayMenu, true);  //gives "go/OK/ok" special status
   }

};

//const string GoMenu::GO_NAME = "go";
 
 
//class DynamicMenu : public EnumMenu { 
//class DynamicMenu : public GoMenu {
class DynamicMenu : public EnumMenu {
   bool alphaSorted;  //A major behavior change, so perhaps there should 
                      //really be a completely separate "AlphaMenu" class. 
                      //But many EnumMenu methods are helpful to inherit... 
 
   void resetIndices() { 
      int i = 0; 
      for (map<string,Entry*>::iterator it = entries.begin(); 
           it != entries.end(); it++) { 
         names[i] = it->first; 
         it->second->index = i; 
         i++; 
      } 
   } 
 public: 
   DynamicMenu(Logging* ensp, bool alpha = true, int firstDisplayedIndex = 1, 
               bool namesAfter = false, bool padNames = false, 
               const string& pre = "", const string& post = "", 
               bool cancelable = true, bool showMoreOptions = true, 
               bool screenAlways = false) 
      : EnumMenu(ensp, 0, firstDisplayedIndex, namesAfter && !alpha, padNames, 
                 pre, post, cancelable, showMoreOptions, screenAlways), 
        alphaSorted(alpha)   //alphaSorted forces !namesAfter 
   { } 
 
   virtual ~DynamicMenu() { } 
 
   bool empty() const { return size() == 0; } 
   void setFDI(int fdi) { firstDisplayedIndex = fdi; } 
   bool hasEntry(const string& name) const { 
      return entries.find(name) != entries.end(); 
   } 
 
   void setDescription(const string& name, const string& desc) { 
      map<string,Entry*>::const_iterator it = entries.find(name); 
      if (it != entries.end()) { 
         it->second->description = desc; 
      } else { 
         cout << "Could not find item to change its description." << endl; 
      } 
   } 
 
 
   virtual void addEntry(const string& name, const string& desc,
                         const string& status = "", 
                         bool rd = false, bool starred = false, 
                         bool novel = false) { 
      map<string,Entry*>::const_iterator it = entries.find(name); 
      if (it != entries.end()) { 
         cout << "Overwriting duplicate entry " << name << " in " 
              << entries.size() << " entries, hope that's OK." << endl; 
         EnumMenu::addEntry(it->second->index, name, desc, status, 
                            rd, starred, novel); 
      } else { 
         EnumMenu::addEntry(name, desc, status, rd, starred, novel); 
         if (alphaSorted) { resetIndices(); }  //not needed for index append 
      } 
   } 
 
   virtual void removeEntry(const string& name) { 
      map<string,Entry*>::iterator it = entries.find(name); 
      if (it != entries.end()) { 
         int i = it->second->index; 
         entries.erase(it); 
         names.erase(names.begin() + i); 
         numItems--; 
         resetIndices();  //regardless of alpha-sort. 
      } else { 
         cerr << "Item not found."; 
      } 
   } 
}; 
 
 
/** Catalog class uses composition, not inheritance, with DynamicMenu. 
    Mandates alpha sort.  Intended for further derivation. 
    Use of a pointer makes the linkage "weaker" thus facilitating separate 
    compilation, but allows "const" where logically perhaps it shouldn't be. 
 */ 
class Catalog { 
 public: 
   DynamicMenu* info;     //pointer consistent with Logging, enables const 
 private: 
   Catalog (const Catalog& rhs);   //disable copying for all descendants too 
   Catalog& operator=(const Catalog& rhs); 
 
 public: 
   explicit Catalog(Logging* ensp, int fdi = 1,  //alpha-sorted, index from 1 
           string pre = "", const string& post = "", bool cancelable = true, 
           bool showMoreOptions = false, bool screenAlways = false) 
      : info(new DynamicMenu(ensp, true, fdi, false, false, pre, post, 
             cancelable, showMoreOptions, screenAlways)) 
   { } 
 
   virtual ~Catalog() { } 
 
   //Delegates---what's missing here are adding and removing. 
 
   bool empty() const { return info->empty(); } 
   int size() const { return info->size(); } 
   bool success(int i) const { return info->success(i); } 
   string nameOf(int index) const { return info->nameOf(index); } 
   void setFDI(int fdi) { info->setFDI(fdi); } 
   int getFDI() const { return info->getFDI(); } 
   void setPreamble(const string& pre) const { info->setPreamble(pre); } 
   void setPostamble(const string& post) const { info->setPostamble(post); } 
   void setScreen(bool sa) const { info->setScreen(sa); } 
   void showCatalog(int offset = 0) const { info->showMenu(offset); } 
   bool hasEntry(const string& name) const { return info->hasEntry(name); } 
   bool isStarred(int i) { return info->isStarred(i); } 
   bool isStarred(const string& name) { return info->isStarred(name); } 
   void setStarred(const string& name, bool starred) const { 
       info->setStarred(name,starred);   //unchecked! 
   } 
   void clearStars() const { info->clearStars(); } 
   void setStatus(const string& name, const string& newStatus) const { 
      info->setStatus(name,newStatus); 
   } 
   string getStatus(const string& name) const { return info->getStatus(name); } 
   void setDescription(const string& name, const string& desc) { 
      info->setDescription(name,desc); 
   } 
   void resetDefaults() const { info->resetDefaults(); } 
   int readChoice(bool displayMenu = true, bool specialGo = false) { 
      return info->readChoice(displayMenu, specialGo); 
   } 
 
   /** IMPT: Since Catalog is *not* intended to be used with an enum, 
       integer returns are meaningless, so this method returns the name. 
       This also is consistent with the unique-names policy.  Re-using the 
       EnumMenu::readChoice() method requires dealing with the int values 
       it returns, which looks kludgey since they get converted back to names, 
       but when users take the int-entry option no extra work is done. 
    */ 
   string readChoiceName() { 
      int i = info->readChoice(); 
      if (i >= 0) {                   //Zero-basedness of returns is hard-coded 
         return info->nameOf(i);      //Range-checking is done by readChoice() 
      } else if (i == EnumMenu::MORE_CHOICE) { 
         return info->MORE_NAME;  //EnumMenu::MORE_NAME;  //Forward special codes to caller. 
      } else if (i == EnumMenu::CANCEL_CHOICE) { 
         return info->CANCEL_NAME; //EnumMenu::CANCEL_NAME; 
      } else if (i == EnumMenu::STREAM_END_CHOICE) { 
         return info->STREAM_END_NAME; //EnumMenu::STREAM_END_NAME; 
      } else { 
         return info->INVALID_NAME;  //EnumMenu::INVALID_NAME; 
      } 
   } 
}; 
 
 
template <class E>  //E extends Catalogable 
class ValueCatalog : public Catalog { 
 
   map<string,E> items;   //CLASS INV: has same size as entries in "info" 
 
 public: 
   explicit ValueCatalog<E>(Logging* ensp, int fdi = 1, 
                 string pre = "", const string& post = "", bool cancelable = true, 
                 bool showMoreOptions = false, bool screenAlways = false) 
    : Catalog(ensp, fdi, pre, post, cancelable, showMoreOptions, screenAlways), 
      items(map<string,E>()) 
   { } 
   virtual ~ValueCatalog<E>() { } 
 
   E& operator[] (const string& name) { return items[name]; } 
 
   void addItem(const E& item, const string& status = "", 
                bool resetDefault = false, bool starred = false) { 
      string name = item.getName();   //via Catalogable 
      info->addEntry(name, item.getDescription(), status, 
                     resetDefault, starred, false); 
                            //catalog items are never "novel" 
      items[name] = item;   //INV: "map" and "addEntry" always agree. 
   } 
 
   void removeItem(const string& name) { 
      //map<string,E>::iterator ite = items.find(name);  //illegal!? 
      if (items.find(name) != items.end()) { 
         items.erase(items.find(name)); 
         info->removeEntry(name);   //again, semantics agrees here. 
      } else { 
         cout << "Item not found---doing nothing." << endl; 
      } 
   }  //caller has responsibility to manage the stars. 
}; 
 
 
template <class E>  //E extends Catalogable 
class RefCatalog : public Catalog { 
 
   map<string,E*> items;   //CLASS INV: has same size as entries in "info" 
 
 public: 
   explicit RefCatalog<E>(Logging* ensp, int fdi = 1, 
                 string pre = "", const string& post = "", bool cancelable = true, 
                 bool showMoreOptions = false, bool screenAlways = false) 
    : Catalog(ensp, fdi, pre, post, cancelable, showMoreOptions, screenAlways), 
      items(map<string,E*>()) 
   { } 
   virtual ~RefCatalog<E>() { } 
 
   E* operator[] (const string& name) { return items[name]; } 
 
   void addItem(E* item, const string& status = "", 
                bool resetDefault = false, bool starred = false) { 
      string name = item->getName();   //via Catalogable 
      info->addEntry(name, item->getDescription(), status, 
                     resetDefault, starred, false); 
                            //catalog items are never "novel" 
      items[name] = item;   //INV: "map" and "addEntry" always agree. 
   } 
 
   void removeItem(const string& name) { 
      //map<string,E*>::iterator ite = items.find(name);  //illegal!? 
      //previous code was like for ValueCatalog 
      if (items.find(name) != items.end()) { 
         E* foundItem = (*(items.find(name))).second;  // = items[name]; 
         //delete(foundItem); 
         items.erase(name); 
         info->removeEntry(name);   //again, semantics agrees here. 
      } else { 
         cout << "Item not found---doing nothing." << endl; 
      } 
   } 
}; 
 
 
#endif    //end of #ifndef __IR_MENUS_H__ 
 
 
 
