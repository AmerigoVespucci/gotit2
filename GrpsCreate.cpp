// GrpsCreate.cpp : Uses ancestor patterns to create groups
//

#include "stdafx.h"

#include "MascReader.h"

void CGotitEnv::CreateGroups()
{
	int im = 1;

	string SpecificFinderName = ImplemParamTbl["Implem.Param.FnParam.CreateGroups.SpecificFinderNm"].Val;
	int SpecificFinder = -1;
	for (uint ifl = 0; ifl < anc_pat_finder_list.size(); ifl++) {
		if (anc_pat_finder_list[ifl].group_name_template == SpecificFinderName) {
			SpecificFinder = ifl;
			break;
		}
	}
	string Seed = ImplemParamTbl["Implem.Param.FnParam.CreateGroups.Seed"].Val;

	for (uint in = 0; in < anc_pat_finder_list.size(); in++) {
		//int rin = rand() * anc_pat_finder_list.size() / RAND_MAX;
		SPatternFinder& first_anc_finder = anc_pat_finder_list[in];
		string group_name = first_anc_finder.group_name_template;
		if (SpecificFinder == -1) {
			if (first_anc_finder.GrpType != cGrpTypeNormal) {
				continue;
			}

			if (im == 0) {
				if (first_anc_finder.create_freq != 0.0f) {
					continue;
				}
			}
			else {
				if (first_anc_finder.create_freq == 0.0f) {
					continue;
				}
				if (((float)rand() / (float)RAND_MAX) > first_anc_finder.create_freq) {
					continue;
				}
			}
		}
		else {
			if ((int)in != SpecificFinder) {
				continue;
			}
		}
		// even if specific finder is not -1, there is no work here if bGrpCreator is false
		if (!first_anc_finder.bGrpCreator) {
			PatGrpMgr.CreatePatGrp(group_name, &first_anc_finder,
									vector<string>(), string(""));
			continue;
		}
		vector<string> members(first_anc_finder.GrpParamList.size());
		vector< vector<string> > GrpParamStrings;
		for (uint igpl = 0; igpl < first_anc_finder.GrpParamList.size(); igpl++) {
			vector< vector<string> > GrpParamStringsCurr;
			SOnePatternFinderGrpParam& GrpParam = first_anc_finder.GrpParamList[igpl];
			if (GrpParam.idx != (int)igpl) {
				cerr << "Error: For now, group parameter indices must be in order: "
					<< first_anc_finder.group_name_template << endl;
			}

			//bool bFurtherFiltering = false;
			//vector<string>* pNewParamsVec = NULL;

			uint NumPrevGrpIters = max(GrpParamStrings.size(), (uint)1);
			for (uint iPrevGrps = 0; iPrevGrps < NumPrevGrpIters; iPrevGrps++) {
				if (GrpParam.sSrcType == "NameListItem") {
					MapStringToNameItems::iterator itnls = BasicTypeLists.find(GrpParam.sSrc);
					if (itnls == BasicTypeLists.end()) {
						cerr << "Error: Name List referenced in finder "
							<< first_anc_finder.group_name_template
							<< " called " << GrpParam.sSrc << ", does not exist\n";
						continue;
					}
					vector<NameItem>& NameListCands = itnls->second;
					vector<NameItem> NameList;
					if (GrpParam.sMatchType == "none") {
						NameList = NameListCands; // full copy
					}
					else if (GrpParam.sMatchType == "GrpParam") {
						uint iGrpParam = stoi(GrpParam.sMatch);
						if (iGrpParam >= GrpParamStrings.size()) {
							cerr << "Error: GrpAnc for MatchWhat = SrcGrpParam is inavlid because insufficient GrpParam values\n";
							return;
						}
						string ToMatchVal = GrpParamStrings[iPrevGrps][iGrpParam];
						for (uint iName = 0; iName < NameListCands.size(); iName++) {
							if (ToMatchVal == NameListCands[iName].Name) {
								NameList.push_back(NameListCands[iName]);
							}
						}
					}
					else if (GrpParam.sMatchType == "GrpParamNot") {
						uint iGrpParam = stoi(GrpParam.sMatch);
						if (iGrpParam >= GrpParamStrings.size()) {
							cerr << "Error: GrpAnc for MatchWhat = SrcGrpParam is inavlid because insufficient GrpParam values\n";
							return;
						}
						string ToMatchVal = GrpParamStrings[iPrevGrps][iGrpParam];
						for (uint iName = 0; iName < NameListCands.size(); iName++) {
							if (ToMatchVal != NameListCands[iName].Name) {
								NameList.push_back(NameListCands[iName]);
							}
						}
					}
					else {
						cerr << "For SrcType = \"NameListItem\" the values accepted for MatchType are: \"none\" (default), GrpParam, GrpParamNot\n";
					}
					vector<string> FillList;
					if (GrpParam.sFillType != "all") {
						if (GrpParam.sSortType == "count") {
							vector<pair<int, int>> ListForSorting;
							//int TotCount = 0;
							for (uint inl = 0; inl < NameList.size(); inl++) {
								ListForSorting.push_back(make_pair(NameList[inl].Count, inl));
								//TotCount += NameList[inl].Count;
							}
							// pair greater<> uses pair < which checks the fisrt of pair and if equal hecks the second
							sort(ListForSorting.begin(), ListForSorting.end(), greater<pair<int, int> >());
							int TopSelect = stoi(GrpParam.sFillPercentile) * NameList.size() / 100;
							int NumToFill = 0;
							if (GrpParam.sFillType == "number") {
								NumToFill = stoi(GrpParam.sFillNumber);
							}
							else if (GrpParam.sFillType == "percentage") {
								NumToFill = stoi(GrpParam.sFillNumber) * NameList.size() / 100;
							}
							else {
								cerr << "Error. GrpParam Fill Type currently supports only: all, number and percentage. Check spelling\n";
							}
							for (int iFill = 0; iFill < NumToFill; iFill++) {
								int iSel = rand() % TopSelect;
								FillList.push_back(NameList[ListForSorting[iSel].second].Name);
							}
						}
						else {
							cerr << "Error. For FillType = all and SrcType = NameListItem, the only valid value for SortType is count\n";
						}
						sort(FillList.begin(), FillList.end());
						vector<string>::iterator itU = unique(FillList.begin(), FillList.end());
						FillList.resize(distance(FillList.begin(), itU));
					}
					else {
						for (uint inl = 0; inl < NameList.size(); inl++) {
							FillList.push_back(NameList[inl].Name);
						}
					}
					//pNewParamsVec = &BackVec;
					//bool bFirstAdd = true;
					//for (uint inl = 0; inl < NameList.size(); inl++) {
					for (uint iFill = 0; iFill < FillList.size(); iFill++) {
						if (GrpParamStrings.size() == 0){
							GrpParamStringsCurr.push_back(vector<string>());
						}
						else {
							GrpParamStringsCurr.push_back(GrpParamStrings[iPrevGrps]);
						}
						vector<string>& BackVec = GrpParamStringsCurr.back();
						//BackVec.push_back(NameList[inl].Name);
						BackVec.push_back(FillList[iFill]);
					}
					//bFurtherFiltering = true;
				}
				else if (GrpParam.sSrcType == "PatGrp") {
					vector<string> FillList;
					CPatGrpHolder * phPatGrpSrc = PatGrpMgr.FindGrp(GrpParam.sSrc);
					if (GrpParam.sFillType != "all") {
						if (phPatGrpSrc != NULL) {
							CPatGrpWrapper PatGrpSrc(phPatGrpSrc);
							vector<pair<int, int>> ListForSorting;
							//int TotCount = 0;
							for (int iEl = 0; iEl < PatGrpSrc.getNumStrongEls(); iEl++) {
#pragma message("Add code for SortType")
								ListForSorting.push_back(make_pair(
									PatGrpSrc.getStrongElByIndex(iEl)->NumOccurs,
									iEl));
								//TotCount += NameList[inl].Count;
							}
							sort(ListForSorting.begin(), ListForSorting.end(), greater<pair<int, int> >());
							int TopSelect = stoi(GrpParam.sFillPercentile) * ListForSorting.size() / 100;
							int NumToFill = 0;
							if (GrpParam.sFillType == "number") {
								NumToFill = stoi(GrpParam.sFillNumber);
							}
							else if (GrpParam.sFillType == "percentage") {
								NumToFill = stoi(GrpParam.sFillNumber) * ListForSorting.size() / 100;
							}
							else {
								cerr << "Error. GrpParam Fill Type currently supports only: all, number and percentage. Check spelling\n";
							}
							for (int iFill = 0; iFill < NumToFill; iFill++) {
								int iSel = rand() % TopSelect;
								FillList.push_back(PatGrpSrc.getStrongElByIndex(ListForSorting[iSel].second)->W);
							}
							for (uint iFill = 0; iFill < FillList.size(); iFill++) {
								if (GrpParamStrings.size() == 0){
									GrpParamStringsCurr.push_back(vector<string>());
								}
								else {
									GrpParamStringsCurr.push_back(GrpParamStrings[iPrevGrps]);
								}
								vector<string>& BackVec = GrpParamStringsCurr.back();
								//BackVec.push_back(NameList[inl].Name);
								BackVec.push_back(FillList[iFill]);
							}
						}
					} // end Fill != all
					else {
						if (phPatGrpSrc != NULL) {
							CPatGrpWrapper PatGrpSrc(phPatGrpSrc);
							for (int inl = 0; inl < PatGrpSrc.getNumStrongEls(); inl++) {
								FillList.push_back(PatGrpSrc.getStrongElByIndex(inl)->W);
							}
						}
					}
					sort(FillList.begin(), FillList.end());
					vector<string>::iterator itU = unique(FillList.begin(), FillList.end());
					FillList.resize(distance(FillList.begin(), itU));
					for (uint iFill = 0; iFill < FillList.size(); iFill++) {
						if (GrpParamStrings.size() == 0){
							GrpParamStringsCurr.push_back(vector<string>());
						}
						else {
							GrpParamStringsCurr.push_back(GrpParamStrings[iPrevGrps]);
						}
						vector<string>& BackVec = GrpParamStringsCurr.back();
						//BackVec.push_back(NameList[inl].Name);
						BackVec.push_back(FillList[iFill]);
					}

				}
				/*
				If the SrcType is "GrpAnc", the source for creating groups are other groups, all of whom derive
				from a single Pattern Group Ancestor (GrpAnc).
				We find such groups and either use the group prarameters of these groups the elements of each group.
				If the SrcStore is "word", then we use the names (W) of the elements of each of the groups. Since there
				are multiple groups and elements may be repeated in the different groups, care must be taken to create
				additional Group Params (GrpParam) for the new groups the indicate the source of the element and make the
				group name (and parameters) unique. In this case each source group found may generate a number of new
				groups.
				If the SrcStore is "SrcGrpParam", each group found can only generate one new group at this level. However,
				further arguments can fan out this one new addition into many. The value of the group parameter from the source
				group is copies into the new group's parameter at the idx value of the node.
				Returning to when SrcStore is "word". In that case all words may be used or a subset may be chosen based
				on random choice biased by the number of occurrences (NumOccurs) of the elements. The groups at this level can
				be filtered by requiring a group parameter of the source group to match a group parameter, previously created,
				of the new group. This does not filter individual elements of the group but rather filters which groups
				are selected.
				To summarize using an example. We have a NodeListItem that creates new groups. It has two GrpParam nodes 
				that define which new groups are to be created. The first is a SrcGrpParam which just picks the groups of some
				other ancesotr - all of them. These are the source groups. We will create as many sets of groups as there are
				source groups. We copy one of the group params of the source group to the group parameters of the new groups.
				Say there were 3 source groups we will be creating 3 sets of new groups with all the groups of the 3 sets having
				a group parameter value corresponding to the one of the source groups. 
				Now comes the second node of our example. Again we search for source groups from an ancestor. This time for each of
				the 3 sets we will only accept a source group if a specific group parameter from that source group matches the group 
				parameter of the new groups we have started to create. We have 3 of these. If the source group is accepted we generate
				new groups for each element (or subset of elements but not in this example). The new groups will have one parameter
				taken from the proto-group of its set and one new parameter which is the "word" copied from the name of the element
				of the sorce group now used to create a new group.
				This process of accepting only specific groups for each of the new sets is done using the parameters as in the following example:
				MatchType="GrpParam"  Match="0" MatchWhat="SrcGrpParam" MatchWhich="0"
				MatchType="GrpParam" means is that this filtering proceeds by requiing a group parameter of the new group to match a 
				parameter of someting. In this case it must match a group parameter of the source group.
				*/
				else if (GrpParam.sSrcType == "GrpAnc") {
					SPatternFinder* pFinderSrc = NULL;
					for (uint iAnc = 0; iAnc < anc_pat_finder_list.size(); iAnc++) {
						if (anc_pat_finder_list[iAnc].group_name_template == GrpParam.sSrc) {
							pFinderSrc = &(anc_pat_finder_list[iAnc]);
							break;
						}
					}
					if (pFinderSrc == NULL) {
						cerr << "Error: GrpAnc Pattern name " << GrpParam.sSrc << " not found for FillGroups\n";
						return;
					}
					vector<pair<int, int>> ListForSorting;
					//int TotCount = 0;
					vector<string> FillList;
					//bool bChildOfAncGrpFound = false;
					vector<CPatGrpHolder*> phPatGrpSrcList;

					for (uint iGrp = 0; iGrp < pFinderSrc->ChildPatGrps.size(); iGrp++) {
						CPatGrpWrapper PatGrpCand(pFinderSrc->ChildPatGrps[iGrp]);
						bool bMatchPassed = true;
						for (uint iMatch = 0; iMatch < GrpParam.MatchList.size(); iMatch++) {
							SPatFinderGrpParamMatch& Match = GrpParam.MatchList[iMatch];
							string SrcGrpParamVal;
							if (Match.sMatchWhat == "SrcGrpParam") {
								uint iSrcGrpParam = stoi(Match.sMatchWhich);
								if (iSrcGrpParam >= pFinderSrc->GrpParamList.size()) {
									cerr << "Error: GrpAnc Pattern name " << GrpParam.sSrc
										<< " does not have enough params for "
										<< iSrcGrpParam << " to do FillGroups\n";
									return;
								}
								SrcGrpParamVal = PatGrpCand.getGrpParam(iSrcGrpParam);
							}
							else if (Match.sMatchWhat == "const") {
								SrcGrpParamVal = Match.sMatchWhich;
							}
							else {
								cerr << "Error. FillGroups: values for GrpParamMatchNode:MatchWhat can be SrcGrpParam or const only.\n";
							}

							string MatchVal;
							if (Match.sMatchType == "GrpParam") {
								uint iGrpParam = stoi(Match.sMatch);
								if (iGrpParam >= GrpParamStrings[iPrevGrps].size()) {
									cerr << "Error: GrpAnc for MatchWhat = SrcGrpParam is inavlid because insufficient GrpParam values\n";
									return;
								}
								MatchVal = GrpParamStrings[iPrevGrps][iGrpParam];
							}
							else if (Match.sMatchType == "const") {
								MatchVal = Match.sMatch;
							}
							else {
								cerr << "Error. FillGroups: values for GrpParamMatchNode:MatchType can be GrpParam or const only.\n";
							}


							if (SrcGrpParamVal != MatchVal) {
								bMatchPassed = false;
							}
						}
						if (bMatchPassed) {
							phPatGrpSrcList.push_back(PatGrpCand.PresentHolder());
						}
						/* remove once code above is debugged
						if (GrpParam.sMatchWhat == "SrcGrpParam") { // Not written well. Should be a test for sMatchType and sMatchWhat inside. Delaying fixing and debugging fix now but please fix and remve this comment
							uint iSrcGrpParam = stoi(GrpParam.sMatchWhich);
							if (iSrcGrpParam >= pFinderSrc->GrpParamList.size()) {
								cerr	<< "Error: GrpAnc Pattern name " << GrpParam.sSrc 
										<< " does not have enough params for " 
										<< iSrcGrpParam << " to do FillGroups\n";
								return;
							}

							string SrcGrpParamVal = PatGrpCand.getGrpParam(iSrcGrpParam);
							
							if (GrpParam.sMatchType == "const") {
								if (SrcGrpParamVal == GrpParam.sMatch) {
									phPatGrpSrcList.push_back(PatGrpCand.PresentHolder());
									// break; // should be breaking out of search among Grps. Only one should fit
								}
							}
							else if (GrpParam.sMatchType == "GrpParam") {
								uint iGrpParam = stoi(GrpParam.sMatch);
								if (iGrpParam >= GrpParamStrings.size()) {
									cerr << "Error: GrpAnc for MatchWhat = SrcGrpParam is inavlid because insufficient GrpParam values\n";
									return;
								}
								if (SrcGrpParamVal == GrpParamStrings[iPrevGrps][iGrpParam]) {
									phPatGrpSrcList.push_back(PatGrpCand.PresentHolder());
									//break; // should be breaking out of search among Grps. Only one should fit
								}
							}
							else {
								cerr << "Error: CreateGroups() MatchType " << GrpParam.sMatchType << " unknown\n";
								return;
							}
						}
						else if (GrpParam.sMatchWhat == "none") {
							phPatGrpSrcList.push_back(PatGrpCand.PresentHolder());
						}
						else {
							cerr << "Error: FillGroups GrpParam: MatchWhat not recognized " << GrpParam.sMatchWhat << endl;
						}
						//TotCount += NameList[inl].Count;
						*/
					} // end loop over groups that are children of ancestor
					for (uint iGrpSrc = 0; iGrpSrc < phPatGrpSrcList.size(); iGrpSrc++) {
					//if (phPatGrpSrc != NULL) {
						CPatGrpWrapper PatGrpSrc(phPatGrpSrcList[iGrpSrc]);

						if (GrpParam.sSrcStore == "SrcGrpParam") {
							string FillWord;
							int iSrcParam = stoi(GrpParam.sSrcStoreWhich);
							int NumParams = PatGrpSrc.getNumGrpParams();
							if (iSrcParam >= NumParams) {
								cerr << "Error. CreateGroups. SrcGrpParam. Cannot request a group param index beyond what is allocated to groups of this size.\n";
								break;
							}
							FillWord = PatGrpSrc.getGrpParam(iSrcParam);
							FillList.push_back(FillWord);
						}
						else if (GrpParam.sSrcStore == "word") {
							if (iGrpSrc > 0) {
								break; //  For \"word\" (default) option of SrcStore, there can only be one group, so we take the first
							}
							vector<pair<int, int>> ListForSorting;
							//int TotCount = 0;
							for (int iEl = 0; iEl < PatGrpSrc.getNumStrongEls(); iEl++) {
#pragma message("Add code for SortType")
								ListForSorting.push_back(make_pair(
									PatGrpSrc.getStrongElByIndex(iEl)->NumOccurs,
									iEl));
								//TotCount += NameList[inl].Count;
							}
							sort(ListForSorting.begin(), ListForSorting.end(), greater<pair<int, int> >());
							int TopSelect = stoi(GrpParam.sFillPercentile) * ListForSorting.size() / 100;
							int NumToFill = 0;
							bool bRandSel = true;
							if (GrpParam.sFillType == "number") {
								NumToFill = stoi(GrpParam.sFillNumber);
							}
							else if (GrpParam.sFillType == "percentage") {
								NumToFill = stoi(GrpParam.sFillNumber) * ListForSorting.size() / 100;
							}
							else if (GrpParam.sFillType == "all") {
								bRandSel = false;
								NumToFill = ListForSorting.size();
							}
							else {
								cerr << "Error. GrpParam Fill Type currently supports only: all, number and percentage. Check spelling\n";
							}
							int ActualToFill = min(NumToFill, TopSelect);
							for (int iFill = 0; iFill < ActualToFill; iFill++) {
								int iSel;
								if (bRandSel) {
									iSel = rand() % TopSelect; // side effect of min above is that 0 is not possible
								}
								else {
									iSel = iFill;
								}
								FillList.push_back(PatGrpSrc.getStrongElByIndex(ListForSorting[iSel].second)->W);
							}
						}
						else {
							cerr << "Error. GrpParam SrcStore currently supports only: word and SrcGrpParam. Check spelling\n";
						}

						sort(FillList.begin(), FillList.end());
						vector<string>::iterator itU = unique(FillList.begin(), FillList.end());
						FillList.resize(distance(FillList.begin(), itU));
						for (uint iFill = 0; iFill < FillList.size(); iFill++) {
							if (GrpParamStrings.size() == 0){
								GrpParamStringsCurr.push_back(vector<string>());
							}
							else {
								GrpParamStringsCurr.push_back(GrpParamStrings[iPrevGrps]);
							}
							vector<string>& BackVec = GrpParamStringsCurr.back();
							//BackVec.push_back(NameList[inl].Name);
							BackVec.push_back(FillList[iFill]);
						}
						FillList.clear();
					}

				}
				else if (GrpParam.sSrcType == "ExtParam") {
					//vector<string> OneMemberVec;
					//OneMemberVec.push_back(Seed);
					//GrpParamStringsCurr.push_back(OneMemberVec);
					if (GrpParamStrings.size() == 0){
						GrpParamStringsCurr.push_back(vector<string>());
					}
					else {
						GrpParamStringsCurr.push_back(GrpParamStrings[iPrevGrps]);
					}
					vector<string>& BackVec = GrpParamStringsCurr.back();
					BackVec.push_back(Seed);
				}
				else {
					cerr << "Unknown Src Type parameter " << GrpParam.sSrcType
						<< " passed to finder "
						<< first_anc_finder.group_name_template << endl;
					continue;
				}
			} // loop over iPrevGrps
			sort(GrpParamStringsCurr.begin(), GrpParamStringsCurr.end());
			vector< vector<string> >::iterator itU = unique(GrpParamStringsCurr.begin(), GrpParamStringsCurr.end());
			GrpParamStringsCurr.resize(distance(GrpParamStringsCurr.begin(), itU));
			GrpParamStrings.clear();
			GrpParamStrings = GrpParamStringsCurr; // full copy. Consider optimizing!

		} // loop over GrpParamList definitins of GrpParam of the current pattern anc

#ifdef OLD_CODE
		uint TotalNumGrps = min(GrpParamStrings.size(), 1);
		uint NumParamsPerGrp = first_anc_finder.GrpParamList.size();
		vector<string> OneSetOfGrpParams(NumParamsPerGrp);
		for (uint igps = 0; igps < GrpParamStrings.size(); igps++) {
			TotalNumGrps *= GrpParamStrings[igps].size();
		}
		uint NumGrpsForThisOne = TotalNumGrps;
		uint NumLoops = 1;
		vector<vector<string> > AllGrpParams(TotalNumGrps, OneSetOfGrpParams);
		for (uint igps = 0; igps < GrpParamStrings.size(); igps++) {
			uint NumThisOne = GrpParamStrings[igps].size();
			NumGrpsForThisOne /= NumThisOne;
			for (uint iLoop = 0; iLoop < NumLoops; iLoop++) {
				for (uint iPut = 0; iPut < NumThisOne; iPut++) {
					for (uint iRep = 0; iRep < NumGrpsForThisOne; iRep++) {
						AllGrpParams[(((iLoop * NumThisOne) + iPut) * NumGrpsForThisOne) + iRep][igps] = GrpParamStrings[igps][iPut];
					}
				}
			}
			NumLoops *= NumThisOne;
		}
#endif //OLD_CODE

		uint NumParamsPerGrp = first_anc_finder.GrpParamList.size();
		vector<string> SrprsGrpNames(GrpParamStrings.size());
		if (first_anc_finder.SurpriseGrpFinder.sGrpAnc.size() > 0) {
			for (uint itng = 0; itng < GrpParamStrings.size(); itng++) {
				string SrprsGrpAncName = "";
				SPatternFinder* pSrprsGrpAnc = NULL;
				for (uint ial = 0; ial < anc_pat_finder_list.size(); ial++) {
					//int rin = rand() * anc_pat_finder_list.size() / RAND_MAX;
					SPatternFinder& GrpAnc = anc_pat_finder_list[ial];
					SrprsGrpAncName = GrpAnc.group_name_template;
					if (SrprsGrpAncName == first_anc_finder.SurpriseGrpFinder.sGrpAnc) {
						pSrprsGrpAnc = &GrpAnc;
						break;
					}
				}
				if (pSrprsGrpAnc != NULL) {
					bool bSrprsGrpFound = false;
					uint iSrprsGrp;
					for (uint iGrp = 0; iGrp < pSrprsGrpAnc->ChildPatGrps.size() && !bSrprsGrpFound; iGrp++) {
						CPatGrpWrapper PatGrp(pSrprsGrpAnc->ChildPatGrps[iGrp]);
						if (first_anc_finder.SurpriseGrpFinder.sGrpParamMatchType0 == "none") {
							iSrprsGrp = iGrp;
							bSrprsGrpFound = true;
							break;
						}
						if (	first_anc_finder.SurpriseGrpFinder.sGrpParamMatchType0
							== "const") {
							if (	GrpParamStrings[itng][0] 
								!=	first_anc_finder.SurpriseGrpFinder.sGrpParamMatchArg0) {
								continue; // not found, move to next Grp
							}
						}
						else if (	first_anc_finder.SurpriseGrpFinder.sGrpParamMatchType0 
								==	"MyGrpParam") {
							int NumParams = PatGrp.getNumGrpParams();
							int iParam = stoi(
											first_anc_finder.SurpriseGrpFinder.sGrpParamMatchArg0);
							if (iParam >= NumParams) {
								cerr << "Error. CreateGroups. Cannot request a group param index beyond what is allocated to groups of this size.\n";
								break;
							}
							if (GrpParamStrings[itng][0] != PatGrp.getGrpParam(iParam)) {
								continue; // not found, move to next Grp
							}

						}
						if (NumParamsPerGrp > 0) {
							if (first_anc_finder.SurpriseGrpFinder.sGrpParamMatchType1 == "none") {
								iSrprsGrp = iGrp;
								bSrprsGrpFound = true;
								break;
							}
							if (first_anc_finder.SurpriseGrpFinder.sGrpParamMatchType1
								== "const") {
								if (GrpParamStrings[itng][1]
									!= first_anc_finder.SurpriseGrpFinder.sGrpParamMatchArg1) {
									continue; // not found, move to next Grp
								}
							}
							else if (first_anc_finder.SurpriseGrpFinder.sGrpParamMatchType1
								== "MyGrpParam") {
								int NumParams = PatGrp.getNumGrpParams();
								int iParam = stoi(
									first_anc_finder.SurpriseGrpFinder.sGrpParamMatchArg1);
								if (iParam >= NumParams) {
									cerr << "Error. CreateGroups. Cannot request a group param index beyond what is allocated to groups of this size.\n";
									break;
								}
								if (GrpParamStrings[itng][1] != PatGrp.getGrpParam(iParam)) {
									continue; // not found, move to next Grp
								}

							}
						} // end iof search on second parameter valid
						iSrprsGrp = iGrp;
						bSrprsGrpFound = true;

					} // end loop over surprise group finder's children groups
					if (bSrprsGrpFound) {
						CPatGrpWrapper PatGrp(pSrprsGrpAnc->ChildPatGrps[iSrprsGrp]);
						string SrprsGrpName;
						if (PatGrp.getGrpName(SrprsGrpName)) {
							SrprsGrpNames[itng] = SrprsGrpName;
						}
					}

				} // end if there is a SurpriseGrpFinder

			} // end if there is a Surprise Grp Finder to search for
		} // loop over GrpParamStrings that WILL create a group each

		for (uint itng = 0; itng < GrpParamStrings.size(); itng++) {
			group_name = first_anc_finder.group_name_template;
			for (uint ippg = 0; ippg < NumParamsPerGrp; ippg++) {
				string tstr = std::to_string(ippg);
				group_name += "[%" + tstr + "%" + GrpParamStrings[itng][ippg] + "]";
			}
			PatGrpMgr.CreatePatGrp(	group_name, &first_anc_finder, 
									GrpParamStrings[itng], SrprsGrpNames[itng]);
		}

	} // loop over # create pattern groups per text

}

