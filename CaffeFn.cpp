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

#include "GenSeed.pb.h"
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
	fniIndex,
	fniRWID,
	fniDepType,
	fniGov,
	fniDep,
	fniRecAndGov,
	fniRecAndDep,
	fniDIDGov,
	fniDIDDep,
	fniDepNameGov,
	fniDepNameDep,
};

struct SFieldNameToID {
	string name;
	int id;
};

SFieldNameToID FieldNameToIDTbl[] = {	{"Word", fniWord}, 
										{"WordCore", fniWordCore},  
										{"POS", fniPOS},
										{"index", fniIndex},
										{"RWID", fniRWID},
										{"DepType", fniDepType},
										{"Gov", fniGov},
										{"Dep", fniDep},										
										{"RecAndGov", fniRecAndGov},
										{"RecAndDep", fniRecAndDep},
										{"DIDGov", fniDIDGov},
										{"DIDDep", fniDIDDep},
										{"DepNameGov", fniDepNameGov},
										{"DepNameDep", fniDepNameDep},
									};

int FieldNamesTblSize = sizeof(FieldNameToIDTbl) / sizeof(FieldNameToIDTbl[0]);

string GetRecFieldByIdx(int SRecID, int WID, WordRec& rec, 
						CaffeGenData_FieldType FieldID, bool& bRetValid)
{
	bRetValid = true;
	switch(FieldID) {
		case CaffeGenData::FIELD_TYPE_WORD: {
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
		case CaffeGenData::FIELD_TYPE_WORD_CORE: 
			return rec.WordCore;
		case CaffeGenData::FIELD_TYPE_POS:
			return rec.POS;
//		case fniIndex: I think this is a misunderstanding
//			return to_string(FieldID);
		case CaffeGenData::FIELD_TYPE_WID:
			return (to_string(WID)) ;
		case CaffeGenData::FIELD_TYPE_RWID:
			return (to_string(SRecID) + ":" + to_string(WID)) ;
		default:
			bRetValid = false;
			break;
	}
	return string();
}

string GetDepRecFieldByIdx(	int SRecID, int DID, vector<string>& DepNames, DepRec& rec, 
							CaffeGenData_FieldType FieldID, bool& bRetValid)
{
	bRetValid = true;
	switch(FieldID) {
		case CaffeGenData::FIELD_TYPE_DEP_NAME: 
			return (DepNames[rec.iDep]) ;
		case CaffeGenData::FIELD_TYPE_GOV_WID: 
			return (to_string((int)rec.Gov)) ;
		case CaffeGenData::FIELD_TYPE_DEP_WID: 
			return (to_string((int)rec.Dep)) ; 
		case CaffeGenData::FIELD_TYPE_GOV_RWID: 
			return (to_string(SRecID) + ":" + to_string((int)rec.Gov)) ;
		case CaffeGenData::FIELD_TYPE_DEP_RWID: 
			return (to_string(SRecID) + ":" + to_string((int)rec.Dep)) ; 
		case CaffeGenData::FIELD_TYPE_GOV_RDID:
			return (to_string(SRecID) + ":" + to_string(DID) + ":g");
		case CaffeGenData::FIELD_TYPE_DEP_RDID:
			return (to_string(SRecID) + ":" + to_string(DID) + ":d");
		case CaffeGenData::FIELD_TYPE_DEP_NAME_G:
			return (DepNames[rec.iDep] + ":g") ;
		case CaffeGenData::FIELD_TYPE_DEP_NAME_D:
			return (DepNames[rec.iDep] + ":d") ;
		default:
			bRetValid = false;
			break;
	}
	return string();
}

int GetIdxFromFieldName (const string& name) {
	for (int ifn = 0; ifn < FieldNamesTblSize; ifn++) {
		if (name == FieldNameToIDTbl[ifn].name) {
			return FieldNameToIDTbl[ifn].id;
		}
	}
	return -1;
}

struct CaffeFnInitData {
	CaffeGenData* gen_data;
	CaffeGenSeed* gen_seed_config;
	int NumVecTbls;
	vector<map<string, int>*> TranslateTblPtrs;
	vector<vector<vector<float> >* > VecTblPtrs;
	map<string, int> TranslateTblNameMap;
	vector<string> DepNames;
	int YesNoTblIdx;
};
void CGotitEnv::CaffeFnInit() 
{
	CaffeFnInitData * InitData = new CaffeFnInitData;

	CaffeFnDataHandle = NULL;
//	CaffeFnHandle = NULL;
//	CaffeFnOutHandle = NULL;
	CaffeGenData* gen_data = new CaffeGenData;
	InitData->gen_data = gen_data;
	
	string sModelProtoName;
	if (!GetImplemParam(sModelProtoName, "Implem.Param.FnParam.CaffeFn.ModelProtoName")) {
		cerr << "CaffeFn cannot be called without setting Implem.Param.FnParam.CaffeFn.ModelProtoName \n";
		return;
	}
	ifstream proto_ifs(sModelProtoName);
	if (proto_ifs.is_open()) {
		google::protobuf::io::IstreamInputStream* proto_input 
			= new google::protobuf::io::IstreamInputStream(&proto_ifs);
		if (!google::protobuf::TextFormat::Parse(proto_input, gen_data)) {
			cerr << "CaffeFn config file Parse failed for " << sModelProtoName << endl;
			return;
		}
		delete proto_input;
	}

	CaffeGenSeed* gen_seed_config = new CaffeGenSeed;
	InitData->gen_seed_config = gen_seed_config;

	const string& CoreDir = gen_data->files_core_dir();
	string H5TrainListFileName = CoreDir + "data/train_list.txt";
	string H5TestListFileName = CoreDir + "data/test_list.txt";
	
	ofstream test_list(H5TestListFileName, ofstream::trunc);
	ofstream train_list(H5TrainListFileName, ofstream::trunc);

	gen_seed_config->set_num_test_cases(0);
	
	struct VecTblData {
		map<string, int> NameMap;
		vector<vector<float> > VecData;
	};
//	vector<string> * pDepNames = new vector<string>();
//	InitData->DepNames = 
	InitData->NumVecTbls = gen_data->vec_tbls_size();
	int ivt = -1;
	for (auto vec_tbl : gen_data->vec_tbls()) { ivt++;
		InitData->TranslateTblNameMap[vec_tbl.name()] = ivt;
		bool bThisTblIsTheVecTbl = false;
		if (vec_tbl.name() == gen_data->dep_name_vec_tbl()) {
			bThisTblIsTheVecTbl = true;
			InitData->DepNames.clear();
		}
		map<string, int>* pNameMap = new map<string, int>();
		InitData->TranslateTblPtrs.push_back(pNameMap);
		vector<vector<float> >* pVecTbl = new vector<vector<float> >();
		InitData->VecTblPtrs.push_back(pVecTbl);
		string VecTblPath = gen_data->vec_tbls_core_path() + vec_tbl.path() + "/OutputVec.txt";
		ifstream VecFile(VecTblPath);
		if (VecFile.is_open()) {
			string ln;
			int NumVecs;
			int NumValsPerVec;
			VecFile >> NumVecs;
			VecFile >> NumValsPerVec;
//			if (bThisTblIsTheVecTbl) {
//				pDepNames->resize(NumVecs);
//			}

			getline(VecFile, ln); // excess .n
			//while (!VecFile.eof()) {
			for (int ic = 0; ic < NumVecs; ic++) {
				getline(VecFile, ln, ' ');
				string w;
				w = ln;
				if (w.size() == 0) {
					cerr << "There should be as many items in the vec file as stated on the first line of file.\n";
					return;
				}
				if (bThisTblIsTheVecTbl) {
					InitData->DepNames.push_back(w);
				}
				(*pNameMap)[w] = ic;
				pVecTbl->push_back(vector<float>());
				vector<float>& OneVec = pVecTbl->back();
				//vector<float>& OneVec = WordsVecs[iw];
				for (int iwv = 0; iwv < NumValsPerVec; iwv++) {
					if (iwv == NumValsPerVec - 1) {
						getline(VecFile, ln);
					}
					else {
						getline(VecFile, ln, ' ');
					}
					float wv;
					wv = stof(ln);
					OneVec.push_back(wv);
				}
			}
		}
	}

	vector<vector<float> > * YesNoTbl = new vector<vector<float> >;
	YesNoTbl->push_back(vector<float>(1, 0.0f));
	YesNoTbl->push_back(vector<float>(1, 1.0f));

	
	InitData->YesNoTblIdx = InitData->TranslateTblPtrs.size();
	InitData->TranslateTblNameMap["YesNoTbl"] = InitData->YesNoTblIdx;
	InitData->TranslateTblPtrs.push_back(NULL);
	InitData->VecTblPtrs.push_back(YesNoTbl);
//	CaffeFnHandle = gen_data;
//	CaffeFnOutHandle = gen_seed_config;
	CaffeFnDataHandle = InitData;
}
void CGotitEnv::CaffeFnComplete()
{
	
	//if (!CaffeFnHandle || !CaffeFnOutHandle) {
	if (!CaffeFnDataHandle)	 {
		cerr << "CaffeFnComplete can only work if preceeded by a succesful call to CaffeFnInit.\n";
		return;
		
	}
	CaffeFnInitData * InitData = (CaffeFnInitData * )CaffeFnDataHandle;
	CaffeGenData* gen_data = InitData->gen_data; // (CaffeGenData *)CaffeFnHandle;
	CaffeGenSeed* gen_seed_config  = InitData->gen_seed_config; // (CaffeGenSeed*)CaffeFnOutHandle;

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
	//CaffeFnHandle = NULL;
	delete gen_seed_config ;
	//CaffeFnOutHandle = NULL;
	for (auto p : InitData->TranslateTblPtrs) {
		if (p != NULL) {
			delete p;
		}
	}
	for (auto p : InitData->VecTblPtrs) {
		if (p != NULL) {
			delete p;
		}
	}
	delete InitData;
	CaffeFnDataHandle = NULL;
}

void CGotitEnv::CaffeFn()
{

	if (!CaffeFnDataHandle)	 {
		cerr << "CaffeFn can only work if preceeded by a succesful call to CaffeFnInit.\n";
		return;
		
	}
	CaffeFnInitData * InitData = (CaffeFnInitData * )CaffeFnDataHandle;
	CaffeGenData* gen_data = InitData->gen_data; // (CaffeGenData *)CaffeFnHandle;
	CaffeGenSeed* gen_seed_config  = InitData->gen_seed_config; // (CaffeGenSeed*)CaffeFnOutHandle;
	vector<map<string, int>*>& TranslateTblPtrs = InitData->TranslateTblPtrs;
	vector<vector<vector<float> >* >& VecTblPtrs = InitData->VecTblPtrs;
	map<string, int>& TranslateTblNameMap = InitData->TranslateTblNameMap;
	vector<string>& DepNames = InitData->DepNames;
	int YesNoTblIdx = InitData->YesNoTblIdx;

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
	
	const bool cbDoubleWordVecs = true; // false for autoencode thin vector
	const int cNumValsPerVecSrc = 50; // 10 - size of thin vector
	int NumValsPerVec = cNumValsPerVecSrc * (cbDoubleWordVecs ? 2 : 1);

	
	const int cNumWords = 6000; //2047;
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
//	map<string, int> WordMap;
//	vector<string> RevWordMap;
////	vector<int> GramsFound(cNumWords, 0); 
//	vector<vector<float> > WordsVecs(cNumWords+1); // indexed by the same as WordMap

//	map<string, int> WordMapClean;
//	vector<string> RevWordMapClean;
//	vector<vector<float> > WordsVecsClean; 
	
	


	// each pair in the table: first - idx of rec to access. second: idx of VarTbl to put it into
	vector<pair<CaffeGenData_FieldType, int> > FirstAccessFieldsIdx; 
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
		//int FieldID = GetIdxFromFieldName(Field.field_name());
		FirstAccessFieldsIdx.push_back(make_pair(Field.field_type(), NextVarNamesMapPos));
	}
//	for (int idt = 0; idt < gen_data->data_translates_size(); idt++) {
//		const CaffeGenData::DataTranslate& Field = gen_data->data_translates(idt);
//		const string& VarName = Field.var_name();
//		map<string, int>::iterator itvnm = VarNamesMap.find(VarName);
//		if (itvnm != VarNamesMap.end()) {
//			cerr << "Error parsing prototxt data. " << VarName << " defined twice\n";
//			return;
//		}
//		int NextVarNamesMapPos = VarNamesMap.size();
//		VarNamesMap[VarName] = NextVarNamesMapPos;
//		int FieldID = GetIdxFromFieldName(Field.field_name());
//		FirstAccessFieldsIdx.push_back(make_pair(FieldID, NextVarNamesMapPos));
//	}
	enum DataTranslateEntryType {
		dtetRWIDToWord,
		dtetRWIDToCoref,
		dtetRWIDToRDID,
		dtetRDIDToDepRWID,
		dtetRDIDToGovRWID,
		dtetRDIDToDepName,
	};
	struct SDataTranslateEntry {
		DataTranslateEntryType dtet;
		int VarTblIdx; // index of field of var tbl to write result to 
		int VarTblMatchIdx; // index of search field to retrieve from var tbl
		int TargetTblMatchIdx; // index of field in target to match
		CaffeGenData_FieldType TargetTblOutputIdx; // idx of field in target table to output
	};
	vector<SDataTranslateEntry> DataTranslateTbl;
	for (int idt = 0; idt < gen_data->data_translates_size(); idt++) {
		SDataTranslateEntry DataTranslateEntry ;
		DataTranslateEntry.dtet = dtetRWIDToWord;
		const CaffeGenData::DataTranslate& GenDataTranslateEntry 
			= gen_data->data_translates(idt);
		CaffeGenData::DataTranslateType TranslateType = GenDataTranslateEntry.translate_type(); 
		switch (TranslateType) {
			case CaffeGenData::DATA_TRANSLATE_RWID_TO_WORD :
				DataTranslateEntry.dtet = dtetRWIDToWord;
				break;				
			case CaffeGenData::DATA_TRANSLATE_RWID_TO_COREF:
				DataTranslateEntry.dtet = dtetRWIDToCoref;
				break;
			case CaffeGenData::DATA_TRANSLATE_RWID_TO_RDID:
				DataTranslateEntry.dtet = dtetRWIDToRDID;
				break;
 			case CaffeGenData::DATA_TRANSLATE_RDID_TO_DEP_RWID:
				DataTranslateEntry.dtet = dtetRDIDToDepRWID;
				break;
 			case CaffeGenData::DATA_TRANSLATE_RDID_TO_GOV_RWID :
				DataTranslateEntry.dtet = dtetRDIDToGovRWID;
				break;
			case CaffeGenData::DATA_TRANSLATE_RDID_TO_DEP_NAME:
				DataTranslateEntry.dtet = dtetRDIDToDepName;
				break;
		}
		
		const string& MatchName = GenDataTranslateEntry.match_name();
		auto itVarNamesMap = VarNamesMap.find(MatchName);
		if (itVarNamesMap == VarNamesMap.end()) {
			cerr << "Error parsing prototxt data. Translate data match_name " << MatchName << " does not exist.\n";
			return;
		}
		DataTranslateEntry.VarTblMatchIdx = itVarNamesMap->second;

		CaffeGenData_FieldType FieldID = CaffeGenData::FIELD_TYPE_INVALID;
		if (GenDataTranslateEntry.has_field_type() ) {
			FieldID = //GetIdxFromFieldNif Dame(GenDataTranslateEntry.field_type());
			FieldID = GenDataTranslateEntry.field_type();
		}
		else {
			if (DataTranslateEntry.dtet == dtetRWIDToWord) {
				cerr << "field_name is required if DATA_TRANSLATE_RWID_TO_WORD is set.\n";
				return;
			}
		}
		DataTranslateEntry.TargetTblOutputIdx = FieldID;
		const string& OutputVarName = GenDataTranslateEntry.var_name();
		auto itvnm = VarNamesMap.find(OutputVarName);
		if (itvnm != VarNamesMap.end()) {
			cerr << "Error parsing prototxt data. " << OutputVarName << " defined twice\n";
			return;
		}
		int VarNamesMapSize = VarNamesMap.size();
		VarNamesMap[OutputVarName] = VarNamesMapSize;
		DataTranslateEntry.VarTblIdx = VarNamesMapSize;

		DataTranslateTbl.push_back(DataTranslateEntry);
	}
	// first field the index of the var to be matched in var tbl, second, the string to match
	vector<pair<int, string> > DataFilterTbl;
	for (int idf = 0; idf < gen_data->data_filters_size(); idf++) {
		const CaffeGenData::DataFilter& GenDataFilter = gen_data->data_filters(idf);
		const string& MatchName = GenDataFilter.var_name();
		auto itVarNamesMap = VarNamesMap.find(MatchName);
		if (itVarNamesMap == VarNamesMap.end()) {
			cerr << "Error parsing prototxt data. Filter data var_name " << MatchName << " does not exist.\n";
			return;
		}
		DataFilterTbl.push_back(make_pair(itVarNamesMap->second, GenDataFilter.match_string()));
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
			int VarNameIdx = -1;
			const string& TableName = pTran->table_name();
			if (pTran->has_var_name()) {
				map<string, int>::iterator itvnm = VarNamesMap.end();
				const string& VarName = pTran->var_name();
				itvnm = VarNamesMap.find(VarName);
				if (itvnm == VarNamesMap.end()) {
					cerr << "Error parsing prototxt data. Translate table field " << VarName << " is not defined previously\n";
					return;
				}
				VarNameIdx = itvnm->second;
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
			pTranslateTbl->push_back(make_pair(VarNameIdx, itttnm->second));
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
	
	// create a reverse table for the coref data
	
	vector<vector<int> > CorefRevTbl(SentenceRec.size(), vector<int>());
	{
		int isrec = -1;
		for (auto srec : SentenceRec) {
			isrec++;
			CorefRevTbl[isrec].resize(srec.OneWordRec.size(), -1);
		}

		int icrec = -1;
		for (auto crec : CorefList) {
			icrec++;
			CorefRevTbl[crec.SentenceID][crec.HeadWordId] = icrec;
		}
	}
	// step through records creating data based on translation tables just created
	
//	int NumWordsClean = RevWordMapClean.size();
//	int iNA = NumWordsClean - 1;
//	int iPosNA = PosMap.size() - 1; 
	 
	// not the actual vecs, but the integers that will give the vecs
	// random, IData, valid, OData
	vector<tuple<int, vector<int>, bool, vector<int> > > DataForVecs;
	int NumCandidates = 0;
	
	int * po = &(OutputTranslateTbl[0].second);
	vector<vector<string> > VarTblsForGo; 

	bool bContinueWithMainLoop = true;
	int isr = 0;
	while (bContinueWithMainLoop) {
		VarTblsForGo.clear();
		if (gen_data->iterate_type() == CaffeGenData::ITERATE_DEP) {
			const int cNumSentenceRecsPerDepGo = 1;
			for (int iisr = 0; iisr < cNumSentenceRecsPerDepGo; iisr++, isr++) {
				if (isr >= SentenceRec.size()) {
					bContinueWithMainLoop = false;
					break;
				}
				SSentenceRec Rec = SentenceRec[isr];
				if (Rec.Deps.size() < cMinRealLength) {
					continue;
				}
				auto& DepRecs = Rec.Deps;
				int idrec = -1; 
				for (auto drec : DepRecs) { idrec++;
					VarTblsForGo.push_back(vector<string> (VarNamesMap.size()));
					vector<string>& VarTbl = VarTblsForGo.back(); 
					bool bAllFieldsFound = true;
					for (auto access : FirstAccessFieldsIdx) {
						bool bValid = true;
						if (access.second >= VarTbl.size()) {
							cerr << "Serious error!\n";
							return;
						}
						VarTbl[access.second] 
								= GetDepRecFieldByIdx(	isr, idrec, DepNames, drec, 
														access.first, bValid);
						if (!bValid) {
							bAllFieldsFound = false;
							break;
						}
					}
					NumCandidates++;
				}
			}
		}	
		else if (gen_data->iterate_type() == CaffeGenData::ITERATE_WORD) {
			const int cNumSentenceRecsPerGo = 1;
			for (int iisr = 0; iisr < cNumSentenceRecsPerGo; iisr++, isr++) {
				if (isr >= SentenceRec.size()) {
					bContinueWithMainLoop = false;
					break;
				}
				SSentenceRec Rec = SentenceRec[isr];
				auto& WordRecs = Rec.OneWordRec;
				if (Rec.OneWordRec.size() < cMinRealLength) {
					continue;
				}
				int iwrec = -1; for (auto wrec : WordRecs) { iwrec++;
					VarTblsForGo.push_back(vector<string> (VarNamesMap.size()));
					vector<string>& VarTbl = VarTblsForGo.back(); 
					bool bAllFieldsFound = true;
					for (auto access : FirstAccessFieldsIdx) {
						bool bValid = true;
						if (access.second >= VarTbl.size()) {
							cerr << "Serious error!\n";
							return;
						}
						VarTbl[access.second] = GetRecFieldByIdx(isr, iwrec, wrec, access.first, bValid);
						if (!bValid) {
							bAllFieldsFound = false;
							break;
						}
					}
					if (!bAllFieldsFound) {
						continue;
					}
					NumCandidates++;
				}
			}
		}
		
		if (DataTranslateTbl.size() > 0) {
			vector<vector<string> > VarTblsForGoTranslated; 
			for (auto& VarTbl : VarTblsForGo) {
				bool bAllGood = true;
				for (auto& DataTranslateEntry :  DataTranslateTbl) {
					string VarNameForMatch = VarTbl[DataTranslateEntry.VarTblMatchIdx];
					// Phase 1. Access the current record
					int WID = -1; // Assumes the translation was from WID, the id of the record in the WordRecs Tbl
					int RecID = -1;
					int DID = -1;
					bool bGov = true;
					if (	DataTranslateEntry.dtet == dtetRWIDToWord 
						||	DataTranslateEntry.dtet == dtetRWIDToCoref
						||	DataTranslateEntry.dtet == dtetRWIDToRDID) {
						int ColonPos = VarNameForMatch.find(":");
						if (ColonPos == -1) {
							cerr << "Error: RWID result is not formatted with a \":\". WID must be formatted as RecID::idx where idx is the index of the word in the sentence\n";
							// this is really a parsing error not a data error so we return
							return;
						}
						RecID = stoi(VarNameForMatch.substr(0, ColonPos));
						if (RecID >= SentenceRec.size()) {
							cerr << "Error: Record number stored from dep result is greater than the number of SentenceRecs\n";
							bAllGood = false;
							break;
						}
						WID = stoi(VarNameForMatch.substr(ColonPos+1));
					}
					if (	( DataTranslateEntry.dtet == dtetRDIDToDepRWID) 
						||	(DataTranslateEntry.dtet == dtetRDIDToGovRWID) 
						||	(DataTranslateEntry.dtet == dtetRDIDToDepName) ) {
						int ColonPos = VarNameForMatch.find(":");
						if (ColonPos == -1) {
							cerr << "Error: RDID result is not formatted with even a single \":\". DID must be formatted as RecID::idx:g/v where idx is the index of the dep rec in the sentence\n";
							// this is really a parsing error not a data error so we return
							return;
						}
						RecID = stoi(VarNameForMatch.substr(0, ColonPos));
						VarNameForMatch = VarNameForMatch.substr(ColonPos+1);
						ColonPos = VarNameForMatch.find(":");
						if (ColonPos == -1) {
							cerr << "Error: RDID result is not formatted with its second \":\". DID must be formatted as RecID::idx:g/v where idx is the index of the dep rec in the sentence\n";
							// this is really a parsing error not a data error so we return
							return;
						}
						DID = stoi(VarNameForMatch.substr(0, ColonPos));
						string sDorG = VarNameForMatch.substr(ColonPos+1);
						if (sDorG == "d") {
							bGov = false;
						}
					}
					// Phase 2. Apply the link to another set of records
					if (	DataTranslateEntry.dtet == dtetRWIDToWord) {
						SSentenceRec SRec = SentenceRec[RecID];
						auto& WordRecs = SRec.OneWordRec;
						// for the following, should be using DataTranslateEntry.TargetTblMatchIdx 
						// and then loop through the records, till the return value matches WID
						// but for dep records, they are sorted by the index anyway.
						if (WID == 255) {
							// ..but 255 is the special case of root
							VarTbl[DataTranslateEntry.VarTblIdx] = "<na>";
						}
						else {
							if (WID >= WordRecs.size()) {
								//cerr << "Error: Word number stored from dep result is greater than the number of words in sentence\n";
								bAllGood = false;
								break;
							}
							auto& wrec = WordRecs[WID];
							bool bValid = true;
							VarTbl[DataTranslateEntry.VarTblIdx] 
								= GetRecFieldByIdx(	RecID, WID, wrec, 
													DataTranslateEntry.TargetTblOutputIdx, 
													bValid);
							if (!bValid) {
								bAllGood = false;
								break;						
							}
						}
					}
					else if (	DataTranslateEntry.dtet == dtetRWIDToRDID) {
						SSentenceRec SRec = SentenceRec[RecID];
						auto& DepRecs = SRec.Deps;
						int idid = -1; for (auto drec : DepRecs) { idid++;
							bool bValid = true;
							if (drec.Gov == WID) { 
#pragma messages("replace setting the last time by addding a new rVarTbl each time")								
								VarTbl[DataTranslateEntry.VarTblIdx] 
										= GetDepRecFieldByIdx(	RecID, idid, DepNames, drec, 
																CaffeGenData::FIELD_TYPE_GOV_RDID, 
																bValid);

							}
							if (drec.Dep == WID) { 
								VarTbl[DataTranslateEntry.VarTblIdx] 
										= GetDepRecFieldByIdx(	RecID, idid, DepNames, drec, 
																CaffeGenData::FIELD_TYPE_DEP_RDID, 
																bValid);

							}
						}
						
						
					}
					else if (	DataTranslateEntry.dtet == dtetRWIDToCoref) {
						if (	(RecID < 0) || (WID < 0 )
							||	(CorefRevTbl.size() <= RecID) || (CorefRevTbl[RecID].size() <= WID)) {
							bAllGood = false;
						}
						else if (CorefRevTbl[RecID][WID] == -1) {
							bAllGood = false;
						}
						else {
							int icrec = CorefRevTbl[RecID][WID];
							if (icrec < CorefList.size() && icrec > 0) {
								auto crec = CorefList[icrec];
								auto crecPrev = CorefList[icrec-1];
								if (crec.GovID != crecPrev.GovID) {
									bAllGood = false;
								}
								else {
//									auto crecGov = CorefList[crec.GovID];
//									cerr << "Found coref of gov: "
//										<< SentenceRec[crecGov.SentenceID].OneWordRec[crecGov.HeadWordId].Word 
//										<< " from "
//										<< SentenceRec[crecPrev.SentenceID].OneWordRec[crecPrev.HeadWordId].Word 
//										<< " and "
//										<< SentenceRec[crec.SentenceID].OneWordRec[crec.HeadWordId].Word 
//										<< endl;
									VarTbl[DataTranslateEntry.VarTblIdx] 
										=		(to_string(crecPrev.SentenceID) + ":" 
											+	to_string(crecPrev.HeadWordId)) ;
								}
							}
							else {
								bAllGood = false;
							}
						}
					}
					else if (	(DataTranslateEntry.dtet == dtetRDIDToDepRWID) 
							||	(DataTranslateEntry.dtet == dtetRDIDToGovRWID) 
							||	(DataTranslateEntry.dtet == dtetRDIDToDepName) ){
						if ((RecID < 0) ||  (RecID > SentenceRec.size())) {
							bAllGood = false;
						}
						else {
							SSentenceRec SRec = SentenceRec[RecID];
							auto& DepRecs = SRec.Deps;
							if ((DID < 0) ||  (DID > DepRecs.size())) {
								bAllGood = false;
							}
							else {
								auto& drec = DepRecs[DID];
								switch (DataTranslateEntry.dtet) {
									case dtetRDIDToDepRWID:
										VarTbl[DataTranslateEntry.VarTblIdx] 
											=		(to_string(RecID) + ":" 
												+	to_string((int)(drec.Dep))) ;
										break;
									case dtetRDIDToGovRWID:
										VarTbl[DataTranslateEntry.VarTblIdx] 
											=		(to_string(RecID) + ":" 
												+	to_string((int)(drec.Gov))) ;
										break;
									case dtetRDIDToDepName:
										VarTbl[DataTranslateEntry.VarTblIdx] =
											(DepNames[drec.iDep] + (bGov ? ":g" : ":d"));
										break;
								}
								
							}
						}
					}

				}
				//add all the other dtet options
				if (bAllGood) {
					VarTblsForGoTranslated.push_back(VarTbl);
				}

			}
			VarTblsForGo.clear();
			VarTblsForGo = VarTblsForGoTranslated;
		}
		if (DataFilterTbl.size() > 0) {
			vector<vector<string> > VarTblsForGoTranslated; 
			for (auto& VarTbl : VarTblsForGo) {
				bool bAllGood = true;
				for (auto& DataFilter :  DataFilterTbl) {
					string FieldVal = VarTbl[DataFilter.first];
					if (FieldVal == DataFilter.second) {
						VarTblsForGoTranslated.push_back(VarTbl);
					}
				}
			}
			
			VarTblsForGo.clear();
			VarTblsForGo = VarTblsForGoTranslated;
		}
		for (auto& VarTbl : VarTblsForGo) {
			bool bAllFieldsFound = true;
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

