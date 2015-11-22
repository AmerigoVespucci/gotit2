// WordGroup.cpp :Find a group to expand a specific word.
//

#include "stdafx.h"

#include "MascReader.h"
#include "RF/librf/instance_set.h"
#include "RF/librf/random_forest.h"

using namespace rapidxml;
using namespace std;
using namespace librf;

bool pgt(pair<float, int> a, pair<float, int> b) {
	return (a.first > b.first);
}


void CGotitEnv::CreateWordGroup(SGrpSeed* pSeedTblEl, vector<CPatGrpIter*>& SeedGrps)
{
	string Seed = pSeedTblEl->SeedName;
	stringstream& ssRFData = *(pSeedTblEl->pRFData);
	stringstream& ssRFDataOOB = *(pSeedTblEl->pRFDataOOB);
	vector<bool>& bOOBTbl = pSeedTblEl->bOOB; 
	vector<string>& PeersTested = pSeedTblEl->SortedPeers;
	map<string, CPatGrpIter*> MapNamesToSeedGrps;
	for (uint isg = 0; isg < SeedGrps.size(); isg++) {
		CPatGrpWrapper PatGrp(*SeedGrps[isg]);
		string GrpName;
		PatGrp.getGrpName(GrpName);
		MapNamesToSeedGrps[GrpName] = SeedGrps[isg];
	}
	fstream fout("WordNumberMatches.csv", fstream::out);
	fout << ",";
	ssRFData << ",";
	ssRFDataOOB << ",";

	CPatGrpHolder * phPatGrpSeed = NULL; // the AllPOSWord group of the seed. 
	float MembershipStrength = 0.0f;
	// our first concern is to determine which POS group the seed most belongs to
	vector<string> PosNames;
	for (uint ip = 0; ip < PatGroups.size(); ip++) {
		CPatGrpWrapper PatGrp(PatGroups[ip]);
		SPatternFinder* pFinder;
		if (PatGrp.getGrpFinder(pFinder)) {
			// place this before search in case we access the members array which may
			// be 0 or too short
			if (pFinder->group_name_template != "AllPOSWords") {
				continue;
			}
		}
		string OnePosName = PatGrp.getGrpParam(0);
		PosNames.push_back(OnePosName);
		// modify pos name for excel csv
		size_t CommaPos = OnePosName.find(",");
		if (CommaPos != string::npos) {
			OnePosName.replace(CommaPos, 1, "comma");
		}
		fout << "prec-" << OnePosName << "," << "following-" << OnePosName << ",";
		ssRFData << "prec-" << OnePosName << "," << "following-" << OnePosName << ",";
		ssRFDataOOB << "prec-" << OnePosName << "," << "following-" << OnePosName << ",";
		SGroupingLink* pEl = PatGrp.getGrpElByName(Seed);
		//MapMemberToGroup::iterator itm = PatGroups[ip]->MemberLinks.find(Seed);
		if (pEl != NULL)  {
			// one way to select only AllPosWords groups
			//size_t found = PatGroups[ip]->Name.find("AllPOSWords");
			//if (found != 0) {
			//	continue;
			//}
			// another way
			float tempMembershipStrength = (float)pEl->NumOccurs / (float)pEl->NumExist; // NumExist not really in use any more
			if (tempMembershipStrength < MembershipStrength) {
				continue;
			}
			MembershipStrength = tempMembershipStrength;
			phPatGrpSeed = PatGrp.PresentHolder();
		}
	}
	if (phPatGrpSeed == NULL) {
		return;
	}

	fout << endl; // end of title line
	ssRFData << endl;
	ssRFDataOOB << endl;

	static int cStrongPeerThresh = 100;
	// iterate over peer members
	CPatGrpWrapper PatGrpSeed(phPatGrpSeed);
	//MapMemberToGroup::iterator itm = patgrpSeed->MemberLinks.begin();
	vector<vector<int> > MatchesTbl; // blongs to old code. Should be removed
	uint NumIn = 0;
	uint NumOOB = 0;
	int NumEls = PatGrpSeed.getGrpElsMapSize();
	for (int iEl = 0; iEl < NumEls; iEl++) {
		if (PatGrpSeed.getGrpElByIndex(iEl)->NumOccurs < cStrongPeerThresh) {
			continue;
		}
		string PeerName = PatGrpSeed.getGrpElByIndex(iEl)->W;
		if (NotPureAlpha(PeerName)) {
			continue;
		}
		bool bOOB = ((rand() % 3) == 0);
		stringstream& ThisRFOut = (!bOOB ? ssRFData : ssRFDataOOB);

		vector<int> MatchesLine;
		bool bFirstInLine = true;
		bool bPeerPresent = true;
		bool bLineEmpty = true;
		for (uint ipg = 0; ipg < PosNames.size() && bPeerPresent; ipg++) {
			for (int bPrec = 0; bPrec < 2 && bPeerPresent; bPrec++) {
				string PeerGrpName;
				string SeedGrpName;
				if (bPrec == 0) {
					PeerGrpName = "SeedGrp[%" + PosNames[ipg] + "%]Prec[%" + PeerName + "%]";
					SeedGrpName = "SeedGrp[%" + PosNames[ipg] + "%]Prec[%" + Seed + "%]";
				}
				else {
					//string SeedGrpName = "SeedGrp[%" + Seed + "%]FollowedBy[%" + PosNames[ipg] + "%]";
					PeerGrpName = "SeedGrp[%" + PeerName + "%]FollowedBy[%" + PosNames[ipg] + "%]";
					SeedGrpName = "SeedGrp[%" + Seed + "%]FollowedBy[%" + PosNames[ipg] + "%]";
				}
				//CGroupingRec * patgrpPeerPosGrp = MapNamesToSeedGrps[PeerGrpName];
				map<string, CPatGrpIter*>::iterator itmnsg = MapNamesToSeedGrps.find(PeerGrpName);
				if (itmnsg == MapNamesToSeedGrps.end()) {
					bPeerPresent = false;
					continue;
				}
				CPatGrpWrapper PatGrpPeer(*(itmnsg->second));
				if (bFirstInLine) {
					fout << PeerName << ",";
					ThisRFOut << PeerName << ",";
					bLineEmpty = false;
					bFirstInLine = false;
					PeersTested.push_back(PeerName);
					MatchesTbl.push_back(MatchesLine);
					if (bOOB) {
						NumOOB++;
						bOOBTbl.push_back(true);
					}
					else {
						NumIn++;
						bOOBTbl.push_back(false);
					}
				}
				itmnsg = MapNamesToSeedGrps.find(SeedGrpName);
				if (itmnsg == MapNamesToSeedGrps.end()) {
					bPeerPresent = false;
					continue;
				}
				CPatGrpWrapper PatGrpSeed(*(itmnsg->second));
				//CGroupingRec * patgrpSeedPosGrp = MapNamesToSeedGrps[SeedGrpName];
				//MapMemberToGroup::iterator itms = patgrpSeedPosGrp->MemberLinks.begin(); // the pos group for the pos of that seed not the seed
				int Matches = 0;
				int NumElsInSeedGrp = PatGrpSeed.getGrpElsMapSize();
				//for (; itms != patgrpSeedPosGrp->MemberLinks.end(); itms++) {
				for (int iElInSeedGrp = 0; iElInSeedGrp < NumElsInSeedGrp; iElInSeedGrp++) {
					string ToFind = PatGrpSeed.getGrpElByIndex(iElInSeedGrp)->W;
					SGroupingLink * pPeerEl = PatGrpPeer.getGrpElByName(ToFind);
					if (pPeerEl != NULL) {
						//add the count in 2d array
						Matches++;
					}
				}
				MatchesTbl.back().push_back(Matches);
				fout << Matches << ",";
				ThisRFOut << (float)Matches << ",";
				bLineEmpty = false;
			} // end of bPrec or not
		} // end of loop over POS names
		fout << endl;
		if (!bLineEmpty) {
			ThisRFOut << endl;
			bLineEmpty = true;
		}
	}

	cout << "For seed " << Seed << " Num test records " << NumIn << " and OOB " << NumOOB << endl;

#ifdef OLD_CODE
	for (uint ipt = 0; ipt < PeersTested.size(); ipt++)  {
		OrderedGroupNames.push_back(PeersTested[ipt]);
	}
// #else // ! OLD_CODE ----> Even older code
	vector<float> GrpAvgs;
	for (uint ip = 0; ip < PatGroups.size()*2; ip++) {
		float Sum= 0.0f;
		float Avg = 0.0f;
		for (uint im = 0; im < MatchesTbl.size(); im++) {
			Sum += (float)(MatchesTbl[im][ip]);
		}
		if (Sum == 0.0f) {
			Avg = 0;
		}
		else {
			Avg = Sum / (float)(MatchesTbl.size());
		}
		GrpAvgs.push_back(Avg);
	}

	vector<pair<float, int> > PeerScores;
	for (uint im = 0; im < MatchesTbl.size(); im++) {
		float PeerSum = 0;
		for (uint ip = 0; ip < PatGroups.size() * 2; ip++) {
			float GA = GrpAvgs[ip];
			PeerSum += ((GA == 0.0f) ? 0.0f : (((float)(MatchesTbl[im][ip]) / GA)) - GA);
		}
		PeerScores.push_back(make_pair(PeerSum, im));
	}
	sort(PeerScores.begin(), PeerScores.end(), pgt);  
	for (uint ipt = 0; ipt < PeersTested.size(); ipt++)  {
		OrderedGroupNames.push_back(PeersTested[PeerScores[ipt].second]);
	}
#endif // OLD_CODE
	fout.close();
}