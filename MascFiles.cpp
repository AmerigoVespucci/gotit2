// MascFiles.cpp : INouts the various masc files								
//

#include "stdafx.h"

#include "MascReader.h"

char * FileToBuf(const char * ModName, const char * ExtName, const char * PathName) {
	FILE * fptr = NULL;
	string sfname = PathName; // = SRC_PATH;
	sfname += ModName;
	sfname += ExtName;

#ifdef GOTIT_LINUX        
	fptr = fopen(sfname.c_str(), "rb");
#else        
	fopen_s(&fptr, sfname.c_str(), "rb");
#endif        
	if (fptr == NULL) {
		return NULL;
	}

	long FileLength;

	fseek(fptr, 0, SEEK_END);
	FileLength = ftell(fptr);
	fseek(fptr, 0, SEEK_SET); // utf encoding starts with 3 funny bytes

	char * FileBuf = new char[FileLength + 2];
	if (fread(FileBuf, 1, FileLength, fptr) != FileLength) {
		cout << "Warning: File " << ModName << " could not be read\n";
	}
	FileBuf[FileLength] = ' ';
	FileBuf[FileLength + 1] = '\0';

	fclose(fptr);

	return FileBuf;
}

bool InputFiles(const char * ModName, MapIDToTextRegion& RegionMap, MapIDToPOSRec& POSMap, 
				MapIDToSentenceRec& SentenceMap)
{
	char * SegFileBuf = FileToBuf(ModName, "-seg.xml");
	char * TextBuf = FileToBuf(ModName, ".txt");
	char * PennFileBuf = FileToBuf(ModName, PENN_SUFFIX);
	char * SFileBuf = FileToBuf(ModName, "-s.xml");
	string sText;
	for (uchar * p = (uchar *)TextBuf; *p != '\0'; p++) {
		uchar c = *p;
		if ((c >> LOW_TEST) == LOW_ASCII) {
			sText += *p;
		}
		else {
			sText += ' ';
			while (((*(p + 1)) >> UTF_TEST) == UTF_CONT) {
				p++;
			}
			//p--;
		}
	}
	xml_document<> xmlRegions;    // character type defaults to char
	xmlRegions.parse<0>(SegFileBuf);    // 0 means default parse flags
	xml_node<> *TopNode = xmlRegions.first_node();
	//map<string, STextRegion> RegionMap;
	for (xml_node<> *node = TopNode->first_node("region"); node; node = node->next_sibling()) {
		STextRegion reg;
		for (xml_attribute<> *attr = node->first_attribute();
			attr; attr = attr->next_attribute()) {
			if (strcmp(attr->name(), "xml:id") == 0) {
				reg.Name = attr->value();
			}
			if (strcmp(attr->name(), "anchors") == 0) {
				string val = attr->value();
				size_t SpacePos = val.find(" ");
				reg.TextIDStart = atoi(val.substr(0, SpacePos).c_str());
				reg.TextIDEnd = atoi(val.substr(SpacePos + 1).c_str());
				reg.Word = sText.substr(reg.TextIDStart, reg.TextIDEnd - reg.TextIDStart);
			}
		}
		RegionMap[reg.Name] = reg;
	}
	delete[] SegFileBuf;

	xml_document<> xmlPenn;    // character type defaults to char
	xmlPenn.parse<0>(PennFileBuf);    // 0 means default parse flags
	TopNode = xmlPenn.first_node();
	//map<string, SPOSRec> POSMap;
	SPOSRec rec;
	for (xml_node<> *node = TopNode->first_node(); node; node = node->next_sibling()) {
		if (strcmp(node->name(), "node") == 0) {
			for (xml_attribute<> *attr = node->first_attribute();
				attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "xml:id") == 0) {
					rec.Name = attr->value();
				}
			}
			xml_node<> *ref = node->first_node("link");
			if (ref != NULL) {
				xml_attribute<> * refa = ref->first_attribute();
				if (refa != NULL) {
					rec.TextRef = refa->value();
				}
			}
		}
		else if (strcmp(node->name(), "a") == 0) {
			for (xml_attribute<> *attr = node->first_attribute();
				attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "ref") == 0) {
					if (rec.Name.compare(attr->value()) != 0) {
						cerr << "mismatched node and a\n";
					}
				}
			}
			xml_node<> *fs = node->first_node("fs");
			if (fs != NULL) {
				for (xml_node<> *fnode = fs->first_node("f"); fnode; fnode = fnode->next_sibling()) {
					string sfname;
					string sfval;
					for (xml_attribute<> *attr = fnode->first_attribute();
						attr; attr = attr->next_attribute()) {
						if (strcmp(attr->name(), "name") == 0) {
							sfname = attr->value();
						}
						else if (strcmp(attr->name(), "value") == 0) {
							sfval = attr->value();
						}
					} // end loop over attributes
					if (sfname.compare("msd") == 0) {
						rec.POS = sfval;
					}
					else if (sfname.compare("string") == 0) {
						rec.Word = sfval;
					}
					else if (sfname.compare("base") == 0) {
						rec.WordBase = sfval;
					}
				} // end koop over f objects
				MapIDToTextRegion::iterator it = RegionMap.find(rec.TextRef);
				if (it != RegionMap.end()) {
					if (it->second.Word.compare(rec.Word) == 0) {
						it->second.POSRef = rec.Name;
					}
				}
				STextRegion reg = RegionMap[rec.TextRef];
				POSMap[rec.Name] = rec;
			}
		}
	}
	delete[] PennFileBuf;

	xml_document<> xmlSs;    // character type defaults to char
	xmlSs.parse<0>(SFileBuf);    // 0 means default parse flags
	TopNode = xmlSs.first_node();
	//map<string, STextRegion> RegionMap;
	map<string, string> MapRegionIdToAnchors;
	map<string, string> MapNodeIdToRef;

	for (xml_node<> *node = TopNode->first_node(); node; node = node->next_sibling()) {
		// assume each record has a region node (not useful) and an a
		if (strcmp(node->name(), "region") == 0) {
			string sId, sAnchors;
			for (xml_attribute<> *attr = node->first_attribute();
				attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "xml:id") == 0) {
					//srec.Name = attr->value();
					sId = attr->value();
				}
				if (strcmp(attr->name(), "anchors") == 0) {
					sAnchors = attr->value();
					//string val = attr->value();
					//size_t SpacePos = val.find(" ");
					//srec.TextIDStart = atoi(val.substr(0, SpacePos).c_str());
					//srec.TextIDEnd = atoi(val.substr(SpacePos + 1).c_str());
					//srec.Sentence = sText.substr(srec.TextIDStart, srec.TextIDEnd - srec.TextIDStart);
				}
			}
			MapRegionIdToAnchors[sId] = sAnchors;
		} // end if node is region
		else if (strcmp(node->name(), "node") == 0) {
			string sId, sRef;
			for (xml_attribute<> *attr = node->first_attribute();
							attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "xml:id") == 0) {
					sId = attr->value();
				}
			}
			xml_node<> *ln = node->first_node("link");
			if (ln != NULL) {
				for (xml_attribute<> *attr = ln->first_attribute();
					attr; attr = attr->next_attribute()) {
					if (strcmp(attr->name(), "targets") == 0) {
						sRef = attr->value();
					}
				}
			}
			MapNodeIdToRef[sId] = sRef;
		}
		else if (strcmp(node->name(), "a") == 0) {
			SSentenceRec srec;
			for (xml_attribute<> *attr = node->first_attribute();
				attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "xml:id") == 0) {
					srec.Name = attr->value();
				}
				else if (strcmp(attr->name(), "label") == 0) {
					srec.Label = attr->value();
				}
				else if (strcmp(attr->name(), "ref") == 0) {
					string val = attr->value();
					string sNode = MapNodeIdToRef[val];
					string sAnchors = MapRegionIdToAnchors[sNode];
					if (!sAnchors.empty())	{
						size_t SpacePos = sAnchors.find(" ");
						srec.TextIDStart = atoi(sAnchors.substr(0, SpacePos).c_str());
						srec.TextIDEnd = atoi(sAnchors.substr(SpacePos + 1).c_str());
						srec.Sentence = sText.substr(srec.TextIDStart, srec.TextIDEnd - srec.TextIDStart);
					}
					else {
						cerr << "mismatched node and a\n";
					}
				} // end if attr is ref
			}
			xml_node<> *fs = node->first_node("fs");
			if (fs != NULL) {
				xml_node<> *f = fs->first_node("f");
				if (f != NULL) {
					for (xml_attribute<> *attr = f->first_attribute();
										attr; attr = attr->next_attribute()) {
						if (strcmp(attr->name(), "value") == 0) {
							string val = attr->value();
							size_t SPos = val.find("s");
							srec.ParaNum = atoi(val.substr(1, SPos).c_str());
							srec.SentInParaNum = atoi(val.substr(SPos + 1).c_str());
						}
					}
				}
			}
			if (srec.Label.compare("s") == 0) {
				SentenceMap[srec.Name] = srec;
			}
		} // node of type a
	} // end search over nodes
	delete[] SFileBuf;
	delete[] TextBuf;

	return true;
}

bool CreateSentenceList(MapModuleNameToIndex& ModsToSentenceIndex,
	vector<vector<SSentenceRec> >& SentenceRecList,
	vector<string>& ModNames)
{
	vector<SSentenceRec> OneEmptySentenceList;

	for (uint im = 0; im < ModNames.size(); im++) {
		SentenceRecList.push_back(OneEmptySentenceList);
		ModsToSentenceIndex[ModNames[im]] = SentenceRecList.size() - 1;
		//InputFiles(ModNames[im].c_str(), RegionMap, POSMap, SentenceMap);
		vector<SSentenceRec>& SentenceList = SentenceRecList.back();
	}

	return true;
}

void WordRec::Store(ofstream& fout)
{
	//StringStore(RegionName, fout);
	StringStore(Word, fout);
	StringStore(WordCore, fout);
	StringStore(POS, fout);
	StringStore(NER, fout);
	fout.write((char *)&(bCap), sizeof(bCap));

}

void WordRec::Load(ifstream& fin)
{
	//StringStore(RegionName, fout);
	StringLoad(Word, fin);
	StringLoad(WordCore, fin);
	StringLoad(POS, fin);
	StringLoad(NER, fin);
	fin.read((char *)&(bCap), sizeof(bCap));

}

void DepRec::Store(ofstream& fout)
{
	uchar StWord[sizeof(uint)];
	StWord[0] = iDep;
	StWord[1] = Gov;
	StWord[2] = Dep;
	fout.write((char *)StWord, sizeof(uint));
}

void DepRec::Load(ifstream& fin)
{
	uchar StWord[sizeof(uint)];
	fin.read((char *)StWord, sizeof(uint));
	iDep = StWord[0];
	Gov = StWord[1];
	Dep = StWord[2];
}

void CorefRec::Store(ofstream& fout)
{
	int CArr[5];
	CArr[0] = SentenceID;
	CArr[1] = StartWordID;
	CArr[2] = EndWordID;
	CArr[3] = HeadWordId;
	CArr[4] = GovID;
	fout.write((char *)CArr, sizeof(uint) * 5);
	
}

void CorefRec::Load(ifstream& fin)
{
	int CArr[5];
	fin.read((char *)CArr, sizeof(uint) * 5);
	SentenceID = CArr[0];
	StartWordID = CArr[1];
	EndWordID = CArr[2];
	HeadWordId = CArr[3];
	GovID = CArr[4];
}

void SSentenceRec::Store(ofstream& fout)
{
	StringStore(Sentence, fout);
	StringStore(Sentence, fout);
	fout.write((char *)&(TextIDStart), sizeof(TextIDStart));
	fout.write((char *)&(TextIDEnd), sizeof(TextIDEnd));
	fout.write((char *)&(ParaNum), sizeof(ParaNum));
	fout.write((char *)&(SentInParaNum), sizeof(SentInParaNum));
	StringStore(Label, fout);
	uint NumRecs = OneWordRec.size();
	fout.write((char *)&(NumRecs), sizeof(NumRecs));
	for (uint ir = 0; ir < NumRecs; ir++) {
		OneWordRec[ir].Store(fout);
	}
	uint NumDeps = Deps.size();
	fout.write((char *)&(NumDeps), sizeof(NumDeps));
	for (uint id = 0; id < NumDeps; id++) {
		Deps[id].Store(fout);
	}
}

void SSentenceRec::Load(ifstream& fin)
{
	StringLoad(Sentence, fin);
	StringLoad(Sentence, fin);
	fin.read((char *)&(TextIDStart), sizeof(TextIDStart));
	fin.read((char *)&(TextIDEnd), sizeof(TextIDEnd));
	fin.read((char *)&(ParaNum), sizeof(ParaNum));
	fin.read((char *)&(SentInParaNum), sizeof(SentInParaNum));
	StringLoad(Label, fin);
	uint NumRecs ;
	fin.read((char *)&(NumRecs), sizeof(NumRecs));
	OneWordRec.resize(NumRecs);
	for (uint ir = 0; ir < NumRecs; ir++) {
		OneWordRec[ir].Load(fin);
	}
	uint NumDeps;
	fin.read((char *)&(NumDeps), sizeof(NumDeps));
	Deps.resize(NumDeps);
	for (uint id = 0; id < NumDeps; id++) {
		Deps[id].Load(fin);
	}
}
// this is a very slow function. Please replace this by storing the post parsed data in a separate file

bool CreateSentenceListOneMod(string ModName,
							vector<SSentenceRec>& SentenceListRet,
							MapBaseWordToCount& BaseWordCount,
							vector<string>& OnlyIfWordsPresent,
							int ModNum)
{
	MapIDToTextRegion RegionMap;
	MapIDToPOSRec POSMap;
	MapIDToSentenceRec SentenceMap;
	vector<SSentenceRec> SentenceList;
	InputFiles(ModName.c_str(), RegionMap, POSMap, SentenceMap);

	vector<STextRegion*> RegionList;
	MapIDToTextRegion::iterator itR;
	for (itR = RegionMap.begin(); itR != RegionMap.end(); itR++) {
		RegionList.push_back(&(itR->second));
	}
	sort(RegionList.begin(), RegionList.end(), STextRegion::lt);
	MapIDToSentenceRec::iterator itS;
	for (itS = SentenceMap.begin(); itS != SentenceMap.end(); itS++) {
		SentenceList.push_back((itS->second));
	}
	sort(SentenceList.begin(), SentenceList.end(), SSentenceRec::lt);
	uint ir = 0;
	uint NumIfWords = OnlyIfWordsPresent.size();
	//int cb = 0;
	for (uint is = 0; is < SentenceList.size(); is++) {
		bool bOnlyIfWordFound = (NumIfWords == 0); // false for now if list not empty
		uint isLast = SentenceListRet.size();
		SentenceListRet.push_back(SentenceList[is]);
		bool irlimit = false;
		while (SentenceList[is].TextIDEnd >= RegionList[ir]->TextIDEnd) {
			if (SentenceList[is].TextIDStart <= RegionList[ir]->TextIDStart) {
				WordRec wrec;
				wrec.RegionName = RegionList[ir]->Name;
				string W = RegionList[ir]->Word;
				if (!bOnlyIfWordFound) {
					for (uint iif = 0; iif < NumIfWords; iif++) {
						if (W == OnlyIfWordsPresent[iif]) {
							bOnlyIfWordFound = true;
							break;
						}
					}
				}
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
					MapBaseWordToCount::iterator fw = BaseWordCount.find(wrec.Word);
					if (fw == BaseWordCount.end()) {
						BaseWordCount[wrec.Word] = 1;
					}
					else {
						fw->second++;
					}
				}
				SPOSRec& rec = POSMap[RegionList[ir]->POSRef];
				if (rec.Word == RegionList[ir]->Word) {
					wrec.POS = rec.POS;
					wrec.WordCore = rec.WordBase;
					//string t = rec.WordBase;
					//std::transform(t.begin(), t.end(), t.begin(), ::tolower);
					//if (t == "before" && wrec.POS != "IN") {
					//	cb++;
					//}
					SentenceList[is].OneWordRec.push_back(wrec);
					SentenceListRet[isLast].OneWordRec.push_back(wrec);
					
				}
			}
			ir++;
			if (ir >= RegionList.size()) {
				irlimit = true;
				break;
			}
		}
		if (!bOnlyIfWordFound) {
			SentenceListRet.pop_back();
		}
		if (irlimit) {
			break;
		}
	}

	//bool bWordsOrdered = false; // make this a parameter
	//if (bWordsOrdered) { 
	//	// this function adds to BaseWordCount so it must redo WordsInOrder
	//	WordsInOrder.clear();
	//	MapBaseWordToCount::iterator it = BaseWordCount.begin();
	//	for (; it != BaseWordCount.end(); it++) {
	//		WordsInOrder.push_back(make_pair(it->first, it->second));
	//	}
	//	sort(WordsInOrder.begin(), WordsInOrder.end(), gtNameItems);
	//}

	if (ModNum != -1) {
		string FileName = "C:\\GotitFiles\\Mod";
		FileName += to_string(ModNum);
		ofstream ModFile(FileName, ios::binary);
		uint NumRecs = SentenceList.size();
		ModFile.write((char *)&(NumRecs), sizeof(NumRecs));
		for (uint im = 0; im < NumRecs; im++) {
			SentenceList[im].Store(ModFile);
		}

	}



	return true;
}

void DoNLPOnFile(string& IFileName, string& OFileName)
{
    
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif

#ifdef GOTIT_LINUX
    cerr << "NLP not implemented on linux yet!\n";
#else            
	int err = 0;
	path StanfordNLPPath = "C:\\Apps\\StanfordNLP";
	current_path(StanfordNLPPath);
	path RootPath = "D:\\guten\\ProcessedText";
	string CoreCmdLine = "java -cp \"*\" -Xmx1g edu.stanford.nlp.pipeline.StanfordCoreNLP -annotators tokenize,ssplit,pos,lemma,ner,parse -outputDirectory D:\\guten\\temp -file ";
	string CmdLine = CoreCmdLine + IFileName;
	err = system(CmdLine.c_str());
#endif
}