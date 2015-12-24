//#define GOTIT_LINUX
#include <cstdlib>
#include <iostream>
//#include <minmax.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <functional>      // For greater<int>( )
#include <algorithm>    // std::sort
#include <cctype>
#include <string>
#include <map>
#include <vector>
#ifdef GOTIT_LINUX
#include <boost/filesystem.hpp>
#else
#include <filesystem>
#endif
#include <deque>
#include <set>
#include <float.h>
#include <math.h>
#include <assert.h>
#define RAPIDXML_STATIC_POOL_SIZE (16 * 1024)
#include "RapidXml/rapidxml.hpp"

using namespace rapidxml;
using namespace std;

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long long u64;

typedef unsigned char uchar;

#define MASC

#ifdef MASC
#define PENN_SUFFIX "-penn.xml"
#else
#define PENN_SUFFIX "-hepple.xml"
#endif
#define SRC_PATH "C:\\MASC-3.0.0\\data\\written\\fiction\\"
#define MODULE "The_Black_Willow"

#ifdef GOTIT_LINUX
#define CODE_SRC "./"
//#define GOTIT_ROOT "/media/abba/Seagate Backup Plus Drive/guten/"
#define GOTIT_ROOT "/guten/"
#define GRPS_ROOT GOTIT_ROOT "GrpsRoot"
#define GRPS_TEMP GOTIT_ROOT "state/GroupsDataTemp.bin"
#define GRPS_DATA GOTIT_ROOT "state/GroupsData.bin"
#define GRPS_FNAME GOTIT_ROOT "state/Groups.bin"
#define GRPS_FNAME_NAME GOTIT_ROOT "state/GrpsFile.bin"
#define LOGS_COUT GOTIT_ROOT "Logs/Cout"
#define NAMED_COUNT_FNAME GOTIT_ROOT "state/NamedCounts.bin"
#define NLP_OUT GOTIT_ROOT "NLPOut"
//#define NLP_OUT GOTIT_ROOT "NLP36Out"
#define FILES_EX GOTIT_ROOT "GotitFilesEx"
//#define FILES_EX GOTIT_ROOT "NLP36Out"
#else
#define CODE_SRC "M:\\MascReader\\"
#define GRPS_ROOT "D:\\guten\\GrpsRoot"
#define GRPS_TEMP "D:\\guten\\state\\GroupsDataTemp.bin"
#define GRPS_DATA "D:\\guten\\state\\GroupsData.bin"
#define GRPS_FNAME "D:\\guten\\state\\Groups.bin"
#define GRPS_FNAME_NAME "D:\\guten\\state\\GrpsFile.bin"
#define LOGS_COUT "D:\\guten\\Logs\\Cout"
#define NAMED_COUNT_FNAME "D:\\guten\\state\\NamedCounts.bin"
#define NLP_OUT "D:\\guten\\NLPOut"
#define FILES_EX "D:\\guten\\GotitFilesEx"
#endif

#define LOW_ASCII 0x0
#define LOW_TEST	7
#define UTF_TEST	6
#define UTF_START	(uchar)0x3 // 11
#define UTF_CONT	(uchar)0x2 // 10

#define FWRITE(fout, val, nb) fout.write((char *)&val, sizeof(val)); nb += sizeof(val)

class CGotitEnv;
class CPatGrpWrapper;
class CPatGrpHolder;
class CPatGrpMgr;
struct SWordData;

struct NameItem {
	NameItem() { Count = 0; }
	NameItem(char * aName) { Name = aName; Count = 0; }
	NameItem(const char * aName) { Name = aName; Count = 0; }
	NameItem(string& aName) { Name = aName; Count = 0; }
	NameItem(string aName) { Name = aName; Count = 0; }
	NameItem(string& aName, int aCount) { Name = aName; Count = aCount; }
	NameItem(const string& aName, int aCount) { Name = aName; Count = aCount; }

	string Name;
	int Count;
};

struct STextRegion {
	//STextRegion(char * aName, int iStart, int iEnd);
	static bool lt(STextRegion* r0, STextRegion * r1) {
		return (r0->TextIDStart < r1->TextIDStart);
	}
	string Name;
	string Word;
	string POSRef; // ref to PO S rec
	int TextIDStart;
	int TextIDEnd;
};

typedef map<string, STextRegion> MapIDToTextRegion;

struct SPOSRec {
	string Name;
	string TextRef; //ref to map for text region
	string POS;
	string Word;
	string WordBase;
};

typedef map<string, SPOSRec> MapIDToPOSRec;

// short version of sentence rec for  random forest
struct WordProps {
	string Word;
	string POS;

};
// used for sentence rec
struct WordRec {
	WordRec() {
		bCap = false;
	}
	WordRec(string& aWord) {
		Word = aWord;
		bCap = false;
	}
	string RegionName;
	string Word;
	string WordCore;
	string POS;
	string NER;
	bool bCap; // only true if the first letter only was capitalized. We store in that case as all lowercase
	void Store(ofstream& fout);
	void Load(ifstream& fin);
};

struct DepRec {
	uchar iDep;
	uchar Gov;
	uchar Dep;
	void Store(ofstream& fout);
	void Load(ifstream& fin);
	bool operator == (DepRec& Other);
};

struct CorefRec {
	int SentenceID; // zero based index into SSentenceRec array
	int StartWordID; // zb index into WordRecArray of OneWordRec of SSentenceRec
	int EndWordID; // a coref mention can include a number of words
	int HeadWordId; // refers to the dependencey head of a phrase that makes up the mention
	int GovID; // zb index into the CorefRec array itself. Who is the first mention. Now all will point to him
	void Store(ofstream& fout);
	void Load(ifstream& fin);
	bool operator == (CorefRec& Other);
};


struct SSentenceRec {
	static bool lt(const SSentenceRec& r0, const SSentenceRec& r1) {
		return (r0.TextIDStart < r1.TextIDStart);
	}
	void Store(ofstream& fout);
	void Load(ifstream& fout);
	string Name;
	string Sentence;
	int TextIDStart;
	int TextIDEnd;
	int ParaNum;
	int SentInParaNum;
	string Label;
	vector<WordRec> OneWordRec;
	vector<DepRec> Deps; // not collapsed dependencies
};

typedef map<string, SSentenceRec> MapIDToSentenceRec;

struct SGroupingLink {
	//string Member;
	SGroupingLink() {
		NumOccurs = 1;
		NumExist = 0;
		PctHit = 0.0f;
		bStrong = false;
		SurprisingNess = 0.0f;
	}
	static bool gt(SGroupingLink * r0, SGroupingLink * r1) {
		return(r0->Strength > r1->Strength);
	}
	static bool gtInOccurs(SGroupingLink * r0, SGroupingLink * r1) {
		return(r0->NumOccurs > r1->NumOccurs);
	}
	void Store(ofstream& fout, int& NumBytesWritten);
	void Store(ofstream& fout);
	void Load(ifstream& fout);
	int NumOccurs;
	int NumExist;
	float PctHit;
	float Strength; // strength of signal taking pct and freq ino account
	string W; // repeat of map
	bool bStrong; // included in StrongMembers of CGroupingRec that has the map of these as members
	float SurprisingNess;
};

typedef map<string, SGroupingLink> MapMemberToGroup;
typedef map<string, int> MapModuleNameToIndex;
typedef map<string, int> MapBaseWordToCount;

enum ElType {
	etBaseWord,
	etWord,
	etPOS,
	etUnused, // option added fro readibility
	etGrouping
};

enum StopActionType {
	satStop, // unconditional no include
	satIncludeAndStop, // Even if word matches stop, only stop next word
	satNoStop, // if word matches, the stop condition is ignored
};

ElType itoet(string s);

struct SOnePatternFinderNode {
	SOnePatternFinderNode(	vector<NameItem>& aOptions, string& aanc_grp_name, ElType aSel, ElType aStore, bool ab_oneof, 
					bool abOneItem, StopActionType asat, int aStoreIdx = -1) {
		pOptions = &aOptions;
		anc_grp_name = aanc_grp_name;
		Sel = aSel;
		Store = aStore;
		//		bReset = abReset;
		StoreIdx = aStoreIdx;
		b_oneof = ab_oneof;
		bOneItem = abOneItem;
		sat = asat;
	}
	SOnePatternFinderNode() {
		return;
	}
	vector<NameItem>* pOptions;
	string anc_grp_name; // name of an ancestor group
	ElType Sel;
	ElType Store;
	string sStoreType;
	string sStore;
	string sSel;
	string sSelType;
	string sReq;
	string sReqType;
	int StoreIdx;
	int NodeIdx; 
	bool b_oneof;
	bool bOneItem;
	StopActionType sat;
	//bool bReset;
};

struct SPatFinderGrpParamMatch {
	string sMatchWhat; // currently a value of none (default) or SrcGrpParam which means that sSrcType is "GrpAnc" and we need to match a member/GrpParam of the *source* group
	string sMatchWhich; // which GrpParam if MatchWhat is SrcGrpParam
	string sMatchType; // what are we matching with something in the group: none, constant or GrpParam (last must be a previous param)
	string sMatch; // for constant, a value to match. For GrpParam, an index previous to this one in the newly created group - not in the source group

};

struct SOnePatternFinderGrpParam {
	int idx;
	string sSrcType; // so far: ExtParam, NameListItem, or GrpAnc which is one group descended from the ancestor named
	string sSrc;
	string sSortType; // If we are not taking all the elements/members from a src, what do we sort by, count, strength ...
	string sFillType; // decide how many groups to create based on: percentage, number or all (default)
	string sFillNumber; // for FillType percentage 0-100, for number, the number to take
	string sFillPercentile; // this is not the percent to take. But when using a sort, limits the percentile to the top value here 
							// for example, if this is 5 and SortType is count, we choose FillNumber elements but only from 
							// the top 5 percentile of the sort
	string sMatchWhat; // currently a value of none (default) or SrcGrpParam which means that sSrcType is "GrpAnc" and we need to match a member/GrpParam of the *source* group
	string sMatchWhich; // which GrpParam if MatchWhat is SrcGrpParam
	string sMatchType; // what are we matching with something in the group: none, constant or GrpParam (last must be a previous param)
	string sMatch; // for constant, a value to match. For GrpParam, an index previous to this one in the newly created group - not in the source group
	string sSrcStore; // For group searches, what to take from the group. The default is "word". Alternatives are SrcGrpParam which needs argument in sSrcStoreWhich
	string sSrcStoreWhich;
	vector <SPatFinderGrpParamMatch> MatchList; // Each of these must be matched for each GrpParams. Normally goes through Src Params requiring a match for each

};

// Instead of just using words with similar probability to evaluate the surprise fcator of a grp element, we can use
// a specific other group to evaluate the surprise group

struct SOnePatternFinderSurpriseQ {
	string sGrpAnc; // the pattern finder name for the group we use to calulate surprise
	string sGrpParamMatchType0; // how to match the PatGrpParam of the surprise group. Can be constant or MyGrpParam, which takes the value from the group itself not the surprise evaluation group. Can also be "none" which means all accepted
	string sGrpParamMatchArg0; // either the value of the constant or the index of the GrpParam from the new group
	string sGrpParamMatchType1; // add more of these if necessary. 
	string sGrpParamMatchArg1;
};
class CGroupingRec;

// ancestor template used to create groups
struct SPatternFinder {
	string group_name_template;
	string GrpType; // passed down to descendents
	string SrcTextType;
	vector<SOnePatternFinderNode> node_list;
	vector<SOnePatternFinderGrpParam> GrpParamList;
	SOnePatternFinderSurpriseQ SurpriseGrpFinder; // [rovides name of another group and requirements for its GrpParams 
	int num_stores; // stores the user supplied val, but we can calc
	vector<NameItem>* p_stop_list;
	float create_freq; // if 0 create once on initialize else every module load, chance out of 1 to create
	float run_prob; // every module load chance to run any and all of groups from this pattern
	bool bGrpCreator; // this pattern is used to create multiple groups
	vector<CPatGrpHolder*> ChildPatGrps;
	//vector<CGroupingRec*> ChildGroups;
	vector<CGroupingRec*> MergedGroups;
	int NumStoreMemUs;
	int NumLocalMemUs;
	int NumTotalMemUs; // store + locals
};

// Implementation Parameters

enum ImplemParamType {
	iptConstant,
	iptOptions,
	iptRange,
	iptInvalid,
};

struct SImplemParam {
	SImplemParam() {
		ipt = iptInvalid;
	}
	ImplemParamType ipt;
	string Name;
	string Val;
};

typedef map<string, SImplemParam> MapNameToImplemParam;

struct STaskItem {
	string sDo;
	string sType;
	string sVal;
};
struct STaskList {
	string Name;
	vector<STaskItem> ItemList;
	vector<int> LoopStarts;
};
// data defining one group

static const char * cGrpTypeNormal = "normal";
//static const char * cGrpTypeSeed = "SeedGrp"; // manually confirm that this matches the spelling in the xml file

class CGroupingRec {
public:
	CGroupingRec() {
		total_strength = 0.0;
		NumSearches = 0;
		NumRecsSearched = 0;
		bFromMerge = false;
		GrpType = cGrpTypeNormal;
	}
	void CreateStrengthSort(MapBaseWordToCount& BaseWordCount);
	//void Store(fstream& fout);
	//void Load(	fstream& fin, vector<SPatternFinder >& anc_pat_finder_list,
	//			map<string, CGroupingRec*>& pat_grp_tbl, MapBaseWordToCount& BaseWordCount);
	string Name;
	//MapMemberToGroup MemberLinks;
	//vector<SGroupingLink*> SortedByStrength;
	//vector<SGroupingLink*> StrongMembers;
	SPatternFinder* pfinder;
	vector<string> members;
	float total_strength;
	int NumSearches;
	int NumRecsSearched;
	bool bFromMerge;
	string GrpType;
	vector<string> MergedFromList;

	void StoreNonData(fstream& fout);
	void LoadNonData(fstream& fout);
};

class CPatGrpData {
	friend CPatGrpWrapper;
	friend CPatGrpHolder;

	MapMemberToGroup MemberLinks;
	vector<SGroupingLink*> SortedByStrength;
	vector<SGroupingLink*> StrongMembers;
	//float total_strength;
	CPatGrpHolder * pGrp;

	void CreateStrengthSort(CGotitEnv * apEnv);

public:
	CPatGrpData() { pGrp = NULL; }
	CPatGrpData(CPatGrpHolder * apGrp) { pGrp = apGrp;  }

};

class CPatGrpHolder {
	friend CPatGrpWrapper;
	friend CPatGrpMgr;
	friend CPatGrpData;

	int VersionNum;
	int NumEls;
	CGroupingRec PatGrpRec;
	bool bDataLoaded;
	bool bModified;
	bool bReadyForRemoval;
	int iFileID; // same as string version without the preceeding directories and file extension
	string FileID;
	CPatGrpData * pData;
	CPatGrpMgr * pMgr;
	u64 DataLastKeepAlive;
	int TotalNumOccurs;
	float TotalStrength;
	int NumStrongEls;
	float AvgOccursPerCount; // Average for strong elements of NumOccurs / that word's hit count
	string SurpriseGrpName;
	float FHolder1;
	float FHolder2;
	float FHolder3;
	int IHolder1;
	int IHolder2;
	int IHolder3;
	int DataFilePos;

	bool SaveData(fstream& f, int& NumBytesWritten);
	static bool SaveData(fstream& f, CPatGrpData * pData, int& NumBytesWritten);
	bool LoadData();
	//static bool LoadGrpData(fstream& f);
	static bool LoadData(fstream& f, CPatGrpData * pData);
	bool UnloadData();
	void Store(fstream& f);
	void Load(fstream& f);


public:
	~CPatGrpHolder();
	CPatGrpHolder() {
		bDataLoaded = false;
		bModified = false;
		pData = NULL;
		NumEls = 0;
		bReadyForRemoval = true;
		pMgr = NULL;
	}

};

class CPatGrpIter {
	friend CPatGrpMgr;
	map<string, CPatGrpHolder>::iterator itpgt;
public:
	CPatGrpIter(map<string, CPatGrpHolder>::iterator aitpgt) {
		itpgt = aitpgt;
	}
	CPatGrpHolder * GetHolder();

};

class CPatGrpWrapper {
	CPatGrpHolder * pGrp;

public:
	CPatGrpWrapper(CPatGrpIter& Iter);
	CPatGrpWrapper(CPatGrpHolder * apGrp);
	~CPatGrpWrapper();
	
	CPatGrpHolder* PresentHolder(); // this does not avoid the destructor just allows the creation of another later on

	bool LoadData();
	// bool getGrpParams(vector<string>& Params); unsafe
	int getNumGrpParams();
	void setNumGrpParams(int Size);
	bool setGrpParam(int i, string& val);
	bool getGrpParam(int i, string& val);
	string  getGrpParam(int i);
	void addGrpParam(string& val);
	//bool getGrpElsMap(MapMemberToGroup& ElsMap);
	int getGrpElsMapSize();
	bool addGrpEl(string& ElName, int& NumOccurs);
	bool incrGrpElNumExists(string& ElName);
	SGroupingLink* getGrpElByName(string& W);
	//bool getGrpStrongEls(vector<SGroupingLink*>& StrongEls); // unsafe
	int getNumStrongEls(); // -1 means operation failed
	SGroupingLink* getStrongElByIndex(int iStrongest); // only get from shorted strong list
	SGroupingLink* getGrpElByIndex(int iStrongest); // get all ordered by strength
	bool getGrpElsSortedByStrength(vector<SGroupingLink*>& ElsSortedByStrength);
	bool getGrpTotalStrength(float& Strength);
	bool getGrpFinder(SPatternFinder*& pFinder);
	bool getGrpName(string& Name);
	string getGrpName();
	void setGrpName(string& Name);
	void setFinder(SPatternFinder* pFinder);
	void setNumSearches(int NumSearches);
	void setNumRecsSearched(int NumRecsSearched);
	bool AddEl(string& ElName, SGroupingLink& El);
	bool CreateStrengthSort();
	int getTotalNumOccurs();
	string& getSrprsGrpName();
	void DeleteData();
	int getGrpID(); // Not the string, just a 32 bit ID
	float getGrpAvgOccursPerCount() { return pGrp->AvgOccursPerCount;  }
};

static const int cNumGrpDirLevels = 2;
static const int cGrpMaxFileIDStart = 1;
static const int cLRUForDataRemovalSize = 100;
static const u64 cTimeLapsForDataRemove = 3000000000ll;

class CPatGrpMgr {
public:
	CPatGrpMgr(CGotitEnv * pTheEnv) {
		bInitialLoadDone = false;
		CleaningTimeExtra = 0;
		pEnv = pTheEnv;
		GrpMaxFileID = cGrpMaxFileIDStart;
		bDataAvailForLoading = false;
		bDestructorCalled = false;
	}
	~CPatGrpMgr() {
		cout << "Unloading PatGroupMgr with " << PatGrpTbl.size() << " groups!!\n";
		bDestructorCalled = true;
	}
	static bool CreateFileForGrp(string& FileID, int iFileID);
	static void RemoveGrpWithoutSave(CPatGrpHolder * pHolder);
	CPatGrpIter getStartIterator();
	CPatGrpHolder *  FindGrp(string& GrpName);
	bool getNextIterator(CPatGrpIter& CurIter);
	bool IsIteratorValid(CPatGrpIter& CurIter);
	void Store();
	void Load(vector<SPatternFinder >& anc_pat_finder_list);
	void CreatePatGrp(	string& GrpName, SPatternFinder* pFinder,
						const vector<string>& GrpParams, const string& SrprsGrpName);
	CPatGrpHolder * CreateUnlinkedPatGrp(	string& GrpName, SPatternFinder* pFinder,
											const vector<string>& GrpParams, const string& SrprsGrpName);
	CPatGrpHolder * LinkPatGrp(CPatGrpHolder * phPatGrp); // deletes the object it receives, not including it's pData and returns the new one
	bool TrueUnload(CPatGrpHolder * pHolder);
	void DataAlivePing(CPatGrpHolder * pHolder);
	void NewData(CPatGrpHolder * pHolder);
	void CleanPatGrps();
	void DeletePatGrp(CPatGrpHolder * pHolder);
	void CreateWordDataLinks(map<string, SWordData>& MapWordToData);
	CPatGrpHolder *  FindGrpByFileID(int iGrpFileID);
	fstream& getFStreamForGrp(bool& bAvail);

	static string GetGrpsRootPath() {
		return string(GRPS_ROOT);
	}

	CGotitEnv * pEnv;
	bool bDestructorCalled;

private:
	map<string, CPatGrpHolder> PatGrpTbl;
	map<int, CPatGrpHolder*> PatGrpIDTbl;
	int GrpMaxFileID;
	bool CreateFileIDForGrp(string& FileID, int& iFileID);
	bool bInitialLoadDone;
	u64 CleaningTimeExtra;
	//list<CPatGrpHolder *> LRUForDataRemoval;
	fstream fileDataForLoading;
	bool bDataAvailForLoading;
	int NumGrpsSavedInDataFile;
	vector<u64> SaveTimes;
};



struct s_pattern_group {
	string group_name;
	SPatternFinder* pfinder;

};

struct SGrpSeed {
	//SGrpSeed(SGrpSeed& o) {
	//	*this = o;
	//}
	string SeedName;
	map<int, int> iPrecGrps; // map of index of pos list to index of grp in groups vector
	map<int, int> iFollowGrps;
	vector<string> SortedPeers;
	vector<string> RFPeers; // only those peers participating in RF data
	vector<bool> bOOB; // Specifies according to order of Sorted peers whether OOB or not
	stringstream* pRFData; // data returnd from CreateWordGroup for records for which bOOB is false
	stringstream* pRFDataOOB;
	stringstream* pRFLabel; // labels returnd from CreateWordGroup for records for which bOOB is false
	stringstream* pRFLabelOOB;
};

class CModNames {
	vector<string> ModNames;
	const int cOOBDenom = 3; // 1 in OOBDenom is OOB
	int iSeq;

public:
	void LoadModuleNames(	string PathName, bool bIfExt, string FindExt, 
							bool bRemoveExt, string NewExt, int Limit = -1);
	string& GetRandModName(bool bOOB);
	uint getSize();
	bool InitSeq() { iSeq = 0; return (ModNames.size() > 0); } // return true if valid to access
	bool GetSeqModName(string& ModNameRet); // returns true if last

};

typedef map<string, CGroupingRec*> map_name_to_pat_grp;
typedef map<string, vector<NameItem> > MapStringToNameItems;

struct SWordData {
	//string W;
	map<int, int> GrpList; // The IDs of all the groups that include the word plus the NumOccurs
	//int CountIndex;
	void Store(ofstream& fout);
	void Load(ifstream& fin);
};

typedef void(CGotitEnv::*InitTaskFn)();

static u64 const cMaintenancePeriod = 60000000ll;

class CGotitEnv {
	static const int cNumElsForSurprise = 20;
public:
	CGotitEnv() : PatGrpMgr(this), AlignConfig(NumAlignParams) {}
	~CGotitEnv();

	void ParseConfigFile();
	void RegisterCallableFunctions();
	bool getTaskFn(string FnName, InitTaskFn& Fn);
	bool GetImplemParam(string& Val, string Key);
	void ExecTaskByName(string TaskName);
	float FindGrpElSurprisingness(string& W, CPatGrpHolder * pPatGrp);
	void AddWordGrpLink(string& W, int GrpID, int NumOccurs);
	map<string, SWordData>& getWordTbl() { return MapWordToData;  }
	int getWordHitCount(string& W);

	// callable Task Functions
	void InitLoadFromNLP();
	void InitLoadFromMasc();
	void InitLoadProcessed();
	void CreateGroups();
	void StartWithGroups();
	void LoadGroups();
	void LoadSentenceListOneMod();
	void FillGroups();
	void SaveGroups();
	void ClearSentenceRecs();
	void TestByWordSwap();
	void InitSGrpSeed();
	void InitSGrpSeed1();
	void RunRF();
	void PrintGroups();
	void CleanGroups();
	void FindAlignedWord();
	void ShowGoodPeers();
	void CreatePeerList();
	void WordCorrect();
	void InitCheckers();
	void PlayCheckers();
	void InitGame();
	void PlayGame();
	void GameCreateHD5();
	void GameConnectToCaffe();
	void MakeGroupsFromData(); // create and fill groups based on data itself
	void SaveSimple();
	void SaveNGrams();
	void SaveWordToPos();
	void SaveAEWordVec();
	void CaffeFn();
	void CaffeFnInit();
	void CaffeFnComplete();
	
private:
	void WordsCreated();
	void LoadWords();

	void CreateWordGroup(SGrpSeed* pSeedTblEl, vector<CPatGrpIter*>& SeedGrps);
	void LearnWordSwap();
	void CreateGrpsFromSeed(string Seed, vector<CGroupingRec*>& RelevantGroups);
	void CreateRFErrExList(	bool bImplementAsPair, // Implementation Strategy Parameter. Instead of errors, differentitate between two anchors
							string FirstOfPair,
							string SecondOfPair,
							float& ScoreBroad, float& ScoreSel,
							vector<int>& SelGroupIndices,
							SGrpSeed& Seed0, SGrpSeed& Seed1);
	void TimeStamp(string& TSName);
	void DoMaintenance();
	void FindRelatingGroups0(string& Seed0, string& seed1, bool& bFirstMorePopular, 
							vector<string>& RelatedGrps, vector<string>& Seed0Only, 
							vector<string>& Seed1Only, vector<string>& WeakGrps);
	void FindRelatingGroups(string& Seed0, string& seed1, bool& bFirstMorePopular, 
							vector<string>& RelatedGrps, vector<string>& Seed0Only, 
							vector<string>& Seed1Only, vector<string>& WeakGrps,
							float& Score);
	void EvalMembershipChances(	string& DepName, string& GovWord, string& GrpNamePrefix, string& DepWordSrc, 
								vector<string>& DepWordComps, int iCurr, float& Score);
	void EvaluateDep(DepRec& Dep, vector<string>& DepNames,
					 vector<WordRec>& WordRec, float& Score);
	void FindGrpConnections(vector<string>& Seed0Only, vector<string>& Seed1Only, 
							int ConnThresh, vector<pair<int, int> >& GrpConnList);
	bool CalcGrpOPC(CPatGrpWrapper& PatGrp, float& OPCHCount, float& OPCMCount, float& OPCLCount);
	void PrintGrpOPC(CPatGrpWrapper& PatGrp);
	void OneAlignConfigRun(	vector<int>& TamperedPosArr, vector<SSentenceRec>& UntamperedRecs, 
							vector<string>& DepNames, float ScoreAlignParam, bool bOOB,
							float& ScoreRet);

	CModNames ModNamesCntrl;
	MapBaseWordToCount BaseWordCount;
	map<string, int> MapWordToCountIndex; // For each word in word list of BasicTypeLists, what is its index in that list
	map<string, SWordData> MapWordToData;
	//vector<NameItem > WordsInOrder;
	vector<SPatternFinder > anc_pat_finder_list;
	vector<CGroupingRec *> MergedGroupsList;
	MapStringToNameItems BasicTypeLists;
	MapBaseWordToCount DepTypes; // a map keeping the names of dependency (not collapsed) unique
	MapNameToImplemParam ImplemParamTbl;
	vector<CPatGrpIter> PatGroups;
	//map_name_to_pat_grp PatGrpTbl;
	CPatGrpMgr PatGrpMgr;
	vector<SSentenceRec> SentenceRec; // not so much a global param as a means of passing around a sample set of sentences
	vector<CorefRec> CorefList; // not alligned with SentenceRec but SentenceID refers to rec in list
	string ModName;
	//vector<SSentenceRec> SentenceListRet;
	vector<CGroupingRec*> RelevantGroups;
	map<string, InitTaskFn> TaskFnMap;
	vector<STaskList> TaskListList;
	// RF Params
	SGrpSeed Seed0;
	SGrpSeed Seed1;
	int RFNumVars; // How many vars input into the forest
	float RFScore;
	float RFScoreOnSel; // selected fields only
	float RFSelfScore;
	vector<int> RFSelVars; // The vars most used in growing the forest
	bool bStreamsAreNew;
	// BasicTypeLists params
	vector<string> DynamicTypeListsNames;
	u64 LastMaintenance;
	map<string, bool> ModsAlreadyUsed;
	vector<pair<float, string> > PeerVec;; // return param from CreatePeerList
	void * CaffeFnHandle; // Avoiding declaring CaffeGenData, a ::google::protobuf::Message at this level
	void * CaffeFnOutHandle; // as above

	// Align Params
	enum {
		AlignParamMatchFoundConst,
		AlignParamMatchFoundOPCR,
		AlignParamNoMatchSurprising,
		AlignParamNumPeersFactor,
		AlignParamNumPeersFactorPerNumPeers,
		AlignParamNumPeersFactorLimit,
		AlignParamNumPeersFactorPerNumPeersLimit,
		AlignParamNotFoundInPeerGrp,
		AlignParamNotFoundInPeerGrpPerNumGrps,
		AlignParamFoundInPeerGrp,
		AlignParamFoundInPeerGrpPerNumGrps,
		AlignParamAWordFactor,
		AlignParamEvalDepAsIs,
		AlignParamEvalDepPerNonZero,
		NumAlignParams
	};
	vector<float> AlignConfig;
	bool bDebugWordAlign;

};

//extern map<string, InitTaskFn> xTaskFnMap;

extern fstream flog;
extern bool xbDebugMemHere;

//#include "windows.h"
#ifdef GOTIT_LINUX
#else
extern"C" __declspec(dllimport) void __stdcall Sleep(unsigned long dwMs);
#endif

int lmain();
char * FileToBuf(const char * ModName, const char * ExtName, const char * PathName = "");
bool InputFiles(const char * ModName, MapIDToTextRegion& RegionMap, MapIDToPOSRec& POSMap,
				MapIDToSentenceRec& SentenceMap);
bool CreateSentenceList(MapModuleNameToIndex& ModsToSentenceIndex,
						vector<vector<SSentenceRec> >& SentenceRecList,
						vector<string>& ModNames);
bool CreateSentenceListOneMod(string ModName,
								vector<SSentenceRec>& SentenceRec,
								MapBaseWordToCount& BaseWordCount,
								vector<string>& OnlyIfWordsPresent,
								int ModNum = -1);
//void LoadSentenceListOneMod(string ModName,
//							vector<SSentenceRec>& SentenceListRet,
//							MapBaseWordToCount& BaseWordCount,
//							vector<pair<string, int> >& WordsInOrder,
//							vector<string>& OnlyIfWordsPresent);
void ParseConfigFile(vector<SPatternFinder >& NodeList, MapStringToNameItems& BasicTypeLists, MapNameToImplemParam& ImplemParamTbl);
void StringStore(const string& s, ofstream& fout);
void StringStore(const string& s, fstream& fout);
void StringStore(const string& s, ofstream& fout, int& NumBytesWritten);
void StringLoad(string& s, ifstream& fin);
void StringLoad(string& s, fstream& fin);
void BWCountStore(MapBaseWordToCount& bwc, fstream& fout);
void BWCountLoad(MapBaseWordToCount& bwc, fstream& fin);
int rfmain();
//void CreateGrpsFromSeed(vector<SPatternFinder >& anc_pat_finder_list,
//						MapBaseWordToCount& BaseWordCount, 
//						vector<pair<string, int> >& WordsInOrder,
//						CModNames& ModNames, vector<CGroupingRec *>& PatGroups,
//						map_name_to_pat_grp& pat_grp_tbl,
//						vector<CGroupingRec *>& MergedGroupsList,
//						string Seed, vector<CGroupingRec*>& RelevantGroups);
//void LearnWordSwap(vector<SPatternFinder >& anc_pat_finder_list, 
//						MapNameToImplemParam& ImplemParamTbl,
//						MapBaseWordToCount& BaseWordCount, vector<pair<string, int> >& WordsInOrder,
//						CModNames& ModNames, vector<CGroupingRec *>& PatGroups,
//						map_name_to_pat_grp& PatGrpTbl,
//						vector<CGroupingRec *>& MergedGroupsList);
void SaveGroups(vector<CGroupingRec *>& PatGroups, map_name_to_pat_grp& PatGrpTbl,
				MapBaseWordToCount& BaseWordCount);
void OneSearch(	CGroupingRec& NewGrouping,
				vector<SSentenceRec> & SentenceList);
//void CreateRFErrExList(MapBaseWordToCount& BaseWordCount, vector<pair<string, int> >& WordsInOrder,
//						CModNames& ModNames,
//						vector<CGroupingRec *>& PatGroups, 
//						bool bImplementAsPair, // Implementation Strategy Parameter. Instead of errors, differentitate between two anchors
//						string FirstOfPair,
//						string SecondOfPair,
//						float& ScoreBroad, float& ScoreSel,
//						vector<int>& SelGroupIndices,
//						SGrpSeed& Seed0, SGrpSeed& Seed1
//						);
bool gtNameItems(const NameItem& r1, const NameItem& r2);
void OrderWords(MapBaseWordToCount& BaseWordCount,
				vector<NameItem >& WordsInOrder);
void CreateWordGroup(	vector<SPatternFinder >& anc_pat_finder_list,
						MapBaseWordToCount& BaseWordCount,
						vector<pair<string, int> >& WordsInOrder,
						CModNames& ModNames, vector<CGroupingRec *>& PatGroups,
						map_name_to_pat_grp& PatGrpTbl,
						SGrpSeed* pSeedTblEl, vector<CGroupingRec*>& SeedGrps);
bool NotPureAlpha(string& sw);
void ConvertStanfordOutput(	string OutputDir, string FileName, string Ext, 
							bool bAddToDB,
							MapBaseWordToCount& BaseWordCount, map<string, int>& DepTypes,
							MapStringToNameItems& BasicTypesList,
							vector<string>& DynamicTypeListsNames);
void StoreDepTypes(map<string, int>& DepTypes, fstream& fout);
void LoadDepTypes(map<string, int>& DepTypes, fstream& fin);
void StoreNameItemList(string& Name, vector<NameItem >& NameItemList, fstream& fout);
void LoadNameItemList(vector<string>& DynamicTypeListsNames, MapStringToNameItems& BasicTypeLists, fstream& fin);
void OneSearchOnDep(MapStringToNameItems& NameLists,
					map<string, int>& DepTypes,
					CPatGrpWrapper& NewGrouping,
					vector<SSentenceRec> & SentenceList,
					CGotitEnv * pEnv);
void FillOneGrpForOneSRec(	MapStringToNameItems& NameLists,
							map<string, int>& DepTypes,
							CPatGrpWrapper& NewGrouping,
							SSentenceRec & SRec,
							CGotitEnv * pEnv,
							SPatternFinder * pFinder,
							vector<string>& ElsToAdd);
void AddElsToGrp(CPatGrpWrapper& NewGrouping, vector<string>& NewEls, CGotitEnv * pEnv);
bool SafeOpen(fstream& f, string FName, ios::openmode Flags);
bool TestFileExists(string& FName);
bool FileRemove(string& FName);
bool FileRename(string& FNameOld, string& FNameNew);
u64 TimerNow();
void StoreWordToCountIndex(map<string, int>& MapWordToCountIndex, fstream& f);
void LoadWordToCountIndex(map<string, int>& MapWordToCountIndex, fstream& f);
float CalcOPC(int NumOccurs, int GlobalCount);
void DoNLPOnFile(string& IFileName, string& OFileName);
void srand(int);

