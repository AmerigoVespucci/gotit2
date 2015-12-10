// CaffeFn.cpp : 
// Takes a GenData proto file and produces all the data needed to rnu a GenSeed Caffe NN generation
//


 
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "stdafx.h"

#include "MascReader.h"
#include "H5Cpp.h"

#include "../../CaffeR/GenSeed.pb.h"
#include "GenData.pb.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif


#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif

enum FieldNameID {
	fniWord,
	fniWordCore,
	fniPOS,
};

struct SFieldNameToID {
	string name;
	int id;
};

SFieldNameToID FieldNameToIDTbl[] = {	{"Word", fniWord}, 
										{"WordCore", fniWordCore},  
										{"POS", fniPOS}   
									};

int FieldNamesTblSize = sizeof(FieldNameToIDTbl) / sizeof(FieldNameToIDTbl[0]);

string GetRecFieldByIdx(WordRec& rec, int FieldID, bool& bRetValid)
{
	bRetValid = true;
	switch(FieldID) {
		case fniWord: {
			string w = rec.Word;
			bool bWordOK = true;
			for (int iiw = 0; iiw < w.size(); iiw++) {
				auto c = w[iiw];
				if (!isalpha(c)) {
					bWordOK = false;
					break;
				}
				if (isupper(c)) {
					w[iiw] = tolower(c);
				}
			}
			if (!bWordOK) {
				bRetValid = false;
				break;
			}
			return w;
		}
		case fniWordCore: 
			return rec.WordCore;
		case fniPOS:
			return rec.POS;
		default:
			bRetValid = false;
			return string();
	}
}

int GetIdxFromFieldName (const string& name) {
	for (int ifn = 0; ifn < FieldNamesTblSize; ifn++) {
		if (name == FieldNameToIDTbl[ifn].name) {
			return FieldNameToIDTbl[ifn].id;
		}
	}
	return -1;
}

void CGotitEnv::CaffeFnInit() 
{
	CaffeFnHandle = NULL;
	CaffeFnOutHandle = NULL;
	CaffeGenData* gen_data = new CaffeGenData;
	
	string sModelProtoName;
	if (!GetImplemParam(sModelProtoName, "Implem.Param.FnParam.CaffeFn.ModelProtoName")) {
		cerr << "CaffeFn cannot be called without setting Implem.Param.FnParam.CaffeFn.ModelProtoName \n";
		return;
	}
	ifstream proto_ifs(sModelProtoName);
	if (proto_ifs.is_open()) {
		google::protobuf::io::IstreamInputStream* proto_input 
			= new google::protobuf::io::IstreamInputStream(&proto_ifs);
		google::protobuf::TextFormat::Parse(proto_input, gen_data);
		delete proto_input;
	}

	CaffeGenSeed* gen_seed_config = new CaffeGenSeed;

	const string& CoreDir = gen_data->files_core_dir();
	string H5TrainListFileName = CoreDir + "data/train_list.txt";
	string H5TestListFileName = CoreDir + "data/test_list.txt";
	
	ofstream test_list(H5TestListFileName, ofstream::trunc);
	ofstream train_list(H5TrainListFileName, ofstream::trunc);

	gen_seed_config->set_num_test_cases(0);
	
	CaffeFnHandle = gen_data;
	CaffeFnOutHandle = gen_seed_config;
}
void CGotitEnv::CaffeFnComplete()
{
	
	if (!CaffeFnHandle || !CaffeFnOutHandle) {
		cerr << "CaffeFnComplete can only work if preceeded by a succesful call to CaffeFnInit";
		return;
		
	}
	CaffeGenData* gen_data = (CaffeGenData *)CaffeFnHandle;
	CaffeGenSeed* gen_seed_config  = (CaffeGenSeed*)CaffeFnOutHandle;

	const string& CoreDir = gen_data->files_core_dir();
	string H5TrainListFileName = CoreDir + "data/train_list.txt";
	string H5TestListFileName = CoreDir + "data/test_list.txt";
	
	ofstream config_ofs(CoreDir + gen_data->config_file_name());
	google::protobuf::io::OstreamOutputStream* config_output 
		= new google::protobuf::io::OstreamOutputStream(&config_ofs);
	//ofstream f_config(ConfigFileName); // I think this one is wrong
	if (config_ofs.is_open()) {
		//CaffeGenSeed config;
		gen_seed_config->set_test_list_file_name(H5TestListFileName);
		gen_seed_config->set_train_list_file_name(H5TrainListFileName);
		gen_seed_config->set_model_file_name(CoreDir + gen_data->model_file_name());
		gen_seed_config->set_proto_file_name(CoreDir + gen_data->proto_file_name());
		gen_seed_config->set_num_accuracy_candidates(gen_data->num_accuracy_candidates());
		google::protobuf::TextFormat::Print(*gen_seed_config, config_output);
		delete config_output;
	}
	
	
	delete gen_data;
	CaffeFnHandle = NULL;
	delete gen_seed_config ;
	CaffeFnOutHandle = NULL;
}

void CGotitEnv::CaffeFn()
{

	if (!CaffeFnHandle || !CaffeFnOutHandle) {
		cerr << "CaffeFn can only work if preceeded by a succesful call to CaffeFnInit";
		return;
		
	}
	CaffeGenData* gen_data = (CaffeGenData *)CaffeFnHandle;
	CaffeGenSeed* gen_seed_config  = (CaffeGenSeed*)CaffeFnOutHandle;
	const string& CoreDir = gen_data->files_core_dir();
	string sConfigLoopNum;
	if (!GetImplemParam(sConfigLoopNum, "Task.Param.DoCaffeFn.Loop0")) {
		cerr << "CaffeFn assumes it is called as part of a loop with loop parameter Task.Param.DoCaffeFn.Loop0 \n";
		return;
	}
	string H5TrainListFileName = CoreDir + "data/train_list.txt";
	string H5TestListFileName = CoreDir + "data/test_list.txt";
	string H5TrainFileName = CoreDir + "data/train" + sConfigLoopNum + ".h5";
	string H5TestFileName = CoreDir + "data/test" + sConfigLoopNum + ".h5";
	string WordListFileName = CoreDir + "data/WordList.txt";
	string WordVecFileName =	gen_data->glove_vec_file_name() ;
	
//	gen_data.set_name("WordToPos");
//	gen_data.set_iterate_type(CaffeGenData_IterateType_ITERATE_WORD);
//	gen_data.set_data_src(CaffeGenData_DataSrc_DATA_SRC_BOOKS);
//	CaffeGenData::DataField * p_field_1 = gen_data.add_data_fields();
//	p_field_1->set_field_name("Word");
//	p_field_1->set_var_name("Word1");
//	{
//		ofstream proto_ofs("/home/abba/NetBeansProjects/gotit2/CaffeSrc/NetGen/WordToPos/WordToPos.prototxt");
//		if (proto_ofs.is_open()) {
//			google::protobuf::io::OstreamOutputStream* proto_output 
//				= new google::protobuf::io::OstreamOutputStream(&proto_ofs);
//			google::protobuf::TextFormat::Print(gen_data, proto_output);
//			delete proto_output;
//		}
//	}
	
	

	const bool cbDoubleWordVecs = true; // false for autoencode thin vector
	const int cNumValsPerVecSrc = 50; // 10 - size of thin vector
	int NumValsPerVec = cNumValsPerVecSrc * (cbDoubleWordVecs ? 2 : 1);

	
	const int cNumWords = 2047;
	const int cNOfNGrams = 5; // the value of N in NGram
	const int cMinRealLength = (cNOfNGrams / 2) + 1; // 3, can't be less
	const int cCenterGram = cNOfNGrams / 2; // 2. center of 5-gram= in 0 based index
	const int cMaxGramsPerWord = 2000;

	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	int NumLabelsPerRec = 1;
	int NumItemsPerRec = cNumWords + 1;

	int NumRecsFound = 0;
	// tuple: rand num for sort, vector of indexes of words, bool if valid, vector of pos indices. 
	vector<tuple<int, vector<int>, bool, vector<int> > > GramRecs; 
	int NumWords = 0;
	map<string, int> WordMap;
	vector<string> RevWordMap;
//	vector<int> GramsFound(cNumWords, 0); 
	vector<vector<float> > WordsVecs(cNumWords+1); // indexed by the same as WordMap

	map<string, int> WordMapClean;
	vector<string> RevWordMapClean;
	vector<vector<float> > WordsVecsClean; 
	
	
	for (int iw=0; iw<cNumWords* 100; iw++) { // factor 100, just to make sure we have enough
		string w = WordsInOrder[iw].Name;
		bool bGood = true;
		for (auto c : w) {
			//if (!isalpha(c)) {
			if (c < 'a' || c > 'z') {
				bGood = false;
				break;
			}
		}
		if (bGood) {
			auto itWordMap = WordMap.find(w);
			if (itWordMap != WordMap.end()) {
				continue;
			}
			int MapSize = WordMap.size(); // saved to avoid ambiguity on next line
			WordMap[w] = MapSize;
			RevWordMap.push_back(w);
			if (MapSize >= cNumWords-1) {
				break;
			}
		}
	}
	{
		int MapSize = WordMap.size(); // saved to avoid ambiguity on next line
		WordMap["<na>"] = MapSize;
		RevWordMap.push_back("<na>");
	}
	
	vector<NameItem>& PosCount = BasicTypeLists["POSCount"];
	map<string, int> PosMap;
	int ini = -1;
	for (NameItem ni : PosCount) {
		ini++;
		PosMap[ni.Name] = ini;
	}
	PosMap["<na>"] = ini;



	const int cVocabLimit = 400000;
	{
		ifstream VecFile(WordVecFileName);
		if (VecFile.is_open()) {
			string ln;
			//VecFile >> NumValsPerVec;
			//while (!VecFile.eof()) {
			for (int ic = 0; ic < cVocabLimit; ic++) {
				getline(VecFile, ln, ' ');
				string w;
				w = ln;
				if (w.size() == 0) {
					break;
				}
				int iw = cNumWords; // default to <na>
				bool bAddVec = false;
					auto itWordMap = WordMap.find(w);
					if (itWordMap != WordMap.end()) {
						bAddVec = true;
						iw = itWordMap->second;
				}
				vector<float>& OneVec = WordsVecs[iw];
				for (int iwv = 0; iwv < cNumValsPerVecSrc; iwv++) {
					if (iwv == cNumValsPerVecSrc - 1) {
						getline(VecFile, ln);
					}
					else {
						getline(VecFile, ln, ' ');
					}
					float wv;
					wv = stof(ln);
					if (bAddVec) {
						if (cbDoubleWordVecs) {
							OneVec.push_back((wv < -0.1f) ? 1.0f : 0.0f);
							OneVec.push_back((wv > 0.1f) ? 1.0f : 0.0f);
						}
						else {
							OneVec.push_back(wv);
						}
					}
				}
			}
		}
		for (int iwv = 0; iwv < WordsVecs.size(); iwv++) {
			vector<float>& OneVec = WordsVecs[iwv];
			if (OneVec.size() == NumValsPerVec) {
				WordMapClean[RevWordMap[iwv]] = WordsVecsClean.size();
				WordsVecsClean.push_back(OneVec);
				RevWordMapClean.push_back(RevWordMap[iwv]);
			}
			else {
				if (RevWordMap[iwv] == "<na>") {
					WordMapClean["<na>"] =  WordsVecsClean.size();
					vector<float> NullVec(NumValsPerVec, 0.0);
					WordsVecsClean.push_back(NullVec);
					RevWordMapClean.push_back("<na>");
				}
				else {
					cerr << "no vec found for " <<RevWordMap[iwv] << endl;
				}
				

			}
		}
		if (cbDoubleWordVecs) {
			ofstream strNames(WordListFileName);
			if (strNames.is_open()) {
				for (auto w : RevWordMapClean) {
					strNames << w << " ";
					int iw = WordMapClean[w];
					auto OneVec = WordsVecsClean[iw];
					for (int iwv = 0; iwv < OneVec.size(); iwv++) {
						auto wv = OneVec[iwv];
						strNames << wv;
						if (iwv == OneVec.size() - 1) {
							strNames << endl;
						}
						else {
							strNames << " ";
						}
					}
				}
			}
		}
//		for (vector<float>& OneVec : WordsVecs) {
//			if (OneVec.size() < NumValsPerVec * 2) {
//				int StartSize = OneVec.size();
//				for (int iwv = StartSize; iwv < NumValsPerVec * 2; iwv++) {
//					OneVec.push_back(0.0);
//				}
//			}
//		}
	
	}
	
	vector<vector<float> > PosVecTbl;
	vector<vector<float> > PosNumTbl;
	int PosMapSize = PosMap.size();
	for (auto PosRec : PosMap) {
		PosVecTbl.push_back(vector<float>(PosMapSize, 0.0f));
		vector<float>& CurrVec = PosVecTbl.back();
		CurrVec[PosRec.second] = 1.0f;
		PosNumTbl.push_back(vector<float>(1, (float)PosRec.second));
	}
	vector<vector<float> > YesNoTbl;
	YesNoTbl.push_back(vector<float>(1, 0.0f));
	YesNoTbl.push_back(vector<float>(1, 1.0f));
	
	int YesNoTblIdx = -1;
	vector<map<string, int>*> TranslateTblPtrs;
	vector<vector<vector<float> >* > VecTblPtrs;
	map<string, int> TranslateTblNameMap;
	TranslateTblNameMap["WordVecTbl"] = TranslateTblPtrs.size();
	TranslateTblPtrs.push_back(&WordMapClean);
	VecTblPtrs.push_back(&WordsVecsClean);
	TranslateTblNameMap["POSVecTbl"] = TranslateTblPtrs.size();
	TranslateTblPtrs.push_back(&PosMap);
	VecTblPtrs.push_back(&PosVecTbl);
	TranslateTblNameMap["POSNumTbl"] = TranslateTblPtrs.size();
	TranslateTblPtrs.push_back(&PosMap); // note, the same pos map tbl as prev
	VecTblPtrs.push_back(&PosNumTbl);
	YesNoTblIdx = TranslateTblPtrs.size();
	TranslateTblNameMap["YesNoTbl"] = YesNoTblIdx;
	TranslateTblPtrs.push_back(NULL);
	VecTblPtrs.push_back(&YesNoTbl);
	// each pair in the table: first - idx of rec to access. second: idx of VarTbl to put it into
	vector<pair<int, int> > FirstAccessFieldsIdx; 
	map<string, int> VarNamesMap;
	for (int idf = 0; idf < gen_data->data_fields_size(); idf++) {
		const CaffeGenData::DataField& Field = gen_data->data_fields(idf);
		const string& VarName = Field.var_name();
		map<string, int>::iterator itvnm = VarNamesMap.find(VarName);
		if (itvnm != VarNamesMap.end()) {
			cerr << "Error parsing prototxt data. " << VarName << " defined twice\n";
			return;
		}
		int NextVarNamesMapPos = VarNamesMap.size();
		VarNamesMap[VarName] = NextVarNamesMapPos;
		int FieldID = GetIdxFromFieldName(Field.field_name());
		FirstAccessFieldsIdx.push_back(make_pair(FieldID, NextVarNamesMapPos));
	}
	// for each pair in table: first - idx of VarTbl. second - index of translate tbl
	vector<pair<int, int> > InputTranslateTbl;
	vector<pair<int, int> > OutputTranslateTbl;
	bool bCanReplace = false;
	vector<bool> ICanReplaceTbl;
	vector<bool> OCanReplaceTbl;
	// Num number of instances of that word in that data field
	// one for in and one for out
	// combining old and vew arrays
	// indexed the same as the translate tbl
	vector<vector<int> > NumInstancesTbl[2];
	vector<int> MaxInstancesTbl[2];
	int OTranslateTableSize;
	for (int iBoth=0; iBoth<2; iBoth++) {
		vector<pair<int, int> >* pTranslateTbl = &InputTranslateTbl;
		vector<bool>* pCanReplaceTbl =  &ICanReplaceTbl;
		int TranslateTableSize = gen_data->input_field_translates_size(); 
		if (iBoth == 1) {
			pTranslateTbl = &OutputTranslateTbl;
			pCanReplaceTbl =  &OCanReplaceTbl;
			TranslateTableSize = gen_data->output_field_translates_size(); 
			OTranslateTableSize = TranslateTableSize;
		}
		for (int ift = 0; ift < TranslateTableSize; ift++) {
			const CaffeGenData::FieldTranslate* pTran;
			if (iBoth == 0) {
				pTran = &(gen_data->input_field_translates(ift));
			}
			else {
				pTran = &(gen_data->output_field_translates(ift));
			}
			map<string, int>::iterator itvnm = VarNamesMap.end();
			const string& TableName = pTran->table_name();
			if (pTran->has_var_name()) {
				const string& VarName = pTran->var_name();
				itvnm = VarNamesMap.find(VarName);
				if (itvnm == VarNamesMap.end()) {
					cerr << "Error parsing prototxt data. Translate table field " << VarName << " is not defined previously\n";
					return;
				}
			}
			else if (TableName != "YesNoTbl") {
				cerr << "Error parsing prototxt data. Translate table field must be provided unless the table name is \"YesNoTbl\"\n";
				return;
			}
			map<string, int>::iterator itttnm = TranslateTblNameMap.find(TableName);
			if (itttnm == TranslateTblNameMap.end()) {
				cerr << "Error parsing prototxt data. Translate table name " << TableName << " does not exist.\n";
				return;
			}
			pTranslateTbl->push_back(make_pair(itvnm->second, itttnm->second));
			bool bThisCanReplace = pTran->b_can_replace();
			if (bThisCanReplace) bCanReplace = true;
			pCanReplaceTbl->push_back(bThisCanReplace);
			if (pTran->has_max_instances()) {
				// if this field needs a count of instance, add an array with the size of the table
				NumInstancesTbl[iBoth].push_back(vector<int> (TranslateTblPtrs[itttnm->second]->size(), 0));
				MaxInstancesTbl[iBoth].push_back(pTran->max_instances());
			}
			else {
				// else add empty array
				// empty or full, one must be added to keep indicies in sync
				NumInstancesTbl[iBoth].push_back(vector<int> ());
				MaxInstancesTbl[iBoth].push_back(-1);
			}

		}
	}
	gen_data->net_end_type();
	int NumOutputNodesNeeded = -1;
	CaffeGenSeed::NetEndType EndType;
	if (gen_data->net_end_type() == CaffeGenData::END_VALID) {
		if (OTranslateTableSize != 1) {
			cerr << "For end type END_VALID, the number of output_field_translates must be exactly 1.\n";	
			return;
		}
		NumOutputNodesNeeded = 2;
		EndType = CaffeGenSeed::END_VALID;
	}
	else if (gen_data->net_end_type() == CaffeGenData::END_ONE_HOT) {
		if (OTranslateTableSize != 1) {
			cerr << "For end type END_ONE_HOT, the number of output_field_translates must be exactly 1.\n";	
			return;
		}
		const string& TableName = gen_data->output_field_translates(0).table_name();
		auto pTbl = TranslateTblPtrs[TranslateTblNameMap[TableName]];
		NumOutputNodesNeeded = pTbl->size();
		EndType = CaffeGenSeed::END_ONE_HOT;
	}
	else if (gen_data->net_end_type() == CaffeGenData::END_MULTI_HOT) {
		NumOutputNodesNeeded = 0;
		for (int ift = 0; ift < OTranslateTableSize; ift++) {
			const string& TableName = gen_data->output_field_translates(0).table_name();
			auto pTbl = VecTblPtrs[TranslateTblNameMap[TableName]];
			NumOutputNodesNeeded += (*pTbl)[0].size();
		}
		EndType = CaffeGenSeed::END_MULTI_HOT;
	}
	
	int NumWordsClean = RevWordMapClean.size();
	int iNA = NumWordsClean - 1;
	int iPosNA = PosMap.size() - 1; 
	 
	// not the actual vecs, but the integers that will give the vecs
	// random, IData, valid, OData
	vector<tuple<int, vector<int>, bool, vector<int> > > DataForVecs;
	int NumCandidates = 0;
	
	for (SSentenceRec Rec : SentenceRec) {
		auto& WordRecs = Rec.OneWordRec;
		if (Rec.OneWordRec.size() < cMinRealLength) {
			continue;
		}
		for (auto wrec : WordRecs) {
			vector<string> VarTbl(VarNamesMap.size()); 
			bool bAllFieldsFound = true;
			for (auto access : FirstAccessFieldsIdx) {
				bool bValid = true;
				VarTbl[access.second] = GetRecFieldByIdx(wrec, access.first, bValid);
				if (!bValid) {
					bAllFieldsFound = false;
					break;
				}
			}
			if (!bAllFieldsFound) {
				continue;
			}
			NumCandidates++;
			vector<int> IData;
			vector<int> OData;
			for (int iBoth=0; bAllFieldsFound && iBoth<2; iBoth++) {
				auto* pTranslateTbl = &InputTranslateTbl;
				vector<int>* pData =  &IData;
				if (iBoth == 1) {
					pData = &OData;
					pTranslateTbl = &OutputTranslateTbl;
				}
				int iitt = -1;
				for (auto itt : *pTranslateTbl) {
					iitt++;
					if (itt.second == YesNoTblIdx) {
						pData->push_back(1);
					}
					else {
						string& FirstAccessVal = VarTbl[itt.first];
						map<string, int>* mapp = TranslateTblPtrs[itt.second];
						map<string, int>::iterator itm = mapp->find(FirstAccessVal);

						if (itm == mapp->end()) {
							bAllFieldsFound = false;
							break;
						}
						if (MaxInstancesTbl[iBoth][iitt] >= 0) {
							if (NumInstancesTbl[iBoth][iitt][itm->second] >= MaxInstancesTbl[iBoth][iitt]) {
								bAllFieldsFound = false;
								break;
							}
							NumInstancesTbl[iBoth][iitt][itm->second]++;
						}
						pData->push_back(itm->second);
					}
					
				}
			}
			if (!bAllFieldsFound) {
				continue;
			}
			DataForVecs.push_back(make_tuple(rand(), IData, true, OData));
			if (bCanReplace) {
				vector<int> IDataRepl(IData.size());
				vector<int> ODataRepl(OData.size());
				for (int iBoth=0; iBoth<2; iBoth++) {
					auto pDataRepl = &IDataRepl;
					auto pTranslateTbl = &InputTranslateTbl;
					auto pData = &IData;
					auto pCanReplaceTbl =  &ICanReplaceTbl;
					if (iBoth == 1) {
						pDataRepl = &ODataRepl;
						pTranslateTbl = &OutputTranslateTbl;
						pData = &OData; 
						pCanReplaceTbl =  &OCanReplaceTbl;
					}
					int iitt = 0;
					for (auto itt : (*pTranslateTbl)) {
						if (itt.second == YesNoTblIdx) {
							(*pDataRepl)[iitt] = 0;
						}
						else {
							map<string, int>* mapp = TranslateTblPtrs[itt.second];
							if ((*pCanReplaceTbl)[iitt]) {
								int ReplaceVal = rand() % mapp->size();
								(*pDataRepl)[iitt] = ReplaceVal;
							}
							else {
								(*pDataRepl)[iitt] = (*pData)[iitt];
							}
						}
						iitt++;
					}
				}
				DataForVecs.push_back(make_tuple(rand(), IDataRepl, false, ODataRepl));
			}
		}
	} // end loop over sentence recs

//		{
//			// this goes outside Rec Loop
//			//something is wrong here. Too few records get here. Count the candidates and see why this is happenining
//			sort(DataForVecs.begin(), DataForVecs.end());
//			for (auto idata : DataForVecs) {
//				for (int iBoth=0; iBoth<2; iBoth++) {
//					auto pTranslateTbl = &InputTranslateTbl;
//					auto pData = &(get<1>(idata));
//					if (iBoth == 1) {
//						pTranslateTbl = &OutputTranslateTbl;
//						pData = &(get<3>(idata));
//					}
//					int ii = 0;
//					for (auto itt : (*pTranslateTbl)) {
//						vector<float>& vec = (*VecTblPtrs[itt.second])[(*pData)[ii]];
//						ii++;
//					}
//				}
//				bool bValid = get<2>(idata);
//			}
//		}

	cerr << "Num candidates " << NumCandidates << " resulting in " << DataForVecs.size() << endl;
	// use the random int in the first field to sort
	sort(DataForVecs.begin(), DataForVecs.end());
	
	int NumRecords = DataForVecs.size() / 2;
	int NumLabelVals = 0;
	NumItemsPerRec = 0;
	for (auto itt : InputTranslateTbl) {
		NumItemsPerRec += (*VecTblPtrs[itt.second])[0].size();
	}
	for (auto ott : OutputTranslateTbl) {
		NumLabelVals += (*VecTblPtrs[ott.second])[0].size();
	}

//	for (auto Gram : DataForVecs) {
//		auto WordIVec = get<1>(Gram);
//		for (int wi : WordIVec) {
//			string& w = RevWordMapClean[wi];
//			cout << w << " ";			
//		}
//		cout << "(" << (get<2>(Gram) ? "OK" : "Bad") << ")\n";
//	}
//	cout << "Found " << NumRecords << " records/ngrams\n";

	string H5FileName = H5TrainFileName;
	
	for (int iBoth = 0; iBoth < 2; iBoth++ ) {
		if (iBoth == 1) {
			H5FileName = H5TestFileName;
		}
		H5std_string	DATASET_NAMES; // ("data1");
		const H5std_string	LABELSET_NAME("label");
		const int cDataRank =  2;
		hsize_t dims[cDataRank];               // dataset dimensions
		dims[1] = NumItemsPerRec;
		dims[0] = NumRecords;
		const int cLabelRank = 2;
		hsize_t dimsLabel[cLabelRank];               // dataset dimensions
		dimsLabel[1] = NumLabelVals;
		//dimsLabel[1] = cLSTMBatchSize * 1;
		dimsLabel[0] = NumRecords;

		int DataSetSize = NumRecords * NumItemsPerRec;
		int LabelSetSize = NumRecords * NumLabelVals;

		float * pDataSet;// = new float[DataSetSize];
		float * plabels = new float[LabelSetSize];
		float * ppl = plabels;

		DATASET_NAMES = "data";
		pDataSet = new float[DataSetSize];
		float * ppd = pDataSet;
		for (int ir=0; ir < NumRecords * 2; ir++) {
			if ((ir % 2) == iBoth) {
				auto pIData = &(get<1>(DataForVecs[ir]));
				auto pOData = &(get<3>(DataForVecs[ir]));
				int ii = 0;
				for (auto itt : InputTranslateTbl) {
					vector<float>& vec = (*VecTblPtrs[itt.second])[(*pIData)[ii]];
					for (float v : vec) {
						*ppd++ = v;
					}
					ii++;
				}
				int io = 0;
				for (auto ott : OutputTranslateTbl) {
					vector<float>& vec = (*VecTblPtrs[ott.second])[(*pOData)[io]];
					for (float v : vec) {
						*ppl++ = v;
					}
					io++;
				}
				
			}
		}
		

		H5File h5file(H5FileName, H5F_ACC_TRUNC);
		DataSpace dataspace(cDataRank, dims);

		DataSet dataset = h5file.createDataSet(DATASET_NAMES, PredType::IEEE_F32LE, dataspace);

		dataset.write(pDataSet, PredType::IEEE_F32LE);

		delete pDataSet;

		
//		cerr << Count[0] << " zeroes vs " << Count[1] << " 1s.\n";

		DataSpace labelspace(cLabelRank, dimsLabel);

		DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

		labelset.write(plabels, PredType::IEEE_F32LE);

		delete plabels;

	}
	{

		ofstream test_list(H5TestListFileName, ofstream::app);
		ofstream train_list(H5TrainListFileName, ofstream::app);
//		ofstream test_list(H5TestListFileName);
		if (test_list.is_open()) {
			test_list << H5TestFileName << endl;
		}
//		ofstream train_list(H5TrainListFileName);
		if (train_list.is_open()) {
			train_list << H5TrainFileName << endl;
		}
	}
	int TestCases = gen_seed_config->num_test_cases() + NumRecords;
	gen_seed_config->set_num_test_cases(TestCases);
	// the following two repeatedly set. No harm AFAICS
	gen_seed_config->set_num_output_nodes(NumOutputNodesNeeded);
	gen_seed_config->set_net_end_type(EndType);
}

