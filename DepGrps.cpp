// DepGrps.cpp : Finds members for groups by searching the linear records of a sentence as well as the dep records
// Should be called GrpSearch or GrpAddElements
//

#include "stdafx.h"

#include "MascReader.h"

bool CreateStoreWord(string& StoreWord, SOnePatternFinderNode& PatNode, int iRec, vector<WordRec>& WordRecs)
{
	StoreWord.clear();
	if (iRec >= (int)WordRecs.size()) {
		return false;
	}
	//WordRec& WRec = WordRecs[iRec];
	if (PatNode.sStore == "word") {
		StoreWord = WordRecs[iRec].Word;
	}
	else if (PatNode.sStore == "pos") {
		StoreWord = WordRecs[iRec].POS;
	}
	else if (PatNode.sStore == "ner") {
		StoreWord = WordRecs[iRec].NER;
	}
	else if (PatNode.sStore == "idx") {
		StoreWord = to_string(iRec);
	}
	else if (PatNode.sStore == "none") {
		StoreWord.clear();
	}
	else {
		cerr << "Unkown store directive " << PatNode.sStore << endl;
	}

	return (StoreWord.length() > 0);
}


bool CreateStoreWord(string& StoreWord, SOnePatternFinderNode& PatNode, DepRec& DRec, vector<WordRec>& WordRecs, int id)
{
	int iRec;
	StoreWord.clear();
	bool bFromRec = false;
	if (PatNode.sStoreType == "gov") {
		iRec = DRec.Gov;
		bFromRec = true;
	}
	else if (PatNode.sStoreType == "dep") {
		iRec = DRec.Dep;
		bFromRec = true;
	}
	if (bFromRec) {
		if (iRec >= (int)WordRecs.size()) {
			return false;
		}
		if (PatNode.sStore == "word") {
			StoreWord = WordRecs[iRec].Word;
		}
		else if (PatNode.sStore == "pos") {
			StoreWord = WordRecs[iRec].POS;
		}
		else if (PatNode.sStore == "ner") {
			StoreWord = WordRecs[iRec].NER;
		}
		else if (PatNode.sStore == "idx") {
			StoreWord = to_string(iRec);
		}
		else {
			cerr << "Unkown store directive " << PatNode.sStore << endl;
		}
	}
	else {
		if (PatNode.sStoreType == "rec") {
			if (PatNode.sStore == "idx") {
				StoreWord = to_string(id);
			}
			else {
				cerr << "Unkown store directive for StoreType rec " << PatNode.sStore << endl;
			}
		}
		else if (PatNode.sStoreType == "rel") {
			if (PatNode.sStore == "idx") {
				StoreWord = to_string(DRec.iDep);
			}
			else if (PatNode.sStore == "word") {
				cerr << "Not implemented storing the name of the rel yet\n";
			}
			else {
				cerr << "Unkown store directive for StoreType rec " << PatNode.sStore << endl;
			}
		}
	}

	return (StoreWord.length() > 0);

}

void SolutionsUpdate(string& StoreWord, int NumTotalMemUs, int iMemU, vector<vector<string> >& Solutions, vector<vector<string> >& SolutionsSnapshot)
{
	vector<string> OneSolution(NumTotalMemUs);
	if ((SolutionsSnapshot.size() == 0) && (iMemU >= 0)) {
		Solutions.push_back(OneSolution);
		Solutions.back()[iMemU] = StoreWord;
	}
	else {
		for (uint iss = 0; iss < SolutionsSnapshot.size(); iss++) {
			Solutions.push_back(SolutionsSnapshot[iss]);
			if (iMemU >= 0) {
				Solutions.back()[iMemU] = StoreWord;
			}
		}
	} // else of if (SolutionsSnapshot.size() == 0)  
}

void OneSearchOnDep(MapStringToNameItems& NameLists,
	map<string, int>& DepTypes,
	CPatGrpWrapper& NewGrouping,
	vector<SSentenceRec> & SentenceList,
	CGotitEnv * pEnv
	)
{
	SPatternFinder * pFinder = NULL;
	NewGrouping.getGrpFinder(pFinder);
	if (pFinder == NULL) {
		cerr << "Error: OneSearchOnDep. No group may have NULL finder.\n";
		return;
	}
	//bool bSentenceReset = false;
	//int iStartRec = 0; // for SearchNextRec, we kep going from the last rec
	vector<string> ElsToAdd;
	for (uint isl = 0; isl < SentenceList.size(); isl++) {
		FillOneGrpForOneSRec(	NameLists, DepTypes, NewGrouping, SentenceList[isl], 
								pEnv, pFinder, ElsToAdd);
		AddElsToGrp(NewGrouping, ElsToAdd, pEnv);
		for (uint iEl = 0; iEl < ElsToAdd.size(); iEl++) {

		}
	}
#ifdef WIERD_CODE
	for (uint is = 0; is < SentenceList.size(); is++) {
		for (uint ir = 0; ir < SentenceList[is].OneWordRec.size(); ir++) {

			WordRec& wrec = SentenceList[is].OneWordRec[ir];
			// error. don't leave. Exists depends on more thsn one word and not only base
			// Previous line old comment. Not sure what it means.
			NewGrouping.incrGrpElNumExists(wrec.Word);
		}
	}
#endif // WIERD_CODE
}

void AddElsToGrp(CPatGrpWrapper& NewGrouping, vector<string>& NewEls, CGotitEnv * pEnv)
{
	for (uint iEl = 0; iEl < NewEls.size(); iEl++) {
		int NumOccurs = -1;
		NewGrouping.addGrpEl(NewEls[iEl], NumOccurs);
		string GrpName;
		NewGrouping.getGrpName(GrpName);
		pEnv->AddWordGrpLink(NewEls[iEl], NewGrouping.getGrpID(), NumOccurs);
	}

}

void FillOneGrpForOneSRec(	MapStringToNameItems& NameLists,
							map<string, int>& DepTypes,
							CPatGrpWrapper& NewGrouping,
							SSentenceRec & SRec,
							CGotitEnv * pEnv,
							SPatternFinder * pFinder,
							vector<string>& ElsToAdd)
{
	//SPatternFinder * pFinder = NULL;
	//NewGrouping.getGrpFinder(pFinder);
	//if (pFinder == NULL) {
	//	cerr << "Error: OneSearchOnDep. No group may have NULL finder.\n";
	//	return;
	//}
	bool bSentenceReset = false;
	int iStartRec = 0; // for SearchNextRec, we kep going from the last rec
//	for (uint isl = 0; isl < SentenceList.size(); isl++) {
	bool bLoopDone = false;
	while (true) {
		if (bSentenceReset) {
			bSentenceReset = false;
			//isl--; // can't happen before isl > 0
		}
		else {
			//iStartRec = 0;
			if (bLoopDone) {
				return;
			}
		}
		bLoopDone = true;
		vector<string> OneSolution(pFinder->NumTotalMemUs);
		vector<vector<string> > Solutions;
		bool bStopWordMatched = false; // used only for SearchNextRec
		bool bLastNodeReached = false;
		for (uint inl = 0; inl < pFinder->node_list.size(); inl++) {
			vector<vector<string> > SolutionsSnapshot = Solutions; // a full copy
			Solutions.clear(); // We we build up a new set of solutions (with a new layer) if there are any
			if ((inl + 1) == pFinder->node_list.size()) {
				bLastNodeReached = true; // make sure we we're forced to break early
			}
			SOnePatternFinderNode& PatNode = pFinder->node_list[inl];
			int iMemU = -1;
			if (PatNode.NodeIdx != -1) {
				iMemU = PatNode.NodeIdx;
			}
			if (PatNode.StoreIdx != -1) { // should be else if, but we don't force it for now
				iMemU = PatNode.StoreIdx + pFinder->NumLocalMemUs;
			}
			string sReqWord;
			bool bDoSearch = true;
			bool bMemSearch = false;
			bool bNameListReq = false;
			//bool bGroupReq = false;
			vector<NameItem>* pNameListReq = NULL;
			if (PatNode.sReqType == "GrpParam") {
				int iParam = stoi(PatNode.sReq);
				sReqWord = NewGrouping.getGrpParam(iParam);
			}
			else if (PatNode.sReqType == "const") {
				sReqWord = PatNode.sReq;
			}
			else if (PatNode.sReqType == "NodeMem") {
				sReqWord = PatNode.sReq;
				bMemSearch = true;
			}
			else if (PatNode.sReqType == "NameListItem") {
				//cerr << "Error. Feature not implemented yet. Need to pass in NameLists\n";
				sReqWord = PatNode.sReq;
				MapStringToNameItems::iterator itnl = NameLists.find(sReqWord);
				if (itnl == NameLists.end()) {
					cerr << "Error. NameListItem specified in config file " << sReqWord << " does not exist.\n";
				}
				else {
					pNameListReq = &(itnl->second);
					bNameListReq = true;
				}
			}
			else if (PatNode.sReqType == "PatGrp") {
				cerr << "Error. Feature not implemented yet. Need to pass in PatGrps\n";
				sReqWord = PatNode.sReq;
				//bGroupReq = true;
			}
			else if (PatNode.sReqType == "none") {
				sReqWord = "";
				bDoSearch = false;
			}
			else {
				cerr << "unknown ReqType directive " << PatNode.sReqType << endl;
				continue;
			}
			string StoreWord;
			bool bEmptySolutionUpdate = false;
			if (PatNode.sSelType == "SearchAllDeps") {
				if (bMemSearch) {
					cerr << "Error. Currently SearchAllDeps cannot be combined with NodeMem in the ReqType field\n";
					bDoSearch = false;
				}
				for (uint id = 0; id < SRec.Deps.size(); id++) {
					DepRec DRec = SRec.Deps[id];
					if (PatNode.sSel == "rel") {
						int iRel = DepTypes[sReqWord];
						if (!bDoSearch || (DRec.iDep == iRel)) {
							if (CreateStoreWord(StoreWord, PatNode, DRec, 
												SRec.OneWordRec, id)) {
								SolutionsUpdate(StoreWord, pFinder->NumTotalMemUs, iMemU, 
												Solutions, SolutionsSnapshot);
							}
						} // if (DRec.iDep == iRel)
					} // if (PatNode.sSel == "rel") 
					else if (PatNode.sSel == "none") {
						string sEmpty = "";
						SolutionsUpdate(sEmpty, pFinder->NumTotalMemUs, -1, Solutions, SolutionsSnapshot);
					}
					else {
						cerr << "Unknown sel directive " << PatNode.sSel << " for SelType SearchAllDeps\n";
					}
				} // end of loop over Deps of Sentence rec
			} // if (PatNode.sSelType == "SearchAllDeps")
			else if (PatNode.sSelType == "SearchNextRec") {
				// if we get here, we cancel any previous sentence continue directives
				// this is because it is only meant to apply of te last SearchNextRec in the list sets it
				bSentenceReset = false;
				if (bMemSearch) {
					cerr << "Error. Currently SearchAllDeps cannot be combined with NodeMem in the ReqType field\n";
					bDoSearch = false;
				}
				uint ir;
				for (ir = iStartRec; ir < SRec.OneWordRec.size(); ir++) {
					WordRec& wrec = SRec.OneWordRec[ir];
					string SrcWord;
					if (PatNode.sSel == "word") {
						SrcWord = wrec.Word;
					}
					else if (PatNode.sSel == "base") {
						cerr << "Warning. This feature was moved to word. Is this a leftover?\n";
						SrcWord = wrec.WordCore;
					}
					else if (PatNode.sSel == "pos") {
						SrcWord = wrec.POS;
					}
					else if (PatNode.sSel == "ner") {
						SrcWord = wrec.NER;
					}
					else if (PatNode.sSel == "idx") {
						SrcWord = to_string(ir); // not sure how this would be used
					}
					else {
						cerr << "Unkown sel directive " << PatNode.sSel << endl;
					}

					bool bWordMatched = false;
					if (!bDoSearch) {
						cerr << "Error. Having no match to search for in SearchNextRec is not implmented currently. \n";
					}
					else {
						if (bNameListReq) {
							for (uint iv = 0; iv < pNameListReq->size(); iv++) {
								if (SrcWord == (*(pNameListReq))[iv].Name) {
									bWordMatched = true;
									break;
								}
							}
						}
						else if (SrcWord == sReqWord) {
							bWordMatched = true;
						}
						// put other search options such as NameList and Group here
					}

					// positioning the stop search here allows us to override the stop
					// action if a match is found. All that has happened is that we recorded
					// the find, we still did not take action ie storing the word
					if (PatNode.sat != satNoStop) {
						for (uint iv = 0; iv < pFinder->p_stop_list->size(); iv++) {
							if (wrec.POS == (*(pFinder->p_stop_list))[iv].Name) {
								// if code reaches here, a stop word has been found and we will start
								// the snode list from scratch but we still move on in the words
								bStopWordMatched = true;
								if (bWordMatched && (PatNode.sat == satStop)) {
									bWordMatched = false;
								}
								// satIncludeAndStop not processed explicitly. If we reach here and
								// it is the current option bStopWordMatched will be true and if BWordMatched
								// is true it will remain true
							}
						}
					}

					// what I believe will happen if bSentenceReset is true is that when the node list
					// gets to the end and its true the list will be processed again
					// however, next time if there are no words left the loop through will be ineffective
					// also, bSentenceReset is made false whenever such a node is 
					if (bWordMatched) {
						CreateStoreWord(StoreWord, PatNode, ir, 
										SRec.OneWordRec);
						SolutionsUpdate(StoreWord, pFinder->NumTotalMemUs, iMemU,
										Solutions, SolutionsSnapshot);						
						bSentenceReset = true;
						iStartRec = ir + 1;
						break; // do not continue with words in this search node

					}

					if (bStopWordMatched) {
						bSentenceReset = true;
						iStartRec = ir + 1;
						break;
					}

				} // end loop over ir - words records in sentence
				if (bStopWordMatched || (ir >= SRec.OneWordRec.size())) {
					// can only be true at this point if there has been a SearchNextRec earlier in the node list
					// FUTURE CODE WARNING, don't add another loop inside the inl loop but outside
					// this statement or we will break out of the wrong loop
					break; // break out of the inl loop 
				}
			}
			else if (PatNode.sSelType == "NodeDepLookup") {
				int iNodeMem = stoi(PatNode.sSel);
				if (bDoSearch) {
					cerr << "error. Currently NodeDepLookup cannot be combined with a ReqType other than none. Resetting ReqType\n";
					bDoSearch = false; // not used in the following code anyway
				}
				for (uint iss = 0; iss < SolutionsSnapshot.size(); iss++) {
					if (iNodeMem >= 0 && (iNodeMem < (int)SolutionsSnapshot[iss].size())) {
						// It is possible for SolutionSnapshot to contain empty entries
						// The reaso for this is that SearchAllDeps and SearchAllRecs always start from scratch
						// even if there are no solutions in the earlier nodes. They will thus create solutions
						// These solutions are winnowed out if ReqType is not "none". However, it is sometimes,
						// and therefore the solutions are not winnowed out
						// This can be valid all the way to the end as long as solutions are not needed from
						// searches prior to the last Search. The following if will make sure that if the 
						// earlier solutions are needed, then the solution as a whole will fail
						string sid = SolutionsSnapshot[iss][iNodeMem];
						if (sid.size() > 0) { // otherwise, we have an empty solution position at iNodeMem
							int id = stoi(sid);
							DepRec DRec = SRec.Deps[id];
							if (CreateStoreWord(StoreWord, PatNode, DRec,
												SRec.OneWordRec, id)) {
								Solutions.push_back(SolutionsSnapshot[iss]);
								if (iMemU >= 0) {
									Solutions.back()[iMemU] = StoreWord;
								}
							}
						}
					}
					else {
						cerr << "Error for NodeDepLookup. Index of rec in search is " << iNodeMem << endl;
					}
				}
			}
			else if (PatNode.sSelType == "NodeRecLookup") {
				if (bDoSearch) {
					cerr << "error. Currently NodeRecLookup cannot be combined with a ReqType other than none. Resetting ReqType\n";
					bDoSearch = false; // not used in the following code anyway
				}
				int iNodeMem = stoi(PatNode.sSel);
				for (uint iss = 0; iss < SolutionsSnapshot.size(); iss++) {
					if (iNodeMem >= 0 && (iNodeMem < (int)SolutionsSnapshot[iss].size())) {
						string sid = SolutionsSnapshot[iss][iNodeMem];
						if (sid.size() > 0) { // otherwise, we have an empty solution position at iNodeMem
							int id = stoi(sid);
							if (CreateStoreWord(StoreWord, PatNode, id,
												SRec.OneWordRec)) {
								Solutions.push_back(SolutionsSnapshot[iss]);
								if (iMemU >= 0) {
									Solutions.back()[iMemU] = StoreWord;
								}
							}
						}
					}
					else {
						cerr << "Error for NodeRecLookup. Index of rec in search is " << iNodeMem << endl;
					}
				}

			}
			else if (PatNode.sSelType == "NodeMem") {
				int iNodeSel = stoi(PatNode.sSel);
				for (uint iss = 0; iss < SolutionsSnapshot.size(); iss++) {
					if (iNodeSel >= 0 && (iNodeSel < (int)SolutionsSnapshot[iss].size())) {
						string sMatch = SolutionsSnapshot[iss][iNodeSel];
						bool bAddFound = false;
						if (!bDoSearch) {
							bAddFound = true;
						}
						else if (bMemSearch) {
							if (sReqWord.size() == 0) {
								cerr << "Error. FillGroups using NodeMem on rule " << inl << ", search requested on ReqType of none\n";
							}
							else {
								int iReqMem = stoi(sReqWord);
								string sReqMemWord = SolutionsSnapshot[iss][iReqMem];
								if ((sMatch.size() > 0) && (sMatch == sReqMemWord)) { // == for strings should not include empty solutions
									bAddFound = true;
								}
							}
						}
						else if ((sMatch.size() > 0) && (sMatch == sReqWord)) {
							bAddFound = true;
						}
						if (bAddFound) {
							Solutions.push_back(SolutionsSnapshot[iss]);
							if (iMemU >= 0) {
								Solutions.back()[iMemU] = StoreWord;
							}
						}
					} // end if iGrpMem in range
					else {
						cerr << "Error for NodeMem. Out of range index of rec in search is " << iNodeSel << endl;
					}
				}

			}
			else if (PatNode.sSelType == "none") {
				bEmptySolutionUpdate = true;
			}
			else {
				cerr << "Unknown SelType directive " << PatNode.sSelType << endl;
			}
			if (bEmptySolutionUpdate) {
				string sEmpty = "";
				SolutionsUpdate(sEmpty, pFinder->NumTotalMemUs, iMemU, Solutions, SolutionsSnapshot);
			}
		} // end of loop over pattern nodes
		if (bLastNodeReached) {
			SGroupingLink InitLink;
			for (uint isns = 0; isns < Solutions.size(); isns++) {
				//vector<string>& OneSolution = Solutions[isns];
				string ElName;
				for (int ist = 0; ist < pFinder->NumStoreMemUs; ist++) {
					if (ist>0) {
						ElName += '\t';
					}
					ElName += Solutions[isns][ist + pFinder->NumLocalMemUs];
				}
				//cout << "OneSearchOnDep: searching for members for group: " << GrpName << ". Solution found for " << ElName << " in sentence: " << SRec.Sentence << endl;
				ElsToAdd.push_back(ElName);

				//int NumOccurs = -1;
				//NewGrouping.addGrpEl(ElName, NumOccurs);
				//string GrpName;
				//NewGrouping.getGrpName(GrpName);
				//pEnv->AddWordGrpLink(ElName, NewGrouping.getGrpID(), NumOccurs);
				//NumElsAdded++;
			} // end loop over solutions
		} // end if bLastNodeReached
	} // end of loop over sentence recs
}
