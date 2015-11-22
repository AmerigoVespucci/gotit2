// WordAligned.cpp :Find a group to expand a specific word.
//

#include "stdafx.h"

#include "MascReader.h"

void CGotitEnv::FindAlignedWord()
{
	/*
	Find a popular word, not too popular
	Find another word that 
	*/
	vector<NameItem>& NameList = BasicTypeLists["word"];
	//vector<pair<int, int>> ListForSorting;
	//for (uint inl = 0; inl < NameList.size(); inl++) {
	//	ListForSorting.push_back(make_pair(NameList[inl].Count, inl));
	//}
	//// pair greater<> uses pair < which checks the fisrt of pair and if equal hecks the second
	//sort(ListForSorting.begin(), ListForSorting.end(), greater<pair<int, int> >());
	for (uint iW = 1031; iW < 1200; iW++) {
		map<string, int> PeerGrpCount; // map of names of peer to number of groups they share
		string W = NameList[iW].Name;
		int WCount = NameList[iW].Count;
		map<string, SWordData>::iterator itWordGrpTbl = MapWordToData.find(W);
		if (itWordGrpTbl != MapWordToData.end()) {
			map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
			map<int, int>::iterator itGrps = WordGrps.begin();
			for (; itGrps != WordGrps.end(); itGrps++) {
				if (itGrps->second > 4) {
					CPatGrpHolder * phPatGrp 
						= PatGrpMgr.FindGrpByFileID(itGrps->first);
					if (phPatGrp != NULL) {
						CPatGrpWrapper PatGrp(phPatGrp);
						int NumSEls = PatGrp.getNumStrongEls();
						for (int iEl = 0; iEl < NumSEls; iEl++) {
							SGroupingLink * pEl = PatGrp.getStrongElByIndex(iEl);
							if (pEl->W == W) {
								continue;
							}
							map<string, int>::iterator itPeer 
								= PeerGrpCount.find(pEl->W);
							if (itPeer != PeerGrpCount.end()) {
								itPeer->second++;
							}
							else {
								PeerGrpCount[pEl->W] = 1;
							}
						}

					}
				}
			} // end loop over groups word appears in
			vector<pair<int, string> > PeerVec;
			map<string, int>::iterator itGrp = PeerGrpCount.begin();
			for (; itGrp != PeerGrpCount.end(); itGrp++) {
				PeerVec.push_back(make_pair(itGrp->second, itGrp->first)); // reverse pair so that we can order on the number not name
			}
			sort(PeerVec.begin(), PeerVec.end(), greater<pair<int, string> >());
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
		} // end if word appears in HitCount table
	} // end loop over popular words
}
