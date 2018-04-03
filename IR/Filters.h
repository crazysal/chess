//File-----------------------Filters.h---------------------------------------- 
 
#ifndef __IR_FILTERS_H__ 
#define __IR_FILTERS_H__ 
 
//-----------------------FILTERS------------------------------------------ 
 
typedef bool (*TurnFilterFn)(const TurnInfo* const); 
 
#ifndef __IR_MOVESELECTION_H__
#include "MoveSelector.h"
//enum COMPARE { LEQ, EQ, GEQ };  //NOT < = >, but with <= and >= 
#endif

class TurnFilter : public Catalogable {
 protected:        //C++: accessible to *immediate* subclasses only
   string name;
   TurnFilter(const string& gname) : name(gname) { }
   TurnFilter() : name("anon") { }
 public:
   virtual ~TurnFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const = 0;

   virtual string getName() const { return name; }
   virtual string getDescription() const = 0;
   virtual operator string() const {  //works for many subclasses
      return getName() + ": " + getDescription();
   }
};

inline bool allowAll(const TurnInfo* const pTurn) { return true; } 
 
class FnFilter : public TurnFilter { 
   TurnFilterFn filterFn; 
 public: 
   FnFilter(TurnFilterFn gfilter, const string& name) 
      : TurnFilter(name), filterFn(gfilter) { } 
   virtual ~FnFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return (*filterFn)(pTurn); 
   } 
   virtual string getDescription() const { 
      return "FnFilter"; 
   } 
}; 
 
class NotFilter : public TurnFilter { 
   TurnFilter* theFilter; 
 public: 
   NotFilter(TurnFilter* filter, const string& name) 
      : TurnFilter(name), theFilter(filter) { } 
   virtual ~NotFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return !(theFilter->apply(pTurn)); 
   } 
   virtual string getDescription() const { 
      return "NotFilter [" + theFilter->getName() + "]"; 
   } 
}; 
 
 
class AndFilter : public TurnFilter { 
   list<TurnFilter*> conjuncts; 
 public: 
   AndFilter(const string& name) 
      : TurnFilter(name), conjuncts(list<TurnFilter*>()) { } 
   AndFilter(TurnFilter* lhs, TurnFilter* rhs, const string& name) 
      : TurnFilter(name), conjuncts(list<TurnFilter*>()) 
   { 
      conjuncts.push_back(lhs); 
      conjuncts.push_back(rhs); 
   } 
   virtual ~AndFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      bool ans = true; 
      for (list<TurnFilter*>::const_iterator it = conjuncts.begin(); 
           it != conjuncts.end(); it++) { 
         if (!(*it)->apply(pTurn)) { 
            ans = false; 
            break; 
         } 
      } 
      return ans; 
   } 
   void addConjunct(TurnFilter* tf) { 
      conjuncts.push_back(tf); 
   } 
 
   virtual string getDescription() const { 
      string out = "AndFilter ["; 
      for (list<TurnFilter*>::const_iterator it = conjuncts.begin(); 
           it != conjuncts.end(); it++) { 
         out += (*it)->getName() + ","; 
      } 
      if (conjuncts.empty()) { 
         out += "]"; 
      } else { 
         out[out.size() - 1] = ']'; 
      } 
      return out; 
   } 
 
   virtual operator string() const { 
      string out = getName() + ": AndFilter ["; 
      for (list<TurnFilter*>::const_iterator it = conjuncts.begin(); 
           it != conjuncts.end(); it++) { 
         out += string(**it) + ", ";    //string() makes this more verbose 
      } 
      if (conjuncts.empty()) { 
         out += "]"; 
      } else { 
         out[out.size() - 2] = ']'; 
      } 
      return out; 
   } 
}; 
 
 
class OrFilter : public TurnFilter { 
   list<TurnFilter*> disjuncts; 
 public: 
   OrFilter(const string& name) 
      : TurnFilter(name), disjuncts(list<TurnFilter*>()) { } 
   OrFilter(TurnFilter* lhs, TurnFilter* rhs, const string& name) 
      : TurnFilter(name), disjuncts(list<TurnFilter*>()) 
   { 
      disjuncts.push_back(lhs); 
      disjuncts.push_back(rhs); 
   } 
   virtual ~OrFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      bool ans = false; 
      for (list<TurnFilter*>::const_iterator it = disjuncts.begin(); 
           it != disjuncts.end(); it++) { 
         if ((*it)->apply(pTurn)) { 
            ans = true; 
            break; 
         } 
      } 
      return ans; 
   } 
   void addDisjunct(TurnFilter* tf) { 
      disjuncts.push_back(tf); 
   } 
 
   virtual string getDescription() const { 
      string out = "OrFilter ["; 
      for (list<TurnFilter*>::const_iterator it = disjuncts.begin(); 
           it != disjuncts.end(); it++) { 
         out += (*it)->getName() + ","; 
      } 
      if (disjuncts.empty()) { 
         out += "]"; 
      } else { 
         out[out.size() - 1] = ']'; 
      } 
      return out; 
   } 
 
   virtual operator string() const { 
      string out = getName() + ": OrFilter ["; 
      for (list<TurnFilter*>::const_iterator it = disjuncts.begin(); 
           it != disjuncts.end(); it++) { 
         //out += string(**it) + ",";    //string() makes this more verbose 
         out += (**it).getDescription() + ", " ; 
      } 
      if (disjuncts.empty()) { 
         out += "]"; 
      } else { 
         out[out.size() - 2] = ']'; 
      } 
      return out; 
   } 
 
}; 
 
class WhiteToMoveFilter : public TurnFilter { 
 public: 
   WhiteToMoveFilter() : TurnFilter("WhiteToMove") { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return pTurn->whiteToMove; 
   } 
   virtual string getDescription() const { 
      return "WhiteToMoveFilter"; 
   } 
}; 
 
class BlackToMoveFilter : public TurnFilter { 
 public: 
   BlackToMoveFilter() : TurnFilter("BlackToMove") { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return !pTurn->whiteToMove; 
   } 
   virtual string getDescription() const { 
      return "BlackToMoveFilter"; 
   } 
}; 

class InCheckFilter : public TurnFilter {
 public:
   InCheckFilter() : TurnFilter("InCheck") { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      Position pos(pTurn->getFEN());
      return pos.inCheck();
   }
   virtual string getDescription() const {
      return "InCheckFilter";
   }
};

 
 
class PlayerFilter : public TurnFilter { 
 protected: 
   const string player; 
 public: 
   PlayerFilter(const string& gplayer, const string& name) 
      : TurnFilter(name), player(gplayer) { } 
   virtual ~PlayerFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
	   return IRutil::delimitedMatch(player,pTurn->pGameInfo->whitePlayer) 
                 || IRutil::delimitedMatch(player,pTurn->pGameInfo->blackPlayer); 
   } 
   virtual string getDescription() const { 
      return "PlayerFilter " + player; 
   } 
}; 
 
class PlayerOrFilter : public TurnFilter { 
 protected: 
   list<string> players; 
 public: 
   PlayerOrFilter(const string& player1, const string& name) 
      : TurnFilter(name), players(list<string>()) 
   { 
      players.push_back(player1); 
   } 
   PlayerOrFilter(const string& name) : TurnFilter(name), players(list<string>()) { } 
   virtual ~PlayerOrFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      bool ans = false; 
      for (list<string>::const_iterator it = players.begin(); 
           it != players.end(); it++) { 
         if (IRutil::delimitedMatch(*it, pTurn->pGameInfo->whitePlayer) 
             || IRutil::delimitedMatch(*it, pTurn->pGameInfo->blackPlayer)) { 
            ans = true; 
            break; 
         } 
      } 
      return ans; 
   } 
   void addPlayer(const string& newPlayer) { 
      players.push_back(newPlayer); 
   } 
   virtual string getDescription() const { 
      string out = "PlayerOrFilter ["; 
      for (list<string>::const_iterator it = players.begin(); 
           it != players.end(); it++) { out += (*it) + ","; } 
      out[out.size() - 1] = ']'; 
      return out; 
   } 
 
}; 
 
 
 
class PlayerOrToMoveFilter : public PlayerOrFilter { 
 public: 
   PlayerOrToMoveFilter(const string& player1, const string& name) 
      : PlayerOrFilter(player1,name) { } 
   PlayerOrToMoveFilter(const string& name) 
      : PlayerOrFilter(name) { } 
   virtual ~PlayerOrToMoveFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      bool ans = false; 
      for (list<string>::const_iterator it = players.begin(); 
           it != players.end(); it++) { 
         if ((IRutil::delimitedMatch(*it, pTurn->pGameInfo->whitePlayer)  
                 && pTurn->whiteToMove) 
         || (IRutil::delimitedMatch(*it, pTurn->pGameInfo->blackPlayer)  
                 && !pTurn->whiteToMove)) { 
            ans = true; 
            break; 
         } 
      } 
      return ans; 
   } 
   virtual string getDescription() const { 
      string out = "PlayerOrToMoveFilter ["; 
      for (list<string>::const_iterator it = players.begin(); 
           it != players.end(); it++) { out += (*it) + ","; } 
      out[out.size() - 1] = ']'; 
      return out; 
   } 
}; 
 
class PlayerToMoveFilter : public PlayerFilter { 
 public: 
   PlayerToMoveFilter(const string& player, const string& name) 
      : PlayerFilter(player,name) { } 
   virtual ~PlayerToMoveFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      bool ans = (pTurn->whiteToMove  
                        && IRutil::delimitedMatch(player,pTurn->pGameInfo->whitePlayer)) 
                  || ((!pTurn->whiteToMove)  
                        && IRutil::delimitedMatch(player,pTurn->pGameInfo->blackPlayer)); 
      return ans; 
   } 
   virtual string getDescription() const { 
      return PlayerFilter::getDescription() + ", to move"; 
   } 
}; 
 
class OpponentToMoveFilter : public PlayerFilter { 
 public: 
   OpponentToMoveFilter(const string& player, const string& name) 
      : PlayerFilter(player,name) { } 
   virtual ~OpponentToMoveFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      bool ans = (pTurn->whiteToMove  
                        && IRutil::delimitedMatch(player,pTurn->pGameInfo->blackPlayer)) 
                  || ((!pTurn->whiteToMove)  
                        && IRutil::delimitedMatch(player,pTurn->pGameInfo->whitePlayer)); 
      return ans; 
   } 
   virtual string getDescription() const { 
      return PlayerFilter::getDescription() + ", opponents' moves only"; 
   } 
}; 
 
class OpponentOrToMoveFilter : public PlayerOrFilter { 
 public: 
   OpponentOrToMoveFilter(const string& player1, const string& name) 
      : PlayerOrFilter(player1,name) { } 
   OpponentOrToMoveFilter(const string& name) 
      : PlayerOrFilter(name) { } 
   virtual ~OpponentOrToMoveFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      bool ans = false; 
      for (list<string>::const_iterator it = players.begin(); 
           it != players.end(); it++) { 
         if ((IRutil::delimitedMatch(*it, pTurn->pGameInfo->whitePlayer)  
                    && !pTurn->whiteToMove) 
              || (IRutil::delimitedMatch(*it, pTurn->pGameInfo->blackPlayer)  
                    && pTurn->whiteToMove)) { 
            ans = true; 
            break; 
         } 
      } 
      return ans; 
   } 
   virtual string getDescription() const { 
      string out = "PlayerOpponentOrToMoveFilter ["; 
      for (list<string>::const_iterator it = players.begin(); 
           it != players.end(); it++) { out += (*it) + ","; } 
      out[out.size() - 1] = ']'; 
      return out; 
   } 
}; 
 
 
 
class PlayerWhiteFilter : public PlayerFilter { 
 public: 
   PlayerWhiteFilter(const string& player, const string& name) 
      : PlayerFilter(player,name) { } 
   virtual ~PlayerWhiteFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return IRutil::delimitedMatch(player,pTurn->pGameInfo->whitePlayer); 
   } 
   virtual string getDescription() const { 
      return PlayerFilter::getDescription() + ", as White only"; 
   } 
}; 
 
class PlayerBlackFilter : public PlayerFilter { 
 public: 
   PlayerBlackFilter(const string& player, const string& name) 
      : PlayerFilter(player,name) { } 
   virtual ~PlayerBlackFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return IRutil::delimitedMatch(player,pTurn->pGameInfo->blackPlayer); 
   } 
   virtual string getDescription() const { 
      return PlayerFilter::getDescription() + ", as Black only"; 
   } 
}; 


class DrawnGameFilter : public TurnFilter {
 public:
   DrawnGameFilter() : TurnFilter("DrawnGame") { }
   virtual ~DrawnGameFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return IRutil::delimitedMatch("1/2", pTurn->pGameInfo->result);
   }
   virtual string getDescription() const {
      return "DrawnGameFilter";
   }
};


class WhiteWinFilter : public TurnFilter {
 public:
   WhiteWinFilter() : TurnFilter("WhiteWin") { }
   virtual ~WhiteWinFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return IRutil::delimitedMatch("1-0", pTurn->pGameInfo->result);
   }
   virtual string getDescription() const {
      return "WhiteWinFilter";
   }
};


class BlackWinFilter : public TurnFilter {
 public:
   BlackWinFilter() : TurnFilter("BlackWin") { }
   virtual ~BlackWinFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return IRutil::delimitedMatch("0-1", pTurn->pGameInfo->result);
   }
   virtual string getDescription() const {
      return "BlackWinFilter";
   }
};


class PlayerWinFilter : public TurnFilter {
 public:
   PlayerWinFilter() : TurnFilter("PlayerWin") { }
   virtual ~PlayerWinFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return (pTurn->whiteToMove && IRutil::delimitedMatch("1-0", pTurn->pGameInfo->result))
          || ((!(pTurn->whiteToMove)) && IRutil::delimitedMatch("0-1", pTurn->pGameInfo->result));
   }
   virtual string getDescription() const {
      return "PlayerWinFilter";
   }
};


class PlayerLossFilter : public TurnFilter {
 public:
   PlayerLossFilter() : TurnFilter("PlayerLoss") { }
   virtual ~PlayerLossFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return (pTurn->whiteToMove && IRutil::delimitedMatch("0-1", pTurn->pGameInfo->result))
          || ((!(pTurn->whiteToMove)) && IRutil::delimitedMatch("1-0", pTurn->pGameInfo->result));
   }
   virtual string getDescription() const {
      return "PlayerLossFilter";
   }
};



 
/** "Q&D" versions of event and date filters use simple delimited substring 
    match, rather than match events to a table or parse into a Date class. 
    Thus one needs to reference Arena's event names from the data files. 
 */ 
class EventFilter : public TurnFilter { 
   const string event; 
 public: 
   EventFilter(const string& ev, const string& name) 
      : TurnFilter(name), event(ev) { } 
   virtual ~EventFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
	   return IRutil::delimitedMatch(event,pTurn->pGameInfo->eventName); 
   } 
   virtual string getDescription() const { 
      return "EventFilter \"" + event + "\""; 
   } 
}; 


/** Intended to match et = "tourn" or "swiss" or "team" or "rapid" or "blitz" 
 *  or "schev" or "ko" or "simul" etc.  Use AndFilter for multiple matches to
 *  things like "swiss(rapid)" and "tourn(team)" etc.
 */
class EventTypeFilter : public TurnFilter {
   const string eventType;
 public:
   EventTypeFilter(const string& et, const string& name)
      : TurnFilter(name), eventType(et) { }
   virtual ~EventTypeFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
           return IRutil::delimitedMatch(eventType,pTurn->pGameInfo->eventType);
   }
   virtual string getDescription() const {
      return "EventTypeFilter \"" + eventType + "\"";
   }
};

/** Many team-event games are marked only swiss() or etc. in ChessBase Big discs etc.
    Hence hunt for two other common telltales of a team event.
 */
class TeamEventFilter : public TurnFilter {
 public:
   TeamEventFilter() : TurnFilter("TeamEvent") { }
   virtual ~TeamEventFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      bool try1 = IRutil::delimitedMatch("team",pTurn->pGameInfo->eventType);
      bool try2 = IRutil::delimitedMatch("chT",pTurn->pGameInfo->eventName);
      bool try3 = IRutil::delimitedMatch("ch-T",pTurn->pGameInfo->eventName);
      bool try4 = IRutil::delimitedMatch("Tch",pTurn->pGameInfo->eventName);
      bool try5 = IRutil::delimitedMatch("T-ch",pTurn->pGameInfo->eventName);
      bool try6 = IRutil::isFilled(pTurn->pGameInfo->whiteTeam);
      return (try1 || try2 || try3 || try4 || try5 || try6);
   }
   virtual string getDescription() const {
      return "TeamEventFilter team/chT/ch-T/T-ch/Tch/WhiteTeam";
   }
};


class EngineFilter : public TurnFilter {
   const string engineNameMatch;
   const string version;  //match as string not number, assume comes after name
 public:
   EngineFilter(const string& enm, const string& ver, const string& name)
      : TurnFilter(name), engineNameMatch(enm), version(ver) { }
   virtual ~EngineFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      string eid = pTurn->engineID;
      string engID = pTurn->pGameInfo->engineID;
      size_t i = eid.find(engineNameMatch); //not delimited to allow "omod" e.g.
      size_t j = engID.find(engineNameMatch);
      if (i == string::npos && j == string::npos) { return false; }
      size_t len = engineNameMatch.size();
      string eidRest = eid.substr(i + len);
      string engIDRest = engID.substr(j + len);
      return (IRutil::delimitedMatch(version,eidRest)  //allows "9.4" to match "9.4.2"
               || IRutil::delimitedMatch(version,engIDRest));
   }
   virtual string getDescription() const {
      return "EngineFilter \"" + engineNameMatch + "\", \"" + version + "\"";
   }
};


 
 
/** General comparison class, maybe should have separate int analogue 
 */ 
class CompareFilter : public virtual TurnFilter { 
   const double rhs; 
   const COMPARE comparison; 
 protected: 
   CompareFilter(COMPARE gc, double r, const string& gname) 
         : TurnFilter(gname), comparison(gc), rhs(r) { 
      name = gname; 
   } 
 
 public: 
   virtual ~CompareFilter() { } 
 
   bool test(double lhs) const { 
      bool ans; 
      switch(comparison) { 
       case LEQ: 
         ans = lhs <= rhs; 
         break; 
       case EQ: 
         ans = lhs == rhs; 
         break; 
       case GEQ: 
         ans = lhs >= rhs; 
         break; 
      } 
      return ans; 
   } 
 
   virtual bool apply(const TurnInfo* const pTurn) const = 0; 
 
   string relation() const { 
      string rel = ""; 
      switch(comparison) { 
       case LEQ: 
         rel = "<="; 
         break; 
       case EQ: 
         rel = "=="; 
         break; 
       case GEQ: 
         rel = ">="; 
         break; 
      } 
      return rel; 
   } 
 
   virtual string getDescription() const { 
      return relation() + " " + IRutil::ftoa(rhs); 
   } 
}; 
 

class DateFilter : public CompareFilter {  //stores mangled date, not date 
   IRutil::SimpleDate date; //not used to compare, instead int passed to parent 
 public: 
   DateFilter(const COMPARE comp, const IRutil::SimpleDate gdate, const string& name) 
      : CompareFilter(comp, gdate.mangledDate(), name), date(gdate) 
   { } 
   virtual ~DateFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return test(pTurn->pGameInfo->date.mangledDate()); 
   } 
   virtual string getDescription() const { 
      return "Date is " + relation() + " " + string(date); 
   } 
}; 

class EventDateFilter : public CompareFilter {  //stores mangled date, not date
   IRutil::SimpleDate date; //not used to compare, instead int passed to parent
 public:
   EventDateFilter(const COMPARE comp, const IRutil::SimpleDate gdate, const string& name)
      : CompareFilter(comp, gdate.mangledDate(), name), date(gdate)
   { }
   virtual ~EventDateFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return test(pTurn->pGameInfo->eventDate.mangledDate());
   }
   virtual string getDescription() const {
      return "Event date is " + relation() + " " + string(date);
   }
};

 
class YearFilter : public CompareFilter {  //stores mangled date, not date 
   int year; //not used to compare, instead int passed to parent 
 public: 
   YearFilter(const COMPARE comp, const int gyear, const string& name) 
      : CompareFilter(comp, gyear, name), year(gyear) 
   { } 
   virtual ~YearFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return test(pTurn->pGameInfo->date.getYear()); 
   } 
   virtual string getDescription() const { 
      return "Year is " + relation() + " " + IRutil::itoa(year); 
   } 
}; 
 

class WhiteEloFilter : public CompareFilter {  
   int elo; //not used to compare, instead int passed to parent
 public:
   WhiteEloFilter(const COMPARE comp, const int gelo, const string& name)
      : CompareFilter(comp, gelo, name), elo(gelo)
   { }
   virtual ~WhiteEloFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      int whiteElo = pTurn->pGameInfo->whiteElo;
      return whiteElo > 0 && test(whiteElo);
   }
   virtual string getDescription() const {
      return "White Elo is " + relation() + " " + IRutil::itoa(elo);
   }
};


class BlackEloFilter : public CompareFilter {
   int elo; //not used to compare, instead int passed to parent
 public:
   BlackEloFilter(const COMPARE comp, const int gelo, const string& name)
      : CompareFilter(comp, gelo, name), elo(gelo)
   { }
   virtual ~BlackEloFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      int blackElo = pTurn->pGameInfo->blackElo;
      return blackElo > 0 && test(blackElo);
   }
   virtual string getDescription() const {
      return "Black Elo is " + relation() + " " + IRutil::itoa(elo);
   }
};


class PlayerEloFilter : public CompareFilter {
   int elo; //not used to compare, instead int passed to parent
 public:
   PlayerEloFilter(const COMPARE comp, const int gelo, const string& name)
      : CompareFilter(comp, gelo, name), elo(gelo)
   { }
   virtual ~PlayerEloFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      int playerElo = pTurn->whiteToMove ? pTurn->pGameInfo->whiteElo 
                                         : pTurn->pGameInfo->blackElo;
      return (playerElo > 0 && test(playerElo));
   }
   virtual string getDescription() const {
      return "Elo of player to move is " + relation() + " " + IRutil::itoa(elo);
   }
};


class EloDiffFilter : public CompareFilter {
   int elodiff;
 public:
   EloDiffFilter(const COMPARE comp, const int gdiff, const string& name)
      : CompareFilter(comp, gdiff, name), elodiff(gdiff)
   { }
   virtual ~EloDiffFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      int playerElo = pTurn->whiteToMove ? pTurn->pGameInfo->whiteElo
                                         : pTurn->pGameInfo->blackElo;
      int oppElo = pTurn->whiteToMove ? pTurn->pGameInfo->blackElo
                                      : pTurn->pGameInfo->whiteElo;
      return (playerElo > 0 && oppElo > 0 && test(playerElo - oppElo));
   }
   virtual string getDescription() const {
      return "Elo diff is " + relation() + " " + IRutil::itoa(elodiff);
   }
};



class EloDiffWithinFilter : public CompareFilter {
   int elodiffbound;
 public:
   EloDiffWithinFilter(const COMPARE comp, const int gbound, const string& name)
      : CompareFilter(comp, gbound, name), elodiffbound(gbound)
   { }
   virtual ~EloDiffWithinFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      int playerElo = pTurn->whiteToMove ? pTurn->pGameInfo->whiteElo
                                         : pTurn->pGameInfo->blackElo;
      int oppElo = pTurn->whiteToMove ? pTurn->pGameInfo->blackElo
                                      : pTurn->pGameInfo->whiteElo;
      return (playerElo > 0 && oppElo > 0 && test(abs(playerElo - oppElo)));
   }
   virtual string getDescription() const {
      return "|Elo diff| is " + relation() + " " + IRutil::itoa(elodiffbound);
   }
};



class EvalWhiteFilter : public CompareFilter { 
 public: 
   EvalWhiteFilter(const COMPARE comp, const double threshold, const string& name) 
         : CompareFilter(comp, threshold, name) { } 
   virtual ~EvalWhiteFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
	   return test(pTurn->currEvalCP/double(100)); 
   } 
   virtual string getDescription() const { 
      return "White's eval " + CompareFilter::getDescription(); 
   } 
}; 
 
class PrevEvalWhiteFilter : public CompareFilter { 
 public: 
   PrevEvalWhiteFilter(const COMPARE comp, const double threshold, const string& name) 
      : CompareFilter(comp, threshold, name) { } 
   virtual ~PrevEvalWhiteFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return (pTurn->prevEvalCP != IRutil::NAVAL && test(pTurn->prevEvalCP/double(100))); 
   } 
   virtual string getDescription() const { 
      return "Prev White eval " + CompareFilter::getDescription(); 
   } 
}; 
 
 
class NextEvalWhiteFilter : public CompareFilter {
 public:
   NextEvalWhiteFilter(const COMPARE comp, const double threshold, const string& name)
      : CompareFilter(comp, threshold, name) { }
   virtual ~NextEvalWhiteFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return (pTurn->nextEvalCP != IRutil::NAVAL && test(pTurn->nextEvalCP/double(100)));
   }
   virtual string getDescription() const {
      return "Next White eval " + CompareFilter::getDescription();
   }
};

class EvalWithinFilter : public CompareFilter { 
   size_t atDepth;
   map<string,double> normFactors;
 public: 
   EvalWithinFilter(const double threshold, const string& name, const size_t d = 0,
                    map<string,double> nf = map<string,double>()) 
         : CompareFilter(LEQ, threshold, name), atDepth(d), normFactors(nf) { } 
   virtual ~EvalWithinFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      int evalToTest = (atDepth == 0 ? pTurn->currEvalCP : pTurn->getBestEvalCPAtDepth(atDepth));
      string eid = pTurn->engineID;
      double factor = 1.0;
      map<string,double>::const_iterator itr = normFactors.begin();
      map<string,double>::const_iterator itre = normFactors.end();
      while (itr != itre) {
         string str = (*itr).first;
         if (eid.find(str) != string::npos) { factor = (*itr).second; }
         itr++;
      }
      return test(fabs(evalToTest*factor/100.0)); 
   } 
   virtual string getDescription() const { 
      string ds = (atDepth == 0 ? "" : string("at depth ") + IRutil::ftoa(atDepth) + string(" "));
      return "Turn |eval| " + ds + CompareFilter::getDescription(); 
   } 
}; 
 
 
class PrevEvalWithinFilter : public CompareFilter {   //count NAVAL as true
   size_t atDepth;
   map<string,double> normFactors;
 public: 
   PrevEvalWithinFilter(const double threshold, const string& name, const size_t d = 0,
                        map<string,double> nf = map<string,double>())
      : CompareFilter(LEQ, threshold, name), atDepth(d), normFactors(nf) { } 
   virtual ~PrevEvalWithinFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      TurnInfo* prevTurn = pTurn->prevTurn;
      if (prevTurn == NULL) { return true; }  //else
      double evalToTest = (atDepth == 0 ? prevTurn->currEvalCP : prevTurn->getBestEvalCPAtDepth(atDepth));
      string eid = pTurn->engineID;
      double factor = 1.0;
      map<string,double>::const_iterator itr = normFactors.begin();
      map<string,double>::const_iterator itre = normFactors.end();
      while (itr != itre) {
         string str = (*itr).first;
         if (eid.find(str) != string::npos) { factor = (*itr).second; }
         itr++;
      }
      return test(fabs(evalToTest*factor/100.0));
   } 
   virtual string getDescription() const { 
      string ds = (atDepth == 0 ? "" : string("at depth ") + IRutil::ftoa(atDepth) + string(" "));
      return "Prev turn |eval| " + ds + CompareFilter::getDescription(); 
   } 
}; 


class NextEvalWithinFilter : public CompareFilter {  //count end of game like current eval
   size_t atDepth;
   map<string,double> normFactors;
 public:
   NextEvalWithinFilter(const double threshold, const string& name, const size_t d = 0,
                        map<string,double> nf = map<string,double>())
      : CompareFilter(LEQ, threshold, name), atDepth(d), normFactors(nf) { }

   virtual ~NextEvalWithinFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      const TurnInfo* nextTurn = pTurn->prevTurn;
      if (nextTurn == NULL) { nextTurn = pTurn; }  //else
      double evalToTest = (atDepth == 0 ? nextTurn->currEvalCP : nextTurn->getBestEvalCPAtDepth(atDepth));
      string eid = pTurn->engineID;
      double factor = 1.0;
      map<string,double>::const_iterator itr = normFactors.begin();
      map<string,double>::const_iterator itre = normFactors.end();
      while (itr != itre) {
         string str = (*itr).first;
         if (eid.find(str) != string::npos) { factor = (*itr).second; }
         itr++;
      }
      return test(fabs(evalToTest*factor/100.0));
   }
   virtual string getDescription() const {
      string ds = (atDepth == 0 ? "" : string("at depth ") + IRutil::ftoa(atDepth) + string(" "));
      return "Next turn |eval| " + ds + CompareFilter::getDescription();
   }
};

 
 
class TurnNoFilter : public CompareFilter { 
 public: 
   TurnNoFilter(const COMPARE comp, const int fromTurn, const string& name) 
      : CompareFilter(comp, fromTurn, name) { } 
   virtual ~TurnNoFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return test(pTurn->turnNumber); 
   } 
   virtual string getDescription() const { 
      return "TurnNumber " + CompareFilter::getDescription(); 
   } 
}; 
 
 
class NumLegalMovesFilter : public CompareFilter { 
 public: 
   NumLegalMovesFilter(const COMPARE comp, const int numLegalMoves, const string& name) 
      : CompareFilter(comp, numLegalMoves, name) { } 
   virtual ~NumLegalMovesFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
	   return test(pTurn->numLegalMoves); 
   } 
   virtual string getDescription() const { 
      return "# legal moves " + CompareFilter::getDescription(); 
   } 
}; 


class PlycountFilter : public CompareFilter {
 public:
   PlycountFilter(const COMPARE comp, const int plyCount, const string& name)
      : CompareFilter(comp, plyCount, name) { }
   virtual ~PlycountFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
           return pTurn->pGameInfo->plyCount > 0 && test(pTurn->pGameInfo->plyCount);
   }
   virtual string getDescription() const {
      return "# game ply " + CompareFilter::getDescription();
   }
};

 
 
//ASSUMES TurnInfo moves are ordered by eval at highest depth 
class DeltaIFilter : public CompareFilter { 
   const int index; 
 public: 
   DeltaIFilter(const COMPARE comp, const double threshold, const int gindex, 
                   string name) 
      : CompareFilter(comp, threshold, name), index(gindex) { } 
   virtual ~DeltaIFilter() { } 
   virtual bool apply(const TurnInfo*  const pTurn) const { 
      if (index >= pTurn->depthEvalsCP->size()) {
		   //TAMAL:TODO: verify
		   return false;
      }
      //const double indexedEval =  //highDepth always given valid data
            //pTurn->depthEvalsCP->at(index).at(pTurn->turnHighestDepth-1)/double(100); 
      //return test(IRutil::round(pTurn->currEvalCP/double(100) - indexedEval, 2)); 
      const int indexedEvalCP = pTurn->depthEvalsCP->at(index).at(pTurn->turnHighestDepth-1);
      return test(fabs(pTurn->currEvalCP - indexedEvalCP)/100.0);
   } 
   virtual string getDescription() const { 
      return "Delta_" + IRutil::itoa(index) + " "  
                      + CompareFilter::getDescription(); 
   } 
}; 
 
class DeltaDiffFilter : public CompareFilter { 
   const int index1; 
   const int index2; 
 public: 
   DeltaDiffFilter(const COMPARE comp, const double threshold, 
               const int gindex1, const int gindex2, const string& name)  
      : CompareFilter(comp, threshold, name), index1(gindex1), index2(gindex2) 
     { } 
   virtual ~DeltaDiffFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      size_t sz = pTurn->depthEvalsCP->size();
      if (index1 >= sz || index2 >= sz) {
         return false;
      } else {
         int cpeval1 = pTurn->depthEvalsCP->at(index1).at(pTurn->turnHighestDepth);
         int cpeval2 = pTurn->depthEvalsCP->at(index2).at(pTurn->turnHighestDepth);
         return test(fabs(cpeval1 - cpeval2)/100.0);
      }
   } 
   virtual string getDescription() const { 
      return "DeltaDiff(" + IRutil::itoa(index1) + "," + IRutil::itoa(index2) 
                      + ") " + CompareFilter::getDescription(); 
   } 
}; 
 
 
/** Counts a turn with only one legal move as excluded. 
 */ 
//TAMAL: TODO DeltaIFilter(comp, threshold, 1, name) { }  //index from 0 .. what if no index 1 ??
class SecondDeltaFilter : public DeltaIFilter { 
 public: 
   SecondDeltaFilter(const COMPARE comp, const double threshold, const string& name) 
      : DeltaIFilter(comp, threshold, 1, name) { }  //index from 0 
   virtual ~SecondDeltaFilter() { } 
   //inherit apply and string getDescription() const 
}; 
 
class DeltaNFilter : public DeltaIFilter { 
 public: 
   DeltaNFilter(const COMPARE comp, const double threshold, const string& name) 
      : DeltaIFilter(comp, threshold, IRutil::MAXNUMLEGALMOVES - 1, name) { } 
   virtual ~DeltaNFilter() { } 
   //inherit apply and string getDescription() const 
}; 

class RepCountFilter : public CompareFilter {
   int count; //not used to compare, instead int passed to parent
   bool useReplines;  //Intended to help filter moves OUT
 public:
   RepCountFilter(const COMPARE comp, const int gcount, bool ur = false, const string& name = "RCF")
      : CompareFilter(comp, gcount, name), count(gcount), useReplines(ur)
   { }
   virtual ~RepCountFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      int rc = pTurn->repeatCount;
      bool extraFilter = true; //need to remain true to keep move IN
      if (useReplines) {
         vector<Move>::const_iterator bi = pTurn->repMoves->begin();
         vector<Move>::const_iterator ei = pTurn->repMoves->end();
         extraFilter &= (find(bi,ei,pTurn->movePlayed) == ei);
         extraFilter &= (pTurn->repLine2.find(string(pTurn->movePlayed)) == string::npos);
         //!!! must improve parsing of repLine2
      }
      return (extraFilter && test(rc));
   }
   virtual string getDescription() const {
      return "RepCount " + relation() + " " + IRutil::itoa(count) + (useReplines ? ", using replines" : "");
   }
};

class PlayedMoveIndexFilter : public CompareFilter {
 public:
   PlayedMoveIndexFilter(const COMPARE comp, const int index, const string& name)
      : CompareFilter(comp, index, name) { }
   virtual ~PlayedMoveIndexFilter() { }
   virtual bool apply(const TurnInfo* const pTurn) const {
      return test(pTurn->playedMoveIndex);
   }
   virtual string getDescription() const {
      return "PlayedMoveIndex " + CompareFilter::getDescription();
   }
};

class SelectorFilter : public virtual TurnFilter {
 protected:
   const MoveSelector* const theSelector;
   SelectorFilter(const string& gname, const MoveSelector* const ms)
         : TurnFilter(gname), theSelector(ms) {
      name = gname;  //??? From CompareFilter
   }

 public:
   virtual ~SelectorFilter() { }

   virtual bool apply(const TurnInfo* const pTurn) const = 0;

   virtual string getDescription() const {
      return "Filter from selector " + theSelector->getName();
   }
};

class PlayedMoveSelectedFilter : public SelectorFilter {
 public: 
   explicit PlayedMoveSelectedFilter(const MoveSelector* const ms)
         : SelectorFilter("PlayedMoveSelected by "+ms->getName(), ms) { }
   virtual ~PlayedMoveSelectedFilter() { }

   virtual bool apply(const TurnInfo* const pTurn) const {
      return theSelector->apply(pTurn, pTurn->movePlayed);
   }

   virtual string getDescription() const {
      return "Played move is a " + theSelector->getDescription();
   }
};

class EngineMoveSelectedFilter : public SelectorFilter {
 public:
   explicit EngineMoveSelectedFilter(const MoveSelector* const ms)
         : SelectorFilter("EngineMoveSelected by "+ms->getName(), ms) { }
   virtual ~EngineMoveSelectedFilter() { }

   virtual bool apply(const TurnInfo* const pTurn) const {
      return theSelector->apply(pTurn, pTurn->engineMove);
   }

   virtual string getDescription() const {
      return "Engine move is a " + theSelector->getDescription();
   }
};

class EqualTopMoveSelectedFilter : public SelectorFilter {
 public:
   explicit EqualTopMoveSelectedFilter(const MoveSelector* const ms)
         : SelectorFilter("EqualTopMoveSelected by"+ms->getName(), ms) { }
   virtual ~EqualTopMoveSelectedFilter() { }

   virtual bool apply(const TurnInfo* const pTurn) const {
      int i = 0;
      while (i < pTurn->consideredMoves->size()
               && pTurn->depthEvalsCP->at(i) == pTurn->depthEvalsCP->at(0)) {
         if (theSelector->apply(pTurn, pTurn->consideredMoves->at(i))) {
            return true;
         }
         i++;
      }
      return false;
   }

   virtual string getDescription() const {
      return "Some optimal move is a " + theSelector->getDescription();
   }
};

class SelectionNonemptyFilter : public SelectorFilter {
 public:
   explicit SelectionNonemptyFilter(const MoveSelector* const ms) 
         : SelectorFilter("Has"+ms->getName(), ms) { }
   virtual ~SelectionNonemptyFilter() { }

   virtual bool apply(const TurnInfo* const pTurn) const {
      int i = 0;
      while (i < pTurn->numLegalMoves) {
         if (theSelector->apply(pTurn, pTurn->legalMoves->at(i))) {
            return true;
         }
         i++;
      }
      return false;
   }

   virtual string getDescription() const {
      return "Some legal move is a " + theSelector->getDescription();
   }
};




//firstLine matches if engine move is the same as played move. 
class FirstLineMatchFilter : public TurnFilter { 
 public: 
   FirstLineMatchFilter() : TurnFilter("FirstLineMatch") { } 
   virtual ~FirstLineMatchFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return pTurn->firstLineMatch; 
   } 
   virtual string getDescription() const { 
      return "FirstLineMatchFilter"; 
   } 
}; 
 
// top patch if currEval is equal to the value of the played move.. played move is tie to the top... 
// Make sure.. currently currEval should be the eval of the best move not the played move. 
class EqualTopMoveFilter : public TurnFilter { 
 public: 
   EqualTopMoveFilter() : TurnFilter("EqualTopMove") { } 
   virtual ~EqualTopMoveFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
      return pTurn->eqTopMatch; 
   } 
   virtual string getDescription() const { 
      return "Move equivalent to top move played"; 
   } 
}; 
 

/** Use top-depth values (before scaling) to meter the error made by player.
    Comparison is with eval(best move) - eval(played move)
 */
class FalloffFilter : public CompareFilter { 
 public: 
   FalloffFilter(const COMPARE comp, const double delta, const string& name)  
      : CompareFilter(comp, delta, name) { } 
   virtual ~FalloffFilter() { } 
   virtual bool apply(const TurnInfo* const pTurn) const { 
	   //TAMAL: TODO: What to do with deltaFallOff // Commemnting right now 
     // return test(IRutil::round(pTurn->deltaFalloff,2)); 
	   return test(0); 
   } 
   virtual string getDescription() const { 
      return "FalloffFilter " + CompareFilter::getDescription(); 
   } 
}; 
 
 
 
#endif    //end of #ifndef __IR_FILTERS_H__ 
 
 

 
