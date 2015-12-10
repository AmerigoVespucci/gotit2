// TaskFns.cpp : Parses the output of Standford CoreNLP							
//

#include "stdafx.h"

#include "MascReader.h"
#include "./RF/librf/instance_set.h"
#include "./RF/librf/random_forest.h"

using namespace librf;

#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif

void CGotitEnv::RegisterCallableFunctions()
{
	TaskFnMap["InitLoadFromNLP"] = &CGotitEnv::InitLoadFromNLP;
	TaskFnMap["InitLoadFromMasc"] = &CGotitEnv::InitLoadFromMasc;
	TaskFnMap["InitLoadProcessed"] = &CGotitEnv::InitLoadProcessed;
	TaskFnMap["CreateGroups"] = &CGotitEnv::CreateGroups;
	//TaskFnMap["StartWithGroups"] = &CGotitEnv::StartWithGroups;
	TaskFnMap["LoadGroups"] = &CGotitEnv::LoadGroups;
	TaskFnMap["LoadSentenceListOneMod"] = &CGotitEnv::LoadSentenceListOneMod;
	TaskFnMap["FillGroups"] = &CGotitEnv::FillGroups;
	TaskFnMap["SaveGroups"] = &CGotitEnv::SaveGroups;
	TaskFnMap["ClearSentenceRecs"] = &CGotitEnv::ClearSentenceRecs;
	TaskFnMap["TestByWordSwap"] = &CGotitEnv::TestByWordSwap;
	TaskFnMap["InitSGrpSeed"] = &CGotitEnv::InitSGrpSeed;
	TaskFnMap["InitSGrpSeed1"] = &CGotitEnv::InitSGrpSeed1;
	TaskFnMap["RunRF"] = &CGotitEnv::RunRF;
	TaskFnMap["PrintGroups"] = &CGotitEnv::PrintGroups;
	TaskFnMap["CleanGroups"] = &CGotitEnv::CleanGroups;
	TaskFnMap["ShowGoodPeers"] = &CGotitEnv::ShowGoodPeers;
	TaskFnMap["FindAlignedWord"] = &CGotitEnv::FindAlignedWord;
	TaskFnMap["WordCorrect"] = &CGotitEnv::WordCorrect;
	TaskFnMap["WordsCreated"] = &CGotitEnv::WordsCreated;
	TaskFnMap["InitCheckers"] = &CGotitEnv::InitCheckers;
	TaskFnMap["PlayCheckers"] = &CGotitEnv::PlayCheckers;
	TaskFnMap["InitGame"] = &CGotitEnv::InitGame;
	TaskFnMap["PlayGame"] = &CGotitEnv::PlayGame;
	TaskFnMap["GameCreateHD5"] = &CGotitEnv::GameCreateHD5;
	TaskFnMap["GameConnectToCaffe"] = &CGotitEnv::GameConnectToCaffe;
	TaskFnMap["MakeGroupsFromData"] = &CGotitEnv::MakeGroupsFromData;
	TaskFnMap["SaveSimple"] = &CGotitEnv::SaveSimple;
	TaskFnMap["SaveNGrams"] = &CGotitEnv::SaveNGrams;
//	TaskFnMap["SaveWordToPos"] = &CGotitEnv::SaveWordToPos;
//	TaskFnMap["SaveAEWordVec"] = &CGotitEnv::SaveAEWordVec;
	TaskFnMap["CaffeFn"] = &CGotitEnv::CaffeFn;
	TaskFnMap["CaffeFnInit"] = &CGotitEnv::CaffeFnInit;
	TaskFnMap["CaffeFnComplete"] = &CGotitEnv::CaffeFnComplete;
}


void CGotitEnv::WordsCreated()
{
	// this function adds to BaseWordCount so it must redo WordsInOrder
	//WordsInOrder.clear();
	vector<NameItem > aNameItemVector;
	BasicTypeLists["word"] = aNameItemVector;
	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	MapBaseWordToCount::iterator it = BaseWordCount.begin();
	for (; it != BaseWordCount.end(); it++) {
		WordsInOrder.push_back(NameItem(it->first, it->second));
	}
	sort(WordsInOrder.begin(), WordsInOrder.end(), gtNameItems);
	uint NumWordsOrdered = WordsInOrder.size();
	for (uint iwio = 0; iwio < NumWordsOrdered; iwio++) {
		MapWordToCountIndex[WordsInOrder[iwio].Name] = iwio;
	}
	//ofstream fileBWCount("D:\\guten\\state\\WordCount.bin", ios::binary);
	fstream fileNameItems;
	cout << "WordsCreated(). Opening NamedCounts.bin" << endl;
	string NamedCountsFileName = NAMED_COUNT_FNAME;
	string sLookup;
	if (GetImplemParam(sLookup, "Implem.Param.NamedCountsFileName")) {
		NamedCountsFileName = sLookup;
	}
	if (SafeOpen(	fileNameItems, NamedCountsFileName,
					ios::binary | ios::out)) {
		BWCountStore(BaseWordCount, fileNameItems);
		//ofstream fileDepTypes("C:\\guten\\DepTypes.bin", ios::binary);
		StoreDepTypes(DepTypes, fileNameItems);
		StoreWordToCountIndex(MapWordToCountIndex, fileNameItems);
		uint NumDynamicTypes = DynamicTypeListsNames.size();
		fileNameItems.write((char *)&NumDynamicTypes, sizeof(NumDynamicTypes));
		for (uint idtln = 0; idtln < NumDynamicTypes; idtln++) {
			string& CountName = DynamicTypeListsNames[idtln];
			vector<NameItem >& NameItemList = BasicTypeLists[CountName];
			StoreNameItemList(CountName, NameItemList, fileNameItems);
		}
		cout << "WordsCreated(). wrote " << NumDynamicTypes << " items. " << endl;
	}

		//bSkipTheRest = true;
}

void CGotitEnv::LoadWords()
{
	fstream fileNameItems;
	string NamedCountsFileName = NAMED_COUNT_FNAME;
	string sLookup;
	if (GetImplemParam(sLookup, "Implem.Param.NamedCountsFileName")) {
		NamedCountsFileName = sLookup;
	}
	if (SafeOpen(fileNameItems, NamedCountsFileName,
		ios::binary | ios::in)) {
		BaseWordCount.clear();
		//ifstream fileBWCount("C:\\guten\\WordCount.bin", ios::binary);
		BWCountLoad(BaseWordCount, fileNameItems);
		DepTypes.clear();
		//ifstream fileDepTypes("C:\\guten\\DepTypes.bin", ios::binary);
		LoadDepTypes(DepTypes, fileNameItems);
		LoadWordToCountIndex(MapWordToCountIndex, fileNameItems);
		uint NumDynamicTypes;
		fileNameItems.read((char *)&NumDynamicTypes, sizeof(NumDynamicTypes));
		for (uint idtln = 0; idtln < NumDynamicTypes; idtln++) {
			//string& CountName = DynamicTypeListsNames[idtln];
			//MapStringToNameItems::iterator itbtl = BasicTypeLists.find(CountName);
			//if (itbtl == BasicTypeLists.end()) {
			//	BasicTypeLists[CountName] = vector<NameItem>();
			//}
			//vector<NameItem >& NameItemList = BasicTypeLists[CountName];
			LoadNameItemList(DynamicTypeListsNames, BasicTypeLists, fileNameItems);
			//sort(NameItemList.begin(), NameItemList.end(), gtNameItems);
		}
	}
	//BaseWordCount.clear();
	//ifstream fileBWCount("WordCount.bin", ios::binary);
	//BWCountLoad(BaseWordCount, fileBWCount);
	
	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	WordsInOrder.clear();
	MapBaseWordToCount::iterator it = BaseWordCount.begin();
	for (; it != BaseWordCount.end(); it++) {
		WordsInOrder.push_back(NameItem(it->first, it->second));
	}
	sort(WordsInOrder.begin(), WordsInOrder.end(), gtNameItems);
	

}

void StoreNameItemList(string& Name, vector<NameItem >& NameItemList, fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	StringStore(Name, fout);
	uint NumEls = NameItemList.size();
	fout.write((char *)&NumEls, sizeof(NumEls));
	vector<NameItem>::iterator itdt = NameItemList.begin();
	for (; itdt != NameItemList.end(); itdt++) {
		StringStore(string(itdt->Name), fout);
		fout.write((char *)&(itdt->Count), sizeof(itdt->Count));
	}

}

//void LoadDepTypes(map<string, int>& DepTypes, fstream& f)
void LoadNameItemList(vector<string>& DynamicTypeListsNames, MapStringToNameItems& BasicTypeLists, fstream& f)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	string CountName;
	StringLoad(CountName, fin);
	DynamicTypeListsNames.push_back(CountName);
	MapStringToNameItems::iterator itbtl = BasicTypeLists.find(CountName);
	if (itbtl == BasicTypeLists.end()) {
		BasicTypeLists[CountName] = vector<NameItem>();
	}
	vector<NameItem >& NameItemList = BasicTypeLists[CountName];
	uint NumEls;
	fin.read((char *)&NumEls, sizeof(NumEls));
	for (uint idt = 0; idt < NumEls; idt++) {
		string w;
		int n;
		StringLoad(w, fin);
		fin.read((char *)&n, sizeof(n));
		NameItemList.push_back(NameItem(w, n));
	}
	// next line commented because we should be storing already sorted
	//sort(NameItemList.begin(), NameItemList.end(), gtNameItems);

}

void StoreWordToCountIndex(map<string, int>& MapWordToCountIndex, fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	uint NumWords = MapWordToCountIndex.size();
	fout.write((char *)&(NumWords), sizeof(NumWords));
	MapBaseWordToCount::iterator it = MapWordToCountIndex.begin();
	for (; it != MapWordToCountIndex.end(); it++) {
		StringStore(string(it->first), fout);
		fout.write((char *)&(it->second), sizeof(it->second));
	}
}

void LoadWordToCountIndex(map<string, int>& MapWordToCountIndex, fstream& f)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	uint NumWords;
	fin.read((char *)&NumWords, sizeof(NumWords));
	for (uint i = 0; i < NumWords; i++) {
		string w;
		int n;
		StringLoad(w, fin);
		fin.read((char *)&n, sizeof(n));
		MapWordToCountIndex[w] = n;
	}
}

static const bool cbIfExt = true;
static const bool cbRemoveExt = true;

void CGotitEnv::InitLoadFromNLP()
{
	CModNames ModNamesCntrlNLP;
	ModNamesCntrlNLP.LoadModuleNames(	NLP_OUT, cbIfExt, ".xml", !cbRemoveExt, 
										"", atoi(ImplemParamTbl["Implem.Param.LoadNLPFilesLimit"].Val.c_str()));
	DynamicTypeListsNames.push_back("depcount");
	DynamicTypeListsNames.push_back("NERCount");
	DynamicTypeListsNames.push_back("POSCount");
	string ModName;
	bool bKeepGoing = ModNamesCntrlNLP.InitSeq();
	int im = 0;
	while (bKeepGoing) {
		//for (int im = 0; im < NumMods; im++) {
		// dummy parameters, not needed for simple transfer to new format
		bKeepGoing = !ModNamesCntrlNLP.GetSeqModName(ModName);
		const bool bAddToDB = true;
		ConvertStanfordOutput(FILES_EX, ModName, ".mod", bAddToDB,
								BaseWordCount, DepTypes, BasicTypeLists, 
								DynamicTypeListsNames);
		cout << "NLP Converted " << ModName << endl;
	}
	// print out for copying to config xml
	//map<string, int>::iterator itdt = DepTypes.begin();
	//for (; itdt != DepTypes.end(); itdt++) {
	//	cout << itdt->first << endl;
	//}
	WordsCreated();

}

void CGotitEnv::InitLoadFromMasc()
{
	ModNamesCntrl.LoadModuleNames("C:\\MASC-3.0.0", cbIfExt, PENN_SUFFIX, cbRemoveExt, "", -1);
	int NumMods = ModNamesCntrl.getSize(); // 
	bool bKeepGoing = ModNamesCntrl.InitSeq();
	string ModName;
	int im = 0;
	while (bKeepGoing) {
		//for (int im = 0; im < NumMods; im++) {
		// dummy parameters, not needed for simple transfer to new format
		bKeepGoing = !ModNamesCntrl.GetSeqModName(ModName);
		vector<SSentenceRec> SentenceRec;
		vector<string> OnlyIfWordsPresent;
		CreateSentenceListOneMod(ModName, SentenceRec,
			BaseWordCount, 
			OnlyIfWordsPresent, im++);
		cout << "Converted " << ModName << endl;
	}
	WordsCreated();

}

void CGotitEnv::InitLoadProcessed()
{
	//ModNamesCntrl.LoadModuleNames("C:\\GotitFiles", !cbIfExt, "", !cbRemoveExt, "", -1);
	ModNamesCntrl.LoadModuleNames(FILES_EX, cbIfExt, ".mod",
		!cbRemoveExt, "",
		atoi(ImplemParamTbl["Implem.Param.LoadGotitFilesLimit"].Val.c_str()));
	LoadWords();
}

void CGotitEnv::LoadSentenceListOneMod()
{
	//string ModName = ImplemParamTbl["Implem.Param.FnParam.LoadSentenceListOneMod.ModName"];
	bool bUseModName = false;
	{
		string UseModNameName("Implem.Param.FnParam.LoadSentenceListOneMod.UseModName");
		MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(UseModNameName);
		if (itipt != ImplemParamTbl.end()) {
			if (itipt->second.Val == "1") {
				bUseModName = true;
			}
			// once used, remove, becuase default is that param does not exist
			ImplemParamTbl.erase(itipt);
		}
	}
	bool bGetOOB = false;
	{
		string GetOOBName("Implem.Param.FnParam.LoadSentenceListOneMod.GetOOB");
		MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(GetOOBName);
		if (itipt != ImplemParamTbl.end()) {
			if (itipt->second.Val == "1") {
				bGetOOB = true;
			}
			ImplemParamTbl.erase(itipt);
		}
	}

	bool bModFound = false;
	string ModName;
	if (bUseModName) {
		ModName = ImplemParamTbl["Implem.Param.FnParam.LoadSentenceListOneMod.ModName"].Val;
	}
	else {
		int NumModTries = 0;
		int NumModsToTry = ModNamesCntrl.getSize() * 3;
		while (!bModFound) {
			ModName = ModNamesCntrl.GetRandModName(bGetOOB);
			map<string, bool>::iterator itUsed = ModsAlreadyUsed.find(ModName);
			if (itUsed != ModsAlreadyUsed.end()) {
				NumModTries++;
				if (NumModTries > NumModsToTry) {
					ModsAlreadyUsed.clear();
				}
			}
			else {
				bModFound = true;
				ModsAlreadyUsed[ModName] = true;
			}
		}
	}
	cout << "LoadSentenceListOneMod: Loading sentence recs from " <<  ModName << endl;
	string AllIfWords = ImplemParamTbl["Implem.Param.FnParam.LoadSentenceListOneMod.OnlyIfWordsPresent"].Val;
	vector<string> OnlyIfWordsPresent;
	string delimiter = ",";
	if (!AllIfWords.empty() && (AllIfWords.back() != ',')) {
		AllIfWords += delimiter;
	}
	size_t pos = 0;
	std::string token;
	while ((pos = AllIfWords.find(delimiter)) != std::string::npos) {
		token = AllIfWords.substr(0, pos);
		//std::cout << token << std::endl;
		OnlyIfWordsPresent.push_back(token);
		AllIfWords.erase(0, pos + delimiter.length());
	}
	string FileName = ModName;
	uint NumIfWords = OnlyIfWordsPresent.size();
	ifstream ModFile(FileName, ios::binary);
	uint NumRecs;
	ModFile.read((char *)&(NumRecs), sizeof(NumRecs));
	if (NumRecs > 100000) {
		cerr << "ERROR! Number of records to read for " << ModName << " seems too large\n";
		return;
	}
	for (uint im = 0; im < NumRecs; im++) {
		SSentenceRec Rec;
		Rec.Load(ModFile);
		bool bOnlyIfWordFound = (NumIfWords == 0); // false for now if list not empty
		for (uint iw = 0; iw < Rec.OneWordRec.size() && !bOnlyIfWordFound; iw++) {
			WordRec wrec = Rec.OneWordRec[iw];
			string W = wrec.Word;
			//MapBaseWordToCount::iterator fw = BaseWordCount.find(W);
			//if (fw == BaseWordCount.end()) {
			//	BaseWordCount[W] = 1;
			//}
			//else {
			//	fw->second++;
			//}
			if (!bOnlyIfWordFound) {
				for (uint iif = 0; iif < NumIfWords; iif++) {
					if (W == OnlyIfWordsPresent[iif]) {
						bOnlyIfWordFound = true;
						break;
					}
				}
			}
		}
		if (bOnlyIfWordFound) {
			SentenceRec.push_back(Rec);
		}
	}


}

void CGotitEnv::ClearSentenceRecs()
{
	SentenceRec.clear();
}

void CGotitEnv::RunRF()
{
	stringstream& RFOut = *(Seed0.pRFData);
	stringstream& RFOutOOB = *(Seed0.pRFDataOOB);
	stringstream& RFLabel = *(Seed0.pRFLabel);; // ("ThatExLabel.csv");
	stringstream& RFLabelOOB = *(Seed0.pRFLabelOOB); //  ("ThatExLabelOOB.csv");
	//cout << "RFOut: \n" << RFOut.str();
	//cout << "RFLabel: \n" << RFLabel.str();
	//return;
	InstanceSet * LearnSet = InstanceSet::LoadFromSS(RFOut, RFLabel, true);
	InstanceSet * LearnSetOOB = InstanceSet::LoadFromSS(RFOutOOB, RFLabelOOB, true);
	int K = (int)(floor(sqrt((double)RFNumVars))); 
	RFSelVars.clear();
	vector< pair< float, int> > ranking;
	{
		RandomForest rf(*LearnSet, 100, K);
		float sscore = rf.training_accuracy();
		RFScore = rf.testing_accuracy(*LearnSetOOB);
		unsigned int seed = rand();
		rf.variable_importance(&ranking, &seed);
	}
	if (RFScore != RFScore) { // NAN test on most compilers
		RFScore = 0.5f;
	}
	//float vari = (float)NumLabel0 / ((float)NumLabel0 + (float)NumLabel1);


	cout << "rf score:" << RFScore << "\n";

	vector<int> SelAttribs;
	for (int ir = 0; ir < 20; ir++) {
		SelAttribs.push_back(ranking[ir].second);
		RFSelVars.push_back(ranking[ir].second);
	}
	InstanceSet * IrisSetSel = InstanceSet::feature_select(*LearnSet, SelAttribs);
	InstanceSet * IrisSetOOBSel = InstanceSet::feature_select(*LearnSetOOB, SelAttribs);
	{
		RandomForest rfSel(*IrisSetSel, 100, 5);
		RFScoreOnSel = rfSel.testing_accuracy(*IrisSetOOBSel);
	}
	if (RFScoreOnSel != RFScoreOnSel) {
		RFScoreOnSel = 0.5f;
	}

	cout << "sel rf score:" << RFScoreOnSel << "\n";
	delete IrisSetOOBSel;
	delete IrisSetSel;
	delete LearnSetOOB;
	delete LearnSet;

	delete Seed0.pRFData;
	delete Seed0.pRFDataOOB;
	delete Seed0.pRFLabel;
	delete Seed0.pRFLabelOOB;

}

void CGotitEnv::FillGroups()
{
	if (SentenceRec.size() > 0) {
		// loop executing pat grps
		//map_name_to_pat_grp::iterator itpg = PatGrpTbl.begin();
		CPatGrpIter itpg = PatGrpMgr.getStartIterator();
		bool bFilterByParentAnc = false;
		string ParentName = ImplemParamTbl["Implem.Param.FnParam.FillGroups.ParentName"].Val;
		SPatternFinder* pFinder = NULL;
		uint iFinderChild = 0;
		if (ParentName != "") {
			for (uint iAnc = 0; iAnc < anc_pat_finder_list.size(); iAnc++) {
				if (anc_pat_finder_list[iAnc].group_name_template == ParentName) {
					bFilterByParentAnc = true;
					pFinder = &(anc_pat_finder_list[iAnc]);
					break;
				}
			}
			if (!bFilterByParentAnc) {
				cerr << "Error: Ancestor Pattern name " << ParentName << " not found for FillGroups\n";
				return;
			}
		}
		CPatGrpWrapper * pPatGrp = NULL;
		while (true) {
			//for (; itpg != PatGrpTbl.end(); itpg++) 
			//CGroupingRec * patgrp;
			if (bFilterByParentAnc) {
				if (pPatGrp != NULL) {
					delete pPatGrp;
					pPatGrp = NULL;
				}
				if (iFinderChild >= pFinder->ChildPatGrps.size()) {
					break;
				}
				pPatGrp = new CPatGrpWrapper(pFinder->ChildPatGrps[iFinderChild]);
				iFinderChild++;
			}
			else {
				if (pPatGrp != NULL) {
					delete pPatGrp;
					pPatGrp = NULL;
				}
				if (!PatGrpMgr.IsIteratorValid(itpg)) {
				//if (itpg == PatGrpTbl.end()) {
					break;
				}
				pPatGrp = new CPatGrpWrapper(itpg);
				pPatGrp->getGrpFinder(pFinder); // needed later but otherwise loaded only for bFilterByParentAnc
				PatGrpMgr.getNextIterator(itpg);
				//itpg++; // for next iter make sure the iter index is not used again in the loop!
			}
			//if (patgrp->GrpType != cGrpTypeNormal) {
			//	continue;
			//}
			string sFilter = ImplemParamTbl["Implem.Param.FnParam.FillGroups.GrpParam0"].Val;
			if (sFilter != "") {
				if ((pPatGrp->getNumGrpParams() > 0) && (pPatGrp->getGrpParam(0) != sFilter)) {
					delete pPatGrp;
					pPatGrp = NULL;
					continue;
				}
			}

			//SPatternFinder* pFinder;
			if (((float)rand() / (float)RAND_MAX) > pFinder->run_prob) {
				delete pPatGrp;
				pPatGrp = NULL;
				continue;
			}
			//if (patgrp->pfinder->SrcTextType == "dep") {
			OneSearchOnDep(BasicTypeLists, DepTypes, *pPatGrp, SentenceRec, this);
			//}
			//else {
			//	OneSearch(*patgrp, SentenceRec);
			//}
			pPatGrp->CreateStrengthSort();
		} // end loop over pattern groups to search on and strengthen
		if (pPatGrp != NULL) {
			cerr << "Coding error! pPatGrp not deleted\n";
		}
	} // end if there are sentences to search on
}

enum etMakeGrpSrc {
	etmgsDep,
	etmgsWord,
	etmgsNumTypes
};

struct SMakeGrpSrc {
	bool bFilter;
	int CountLimit;
	etMakeGrpSrc etmgs;
};

//SMakeGrpSrc SrcTbl[] = { { false, 0, etmgsDep }, { true, 600, etmgsWord } };

void CGotitEnv::MakeGroupsFromData()
{
	if (SentenceRec.size() == 0) {
		return;
	}
	string SpecificFinderName = ImplemParamTbl["Implem.Param.FnParam.CreateGroups.SpecificFinderNm"].Val;
	int SpecificFinder = -1;
	for (uint ifl = 0; ifl < anc_pat_finder_list.size(); ifl++) {
		if (anc_pat_finder_list[ifl].group_name_template == SpecificFinderName) {
			SpecificFinder = ifl;
			break;
		}
	}
	if (SpecificFinder == -1) {
		return;
	}

	vector <SMakeGrpSrc> SrcTbl;
	SPatternFinder& Finder = anc_pat_finder_list[SpecificFinder];
	for (uint igpl = 0; igpl < Finder.GrpParamList.size(); igpl++) {
		SrcTbl.push_back(SMakeGrpSrc());
		SMakeGrpSrc& GrpSrc = SrcTbl.back();
		SOnePatternFinderGrpParam& GrpParam = Finder.GrpParamList[igpl];
		if (GrpParam.sSrcType == "SentenceRec") {
			if (GrpParam.sSrc == "dep") {
				GrpSrc.etmgs = etmgsDep;
				GrpSrc.bFilter = false;
			}
			else if (GrpParam.sSrc == "word") {
				GrpSrc.etmgs = etmgsWord;
				int iPercentile = stoi(GrpParam.sFillPercentile);
				if (iPercentile == 100) {
					GrpSrc.bFilter = false;
				}
				else {
					int NumWordsInTotal = BaseWordCount.size();
					GrpSrc.CountLimit = NumWordsInTotal * iPercentile / 100;
				}
			}
		}
	}

#ifdef OLD_CODE_A
	bool bFilterWords = false;
	int NumWordsInTotal = BaseWordCount.size();
	string sWordsFromPercentile = ImplemParamTbl["Implem.Param.FnParam.MakeGroupsFromData.WordsFromPercentile"].Val;
	int WordCountLimit = NumWordsInTotal;
	if (sWordsFromPercentile != "") {
		int iPercentile = stoi(sWordsFromPercentile);
		WordCountLimit = NumWordsInTotal * iPercentile / 100;
		bFilterWords = true;
	}
#endif // OLD_CODE_A


	vector<string> DepNames;
	map<CPatGrpHolder *, bool> GrpsNeedingSort; // second not used. JUst a unique list
	DepNames.clear();
	DepNames.resize(DepTypes.size());
	MapBaseWordToCount::iterator itDep = DepTypes.begin();

	for (; itDep != DepTypes.end(); itDep++) {
		DepNames[itDep->second] = itDep->first;
	}
	for (uint isl = 0; isl < SentenceRec.size(); isl++) {
		vector<vector<string> > GrpParamSrcs;
		vector<WordRec>& AllWordRecs = SentenceRec[isl].OneWordRec;
		vector<DepRec>& AllDepRecs = SentenceRec[isl].Deps;
		int TotalNumGrps = 1;
		int NumParamsPerGrp = SrcTbl.size(); // sizeof(SrcTbl) / sizeof(*SrcTbl);
		for (int iSrc = 0; iSrc < NumParamsPerGrp; iSrc++) {
			SMakeGrpSrc& GrpParamSrc = SrcTbl[iSrc];
			GrpParamSrcs.push_back(vector<string>());
			vector<string>& AllOfSrc = GrpParamSrcs.back();
			if (GrpParamSrc.etmgs == etmgsDep) {
				for (uint iDep = 0; iDep < AllDepRecs.size(); iDep++) {
					string& DepName = DepNames[AllDepRecs[iDep].iDep];
					AllOfSrc.push_back(DepName);
				}
			}
			else if (GrpParamSrc.etmgs == etmgsWord) {
				for (uint iWord = 0; iWord < AllWordRecs.size(); iWord++) {
					string& W = AllWordRecs[iWord].Word;
					if (GrpParamSrc.bFilter) {
						map<string, int>::iterator itCountMap = MapWordToCountIndex.find(W);
						if (itCountMap == MapWordToCountIndex.end()) {
							continue;
						}
						if (itCountMap->second > GrpParamSrc.CountLimit) {
							continue;
						}
					}
					AllOfSrc.push_back(W);
				}
			}
			std::sort(AllOfSrc.begin(), AllOfSrc.end());
			vector<string>::iterator itRemoveStart = unique(AllOfSrc.begin(), AllOfSrc.end());
			AllOfSrc.resize(distance(AllOfSrc.begin(), itRemoveStart));
			TotalNumGrps *= AllOfSrc.size();
		}
		if (TotalNumGrps == 0) {
			continue; // will occur if any of the AllOfSrc is empty
		}
		vector<string> OneSetOfGrpParams(NumParamsPerGrp);
		uint NumGrpsForThisOne = TotalNumGrps;
		uint NumLoops = 1;
		vector<vector<string> > AllGrpParams(TotalNumGrps, OneSetOfGrpParams);
		for (uint igps = 0; igps < GrpParamSrcs.size(); igps++) {
			uint NumThisOne = GrpParamSrcs[igps].size();
			NumGrpsForThisOne /= NumThisOne;
			for (uint iLoop = 0; iLoop < NumLoops; iLoop++) {
				for (uint iPut = 0; iPut < NumThisOne; iPut++) {
					for (uint iRep = 0; iRep < NumGrpsForThisOne; iRep++) {
						AllGrpParams[(((iLoop * NumThisOne) + iPut) * NumGrpsForThisOne) + iRep][igps] = GrpParamSrcs[igps][iPut];
					}
				}
			}
			NumLoops *= NumThisOne;
		}
		vector<CPatGrpHolder *> NewGrps;
		vector<vector<string> > ElsAddedInNewGrps;
		for (uint iGrp = 0; iGrp < AllGrpParams.size(); iGrp++) {
#ifdef OLD_CODE
		continue;
		vector<int> AllDeps;
		vector<string> AllWords;
		for (uint iWord = 0; iWord < AllWordRecs.size(); iWord++) {
			string& W = AllWordRecs[iWord].Word;
			if (bFilterWords) {
				map<string, int>::iterator itCountMap = MapWordToCountIndex.find(W);
				if (itCountMap == MapWordToCountIndex.end()) {
					continue;
				}
				if (itCountMap->second > WordCountLimit) {
					continue;
				}
			}
			//int HitCount = getWordHitCount(W);
			AllWords.push_back(W);
		}
		if (AllWords.size() == 0) {
			continue;
		}
		sort(AllWords.begin(), AllWords.end());
		vector<string>::iterator itRemoveStart = unique(AllWords.begin(), AllWords.end());
		AllWords.resize(distance(AllWords.begin(), itRemoveStart));
		for (uint iDep = 0; iDep < AllDepRecs.size(); iDep++) {
			AllDeps.push_back(AllDepRecs[iDep].iDep);
		}
		sort(AllDeps.begin(), AllDeps.end());
		vector<int>::iterator itRemoveStartInt = unique(AllDeps.begin(), AllDeps.end());
		AllDeps.resize(distance(AllDeps.begin(), itRemoveStartInt));
		vector<CPatGrpHolder *> NewGrps;
		vector<vector<string> > ElsAddedInNewGrps;
		for (uint iWord = 0; iWord < AllWords.size(); iWord++) {
			for (uint iDep = 0; iDep < AllDeps.size(); iDep++) {
				string& DepName = DepNames[AllDeps[iDep]];
				vector<string> GrpParams;
				GrpParams.push_back(DepName);
				GrpParams.push_back(AllWords[iWord]);
#endif // OLD_CODE
			string GrpName;
			GrpName = Finder.group_name_template;
			for (int ippg = 0; ippg < NumParamsPerGrp; ippg++) {
				string tstr = std::to_string(ippg);
				GrpName += "[%" + tstr + "%" + AllGrpParams[iGrp][ippg] + "]";
			}

			//GrpName += "[%0%" + AllGrpParams + "][%1%" + AllWords[iWord] + "]";
			bool bGrpAlreadyAdded = false;
			CPatGrpHolder * phPatGrp = PatGrpMgr.FindGrp(GrpName);
			vector<string> ElsToAdd;
			if (phPatGrp == NULL) {
				phPatGrp = PatGrpMgr.CreateUnlinkedPatGrp(	GrpName, &Finder,
															AllGrpParams[iGrp], string(""));
				NewGrps.push_back(phPatGrp);
			}
			else {
				bGrpAlreadyAdded = true;
			}
			//if ((rand() % 5) == 0) {
			//	CPatGrpWrapper PatGrp(phPatGrp);
			//	int NumOccures = -1;
			//	PatGrp.addGrpEl(string("testing"), NumOccures);
			//}
			int NumElsAdded = 0;
			{
				CPatGrpWrapper PatGrp(phPatGrp);
				FillOneGrpForOneSRec(	BasicTypeLists, DepTypes, PatGrp,
										SentenceRec[isl], this, &Finder,
										ElsToAdd);
			}
			if (bGrpAlreadyAdded) {
				if (ElsToAdd.size() > 0) {
					CPatGrpWrapper PatGrp(phPatGrp);
					AddElsToGrp(PatGrp, ElsToAdd, this);
					GrpsNeedingSort[phPatGrp] = true;
				}
			}
			else {
				ElsAddedInNewGrps.push_back(ElsToAdd);
			}
		}
		//} // end loop over words
		for (uint iGrp = 0; iGrp < NewGrps.size(); iGrp++) {
			// This code relies on the list of combined dep and word having no duplicates
			//int NumEls = 0;
			//{
			//	CPatGrpWrapper PatGrp(NewGrps[iGrp]);
			//	NumEls = PatGrp.getGrpElsMapSize();
			//}
			if (ElsAddedInNewGrps[iGrp].size() > 0) {
				CPatGrpHolder * pInserted =  PatGrpMgr.LinkPatGrp(NewGrps[iGrp]);
				CPatGrpWrapper PatGrp(pInserted);
				AddElsToGrp(PatGrp, ElsAddedInNewGrps[iGrp], this);
				GrpsNeedingSort[pInserted] = true;
			}
			else {
				PatGrpMgr.RemoveGrpWithoutSave(NewGrps[iGrp]);
				//delete NewGrps[iGrp];
			}
		}
	} // end loop over "sentence" regs
	map<CPatGrpHolder *, bool>::iterator itForSort =  GrpsNeedingSort.begin();
	for (; itForSort != GrpsNeedingSort.end(); itForSort++) {
		CPatGrpWrapper PatGrp(itForSort->first);
		PatGrp.CreateStrengthSort();
	}
}

void CGotitEnv::CleanGroups()
{
	// loop executing pat grps
	//map_name_to_pat_grp::iterator itpg = PatGrpTbl.begin();
	CPatGrpIter itpg = PatGrpMgr.getStartIterator();
	bool bFilterByParentAnc = false;
	string ParentName = ImplemParamTbl["Implem.Param.FnParam.CleanGroups.ParentName"].Val;
	SPatternFinder* pFinder = NULL;
	int iFinderChild = 0;
	if (ParentName != "") {
		for (uint iAnc = 0; iAnc < anc_pat_finder_list.size(); iAnc++) {
			if (anc_pat_finder_list[iAnc].group_name_template == ParentName) {
				bFilterByParentAnc = true;
				pFinder = &(anc_pat_finder_list[iAnc]);
				iFinderChild = pFinder->ChildPatGrps.size();
				break;
			}
		}
		if (!bFilterByParentAnc) {
			cerr << "Error: Ancestor Pattern name " << ParentName << " not found for FillGroups\n";
			return;
		}
	}
	else {
		cerr << "Error. Clean groups can only be used by specifying a group pattern ancestor name whose groups are to be deleted.\n";
		return;
	}
	CPatGrpWrapper * pPatGrp = NULL;
	while (true) {
		//for (; itpg != PatGrpTbl.end(); itpg++) 
		//CGroupingRec * patgrp;
		if (bFilterByParentAnc) {
			if (pPatGrp != NULL) {
				delete pPatGrp;
				pPatGrp = NULL;
			}
			if (iFinderChild <= 0) {
				break;
			}
			iFinderChild--;
			pPatGrp = new CPatGrpWrapper(pFinder->ChildPatGrps[iFinderChild]);
		}
#ifdef SUPPORT_UNNAMED_GROUP_CLEAN
		else {
			if (pPatGrp != NULL) {
				delete pPatGrp;
				pPatGrp = NULL;
			}
			if (!PatGrpMgr.IsIteratorValid(itpg)) {
				//if (itpg == PatGrpTbl.end()) {
				break;
			}
			pPatGrp = new CPatGrpWrapper(itpg);
			pPatGrp->getGrpFinder(pFinder); // needed later but otherwise loaded only for bFilterByParentAnc
			PatGrpMgr.getNextIterator(itpg);
			//itpg++; // for next iter make sure the iter index is not used again in the loop!
		}
#endif // #ifdef SUPPORT_UNNAMED_GROUP_CLEAN
		//if (patgrp->GrpType != cGrpTypeNormal) {
		//	continue;
		//}
		string sFilter = ImplemParamTbl["Implem.Param.FnParam.CleanGroups.GrpParam0"].Val;
		if (sFilter != "") {
			if ((pPatGrp->getNumGrpParams() > 0) && (pPatGrp->getGrpParam(0) != sFilter)) {
				delete pPatGrp;
				pPatGrp = NULL;
				continue;
			}
		}

		if (bFilterByParentAnc) {
			pFinder->ChildPatGrps.erase(pFinder->ChildPatGrps.begin() + iFinderChild);
			//iFinderChild--;
		}
		//pPatGrp->DeleteData();
		CPatGrpHolder* phGrp = pPatGrp->PresentHolder();
		delete pPatGrp;
		pPatGrp = NULL;
		PatGrpMgr.DeletePatGrp(phGrp);

	} // end loop over pattern groups to search on and strengthen
	if (pPatGrp != NULL) {
		cerr << "Coding error! pPatGrp not deleted\n";
	}
}

void CGotitEnv::DoMaintenance()
{
	//PatGrpMgr.CleanPatGrps(); // No longer cleaning out data here
}

void CGotitEnv::PrintGroups()
{
	string& AncGrpName = ImplemParamTbl["Implem.Param.PrintGroups.AncGrp"].Val;
	SPatternFinder* pFinder = NULL;
	for (uint iAnc = 0; iAnc < anc_pat_finder_list.size(); iAnc++) {
		if (anc_pat_finder_list[iAnc].group_name_template == AncGrpName) {
			pFinder = &(anc_pat_finder_list[iAnc]);
			break;
		}
	}
	if (pFinder == NULL) {
		cerr << "Error: PrintGroups GrpAnc Pattern name " << AncGrpName << " not found\n";
		return;
	}

	MapNameToImplemParam::iterator itStrong = ImplemParamTbl.find("Implem.Param.PrintGroups.NumStrong");
	int NumStrongThresh = -1;
	if (itStrong != ImplemParamTbl.end()) {
		NumStrongThresh = stoi(itStrong->second.Val);
	}

	for (uint iGrp = 0; iGrp < pFinder->ChildPatGrps.size(); iGrp++) {
		CPatGrpWrapper PatGrp(pFinder->ChildPatGrps[iGrp]);
		string GrpName;
		if (!PatGrp.getGrpName(GrpName) || GrpName.size() == 0) {
			return;
		}
		int NumEls = PatGrp.getGrpElsMapSize();
		if (NumStrongThresh >= 0) {
			NumEls = PatGrp.getNumStrongEls();
			if (NumEls < NumStrongThresh) {
				NumEls = 0;
			}
		}
		if (NumEls > 0) {
			cout << "Group Name: " << GrpName << endl;
		}
		for (int iEl = 0; iEl < NumEls; iEl++) {
			SGroupingLink * pEl;
			if (NumStrongThresh >= 0) {
				pEl = PatGrp.getStrongElByIndex(iEl);
			}
			else {
				pEl = PatGrp.getGrpElByIndex(iEl);
			}
			if (pEl) {
				cout << " Name: " << pEl->W;
				cout << " NumOccurs: " << pEl->NumOccurs;
				cout << " NumExist: " << pEl->NumExist;
				cout << " bStrong: " << pEl->bStrong;
				cout << " Strength: " << pEl->Strength;
				cout << " PctHit: " << pEl->PctHit;
				cout << " SurpriseQ: " << pEl->SurprisingNess;
				cout << endl;
			}
		}

	}


}

