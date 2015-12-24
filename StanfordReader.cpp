// StanfordReader.cpp : Parses the output of Standford CoreNLP							
//

#include "stdafx.h"

#include "MascReader.h"

void ConvertStanfordOutput(	string OutputDir, string FileName, string Ext, 
							bool bAddToDB, 
							map<string, int>& BaseWordCount,
							map<string, int>& DepTypes,
							map<string, vector<NameItem> >& BasicTypesList, // a NameItem is a struct with a Name (string) and a count (int)
							vector<string>& DynamicTypeListsNames)
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif
	vector<SSentenceRec> SentenceList;
	char * FileBuf = FileToBuf(FileName.c_str(), "");
	map<string, map<string, int>> CountTypesMap;
	map<string, int> aCountMap;
	//MapBaseWordToCount DepCount;
	//MapBaseWordToCount NERCount;
	//MapBaseWordToCount POSCount;
	vector<NameItem> aNameItemVector;

	if (bAddToDB) {
		for (uint idtln = 0; idtln < DynamicTypeListsNames.size(); idtln++) {
			string& CountName = DynamicTypeListsNames[idtln];
			CountTypesMap[CountName] = aCountMap;
			map<string, vector<NameItem> >::iterator itbtl = BasicTypesList.find(CountName);
			if (itbtl == BasicTypesList.end()) {
				BasicTypesList[CountName] = aNameItemVector;
			}
			else {
				vector<NameItem>& CountVector = BasicTypesList[CountName];
				for (uint ic = 0; ic < CountVector.size(); ic++) {
					(CountTypesMap[CountName])[CountVector[ic].Name] = CountVector[ic].Count;
				}
			}
		}
	}
	/*
	MapStringToNameItems::iterator itbtl = BasicTypesList.find("depcount");
	if (itbtl == BasicTypesList.end()) {
		BasicTypesList["depcount"] = aNameItemVector;
	}
	else {
		vector<NameItem>& CountVector = BasicTypesList["depcount"];
		for (uint ic = 0; ic < CountVector.size(); ic++) {
			DepCount[CountVector[ic].Name] = CountVector[ic].Count;
		}
	}

	itbtl = BasicTypesList.find("NERCount");
	if (itbtl == BasicTypesList.end()) {
		BasicTypesList["NERCount"] = aNameItemVector;
	}
	else {
		vector<NameItem>& CountVector = BasicTypesList["NERCount"];
		for (uint ic = 0; ic < CountVector.size(); ic++) {
			NERCount[CountVector[ic].Name] = CountVector[ic].Count;
		}
	}

	itbtl = BasicTypesList.find("POSCount");
	if (itbtl == BasicTypesList.end()) {
		BasicTypesList["POSCount"] = aNameItemVector;
	}
	else {
		vector<NameItem>& CountVector = BasicTypesList["POSCount"];
		for (uint ic = 0; ic < CountVector.size(); ic++) {
			POSCount[CountVector[ic].Name] = CountVector[ic].Count;
		}
	}
	*/
	xml_document<> xmlDoc;    // character type defaults to char
	xmlDoc.parse<0>(FileBuf);    // 0 means default parse flags
	xml_node<> *RootNode = xmlDoc.first_node("root");
	xml_node<> *DocNode = RootNode->first_node("document");
	xml_node<> *AllSentencesNode = DocNode->first_node("sentences");
	for (xml_node<> *SentenceNode = AllSentencesNode->first_node("sentence"); SentenceNode; SentenceNode = SentenceNode->next_sibling()) {
		bool bSentenceCutoff = false;
		xml_node<> *AllTokensNode = SentenceNode->first_node("tokens");
		SentenceList.push_back(SSentenceRec());
		for (xml_node<> *TokenNode = AllTokensNode->first_node("token"); TokenNode; TokenNode = TokenNode->next_sibling()) {
			WordRec wrec;
			for (xml_node<> *node = TokenNode->first_node(); node; node = node->next_sibling()) {
				if (strcmp(node->name(), "word") == 0) {

					//xml_node<> *WordNode = node->first_node("word");
					//xml_attribute<> *attr = WordNode->first_attribute();
					string W = node->value();
					if (W.size() > 0) {
						bool bAllButFirstLower = true;
						for (uint iChar = 0; iChar < W.size(); iChar++) {
							if (iChar == 0 && !isupper(W[0])) {
								bAllButFirstLower = false;
								break;
							}
							if (iChar > 0 && isupper(W[iChar])) {
								bAllButFirstLower = false;
								break;
							}
						}
						if (bAllButFirstLower) {
							wrec.Word = tolower(W[0]);
							for (uint iChar = 1; iChar < W.size(); iChar++) {
								wrec.Word += W[iChar];
							}
							wrec.bCap = true;
						}
						else {
							wrec.Word = W;
						}
						if (bAddToDB) {
							MapBaseWordToCount::iterator fw = BaseWordCount.find(wrec.Word);
							if (fw == BaseWordCount.end()) {
								BaseWordCount[wrec.Word] = 1;
							}
							else {
								fw->second++;
							}
						}
					}
					SentenceList.back().Sentence += string(node->value()) + " ";
				} // end if node is word
				else if (strcmp(node->name(), "POS") == 0) {
					wrec.POS = node->value();
					if (bAddToDB) {
						MapBaseWordToCount::iterator itpc 
								= (CountTypesMap["POSCount"]).find(wrec.POS);
						if (itpc == (CountTypesMap["POSCount"]).end()) {
							(CountTypesMap["POSCount"])[wrec.POS] = 1;
						}
						else {
							itpc->second++;
						}
					}
				}
				else if (strcmp(node->name(), "lemma") == 0) {
					wrec.WordCore = node->value();
				}
				else if (strcmp(node->name(), "NER") == 0) {
					wrec.NER = node->value();
					if (bAddToDB) {
						MapBaseWordToCount::iterator itnc 
								= (CountTypesMap["NERCount"]).find(wrec.NER);
						if (itnc == (CountTypesMap["NERCount"]).end()) {
							(CountTypesMap["NERCount"])[wrec.NER] = 1;
						}
						else {
							itnc->second++;
						}
					}
				}
			} // end loop over nodes inside token
			//wrec.Word = WordNode->value();
			//wrec.WordBase = WordNode->value(); // Remove soon or replace with lemma
			SentenceList.back().OneWordRec.push_back(wrec);
			if (!bSentenceCutoff && SentenceList.back().OneWordRec.size() > 250) {
				cerr << "Sentence unacceptably long: " << SentenceList.back().Sentence << ". No Deps recorded " << endl;
				bSentenceCutoff = true;
			}
		} // end loop over token
		if (bSentenceCutoff) {
			continue;
		}
		xml_node<> *AllDepsNode = SentenceNode->first_node("dependencies");
		for (xml_node<> *DepNode = AllDepsNode->first_node("dep"); DepNode; DepNode = DepNode->next_sibling()) {
			DepRec drec;
			xml_attribute<> *attr = DepNode->first_attribute("type");
			string DepName = attr->value();
			int iDep;
			map<string, int>::iterator itdm = DepTypes.find(DepName);
			if (itdm == DepTypes.end()) {
				if (!bAddToDB) {
					cerr << "Error! New dep type " << DepName << " found yet bAddToDB is false\n";
					iDep = 0; // hopefully innocuous value
				}
				else {
					iDep = DepTypes.size();
					DepTypes[DepName] = iDep;
					(CountTypesMap["depcount"])[DepName] = 1;
				}

			}
			else {
				iDep = itdm->second;
				if (bAddToDB) {
					(CountTypesMap["depcount"])[DepName]++;
				}
			}
			drec.iDep = (uchar)iDep;
			xml_node<> *GovNode = DepNode->first_node("governor");
			attr = GovNode->first_attribute("idx");
			drec.Gov = (uchar)atoi(attr->value()) - 1;
			xml_node<> *EntNode = DepNode->first_node("dependent");
			attr = EntNode->first_attribute("idx");
			drec.Dep = (uchar)atoi(attr->value()) - 1;
			SentenceList.back().Deps.push_back(drec);

		}
	} // end loop over sentences
	
	vector<CorefRec> CorefList;
	int LastGov = -1;
	xml_node<> *TopCorefNode = DocNode->first_node("coreference");
	//xml_node<> *AllCorefNodes = TopCorefNode->first_node("coreference");
	for (xml_node<> *CorefNode = TopCorefNode->first_node("coreference"); CorefNode; CorefNode = CorefNode->next_sibling()) {
		for (xml_node<> *MentionNode = CorefNode->first_node("mention"); MentionNode; MentionNode = MentionNode->next_sibling()) {
			CorefRec crec;
			xml_attribute<> *attr = MentionNode->first_attribute("representative");
			if (attr) {
				LastGov = CorefList.size();
			}
			for (xml_node<> *node = MentionNode->first_node(); node; node = node->next_sibling()) {
				if (strcmp(node->name(), "sentence") == 0) {
					crec.SentenceID = atoi(node->value()) - 1;
				}
				else if (strcmp(node->name(), "start") == 0) {
					crec.StartWordID = atoi(node->value()) - 1;
				}
				else if (strcmp(node->name(), "end") == 0) {
					crec.EndWordID = atoi(node->value()) - 1;
				}
				else if (strcmp(node->name(), "head") == 0) {
					crec.HeadWordId = atoi(node->value()) - 1;
				}
			}
			crec.GovID = LastGov;
			CorefList.push_back(crec);
		}
	}

	{
		path OrigPath = FileName;
#ifndef GOTIT_LINUX
		string BaseName = OrigPath.basename().c_str();
#else
        string BaseName = basename(OrigPath);
#endif
		ofstream ModFile(OutputDir+"/"+BaseName+Ext, ios::binary);
		uint NumRecs = SentenceList.size();
		ModFile.write((char *)&(NumRecs), sizeof(NumRecs));
		for (uint im = 0; im < NumRecs; im++) {
			SentenceList[im].Store(ModFile);
		}
		uint NumCorefMentions = CorefList.size();
		ModFile.write((char *)&(NumCorefMentions), sizeof(NumCorefMentions));
		for (auto crec : CorefList) {
			crec.Store(ModFile);
		}
		ModFile.close();
	}
	delete[] FileBuf;
	if (bAddToDB) {
		for (uint idtln = 0; idtln < DynamicTypeListsNames.size(); idtln++) {
			string& CountName = DynamicTypeListsNames[idtln];
			vector<NameItem>& CountVector = BasicTypesList[CountName];
			CountVector.clear();
			MapBaseWordToCount::iterator itmc = (CountTypesMap[CountName]).begin();
			for (; itmc != (CountTypesMap[CountName]).end(); itmc++) {
				CountVector.push_back(NameItem(itmc->first, itmc->second));
			}
			sort(CountVector.begin(), CountVector.end(), gtNameItems);

		}
	}
	/*
	{
		vector<NameItem>& CountVector = BasicTypesList["depcount"];
		CountVector.clear();
		MapBaseWordToCount::iterator itmc = DepCount.begin();
		for (; itmc != DepCount.end(); itmc++) {
			CountVector.push_back(NameItem(itmc->first, itmc->second));
		}
	}
	{
		vector<NameItem>& CountVector = BasicTypesList["NERCount"];
		CountVector.clear();
		MapBaseWordToCount::iterator itmc = NERCount.begin();
		for (; itmc != NERCount.end(); itmc++) {
			CountVector.push_back(NameItem(itmc->first, itmc->second));
		}
	}
	{
		vector<NameItem>& CountVector = BasicTypesList["POSCount"];
		CountVector.clear();
		MapBaseWordToCount::iterator itmc = POSCount.begin();
		for (; itmc != POSCount.end(); itmc++) {
			CountVector.push_back(NameItem(itmc->first, itmc->second));
		}
	}
	*/
}

void StoreDepTypes(map<string, int>& DepTypes, fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	uint NumEls = DepTypes.size();
	fout.write((char *)&NumEls, sizeof(NumEls));
	map<string, int>::iterator itdt = DepTypes.begin();
	for (; itdt != DepTypes.end(); itdt++) {
		StringStore(string(itdt->first), fout);
		fout.write((char *)&(itdt->second), sizeof(itdt->second));
	}

}

void LoadDepTypes(map<string, int>& DepTypes, fstream& f)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	uint NumEls;
	fin.read((char *)&NumEls, sizeof(NumEls));
	for (uint idt = 0; idt < NumEls; idt++) {
		string w;
		int n;
		StringLoad(w, fin);
		fin.read((char *)&n, sizeof(n));
		DepTypes[w] = n;

	}

}