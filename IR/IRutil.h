//File-------------------------IRutil.h-------------------------------------- 
 
#ifndef __IR_UTIL_H__ 
#define __IR_UTIL_H__ 
 
namespace IRutil {    
 
   const string DASHLINE = 
   "------------------------------------------------------------------------"; 
   const string DASHES = "----------";
   const size_t MAXDEPTH = 99;   //exact value almost immaterial, used for only 1 object
   const size_t MAXNUMLEGALMOVES = 100;  //ditto
   //const int NCUTOFF = 64;               //ditto
   const int NCUTOFF = 50;
   const int MATEVALCP = 100000;
   const double MATEVAL = 1000.0;
   const int NAVAL = 100001;          //indicates analysis value not available
   const int PRUNVAL = 100002;
   const int NRECVAL = 100003;
   const int DEFAULTDEPTH = 17;       //prefer 19 or 20 or *; 13 was for Rybka

   static int turnsMade = 0;
   static int turnsDeleted = 0;
   static int decsMade = 0;
   static int decsDeleted = 0;
   static int tuplesMade = 0;
   static int tuplesDeleted = 0;
   static int perfDataMade = 0;
   static int perfDataDeleted = 0;
   const int TUPLEMODULUS = 50000000;
   const int LINESMODULUS = 100000;
   const int TURNSMODULUS = 50000;

   
   const int playedMoveNotInTopN = NCUTOFF;    //should be unused?
   const double DELTACAP = 10.00;     //Perl script's default is 10.00, but prefer 7.00 or 9.00?
   const double PRUNCAP = 9.00;      //use 4.00 for Rybka
   //const double EVALMEMOCAP = 5.00;   //!!! change to 5.0 after debugging curves
                                      //would use 10.0 or higher with unscaled deltas
   //const double MEMO_DENSITY = 100.0;   //allows extra precision place on scaled deltas
   //const size_t MEMO_SIZE = size_t(MEMO_DENSITY * IRutil::EVALMEMOCAP) + 1;

   const double MINGTZERO = 0.00000001; //any smaller number is considered zero 
   const double MINPROB = MINGTZERO;   
   const double P0TOLERANCE = min(0.999999, 1.0 - MINPROB); 
   const double INVVARP0CAP = 100.0; 
     //maximum value for probs[0]---IMPT: = initial Newton point for vectorRoot 
   const double SUMPROBSTOLERANCE = (1 - P0TOLERANCE) + MINPROB*MAXNUMLEGALMOVES/5; //divide latter by 10? 
   const double SHOUTTOLERANCE = 1.005;
   const double TOPMOVETOLERANCE = 0.00001;  //allows for after scaling, less than it means 0.00 
   const double LINESCALEEPSILON= 0.005;

   const double CVTOLERANCE = 0.001;  //lower coeffs of variation treated as 0
   const double VRPREC = MINPROB;  //for VectorRoot 
   const double MAXINV = 1/MINPROB; 
   const int VRDEPTH = 32;       //for debugging Newton's method. 
    
   const int AIFoffset = 1;      //AIF begins indexing depths at 1
   static ios::fmtflags URFORMAT = cout.flags(); 
   const string IRCOMMENT = "//IR "; 
   const string LOGFILENAME = "IRcommandLog.txt"; 
   const string SESSIONFILENAME = "IRsessionData.txt"; 
   const string DEMOFILENAME = "IRdemos.txt"; 

   const double CENTRALPATCHPOWER = 0.58;  //was 0.5806
    
   //--------------------------String Utilities-------------------------------- 

   static set<string> PROBLEM_FENS;
   static set<string> PROBLEM_GAMES;

   inline int ord(char c) { return (unsigned char)c; }
   inline char chr(int n) {
      return (unsigned char)n;
   }
    
   inline void finishLine(istream& IN) { 
     //after www.daniweb.com/forums/thread90228.html 
     IN.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
   } 
    
   inline string trim(const string& st) { 
      int left = 0; 
      int right = st.size() - 1; 
      while (left <= right && isspace(st[left])) { 
         left++; 
      } 
      while (right >= left && isspace(st[right])) { 
         right--; 
      } 
      return st.substr(left,right-left+1); 
   } 
   inline char makeUpper(char c) {
      return ('a' <= c && c <= 'z' ? chr(ord(c) - 32) : c);
   }
   inline char makeLower(char c) {
      return ('A' <= c && c <= 'Z' ? chr(ord(c) + 32) : c);
   }

   inline string fetchField(const string& line) {  //finds first str in "str"
      size_t qstart = line.find('\"');
      size_t qend = line.find('\"',qstart+1);
      return ((qstart != string::npos && qend != string::npos) ?
               IRutil::trim(line.substr(qstart+1,qend-qstart-1)) : "");
   }

 
   inline string getBetweenDelm(const string& st, char begDel, char endDel) { 
      int delmStart = st.find(begDel); 
 
      int delmEnd = st.find(endDel,delmStart+1); 
 
      if ((delmStart == -1) || (delmEnd == -1)) 
         return ""; 
 
      return IRutil::trim(st.substr(delmStart+1, delmEnd-delmStart-1)); 
   } 
    
 
   inline bool isIRComment(const string& st) { return trim(st) == trim(IRCOMMENT); } 
    
   /** Must be called on strings of length2 only. 
    */ 
   inline bool isInNumber(char c, char prevChar) { 
      return isdigit(c)  
          || (c == '.' && isdigit(prevChar)) 
          || (c == 'e' && isdigit(prevChar)) 
          || ((c == '+' || c == '-') && (prevChar == 'e' || prevChar == 'E')); 
   } 
    
   /** Finds legal doubles embedded in lines.   
       TRICK: must regard e.g. "3--4" as 3 and 4, not 3 and -4. 
    */ 
   inline vector<double> extractDoubles(const string& line) { 
      vector<double> nums(0); 
      bool inNumFlag = false; 
      int i = 0; 
      while (i < line.size()) { 
         if (isdigit(line[i]) && !inNumFlag) { 
            int j = i; 
            if (i > 0 && (line[i-1] == '-' || line[i-1] == '+') 
                      && (i == 1 || (line[i-2] != '-' && line[i-2] != '+'))) { 
               j = i - 1;  //number begins with - sign, or with extra + sign. 
            } 
            istringstream istr(line.substr(j)); 
            double x; 
            istr >> x; 
            inNumFlag = true; 
            nums.push_back(x); 
            i++; 
         } else if (inNumFlag) { 
            if (!isInNumber(line[i], line[i-1])) {  //NB: Always i > 0 here. 
               inNumFlag = false; 
            } 
            i++; 
         } else {   //not a digit and not flagged, so look for next number 
            i++; 
         } 
      } 
      return nums; 
   } 
    
   inline string ftoa(const double x, const int places = -1) { 
      ostringstream o; 
      if (places >= 0) {  
         o << fixed << setprecision(max(places,1)) << showpoint;  
      } 
      o << x; 
      return o.str(); 
   } 
    
   inline string itoa(const int x) { 
      ostringstream o; 
      o << x; 
      return o.str(); 
   } 

   inline string itoa(const int x, const int places) {
      ostringstream o;
      o << x;
      string st = o.str();
      for (int i = st.size(); i < places; i++) {
         st = " " + st;
      }
      return st;
   }

    
   inline bool isYes(const string& ans, bool deflt = true) { 
      return (ans.size() == 0 && deflt) 
                || ans.size() > 0 && (ans[0] == 'y' || ans[0] == 'Y'); 
   } 

   inline bool isFilled(const string& ans, bool deflt = true) {
      return (ans.size() == 0 && deflt)
                || ans.size() > 0 && (ans[0] != '?');
   }
    
   /** Match if arg occurs in st surrounded by ends or non-alpha chars. 
       Intent is e.g. for "Karp" to match "Karp, R" but not "Karpov". 
       Works even if arg itself has non-alpha characters. 
    */ 
   inline bool delimitedMatch(const string& arg, const string& st) { 
      int fromPos = st.find(arg); 
      while (fromPos != string::npos) { 
         bool leftOK = (fromPos == 0 || !isalpha(st[fromPos-1])); 
         int rightEdge = fromPos + arg.size(); 
         bool rightOK = (rightEdge == st.size() || !isalpha(st[rightEdge])); 
         if (rightOK && leftOK) { 
            return true; 
         } else { 
            fromPos = st.find(arg,fromPos+1); 
         } 
      } 
      return false; 
   } 

   inline map<string,double> parseStarredPairs(string pairs) {
      map<string,double> ret;
      size_t pos = pairs.find("(*");
      while (pos != string::npos) {
         size_t pos2 = pairs.find("*,");
         string key = pairs.substr(pos+2,pos2-pos-2);
         size_t pos3 = pairs.find(")");
         double value = atof(pairs.substr(pos2+2,pos3-pos2-2).c_str());
         ret[key] = value;
         pairs = pairs.substr(pos3+1);
         pos = pairs.find("(*");
      }
      return ret;
   }
    
   //----------------Calendar and Time Utilities--------------------- 
    
   enum DATE_FORMAT {  
      ARENA_FORMAT,      //format of the Arena 1.99b5 GUI, yyyy.mm.dd 
      NUM_FORMATS 
   }; 
    
   class SimpleDate { 
      int year; 
      int month; 
      int day; 
    public:  
      SimpleDate(int yr, int mo, int dy) : year(yr), month(mo), day(dy) { } 
      SimpleDate(const string& st, DATE_FORMAT format) { 
         string st1 = trim(st); 
         switch(format) { 
          case ARENA_FORMAT: 
            { 
               int firstDotPos = st1.find('.'); 
               int secondDotPos = st1.find_last_of('.'); 
               year = atoi((st1.substr(0, firstDotPos)).c_str()); 
               month = atoi((st1.substr(firstDotPos + 1,  
                                secondDotPos - firstDotPos - 1)).c_str()); 
               day = atoi((st1.substr(secondDotPos + 1)).c_str()); 
            } 
            break; 
          default: 
            cerr << "Unparsable date!" << endl; 
         } 
      } 
    
      int mangledDate() const { 
         return 10000*year + 100*month + day; 
      } 
      int getYear() const { 
         return year; 
      } 
      operator string() const { 
         return itoa(year) + '.' + itoa(month) + '.' + itoa(day); 
      } 
   }; 
    
   inline string getTime() { 
      time_t currentTime = time(NULL); 
      string timeOut = ctime(&currentTime); 
      return trim(timeOut); 
   } 
    
   inline string getRawTime() { 
      ostringstream o; 
      o << time(NULL); 
      return o.str(); 
   } 
    
   /** Hack found on Net to kill "cout" when instream is not "cin". 
       Took out "std::ios(&m_sbuf)," from constructor. 
    */ 
   struct nullstream: std::ostream { 
      struct nullbuf: std::streambuf { 
         int overflow(int c) { return traits_type::not_eof(c); } 
      } m_sbuf; 
      nullstream(): std::ostream(&m_sbuf) {} 
   }; 

   /** Get current working directory, by user Erroneous at 
       http://stackoverflow.com/questions/2203159/is-there-a-c-equivalent-to-getcwd
    */
   static string getmycwd() {
      char *buffer = new char[MAXPATHLEN];
      getcwd(buffer,MAXPATHLEN);
      if (buffer != NULL) {
         string ret(buffer);
         delete[] buffer;
         return ret;
      } else {
         return string();
      }
   }
    
    
   //---------------Numerical Utilities--------------------------------- 

   inline int sgn(int x) { return (x > 0) - (x < 0); }
    
   inline double sqr(const double x) { return x*x; } 
 
   inline double cube(const double x) { return x*x*x; } 
 
   inline double divifgt0(const double x, const double y) {
      return (y > 0.0 ? x/y : (fabs(x) < MINPROB ? 0.0 : 1.0/MINPROB));
   }

   inline double divifne0(const double x, const double y) {
      return (y != 0.0 ? x/y : (fabs(x) < MINPROB ? 0.0 : 1.0/MINPROB));
   }

   inline double log2(double x) { 
      return log(x)/log(2.0); 
   } 

   inline double arcsinh(double x) {
      return log(x + sqrt(1.0 + x*x));
   }
 
   inline double round(const double x, int place = 0) { 
      double by = 1.0; 
      int i = 0; 
      while (i < place) { by *= 10; i++; } 
      int rounded = (int)(by*x + 0.5); 
      return rounded / by; 
   } 
 
   /** Given y = x*ln(x), compute x to high precision.  Code from the book 
       W.H Press et al., "Numerical Recipes, The Art of Scientific Computing 
       (3rd ed.)", C.U.P. 2007, section 6.11. 
    */ 
   inline double invxlnx(const double y) { 
      double t0 = 0; 
      double t; 
      const double ie = 0.367879441171442322; 
      if (y >= 0 || y <= -ie) { 
         cerr << "Invalid argument for primary branch of invxlnx." << endl; 
         return -100000000; 
      } 
      double u = (y < -0.2) ? log(ie - sqrt(2*ie*(y + ie))) : -10.0; 
      do { 
         t = (log(y/u) - u)*(u/1.0 + u); 
         u += t; 
         if (t < 1.0e-8 && fabs(t + t0) < 0.01*fabs(t)) { 
            break;  //exits do-while loop 
         } 
         t0 = t; 
      } while (fabs(t/u) > 1.0e-15); 
      return exp(u); 
   } 

 
   /** Random number between a and b-1, using built-in rand() but avoiding 
       low-order pitfalls, see http://members.cox.net/srice1/random/crandom.html 
    */ 
   inline const int myRand(const int a, const int b) { 
      const int m = b - a;  //now generate random in 0...m-1 
      const int y = int((double(rand())/(double(RAND_MAX) + 1.0)) * m); 
      return y + a; 
   } 
 
   /** Randomly shuffle indices a .. b-1 of the array arr 
       Based on http://www.algoblog.com/2007/06/04/permutation/ via myRand 
       Preferred to STL's "shuffle" to make reproducibility more transparent; 
    */ 
   template<class E> 
   inline void randShuffle(vector<E>* arr, const int a, const int b) { 
      for (int i = a + 1; i < b; i++) { 
         int j = myRand(a, i+1);  //random index in a .. i 
         E temp = arr->at(i); 
         arr->at(i) = arr->at(j); 
         arr->at(j) = temp; 
      } 
   } 
    

//-----------------------Vector utilities---------------------------

   /** Return argv[left .. right], REQ left <= right <= argv.size().
    */
   inline double fracsum(const vector<double>& argv, double left, double right) {
      size_t li = size_t(left);
      size_t ri = size_t(right);
      if (li == ri) {   //arguments within same cell
         return argv[li]*(right - left);
      } //else
      double lfp = 1.0 - (left - li);
      double rfp = right - ri;
      double sum = lfp*argv[li];
      //The "gotcha": adding rfp*argv[ri] might blow when right == argv.size()
      if (right < argv.size()) {   //note: compares double to size_t/int
         sum += rfp*argv[ri];
      }
      for (size_t i = li+1; i < ri; i++) {  //INV: li < i < ri
         sum += argv[i];
      }
      return sum;
   }


   inline vector<double> redist(const vector<double>& argv, size_t newSize) {
      double r = double(newSize)/argv.size();
      vector<double> result(newSize);
      for (size_t i = 0; i < newSize; i++) {
         result[i] = fracsum(argv,r*i,r*i + r);
      }
      return result;
   }

   inline void normalize(vector<double>& argv) {
      double sum = 0.0;
      for (size_t i = 0; i < argv.size(); i++) { sum += argv[i]; }
      for (size_t j = 0; j < argv.size(); j++) { argv[j] /= sum; }
   }

   inline void normalizeP(vector<double>* argv) {
      double sum = 0.0;
      for (size_t i = 0; i < argv->size(); i++) { sum += argv->at(i); }
      for (size_t j = 0; j < argv->size(); j++) { argv->at(j) /= sum; }
   }




   /** Inverts positive values in a vector, capping inverses at MAXINV to 
       ensure numerical stability of routines using these values as powers. 
    */ 
   inline vector<double> anvert(vector<double> args, const double a = 0.0) { 
      for (int i = 0; i < args.size(); i++) { 
         args[i] = args[i] > 1.0/MAXINV ? (a + 1.0 + a*args[i])/args[i] : MAXINV ; 
      } 
      return args; 
   } 
    
   /** Sum over i of x^{powers[i]}.  Not used outside this namespace 
    */ 
   inline double powSum(const double x, const vector<double>& powers) { 
      double sum = 0.0; 
      for (int i = 0; i < powers.size(); i++) { 
         sum += pow(x,powers[i]); 
      } 
      return sum; 
   } 
    
   inline double derive(const double x, const vector<double>& powers) { 
      double sum = 0.0; 
      for (int i = 0; i < powers.size(); i++) { 
         double summand = powers[i]*pow(x, powers[i] - 1); 
    
   // if (isnan(summand)) { 
   //    cerr << "Trouble x = " << x << ", powers[" << i << "] = " << powers[i] 
   //         << endl; 
   // } 
         sum += summand; 
      } 
      return sum; 
   } 
    
   static int superMaxNewtons = 0;  //over entire session 
   static int maxNewtons = 0;       //over one fit run 
   static int superNumNewtons = 0; 
   static int numNewtons = 0; 
   static int superNumVRCalls = 0;  //over session 
   static int numVRCalls = 0;       //over fit run 
   static double maxRoot = 0.0;     //how close to 1 does the root come? 

   static int DIcount = 0;
   static int numResets = 0;
    
    
   /** Given g > 0 and e_1,...,e_n, find x s.t. x^{e_1} + ... + x^{e_n} = g, 
       to precision prec.  When n=1, b = g^{1/e_1}, hence name "vector root". 
       PRE: 0 < g < n, so that 0 < x < 1, and e_1,...,e_n > 1, so that the 
       second derivative is always positive, making the function f concave. 
       This makes Newton's method particularly easy: All values are > g, so 
       subtracting "prec" from the next point p makes "f(p-prec) <= g" a 
       natural and efficient stopping condition that still achieves precision 
       prec, speeds convergence, & lessens high-derivative trouble near p = 1. 
       Caller guards against numerical extremes. 
    */ 
   inline double vectorRoot(const vector<double>& powers, const double goalVal, 
                            const double prec = VRPREC) { 
      numVRCalls++; superNumVRCalls++; 
      int count = 1; 
      double arg = IRutil::P0TOLERANCE; //max OK value for root, <= 1 - prec. 
      double val, deriv; 
      while ((val = powSum(arg, powers)) > goalVal) { 
         count++; 
         deriv = derive(arg, powers); 
         arg = arg - (val - goalVal)/deriv - prec; 
      } 
      numNewtons += count; superNumNewtons += count; 
      maxNewtons = max(maxNewtons,count); 
      superMaxNewtons = max(superMaxNewtons,count); 
      maxRoot = max(maxRoot,arg); 
      return arg; 
   } 
    
   /** Binary-search version, for comparison 
    */ 
   inline double vectorRootBS(const vector<double>& powers,  
                              const double goalVal, 
                              const double prec = VRPREC) { 
      double hi = IRutil::P0TOLERANCE; 
      double lo = 0.0; 
      double mid; 
      while (hi - lo > prec) { 
         mid = (hi + lo)/2.0; 
         (powSum(mid,powers) < goalVal ? lo : hi) = mid;  //yes, that's cutesy 
      } 
      return (hi + lo)/2.0; 
   } 


    
    
    
   // Alternate way of computing vectorRoot.  Not used because its 
   // numerical instability is even worse...
    
   inline double powSum2(const vector<double>& bases, const double b) { 
      double sum = 0.0; 
      for (int i = 0; i < bases.size(); i++) { 
         sum += pow(bases[i], b); 
      } 
      return sum; 
   } 
    
    
   /** Given g > 0 and e_1,...,e_n, find b such that e_1^b + ... + e_n^b = g, 
       provided lo <= b <= hi, to precision 1/2^d.  When n=1,  
       b = (ln g)/(ln e) = log_e(g), so we can call this the  
       "vector log" b = log_(e_1,...,e_n)(g).  Alg. uses binary search. 
    */ 
   inline double vectorLog(const vector<double>& bases, 
                           const double g, const double prec = VRPREC) { 
      double lo = 0.0, hi = 1.0; 
      while (powSum2(bases,hi) < g) { 
         lo = hi; 
         hi *= 2; 
      } 
      double mid = (lo + hi)/2.0; 
      while (mid - lo > prec) { 
         if (powSum2(bases,mid) > g) { 
            hi = mid; mid = (lo + hi)/2.0; 
         } else { 
            lo = mid; mid = (lo + hi)/2.0; 
         } 
      } 
      return mid; 
   } 
 
   inline int factorial(int x) { 
      return (x <= 1) ? 1 : x * factorial(x - 1); 
   } 

   inline double vectorRoot2(const vector<double>& powers, 
                             const double g, const double prec = VRPREC) { 
      vector<double> bases(powers.size()); 
      for (int i = 0; i < bases.size(); i++) { 
         bases[i] = exp(powers[i]); 
      } 
      return exp(vectorLog(bases,g,prec)); 
   } 

   /** Generate Poisson dist. over integers 0..numWeights-1 with Lambda = dpeak 
       If dpeak == 0.0 then return vector with 1 at last position, 0 elsewhere. 
       When 
    */ 
   //vector<double> poissonWeights (double dpeak, size_t numWeights)  { 
   inline void genPoissonWeights(const double& lambda, const size_t& numWeights,
                                 vector<double>& depthWts) {
      //vector<double> depthWts(numWeights); 

      if (lambda == 0.0) { 
         depthWts.at(0) = 1.0; 
         for (size_t k = 1; k < numWeights; k++) { depthWts.at(k) = 0.0; } 
         return; // depthWts; 
      } //else 
      double cume = 0.0; 
      double pval = exp(-lambda);
      depthWts.at(0) = cume = pval;
      for(size_t i = 1; i < numWeights; i++) { 
         pval = pval*lambda/i;
         //pval = pow(lambda,double(i))*exp(-lambda)/IRutil::factorial(i); 
         cume += pval; 
         depthWts.at(i) = pval; 
      } 
      //Lump remaining Poisson distribution onto highest depth 
      size_t j = numWeights; 
      while (cume < 1.0 - IRutil::SUMPROBSTOLERANCE && j <= IRutil::NCUTOFF) { 
         //pval = pow (lambda,double(j))*exp(-lambda)/IRutil::factorial(j); 
         pval = pval*lambda/j;
         cume += pval; 
         depthWts.at(numWeights - 1) += pval; 
         j++; 
      } 
      if (j == IRutil::NCUTOFF+1) { cerr << "Normalization failure\n"; } 
/*
cerr << "Poisson weights: ";
for (int i = 0; i < depthWts.size(); i++) {
   cerr << i << "-" << IRutil::ftoa(depthWts.at(i), 4) << " ";
}
cerr << endl;
*/
   } 

   /** Modified Poisson distribution with coefficient of variation ~= v.
       v = 0 means all weight on peak; large v means completely flat;
       v = 1 gives ordinary Poisson distribution.  REQ: dpeak < last
       NOT USED---replaced by a simple dilation by v.
    */
/*
   vector<double> poissonWeights(double dpeak, size_t numWeights, double v) {
      if (v > 1.0/IRutil::CVTOLERANCE) {   //return twin peaks near dpeak
         vector<double> depthWts(numWeights, 0.0);
         size_t b = size_t(dpeak);
         if (b >= numWeights) {
            depthWts.at(numWeights-1) = 1.0;
         } else {
            depthWts.at(b-1) = dpeak - double(b);
            depthWts.at(b) = 1.0 - depthWts.at(b-1);
         }
         return depthWts;
      } else if (v > double(numWeights - 1)) {
          vector<double> depthWts(numWeights, 1/double(numWeights));
          return depthWts;
      }
      //else
      //size_t vd = size_t(dpeak/v + 0.5);
      //double ratio = vd/double(last);

      size_t expandTo = size_t(numWeights/v + 0.5);
      vector<double> vr = poissonWeights(dpeak/v, expandTo);
      return redist(vr, numWeights);
   }
*/
 
 
//-----------A useful pair structure for probability endpoints---------------- 
 
   struct Endpt { 
      double val; 
      double partner;   //INV: val != partner 
      double weight; 
      Endpt(double v, double p, double w) : val(v), partner(p), weight(w) { } 
      bool isP() const { return val < partner; } 
   }; 
   inline bool endPtLT(const Endpt& a, const Endpt& b)  {   // comparison function 
      return a.val < b.val; 
   } 
 
 
 
};                          //end of IRutil namespace 
 
#endif                      //end of #ifndef __IR_UTIL_H__ 
 
 
 
