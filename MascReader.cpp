// TestRapidXml.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "MascReader.h"
#include "./RF/librf/instance_set.h"
#include "./RF/librf/discrete_dist.h"

using namespace rapidxml;
using namespace std;
using namespace librf;

#define MODULE "The_Black_Willow"

#define LOW_ASCII 0x0
#define LOW_TEST	7
#define UTF_TEST	6
#define UTF_START	(uchar)0x3 // 11
#define UTF_CONT	(uchar)0x2 // 10


//vector<vector<SSentenceRec> > gSentenceDB;

fstream flog;


#ifdef OLD_CODE
// SNodes are group nodes for finding patterns withi sentences
// DNodes are for finding patterns between sentences
// This function's job is to add 
void OneSearch(CGroupingRec& NewGrouping,
				vector<SSentenceRec> & SentenceList
				)
{
	// stats
	NewGrouping.NumSearches++;
	NewGrouping.NumRecsSearched += SentenceList.size();
	vector<vector<string> > ComboSets;
	vector<string> OneSet(NewGrouping.pfinder->num_stores);
	SGroupingLink InitLink;
	uint NumSNodes = NewGrouping.pfinder->node_list.size();
	//vector<SSentenceRec>& SentenceList = SentenceRecList[itModMap->second];

	//int cb = 0;
	for (uint is = 0; is < SentenceList.size(); is++) {
		uint ir = 0;
		while (ir < SentenceList[is].OneWordRec.size()) { // keep looping over node list till end of sentence
			bool NodeFound = true;
			bool bSNodeEnd = false;
			bool bDelayedReset = false;
			for (uint isn = 0; (isn < NumSNodes) && !bSNodeEnd; isn++) {
				// the following flag is set to false on every iteration
				// of teh snode list so. even the last must be found
				bool ThisNodeFound = false;
				//OneSet.clear();
				//OneSet[1] = "false";
				for (; ir < SentenceList[is].OneWordRec.size() && !ThisNodeFound; ir++) {
					bool bSentenceReset = false;
					WordRec& wrec = SentenceList[is].OneWordRec[ir];
					//if (wrec.WordBase == "about") {
					//	cb++;
					//}
					string SrcWord;
					vector<SOnePatternFinderNode>& NodeList = NewGrouping.pfinder->node_list;
					switch (NodeList[isn].Sel) { // etSrcSel)	{
						case etBaseWord:
							SrcWord = wrec.WordCore;
							cerr << "Accessing deprecated WordBase\n";
							break;
						case etWord:
							SrcWord = wrec.Word;
							break;
						case etPOS:
							SrcWord = wrec.POS;
							break;
						case etGrouping:
							break;
						default:
							break;
					}
					bool b_word_matches = false;
					if (NodeList[isn].b_oneof || NodeList[isn].bOneItem) {
						if (SrcWord == NewGrouping.members[isn]) {
							b_word_matches = true;
						}
					}
					else {
						for (uint iv = 0; iv < NodeList[isn].pOptions->size(); iv++) {
							if (SrcWord == (*(NodeList[isn].pOptions))[iv].Name) {
								b_word_matches = true;
								break;
							}
						}

					}
					// positioning the stop search here allows us to override the stop
					// action if a match is found. All that has happened is that we recorded
					// the find, we still did not take action ie storing the word
					for (uint iv = 0; iv < NewGrouping.pfinder->p_stop_list->size(); iv++) {
						if (wrec.POS == (*(NewGrouping.pfinder->p_stop_list))[iv].Name) {
							if (	!b_word_matches 
								||	(NodeList[isn].sat == satStop)) {
								// if code reaches here, a stop word has been found and we will start
								// the snode list from scratch but we still move on in the words
								isn = 0;
								bSentenceReset = true;
								break;
							}
						}
					}
					if (bSentenceReset) {
						continue;
					}
					if (b_word_matches) {
						ThisNodeFound = true;
						if (NodeList[isn].StoreIdx != -1) {
							switch (NodeList[isn].Store) { // etSrcSel)	{                       
								case etBaseWord:
									OneSet[NodeList[isn].StoreIdx] = wrec.WordCore;
									cerr << "Error accesing deprecated WordBase\n";
									break;
								case etWord:
									OneSet[NodeList[isn].StoreIdx] = wrec.Word;
									break;
								case etPOS:
									OneSet[NodeList[isn].StoreIdx] = wrec.POS;
									break;
								case etGrouping:
									break;
								default:
									break;
							}
						} // end if storing
							//if (NodeList[isn].bReset) {
							//	bSNodeEnd = true;
							//	break;
							//}
							// incorrect must use param
							// OneSet[0] = wrec.WordBase;
						ir++; 
						break; // break to next node, since this one is fulfilled
					} // end if word matches
				} // end of loop of words on sentence
				// if we run out of words without getting  a ThisNodeFound
				// we have failed
				// Even if a node is found in a non-last snode
				// we will not have successeded because we simply moved to the 
				// next in the snode list, which resets ThisNodeFound
				if (!ThisNodeFound /* && !NodeList[isn].bReset */) {
					NodeFound = false; // next sentence
					break;
				}
			} // end loop over nodes
			if (NodeFound) {
				string SearchStr;
				for (int ist = 0; ist < NewGrouping.pfinder->num_stores; ist++) {
					if (ist>0) {
						SearchStr += '\t';
					}
					SearchStr += OneSet[ist];
				}
				//flog << "hi";
				if (NewGrouping.Name == "SeedGrp[%CC%]Prec[%for%]") {
					//if (SearchStr == "fact") {
					//	flog << "break: ";
					//}
					flog	<< "Adding to " << NewGrouping.Name << ": " << SearchStr << " from sentence :"
							<< SentenceList[is].Sentence << "\n";
				}
				MapMemberToGroup::iterator iFound = NewGrouping.MemberLinks.find(SearchStr);
				if (iFound == NewGrouping.MemberLinks.end()) {
					NewGrouping.MemberLinks[SearchStr] = InitLink;
					iFound = NewGrouping.MemberLinks.find(SearchStr);
				}
				else {
					iFound->second.NumOccurs++;
				}
			} // end node found processing for the list of nodes
		} // end keep looping over the wholw node list again
	} // end loop over sentences
	for (uint is = 0; is < SentenceList.size(); is++) {
		for (uint ir = 0; ir < SentenceList[is].OneWordRec.size(); ir++) {

			WordRec& wrec = SentenceList[is].OneWordRec[ir];
			// error. don't leave. Exists depends on more thsn one word and not only base
			MapMemberToGroup::iterator iFound = NewGrouping.MemberLinks.find(wrec.Word);
			//cerr << "Moved WordBase to word. Is this what you wanted?\n";
			if (iFound != NewGrouping.MemberLinks.end()) {
				iFound->second.NumExist++;
			}
		}
	}
		//for (; ir < SentenceList[is].OneWordRec.size(); ir++) {
		//	WordRec& wrec = SentenceList[is].OneWordRec[ir];
		//	// incorrect must use param
		//	if (wrec.POS == "IN") {
		//		OneSet[1] = wrec.WordBase;
		//		MapMemberToGroup::iterator iFound = NewGrouping.MemberLinks.find(OneSet[0]);
		//		if (iFound == NewGrouping.MemberLinks.end()) {
		//			NewGrouping.MemberLinks[OneSet[0]] = InitLink;
		//			iFound = NewGrouping.MemberLinks.find(OneSet[0]);
		//		}
		//		else {
		//			iFound->second.NumOccurs++;
		//		}
		//		break;
		//	}
		//	// incorrect must use param
		//	if (wrec.WordBase == "and" || wrec.WordBase == ",") {
		//		break;
		//	}
		//}
		//ComboSets.push_back(OneSet);
		//int IncrConnected = 1;
		//if (OneSet[1] == "false") {
		//	IncrConnected = 0;
		//}
		//iFound->second.NumConnected += IncrConnected;

}

#endif // OLD_CODE

void StringStore(const string& s, ofstream& fout, int& NumBytesWritten)
{
	uint ssize = s.size();
	fout.write((char *)&(ssize), sizeof(ssize));
	fout << s;
	NumBytesWritten += sizeof(ssize)+ssize;
}

void StringStore(const string& s, fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	int NumBytesWritten = 0;
	StringStore(s, fout, NumBytesWritten);
}

void StringStore(const string& s, ofstream& fout)
{
	int NumBytesWritten = 0;
	StringStore(s, fout, NumBytesWritten);
}

void StringLoad(string& s, ifstream& fin)
{
	s.clear();
	uint ssize = 0;
	fin.read((char *)&(ssize), sizeof(ssize));
	if (!fin) {
		cout << "Potential Error. Only " << fin.gcount() << " chars read instead of " << sizeof(ssize) << " bytes\n";
		return;
	}
	for (uint n = 0; n < ssize; n++) {
		char c;
		fin.read(&c, sizeof(c));
		s += c;
	}
}

void StringLoad(string& s, fstream& f)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	StringLoad(s, fin);
}

void BWCountStore(MapBaseWordToCount& bwc, fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	uint NumWords = bwc.size();
	fout.write((char *)&(NumWords), sizeof(NumWords));
	MapBaseWordToCount::iterator it = bwc.begin();
	for (; it != bwc.end(); it++) {
		StringStore(string(it->first), fout);
		fout.write((char *)&(it->second), sizeof(it->second));
	}
}

void BWCountLoad(MapBaseWordToCount& bwc, fstream& f)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	uint NumWords;
	fin.read((char *)&NumWords, sizeof(NumWords));
	for (uint i = 0; i < NumWords; i++) {
		string w;
		int n;
		StringLoad(w, fin);
		fin.read((char *)&n, sizeof(n));
		bwc[w] = n;
	}
}

#define FWRITE(fout, val, nb) fout.write((char *)&val, sizeof(val)); nb += sizeof(val)

void SGroupingLink::Store(ofstream& fout, int& NumBytesWritten)
{
	StringStore(W, fout, NumBytesWritten);
	FWRITE(fout, NumOccurs, NumBytesWritten);
	FWRITE(fout, NumExist, NumBytesWritten);
	FWRITE(fout, PctHit, NumBytesWritten);
	FWRITE(fout, Strength, NumBytesWritten);
	FWRITE(fout, bStrong, NumBytesWritten);
	FWRITE(fout, SurprisingNess, NumBytesWritten);
}

void SGroupingLink::Store(ofstream& fout)
{
	int NumBytesWritten = 0;
	Store(fout, NumBytesWritten);
}

void SGroupingLink::Load(ifstream& fin)
{
	StringLoad(W, fin);
	fin.read((char *)&NumOccurs, sizeof(NumOccurs));
	fin.read((char *)&NumExist, sizeof(NumExist));
	fin.read((char *)&PctHit, sizeof(PctHit));
	fin.read((char *)&Strength, sizeof(Strength));
	fin.read((char *)&bStrong, sizeof(bStrong));
	fin.read((char *)&SurprisingNess, sizeof(SurprisingNess));

}

#ifdef OLD_CODE
void CGroupingRec::Store(fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	StringStore(Name, fout);
	fout.write((char *)&total_strength, sizeof(total_strength));
	fout.write((char *)&NumSearches, sizeof(NumSearches));
	fout.write((char *)&NumRecsSearched, sizeof(NumRecsSearched));
	fout.write((char *)&bFromMerge, sizeof(bFromMerge));
	StringStore(pfinder->group_name_template, fout);

	uint NumNodesInFinder = members.size();
	fout.write((char *)&NumNodesInFinder, sizeof(NumNodesInFinder));
	for (uint im = 0; im < NumNodesInFinder; im++) {
		StringStore(members[im], fout);
	}

	uint NumMembers = MemberLinks.size();;
	fout.write((char *)&NumMembers, sizeof(NumMembers));

	MapMemberToGroup::iterator img = MemberLinks.begin();
	for (; img != MemberLinks.end(); img++) {
		img->second.Store(fout);
	}
	// add the following to Load below
	StringStore(GrpType, fout); 
}

void CGroupingRec::Load(fstream& f, vector<SPatternFinder >& anc_pat_finder_list,
						map_name_to_pat_grp& pat_grp_tbl, MapBaseWordToCount& BaseWordCount)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	StringLoad(Name, fin);
	fin.read((char *)&total_strength, sizeof(total_strength));
	fin.read((char *)&NumSearches, sizeof(NumSearches));
	fin.read((char *)&NumRecsSearched, sizeof(NumRecsSearched));
	fin.read((char *)&bFromMerge, sizeof(bFromMerge));
	string FinderName;
	StringLoad(FinderName, fin);
	for (uint ia = 0; ia < anc_pat_finder_list.size(); ia++) {
		if (FinderName == anc_pat_finder_list[ia].group_name_template) {
			pfinder = &(anc_pat_finder_list[ia]);
			map_name_to_pat_grp::iterator itGrpTbl = pat_grp_tbl.find(Name);
			if (itGrpTbl == pat_grp_tbl.end()) {
				pat_grp_tbl[Name] = this;
				pfinder->ChildGroups.push_back(this);
			}
			else {
				cerr << "Why are we loading a group again?\n";
//				patgrp = itGrpTbl->second;
			}
			break;
		}
	}

	uint NumNodersInFinder;
	fin.read((char *)&NumNodersInFinder, sizeof(NumNodersInFinder));
	for (uint im = 0; im < NumNodersInFinder; im++) {
		string MemberName;
		StringLoad(MemberName, fin);
		members.push_back(MemberName);
	}

	uint NumMembers;
	fin.read((char *)&NumMembers, sizeof(NumMembers));
	for (uint img = 0; img < NumMembers; img++) {
		SGroupingLink InitLink;
		InitLink.Load(fin);
		MemberLinks[InitLink.W] = InitLink; // full copy here
	}

	CreateStrengthSort(BaseWordCount);

	StringLoad(GrpType, fin);
}
#endif // OLD_CODE

void SaveWordProps(vector<SSentenceRec>& Src, ofstream& fout, ofstream& foutText)
{
	uint NumRecs = Src.size();
	fout.write((char *)&(NumRecs), sizeof(NumRecs));
	for (uint ir = 0; ir < NumRecs; ir++) {
		uint NumWords = Src[ir].OneWordRec.size();
		fout.write((char *)&(NumWords), sizeof(NumWords));
		for (uint iw = 0; iw < NumWords; iw++) {
			string& Word = Src[ir].OneWordRec[iw].Word;
			StringStore(Word, fout);
			string& POSWord = Src[ir].OneWordRec[iw].POS;
			StringStore(POSWord, fout);
		}
		foutText << ir+1 << ". " << Src[ir].Sentence << endl; 
	}

}

void LoadWordProps(vector<vector<WordProps> >& SentenceTbl, ifstream& fin)
{
	uint NumRecs;
	fin.read((char *)&(NumRecs), sizeof(NumRecs));
	for (uint ir = 0; ir < NumRecs; ir++) {
		vector<WordProps> Sentence;
		SentenceTbl.push_back(Sentence);
		uint NumWords;
		fin.read((char *)&(NumWords), sizeof(NumWords));
		for (uint iw = 0; iw < NumWords; iw++) {
			WordProps WordProp;
			SentenceTbl[ir].push_back(WordProp);
			string WordRead;
			StringLoad(WordRead, fin);
			SentenceTbl[ir][iw].Word = WordRead;
			WordRead.clear();
			StringLoad(WordRead, fin);
			SentenceTbl[ir][iw].POS = WordRead;
		}
	}
}



ElType itoet(string s) {
	if (s == "base") {
		return etBaseWord;
	}
	if (s == "word") {
		return etWord;
	}
	if (s == "pos") {
		return etPOS;
	}
	if (s == "group") {
		return etGrouping;
	}
	return etUnused;
}

// if bIfExt is true, we're looking for FindExt 
// removal, replacement etc. is just on the name
// no modification yet
void CModNames::LoadModuleNames(string PathName, bool bIfExt, string FindExt, 
								bool bRemoveExt, string NewExt, int Limit)
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif

#undef MASC
#ifdef MASC
	path CorePath = "C:\\MASC-3.0.0";
#else
	//path CorePath = "D:\OANC\OANC-GrAF\data";
	path CorePath = PathName;
#endif

	recursive_directory_iterator endit;
	recursive_directory_iterator it(CorePath);
	int NumAdded = 0; // can't use .size cos it may not have come in empty
	for (; it != endit; ++it)
	{
		if ((Limit > 0) && (NumAdded >= Limit)) {
			break;
		}
		auto& apath = it->path();
#ifdef GOTIT_LINUX
		string nm = apath.string();
#else        
		string nm = apath.file_string();
#endif        
		if (is_directory(apath)) {
			//cout << "Dir ";
		}
		else {
			//cout << "File: ";
			bool bAdd = true;
			string modname = nm;
			if (bIfExt) {
				size_t fpos;
				fpos = nm.find(FindExt);
				if (fpos == string::npos) {
					bAdd = false;
				}
				else {
					if (bRemoveExt) {
						modname = nm.substr(0, fpos);
					}
				}
			}
			if (bAdd) {
				ModNames.push_back(modname + NewExt);
				NumAdded++;
			}
		}
		//cout << nm << endl;

	}

}

string& CModNames::GetRandModName(bool bOOB)
{
	int rim;
	while (true) {
		rim = rand() % ModNames.size();
        if (ModNames.size() < cOOBDenom) {
            // for very small number of files, OOB is irrelevant.
            return ModNames[rim];
        }
		if ((rim % cOOBDenom) > 0) {
			if (!bOOB) return ModNames[rim];
		}
		else {
			if (bOOB) return ModNames[rim];
		}
	}
}


bool CModNames::GetSeqModName(string& ModNameRet) // returns true if last
{
	ModNameRet = ModNames[iSeq];
	iSeq++;
	return ((uint)iSeq >= ModNames.size());
}

uint CModNames::getSize()
{
	return ModNames.size();
}

//void CreateExList(MapBaseWordToCount& BaseWordCount, vector<string>& ModNames)
//{
//	vector<SSentenceRec> SentenceRec;
//	for (int im = 0; im < 150; im++) {
//		int rim = rand() * ModNames.size() / RAND_MAX;
//		vector<string> OnlyIfWordsPresent; // NULL in  initial group creation
//		OnlyIfWordsPresent.push_back("that");
//		CreateSentenceListOneMod(ModNames[rim], SentenceRec,
//								BaseWordCount, OnlyIfWordsPresent);
//	}
//
//	{
//		ofstream foutWords("WordProps.bin", ios::binary);
//		ofstream foutText("ExSentences.txt");
//		SaveWordProps(SentenceRec, foutWords, foutText);
//	}
//}
//

#ifdef OLD_CODE

void CGotitEnv::StartWithGroups()
{
	using namespace std::tr2::sys;

	flog.open("log.txt", fstream::out);

	//vector<vector<SOnePatternFinderNode> > NodeList; // list of relus for how to create new groupings

	vector<CGroupingRec *> PatGrpsSortedByStrongMembers;
	const int cSortEveryNumModules = 10;
	const int cMergeEveryNumModules = 10;
	const int cIntersectThresh = 4;
	const bool cbStopNode = true;
	int imStart = 0;
	if (PatGrpTbl.size() > 0) {
		imStart = 1;
	}
	int NumMods = ModNamesCntrl.getSize(); // 15; // 
	for (int im = imStart; im < NumMods; im++) {
		//int rim = rand() * ModNamesCntrl.size() / RAND_MAX;
		//int rim = im;
		SentenceRec.clear();
		//vector<string> OnlyIfWordsPresent; // NULL in  initial group creation
		if (im != 0) {
			LoadSentenceListOneMod(	);
		}
		// loop creating groups
		CreateGroups();

		// process sentences to fill groups
		if (SentenceRec.size() > 0) {
			// loop executing pat grps
			map_name_to_pat_grp::iterator itpg = PatGrpTbl.begin();
			for (; itpg != PatGrpTbl.end(); itpg++) {
				CGroupingRec * patgrp = itpg->second;
				//if (patgrp->GrpType != cGrpTypeNormal) {
				//	continue;
				//}

				if (((float)rand() / (float)RAND_MAX) > patgrp->pfinder->run_prob) {
					continue;
				}
				//if (patgrp->pfinder->SrcTextType == "dep") {
				OneSearchOnDep(BasicTypeLists, PatGrpTbl, DepTypes, *patgrp, SentenceRec);
				//}
				//else {
				//	OneSearch(*patgrp, SentenceRec);
				//}
				patgrp->CreateStrengthSort(BaseWordCount);
			} // end loop over pattern groups to search on and strengthen
		} // end if there are sentences to search on
		// every so often create sorted list of succesful groups
		if ((im % cSortEveryNumModules) == 0) {
			PatGrpsSortedByStrongMembers.clear();
			// not resetting bStrong. Assuming Strong membership is monotonic
			map_name_to_pat_grp::iterator itpg = PatGrpTbl.begin();
			for (; itpg != PatGrpTbl.end(); itpg++) {
				CGroupingRec * patgrp = itpg->second;
				if (patgrp->StrongMembers.size() > 5) {
					PatGrpsSortedByStrongMembers.push_back(patgrp);
				}
			}
		}
		//cMergeEveryNumModules
		if ((im % cMergeEveryNumModules) == 0) {
			SGroupingLink InitLink;
			for (uint in = 0; in < anc_pat_finder_list.size(); in++) {
				//int rin = rand() * anc_pat_finder_list.size() / RAND_MAX;
				SPatternFinder& first_anc_finder = anc_pat_finder_list[in];
				for (uint ig0 = 0; ig0 < first_anc_finder.ChildGroups.size(); ig0++) {
					for (uint ig1 = 0; ig1 < first_anc_finder.ChildGroups.size(); ig1++) {
						if (ig0 == ig1) {
							continue;
						}
						int NumIntersect = 0;
						CGroupingRec* pGroup0 = first_anc_finder.ChildGroups[ig0];
						CGroupingRec* pGroup1 = first_anc_finder.ChildGroups[ig1];
						vector<string> Intersects;
						for (uint ism = 0; ism < pGroup0->StrongMembers.size(); ism++) {
							string sMember = pGroup0->StrongMembers[ism]->W;
							MapMemberToGroup::iterator itML = pGroup1->MemberLinks.find(sMember);
							if (itML != pGroup1->MemberLinks.end()) {
								if (itML->second.bStrong) {
									NumIntersect++;
									Intersects.push_back(sMember);
								}
							}
						}
						if (NumIntersect >= cIntersectThresh) {
							// first we do a BAD update by doing a replace instead
							int imgFound = -1;
							for (uint img = 0; img < first_anc_finder.MergedGroups.size(); img++) {
								CGroupingRec * mgrp = first_anc_finder.MergedGroups[img];
								if (	(mgrp->MergedFromList[0] == pGroup0->Name)
									&&	(mgrp->MergedFromList[1] == pGroup1->Name)) {
									delete mgrp;
									vector<CGroupingRec *>::iterator itdel = MergedGroupsList.begin();
									for (; itdel != MergedGroupsList.end(); itdel++) {
										if (*itdel == mgrp) {
											MergedGroupsList.erase(itdel);
											break;
										}
									}
									imgFound = img;
									break;
								}
							}
							//do merged groups here
							CGroupingRec * mgrp = new CGroupingRec();
							mgrp->bFromMerge = true;
							mgrp->pfinder = NULL;
							for (uint ii = 0; ii < Intersects.size(); ii++) {
								string sMember = Intersects[ii];
								mgrp->MemberLinks[sMember] = InitLink;
								SGroupingLink& Link = (mgrp->MemberLinks.find(sMember))->second;
								Link.W = sMember;
								int Num = pGroup0->MemberLinks[sMember].NumOccurs;
								Num += pGroup1->MemberLinks[sMember].NumOccurs;
								Link.NumOccurs = Num;
							}
							mgrp->MergedFromList.push_back(pGroup0->Name);
							mgrp->MergedFromList.push_back(pGroup1->Name);
							if (imgFound != -1) {
								first_anc_finder.MergedGroups[imgFound] = mgrp;
							}
							else {
								first_anc_finder.MergedGroups.push_back(mgrp);
							}
							MergedGroupsList.push_back(mgrp);
						}
					} // inner loop over groups descended from anc pattern
				} // outer loop over groups descended from anc pattern

			}  // for each pat finder ancestor
		} // if this module is one every that we want a merge do
	}// loop over modules (texts)

	//SaveGroups(PatGroups, PatGrpTbl, BaseWordCount);
	return;


	///map_name_to_pat_grp::iterator ipg;

	//for (ipg = PatGrpTbl.begin(); ipg != PatGrpTbl.end(); ipg++) {
	//	ipg->second->CreateStrengthSort(BaseWordCount);
	//}
	flog.close();

}
#endif //OLD_CODE
static const int cMaxDistance = 8;

#ifdef OLD_CODE
void CreateRFData(vector<CGroupingRec *>& PatGroups, MapStringToNameItems& NameLists)
{
	ifstream fileIn("WordProps.bin", ios::binary);
	ifstream SupervizedLabels("ThatSup.txt");
	ofstream RFOut("ThatEx.csv");
	ofstream RFLabel("ThatExLabel.csv");
	ofstream RFOutOOB("ThatExOOB.csv");
	ofstream RFLabelOOB("ThatExLabelOOB.csv");
	vector<vector<WordProps> > SentenceTbl;
	LoadWordProps(SentenceTbl, fileIn);

	//vector<string> Headers;
	vector<string>& POSNames = NameLists["AllPOSList"];
	for (uint ip = 0; ip < POSNames.size(); ip++) {
		//Headers.push_back(POSNames[ip]);
		if (POSNames[ip] == ",") {
			RFOut << ",";
			RFOutOOB << ",";
		}
		else {
			RFOut << POSNames[ip] << ",";
			RFOutOOB << POSNames[ip] << ",";
		}
	}
	RFOut << endl;
	RFOutOOB << endl;
	uint NumRecs = SentenceTbl.size();
	string SupLine;
	while (getline(SupervizedLabels, SupLine)) {
		bool bOOB = ((rand() % 4) != 0);
		ofstream& ThisRFOut = (bOOB ? RFOut : RFOutOOB);
		ofstream& ThisRFLabel = (bOOB ? RFLabel : RFLabelOOB);
		int LineNo;
		int OneNotZero;
		istringstream iss(SupLine);
		if ((iss >> LineNo) && (iss >> OneNotZero)) {
			uint ir = LineNo - 1;
			vector<int> DistanceVals(POSNames.size(), 0);
			int RefIndex = -1;
			// search through the senstence fining the FIRST instance
			// of the reference word. RefIndex becomes the index
			// of that word within the sentence
			for (uint iw = 0; iw < SentenceTbl[ir].size(); iw++) {
				if (SentenceTbl[ir][iw].Word == "that") {
					RefIndex = iw;
					break;
				}
			}
			if (RefIndex == -1) {
				continue;
			}
			int FirstWordAnalyzed = max(RefIndex - cMaxDistance + 1, 0);
			int LastWordAnalyzed = min(RefIndex + cMaxDistance - 1, (int)(SentenceTbl[ir].size()) - 1);
			for (int iw = FirstWordAnalyzed; iw <= LastWordAnalyzed; iw++) {
				if (iw == RefIndex) {
					continue;
				}
				bool bFound = false;
				for (uint ip = 0; ip < POSNames.size(); ip++) {
					if (SentenceTbl[ir][iw].POS == POSNames[ip]) {
						int WouldBe = ((iw > RefIndex)
							? (cMaxDistance - (iw - RefIndex))
							: (-(cMaxDistance - (RefIndex - iw))));
						// test that this is the closest instance to the ref
						if (abs(DistanceVals[ip]) < abs(WouldBe)) {
							DistanceVals[ip] = WouldBe;
						}
						bFound = true;
						break;
					} // end if POS matches
				} // end loop over POS referfnce values
			} // end loop over words in sentence
			for (uint id = 0; id < DistanceVals.size(); id++) {
				ThisRFOut << DistanceVals[id] << ".0,";
			}
			ThisRFOut << endl;
			if (OneNotZero != 0) {
				ThisRFLabel << "1.0" << endl;
			}
			else {
				ThisRFLabel << "0.0" << endl;
			}
		}
	}
}
#endif // OLD_CODE

bool gtNameItems(const NameItem& r1, const NameItem& r2)
{
	if (r1.Count > r2.Count) return true;
	return false;

}

void OrderWords(MapBaseWordToCount& BaseWordCount,
				vector<NameItem >& WordsInOrder)
{
	WordsInOrder.clear();
	MapBaseWordToCount::iterator it = BaseWordCount.begin();
	for (; it != BaseWordCount.end(); it++) {
		WordsInOrder.push_back(NameItem(it->first, it->second));
	}
	sort(WordsInOrder.begin(), WordsInOrder.end(), gtNameItems);
}

#ifdef OLD_CODE

void CGotitEnv::CreateRFErrExList(	bool bImplementAsPair, // Implementation Strategy Parameter. Instead of errors, differentitate between two anchors
						string FirstOfPair ,
						string SecondOfPair,
						float& ScoreBroad, float& ScoreSel,
						vector<int>& SelGroupIndices,
						SGrpSeed& Seed0, SGrpSeed& Seed1
						)
{
	if (FirstOfPair == "") FirstOfPair = "for";
	if (SecondOfPair == "") SecondOfPair = "president";

	stringstream RFOut; // ("ThatEx.csv");
	stringstream RFLabel; // ("ThatExLabel.csv");
	stringstream RFOutOOB; //  ("ThatExOOB.csv");
	stringstream RFLabelOOB; //  ("ThatExLabelOOB.csv");

	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	uint NumVars = 0;
	for (uint ip = 0; ip < PatGroups.size(); ip++) {
		////Headers.push_back(POSNames[ip]);
		//if (POSNames[ip] == ",") {
		//	RFOut << ",";
		//	RFOutOOB << ",";
		//}
		//else {
		string nm = PatGroups[ip]->Name;
		size_t pos;
		if ((pos = nm.find(",")) != string::npos) {
			nm.replace(pos, 1, "comma");
		}
		RFOut << nm << ",";
		RFOutOOB << nm << ",";
		NumVars++; // cuurently same as PatGrous.size but may change
		//}
	}
	RFOut << endl;
	RFOutOOB << endl;
	if (!bImplementAsPair) {
		//BaseWordCount.clear();
		//ifstream fileBWCount("WordCount.bin", ios::binary);
		//BWCountLoad(BaseWordCount, fileBWCount);
		OrderWords(BaseWordCount, WordsInOrder);
	}
	vector<SSentenceRec> SentenceRec;
	vector<vector<string> > TestSentences;
	vector<bool> ErrSentenceLabels;
	uint PrevSententenceRecSize = 0;
	vector<vector<int> > Distances; // 2D table. Each row a test sentence. Each column  the distance of a pat group to the anchor
	uint NumIn = 0;
	uint NumOOB = 0;
	string Anchor = FirstOfPair;

	// create test sentences
	uint NumTestRecs = 0;
	uint cMinTestRecs = 300;
	uint ModRuns = min(ModNamesCntrl.getSize() / 2, 500);
	uint NumLabel0 = 0;
	uint NumLabel1 = 0;
	int NumOverLoops; // the data preparation stage is called NumOverLoops times
	if (bImplementAsPair) {
		NumOverLoops = ((Seed0.RFPeers.size() > 0) ? 2 : 1);
	}
	else {
		NumOverLoops = 1;
	}
	for (int iOverLoop = 0; iOverLoop < NumOverLoops; iOverLoop++) {
		for (uint im = 0; im < ModRuns; im++) {
			//if ((NumOOB + NumIn) > cMinTestRecs) {
			if (NumIn > cMinTestRecs) {
				break;
			}
			//int rim = rand() * ModNames.size() / RAND_MAX;
			bool bLabel0 = true; // if true the sentence will be label 0 else label 1
			if (bImplementAsPair) {
				if (iOverLoop == 0) {
					FirstOfPair = Seed0.SeedName;
					SecondOfPair = Seed1.SeedName;
				}
				else {
					FirstOfPair = Seed0.RFPeers[rand() % Seed0.RFPeers.size()];
					SecondOfPair = Seed1.RFPeers[rand() % Seed1.RFPeers.size()];
				}
			}
			if (bImplementAsPair) {
				if (NumLabel0 < NumLabel1) {
					bLabel0 = true;
					Anchor = FirstOfPair;
				}
				else {
					bLabel0 = false;
					Anchor = SecondOfPair;
				}
			}
			bool bOOB = ((rand() % 2) == 0);
			if (iOverLoop > 0) { // on the second run we generate no OOB data
				// this means that we build a tree by mixing in Peers
				// but we test the tree on the real swaps only
				// The test data can be small (but not 0), the tree building
				// needs a serious number of examples
				bOOB = false;
			}
			vector<string> OnlyIfWordsPresent; // NULL in  initial group creation
			OnlyIfWordsPresent.push_back(Anchor);
			ImplemParamTbl["Implem.Param.FnParam.LoadSentenceListOneMod.OnlyIfWordsPresent"].Val = Anchor;
			LoadSentenceListOneMod();

			int RefIndex = -1;
			for (uint ir = PrevSententenceRecSize; ir < SentenceRec.size(); ir++) {
				if (!bImplementAsPair) {
					if ((rand() % 2) == 0) {
						bLabel0 = true;
					}
					else {
						bLabel0 = false;
					}
				}
				if (bOOB) NumOOB++; else NumIn++;
				stringstream& ThisRFOut = (!bOOB ? RFOut : RFOutOOB);
				stringstream& ThisRFLabel = (!bOOB ? RFLabel : RFLabelOOB);
				Distances.push_back(vector<int>(PatGroups.size(), 0));
				vector<int>& DistancesThisSentence = Distances.back();
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
					cerr << "Anchor " << Anchor << "not found\n";
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
					if (!bImplementAsPair) {
						static int cMaxErrDist = 3;
						static int cMaxFromMostUsed = 200;
						int ErrDist = (rand() % ((cMaxErrDist * 2) + 1) - cMaxErrDist); // distance from anchorword
						if (ErrDist == 0) ErrDist = 1;
						ErrRep = max(RefIndex + ErrDist, 0);
						ErrRep = min(ErrRep, (int)(Words.size()) - 1);
						// currently very poor results areseen for a random alternate word
						// the first option is commented therefore 
						// however (1) it should be presented as an implementaition param instead
						// and (2) it may work by combining the trees from many of the words in the
						// sentence

						// int SelChosen = rand() % cMaxFromMostUsed;
						// Words[ErrRep].WordBase = WordsInOrder[SelChosen].first;

						Words[ErrRep].Word = SecondOfPair;
					}
				} // end if we modify this guy

				//if (ErrSentenceLabels.size() != (NumIn + NumOOB)) {
				//	cerr << "ErrLabels out of sync\n";
				//}

				int FirstWordAnalyzed = max(RefIndex - cMaxDistance + 1, 0);
				int LastWordAnalyzed = min(RefIndex + cMaxDistance - 1, (int)(Words.size()) - 1);
				for (int iw = FirstWordAnalyzed; iw <= LastWordAnalyzed; iw++) {
					if (iw == RefIndex) {
						continue;
					}
					bool bFound = false;
					//#define TEST_WITH_POS
#ifdef TEST_WITH_POS
					int WouldBe = ((iw > RefIndex)
						? (cMaxDistance - (iw - RefIndex))
						: (-(cMaxDistance - (RefIndex - iw))));
					if (iw != ErrRep) {
						for (uint ip = 0; ip < PatGroups.size(); ip++) {
							if (PatGroups[ip]->members[0] == Words[iw].POS) {
								// test that this is the closest instance to the ref
								if (abs(DistancesThisSentence[ip]) < abs(WouldBe)) {
									DistancesThisSentence[ip] = WouldBe;
								}
							}
						}
					}
					else {
						uint ip = 5; //  rand() % PatGroups.size();
						if (abs(DistancesThisSentence[ip]) < abs(WouldBe)) {
							DistancesThisSentence[ip] = WouldBe;
						}
					}
#else// TEST_WITH_POS
					static const float cRealityCheckThresh = 0.05f;
					for (uint ip = 0; ip < PatGroups.size(); ip++) {
						MapMemberToGroup::iterator itm = PatGroups[ip]->MemberLinks.find(Words[iw].Word);
						if (itm != PatGroups[ip]->MemberLinks.end())  {
							float RealityCheck = (float)itm->second.NumOccurs / (float)itm->second.NumExist;
							if (bImplementAsPair) {
								RealityCheck = 1.0;
							}
							if (RealityCheck < cRealityCheckThresh) {
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
#endif // TEST_WITH_POS
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
			PrevSententenceRecSize = SentenceRec.size();

		} // end loop over modules
		cout << "Num LearnSet " << NumIn << " and Num OOB " << NumOOB << " for sym " << FirstOfPair << " and " << SecondOfPair << " in loop " << iOverLoop <<  endl;
	} // end over loop

	if (NumOOB < 1) {
		cout << "Cannot create even one non-peer test case! Aborting\n";
		ScoreBroad = 0.0; ScoreSel = 0.0;
		return;
	}
	InstanceSet * LearnSet = InstanceSet::LoadFromSS(RFOut, RFLabel, true);
	InstanceSet * LearnSetOOB = InstanceSet::LoadFromSS(RFOutOOB, RFLabelOOB, true);
	int K = (int)(floor(sqrt((double)NumVars)));
	vector< pair< float, int> > ranking;
	{
		RandomForest rf(*LearnSet, 100, K);
		float sscore = rf.training_accuracy();
		ScoreBroad = rf.testing_accuracy(*LearnSetOOB);
		unsigned int seed = rand();
		rf.variable_importance(&ranking, &seed);
	}
	if (ScoreBroad != ScoreBroad) {
		ScoreBroad = 0.5f;
	}
	float vari = (float)NumLabel0 / ((float)NumLabel0 + (float)NumLabel1);


	cout << "rf score:" << ScoreBroad << "\n";

	vector<int> SelAttribs;
	for (int ir = 0; ir < 20; ir++) {
		SelAttribs.push_back(ranking[ir].second);
		SelGroupIndices.push_back(ranking[ir].second);
	}
	InstanceSet * IrisSetSel = InstanceSet::feature_select(*LearnSet, SelAttribs);
	InstanceSet * IrisSetOOBSel = InstanceSet::feature_select(*LearnSetOOB, SelAttribs);
	{
		RandomForest rfSel(*IrisSetSel, 100, 5);
		ScoreSel = rfSel.testing_accuracy(*IrisSetOOBSel);
	}
	if (ScoreSel != ScoreSel) {
		ScoreSel = 0.5f;
	}

	cout << "sel rf score:" << ScoreSel << "\n";
	delete IrisSetOOBSel;
	delete IrisSetSel;
	delete LearnSetOOB;
	delete LearnSet;

//	{
//		ofstream foutWords("WordProps.bin", ios::binary);
//		ofstream foutText("ExSentences.txt");
//		SaveWordProps(SentenceRec, foutWords, foutText);
//	}
}

#endif // OLD_CODE

#ifdef OLD_CODE
bool GroupingSort(CGroupingRec * r1, CGroupingRec * r2)
{
	if (r1->MemberLinks.size() > r2->MemberLinks.size()) {
		return true;
	}

	return false;
}
#endif // OLD_CODE

static const uint cSafeopenWaitTime = 1000; // ms. Note linux sleep is in seconds so / 1000 when used
static const uint cSafeOpenNumTries = 5;


bool TestFileExists(string& FName)
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif
	return (exists(path(FName)));
}

bool CreateDirectory(string& DirName)
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif
	return (create_directory(path(DirName)));
    
}

bool FileRemove(string& FName)
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif
	return remove(path(FName));
}

bool FileRename(string& FNameOld, string& FNameNew)
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif
	rename(path(FNameOld), path(FNameNew));
        return true;
}


bool SafeOpen(fstream& f, string FName, ios::openmode Flags) 
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif

	if (((Flags & ios::in) != 0) && !exists(path(FName))) {
		return false;
	}

	for (uint iTry = 0; iTry < cSafeOpenNumTries; iTry++) {
		try {
#ifdef GOTIT_LINUX                    
			f.open(FName, Flags);
#else                        
			f.open(FName, Flags, _SH_DENYRW);
#endif                        
		}
		catch (fstream::failure e) {
			cerr << "SafeOpen: Exception opening/reading/closing file " << FName << " with error " << e.what() << endl;
			return false;
		}

		if (f.is_open()) {
			return true;
		}

#ifdef GOTIT_LINUX                    
		sleep(cSafeopenWaitTime/1000);
#else
		Sleep(cSafeopenWaitTime);
#endif                
	}
	cerr << "SafeOpen: Failed to open " << FName << endl;
	return false;
}

void CGotitEnv::SaveGroups()
{
	PatGrpMgr.Store();
#ifdef OLD_CODE
	// while we are writing out the groups and their details
	// we also create a vector of the pointers to the groups so that
	// we can sort the vector later
	// we do this at writing because loading does this and a write
	// may not be the last op but part of a continuation
	PatGroups.clear(); // start fresh
	fstream fileOut;
	SafeOpen(fileOut, "D:\\guten\\state\\Groups.bin", ios::binary | ios::out);
	uint RandSoFar = rand();
	fileOut.write((char *)&(RandSoFar), sizeof(RandSoFar));
	uint NumGroups = PatGrpTbl.size();
	fileOut.write((char *)&(NumGroups), sizeof(NumGroups));
	map_name_to_pat_grp::iterator itpg = PatGrpTbl.begin();
	for (; itpg != PatGrpTbl.end(); itpg++) {
		CGroupingRec * patgrp = itpg->second;
		patgrp->Store(fileOut);
		PatGroups.push_back(patgrp);
	}
	//ofstream fileBWCount("WordCount.bin", ios::binary);
	//BWCountStore(BaseWordCount, fileBWCount);
#endif // OLD_CODE
}


void CGotitEnv::LoadGroups()
{
	PatGrpMgr.Load(anc_pat_finder_list);
	//PatGrpMgr.CreateWordDataLinks(MapWordToData);
}

#ifdef OLD_CODE
void UseGroups(	CModNames& ModNames, MapStringToNameItems& NameLists, 
				MapBaseWordToCount& BaseWordCount, vector<pair<string, int> >& WordsInOrder,
				vector<CGroupingRec *>& PatGroups)
{
	sort(PatGroups.begin(), PatGroups.end(), GroupingSort);

	const uint cMaxGroups = 100; // large for this test, should be smaller
	uint MaxGroups = min(PatGroups.size(), cMaxGroups);
	vector<CGroupingRec *> PatGroupsSel(MaxGroups);

	int NumSelGroups = 0;
	for (uint ig = 0; ig < PatGroups.size(); ig++) {
		if ((rand() % 7) >= 0) { // >= instead of == disables selection for testing
			PatGroupsSel[NumSelGroups] = PatGroups[ig];
			NumSelGroups++;
			if (NumSelGroups >= cMaxGroups) {
				break;
			}
		}
	}

	//CreateRFData(PatGroups, NameLists);
	bool bWordsOrdered = false;
	float ScoreBroad, ScoreSel;
	vector<int> SelGroupIndices;
	SGrpSeed DummySeed;
	CreateRFErrExList(BaseWordCount, WordsInOrder, ModNames, PatGroupsSel, false, 
						"", "",	ScoreBroad, ScoreSel, SelGroupIndices, DummySeed, DummySeed);

}
#endif //OLD_CODE


std::ofstream fstrCout;
std::streambuf *backupCout;

//map<string, InitTaskFn> xTaskFnMap;

CGotitEnv::~CGotitEnv()
{
	//map_name_to_pat_grp::iterator ipg;
	//for (ipg = PatGrpTbl.begin(); ipg != PatGrpTbl.end(); ipg++) {
	//	delete ipg->second;
	//}

	for (uint img = 0; img < MergedGroupsList.size(); img++) {
		delete MergedGroupsList[img];
	}
}

bool CGotitEnv::getTaskFn(string FnName, InitTaskFn& Fn)
{
	map<string, InitTaskFn>::iterator ittfm = TaskFnMap.find(FnName);
	if (ittfm != TaskFnMap.end()) {
		Fn = ittfm->second;
		return true;
	}
	return false;	
}

bool CGotitEnv::GetImplemParam(string& Val, string Key)
{
	MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(Key);
	if (itipt == ImplemParamTbl.end()) {
		Val = "";
		return false;
	}
	Val = itipt->second.Val;
	return true;
}

void TestGrpsFile();
void TestRegex();

int lmain() {
	//TestGrpsFile();
	//return 0;

	//TestRegex();
	//return 0;

	std::streambuf *psbuf;
	string CoutFileName = LOGS_COUT + to_string(TimerNow()) + ".txt";

	fstrCout.open(CoutFileName);

	backupCout = std::cout.rdbuf();     // back up cout's streambuf

	psbuf = fstrCout.rdbuf();        // get file's streambuf
	std::cout.rdbuf(psbuf);         // assign streambuf to cout

	CGotitEnv * pEnv = new CGotitEnv;

	pEnv->ParseConfigFile();
	pEnv->RegisterCallableFunctions();

	string MainTaskName;
	if (pEnv->GetImplemParam(MainTaskName, "Implem.Param.Cntrl.MainTask")) {
		pEnv->ExecTaskByName(MainTaskName);
		//InitTaskFn TaskFn;
		//if (pEnv->getTaskFn(MainTaskName, TaskFn)) {
		//	(pEnv->*TaskFn)();
		//}
	}
	
	delete pEnv;
	//ModNamesCntrl.LoadModuleNames("C:\\GotitFilesEx");


	// The following code is only for converting MASC files
	// to SentenceRec s uncomment for a new batch
	// Don't forget to uncomment undef MASC
	//srand(173);


	
#ifdef SUSPEND_CODE
		if (ImplemParamTbl["bRecreateInitialGroups"].Val == "true") {
			pEnv->StartWithGroups();
		}
		else {
			pEnv->LoadGroups();
		}
#endif // SUSPEND_CODE

	//bSkipTheRest = true;
	
	// One day learning should be an iterative process. This loop is an example

	//for (int ii = 0; ii < 1; ii++) {
	//	StartWithGroups(anc_pat_finder_list, BaseWordCount, WordsInOrder, ModNamesCntrl, PatGroups, 
	//					PatGrpTbl, MergedGroupsList);
		//CreateExList(BaseWordCount, ModNames);
		//UseGroups(ModNames, BasicTypeLists, BaseWordCount, PatGroups);
	//}

	//if (!bSkipTheRest) {
	//	LearnWordSwap(anc_pat_finder_list, ImplemParamTbl, BaseWordCount, WordsInOrder, ModNamesCntrl,
	//		PatGroups, PatGrpTbl, MergedGroupsList);
	//}

	// final cleanup
	return 0;

}

