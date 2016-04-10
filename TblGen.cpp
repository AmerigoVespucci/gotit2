// TblGen.cpp : 
// Takes a GenTbl proto file and produces a table to be used by CaffeFn
//


 
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "stdafx.h"

#include "MascReader.h"
#include "H5Cpp.h"

#include "GenTbl.pb.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif


#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif


void CGotitEnv::TblGenInit() 
{
}
void CGotitEnv::TblGen()
{

	CaffeGenTbl gen_tbl;
	
	string sModelProtoName;
	if (!GetImplemParam(sModelProtoName, "Implem.Param.FnParam.TblGen.ModelProtoName")) {
		cerr << "TblGen cannot be called without setting Implem.Param.FnParam.TblGen.ModelProtoName \n";
		return;
	}
	ifstream proto_ifs(sModelProtoName);
	if (proto_ifs.is_open()) {
		google::protobuf::io::IstreamInputStream* proto_input 
			= new google::protobuf::io::IstreamInputStream(&proto_ifs);
		google::protobuf::TextFormat::Parse(proto_input, &gen_tbl);
		delete proto_input;
	}

	const string& CoreDir = gen_tbl.files_core_dir();
	//WordListFileName
	string OutputFileName = CoreDir + "OutputVec.txt";
	
	const bool cbDoubleWordVecs = true; // false for autoencode thin vector
	const int cNumValsPerVecSrc = 50; // 10 - size of thin vector
	int NumValsPerVec = cNumValsPerVecSrc * (cbDoubleWordVecs ? 2 : 1);

	int NumWords = -1; 
	bool bAlphaOnly = true;
	string InputVecFileName = "invalid";
	bool bOneHot = false;

	if (gen_tbl.tbl_type() == CaffeGenTbl::TBL_TYPE_WORD) {
		if (	!gen_tbl.has_alpha_only() || !gen_tbl.has_input_vec_file_name() 
			||	!gen_tbl.has_input_vec_file_name() ) {
			cerr <<	"Error in config file. For WORD type tables, alpha_only, "
					"has_input_vec_file_name and has_input_vec_file_name must be included.\n";
			return;
		}
		
		NumWords = gen_tbl.max_words(); //2047;
		bAlphaOnly = gen_tbl.alpha_only();
		InputVecFileName = gen_tbl.input_vec_file_name() ;
	}
	else {
		if (!gen_tbl.has_one_hot()) {
			cerr << "Error in config file. For tbl types that are not WORD, one_hot must be included\n";
			return;
		}
		bOneHot = gen_tbl.one_hot();
	}

	vector<string> OutputNames;
	vector<vector<float> > OutputVecs; 
	
	if (gen_tbl.tbl_type() == CaffeGenTbl::TBL_TYPE_WORD) {
		vector<NameItem >& WordsInOrder = BasicTypeLists["word"];

		map<string, int> WordMap;
		vector<string> RevWordMap;
	//	vector<int> GramsFound(cNumWords, 0); 
		vector<vector<float> > WordsVecs(NumWords+1); // indexed by the same as WordMap

		map<string, int> WordMapClean;

		const int cMulSoThereAreEnough = 3;
		const vector<string> AlphaExceptions = {".", ";", ",", "\"", "'", "''", "1", "2", "3", "4", "0"};


		int NumWordsToSearch = min(NumWords* cMulSoThereAreEnough, (int)WordsInOrder.size() ); 
		for (int iw=0; iw<NumWordsToSearch; iw++) { 
			string w = WordsInOrder[iw].Name;
			bool bGood = true;
			if (bAlphaOnly) {
				for (auto c : w) {
					//if (!isalpha(c)) {
					if (c < 'a' || c > 'z') {
						bGood = false;
						break;
					}
				}
				if (!bGood) {
					for (int ie = 0; ie < AlphaExceptions.size(); ie++) {
						if (w == AlphaExceptions[ie]) {
							bGood = true;
							break;
						}
					}
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
				if (MapSize >= NumWords-1) {
					break;
				}
			}
		}
		{
			int MapSize = WordMap.size(); // saved to avoid ambiguity on next line
			WordMap["<na>"] = MapSize;
			RevWordMap.push_back("<na>");
		}


		const int cVocabLimit = 400000;
		{
			ifstream VecFile(InputVecFileName);
			if (!VecFile.is_open()) {
				cerr	<< "Error. Parameter in config file: input_vec_file_name: " 
						<< InputVecFileName << " not found.\n";
				return;
			}
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
				int iw = NumWords; // default to <na>
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

			for (int iwv = 0; iwv < WordsVecs.size(); iwv++) {
				vector<float>& OneVec = WordsVecs[iwv];
				if (OneVec.size() == NumValsPerVec) {
					WordMapClean[RevWordMap[iwv]] = OutputVecs.size();
					OutputVecs.push_back(OneVec);
					OutputNames.push_back(RevWordMap[iwv]);
				}
				else {
					if (RevWordMap[iwv] == "<na>") {
						WordMapClean["<na>"] =  OutputVecs.size();
						vector<float> NullVec(NumValsPerVec, 0.0);
						OutputVecs.push_back(NullVec);
						OutputNames.push_back("<na>");
					}
					else {
						cerr << "no vec found for " <<RevWordMap[iwv] << endl;
					}


				}
			}
			if (cbDoubleWordVecs) { // not sure why we don't always output. revisit
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
	} // end if WORD type tbl
	else if (gen_tbl.tbl_type() == CaffeGenTbl::TBL_TYPE_POS) {
		vector<NameItem>& PosCount = BasicTypeLists["POSCount"];
		map<string, int> PosMap;
		int ini = -1;
		for (NameItem ni : PosCount) {
			ini++;
			PosMap[ni.Name] = ini;
		}
		ini++;
		PosMap["<na>"] = ini;

		int PosMapSize = PosMap.size();
		OutputNames.resize(PosMapSize);
		if (bOneHot) {
			OutputVecs.resize(PosMapSize, vector<float>(PosMapSize, 0.0f));
		}
		else {
			OutputVecs.resize(PosMapSize, vector<float>(1, 0.0f));
		}
//		OutputNames.clear();
//		OutputVecs.clear();
		for (auto PosRec : PosMap) {
			int ipr = PosRec.second;
			OutputNames[ipr] = PosRec.first;
			if (bOneHot) {
				OutputVecs[ipr][ipr] = 1.0f;
			}
			else {
				OutputVecs[ipr][0] = (float)ipr;
			}
		}
	}
	else if (gen_tbl.tbl_type() == CaffeGenTbl::TBL_TYPE_DEP) {
		//int NumDepTypes = BasicTypeLists["AllDepList"].size();"/ExtDownloads/glove.6B.50d.txt"
		int NumDepTypes = DepTypes.size();

//		vector<string> DepNames(DepTypes.size());
//		for (auto deprec : DepTypes) {
//			DepNames[deprec.second] = deprec.first;
//		}

		OutputNames.resize(NumDepTypes);
		if (bOneHot) {
			OutputVecs.resize(NumDepTypes, vector<float>(NumDepTypes, 0.0f));
		}
		else {
			OutputVecs.resize(NumDepTypes, vector<float>(1, 0.0f));
		}

		//map<string, int> DepMap;
		vector<vector<float> > DepVecTbl(NumDepTypes, vector<float>(NumDepTypes, 0.0f));
		vector<vector<float> > DepNumTbl;
		//for (int idt = 0; idt < NumDepTypes; idt++) {
		for (auto deprec : DepTypes) {
			//DepMap[to_string(idt)] = idt;
			int idt = deprec.second;
			OutputNames[idt] = deprec.first;
			if (bOneHot) {
				OutputVecs[idt][idt] = 1.0f;
			}
			else {
				OutputVecs[idt][0] = (float)idt;
			}
		}
	}

//		vector<vector<float> > YesNoTbl;
//		YesNoTbl.push_back(vector<float>(1, 0.0f));
//		YesNoTbl.push_back(vector<float>(1, 1.0f));

	ofstream strNames(OutputFileName);
	if (strNames.is_open()) {
		strNames << OutputNames.size() << endl;
		strNames << OutputVecs[0].size() << endl;
		int iw = -1;
		for (auto w : OutputNames) { iw++;
			strNames << w << " ";
			//int iw = WordMapClean[w];
			auto OneVec = OutputVecs[iw];
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
	
		/*
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
	TranslateTblNameMap["DepVecTbl"] = TranslateTblPtrs.size();
	TranslateTblPtrs.push_back(&DepTypes);
	VecTblPtrs.push_back(&DepVecTbl);
	TranslateTblNameMap["DepNumTbl"] = TranslateTblPtrs.size();
	TranslateTblPtrs.push_back(&DepTypes);
	VecTblPtrs.push_back(&DepNumTbl);
	// each pair in the table: first - idx of rec to access. second: idx of VarTbl to put it into
		 * */
}

