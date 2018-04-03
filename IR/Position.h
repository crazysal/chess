//File--------------------Position.h---------------------------------------

#ifndef __IR_POSITION_H__
#define __IR_POSITION_H__

#include <vector>
#include <set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>



/** "Total Algebraic Notation" (TAN) gives all the information needed
 *  to translate a move into any format (SAN or LAN or UCI) without
 *  needing to refer to the board in order to disambiguate.  Format:
 *
 *     <piece><origin square>[- or x]<dest. square>[optional suffix]
 *
 *  Even for a Pawn move, the piece ('P' for White, 'p' for Black) is
 *  included.  The squares are in algebraic format [a-h][1-8].  Suffixes:
 *
 *  c for castling, e.g. Ke1-g1c for O-O by White.
 *  e for an en-passant capture.
 *  QRBNqrbn for promotion by White or Black
 *  i if the SAN translation needs to give the row (rank), e.g. N5d4
 *  j if SAN needs to give the file, e.g. Nbd7
 *  k if SAN needs to give both.
 *
 *  The advantage is that all information about a move is present and
 *  appears at a fixed place in the string.
 *
 */


const string promStr = "qrbnQRBN";
inline bool isProm(char c) { return promStr.find(c) != string::npos; }

class Move {
   string tanMove;
 public:
   Move(): tanMove(string("")) { }
   Move(const Move& mv): tanMove(mv.tanMove) { }
   explicit Move(const string& smv): tanMove(smv) { }
   Move& operator=(const Move& rhs) {
      tanMove = rhs.tanMove;
      return(*this);
   }
   string getTAN() const { return tanMove; }
   void setTAN(const string& tmv) { tanMove = tmv; }
   //Move(const string& fen, const string& move): tanMove(Position(fen).move2TAN(move))
   //{ }
   string TAN2UCI() const {  //allows "King Takes Rook" style castling
      if (tanMove.length() < 6 || tanMove.length() > 7) {
         cerr << "Bad TAN2UCI " << tanMove << endl; return tanMove; 
      }
      string suff = tanMove.substr(6);
      string prom = ((suff != "" && isProm(suff.at(0))) ? 
            string("")+IRutil::makeLower(suff.at(0)) : "");
      return (tanMove.substr(1,2) + tanMove.substr(4,2) + prom);
   }
   
   string TAN2LAN() const {
      if (tanMove.length() < 6 || tanMove.length() > 7) {
         cerr << "Bad TAN2LAN " << tanMove << endl; return tanMove; 
      }
      string suff = tanMove.substr(6);
      if (suff == "c") {
         return ((IRutil::ord(tanMove.at(1)) < IRutil::ord(tanMove.at(4))) ? "O-O" : "O-O-O");
      }
      string prom = ((suff != "" && isProm(suff.at(0))) ? 
                                          string("")+IRutil::makeUpper(suff.at(0)) : "");
      char piece = (IRutil::ord(tanMove.at(0) < IRutil::ord('a') ? 
                          tanMove.at(0) : IRutil::chr(IRutil::ord(tanMove.at(0)) - 32)));
      string lanPiece = (piece == 'P' ? "" : string("") + piece);
      return (lanPiece + tanMove.substr(1,5) + prom);
   }
   
   string TAN2SAN() const {
      if (tanMove.length() < 6 || tanMove.length() > 7) {
         cerr << "Bad TAN2SAN " << tanMove << endl; return tanMove; 
      }
      string suff = tanMove.substr(6);
      if (suff == "c") {
         return ((IRutil::ord(tanMove[1]) < IRutil::ord(tanMove[4])) ? "O-O" : "O-O-O");
      }
      string prom = ((suff != "" && isProm(suff.at(0))) ? 
            string("")+IRutil::makeUpper(suff.at(0)) : "");
      char piece = (IRutil::ord(tanMove.at(0) < IRutil::ord('a') ? tanMove.at(0) 
            : IRutil::chr(IRutil::ord(tanMove.at(0)) - 32)));
      string disambig = (suff == "" ? "" 
                           : (suff == "j" ? tanMove.substr(1,1)
                                : (suff == "i" ? tanMove.substr(2,1)
                                     : (suff == "k" ? tanMove.substr(1,2) : ""))));
      string bridge = (tanMove[3] == 'x' ? "x" : "");
      string sanPiece = (piece == 'P' ? (bridge == "x" ? ""+tanMove.at(1) : "") : string("") + piece);
      
      return (sanPiece + disambig + bridge + tanMove.substr(4,2) + prom);
   }

   operator string() const { return tanMove; }

   bool operator==(const Move& rhs) const { return tanMove == rhs.tanMove; }
   bool operator<(const Move& rhs) const { return tanMove < rhs.tanMove; }
};

inline ostream& operator<<(ostream& OUT, const Move& rhs) {
   OUT << rhs.getTAN();
   return OUT;
}
   


enum PIECES {
   EM, WK, WQ, WR, WB, WN, WP, BK, BQ, BR, BB, BN, BP
};

const string pieceOf = " KQRBNPkqrbnp";

struct MoveParse {
 public:
   char piece;
   string sourceInfo;
   string dest;  //always known in full
   bool capture;
   string prom;
   MoveParse(char p, const string& si, const string& d, bool cap, const string& suff)
    : piece(p), sourceInfo(si), dest(d), capture(cap), prom(suff)
   { }
   MoveParse() : piece(' '), sourceInfo(""), dest(""), capture(false), prom("") { }
   string toString() const { 
      string bridge = (capture ? "x" : "-");
      return string("") + piece + sourceInfo + bridge + dest + prom;
   }
};

   
/** Partial implementation of a chess position---only enough to
 *  decode FEN input and disambiguate SAN moves presumed legal.
 *  Does not check legality or do move generation.  Hence does not
 *  maintain castling rights and other FEN features.
 */
class Position {
 public:
   vector<PIECES>* hexBoard; // a1=0 .. a2=16 .. a8=112 .. h8=127
   set<int> *WKSQ, *WQSQ, *WRSQ, *WBSQ, *WNSQ, *WPSQ;
   set<int> *BKSQ, *BQSQ, *BRSQ, *BBSQ, *BNSQ, *BPSQ;
   bool wtm;
   bool chess960;
   map<PIECES,set<int>*> sqsOf;

   explicit Position(const string& fen = "", bool c960 = false)
    : hexBoard(new vector<PIECES>(128,EM))
    , WKSQ(new set<int>()), WQSQ(new set<int>()), WRSQ(new set<int>())
    , WBSQ(new set<int>()), WNSQ(new set<int>()), WPSQ(new set<int>())
    , BKSQ(new set<int>()), BQSQ(new set<int>()), BRSQ(new set<int>())
    , BBSQ(new set<int>()), BNSQ(new set<int>()), BPSQ(new set<int>())
    , chess960(c960)
   {
      sqsOf[WK] = WKSQ; sqsOf[WQ] = WQSQ; sqsOf[WR] = WRSQ; 
      sqsOf[WB] = WBSQ; sqsOf[WN] = WNSQ; sqsOf[WP] = WPSQ;
      sqsOf[BK] = BKSQ; sqsOf[BQ] = BQSQ; sqsOf[BR] = BRSQ; 
      sqsOf[BB] = BBSQ; sqsOf[BN] = BNSQ; sqsOf[BP] = BPSQ;
      if (fen == "") {
         string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "; 
         setFromFEN(startFEN);
      } else {
         setFromFEN(fen);
      }
   }

   Position(const Position& p)
    : hexBoard(new vector<PIECES>(*(p.hexBoard)))
    , WKSQ(new set<int>(*(p.WKSQ))), BKSQ(new set<int>(*(p.BKSQ)))
    , WQSQ(new set<int>(*(p.WQSQ))), WRSQ(new set<int>(*(p.WRSQ)))
    , WBSQ(new set<int>(*(p.WBSQ))), WNSQ(new set<int>(*(p.WNSQ)))
    , WPSQ(new set<int>(*(p.WPSQ))), BQSQ(new set<int>(*(p.BQSQ)))
    , BRSQ(new set<int>(*(p.BRSQ))), BBSQ(new set<int>(*(p.BBSQ)))
    , BNSQ(new set<int>(*(p.BNSQ))), BPSQ(new set<int>(*(p.BPSQ)))
    , wtm(p.wtm), sqsOf(p.sqsOf)
   { }

   Position& operator=(const Position& p) {
      hexBoard = new vector<PIECES>(*(p.hexBoard));
      WKSQ = new set<int>(*(p.WKSQ)); BKSQ = new set<int>(*(p.BKSQ));
      WQSQ = new set<int>(*(p.WQSQ)); WRSQ = new set<int>(*(p.WRSQ));
      WBSQ = new set<int>(*(p.WBSQ)); WNSQ = new set<int>(*(p.WNSQ));
      WPSQ = new set<int>(*(p.WPSQ)); BQSQ = new set<int>(*(p.BQSQ));
      BRSQ = new set<int>(*(p.BRSQ)); BBSQ = new set<int>(*(p.BBSQ));
      BNSQ = new set<int>(*(p.BNSQ)); BPSQ = new set<int>(*(p.BPSQ));
      wtm = p.wtm;
      sqsOf = p.sqsOf;
      return (*this);
   }

   ~Position() {
      delete(hexBoard);
      delete(WKSQ); delete(WQSQ); delete(WRSQ); delete(WBSQ); delete(WNSQ); delete(WPSQ);
      delete(BKSQ); delete(BQSQ); delete(BRSQ); delete(BBSQ); delete(BNSQ); delete(BPSQ);
      //for (map<PIECES,set<int>*>::iterator it = sqsOf.begin(); it != sqsOf.end(); it++) {
         //delete(it->second);
      //}
   }


   void setFromFEN(const string& fen) {
      map<PIECES,set<int>*>::iterator it = sqsOf.begin();
      while (it != sqsOf.end()) {
         (*it++).second->clear();
      }
      for (int i = 0; i < 128; i++) { hexBoard->at(i) = EM; }
      //hexBoard = new vector<PIECES>(128,EM);
      int i = 0;
      int sq = 112;
      char c = fen.at(i);
      //size_t pp = pieceOf.find(string("")+c);
      size_t pp = pieceOf.find(c);
      PIECES piece;
      while (c != ' ') {
         switch(c) {
          case '/':
            sq -= 25;  //will add 1 later
            break;
          case 'k':
          case 'q':
          case 'r':
          case 'b':
          case 'n':
          case 'p':
          case 'K':
          case 'Q':
          case 'R':
          case 'B':
          case 'N':
          case 'P':
            piece = PIECES(pp);
            hexBoard->at(sq) = piece;
            sqsOf[piece]->insert(sq);
            break;
          case '8':
          case '7':
          case '6':
          case '5':
          case '4':
          case '3':
          case '2':
          case '1':
            sq += (IRutil::ord(c) - IRutil::ord('1')); // 1 gets added below
            break;
          default:
            cerr << "Unhandled FEN char " << c << endl;
         }
         i++;
         c = fen.at(i);
         //pp = pieceOf.find(string("")+c);
         pp = pieceOf.find(c);
         sq++;
      }
      i++;  //was on first space, now on side to move char
      wtm = (fen.at(i) == 'w');
   }

   string toBoardDiagram() {   //from Hugh Meyers' Perl version.
      bool whiteSquare = true;  //a8 is white
      ostringstream OUT;
      for (int rank = 7; rank >= 0; rank--) {
   
         // append numbers on left-hand side
         OUT << (rank + 1);
         OUT << "  ";
   
         for (int file = 0; file < 8; file++) {
   
            // append the piece, or a dash if it's a vacant black square
            PIECES piece = (hexBoard->at(16*rank + file));
            if (piece != EM) {
               OUT << pieceOf[piece];
            } else {
               OUT << (whiteSquare ? " " : "-");
            }
            // toggle whitesquare each square
            whiteSquare = !whiteSquare;
         }
         OUT << endl;
   
         // also toggle whitesquare each rank
         whiteSquare = !whiteSquare;
      }
      OUT << "   abcdefgh " << (wtm ? "wtm" : "btm") << endl;
      return OUT.str();
   }

   static bool valid(int hexsq) { 
      return (hexsq >= 0 && hexsq <= 127 && hexsq % 16 <= 7); 
   } 

   static int alg2hex(const string& algsq) {
      return 16*(IRutil::ord(algsq[1]) - IRutil::ord('1'))
                    + IRutil::ord(algsq[0]) - IRutil::ord('a');
   }

   static string hex2alg(int hexsq) {
      int quot = hexsq/16;
      int rem = hexsq % 16;
      string algsq = "00";
      algsq[0] = IRutil::chr(IRutil::ord('a') + rem);
      algsq[1] = IRutil::chr(IRutil::ord('1') + quot);
      return algsq;
   }

   bool canSlideAlong(int hexFrom, int hexTo, int baseDiff) const {
      int ssq = hexFrom + baseDiff;
      while (valid(ssq) && ssq != hexTo) {
         if (hexBoard->at(ssq) != EM) {
            return false;
         } else {
            ssq += baseDiff;
         }
      }
      return (ssq == hexTo);
   }

   //Return nonzero increment if true, else 0.
   int slideDirection(int hexFrom, int hexTo) const {
      int diff = hexTo - hexFrom;  //add to from in order to move
      int sdiff = IRutil::sgn(diff);
      int baseDiff = (fabs(diff) <= 7 ? sdiff
                        : (diff % 15 == 0 ? 15*sdiff
                             : (diff % 16 == 0 ? 16*sdiff
                                  : diff % 17 == 0 ? 17*sdiff : 0)));
      return ((baseDiff != 0 && canSlideAlong(hexFrom, hexTo, baseDiff)) ? baseDiff : 0);
   }
      
   int pinDirection(int hexsq) const {   //toward pinner when adding, 0 if none
      int ksq = (wtm ? (*WKSQ->begin()) : (*BKSQ->begin()));
      int baseDiff = slideDirection(ksq,hexsq);
      if (baseDiff == 0) { return 0; }
      int ssq = hexsq + baseDiff;
      while (valid(ssq) && hexBoard->at(ssq) == EM) {
         ssq += baseDiff;
      }
      if (!valid(ssq)) { return 0; }  //any invalid square could be used
      if ((wtm && hexBoard->at(ssq) == BQ) || ((!wtm) && hexBoard->at(ssq) == WQ)) {
         return baseDiff;   //return ssq; 
      } //else
      if ((wtm && hexBoard->at(ssq) == BR) || ((!wtm) && hexBoard->at(ssq) == WR)) {
         return ((baseDiff == 1 || baseDiff == -1 || baseDiff % 16 == 0) ? baseDiff : 0);
      } //else
      if ((wtm && hexBoard->at(ssq) == BB) || ((!wtm) && hexBoard->at(ssq) == WB)) {
         return ((baseDiff % 15 == 0 || baseDiff % 17 == 0) ? baseDiff : 0);
      } //else
      return 0;
   }
  

   /** REQ: piece on source square is QRBN, returns false if not.
    *  Since we presume given move is legal, we need not check that dest square
    *  is not occupied by a same-colored piece in filtering source squares.
    */
   bool isSemiLegal(int hexFrom, int hexTo) const {
      PIECES piece = hexBoard->at(hexFrom);
      if (piece == WN || piece == BN) {
         int ad = fabs(hexFrom - hexTo);
         return ((ad == 14 || ad == 18 || ad == 31 || ad == 33)
                    && pinDirection(hexFrom) == 0);
      } //else
      int baseDiff = slideDirection(hexFrom,hexTo);
      int pinDiff = pinDirection(hexFrom);
      bool moveOK = (baseDiff != 0 && (piece == WQ || piece == BQ 
         || ((piece == WB || piece == BB) && (baseDiff % 15 == 0 || baseDiff % 17 == 0))
         || ((piece == WR || piece == BR) && 
               (baseDiff == -1 || baseDiff == 1 || baseDiff % 16 == 0))));
      return (moveOK && (pinDiff == 0 || fabs(pinDiff) == fabs(baseDiff)));
   }

   bool blackInCheck() const {
      set<int>::const_iterator itr = BKSQ->begin();
      int bksq = *itr;
      int wksq = *(WKSQ->begin());
      int dir = bksq - wksq;  //direction from source square ssq, = bksq - ssq;
      int ad = fabs(dir);
      if (ad == 1 || ad == 15 || ad == 16 || ad == 17) { return true; }
      for (itr = WPSQ->begin(); itr != WPSQ->end(); itr++) {
         dir = bksq - *itr;
         if (dir == 15 || dir == 17) { return true; }
      }
      for (itr = WNSQ->begin(); itr != WNSQ->end(); itr++) {
         ad = fabs(bksq - *itr);
         if (ad == 14 || ad == 18 || ad == 31 || ad == 33) { return true; }
      }
      for (itr = WBSQ->begin(); itr != WBSQ->end(); itr++) {
         ad = fabs(slideDirection(*itr,bksq));
         if (ad == 15 || ad == 17) { return true; }
      }
      for (itr = WRSQ->begin(); itr != WRSQ->end(); itr++) {
         ad = fabs(slideDirection(*itr,bksq));
         if (ad == 1 || ad == 16) { return true; }
      }
      for (itr = WQSQ->begin(); itr != WQSQ->end(); itr++) {
         ad = fabs(slideDirection(*itr,bksq));
         if (ad == 1 || ad == 15 || ad == 16 || ad == 17) { return true; }
      }
      return false;
   }
      
   bool whiteInCheck() const {
      set<int>::const_iterator itr = BKSQ->begin();
      int bksq = *itr;
      int wksq = *(WKSQ->begin());
      int dir = wksq - bksq;  //direction from source square ssq, = wksq - ssq;
      int ad = fabs(dir);
      if (ad == 1 || ad == 15 || ad == 16 || ad == 17) { return true; }
      for (itr = BPSQ->begin(); itr != BPSQ->end(); itr++) {
         dir = wksq - *itr;
         if (dir == -15 || dir == -17) { return true; }
      }
      for (itr = BNSQ->begin(); itr != BNSQ->end(); itr++) {
         ad = fabs(wksq - *itr);
         if (ad == 14 || ad == 18 || ad == 31 || ad == 33) { return true; }
      }
      for (itr = BBSQ->begin(); itr != BBSQ->end(); itr++) {
         ad = fabs(slideDirection(*itr,wksq));
         if (ad == 15 || ad == 17) { return true; }
      }
      for (itr = BRSQ->begin(); itr != BRSQ->end(); itr++) {
         ad = fabs(slideDirection(*itr,wksq));
         if (ad == 1 || ad == 16) { return true; }
      }
      for (itr = BQSQ->begin(); itr != BQSQ->end(); itr++) {
         ad = fabs(slideDirection(*itr,wksq));
         if (ad == 1 || ad == 15 || ad == 16 || ad == 17) { return true; }
      }
      return false;
   }

   bool inCheck() const { return (wtm ? whiteInCheck() : blackInCheck()); }
         

   /** Parsing strategy: After treating castling separately, we look for the
    *  rightmost match to [a-h][1-8] as destination square, with m indexing a-h.
    *  If m == 0 then it's a simple SAN pawn move like e4.
    *  Else we test for a bridge - x : or source char in [a-h] or [1-8].
    *  If bridge is present then we test previous char for [a-h] or [1-8].  
    *  Let k index this test char.  Cases:
    *  [1-8] is a rank disambiguator---then test prev char again
    *     If prevchar is in [a-h] then we have full source square, which means
    *        move is in UCI or LAN or TAN.  Check board for piece, not first char.
    *     Else it should be a piece and k-1 == 0,
    *  [a-h] is a file disambiguator, as in exd5 or Nfxd4 or Rae8. 
    *     Again prev char should be piece with index k-1 == 0.
    *  MoveParse(char p, string si, string d, bool cap, string suff)
    */
   MoveParse parseMove(const string& move) const {
      //ofstream oflog("out.txt", ios::app);
      //oflog << move << endl;
      //oflog.close();
      MoveParse mp; //construction not just declaration
      size_t n = move.size();
      if (n <= 1) { cerr << "Bad move " << move << endl; return mp; }
      char c = move.at(0);
      if (c == 'O' || c == '0') {  //castles, allows Chess960 "King takes Rook" form
         int ksq = (wtm ? (*WKSQ->begin()) : (*BKSQ->begin()));
         int basediff = ((n >= 4 && move.at(3) == '-') ? -1 : +1); //O-O-O apart from O-O+!
         int rooksq = ksq + basediff;
         while (valid(rooksq) && hexBoard->at(rooksq) == EM) { rooksq += basediff; }
         int kdest = (chess960 ? rooksq : ksq + 2*basediff);
         char k = (wtm ? 'K' : 'k');
         MoveParse mp(k,hex2alg(ksq),hex2alg(kdest),false,"c");
         return mp;
      } //now parse promotion apart from annotations like +, #, !, ?
      int m = n - 1;
      char destRank = move.at(m);
      if (destRank == 'c') { mp.prom = "c"; }  //could be TAN castling
      while (IRutil::ord(destRank) > IRutil::ord('8') 
                || IRutil::ord(destRank) < IRutil::ord('1')) {
         destRank = move.at(--m);
      }
      if (m < n - 1) {
         char e = move.at(m+1);  
         if (e == '=' && m < n - 2) {  //promotion in "=Q" style
            e = move.at(m+2);
         }
         if (e == 'Q' || e == 'q') {   //UCI as well as TAN uses lowercase
            mp.prom = string("") + (wtm ? 'Q' : 'q');
         } else if (e == 'R' || e == 'r') {
            mp.prom = string("") + (wtm ? 'R' : 'r');
         } else if (e == 'B' || e == 'b') {
            mp.prom = string("") + (wtm ? 'B' : 'b');
         } else if (e == 'N' || e == 'n') {
            mp.prom = string("") + (wtm ? 'N' : 'n');
         } // else leave prom == ""
      }
      if (m < 1) { cerr << "Badd move " << move << endl; return mp; }
      char destFile = move.at(--m);
      if (IRutil::ord(destFile) < IRutil::ord('a') 
             || IRutil::ord(destFile) > IRutil::ord('h')) {
         cerr << "Baddd move " << move << endl; return mp;
      }
      mp.dest = string("") + destFile + destRank;
      int hexsq;  //source square we are finding
      mp.capture = (hexBoard->at(alg2hex(mp.dest)) != EM);
      if (m > 0) {  //not a simple Pawn move like e4 
         int k = m - 1;
         char b = move.at(k);
         if (b == 'x' || b == ':') {
            mp.capture = true;
            k--;
         } else if (b == '-') {
            k--;  //leave capture false
         }

         if (k < 0) { cerr << "Badddd move " << move << endl; return mp; }
         b = move.at(k);
         if (IRutil::ord('1') <= IRutil::ord(b) && IRutil::ord(b) <= IRutil::ord('8')) { 
            //source rank given
            mp.sourceInfo = string("") + b;
            if (k == 0) { cerr << "Baddddd move " << move << endl; return mp; }
            char a = move.at(k-1);
            if (IRutil::ord('a') <= IRutil::ord(a) && IRutil::ord(a) <= IRutil::ord('h')) { 
               //whole source square given.  Might be only info in UCI move.
               mp.sourceInfo = string("") + a + mp.sourceInfo;
               hexsq = alg2hex(mp.sourceInfo);
               mp.piece = pieceOf.at(hexBoard->at(hexsq));
               if (mp.piece == ' ') { cerr << "Mislaid move " << move << endl; }
               //now must allow for UCI castling including chess960 variant
               if ((mp.piece == 'K' && (move == "e1g1" || move == "e1c1" 
                                        || hexBoard->at(alg2hex(mp.dest)) == WR) 
                    || (mp.piece == 'k' && (move == "e8g8" || move == "e8c8" 
                                        || hexBoard->at(alg2hex(mp.dest)) == BR)))) {
                  mp.prom = "c";
               } else if ((mp.piece == 'P' || mp.piece == 'p')
                             && mp.dest.at(0) != mp.sourceInfo.at(0)
                             && hexBoard->at(alg2hex(mp.dest)) == EM) {  
                  mp.prom = "e";
               }
               return mp;
            } else {   //it must be a piece---nothing more to do yet.
               k--;
            }
         } else if (IRutil::ord('a') <= IRutil::ord(b) && IRutil::ord(b) <= IRutil::ord('h')) { 
            //source file given
            mp.sourceInfo = string("") + b;
            if (k == 0) {   //move was capture like exd4
               mp.piece = (wtm ? 'P' : 'p');
               k = -1;
            } else {
               k--;
            }
         }
         if (k > 0) {
            cerr << "Parsing mishap on move " << move << endl; return mp; 
         } else if (k == 0 && mp.piece == ' ') {  //piece given in move, need to read
            mp.piece = (wtm ? IRutil::makeUpper(move.at(0)) : IRutil::makeLower(move.at(0)));
         }
      } else {   //simple pawn move case, no help with source square
         mp.piece = (wtm ? 'P' : 'p');
      } //now we have piece and sourceInfo and dest square.  Must complete source square.
      return mp;
   }


   /** TAN extends LAN with info needed to translate to SAN without looking at the board.
    *  If the rank is needed to disambiguate, append "i"; if the file, "j"; if both, "k";
    *  Else the suffix can be "c" for castling, "e" for en-passant, or a promoted piece.
    *  All mentions of Black pieces are lowercased in TAN.
    *  Legality of moves not needing disambiguation, e.g. pinned pawns, is *not* checked.
    */
   string move2TAN(const string& move) const {
      MoveParse mp = parseMove(move);
      string bridge = (mp.capture ? "x" : "-");
      string suff = (mp.capture && hexBoard->at(alg2hex(mp.dest)) == EM ? "e" : mp.prom);
      string source;
      if (mp.piece == 'K') {
         source = hex2alg(*WKSQ->begin());  //good for castling too
         if (hexBoard->at(alg2hex(mp.dest)) == WR) { suff = "c"; } //Chess960 UCI
         return string("K") + source + bridge + mp.dest + suff;
      }  //else
      if (mp.piece == 'k') {
         source = hex2alg(*BKSQ->begin()); //TAN for Black O-O is "ke8-g8c"
         if (hexBoard->at(alg2hex(mp.dest)) == BR) { suff = "c"; } //Chess960 UCI
         return string("k") + source + bridge + mp.dest + suff;
      }  //else
      if (mp.piece == 'P') {
         if (mp.sourceInfo.length() == 2) {
            return string("P") + mp.sourceInfo + bridge + mp.dest + suff;
         } else if (mp.sourceInfo.length() == 1) {  //pawn capture
            char sourceRank = IRutil::chr(-1 + IRutil::ord(mp.dest[1]));
            return string("P") + mp.sourceInfo + sourceRank + bridge + mp.dest + suff;
         } else {   //simple SAN move such as e4, could be double-move
            int prevsq = alg2hex(mp.dest) - 16;
            bool djump = (hexBoard->at(prevsq) == EM);
            source = hex2alg(djump ? prevsq - 16 : prevsq);
            return string("P") + source + bridge + mp.dest + suff;
         }
      }  //else
      if (mp.piece == 'p') {
         if (mp.sourceInfo.length() == 2) {
            return string("p") + mp.sourceInfo + bridge + mp.dest + suff;
         } else if (mp.sourceInfo.length() == 1) {  //pawn capture
            char sourceRank = IRutil::chr(1 + IRutil::ord(mp.dest[1]));
            return string("p") + mp.sourceInfo + sourceRank + bridge + mp.dest + suff;
         } else {   //simple SAN move such as e4, could be double-move
            int prevsq = alg2hex(mp.dest) + 16;
            bool djump = (hexBoard->at(prevsq) == EM);
            source = hex2alg(djump ? prevsq + 16 : prevsq);
            return string("p") + source + bridge + mp.dest + suff;
         }
      }  //else
      size_t pp = pieceOf.find(mp.piece);
      PIECES piece;
      if (pp != string::npos) {
         piece = PIECES(pp);
      } else {
         cerr << "Cannot determine move from " << mp.toString() << endl;
         return "";
      }
      vector<string> hitSquares;
      vector<string> filteredSquares; //not used
      int hitCount = 0;
      int trueCount = 0;
      int hexTo = alg2hex(mp.dest);
   
      set<int>::const_iterator it = ((sqsOf.find(piece))->second)->begin();
      set<int>::const_iterator ed = ((sqsOf.find(piece))->second)->end();
      while (it != ed) {
         int hexFrom = *it++;
         string algsq = hex2alg(hexFrom);
         if (isSemiLegal(hexFrom,hexTo)) {
            hitCount++;
            hitSquares.push_back(algsq);
            if (mp.sourceInfo == "" || mp.sourceInfo == algsq 
                   || mp.sourceInfo == string("") + algsq[0]
                   || mp.sourceInfo == string("") + algsq[1]) {
               trueCount++;
               filteredSquares.push_back(algsq);
               source = algsq;  //optimistic
            }
         }
      }
               
         
      if (hitCount == 1) {   // no disambiguation needed
   
         return string("") + mp.piece + source + bridge + mp.dest + suff;
   
      } else if (trueCount != 1) {
         string flaw = (trueCount == 0 ? "unmakable" : "ambiguous");
         cerr << "Move " << mp.toString() << " is " << flaw << endl;
         return "";
      } //else---trueCount == 1 but hitCount >= 2.  Assume file disambig unless rank needed
   
      bool needFile = false;
      bool needRank = false;
   
      vector<string>::iterator itv = hitSquares.begin();
      while (itv != hitSquares.end()) {
         string algsqr = *itv++;
         if (algsqr != source) { 
            needRank = (needRank || algsqr[0] == source[0]);
            needFile = (needFile || algsqr[1] == source[1]);
         }
      }
      suff = "j";              // file---will we override it?
      if (needRank) {          // two source squares have same file
         suff = (needFile ? "k" : "i");
      }
   
      return (string("") + mp.piece + source + bridge + mp.dest + suff);
   
   }

   string move2UCI(const string& move) const { return Move(move2TAN(move)).TAN2UCI(); }
   string move2LAN(const string& move) const { return Move(move2TAN(move)).TAN2LAN(); }
   string move2SAN(const string& move) const { return Move(move2TAN(move)).TAN2SAN(); }

   void playMove(const Move& move) {
      string tanMove = move.getTAN();
      if (tanMove.length() < 6) { 
         cerr << "Bad TAN " << move << "->" << tanMove << endl << toBoardDiagram(); 
      }
      string algFrom = tanMove.substr(1,2);
      string algTo = tanMove.substr(4,2);
      string suff = tanMove.substr(6); // = "" if tanMove has none
      int hexFrom = alg2hex(algFrom);
      int hexTo = alg2hex(algTo);
      if (suff == "c") { //castling---need to find castled-with Rook (in Chess960)
         int diff = IRutil::sgn(hexTo - hexFrom);
         int hexsq = hexFrom + diff;
         while (hexBoard->at(hexsq) == EM) { hexsq += diff; }
         set<int>* rookSqs = (wtm ? WRSQ : BRSQ);
         set<int>* ksqs = (wtm ? WKSQ : BKSQ);
         hexBoard->at(hexsq) = EM;
         rookSqs->erase(hexsq);
         hexBoard->at(hexFrom) = EM;  //must be king's square
         ksqs->erase(hexFrom);
         int newksq = (wtm ? (diff > 0 ? alg2hex("g1") : alg2hex("c1"))
                           : (diff > 0 ? alg2hex("g8") : alg2hex("c8")));
         int newrsq = (wtm ? (diff > 0 ? alg2hex("f1") : alg2hex("d1"))
                           : (diff > 0 ? alg2hex("f8") : alg2hex("d8")));
         hexBoard->at(newksq) = (wtm ? WK : BK);
         ksqs->insert(newksq);
         hexBoard->at(newrsq) = (wtm ? WR : BR);
         rookSqs->insert(newrsq);
      } else if (suff == "e") {   //en-passant capture
         string capSq = string("") + algTo.at(0) + algFrom.at(1);
         int capHex = alg2hex(capSq);
         set<int>* samePawns = (wtm ? WPSQ : BPSQ);
         set<int>* oppPawns = (wtm ? BPSQ : WPSQ);
         hexBoard->at(hexFrom) = EM;
         samePawns->erase(hexFrom);
         hexBoard->at(capHex) = EM;
         oppPawns->erase(capHex);
         hexBoard->at(hexTo) = (wtm ? WP : BP);
         samePawns->insert(hexTo);
      } else {
         PIECES taken = hexBoard->at(hexTo);
         if (taken != EM) {
            hexBoard->at(hexTo) = EM;
            sqsOf[taken]->erase(hexTo);
         }
         PIECES piece = hexBoard->at(hexFrom);
         hexBoard->at(hexFrom) = EM;
         sqsOf[piece]->erase(hexFrom);
         size_t pp = pieceOf.find(suff);
         if (suff != "" && pp != string::npos) {
            PIECES promPiece = PIECES(pp);
            hexBoard->at(hexTo) = promPiece;
            sqsOf[promPiece]->insert(hexTo);
         } else {
            hexBoard->at(hexTo) = piece;
            sqsOf[piece]->insert(hexTo);
         }
      }
      wtm = !wtm;
   }

   void playMove(const string& move) {
      playMove(Move(move2TAN(move)));
   }
};

inline string playAIFfile(const string& fileName) {
   int count = 0;
   int turn;
   ifstream AIF(fileName.c_str());
   ostringstream OUT;
   string line;
   string movePlayed, engineMove, fen;
   Position p;
   while (getline(AIF,line)) {
      if (line.find("GameID ") != string::npos) {  //begin new game
         OUT << endl << endl << line << endl;
         turn = 9;
         while (getline(AIF,line)) {
            if (line.find("MovePlayed ") != string::npos) {
               movePlayed = IRutil::fetchField(line);
            } 
            if (line.find("EngineMove ") != string::npos) {
               engineMove = IRutil::fetchField(line);
            }
            if (line.find("FEN ") != string::npos) {
               fen = IRutil::fetchField(line);
               p.setFromFEN(fen);
               p.playMove(movePlayed);
               OUT << fen << endl << turn << ". " << movePlayed << " ";
               break;
            }
         }
      } else if (line.find("MovePlayed ") != string::npos) {
         movePlayed = IRutil::fetchField(line);
         count++;
         if (p.wtm) { 
            turn++;
         }
         string tanMove = p.move2TAN(movePlayed);
         string uciMove = Move(tanMove).TAN2UCI();
         string sanMove = Move(tanMove).TAN2SAN();
         string lanMove = Move(tanMove).TAN2LAN();
         string move = (count % 4 == 0 ? movePlayed 
                          : (count % 4 == 1 ? lanMove
                               : (count % 4 == 2 ? tanMove : uciMove)));
         if (p.wtm) { OUT << turn << ". "; }
         string actualTAN = p.move2TAN(move);
         p.playMove(move);
         OUT << lanMove << " ";
      }
   }
   OUT << endl;
   return OUT.str();
}


#endif
 
