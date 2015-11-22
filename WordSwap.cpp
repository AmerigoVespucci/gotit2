// WordSwap.cpp :Find which if two words is the more likely.
//

#include "stdafx.h"

#include "MascReader.h"
#include "RF/librf/instance_set.h"
#include "RF/librf/random_forest.h"

using namespace rapidxml;
using namespace std;
using namespace librf;

#define sout std::cout

bool NotPureAlpha(string& sw) 
{
	return (find_if(sw.begin(), sw.end(),
			not1(ptr_fun((int(*)(int))isalpha))) != sw.end());

}

#ifdef OLD_CODE

scarry to remove this code. The code was never really replaced by TaskFns

void CGotitEnv::LearnWordSwap(	)
{
	vector<CGroupingRec*> CombGrps;
	map<string, int> GroupsSaved;
	vector<SGrpSeed> SeedTbl;

	static const uint cNumTestSeeds = 20;
	static const uint cNumSeedPartners = 5;

	// First create PosNames vector
	vector<string> PosNames;
	for (uint ip = 0; ip < PatGroups.size(); ip++) {
		// place this before search in case we access the members array which may
		// be 0 or too short
		if (PatGroups[ip]->pfinder->group_name_template != "AllPOSWords") {
			continue;
		}
		string OnePosName = PatGroups[ip]->members[0];
		PosNames.push_back(OnePosName);
	}

	
	{
		fstream fileIn;
		if (SafeOpen(fileIn, "SwapGroups.bin", ios::binary | ios::in)) {
			uint NumGroups;
			fileIn.read((char *)&(NumGroups), sizeof(NumGroups));
			for (uint ig = 0; ig < NumGroups; ig++) {
				CGroupingRec * patgrp = new CGroupingRec;
				patgrp->Load(fileIn, anc_pat_finder_list, PatGrpTbl, BaseWordCount);
				PatGroups.push_back(patgrp);
				CombGrps.push_back(patgrp);
			}
		}
		ifstream fileListIn("SeedGroupsList.txt");
		string SavedSeedName;
		while (getline(fileListIn, SavedSeedName)) {
			map<string, int>::iterator itgs = GroupsSaved.find(SavedSeedName);
			if (itgs == GroupsSaved.end()) {
				uint SeedNum = SeedTbl.size();
				SeedTbl.push_back(SGrpSeed());
				//SeedTbl.resize(SeedNum + 1);
				SeedTbl[SeedNum].SeedName = SavedSeedName;
				GroupsSaved[SavedSeedName] = SeedNum;
			}
		}
	}



	//if (!bImplementAsPair) {
	//	static const uint cMaxMUAnchor = 30;
	//	static const uint cMinMUAnchor = 10;
	//	int SelChosen = (rand() % cMaxMUAnchor) + cMinMUAnchor;
	//	Anchor = WordsInOrder[SelChosen].first;
	//}
	
	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	// turn this on only if needed
	if (ImplemParamTbl["bOrderWordsOnLearnSwap"].Val == "true") {
		OrderWords(BaseWordCount, WordsInOrder);
	}
	
	vector<string> Seeds;
	for (uint is = 1110; is < 4010; is+=3) {
		string sw = WordsInOrder[is].Name;
		if (!NotPureAlpha(sw)) {
			Seeds.push_back(sw);
		}
		if (Seeds.size() >= cNumTestSeeds) {
			break;
		}
	}
	vector<string> SeedPartners;
	for (uint is2 = 1010; is2 < 1300; is2 += 1) {
		string sw = WordsInOrder[is2].Name;
		if (!NotPureAlpha(sw)) {
			SeedPartners.push_back(sw);
		}
	}
	//const int SeedsTestLimit = 4; // Seeds.size();
	//Seeds.clear();
	//Seeds.push_back("dumont");
	//Seeds.push_back("sanction");

	// Add groups for any names in our list that are not already saved on disk

	// Greate POS groups, groups based on POS of neighbors in sample sentences
	// for each seed
	ofstream fileListOut("SeedGroupsList.txt", ios::app);
	vector<string> SeedsRecorded;
	for (uint is = 0; is < Seeds.size(); is++) {
		map<string, int>::iterator itSaved = GroupsSaved.find(Seeds[is]);
		if (itSaved != GroupsSaved.end()) {
			continue;
		}
		//GroupsSaved[Seeds[is]] = true;
		uint SeedNum = SeedTbl.size();
		SeedTbl.push_back(SGrpSeed());
		SeedTbl[SeedNum].SeedName = Seeds[is];
		GroupsSaved[Seeds[is]] = SeedNum;
		vector<CGroupingRec*> RelevantGroups;
		CreateGrpsFromSeed(Seeds[is], RelevantGroups);
		CombGrps.insert(CombGrps.end(), RelevantGroups.begin(), RelevantGroups.end());
		SeedsRecorded.push_back(Seeds[is]);
		cout << "Swap Groups found for " << Seeds[is] << endl;
		// update SwapGroups.bin file with replacement
		if (((is % 100) == 0) || (is == (Seeds.size()-1))) {
			fstream fileOut;
			if (SafeOpen(fileOut, "SwapGroups.bin", ios::binary | ios::out)) {
				uint NumGroups = CombGrps.size();
				fileOut.write((char *)&(NumGroups), sizeof(NumGroups));
				for (uint ig = 0; ig < NumGroups; ig++) {
					CGroupingRec * patgrp = CombGrps[ig];
					cerr << "Error. Code here resurrected but Store here deprecated\n";
					patgrp->Store(fileOut); 
				}
				for (uint isr = 0; isr < SeedsRecorded.size(); isr++) {
					fileListOut << SeedsRecorded[isr] << endl;
				}
				SeedsRecorded.clear();
				cerr << "Recorded: " << is << " seeds\' groups in total\n";
			}
		}
	} // end of loop over seeds
	fileListOut.close();
	//Seeds.clear(); // ---------------------------------------------------------------------------------> remove
	//map<string, int> SeedNameMap; // Maps the name of a seed to its index in Seeds
	//for (uint is = 0; is < Seeds.size(); is++) {
	//	SeedTbl.push_back(SGrpSeed());
	//	SeedTbl[is].SeedName = Seeds[is];
	//	SeedNameMap[Seeds[is]] = is;
	//}
	map<string, int> PosNameMap;
	for (uint ipg = 0; ipg < PosNames.size(); ipg++) {
		PosNameMap[PosNames[ipg]] = ipg;
	}

	uint NumGroups = CombGrps.size();
	for (uint ig = 0; ig < NumGroups; ig++) {
		CGroupingRec * patgrp = CombGrps[ig];
		string POSName, W;
		bool bPrec;
		if (patgrp->Name.find("%]Prec[%") != string::npos) {
			POSName = patgrp->members[0];
			W = patgrp->members[1];
			bPrec = true;
		}
		else {
			POSName = patgrp->members[1];
			W = patgrp->members[0];
			bPrec = false;
		}
		map<string, int>::iterator isnm = GroupsSaved.find(W);
		if (isnm != GroupsSaved.end()) {
			uint is = isnm->second;
			map<string, int>::iterator ipnm = PosNameMap.find(POSName);
			if (ipnm != PosNameMap.end()) {
				if (bPrec) {
					SeedTbl[is].iPrecGrps[ipnm->second] = ig;
				}
				else {
					SeedTbl[is].iFollowGrps[ipnm->second] = ig;
				}
			}
		}
	}
	
	vector<stringstream> RFDataArr(Seeds.size()); // make sure these don't go out of scope. Addresses taken.
	vector<stringstream> RFDataArrOOB(Seeds.size());

	for (uint is = 0; is < Seeds.size(); is++) {
		uint iis = GroupsSaved[Seeds[is]];
		SeedTbl[iis].pRFData = &(RFDataArr[is]);
		SeedTbl[iis].pRFDataOOB = &(RFDataArrOOB[is]);
		CreateWordGroup(&(SeedTbl[iis]), CombGrps);
		sout << "Word group for " << Seeds[is] << " is " << SeedTbl[iis].SortedPeers.size() << " long\n";
		for (uint ipe = 0; ipe < SeedTbl[iis].SortedPeers.size(); ipe++) {
			sout << SeedTbl[iis].SortedPeers[ipe] << " (" << BaseWordCount[SeedTbl[iis].SortedPeers[ipe]] << "), ";
		}
		sout << endl;
		sout << "\n RF Data: \n";
		sout << SeedTbl[iis].pRFData->str() << endl;
		sout << "\n RF OOB Data: \n";
		sout << SeedTbl[iis].pRFDataOOB->str() << endl << endl;
	}


	fstream fSwapScores("SwapScores.csv", fstream::out);
	fSwapScores << ",";
	//for (uint is = 0; is < Seeds.size(); is++) {
	//	fSwapScores << Seeds[is] << ",";
	//}
	fSwapScores << ",";
	fSwapScores << endl;

	for (uint iTestSeed = 0; iTestSeed < Seeds.size(); iTestSeed++ ) {
		//uint iTestSeed = 1; // replace with loop over seeds
		uint iSeedTblSeed = GroupsSaved[Seeds[iTestSeed]];

		vector<float> TestPeerCumScores;

		for (	uint iTestPeer = 0; 
				iTestPeer < SeedTbl[iSeedTblSeed].SortedPeers.size(); 
				iTestPeer++) {
			vector<CGroupingRec> OneRFGrps(PosNames.size() * 2); // follow and prec for each seed
			vector<vector<CGroupingRec> > RFGrpsTbl(Seeds.size(), OneRFGrps);
			for (uint is = 0; is < Seeds.size(); is++) {
				const uint cMinUses = 300; // minimum required in the word count in order to have Prec and following groups filled sufficiently for rf
				uint SeedNumUses = BaseWordCount[Seeds[is]];
				uint TotalUses = SeedNumUses;
				uint iis = GroupsSaved[Seeds[is]]; // iis is the seed table index not the index in this loop
				//vector<CGroupingRec> RFGrps(SeedTbl[iis].iFollowGrps.size() * 2); // follow and prec for each seed
				vector<CGroupingRec>& RFGrps = RFGrpsTbl[is];
				for (int iBoth = 0; iBoth < 2; iBoth++) {
					map<int, int>* pMap;
					if (iBoth == 0) {
						pMap = &(SeedTbl[iis].iFollowGrps);
					}
					else {
						pMap = &(SeedTbl[iis].iPrecGrps);

					}
					for (	map<int, int>::iterator itrfg = pMap->begin(); 
							itrfg != pMap->end(); itrfg++) {
						CGroupingRec * patgrp = CombGrps[itrfg->second];
						int AddToIndex = ((iBoth == 0) ? 0 : pMap->size());
						RFGrps[AddToIndex + itrfg->first].MemberLinks 
							= patgrp->MemberLinks;
					}
				}
				if (TotalUses >= cMinUses) {
					sout << "TotalUses for " << Seeds[is] << " is " << TotalUses << ", which means no peer\n";
				}
				//for (		uint iPeer = 0;
				//				iPeer < SeedTbl[iis].SortedPeers.size()
				//			&&	TotalUses < cMinUses;
				//			iPeer++) 
				if (TotalUses < cMinUses) {
					uint riPeer = rand() % SeedTbl[iis].SortedPeers.size();
					//uint riPeer = iPeer;
					if (is == iTestSeed) {
						riPeer = iTestPeer;
						cout << "Cancelling random for test seed " << is << " called " << Seeds[is] << endl;
					}
					string PeerName = SeedTbl[iis].SortedPeers[riPeer];
					map<string, int>::iterator itsg 
						= GroupsSaved.find(PeerName);
					if (itsg == GroupsSaved.end()) {
						continue;
					}
					TotalUses += BaseWordCount[PeerName];
					sout << "TotalUses " << BaseWordCount[PeerName] << " for " << PeerName << " supporting " << Seeds[is] << endl;
					SeedTbl[iis].RFPeers.push_back(PeerName);
					uint iip = itsg->second;
					for (int iBoth = 0; iBoth < 2; iBoth++) {
						map<int, int>* pMap;
						if (iBoth == 0) {
							pMap = &(SeedTbl[iip].iFollowGrps);
						}
						else {
							pMap = &(SeedTbl[iip].iPrecGrps);
						}
						for (	map<int, int>::iterator itrfg = pMap->begin(); 
								itrfg != pMap->end(); itrfg++) {
							CGroupingRec * patgrp = CombGrps[itrfg->second];
							int AddToIndex = ((iBoth == 0) ? 0 : pMap->size());
							CGroupingRec * patgrpDest 
								= &(RFGrps[AddToIndex + itrfg->first]);
							for (	MapMemberToGroup::iterator itms 
										= patgrp->MemberLinks.begin();
									itms != patgrp->MemberLinks.end();
									itms++) {
								MapMemberToGroup::iterator itmsd 
									= patgrpDest->MemberLinks.find(itms->first);
								if (itmsd == patgrpDest->MemberLinks.end()) {
									patgrpDest->MemberLinks[itms->first] = itms->second;
								}
							} // end loop over member links
						} //  end loop over POS groups
					} // end iBoth for prec and follows

				} // end loop over Peers
			} // end loop over Seeds

			for (uint is = 0; is < Seeds.size(); is++) {
				if (is != iTestSeed) {
					continue;
				}
				float CumScore = 0.0;
				int NumScores = 0;


				for (uint is2 = 0; is2 < cNumSeedPartners /*Seeds.size()*/; is2++) {
					uint ris = rand() % Seeds.size();
					if (is == ris) {
						continue;
					}

					vector<CGroupingRec*> TestGrps;
					//map<int, int>::iterator itgrps = SeedTbl[GroupsSaved[Seeds[is]]].iPrecGrps.begin();
					uint iis = is;
					SGrpSeed& Seed0 = SeedTbl[GroupsSaved[Seeds[is]]];
					SGrpSeed& Seed1 = SeedTbl[GroupsSaved[Seeds[ris]]];
					for (uint iBoth = 0; iBoth < 2; iBoth++, iis = ris) {
						for (	uint iRFGrp = 0; iRFGrp < RFGrpsTbl[iis].size(); 
								iRFGrp++) {
							TestGrps.push_back(&RFGrpsTbl[iis][iRFGrp]);
						}
					}
					//for (uint ig = 0; ig < CombGrps.size(); ig++) {
					//	CGroupingRec * patgrp = CombGrps[ig];
					//	if (	(patgrp->members[0] == Seeds[is])
					//		||	(patgrp->members[1] == Seeds[is])
					//		||	(patgrp->members[0] == SeedPartners[ris])
					//		||	(patgrp->members[1] == SeedPartners[ris])) {
					//		TestGrps.push_back(patgrp);
					//	}
					//}
					//srand(173);
					sout << "Swap result for " << Seeds[is] << " with " << Seeds[ris] << endl;
					float ScoreBroad, ScoreSel;
					vector<int> SelGroupIndices;
					CreateRFErrExList(true, Seeds[is], Seeds[ris],
						ScoreBroad, ScoreSel, SelGroupIndices,
						Seed0, Seed1);
					//fSwapScores << ScoreBroad << ",";
					if (ScoreBroad > 0.0f) {
						CumScore += ScoreBroad;
						NumScores++;
					}
					/* commment out until we decide what to do with top groups
					sout << "top groups: ";
					for (uint isg = 0; isg < 6; isg++) {
					int iSel = SelGroupIndices[isg];
					if (iSel < 0 || iSel >= (int)TestGrps.size()) {
					continue;
					}
					CGroupingRec * patgrp = TestGrps[SelGroupIndices[isg]];
					if ((patgrp->members[0] == Seeds[is])
					|| (patgrp->members[1] == Seeds[is])) {
					sout << patgrp->Name << ", ";
					}
					}
					sout << endl;
					// top group processing end
					*/

					// individual comparison using error instead of swap
					//CreateRFErrExList(BaseWordCount, WordsInOrder, ModNames, TestGrps, 
					//					false, Seeds[is], Seeds[is2]);
				}// end inner loop over seeds
				if (NumScores > 0) {
					fSwapScores << Seeds[is] << ",";
					CumScore /= (float)NumScores;
					fSwapScores << CumScore << ",";
					fSwapScores << endl;
				}
				TestPeerCumScores.push_back(CumScore);
			} // end outer loop over seeds which has deprecated to just the test seed
		} // end loop over test peers to evaluate the effectiveness of each peer
		float Sum = 0.0f;
		float Avg = 0.0f;
		for (uint iTestPeer = 0;
				iTestPeer < TestPeerCumScores.size();
				iTestPeer++) {
			Sum += TestPeerCumScores[iTestPeer];
		}
		if (TestPeerCumScores.size()) {
			Avg = Sum / (float)(TestPeerCumScores.size());
		}
		sout << "Average cumumlative score: " << Avg << endl;
		stringstream RFLabel; //  ("ThatExOOB.csv");
		stringstream RFLabelOOB; //  ("ThatExLabelOOB.csv");
		for (uint iTestPeer = 0;
				iTestPeer < TestPeerCumScores.size();
				iTestPeer++) {
			string PeerName = SeedTbl[iSeedTblSeed].SortedPeers[iTestPeer];
			sout << "Test score for " << PeerName << " is " << TestPeerCumScores[iTestPeer];
			if (TestPeerCumScores[iTestPeer] > Avg) {
				sout << " above average\n";
				if (SeedTbl[iSeedTblSeed].bOOB[iTestPeer]) {
					RFLabelOOB << "1.0\n";
				}
				else {
					RFLabel << "1.0\n";
				}
			}
			else {
				sout << " below average\n";
				if (SeedTbl[iSeedTblSeed].bOOB[iTestPeer]) {
					RFLabelOOB << "0.0\n";
				}
				else {
					RFLabel << "0.0\n";
				}
			}
		}
		sout << "\n RF Label: \n";
		sout << RFLabel.str() << endl;
		sout << "\n RF OOB Label: \n";
		sout << RFLabelOOB.str() << endl;
		sout << "Results for " << SeedTbl[iSeedTblSeed].SeedName << ":\n";
		InstanceSet * LearnSet = InstanceSet::LoadFromSS(*(SeedTbl[iSeedTblSeed].pRFData), RFLabel, true);
		InstanceSet * LearnSetOOB = InstanceSet::LoadFromSS(*(SeedTbl[iSeedTblSeed].pRFDataOOB), RFLabelOOB, true);
		int K = (int)(floor(sqrt((double)78)));
		vector< pair< float, int> > ranking;
		{
			RandomForest rf(*LearnSet, 100, K);
			float sscore = rf.training_accuracy();
			float score = rf.testing_accuracy(*LearnSetOOB);
			sout << "rf on OOB score: " << score << " self score " << sscore << "\n";
			unsigned int seed = rand();
			rf.variable_importance(&ranking, &seed);
		}
		for (uint irnk = 0; irnk < ranking.size(); irnk++) {
			sout << "(" << ranking[irnk].first << ":" << ranking[irnk].second << "), ";
		}
		sout << endl;
		delete LearnSetOOB;
		delete LearnSet;
	} // end "loop" over seeds for which we want to find the optimal peer
	fSwapScores.close();
	//for (uint ig = 0; ig < CombGrps.size(); ig++) {
	//	delete CombGrps[ig];
	//}
}

#endif // OLD_CODE


static const int cMaxDistance = 8;

void CGotitEnv::InitSGrpSeed()
{
	string SeedName = ImplemParamTbl["Task.Param.InitSGrpSeed.SeedName"].Val;
	Seed0.SeedName = SeedName;
	Seed0.pRFData = new stringstream;
	Seed0.pRFDataOOB = new stringstream;
	Seed0.pRFLabel = new stringstream;
	Seed0.pRFLabelOOB = new stringstream;
	RFNumVars = 0;
	bStreamsAreNew = true;
}

void CGotitEnv::InitSGrpSeed1()
{
	string SeedName = ImplemParamTbl["Task.Param.InitSGrpSeed1.SeedName"].Val;
	Seed1.SeedName = SeedName;
}

// The purpose of this function is to test the groups a word belongs to 
// by seeing whether we can detect whether a sentence originally included 
// a word or not. Positive examples are sentences that created the seed word
// negative examples are either (a) random sentences that did not contain the 
// word, (b) sentences that contain a random alternate word.
// (a) and (b) might be enhanced by taking some other words from his group
// We check using different methods. (1) looking at distances from the test 
// word and seeing whether groups associated with neigbours of our test group are
// there. 
void CGotitEnv::TestByWordSwap()
{
	//select PatGroups
	//initialize Seed0 using xml
	//make function load from only one mod
	//create a separate function to run the RF

	vector<CPatGrpIter> NearGroups;
	CPatGrpIter itpgt = PatGrpMgr.getStartIterator();
	for (; PatGrpMgr.IsIteratorValid(itpgt); PatGrpMgr.getNextIterator(itpgt)) {
		CPatGrpWrapper PatGrp(itpgt);
		SPatternFinder* pFinder;
		if (PatGrp.getGrpFinder(pFinder)) {
			if (pFinder->group_name_template == "GrpSeedAncPrec") {
				if (PatGrp.getGrpParam(1) == Seed0.SeedName) {
					NearGroups.push_back(itpgt);
				}
			}
			else if (pFinder->group_name_template == "GrpSeedAncAfter") {
				if (PatGrp.getGrpParam(1) == Seed0.SeedName) {
					NearGroups.push_back(itpgt);
				}
			}
		}
	}
	string FirstOfPair;
	string SecondOfPair;
	//if (FirstOfPair == "") FirstOfPair = "for";
	//if (SecondOfPair == "") SecondOfPair = "president";

	stringstream& RFOut = *(Seed0.pRFData);
	stringstream& RFOutOOB = *(Seed0.pRFDataOOB);
	stringstream& RFLabel = *(Seed0.pRFLabel);; // ("ThatExLabel.csv");
	stringstream& RFLabelOOB = *(Seed0.pRFLabelOOB); //  ("ThatExLabelOOB.csv");

	if (bStreamsAreNew) {
		for (uint ip = 0; ip < NearGroups.size(); ip++) {
			////Headers.push_back(POSNames[ip]);
			//if (POSNames[ip] == ",") {
			//	RFOut << ",";
			//	RFOutOOB << ",";
			//}
			//else {
			CPatGrpWrapper PatGrp(NearGroups[ip]);
			string nm;
			PatGrp.getGrpName(nm);
			size_t pos;
			if ((pos = nm.find(",")) != string::npos) {
				nm.replace(pos, 1, "comma");
			}
			RFOut << nm << ",";
			RFOutOOB << nm << ",";
			RFNumVars++; // cuurently same as PatGrous.size but may change
			//}
		}
		bStreamsAreNew = false;
		RFOut << endl;
		RFOutOOB << endl;
	}
	//vector<SSentenceRec> SentenceRec;
	//vector<vector<string> > TestSentences;
	vector<bool> ErrSentenceLabels;
	//uint PrevSententenceRecSize = 0;
	//vector<vector<int> > Distances; // 2D table. Each row a test sentence. Each column  the distance of a pat group to the anchor
	uint NumIn = 0;
	uint NumOOB = 0;

	// create test sentences
	uint NumTestRecs = 0;
	uint cMinTestRecs = 300;
	//uint ModRuns = min(ModNamesCntrl.getSize() / 2, 500);
	uint NumLabel0 = 0;
	uint NumLabel1 = 0;
	int NumOverLoops; // the data preparation stage is called NumOverLoops times
	NumOverLoops = ((Seed0.RFPeers.size() > 0) ? 2 : 1);
	int iOverLoop = stoi(ImplemParamTbl["Task.Param.TestByWordSwap.iOverLoop"].Val);
	//if ((NumOOB + NumIn) > cMinTestRecs) {
	if (NumIn > cMinTestRecs) {
		ImplemParamTbl["Task.Param.TestByWordSwap.KeepGoing"].Val = "0";
		//break;
	}
	//int rim = rand() * ModNames.size() / RAND_MAX;

	//vector<string> OnlyIfWordsPresent; // NULL in  initial group creation
	//OnlyIfWordsPresent.push_back(Anchor);
	//ImplemParamTbl["Implem.Param.FnParam.LoadSentenceListOneMod.OnlyIfWordsPresent"].Val = Anchor;
	//LoadSentenceListOneMod();

	int RefIndex = -1;
	for (uint ir = 0; ir < SentenceRec.size(); ir++) {
		bool bLabel0 = true; // if true the sentence will be label 0 else label 1
		if (iOverLoop == 0) {
			FirstOfPair = Seed0.SeedName;
			SecondOfPair = Seed1.SeedName;
		}
		else {
			FirstOfPair = Seed0.RFPeers[rand() % Seed0.RFPeers.size()];
			SecondOfPair = Seed1.RFPeers[rand() % Seed1.RFPeers.size()];
		}
		string Anchor = FirstOfPair;

		if (NumLabel0 < NumLabel1) {
			bLabel0 = true;
			Anchor = FirstOfPair;
		}
		else {
			bLabel0 = false;
			Anchor = SecondOfPair;
		}
		bool bOOB = ((rand() % 2) == 0);
		if (iOverLoop > 0) { // on the second run we generate no OOB data
			// this means that we build a tree by mixing in Peers
			// but we test the tree on the real swaps only
			// The test data can be small (but not 0), the tree building
			// needs a serious number of examples
			bOOB = false;
		}
		if (bOOB) NumOOB++; else NumIn++;
		stringstream& ThisRFOut = (!bOOB ? RFOut : RFOutOOB);
		stringstream& ThisRFLabel = (!bOOB ? RFLabel : RFLabelOOB);
		vector<int> DistancesThisSentence(NearGroups.size(), 0);
		vector<WordRec>& Words = SentenceRec[ir].OneWordRec;
		//TestSentences.push_back(vector<string>());
		for (uint iw = 0; iw < Words.size(); iw++) {
			if (Words[iw].Word == Anchor) {
				RefIndex = iw;
				break;
			}
			//TestSentences.back().push_back(Words[iw].WordBase);
		} // end search for first anchor word
		if (RefIndex == -1) {
			std::cerr << "Anchor " << Anchor << "not found\n";
			continue;
		}
		int ErrRep = -1; // position in the sentence of the word we want to switch
		// modify about half the test cases by replacing
		// words close to an anchor word
		if (bLabel0) {
			ErrSentenceLabels.push_back(false);
			NumLabel0++;
		}
		else {
			ErrSentenceLabels.push_back(true);
			NumLabel1++;
		} // end if we modify this guy

		//if (ErrSentenceLabels.size() != (NumIn + NumOOB)) {
		//	cerr << "ErrLabels out of sync\n";
		//}
		//TimeStamp(string("pt 2"));

		int FirstWordAnalyzed = max(RefIndex - cMaxDistance + 1, 0);
		int LastWordAnalyzed = min(RefIndex + cMaxDistance - 1, (int)(Words.size()) - 1);
		for (int iw = FirstWordAnalyzed; iw <= LastWordAnalyzed; iw++) {
			if (iw == RefIndex) {
				continue;
			}
			bool bFound = false;
			//#define TEST_WITH_POS
			static const float cRealityCheckThresh = 0.05f;
			//TimeStamp(string("pt 3"));
			for (uint ip = 0; ip < NearGroups.size(); ip++) {
				//TimeStamp(string("pt 4"));
				CPatGrpWrapper PatGrp(NearGroups[ip]);
				SGroupingLink* pEl = PatGrp.getGrpElByName(Words[iw].Word);
				if (pEl != NULL)  {
					float RealityCheck = (float)pEl->NumOccurs / (float)pEl->NumExist;
					RealityCheck = 1.0; // ----------------------------------------------------------> Make this an implementation parameter
					if (RealityCheck < cRealityCheckThresh) {
						continue;
					}
					if (!pEl->bStrong) {
						continue;
					}
					int WouldBe = ((iw > RefIndex)
						? (cMaxDistance - (iw - RefIndex))
						: (-(cMaxDistance - (RefIndex - iw))));
					// test that this is the closest instance to the ref
					if (abs(DistancesThisSentence[ip]) < abs(WouldBe)) {
						DistancesThisSentence[ip] = WouldBe;
					}
				} // end loop over members of pat group
			} // end loop over pat grousps
		} // end loop over words in sentence
		for (uint id = 0; id < DistancesThisSentence.size(); id++) {
			ThisRFOut << DistancesThisSentence[id] << ".0,";
		}
		ThisRFOut << endl;
		if (ErrSentenceLabels.back()) {
			ThisRFLabel << "1.0" << endl;
		}
		else {
			ThisRFLabel << "0.0" << endl;
		}
	} // end loop over sentences
	//PrevSententenceRecSize = SentenceRec.size();

	cout << "Num LearnSet " << NumIn << " and Num OOB " << NumOOB << " for sym " << FirstOfPair << " and " << SecondOfPair << " in loop " << iOverLoop << endl;

}

