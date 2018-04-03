//File--------------------------GameInfo.h-------------------------------- 
 
#ifndef __IR_GAME_INFO_H__ 
#define __IR_GAME_INFO_H__ 

class TurnInfo;

class GameInfo { 
public:  
   const string gameID;
   const string engineID;
   const string platform;
   const int numThreads;
   const int hash;
   const int numPV;
   int minEngineDepth; //first depth with values given by engine
   const int minHighDepth;
   const int maxHighDepth;   //from AIF DepthRange
   const string egt;
   const string mode;
   //const string direction;  //from AIF mode
   //const string hashPolicy;
   //const string startTurn;
   
   const string eventName; 
   const string site; 
   const IRutil::SimpleDate date; 
   const int round; 
   const string whitePlayer; 
   const string blackPlayer; 
   const string result;//string is fine, as it is the pgn standard 

   const string eco; 
   const int whiteElo; 
   const int blackElo; 
   const int plyCount; 
   const IRutil::SimpleDate eventDate; 
   const string eventType;
   const int eventRounds;
   const string eventCountry;
   const string eventCategory;
   const string source; 
   const IRutil::SimpleDate sourceDate; 
   const string timeControl;  //e.g. [TimeControl "40/4800:20/2400:1200"]
   const string whiteTeam;
   const string blackTeam;
   const string whiteTeamCountry;
   const string blackTeamCountry;
   const string whiteFIDEID;
   const string blackFIDEID;


   //Extra fields
   const string gameBody;
   vector<TurnInfo*>* gameTurns;  //since turns are unique to game, can delete
 
   GameInfo(const string& gid, const string& eid, const string& pl,
      int nt, int ha, int numpv, int mindepth, int maxdepth, 
      const string& egtb, const string& modestr, const string& ev, const string& si,
      const IRutil::SimpleDate& dt, int rd, const string& wp, 
      const string& bp, const string& rslt, const string& eco2,
      int welo, int belo, int plyc, const IRutil::SimpleDate& edate, 
      const string& et, int er, const string& ec, const string& ecat, const string& src,
      const IRutil::SimpleDate& sdate, const string& tc, const string& wt, const string& bt,
      const string& wtc, const string& btc, const string& wfid, const string& bfid,
      const string& gb = "") 
      : gameID(gid), engineID(eid), platform(pl), numThreads(nt), hash(ha), numPV(numpv),
        minEngineDepth(1), minHighDepth(mindepth), maxHighDepth(maxdepth), egt(egtb), mode(modestr),
        eventName(ev), site(si), date(dt), round(rd), whitePlayer(wp), blackPlayer(bp),
        result(rslt), eco(eco2), whiteElo(welo), blackElo(belo), plyCount(plyc),
        eventDate(edate), eventType(et), eventRounds(er), eventCountry(ec), eventCategory(ecat),
        source(src), sourceDate(sdate), timeControl(tc), whiteTeam(wt), blackTeam(bt),
        whiteTeamCountry(wtc), blackTeamCountry(btc), whiteFIDEID(wfid), blackFIDEID(bfid),
        gameBody(gb), gameTurns(new vector<TurnInfo*>())
   {
      if (eid.find("omod") != string::npos) {
         minEngineDepth = 5;
      }
   } 
 
   virtual ~GameInfo() {
      /*
      for (vector<TurnInfo*>::iterator itr = gameTurns->begin(); itr != gameTurns->end(); itr++) {
         delete(*itr);
      } //Not allowed with forward reference, managed by Ensemble
      */
      delete(gameTurns); //individual turns are cleared from focus trial
   } 
}; 
 
#endif //end of __IR_GAME_INFO_H__ 
 
 
 
 
