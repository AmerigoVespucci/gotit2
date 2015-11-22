// WordCorrect.cpp : Access, Iterate, Save, Load etc for groups
//

#include "stdafx.h"

#include "MascReader.h"

void CGotitEnv::WordCorrect()
{
	bDebugWordAlign = false;
	const int cReplaceDelta = 10;
	const int cNumExamplesForAlt = 400;
	//const int cFromStartOffset = 40;
	vector<NameItem>& WordListSorted = BasicTypeLists["word"];
	ImplemParamTbl["Implem.Param.FnParam.LoadSentenceListOneMod.OnlyIfWordsPresent"].Val = "";
	SImplemParam Param;
	Param.Name = "Implem.Param.FnParam.LoadSentenceListOneMod.GetOOB"; // for this we want to make sure the file wasn't use to build groups
	Param.Val = "1";
	ImplemParamTbl[Param.Name] = Param;
	srand(1001);
	LoadSentenceListOneMod();
	vector<int> TamperedPosArr; // (SentenceRec.size());
	vector<SSentenceRec> UntamperedRecs;
	vector<string> SentencesAltered;
	//for (uint isl = 1000; isl < SentenceRec.size() && isl < 1100; isl++) {
	int iLoopSanityCheck = 0;
	while (true) {
		if (++iLoopSanityCheck > 100000000) {
			cerr << "WordCorrect would enter infinite loop. Aborting!";
			return;
		}
		if (UntamperedRecs.size() > cNumExamplesForAlt) {
			break;
		}
		uint isl = rand() % SentenceRec.size();
		vector<WordRec>& Sentence = SentenceRec[isl].OneWordRec;
		uint SLen = Sentence.size();
		if (SLen < 3) {
			continue;
		}
		bool bGoodEx = true;
		for (uint iW = 0; iW < SLen - 1; iW++) {
			if (NotPureAlpha(Sentence[iW].Word) && Sentence[iW].Word != ",") {
				bGoodEx = false;
				break;
			}
		}
		if (!bGoodEx) {
			continue;
		}
		int iTW = rand() % (SLen - 1); // not the sentence separator
		string& TW = Sentence[iTW].Word;
		map<string, int>::iterator itTWCountIndex
			= MapWordToCountIndex.find(TW);
		if (itTWCountIndex == MapWordToCountIndex.end()) {
			continue;
		}
		uint iTWSortedReplace;
		string TWAltered;
		while (true) {
			iTWSortedReplace = itTWCountIndex->second + (rand() % (cReplaceDelta + 1));
			if (iTWSortedReplace == itTWCountIndex->second) {
				continue;
			}
			if (iTWSortedReplace >= WordListSorted.size()) {
				continue; 
			}
			TWAltered = WordListSorted[iTWSortedReplace].Name;
			if (NotPureAlpha(TWAltered)) {
				continue;
			}
			break;
		}
		
		TamperedPosArr.push_back(iTW);
		UntamperedRecs.push_back(SentenceRec[isl]);
		SentencesAltered.push_back("");
		string& NewSentence = SentencesAltered.back();
		for (uint iW = 0; iW < SLen; iW++) {
			if (iW == iTW) {
				NewSentence += TWAltered;
			}
			else {
				NewSentence += Sentence[iW].Word;
			}
			NewSentence += " ";
		}
		if (bDebugWordAlign) {
			std::cout << "Replaced word " << iTW << " with " << TWAltered << endl;
			std::cout << NewSentence << endl;
		}
	}
	string AltFName = "d:\\guten\\temp\\alt.txt";
	string AltNLPFile = "d:\\guten\\temp\\alt.txt.xml";
	string AltModFile = "d:\\guten\\temp\\alt.txt.mod";
//#define RECREATE_FILE
#ifdef RECREATE_FILE
	{
		fstream fileAltered;
		if (SafeOpen(fileAltered, AltFName, ios::out)) {
			for (uint ias = 0; ias < SentencesAltered.size(); ias++) {
				fileAltered << SentencesAltered[ias];
			}
		}
	}
	DoNLPOnFile(AltFName, AltNLPFile);
	const bool bAddToDB = true;
	ConvertStanfordOutput("d:\\guten\\temp", AltNLPFile, ".mod", !bAddToDB,
							BaseWordCount, DepTypes, BasicTypeLists,
							DynamicTypeListsNames);
#endif // #ifdef RECREATE_FILE
	ClearSentenceRecs();
	Param.Name = "Implem.Param.FnParam.LoadSentenceListOneMod.UseModName";
	Param.Val = "1";
	ImplemParamTbl[Param.Name] = Param;
	Param.Name = "Implem.Param.FnParam.LoadSentenceListOneMod.ModName";
	Param.Val = AltModFile;
	ImplemParamTbl[Param.Name] = Param;
	LoadSentenceListOneMod();

	vector<string> DepNames(DepTypes.size());
	MapBaseWordToCount::iterator itDep = DepTypes.begin();

	for (; itDep != DepTypes.end(); itDep++) {
		DepNames[itDep->second] = itDep->first;
	}

	float ScoreAlignParam = 0.01f;

	//AlignConfig[AlignParamMatchFoundConst] = 1.0f;
	//AlignConfig[AlignParamMatchFoundOPCR] = 1.0f;
	//AlignConfig[AlignParamNoMatchSurprising] = -1.0f;
	//AlignConfig[AlignParamNumPeersFactor] = 1.0f;
	//AlignConfig[AlignParamNumPeersFactorPerNumPeers] = 1.0f;
	//AlignConfig[AlignParamNumPeersFactorLimit] = 1.0f;
	//AlignConfig[AlignParamNumPeersFactorPerNumPeersLimit] = 1.0f;
	//AlignConfig[AlignParamNotFoundInPeerGrp] = -1.0f;
	//AlignConfig[AlignParamNotFoundInPeerGrpPerNumGrps] = -1.0f;
	//AlignConfig[AlignParamFoundInPeerGrp] = 1.0f;
	//AlignConfig[AlignParamFoundInPeerGrpPerNumGrps] = 1.0f;
	//AlignConfig[AlignParamAWordFactor] = 0.8f;
	//AlignConfig[AlignParamEvalDepAsIs] = 1.0f;
	//AlignConfig[AlignParamEvalDepPerNonZero] = 1.0f;

	vector<vector<float>> ConfigsTbl;
	for (int iRun = 0; iRun < 700; iRun++) {
		for (int iP = 0; iP < (int)NumAlignParams; iP++) {
			AlignConfig[iP] = -1.0f + (float)(rand() % 20) / 10.0f;
		}
		ConfigsTbl.push_back(AlignConfig);
	}

	float ScoreMax = -1000.0;
	int ScoreMaxRun = -1;
	vector<pair<float, vector<float> > > ScoreStack;
	for (uint iRun = 0; iRun < ConfigsTbl.size(); iRun++) {
		float Score = -1000.0f;
		AlignConfig = ConfigsTbl[iRun];
		srand(999); // hold down the random decisions in the following function for repeatability
		OneAlignConfigRun(	TamperedPosArr, UntamperedRecs, DepNames, ScoreAlignParam, 
							false, Score);
		ScoreStack.push_back(make_pair(Score, AlignConfig));
		//OneAlignConfigRun(TamperedPosArr, UntamperedRecs, DepNames, ScoreAlignParam, true);
		if (Score > ScoreMax) {
			ScoreMax = Score;
			ScoreMaxRun = iRun;
		}
	}
	sort(ScoreStack.begin(), ScoreStack.end());
	ScoreStack.erase(ScoreStack.begin(), ScoreStack.begin() + (ScoreStack.size() - 5));
	ScoreMax = ScoreStack.back().first;
	//ScoreMaxRun = ScoreStack.back().second;
	if (ScoreMaxRun < 0) {
		std::cout << "WordCorrect failed to find any good config\n";
		return;
	}

	std::cout << "Attempt to improve on " << ScoreMax << "\n";

	vector<vector<pair<float, vector<float> > > > ScoreStackStack;
	ScoreStackStack.push_back(ScoreStack);
	int NumSearches = 0;
	const int cMaxSearches = 10000;
	vector<float> TopScoringConfig = ScoreStack.back().second;
	while (NumSearches < cMaxSearches) {
		cout << "Num searches so far " << NumSearches << endl;
		while (ScoreStackStack.back().size() == 0) {
			std::cout << "Popping ScoreStackStack\n";
			ScoreStackStack.pop_back();
			if (ScoreStackStack.size() == 0) {
				break;
			}
		}
		if (ScoreStackStack.size() == 0) {
			break;
		}
		vector<pair<float, vector<float> > >& TopScoreStack = ScoreStackStack.back();
		vector<float> BestRandConfig = TopScoreStack.back().second;  // ConfigsTbl[ScoreMaxRun];
		TopScoreStack.pop_back();
		ConfigsTbl.clear();
		const float TestChange = 0.1f;
		for (float iBoth = -1.0f; iBoth < 2.0f; iBoth += 2.0f) {
			for (int iP = 0; iP < (int)NumAlignParams; iP++) {
				AlignConfig = BestRandConfig;
				AlignConfig[iP] = max(-1.0f, min(1.0f, AlignConfig[iP] + (iBoth * TestChange)));
				ConfigsTbl.push_back(AlignConfig);
			}

		}

		float ScoreMaxImp = -1000.0;
		ScoreMaxRun = -1;
		vector<pair<float, vector<float> > > ScoreStackImp;
		for (uint iRun = 0; iRun < ConfigsTbl.size(); iRun++) {
			float Score = -1000.0f;
			AlignConfig = ConfigsTbl[iRun];
			srand(999); // hold down the random decisions in the following function for repeatability
			OneAlignConfigRun(	TamperedPosArr, UntamperedRecs, DepNames, ScoreAlignParam,
								false, Score);
			NumSearches++;
			ScoreStackImp.push_back(make_pair(Score, AlignConfig));
			//OneAlignConfigRun(TamperedPosArr, UntamperedRecs, DepNames, ScoreAlignParam, true);
			if (Score > ScoreMaxImp) {
				ScoreMaxImp = Score;
				ScoreMaxRun = iRun;
			}
		}
		sort(ScoreStackImp.begin(), ScoreStackImp.end());
		ScoreStackImp.erase(ScoreStackImp.begin(), ScoreStackImp.begin() + (ScoreStackImp.size() - 5));
		ScoreMaxImp = ScoreStackImp.back().first;

		if (ScoreMaxImp > ScoreMax) {
			std::cout << "Score improved to " << ScoreMaxImp << endl;
			ScoreMax = ScoreMaxImp;
			TopScoringConfig.clear();
			TopScoringConfig = ScoreStackImp.back().second;
			ScoreStackStack.push_back(ScoreStackImp);
		}
		else {
			std::cout << "Score did not improve\n";
		}
	}

	cout << "Testing\n";
	AlignConfig = TopScoringConfig;
	float Score;
	OneAlignConfigRun(	TamperedPosArr, UntamperedRecs, DepNames, ScoreAlignParam,
						true, Score);
	cout << "Best scoring config is: \n";
	for (int iP = 0; iP < (int)NumAlignParams; iP++) {
		std::cout << "\t" << TopScoringConfig[iP] << endl;
	}
	bDebugWordAlign = true;
	OneAlignConfigRun(TamperedPosArr, UntamperedRecs, DepNames, ScoreAlignParam,
						false, Score);


	std::cout << "aahdone\n";
}

void CGotitEnv::OneAlignConfigRun(	vector<int>& TamperedPosArr, 
									vector<SSentenceRec>& UntamperedRecs, 
									vector<string>& DepNames, float ScoreAlignParam, 
									bool bOOB, float& ScoreRet)
{
	ScoreRet = -1000.0f;
	float NumUndecided = 0.0f;
	float NumOrigWins = 0.0f;
	float NumAltWins = 0.0f;

	uint iStart = 0;
	uint iEnd = SentenceRec.size() / 2;
	if (bOOB) {
		iStart = iEnd;
		iEnd = SentenceRec.size();
	}

	for (uint isr = iStart; isr < iEnd; isr++) {
		int iAltered = TamperedPosArr[isr];
		vector<DepRec>& DepsAlt = SentenceRec[isr].Deps;
		vector<DepRec>& DepsOrig = UntamperedRecs[isr].Deps;
		if (bDebugWordAlign) {
			std::cout << "Was: " << UntamperedRecs[isr].Sentence << endl;
			std::cout << "Now: " << SentenceRec[isr].Sentence << endl << endl;
		}
		vector<DepRec> AltOnlyDeps;
		vector<DepRec> OrigOnlyDeps;
		for (uint iDepAlt = 0; iDepAlt < DepsAlt.size(); iDepAlt++) {
			if (DepsAlt[iDepAlt].Gov >= SentenceRec[isr].OneWordRec.size()) {
				continue;
			}
			// if it makes reference to the altered word we know this is a 
			// dep rec you won't find in the othe, so don't bother looking further
			if (DepsAlt[iDepAlt].Gov == iAltered
				|| DepsAlt[iDepAlt].Dep == iAltered) {
				AltOnlyDeps.push_back(DepsAlt[iDepAlt]);
				continue;
			}
			bool bRecFound = false;
			for (uint iDepOrig = 0; iDepOrig < DepsOrig.size(); iDepOrig++) {
				if (DepsOrig[iDepOrig].Gov >= UntamperedRecs[isr].OneWordRec.size()) {
					continue;
				}
				if (DepsAlt[iDepAlt] == DepsOrig[iDepOrig]) {
					bRecFound = true;
					break;
				}
			}
			if (!bRecFound) {
				AltOnlyDeps.push_back(DepsAlt[iDepAlt]);
			}
		}

		for (uint iDepOrig = 0; iDepOrig < DepsOrig.size(); iDepOrig++) {
			if (DepsOrig[iDepOrig].Gov >= UntamperedRecs[isr].OneWordRec.size()) {
				continue;
			}
			// if it makes reference to the altered word we know this is a 
			// dep rec you won't find in the othe, so don't bother looking further
			if (DepsAlt[iDepOrig].Gov == iAltered
				|| DepsAlt[iDepOrig].Dep == iAltered) {
				OrigOnlyDeps.push_back(DepsOrig[iDepOrig]);
				continue;
			}
			bool bRecFound = false;
			for (uint iDepAlt = 0; iDepAlt < DepsAlt.size(); iDepAlt++) {
				if (DepsAlt[iDepAlt].Gov >= SentenceRec[isr].OneWordRec.size()) {
					continue;
				}
				if (DepsAlt[iDepAlt] == DepsOrig[iDepOrig]) {
					bRecFound = true;
					break;
				}
			}
			if (!bRecFound) {
				OrigOnlyDeps.push_back(DepsOrig[iDepOrig]);
			}
		}

		if (bDebugWordAlign) {
			std::cout << "Altered recs\n";
		}
		float ScoreOrig = 0.0f;
		float ScoreAlt = 0.0;
		int NumAltOnlyNonZeroRet = 0;
		int NumOrigOnlyNonZeroRet = 0;
		for (int iBoth = 0; iBoth < 2; iBoth++) {
			vector<DepRec>* pOnlyDeps = &AltOnlyDeps;
			vector<WordRec>* pWordRec = &(SentenceRec[isr].OneWordRec);
			if (iBoth == 1) {
				pOnlyDeps = &OrigOnlyDeps;
				pWordRec = &(UntamperedRecs[isr].OneWordRec);
				if (bDebugWordAlign) {
					std::cout << "Original recs\n";
				}
			}
			for (uint iDep = 0; iDep < pOnlyDeps->size(); iDep++) {
				if (bDebugWordAlign) {
					std::cout << "Nonmatched dep rec " << DepNames[(*pOnlyDeps)[iDep].iDep]
						<< ", " << (*pWordRec)[(*pOnlyDeps)[iDep].Gov].Word
						<< ", " << (*pWordRec)[(*pOnlyDeps)[iDep].Dep].Word
						<< endl;
				}
				float LocalScore = 0.0f;
				EvaluateDep((*pOnlyDeps)[iDep], DepNames, (*pWordRec), LocalScore);
				if (bDebugWordAlign) {
					std::cout << "EvaluateOnDep returned " << LocalScore << endl;
				}
				if (iBoth == 0) {
					if (LocalScore != 0.0f) {
						if (((*pOnlyDeps)[iDep].Gov == TamperedPosArr[iDep])
							|| ((*pOnlyDeps)[iDep].Dep == TamperedPosArr[iDep])) {
							ScoreAlt += AlignConfig[AlignParamAWordFactor] * LocalScore;
						}
						else {
							ScoreAlt += (1.0f - AlignConfig[AlignParamAWordFactor]) * LocalScore;
						}
						NumAltOnlyNonZeroRet++;
					}
				}
				else {
					if (LocalScore != 0.0f) {
						if (((*pOnlyDeps)[iDep].Gov == TamperedPosArr[iDep])
							|| ((*pOnlyDeps)[iDep].Dep == TamperedPosArr[iDep])) {
							ScoreOrig += AlignConfig[AlignParamAWordFactor] * LocalScore;
						}
						else {
							ScoreOrig += (1.0f - AlignConfig[AlignParamAWordFactor]) * LocalScore;
						}
						NumOrigOnlyNonZeroRet++;
					}
				}
			}

		}
		float fac = 0.0f;
		if (NumOrigOnlyNonZeroRet != 0) {
			fac = (AlignConfig[AlignParamEvalDepPerNonZero] / (float)NumOrigOnlyNonZeroRet);
		}
		ScoreOrig *= AlignConfig[AlignParamEvalDepAsIs] + fac;
		fac = 0.0f;
		if (NumAltOnlyNonZeroRet != 0) {
			fac = (AlignConfig[AlignParamEvalDepPerNonZero] / (float)NumAltOnlyNonZeroRet);
		}
		ScoreAlt *= AlignConfig[AlignParamEvalDepAsIs] + fac;

		if (bDebugWordAlign) {
			std::cout << "Orig score is " << ScoreOrig << " and alt score is " << ScoreAlt << endl;
		}
		if ((float)abs(ScoreOrig - ScoreAlt) < 0.01f) {
			if (bDebugWordAlign) {
				std::cout << "Orig-Alt undecided!\n";
			}
			NumUndecided += 1.0f;
		}
		else if (ScoreOrig > ScoreAlt) {
			if (bDebugWordAlign) {
				std::cout << "Orig wins!\n";
			}
			NumOrigWins += 1.0f;
		}
		else {
			if (bDebugWordAlign) {
				std::cout << "Alt wins!\n";
			}
			NumAltWins += 1.0f;
		}
		if (bDebugWordAlign) {
			std::cout << "--------------------------------------------------------------\n";
		}
#ifdef OLD_CODE
		uint MaxDeps = min(DepsAlt.size(), DepsOrig.size());
		int NumDepsDiff = 0;
		for (uint iDep = 0; iDep < MaxDeps; iDep++) {
			if (DepsOrig[iDep].Gov >= UntamperedRecs[isr].OneWordRec.size()) {
				continue;
			}
			if (DepsAlt[iDep].Gov >= SentenceRec[isr].OneWordRec.size()) {
				continue;
			}
			if (!(DepsAlt[iDep] == DepsOrig[iDep])
				|| UntamperedRecs[isr].OneWordRec[DepsOrig[iDep].Gov].Word
				!= SentenceRec[isr].OneWordRec[DepsAlt[iDep].Gov].Word
				|| UntamperedRecs[isr].OneWordRec[DepsOrig[iDep].Dep].Word
				!= SentenceRec[isr].OneWordRec[DepsAlt[iDep].Dep].Word) {
				if (bDebugWordAlign) {
					cout << "Dep delta at " << iDep << ". Was " << DepNames[DepsOrig[iDep].iDep]
						<< ", " << UntamperedRecs[isr].OneWordRec[DepsOrig[iDep].Gov].Word
						<< ", " << UntamperedRecs[isr].OneWordRec[DepsOrig[iDep].Dep].Word
						<< ". Now " << DepNames[DepsAlt[iDep].iDep]
						<< ", " << SentenceRec[isr].OneWordRec[DepsAlt[iDep].Gov].Word
						<< ", " << SentenceRec[isr].OneWordRec[DepsAlt[iDep].Dep].Word
						<< endl;
				}
				NumDepsDiff++;
			}
		}
		if (bDebugWordAlign) {
			cout << "--------------------------------------------------------------\n";
		}
#endif // OLD_CODE
	}
	ScoreRet = NumOrigWins - NumAltWins - (ScoreAlignParam * NumUndecided);
	std::cout << "bOOB (" << bOOB << ") Final score: " << ScoreRet << " with " << NumAltWins << " bad and " << NumUndecided <<  " undecided " << endl;

}
bool DepRec::operator== (DepRec& Other)
{
	if (iDep == Other.iDep && Gov == Other.Gov && Dep == Other.Dep) {
		return true;
	}
	return false;
}
