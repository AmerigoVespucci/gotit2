// WordAligned.cpp :Find a group to expand a specific word.
//

#include "stdafx.h"

#include "MascReader.h"

const int cAcceptableNumOccurs = 3;
const int cMinGlocalCount = 100; // very arbitrary number. Depends on size of corpus

float CalcOPC(int NumOccurs, int GlobalCount) {
	// bias away from very small NumOccurs which normally have correspondingly
	// large 1/GlobalCounts. Problem is that a value that should be, say, 0.1 for NumOccurs
	// will either carry a 0 or, in lucky cases 1 which will throw off the OPC
	if (NumOccurs < 1 || GlobalCount < cMinGlocalCount) {
		return 0.0f;
	}
	float Bias = (float)min(NumOccurs, cAcceptableNumOccurs) / (float)cAcceptableNumOccurs;

	return (float)NumOccurs * Bias * Bias / (float)GlobalCount;

}

bool CGotitEnv::CalcGrpOPC(CPatGrpWrapper& PatGrp, float& OPCHCount, float& OPCMCount, float& OPCLCount)
{
	const int cMidCountThresh = 3000;
	const int cHighCountThresh = 20000;

	int NumSEls = PatGrp.getNumStrongEls();
	if (NumSEls == 0) {
		return false; // no good val created
	}
	int NumHRatios = 0;
	int NumMRatios = 0;
	int NumLRatios = 0;
	OPCHCount = 0.0f;
	OPCMCount = 0.0f;
	OPCLCount = 0.0f;
	//GOPC = PatGrp.getGrpAvgOccursPerCount();
	for (int iElPeer = 0; iElPeer < NumSEls; iElPeer++) {
		SGroupingLink * pElPeer = PatGrp.getStrongElByIndex(iElPeer);
		MapBaseWordToCount::iterator itWPeerCount
				= BaseWordCount.find(pElPeer->W);
		if (itWPeerCount == BaseWordCount.end()) {
			continue;
		}
		int WPeerCount = itWPeerCount->second;
		float OPC = CalcOPC(pElPeer->NumOccurs, WPeerCount);
		if (WPeerCount>0) {
			OPCLCount += OPC * pElPeer->NumOccurs;
			NumLRatios += pElPeer->NumOccurs;
		}
		if (WPeerCount > cMidCountThresh) {
			OPCMCount += OPC * pElPeer->NumOccurs;
			NumMRatios += pElPeer->NumOccurs;
		}
		if (WPeerCount > cHighCountThresh) {
			OPCHCount += OPC * pElPeer->NumOccurs;
			NumHRatios += pElPeer->NumOccurs;
		}

	}
	if (NumLRatios > 0 && OPCLCount != 0) {
		OPCLCount /= (float)NumLRatios;
	}
	else {
		return false;
	}
	if (NumMRatios > 0 && OPCMCount != 0) {
		OPCMCount /= (float)NumMRatios;
	}
	else {
		OPCMCount = -1.0f;
	}
	if (NumHRatios > 0 && OPCHCount != 0) {
		OPCHCount /= (float)NumHRatios;
	}
	else {
		OPCHCount = -1.0f;
	}
	return true;
}

void CGotitEnv::PrintGrpOPC(CPatGrpWrapper& PatGrp)
{
	int NumEls = PatGrp.getGrpElsMapSize();
	if (NumEls == 0) {
		return; // no good val created
	}
	cout << "OPC vals by count for " << PatGrp.getGrpName() << endl;

	for (int iElPeer = 0; iElPeer < NumEls; iElPeer++) {
		SGroupingLink * pElPeer = PatGrp.getGrpElByIndex(iElPeer);
		MapBaseWordToCount::iterator itWPeerCount
			= BaseWordCount.find(pElPeer->W);
		if (itWPeerCount == BaseWordCount.end()) {
			continue;
		}
		int WPeerCount = itWPeerCount->second;
		float OPC = CalcOPC(pElPeer->NumOccurs, WPeerCount);
		if (WPeerCount>0) {
			cout << OPC << "\t" << WPeerCount << endl;
		}

	}
	return;
}


void CGotitEnv::CreatePeerList()
{
	static const float cOPCMyGrpThresh = 0.001f;

	string W = ImplemParamTbl["Implem.Param.FnParam.CreatePeerList.SrcW"].Val;
	//vector<NameItem>& NameList = BasicTypeLists["word"];
	//for (uint iW = 5031; iW < 5200; iW++) {
	map<string, float> PeerScoreTbl; // map of names of peer to number of groups they share
	PeerVec.clear(); // Used as an output parameter dedicated to this function
	//string W = NameList[iW].Name;
	//int WCount = NameList[iW].Count;
	MapBaseWordToCount::iterator itWCount
		= BaseWordCount.find(W);
	if (itWCount == BaseWordCount.end()) {
		return;
	}
	int WCount = itWCount->second;
	map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(W);
	if (itWordGrpTbl != MapWordToData.end()) {
		map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
		map<int, int>::iterator itGrps = WordGrps.begin();
		for (; itGrps != WordGrps.end(); itGrps++) {
			CPatGrpHolder * phPatGrp 
				= PatGrpMgr.FindGrpByFileID(itGrps->first);
			if (phPatGrp != NULL) {
				CPatGrpWrapper PatGrp(phPatGrp);
				int NumSEls = PatGrp.getNumStrongEls();
				if (NumSEls == 0) {
					continue;
				}
				int NumRatios = 0;
				float GOPC = 0.0;
				//PrintGrpOPC(PatGrp);
				//GOPC = PatGrp.getGrpAvgOccursPerCount();
				for (int iElPeer = 0; iElPeer < NumSEls; iElPeer++) {
					SGroupingLink * pElPeer = PatGrp.getStrongElByIndex(iElPeer);
					MapBaseWordToCount::iterator itWPeerCount
						= BaseWordCount.find(pElPeer->W);
					if (itWPeerCount == BaseWordCount.end()) {
						continue;
					}
					int WPeerCount = itWPeerCount->second;
					if (WPeerCount>0) {
						GOPC += CalcOPC(pElPeer->NumOccurs, WPeerCount);
						NumRatios++;
					}

				}
				if (NumRatios > 0 && GOPC != 0) {
					GOPC /= (float)NumRatios;
				}
				else {
					continue;
				}
				//if (GOPC != GOPC) {
				//	cerr << "Bad GOPC float!!!\n";
				//}

				int NumEls = PatGrp.getGrpElsMapSize();
				if (NumEls == 0) {
					continue;
				}
				float GOPCCheck = PatGrp.getGrpAvgOccursPerCount();
				float GrpTotalOccurs = (float)PatGrp.getTotalNumOccurs();
				float WOPCR = CalcOPC(itGrps->second, WCount) / GOPC;
				if (WOPCR < cOPCMyGrpThresh) {
					continue;
				}
				for (int iElPeer = 0; iElPeer < NumEls; iElPeer++) {
					SGroupingLink * pElPeer = PatGrp.getGrpElByIndex(iElPeer);
					if (pElPeer == NULL || pElPeer->W == W) {
						continue;
					}
					float Score = 0.0f;
					int WPeerOccurs = pElPeer->NumOccurs;
					MapBaseWordToCount::iterator itWPeerCount
						= BaseWordCount.find(pElPeer->W);
					if (itWPeerCount == BaseWordCount.end() || itWPeerCount->second == 0) {
						continue;
					}
					int WPeerCount = itWPeerCount->second;
					float PeerOPCR = CalcOPC(WPeerOccurs, WPeerCount) / GOPC;
					//Score = WOPCR * PeerOPCR * (float)pElPeer->NumOccurs / log(GrpTotalOccurs);
					//Score = WOPCR * PeerOPCR * (float)pElPeer->NumOccurs / (float)log(NumEls);
					//Score = max(WOPCR, 1.0f) * max(1.0f, PeerOPCR);
					//Score = (float)WPeerOccurs;
					//Score = WOPCR * PeerOPCR * (float)pElPeer->NumOccurs;
					if (WOPCR != WOPCR) {
						cerr << "Bad WOPPCR float!!!\n";
					}
					if (PeerOPCR != PeerOPCR) {
						cerr << "Bad PeerOPCR float!!! " << WPeerOccurs << " " << WPeerCount << " " << GOPC << endl;
					}
					Score = WOPCR * (float)WPeerOccurs / (float)NumEls;
					//if ((pElPeer->W == "his") || (pElPeer->W == "my") || (pElPeer->W == "their") || (pElPeer->W == "our") || (pElPeer->W == "your")) {
					//	WOPCR = WOPCR;
					//}
					map<string, float>::iterator itPeer
						= PeerScoreTbl.find(pElPeer->W);
					if (itPeer != PeerScoreTbl.end()) {
						itPeer->second += Score;
					}
					else {
						PeerScoreTbl[pElPeer->W] = Score;
					}
				}

			}
		} // end loop over groups word appears in

		//vector<pair<float, string> > PeerVec;
		map<string, float>::iterator itGrp = PeerScoreTbl.begin();
		for (; itGrp != PeerScoreTbl.end(); itGrp++) {
			MapBaseWordToCount::iterator itWPeerCount
				= BaseWordCount.find(itGrp->first);
			if (itWPeerCount == BaseWordCount.end() || itWPeerCount->second == 0) {
				continue;
			}
			float WPeerCount = (float)itWPeerCount->second;
			float lf = log(WPeerCount);
			if (lf < 0.1f) {
				continue;
			}
			//PeerVec.push_back(make_pair(itGrp->second / WPeerCount, itGrp->first)); // reverse pair so that we can order on the number not name
			//PeerVec.push_back(make_pair(itGrp->second , itGrp->first)); // reverse pair so that we can order on the number not name
			PeerVec.push_back(make_pair(itGrp->second / lf, itGrp->first)); // reverse pair so that we can order on the number not name
		}
		sort(PeerVec.begin(), PeerVec.end(), greater<pair<float, string> >());
		cout << "\n" << W << " has count of " << WCount << ". List is " << WordGrps.size() << " long. Count start ----------------->\n";
		std::cout << "Printing peers for \"" << W << "\"\n";
		for (uint iVec = 0; iVec < PeerVec.size() && iVec < 40; iVec++) {
			string& WPeer = PeerVec[iVec].second;
			std::cout << "\t\"" << WPeer << "\" with score " << PeerVec[iVec].first << endl;
		}
#ifdef OLD_CODE
		int NumPeersCompared = 0;
		for (uint iVec = 0; iVec < PeerVec.size(); iVec++) {
			if (PeerVec[iVec].first < 5) {
				cout << NumPeersCompared << " comparisons made for " << W << ".\n";
				break;
			}
			string& WPeer = PeerVec[iVec].second;
			MapBaseWordToCount::iterator itWPeerCount = BaseWordCount.find(WPeer);
			if (itWPeerCount == BaseWordCount.end()) {
				continue;
			}
			NumPeersCompared++;
			int WPeerCount = itWPeerCount->second;
			cout << "Printing comparison between " << W << "(" << WCount << ") and " << WPeer << "(" << WPeerCount << ")\n";
			map<int, int>::iterator itGrps = WordGrps.begin();
			for (; itGrps != WordGrps.end(); itGrps++) {
				if (itGrps->second > 4) {
					CPatGrpHolder * phPatGrp
						= PatGrpMgr.FindGrpByFileID(itGrps->first);
					if (phPatGrp != NULL) {
						CPatGrpWrapper PatGrp(phPatGrp);
						SGroupingLink* pElWPeer = PatGrp.getGrpElByName(WPeer);
						int WPeerOccurs = 0;
						if (pElWPeer != NULL) {
							WPeerOccurs = pElWPeer->NumOccurs;
						}
						string GrpName = "Group name not found";
						PatGrp.getGrpName(GrpName);
						float GOPC = PatGrp.getGrpAvgOccursPerCount();
						cout	<< "For group " << GrpName
								<< " with average OPC " 
								<< GOPC << ", and "
								<< W << " has O " << itGrps->second
								<< " and OPCR " 
								<< (float)itGrps->second / (float)WCount / GOPC
								<< " while " << WPeer << " has O " 
								<< WPeerOccurs
								<< " and OPCR " 
								<< (float)WPeerOccurs / (float)WPeerCount / GOPC << endl;
					}
				}

			}
		}
#endif // OLD_CODE
	} // end if word appears in HitCount table
}

void CGotitEnv::FindRelatingGroups0(	string& Seed0, string& Seed1, bool& bFirstMorePopular,
									vector<string>& RelatedGrps, vector<string>& Seed0Only,
									vector<string>& Seed1Only, vector<string>& WeakGrps)
{
	static const int cGrpOccursThresh = 3;
	static const float cOPCThresh = 0.02f;
	static const float cOPCThreshMin = 0.004f;
	static const float cOPCRelThresh = 20.0; // If one OPC is 10 times the other, we don't consider them equal
	static const int cOccursOnlyThresh = 5; // The dominant side here can't be a weakling
	MapBaseWordToCount::iterator itCount
		= BaseWordCount.find(Seed0);
	if (itCount == BaseWordCount.end()) {
		return;
	}
	int Count0 = itCount->second;
	itCount	= BaseWordCount.find(Seed1);
	if (itCount == BaseWordCount.end()) {
		return;
	}
	int Count1 = itCount->second;
	bFirstMorePopular = false;
	if (Count0 > Count1) {
		bFirstMorePopular = true;
	}
	std::cout << "Finding relating groups for \"" << Seed0 << "\" (" << Count0 << ") and \"" << Seed1 << "\" (" << Count1 << ")\n";
	string W0, W1;
	int WCount, WPeerCount;
	bool b2ndWeaker;
	for (int iBoth = 0; iBoth < 2; iBoth++) {
		if (iBoth == 0) {
			W0 = Seed0;
			W1 = Seed1;
			WCount = Count0;
			WPeerCount = Count1;
			b2ndWeaker = bFirstMorePopular;
		}
		else {
			W0 = Seed1;
			W1 = Seed0;
			WCount = Count1;
			WPeerCount = Count0;
			b2ndWeaker = !bFirstMorePopular;
		}
		map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(W0);
		if (itWordGrpTbl == MapWordToData.end()) {
			return;
		}

		map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
		map<int, int>::iterator itGrps = WordGrps.begin();
		for (; itGrps != WordGrps.end(); itGrps++) {
			if (itGrps->second < cGrpOccursThresh) {
				continue;
			}
			CPatGrpHolder * phPatGrp
				= PatGrpMgr.FindGrpByFileID(itGrps->first);
			if (phPatGrp == NULL) {
				continue;
			}
			CPatGrpWrapper PatGrp(phPatGrp);
			int NumEls = PatGrp.getGrpElsMapSize();
			if (NumEls < 3) {
				continue;
			}
			string GrpName = PatGrp.getGrpName();
			float GOPC = PatGrp.getGrpAvgOccursPerCount();
			float GrpTotalOccurs = (float)PatGrp.getTotalNumOccurs();
			//float OPCR0 = CalcOPC(itGrps->second, WCount) / (GOPC * log(GrpTotalOccurs));
			float SizeFactor = (float)log(NumEls);
			float OPCR0 = CalcOPC(itGrps->second, WCount) / (GOPC * SizeFactor);
			if (OPCR0 < cOPCThresh) {
				continue;
			}
			SGroupingLink* pElWPeer = PatGrp.getGrpElByName(W1);
			int WPeerOccurs = 0;
			if (pElWPeer != NULL) {
				WPeerOccurs = pElWPeer->NumOccurs;
			}
			float OPCR1 = CalcOPC(WPeerOccurs, WPeerCount) / (GOPC * SizeFactor);
			std::cout << "Grp " << GrpName << ". Total Occurs: " << GrpTotalOccurs
					<< " NumEls " << NumEls << " For \"" << W0 
					<< "\" Occurs is " << itGrps->second << ", OPCR is " 
					<< OPCR0 << " and for \"" << W1 << "\" Occurs is " 
					<< WPeerOccurs << ", OPCR is " << OPCR1 << endl;
			if (OPCR1 > cOPCThreshMin) {
				if (OPCR1 > OPCR0 * cOPCRelThresh) {
					continue; // add this group when considering them the other way around
				}
				else if (OPCR0 > OPCR1 * cOPCRelThresh) {
					if (itGrps->second > cOccursOnlyThresh) {
						if (iBoth == 0) {
							Seed0Only.push_back(GrpName);
							std::cout << "\tadded to 0 only\n";
						}
						else {
							Seed1Only.push_back(GrpName);
							std::cout << "\tadded to 1 only\n";
						}
					}
				}
				else {
					if (OPCR1 > cOPCThresh) {
						if (WPeerOccurs < cGrpOccursThresh) {
							if (b2ndWeaker) {
								WeakGrps.push_back(GrpName);
								std::cout << "\tadded to weak\n";
							}
						}
						else {
							if (iBoth == 0) { // already done  first time
								RelatedGrps.push_back(GrpName);
								std::cout << "\tadded to related\n";
							}
						}
					}
				}
			}
			else {
				if (itGrps->second > cOccursOnlyThresh) {
					if (iBoth == 0) {
						Seed0Only.push_back(GrpName);
						std::cout << "\tadded to 0 only\n";
					}
					else {
						Seed1Only.push_back(GrpName);
						std::cout << "\tadded to 1 only\n";
					}
				}
			}
		}
	}
	std::cout << RelatedGrps.size() << " related, " << Seed0Only.size() << " only first and "
			<< Seed1Only.size() << " for second.  " << WeakGrps.size() << " Weak.\n";

}


void CGotitEnv::FindRelatingGroups(	string& Seed0, string& Seed1, bool& bFirstMorePopular,
									vector<string>& RelatedGrps, vector<string>& Seed0Only,
									vector<string>& Seed1Only, vector<string>& WeakGrps,
									float& Score)
{
	static const int cGrpOccursThresh = 3;
	static const float cOPCThresh = 0.02f;
	static const float cOPCThreshMin = 0.004f;
	static const float cOPCRelThresh = 20.0; // If one OPC is 10 times the other, we don't consider them equal
	static const int cOccursOnlyThresh = 5; // The dominant side here can't be a weakling
	MapBaseWordToCount::iterator itCount
		= BaseWordCount.find(Seed0);
	if (itCount == BaseWordCount.end()) {
		return;
	}
	int Count0 = itCount->second;
	itCount	= BaseWordCount.find(Seed1);
	if (itCount == BaseWordCount.end()) {
		return;
	}
	int Count1 = itCount->second;
	bFirstMorePopular = false;
	if (Count0 > Count1) {
		bFirstMorePopular = true;
	}
	std::cout << "Finding relating groups for \"" << Seed0 << "\" (" << Count0 << ") and \"" << Seed1 << "\" (" << Count1 << ")\n";
	string W0, W1;
	int WCount, WPeerCount;
	bool b2ndWeaker;
	for (int iBoth = 0; iBoth < 2; iBoth++) {
		if (iBoth == 0) {
			W0 = Seed0;
			W1 = Seed1;
			WCount = Count0;
			WPeerCount = Count1;
			b2ndWeaker = bFirstMorePopular;
		}
		else {
			W0 = Seed1;
			W1 = Seed0;
			WCount = Count1;
			WPeerCount = Count0;
			b2ndWeaker = !bFirstMorePopular;
		}
		map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(W0);
		if (itWordGrpTbl == MapWordToData.end()) {
			return;
		}

		map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
		map<int, int>::iterator itGrps = WordGrps.begin();
		for (; itGrps != WordGrps.end(); itGrps++) {
			if (itGrps->second < cGrpOccursThresh) {
				continue;
			}
			CPatGrpHolder * phPatGrp
				= PatGrpMgr.FindGrpByFileID(itGrps->first);
			if (phPatGrp == NULL) {
				continue;
			}
			CPatGrpWrapper PatGrp(phPatGrp);
			int NumEls = PatGrp.getGrpElsMapSize();
			if (NumEls < 3) {
				continue;
			}
			string GrpName = PatGrp.getGrpName();
			float GOPCH = 0.0;
			float GOPCM = 0.0;
			float GOPCL = 0.0;
			float GOPC = PatGrp.getGrpAvgOccursPerCount();
			if (!CalcGrpOPC(PatGrp, GOPCH, GOPCM, GOPCL)) {
				continue;
			}
			cout << "Avg OPCs for grp " << GrpName << ", GOPC " << GOPC << " L " << GOPCL << " M " << GOPCM << " H " << GOPCH << endl;
			float GrpTotalOccurs = (float)PatGrp.getTotalNumOccurs();
			//float OPCR0 = CalcOPC(itGrps->second, WCount) / (GOPC * log(GrpTotalOccurs));
			float SizeFactor = 1.0f ; // (float)log(NumEls);
			float OPCR0 = CalcOPC(itGrps->second, WCount) / (GOPC * SizeFactor);
			if (OPCR0 < cOPCThresh) {
				continue;
			}
			SGroupingLink* pElWPeer = PatGrp.getGrpElByName(W1);
			int WPeerOccurs = 0;
			if (pElWPeer != NULL) {
				WPeerOccurs = pElWPeer->NumOccurs;
			}
			float OPCR1 = CalcOPC(WPeerOccurs, WPeerCount) / (GOPC * SizeFactor);
			std::cout << "Grp " << GrpName << ". Total Occurs: " << GrpTotalOccurs
					<< " NumEls " << NumEls << " For \"" << W0 
					<< "\" Occurs is " << itGrps->second << ", OPCR is " 
					<< OPCR0 << " and for \"" << W1 << "\" Occurs is " 
					<< WPeerOccurs << ", OPCR is " << OPCR1 << endl;
			if (OPCR1 > cOPCThreshMin) {
				if (OPCR1 > OPCR0 * cOPCRelThresh) {
					continue; // add this group when considering them the other way around
				}
				else if (OPCR0 > OPCR1 * cOPCRelThresh) {
					if (itGrps->second > cOccursOnlyThresh) {
						if (iBoth == 0) {
							Seed0Only.push_back(GrpName);
							std::cout << "\tadded to 0 only\n";
						}
						else {
							Seed1Only.push_back(GrpName);
							std::cout << "\tadded to 1 only\n";
						}
					}
				}
				else {
					if (OPCR1 > cOPCThresh) {
						if (WPeerOccurs < cGrpOccursThresh) {
							if (b2ndWeaker) {
								WeakGrps.push_back(GrpName);
								std::cout << "\tadded to weak\n";
							}
						}
						else {
							if (iBoth == 0) { // already done  first time
								RelatedGrps.push_back(GrpName);
								std::cout << "\tadded to related\n";
							}
						}
					}
				}
			}
			else {
				if (itGrps->second > cOccursOnlyThresh) {
					if (iBoth == 0) {
						Seed0Only.push_back(GrpName);
						std::cout << "\tadded to 0 only\n";
					}
					else {
						Seed1Only.push_back(GrpName);
						std::cout << "\tadded to 1 only\n";
					}
				}
			}
		}
	}
	std::cout << RelatedGrps.size() << " related, " << Seed0Only.size() << " only first and "
			<< Seed1Only.size() << " for second.  " << WeakGrps.size() << " Weak.\n";

}


void CGotitEnv::ShowGoodPeers()
{
	string W = "they";
	ImplemParamTbl["Implem.Param.FnParam.CreatePeerList.SrcW"].Val = W;
	CreatePeerList();
	MapBaseWordToCount::iterator itWCount
		= BaseWordCount.find(W);
	if (itWCount == BaseWordCount.end()) {
		return;
	}
	int WCount = itWCount->second;
	map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(W);
	if (itWordGrpTbl == MapWordToData.end()) {
		std::cout << "Seed word not found in \n";
		return;

	}
	map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
	for (uint iVec = 0; iVec < PeerVec.size() && iVec < 7; iVec++) {
		string& WPeer = PeerVec[iVec].second;
		MapBaseWordToCount::iterator itWPeerCount = BaseWordCount.find(WPeer);
		if (itWPeerCount == BaseWordCount.end()) {
			continue;
		}
		int WPeerCount = itWPeerCount->second;
		std::cout << "Printing comparison between " << W << "(" << WCount << ") and " << WPeer << "(" << WPeerCount << ")\n";
		map<int, int>::iterator itGrps = WordGrps.begin();
		for (; itGrps != WordGrps.end(); itGrps++) {
			if (itGrps->second < 3) {
				continue;
			}
			CPatGrpHolder * phPatGrp
				= PatGrpMgr.FindGrpByFileID(itGrps->first);
			if (phPatGrp == NULL) {
				continue;
			}
			CPatGrpWrapper PatGrp(phPatGrp);
			string GrpName = "Group name not found";
			PatGrp.getGrpName(GrpName);
			float GOPC = PatGrp.getGrpAvgOccursPerCount();
			float WOPCR = (float)itGrps->second / (float)WCount / GOPC;

			SGroupingLink* pElWPeer = PatGrp.getGrpElByName(WPeer);
			int WPeerOccurs = 0;
			if (pElWPeer != NULL) {
				WPeerOccurs = pElWPeer->NumOccurs;
			}
			float WPeerOPCR = (float)WPeerOccurs / (float)WPeerCount / GOPC;
			static const float cOPCThresh = 0.1f;
			if (WPeerOccurs >= 3 && WOPCR > cOPCThresh && WPeerOPCR > cOPCThresh) {
				string GrpName = "Group name not found";
				PatGrp.getGrpName(GrpName);
				std::cout << "For group " << GrpName
					<< " with average OPC "
					<< GOPC << ", and \""
					<< W << "\" has O " << itGrps->second
					<< " and OPCR "
					<< WOPCR
					<< " while \"" << WPeer << "\" has O "
					<< WPeerOccurs
					<< " and OPCR "
					<< WPeerOPCR << endl;

			}
		}
	}
}

void CGotitEnv::FindGrpConnections(vector<string>& GrpList0, vector<string>& GrpList1,
									int ConnThresh, vector<pair<int, int> >& GrpConnList)
{
	static const float cOPCThresh = 0.1f;
	for (uint is0 = 0; is0 < GrpList0.size(); is0++) {
		CPatGrpHolder *  phGrp0 = PatGrpMgr.FindGrp(GrpList0[is0]);
		CPatGrpWrapper Grp0(phGrp0);
		std::cout << "\tFinding connections for group " << Grp0.getGrpName() << endl;
		for (uint is1 = 0; is1 < GrpList1.size(); is1++) {
			CPatGrpHolder *  phGrp1 = PatGrpMgr.FindGrp(GrpList1[is1]);
			if (phGrp0 == phGrp1) {
				// for now we don't consider identitiy a relation. Reconsider!
				continue;
			}
			CPatGrpWrapper Grp1(phGrp1);
			if (Grp0.getNumGrpParams() != Grp1.getNumGrpParams()) {
				continue;
			}
			int iParamNotMatching = -1;
			bool bTooManyMisMatches = false;
			for (int iParam = 0; iParam < Grp0.getNumGrpParams(); iParam++) {
				if (Grp0.getGrpParam(iParam) != Grp1.getGrpParam(iParam)) {
					if (iParamNotMatching != -1) {
						bTooManyMisMatches = true;
						break;
					}
					iParamNotMatching = iParam;
				}
			}
			if (bTooManyMisMatches || iParamNotMatching == -1) {
				continue;
			}
			std::cout << "\t\tconnections for group " << Grp1.getGrpName() << endl;
			string Seed0 = Grp0.getGrpParam(iParamNotMatching);
			string Seed1 = Grp1.getGrpParam(iParamNotMatching);
			map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(Seed0);
			if (itWordGrpTbl == MapWordToData.end()) {
				continue;
			}
			int NumPeerOccurs = 0;
			int NumOccurs = 0;
			map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
			map<int, int>::iterator itGrps = WordGrps.begin();
			for (; itGrps != WordGrps.end(); itGrps++) {
				NumOccurs += itGrps->second;
				CPatGrpHolder * phPatGrp
					= PatGrpMgr.FindGrpByFileID(itGrps->first);
				if (phPatGrp == NULL) {
					continue;
				}
				CPatGrpWrapper PatGrp(phPatGrp);
				int NumEls = PatGrp.getGrpElsMapSize();
				if (NumEls == 0) {
					continue;
				}
				SGroupingLink * pElPeer = PatGrp.getGrpElByName(Seed1);
				if (pElPeer == NULL) {
					continue;
				}
				NumPeerOccurs += pElPeer->NumOccurs;
			}
			MapBaseWordToCount::iterator itCount
				= BaseWordCount.find(Seed0);
			if (itCount == BaseWordCount.end()) {
				continue;
			}
			float Count0 = (float)itCount->second;
			float GrpsOPC0 = (float)NumOccurs / Count0;
			if (GrpsOPC0 < cOPCThresh) {
				continue;
			}
			itCount = BaseWordCount.find(Seed1);
			if (itCount == BaseWordCount.end()) {
				continue;
			}
			int Count1 = itCount->second;
			float GrpsOPC1 = (float)NumOccurs / Count0;
			if (GrpsOPC0 < cOPCThresh) {
				continue;
			}

			GrpConnList.push_back(make_pair(is0, is1));

#ifdef OLD_CODE
			bool bFirstMorePopular;
			vector<string> RelatedGrps;
			vector<string> Seed0Only;
			vector<string> Seed1Only;
			vector<string> WeakGrps;
			FindRelatingGroups(Seed0, Seed1, bFirstMorePopular,
								RelatedGrps, Seed0Only,
								Seed1Only, WeakGrps);
			std::cout << "\t\t\t" << RelatedGrps.size() << " grps found\n";
			if (RelatedGrps.size() > (uint)ConnThresh) {
				GrpConnList.push_back(make_pair(is0, is1));
			}
#endif  // OLD_CODE
		}
	}
}

void CGotitEnv::EvaluateDep(DepRec& Dep, vector<string>& DepNames,
							vector<WordRec>& WordRec, float& Score)
{
	Score = 0.0f;
	string& DepName = DepNames[Dep.iDep];
	string& GovWord = WordRec[Dep.Gov].Word;
	string& DepWord = WordRec[Dep.Dep].Word;
	for (int iBoth = 0; iBoth < 2; iBoth++) {
		string GrpName;
		string GrpWord;
		string ElWord;
		string GrpNamePrefix;
		if (iBoth == 0) {
			GrpWord = GovWord;
			ElWord = DepWord;
			GrpNamePrefix = "DepNears";
		}
		else {
			GrpWord = DepWord;
			ElWord = GovWord;
			GrpNamePrefix = "DepNearsOnGov";
		}
		GrpName = GrpNamePrefix + "[%0%" + DepName + "][%1%" + GrpWord + "]";
		CPatGrpHolder * phPatGrp = PatGrpMgr.FindGrp(GrpName);
		if (phPatGrp != NULL) {
			CPatGrpWrapper PatGrp(phPatGrp);
			float GOPC = PatGrp.getGrpAvgOccursPerCount();
			if (GOPC < 0) {
				continue; // not enough to generate a valid value
			}
			if (bDebugWordAlign) {
				cout << "Grp found " << PatGrp.getGrpName() << " with "
					<< PatGrp.getGrpElsMapSize() << " Els\n";
			}
			// else match not found:
			MapBaseWordToCount::iterator itCount
				= BaseWordCount.find(GrpWord);
			if (itCount == BaseWordCount.end()) {
				continue;
			}
			int CountGW = itCount->second;
			itCount = BaseWordCount.find(ElWord);
			if (itCount == BaseWordCount.end()) {
				continue;
			}
			int CountEW = itCount->second;
			SGroupingLink * pEl = PatGrp.getGrpElByName(ElWord);
			if (pEl != NULL) {
				if (bDebugWordAlign) {
					cout << "Match found on " << ElWord << "!\n";
				}
				Score += AlignConfig[AlignParamMatchFoundConst];
				Score += AlignConfig[AlignParamMatchFoundOPCR] * min(1.0f, ((float)pEl->NumOccurs / (float)CountEW) / GOPC);
				continue;
			}
			if ((float)CountEW < (1.0f / GOPC)) {
				if (bDebugWordAlign) {
					cout << "Not matching not surprising, " << ElWord << " has hit count " << CountEW << " and Grp has thresh count " << (1.0f / GOPC) << endl;
				}
			}
			else {
				Score += AlignConfig[AlignParamNoMatchSurprising] * min(1.0f, CountEW * GOPC);
				if (bDebugWordAlign) {
					cout << "Not matching Surprising! " << ElWord << " has hit count " << CountEW << " and Grp has thresh count " << (1.0f / GOPC) << endl;
					cout << "EvaluateDep score now " << Score << endl;
				}
			}
			int NumEls = PatGrp.getGrpElsMapSize();
			vector<string> CompNames;
			vector<int> CummOccursList;
			int CummOccurs = 0;
			for (int iEl = 0; iEl < NumEls; iEl++) {
				SGroupingLink * pEl = PatGrp.getGrpElByIndex(iEl);
				CummOccurs += pEl->NumOccurs;
				CompNames.push_back(pEl->W);
				CummOccursList.push_back(CummOccurs);
			}
			static const int cMaxPeersConsidered = 50;
			if (NumEls > cMaxPeersConsidered) {
				// What I am trying to do here is to create a small subset 
				// of the original list while biasing the search to the earlier
				// originals that should have higher NumOccurs generally.
				//NumEls = cMaxPeersConsidered;
				vector<string> CandNames;
				int iCurr = 0;
				int CurrRand = 0;
				for (int iEl = 0; iEl < NumEls; iEl++) {
					if (CurrRand > CummOccursList[iEl]) {
						continue;
					}
					CurrRand += rand() % (CummOccurs / cMaxPeersConsidered);
					CandNames.push_back(CompNames[iEl]);
					//cout << iEl << " added to cands\n";
				}
				CompNames.clear();
				CompNames = CandNames; // list copy
			}
			if (bDebugWordAlign) {
				cout << "Comps list size is " << CompNames.size() << endl;
			}
			for (uint iEl = 0; iEl < CompNames.size(); iEl++) {
				float LocalScore = 0;
				EvalMembershipChances(	DepName, GrpWord, GrpNamePrefix, ElWord, 
										CompNames, iEl, LocalScore);
				if (bDebugWordAlign) {
					cout << "EvalMembershipChances returned score " << LocalScore << endl;
				}
				Score +=	(LocalScore * AlignConfig[AlignParamNumPeersFactor]) 
						+	(	(LocalScore / (float)CompNames.size()) 
							*	AlignConfig[AlignParamNumPeersFactorPerNumPeers]);
				LocalScore = max(-1.0f, min(1.0f, LocalScore));
				Score +=	(LocalScore * AlignConfig[AlignParamNumPeersFactorLimit]) 
						+	(	(LocalScore / (float)CompNames.size()) 
							*	AlignConfig[AlignParamNumPeersFactorPerNumPeersLimit]);
				if (bDebugWordAlign) {
					cout << "EvaluateDep score now " << Score << endl;
				}
			}

		}

	}
}

void CGotitEnv::EvalMembershipChances(	string& DepName, string& GrpWord, 
										string& GrpNamePrefix,
										string& ElWordSrc, vector<string>& DepWordComps, 
										int iCurr, float& Score)
{
	//string GrpName = GrpNamePrefix + "[%0%" + DepName + "][%1%" + GrpWord + "]";
	static int cNumGrpParamsExpected = 2;
	Score = 0.0f;
	string& CompW = DepWordComps[iCurr];
	map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(CompW);
	if (itWordGrpTbl == MapWordToData.end()) {
		return;
	}
	MapBaseWordToCount::iterator itCount
		= BaseWordCount.find(CompW);
	if (itCount == BaseWordCount.end()) {
		return;
	}
	int CompWCount = itCount->second;
	if (bDebugWordAlign) {
		cout << "Considering comparison with \"" << CompW << "\" which has a count of "
			<< CompWCount << endl;
	}
	itCount = BaseWordCount.find(ElWordSrc);
	if (itCount == BaseWordCount.end()) {
		return;
	}
	int SrcWCount = itCount->second;
	map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
	map<int, int>::iterator itGrps = WordGrps.begin();
	int NumWordGrps = WordGrps.size();
	for (; itGrps != WordGrps.end(); itGrps++) {
		CPatGrpHolder * phPatGrp
			= PatGrpMgr.FindGrpByFileID(itGrps->first);
		if (phPatGrp == NULL) {
			continue;
		}
		CPatGrpWrapper PatGrp(phPatGrp);
		SPatternFinder* pFinder;
		if (!PatGrp.getGrpFinder(pFinder)) {
			continue;
		}
		if (pFinder->group_name_template != GrpNamePrefix) {
			continue;
		}
		if (	(PatGrp.getNumGrpParams() < cNumGrpParamsExpected )
			||	(PatGrp.getGrpParam(0) != DepName)) {
			continue;
		}
		float GOPC = PatGrp.getGrpAvgOccursPerCount();
		if (GOPC < 0.0f) {
			continue;
		}
		float CompOPCR = (float)itGrps->second / (float)CompWCount / GOPC;
		SGroupingLink* pEl = PatGrp.getGrpElByName(ElWordSrc);
		if (pEl == NULL) {
			Score += AlignConfig[AlignParamNotFoundInPeerGrp] * SrcWCount * GOPC * min(CompOPCR, 1.0f);
			Score +=	AlignConfig[AlignParamNotFoundInPeerGrpPerNumGrps] * SrcWCount * GOPC  
					*	min(CompOPCR, 1.0f) / NumWordGrps;
			string DespiteOrBecause = "despite";
			if ((1.0f / GOPC) > SrcWCount) {
				DespiteOrBecause = "because";
			}
			if (bDebugWordAlign) {
				cout << "\"" << ElWordSrc << "\" not found in grp " << PatGrp.getGrpName()
					<< " " << DespiteOrBecause << " GCPO " << (1.0f / GOPC)
					<< ". " << CompW << " has "
					<< itGrps->second << " occurences and OPCR " << CompOPCR << endl;
			}
		}
		else {
			float SrcOPCR = (float)(pEl->NumOccurs) / (float)SrcWCount / GOPC;
			Score += AlignConfig[AlignParamFoundInPeerGrp] * min(SrcOPCR , 1.0f) * min(CompOPCR, 1.0f);
			Score +=	AlignConfig[AlignParamFoundInPeerGrpPerNumGrps] * min(SrcOPCR , 1.0f)
					*	min(CompOPCR, 1.0f) / NumWordGrps;
			if (bDebugWordAlign) {
				cout << "\"" << ElWordSrc << "\" in grp " << PatGrp.getGrpName()
					<< " with " << pEl->NumOccurs << " occurrences and OPCR " << SrcOPCR
					<< " consider GCPO " << (1.0f / GOPC)
					<< ". " << CompW << " has "
					<< itGrps->second << " occurences and OPCR " << CompOPCR << endl;
			}
		}
		if (bDebugWordAlign) {
			cout << "EvalMembershipChances score now " << Score << endl;
		}

	}
}

void CGotitEnv::FindAlignedWord()
{
	static const int cNumRelatedThresh = 3;
#ifdef RECALC_GRP
	CPatGrpIter itpgt = PatGrpMgr.getStartIterator();
	for (; PatGrpMgr.IsIteratorValid(itpgt); PatGrpMgr.getNextIterator(itpgt)) {
		CPatGrpWrapper PatGrp(itpgt);
		float GOPCH = 0.0;
		float GOPCM = 0.0;
		float GOPCL = 0.0;
		float GOPC = PatGrp.getGrpAvgOccursPerCount();
		if (!CalcGrpOPC(PatGrp, GOPCH, GOPCM, GOPCL)) {
			continue;
		}
		cout	<< "Avg OPCs for grp " << PatGrp.getGrpName() << " NumOccurs " 
				<< PatGrp.getTotalNumOccurs() << " NumEls " 
				<< PatGrp.getGrpElsMapSize() << ", GOPC " << GOPC 
				<< " L " << GOPCL << " M " << GOPCM << " H " << GOPCH << endl;
	}
	return;
#endif // RECALC_GRP

	vector<NameItem>& NameList = BasicTypeLists["word"];
	for (uint iW = 31; iW < 200; iW+=40) {
		string W = NameList[iW].Name;
		///W = "her";
		//int WCount = NameList[iW].Count;
		ImplemParamTbl["Implem.Param.FnParam.CreatePeerList.SrcW"].Val = W;
		CreatePeerList();

		MapBaseWordToCount::iterator itWCount
			= BaseWordCount.find(W);
		if (itWCount == BaseWordCount.end()) {
			return;
		}
		int WCount = itWCount->second;
		map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(W);
		if (itWordGrpTbl == MapWordToData.end()) {
			std::cout << "Seed word not found in \n";
			return;

		}
		map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
		for (uint iVec = 0; iVec < PeerVec.size() && iVec < 30; iVec++) {
			string& WPeer = PeerVec[iVec].second;
			bool bFirstMorePopular;
			vector<string> RelatedGrps;
			vector<string> Seed0Only;
			vector<string> Seed1Only;
			vector<string> WeakGrps;
			float Score = 0.0f;
			FindRelatingGroups(W, WPeer, bFirstMorePopular,
								RelatedGrps, Seed0Only,
								Seed1Only, WeakGrps, Score);
			if (RelatedGrps.size() < cNumRelatedThresh) {
				continue;
			}

			if (Seed0Only.size() == 0 || Seed1Only.size() == 0) {
				continue;
			}

			//if ((Seed0Only.size() + Seed1Only.size()) > (RelatedGrps.size() * 3)) {
			//	std::cout << "Despite seeming related, the only outnumber the related\n";
			//	continue;
			//}

			int ConnThresh = 3;
			vector<pair<int, int> > GrpConnList;

			continue; // ------------------------------------------------------------------------------

			std::cout << "Starting search for connections between *only* groups of \"" << W << "\" and \"" << WPeer << "\" ---------------------------------->\n";
			FindGrpConnections(Seed0Only, Seed1Only,
								ConnThresh, GrpConnList);

			for (uint iConn = 0; iConn < GrpConnList.size(); iConn++) {
				string& GrpName0 = Seed0Only[GrpConnList[iConn].first];
				string& GrpName1 = Seed1Only[GrpConnList[iConn].second];

				CPatGrpHolder * phGrp0 = PatGrpMgr.FindGrp(GrpName0);
				CPatGrpHolder * phGrp1 = PatGrpMgr.FindGrp(GrpName1);
				if (phGrp0 == NULL || phGrp1 == NULL) {
					continue;
				}

				CPatGrpWrapper PatGrp0(phGrp0);
				CPatGrpWrapper PatGrp1(phGrp1);

				std::cout << "\"" << W << "\" is to " << PatGrp0.getGrpName()
						<< " as \"" << WPeer << "\" is to "
						<< PatGrp1.getGrpName() << endl;

			}

		}
	}
}
