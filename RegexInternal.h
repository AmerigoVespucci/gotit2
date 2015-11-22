enum RegexDir {
	rdtUp,
	rdtDown,
	rdtLeft,
	rdtRight,
	rdtHoriz,
	rdtVert,
	rdtAny
};

enum RegexElType {
	ctMatchSyms,
	ctRange,
};

enum RegexSideEffectType {
	rsetNone,
	rsetSetFV,
	rsetSetA,
	rsetSetB,
	rsetSetGA,
	rsetIncrA,
	rsetIncrB,
	rsetIncrGA,
	rsetDecrA,
	rsetDecrB,
	rsetDecrGA,
	rsetSetS,
	rsetAddToS,
	rsetSetEnd,
	rsetJumpEA,
	rsetJumpEB,
	rsetExit,
};

struct RegexSideEffect {
	RegexSideEffect(RegexSideEffectType arset, char aParam = 0) {
		rset = arset;
		Param = aParam;
	}
	RegexSideEffect() {
		rset = rsetNone;
		Param = 0;
	}

	bool operator ==(const RegexSideEffect& Other) const;
	RegexSideEffectType rset;
	char Param;
};

const RegexSideEffect crseNone(rsetNone);
struct RegexThread;
struct RegexEl;

struct RegexElChar {
	RegexElChar(char aInputSym, RegexSideEffect rse = crseNone, RegexDir ardt = rdtAny) {
		InputSym = aInputSym;
		InputMax = InputSym;
		SideEffects.clear();
		SideEffects.push_back(rse);
		rdt = ardt;
		ct = ctMatchSyms;
	}
	RegexElChar(char SymMin, char SymMax, RegexSideEffect rse = crseNone, RegexDir ardt = rdtAny)
			: RegexElChar(SymMin, rse, ardt)
	{
		InputMax = SymMax;
		ct = ctRange;
	}
	RegexElChar(char SymMin, char SymMax, RegexSideEffect rse1, RegexSideEffect rse2, RegexDir ardt) 
			: RegexElChar(SymMin, rse1, ardt)
	{
		InputMax = SymMax;
		ct = ctRange;
		SideEffects.push_back(rse2);
	}
	RegexElChar(char SymMin, RegexSideEffect rse1, RegexSideEffect rse2, RegexDir ardt) 
			: RegexElChar(SymMin, rse1, ardt)
	{
		SideEffects.push_back(rse2);
	}

	void AddSideEffect(RegexSideEffect rse) {
		SideEffects.push_back(rse);
	}
	bool operator ==(const RegexElChar& Other) const;
	bool EqualsCore(const RegexElChar& Other) const; //minus SE
	char InputSym;
	char InputMax;
	RegexElType ct;
	RegexDir rdt;
	vector<RegexSideEffect> SideEffects;
};

struct RegexStackEl {
	RegexStackEl(RegexEl * apRule) {
		pCurrEl = apRule;
		OptionCount = 0;
		CurrRepeat = 0;
		MatchSymsDone = 0;
		CurrAlive = 0;
		NumContsDone = 0;
		bRangeDone = false;
		bDoingOptions = false;
		bDoingConts = false;
	}
	uint MatchSymsDone;
	bool bRangeDone;
	RegexEl * pCurrEl;
	uint OptionCount;
	int CurrRepeat;
	uint CurrAlive;
	bool bDoingOptions;
	int NumContsDone;
	bool bDoingConts;
	vector<RegexThread> AliveThreads;
	vector<RegexThread> PausingThreads;
	vector<RegexThread> StackedThreads;
	vector<RegexThread> CompletedThreads;
};

//struct BoardDelta {
//	BoardDelta(int ax, int ay, char Val) {
//		x = ax; y = ay; NewVal = Val;
//	}
//	int x;
//	int y;
//	char NewVal;
//};
//

struct RegexRule {
	RegexRule() {
		bUseGlobalState = false;
		bDebug = false;
	}
	void Init() { // called when the var needs to be reused
		CompletedThreads.clear();
		RuleExecPos.clear();
		pRule = NULL;
		StateTbl.clear();
		bUseGlobalState = false;
		bDebug = false;
	}
	vector<vector<char> > OldTbl;
	vector<RegexThread> CompletedThreads;
	vector<RegexStackEl> RuleExecPos;
	RegexEl * pRule;
	int TblSize;
	int regGA;
	vector<vector<char> > StateTbl;
	bool bUseGlobalState;
	bool bDebug;

	int OnePosExec(RegexEl * apRule, int TblSize, char * pOldTbl, int x, int y,
					bool bEndReq = false, int xEnd = 0, int yEnd = 0);
	int TestThreadsOnRealTbl(char * pNewTbl);
	void CreateMoveDeltas(vector<OneMove >& AllMoveOpts);
};

struct RegexThread {
	RegexThread() {
		bStart = false;
	}
	//RegexRule * pOwner;
	vector<vector<char> > StateTbl;
	vector<BoardDelta> FVRecs;
	int x;
	int y;
	int A;
	int B;
	pair<int, int> End; // the end point for a two-click criterion
	int Score;
	bool bStart;
};

enum RegexPred {
	rptNone,
	rptEQ,
	rptNE,
	rptGT,
	rptGE,
	rptLT,
	rptLE,
	rptEQx,
	rptEQy,
};

struct RegexCond {
	RegexCond(RegexPred arpt = rptNone, char aVal = 0) {
		rpt = arpt; Val = aVal;
	}
	bool operator ==(const RegexCond& Other) const;
	RegexPred rpt;
	char Val;
};

struct  RegexEl{
	static int CreateCount;
	//RegexElType ct;
	vector<RegexElChar> MatchChars;
	//uchar RangeMin;
	//uchar RangeMax;
	vector<RegexEl> Options;
	int MinRepeat;
	int MaxRepeat;
	//RegexEl Cont;
	vector<RegexEl> Cont;
	vector<RegexCond> Cond;
	bool bGlobalStateTbl;
	bool bDebug;


	RegexEl() {
		CreateCount++;
		MatchChars.clear();
		Options.clear();
		MinRepeat = 0;
		MaxRepeat = 0;
		bGlobalStateTbl = false;
		bDebug = false;
	}
	RegexEl(const vector<RegexElChar>& aMatchChars) : RegexEl() {
		MatchChars = aMatchChars;
	}
	RegexEl(const RegexElChar& aMatchChar)  : RegexEl()  {
		MatchChars = vector<RegexElChar>(1, aMatchChar);
	}
	// A constructor for creating a limited version
	// of the same rule that identifies only the beginning of a match
	//RegexEl(RegexEl *pFullRule, int CharLimit, bool bOneLevelOnly = false);
	~RegexEl() {
		CreateCount--;
		//for (uint iOption = 0; iOption < Options.size(); iOption++) {
		//	delete Options[iOption];
		//}
		Options.clear();
		//if (Cont != NULL) {
		//	delete Cont;
		//}
	}
	void AddOption(const RegexEl& pRule);
	void SetOptionRepeats(int MinR, int MaxR);
	void SetCont(const RegexEl& RuleForCont);
	void AddCond(const RegexCond& aCond);
	void PrintDir(string& sOut, RegexDir rdt);
	void PrintSideEffect(string& sOut, RegexSideEffectType rset, char Param);
	void PrintCond(string& sOut, const RegexCond& Cond);
	void Print(string& ret);
	bool Equals(RegexEl& Another, bool& SEEquals);
	void SetStateTblGlobal() { bGlobalStateTbl = true; }
	//int OnePosExec(int TblSize, char * pOldTbl, char * pNewTbl, char * pStateTbl, int x, int y);
	//int Exec(int TblSize, char * pOldTbl, char * pNewTbl);
	//int Exec();
};

struct HeadRegexEl : public RegexEl {
	bool bValid;	
	vector<RegexCond> StartCond; 

	HeadRegexEl() : RegexEl() {
		bValid = true;
	}
	HeadRegexEl(const vector<RegexElChar>& aMatchChars) : RegexEl(aMatchChars) {
		bValid = true;
	}
	HeadRegexEl(const RegexElChar& aMatchChar)  : RegexEl(aMatchChar)  {
		bValid = true;
	}
	void AddStartCond(const RegexCond& aCond);
	void Print(string& ret);
	
};

class RegexRuleProvider : public RuleProviderParent {
	
	vector<HeadRegexEl>* pRules;
public:
	RegexRuleProvider(vector<HeadRegexEl>* apRules) {
		pRules = apRules;
	}
	vector<HeadRegexEl>* GetRules() { return pRules; }
};
extern RegexEl OEl;

