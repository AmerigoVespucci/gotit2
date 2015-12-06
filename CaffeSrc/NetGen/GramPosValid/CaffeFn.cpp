// CaffeFn.cpp : 
// /home/abba/NetBeansProjects/gotit2/CaffeSrc/NetGen/GramPosValid
// Saving ngrams as doubled GloVe vectors with pos
// control is auto net gen instead of prototxt
//


 
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "../../../stdafx.h"

#include "../../../MascReader.h"
#include "H5Cpp.h"

#include "../../../../CaffeR/GenSeed.pb.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif


#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif

void CGotitEnv::CaffeFn()
{
    string H5TrainFileName=		"/devlink/caffe/data/NetGen/GramPosValid/data/train.h5";
    string H5TestFileName=		"/devlink/caffe/data/NetGen/GramPosValid/data/test.h5";
    string H5TrainListFileName=	"/devlink/caffe/data/NetGen/GramPosValid/data/train_list.txt";
    string H5TestListFileName=	"/devlink/caffe/data/NetGen/GramPosValid/data/test_list.txt";
	string ProtoFileName=		"/devlink/caffe/data/NetGen/GramPosValid/models/train.prototxt";
	string ModelFileName=		"/devlink/caffe/data/NetGen/GramPosValid/models/g_best.caffemodel";
	string ConfigFileName=		"/devlink/caffe/data/NetGen/GramPosValid/data/config.prototxt";
	string WordListFileName=	"/devlink/caffe/data/NetGen/GramPosValid/data/WordList.txt";
	string WordVecFileName=		"/ExtDownloads/glove.6B.50d.txt" ;

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
	vector<int> GramsFound(cNumWords, 0); 
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
	for (NameItem ni : PosCount) {
		PosMap[ni.Name] = PosMap.size();
	}
	PosMap["<na>"] = PosMap.size();



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
					cerr << "no vec found for " <<RevWordMap[iwv] << std::endl;
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
							strNames << std::endl;
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
	
	int NumWordsClean = RevWordMapClean.size();
	int iNA = NumWordsClean - 1;
	int iPosNA = PosMap.size() - 1; 
	 
	for (SSentenceRec Rec : SentenceRec) {
		auto& WordRecs = Rec.OneWordRec;
		if (Rec.OneWordRec.size() < cMinRealLength) {
			continue;
		}
		for (int iCenter = 0; iCenter < WordRecs.size(); iCenter++) {
			bool bGramOK = true;
			vector<int> WordIList;
			vector<int> PosIList;
			for (int iw = iCenter - cCenterGram; iw <= iCenter + cCenterGram; iw++) {
				int WordI, PosI;
				if (iw < 0 || iw >= WordRecs.size()) {
					WordI = iNA; // we represent no word by the last index
					PosI = iPosNA;
					WordIList.push_back(WordI);
				}
				else {
					string w = WordRecs[iw].Word;
					string p = WordRecs[iw].POS;
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
						bGramOK = false;
						break;
					}
					map<string, int>::iterator itw = WordMapClean.find(w);
					if (itw == WordMapClean.end()) {
						bGramOK = false;
						break;
					}
					WordI = itw->second;
					WordIList.push_back(WordI);

					map<string, int>::iterator itp = PosMap.find(p);
					if (itp == PosMap.end()) {
						cerr << "pos index not found. Should not happen\n";
						bGramOK = false;
						break;
					}
					PosI = itp->second;
					PosIList.push_back(PosI);
					
				}
			} // end loop over word of gram
			if (bGramOK) {
				if (GramsFound[WordIList[cCenterGram]] >= cMaxGramsPerWord) {
					bGramOK = false;
				}
			}
			if (bGramOK) {
				GramsFound[WordIList[cCenterGram]]++;
				GramRecs.push_back(make_tuple(rand(), WordIList, true, PosIList));
					// add an invalid record. Only for init
				int ReplaceI = rand() % (NumWordsClean-1); // don't include <na> I think
				if (ReplaceI >= WordIList[cCenterGram]) {
					ReplaceI++;
				}
				// The main point of this test is that we're not changing PosI
				WordIList[cMinRealLength-1] = ReplaceI;
				GramRecs.push_back(make_tuple(rand(), WordIList, false, PosIList));
			}
			
		} // end loop over grams of sentence rec
	} // end loop over sentence recs

	// use the random int in the first field to sort
	sort(GramRecs.begin(), GramRecs.end());
	
	int NumRecords = GramRecs.size() / 2;
	int NumLabelVals = 1;
	NumItemsPerRec = (NumValsPerVec + PosMap.size()) * cNOfNGrams;

	for (auto Gram : GramRecs) {
		auto WordIVec = get<1>(Gram);
		for (int wi : WordIVec) {
			string& w = RevWordMapClean[wi];
			cout << w << " ";			
		}
		cout << "(" << (get<2>(Gram) ? "OK" : "Bad") << ")\n";
	}
	cout << "Found " << NumRecords << " records/ngrams\n";

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
				vector<int>& WordIList = get<1>(GramRecs[ir]);
				vector<int>& PosIList = get<3>(GramRecs[ir]);
				for (int in = 0; in < cNOfNGrams; in++) {
					int wi = WordIList[in];
					int pi = PosIList[in];
					vector<float>& OneVec = WordsVecsClean[wi];
					for (int ia = 0; ia < NumValsPerVec; ia++) {
						*ppd++ = OneVec[ia]; 
					}
					for (int ip = 0; ip < PosMap.size(); ip++) {
						*ppd++ = ((ip == pi) ? 1.0f : 0.0f);
					}
				}
			}
		}
		

		H5File h5file(H5FileName, H5F_ACC_TRUNC);
		DataSpace dataspace(cDataRank, dims);

		DataSet dataset = h5file.createDataSet(DATASET_NAMES, PredType::IEEE_F32LE, dataspace);

		dataset.write(pDataSet, PredType::IEEE_F32LE);

		delete pDataSet;

		for (int ir=0; ir < NumRecords * 2; ir++) {
			if ((ir % 2) == iBoth) {
				*ppl++ = (get<2>(GramRecs[ir]) ? 1.0f : 0.0f); 
			}
		}
		
//		cerr << Count[0] << " zeroes vs " << Count[1] << " 1s.\n";

		DataSpace labelspace(cLabelRank, dimsLabel);

		DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

		labelset.write(plabels, PredType::IEEE_F32LE);

		delete plabels;

		{
			ofstream test_list(H5TestListFileName);
			if (test_list.is_open()) {
				test_list << H5TestFileName << endl;
			}
			ofstream train_list(H5TrainListFileName);
			if (train_list.is_open()) {
				train_list << H5TrainFileName << endl;
			}
		}
		ofstream config_ofs(ConfigFileName);
		google::protobuf::io::OstreamOutputStream* config_output 
			= new google::protobuf::io::OstreamOutputStream(&config_ofs);
//		int fd = open(ConfigFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
//		google::protobuf::io::FileOutputStream* output = new google::protobuf::io::FileOutputStream(fd);
		ofstream f_config(ConfigFileName);
		if (f_config.is_open()) {
			CaffeGenSeed config;
			config.set_test_list_file_name(H5TestListFileName);
			config.set_train_list_file_name(H5TrainListFileName);
			config.set_num_test_cases(NumRecords);
			config.set_net_end_type(CaffeGenSeed::END_VALID);
			config.set_num_output_nodes(2);
			config.set_model_file_name(ModelFileName);
			config.set_proto_file_name(ProtoFileName);
			google::protobuf::TextFormat::Print(config, config_output);
			delete config_output;
			//close(fd);

			//config.SerializeToOstream(&f_config);
		}
		
	}
}

