// GrpSeed.cpp : Implements a strategy for creating groups from a seed.
//

#include "stdafx.h"

#include "MascReader.h"
#include "rf/librf/instance_set.h"
#include "rf/librf/random_forest.h"

using namespace rapidxml;
using namespace std;
using namespace librf;

#ifdef OLD_CODE
void CGotitEnv::CreateGrpsFromSeed(string Seed, vector<CGroupingRec*>& RelevantGroups)
{
	//flog.open("log.txt", fstream::out);
	// loop creating groups
	//map<string&, CGroupingRec*> RelevantGroups;
	for (uint in = 0; in < anc_pat_finder_list.size(); in++) {
		//int rin = rand() * anc_pat_finder_list.size() / RAND_MAX;
		SPatternFinder& first_anc_finder = anc_pat_finder_list[in];
		if (first_anc_finder.GrpType != cGrpTypeSeed) {
			continue;
		}
		int iBoth = 1;
		if (first_anc_finder.group_name_template == "GrpSeedAncPrec") {
			iBoth = 0;
		}
		vector<NameItem>* pOptions = first_anc_finder.node_list[iBoth].pOptions;
		for (uint ip = 0; ip < pOptions->size(); ip++) {
			string chosen = (*pOptions)[ip].Name;
			string GrpName;
			if (iBoth == 0) {
				GrpName = "SeedGrp[%" + chosen + "%]Prec[%" + Seed + "%]";
			}
			else {
				GrpName = "SeedGrp[%" + Seed + "%]FollowedBy[%" + chosen + "%]";
			}

			map_name_to_pat_grp::iterator itGrpTbl = PatGrpTbl.find(GrpName);
			CGroupingRec * patgrp;
			if (itGrpTbl == PatGrpTbl.end()) {
				patgrp = new CGroupingRec;
				patgrp->Name = GrpName;
				patgrp->pfinder = &(anc_pat_finder_list[in]);
				patgrp->total_strength = 0.0;
				patgrp->members.push_back((iBoth == 0) ? chosen : Seed);
				patgrp->members.push_back((iBoth == 0) ? Seed : chosen);
				PatGrpTbl[patgrp->Name] = patgrp;
				patgrp->pfinder->ChildGroups.push_back(patgrp);
				patgrp->GrpType = cGrpTypeSeed;
			}
			else {
				patgrp = itGrpTbl->second;
			}
			RelevantGroups.push_back(patgrp);
			
		}
	}
	vector<SSentenceRec> SentenceRec;
	vector<string> OnlyIfWordsPresent; // NULL in  initial group creation
	OnlyIfWordsPresent.push_back(Seed);
	ImplemParamTbl["Implem.Param.FnParam.LoadSentenceListOneMod.OnlyIfWordsPresent"].Val = Seed;
	int imStart = 0;
	if (PatGrpTbl.size() > 0) {
		imStart = 1;
	}
	const uint cMinSentenceRecs = 1000;
	int NumMods = ModNamesCntrl.getSize(); // 
	for (int im = imStart; im < NumMods; im++) {
		if (SentenceRec.size() > cMinSentenceRecs) {
			break;
		}
		//int rim = rand() * ModNames.getSize() / RAND_MAX;

		//int rim = im;
		if (im != 0) {
			LoadSentenceListOneMod();
		}
	}
	for (uint irg = 0; irg != RelevantGroups.size(); irg++) {
		CGroupingRec * patgrp = RelevantGroups[irg];
		if (patgrp->GrpType != cGrpTypeSeed) {
			continue; // unlikely
		}

		if (((float)rand() / (float)RAND_MAX) > patgrp->pfinder->run_prob) {
			continue;
		}
		OneSearch(*patgrp, SentenceRec);
		patgrp->CreateStrengthSort(BaseWordCount);
	} // end loop over pattern groups to search on and strengthen

	//flog.close();
}
#endif // OLD_CODE

