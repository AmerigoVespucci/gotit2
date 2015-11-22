// ForLearn.cpp : Parses the output of Standford CoreNLP							
//

#include "stdafx.h"

#include "MascReader.h"
#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif


#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif

enum tMode {
	SM_WORD_PHRASES,
	SM_WORDS,
	SM_POS,
	SM_AE_WORDS_VECS,
	SM_WORDS_TO_POS,
	SM_EMBED_INIT,
	SM_EMBED_VEC_PREDICT,
	SM_GRAM_VALID,
	SM_GRAM_POS_VALID,
	SM_NONE
};
tMode itMode = SM_EMBED_VEC_PREDICT;

const int cAsciiMax = '~';
const int cAsciiMin = ' ';
const int cAsciiRange = cAsciiMax - cAsciiMin;
const int cLSTMBatchSize = 32;

void CGotitEnv::SaveSimple()
{
	string H5FileName;
	if (!GetImplemParam(H5FileName, "Task.Param.SaveSimple.H5FileName")) {
		cerr << "SaveSimple Error: No mod name provided\n";
		return;
	}
	string sMode;
	if (!GetImplemParam(sMode, "Task.Param.SaveSimple.Mode")) {
		cerr << "SaveSimple Error: Mode must be provided\n";
		return;
	}
	if (sMode == "words") {
		itMode = SM_WORDS;
	}
	else 	if (sMode == "word phrases") {
		itMode = SM_WORD_PHRASES;
	}
	else if (sMode == "POS") {
		itMode = SM_POS;
	}
	else if (sMode == "words to pos") {
		itMode = SM_WORDS_TO_POS;
	}
	else {
		cerr << "SaveSimple Error: Mode provided not one of the options\n";
		return;
	}

	
	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	int NumLabelsPerRec = 1;
	int NumItemsPerRec = cAsciiRange;

	
	int NumRecsFound = 0;
	vector<pair<int, bool> > CharRecs;
	
	
	if (itMode == SM_WORDS) {
		const int cNumWords = 300;
		int NumWords = 0;
		for (int iw=0; iw<cNumWords* 100; iw++) {
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
				bool bFirstOfRec = true;
				cerr << w << endl;
				for (auto c : w) {
					int wci = (int)c - cAsciiMin;
					wci = min(max(0, wci), cAsciiRange);
					CharRecs.push_back(make_pair(wci, bFirstOfRec));
					bFirstOfRec = false;
				}
				CharRecs.push_back(make_pair('.' - cAsciiMin, false));
				NumWords++;
			}
			if (NumWords >= cNumWords) {
				break;
			}
		}
	}
	else if (itMode == SM_WORD_PHRASES) {
		for (SSentenceRec Rec : SentenceRec) {
			bool bRecOK = true;
			for (uint iw = 0; iw < Rec.OneWordRec.size() ; iw++) {
				WordRec wrec = Rec.OneWordRec[iw];
				string W = wrec.Word;
				bool bCap = wrec.bCap;
				bool bFound = false;
				for (int im = 0; im < 1000; im++) {
					if (WordsInOrder[im].Name == W) {
						bFound = true;
						break;
					}
				}
				if (!bFound) {
					bRecOK = false;
					break;
				}
			}
			if (bRecOK) {
				NumRecsFound++;
				bool bFirstOfRec = true;
				for (WordRec wrec : Rec.OneWordRec) {
					string W = wrec.Word;
					bool bCap = wrec.bCap;
					if (bCap) {
						W[0] = toupper(W[0]);
					}
					//ModFile << W << " ";
					for (char wc : W) {
						int wci = (int)wc - cAsciiMin;
						wci = min(max(0, wci), cAsciiRange);
						CharRecs.push_back(make_pair(wci, bFirstOfRec));
						bFirstOfRec = false;
					}
					CharRecs.push_back(make_pair((int)' ' - cAsciiMin, bFirstOfRec));
					bFirstOfRec = false;
				}
				//ModFile << endl;
			}

		}

		cout << "found " << NumRecsFound << endl;
	}
	else if (itMode == SM_POS) {
		const int cNumPOSTypes = 20;
		vector<NameItem>& PosCount = BasicTypeLists["POSCount"];
		//NumItemsPerRec = PosCount.size() + 1; // add 1 for EOR
		NumItemsPerRec = cNumPOSTypes + 1; // add 1 for EOR
		map<string, int> PosMap;
		for (NameItem ni : PosCount) {
			PosMap[ni.Name] = PosMap.size();
		}
		int NumRecsFound = 0;
		for (SSentenceRec Rec : SentenceRec) {
			if (Rec.OneWordRec.size() < 4) {
				continue;
			}
			bool bRecOK = true;
			bool bFirstOfRec = true;
			for (WordRec wrec : Rec.OneWordRec) {			
				//string W = wrec.Word;
				string POS = wrec.POS;
				int POSpos = PosMap[POS];
				if (POSpos >= cNumPOSTypes) {
					bRecOK = false;
					break;
				}
			}
			if (bRecOK) {
				NumRecsFound++;
				bool bFirstOfRec = true;
				for (WordRec wrec : Rec.OneWordRec) {
					string POS = wrec.POS;
					CharRecs.push_back(make_pair(PosMap[POS], bFirstOfRec));
					bFirstOfRec = false;
					cout << wrec.Word << " ";
				}
				cout << endl;
				CharRecs.push_back(make_pair(cNumPOSTypes, false));
			}
		}
		cout << "found " << NumRecsFound << endl;
	}

	int NumRecords = CharRecs.size() / cLSTMBatchSize * cLSTMBatchSize; // round down
	
	const H5std_string	DATASET_NAME("data");
	const H5std_string	DELTASET_NAME("delta");
	const H5std_string	LABELSET_NAME("label");
	const int cDataRank =  3;
	hsize_t dims[cDataRank];               // dataset dimensions
	dims[2] = NumItemsPerRec;
	dims[1] = 1;
	//dims[1] = cLSTMBatchSize;
	dims[0] = NumRecords;
	const int cDeltaRank =  2;
	hsize_t dimsDelta[cDataRank];               // dataset dimensions
	dims[1] = 1;
	//dims[1] = cLSTMBatchSize;
	dims[0] = NumRecords;
	const int cLabelRank = 2;
	hsize_t dimsLabel[cLabelRank];               // dataset dimensions
	dimsLabel[1] = 1;
	//dimsLabel[1] = cLSTMBatchSize * 1;
	dimsLabel[0] = NumRecords;
	
	int DataSetSize = NumRecords * NumItemsPerRec;
	int DeltaSetSize = NumRecords  ;
	int LabelSetSize = NumRecords;

	float * pDataSet = new float[DataSetSize];
	float * ppd = pDataSet;
	float * pDeltaSet = new float[DeltaSetSize];
	float * ppt = pDeltaSet;
	float * plabels = new float[LabelSetSize];
	float * ppl = plabels;

	for (int ir=0; ir < NumRecords; ir++) {
		int c = CharRecs[ir].first;
		for (int ia = 0; ia < NumItemsPerRec; ia++) {
			*ppd++ = (((int)c == ia) ? 1.0f : 0.0f); 
		}
		if (ir > 0) {
			*ppl++ = (float)c; 
		}
		*ppt++ = ((CharRecs[ir].second) ? 0.0f : 1.0f);
	}

	H5File h5file(H5FileName, H5F_ACC_TRUNC);
	DataSpace dataspace(cDataRank, dims);
	DataSpace deltaspace(cDeltaRank, dims);
	DataSpace labelspace(cLabelRank, dimsLabel);

	DataSet dataset = h5file.createDataSet(DATASET_NAME, PredType::IEEE_F32LE, dataspace);

	dataset.write(pDataSet, PredType::IEEE_F32LE);

	DataSet deltaset = h5file.createDataSet(DELTASET_NAME, PredType::IEEE_F32LE, deltaspace);

	deltaset.write(pDeltaSet, PredType::IEEE_F32LE);

	DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

	labelset.write(plabels, PredType::IEEE_F32LE);
	

	delete pDataSet;
	delete pDeltaSet;
	delete plabels;
}
 
void CGotitEnv::SaveNGrams()
{
#include "SaveNGramPosValid.cpp"
	return;
	string sMode;
	if (!GetImplemParam(sMode, "Task.Param.SaveNGrams.Mode")) {
		cerr << "SaveSimple Error: Mode must be provided\n";
		return;
	}
	if (sMode == "init") {
		itMode = SM_EMBED_INIT;
	}
	else 	if (sMode == "vec predict") {
		itMode = SM_EMBED_VEC_PREDICT;
	}
	else 	if (sMode == "gram valid") {
		itMode = SM_GRAM_VALID;
	}
	else if (sMode == "gram pos valid") {
		itMode = SM_GRAM_POS_VALID;
	}
	else {
		cerr << "SaveNGrams Error: Mode provided not one of the options\n";
		return;
	}

	string H5TrainFileName;
	if (!GetImplemParam(H5TrainFileName, "Task.Param.SaveNGrams.H5TrainFileName")) {
		cerr << "SaveSimple Error: No train mod name provided\n";
		return;
	}
	string H5TestFileName;
	if (!GetImplemParam(H5TestFileName, "Task.Param.SaveNGrams.H5TestFileName")) {
		cerr << "SaveSimple Error: No test mod name provided\n";
		return;
	}
	string WordListFileName;
	if (!GetImplemParam(WordListFileName, "Task.Param.SaveNGrams.WordListFileName")) {
		cerr << "SaveSimple Error: No word list file name provided\n";
		return;
	}
	string WordVecFileName;
	if (itMode == SM_EMBED_VEC_PREDICT || itMode == SM_GRAM_VALID || itMode == SM_GRAM_POS_VALID) {
		if (!GetImplemParam(WordVecFileName, "Task.Param.SaveNGrams.WordVecFileName")) {
			cerr << "SaveSimple Error: No word vector file name provided\n";
			return;
		}
	}
	
	
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
	// tuple: rand num for sort, 5 indexes of words, bool if valid. 
	vector<tuple<int, vector<int>, bool> > GramRecs; 
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
	if (itMode == SM_EMBED_INIT) {
		ofstream strNames(WordListFileName);
		if (strNames.is_open()) {
			for (auto w : RevWordMap) {
				strNames << w << endl;
			}
		}
	}
	
	const int cVocabLimit = 400000;
	if (itMode == SM_EMBED_VEC_PREDICT || itMode == SM_GRAM_VALID || itMode == SM_GRAM_POS_VALID) {
		ifstream VecFile(WordVecFileName);
		if (VecFile.is_open()) {
			string ln;
			//VecFile >> NumValsPerVec;
			//while (!VecFile.eof()) {
			for (int ic = 0; ic < cVocabLimit; ic++) {
				getline(VecFile, ln, ' ');
				string w;
				//VecFile >> w;
				w = ln;
				if (w.size() == 0) {
					break;
				}
				int iw = cNumWords; // default to <na>
				bool bAddVec = false;
//				if (w != "<na>") {
					auto itWordMap = WordMap.find(w);
					if (itWordMap != WordMap.end()) {
						bAddVec = true;
						iw = itWordMap->second;
//						cerr << "Error in word vector file. Word found that is not in the word map\n";
//						return;
//					}
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
					//VecFile >> wv;
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
	
	int NumWordsClean = RevWordMapClean.size();
	int iNA = NumWordsClean - 1;
	 
	for (SSentenceRec Rec : SentenceRec) {
		auto& WordRecs = Rec.OneWordRec;
		if (Rec.OneWordRec.size() < cMinRealLength) {
			continue;
		}
		for (int iCenter = 0; iCenter < WordRecs.size(); iCenter++) {
			bool bGramOK = true;
			vector<int> WordIList;
			for (int iw = iCenter - cCenterGram; iw <= iCenter + cCenterGram; iw++) {
				int WordI;
				if (iw < 0 || iw >= WordRecs.size()) {
					WordI = iNA; // we represent no word by the last index
					WordIList.push_back(WordI);
				}
				else {
					string w = WordRecs[iw].Word;
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
				}
			} // end loop over word of gram
			if (bGramOK) {
				if (GramsFound[WordIList[cCenterGram]] >= cMaxGramsPerWord) {
					bGramOK = false;
				}
			}
			if (bGramOK) {
				GramsFound[WordIList[cCenterGram]]++;
				GramRecs.push_back(make_tuple(rand(), WordIList, true));
				if (itMode == SM_EMBED_INIT || itMode == SM_GRAM_VALID || itMode == SM_GRAM_POS_VALID) {
					// add an invalid record. Only for init
					int ReplaceI = rand() % (NumWordsClean-1); // don't include <na> I think
					if (ReplaceI >= WordIList[cCenterGram]) {
						ReplaceI++;
					}
					WordIList[cMinRealLength-1] = ReplaceI;
					GramRecs.push_back(make_tuple(rand(), WordIList, false));
				}
			}
			
		} // end loop over grams of sentence rec
	} // end loop over sentence recs

	// use the random int in the first field to sort
	sort(GramRecs.begin(), GramRecs.end());
	
	int NumRecords = GramRecs.size() / 2;
	int NumLabelVals = 1;
	int NumDataSets = cNOfNGrams;
	if (itMode == SM_EMBED_VEC_PREDICT) {
		NumItemsPerRec = NumValsPerVec * (cNOfNGrams-1);
		//NumRecords = GramRecs.size();
		NumLabelVals = NumValsPerVec ;
		NumDataSets = 1;
	}
	else if (itMode == SM_GRAM_VALID) {
		NumItemsPerRec = NumValsPerVec * cNOfNGrams;
		NumLabelVals = 1;
		NumDataSets = 1;
	}
	else if (itMode == SM_GRAM_POS_VALID) {
#pragma message("Change this")			
		//NumItemsPerRec = (NumValsPerVec + NumPosVals) * cNOfNGrams ;
	}
	for (auto Gram : GramRecs) {
		auto WordIVec = get<1>(Gram);
		for (int wi : WordIVec) {
			string& w = RevWordMapClean[wi];
			cout << w << " ";			
		}
		if (itMode == SM_EMBED_INIT || itMode == SM_GRAM_VALID || itMode == SM_GRAM_POS_VALID) {
			cout << "(" << (get<2>(Gram) ? "OK" : "Bad") << ")\n";
		}
		else {
			cout << endl;
		}
	}
	cout << "Found " << NumRecords << " records/ngrams\n";

	string H5FileName = H5TrainFileName;
	
	for (int iBoth = 0; iBoth < 2; iBoth++ ) {
		if (iBoth == 1) {
			H5FileName = H5TestFileName;
		}
		H5std_string	DATASET_NAMES[cNOfNGrams]; // ("data1");
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

		float * pDataSet[NumDataSets];// = new float[DataSetSize];
		float * plabels = new float[LabelSetSize];
		float * ppl = plabels;

		if (itMode == SM_EMBED_INIT) {
			for (int in = 0; in < cNOfNGrams; in++) {
				DATASET_NAMES[in] = string("data") + to_string(in);
				pDataSet[in] = new float[DataSetSize];
				float * ppd = pDataSet[in];
				for (int ir=0; ir < NumRecords * 2; ir++) {
					if ((ir % 2) == iBoth) {
						vector<int>& GramRec = get<1>(GramRecs[ir]);
						int wi = GramRec[in];
						for (int ia = 0; ia < NumItemsPerRec; ia++) {
							*ppd++ = ((wi == ia) ? 1.0f : 0.0f); 
						}
					}
				}
			}
		}
		else if (itMode == SM_EMBED_VEC_PREDICT) {
			DATASET_NAMES[0] = "data";
			pDataSet[0] = new float[DataSetSize];
			float * ppd = pDataSet[0];
			for (int ir=0; ir < NumRecords * 2; ir++) {
				if ((ir % 2) == iBoth) {
					vector<int>& GramRec = get<1>(GramRecs[ir]);
					for (int in = 0; in < cNOfNGrams; in++) {
						if (in == cCenterGram) {
							continue;
						}
						int wi = GramRec[in];
						vector<float>& OneVec = WordsVecsClean[wi];
						for (int ia = 0; ia < NumValsPerVec; ia++) {
							*ppd++ = OneVec[ia]; 
						}
					}
				}
			}			
		}
		else if (itMode == SM_GRAM_VALID) {
			DATASET_NAMES[0] = "data";
			pDataSet[0] = new float[DataSetSize];
			float * ppd = pDataSet[0];
			for (int ir=0; ir < NumRecords * 2; ir++) {
				if ((ir % 2) == iBoth) {
					vector<int>& GramRec = get<1>(GramRecs[ir]);
					for (int in = 0; in < cNOfNGrams; in++) {
						int wi = GramRec[in];
						vector<float>& OneVec = WordsVecsClean[wi];
						for (int ia = 0; ia < NumValsPerVec; ia++) {
							*ppd++ = OneVec[ia]; 
						}
					}
				}
			}			
		}
		else if (itMode == SM_GRAM_POS_VALID) {
#pragma message("Change this")			
			DATASET_NAMES[0] = "data";
			pDataSet[0] = new float[DataSetSize];
			float * ppd = pDataSet[0];
			for (int ir=0; ir < NumRecords * 2; ir++) {
				if ((ir % 2) == iBoth) {
					vector<int>& GramRec = get<1>(GramRecs[ir]);
					for (int in = 0; in < cNOfNGrams; in++) {
						int wi = GramRec[in];
						vector<float>& OneVec = WordsVecsClean[wi];
						for (int ia = 0; ia < NumValsPerVec; ia++) {
							*ppd++ = OneVec[ia]; 
						}
					}
				}
			}			
		}
		

		H5File h5file(H5FileName, H5F_ACC_TRUNC);
		for (int in = 0; in < NumDataSets; in++) {
			DataSpace dataspace(cDataRank, dims);

			DataSet dataset = h5file.createDataSet(DATASET_NAMES[in], PredType::IEEE_F32LE, dataspace);

			dataset.write(pDataSet[in], PredType::IEEE_F32LE);

			delete pDataSet[in];
		}

//		int Count[2] = {0, 0};
		int cTestOutput = 49;
		for (int ir=0; ir < NumRecords * 2; ir++) {
			if ((ir % 2) == iBoth) {
				if (itMode == SM_EMBED_INIT || itMode == SM_GRAM_VALID || itMode == SM_GRAM_POS_VALID) {
					*ppl++ = (get<2>(GramRecs[ir]) ? 1.0f : 0.0f); 
				}
				else if (itMode == SM_EMBED_VEC_PREDICT) {
					vector<int>& GramRec = get<1>(GramRecs[ir]);
					int wi = GramRec[cCenterGram];
					vector<float>& OneVec = WordsVecsClean[wi];
//						*ppl++ = OneVec[cTestOutput]; 
//						if (OneVec[cTestOutput] == 0.0) {
//							Count[0]++;
//						}
//						else {
//							Count[1]++;							
//						}
					for (int ia = 0; ia < NumValsPerVec; ia++) {
						*ppl++ = OneVec[ia]; 
					}
				}				
			}
		}
		
//		cerr << Count[0] << " zeroes vs " << Count[1] << " 1s.\n";

		DataSpace labelspace(cLabelRank, dimsLabel);

		DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

		labelset.write(plabels, PredType::IEEE_F32LE);

		delete plabels;
	}
}

void CGotitEnv::SaveAEWordVec()
{
	string sMode;
	if (!GetImplemParam(sMode, "Task.Param.SaveAEWordVec.Mode")) {
		cerr << "SaveSimple Error: Mode must be provided\n";
		return;
	}
	
	if (sMode == "ae words vecs") {
		itMode = SM_AE_WORDS_VECS;
	}
	else {
		cerr << "SaveNGrams Error: Mode provided not one of the options\n";
		return;
	}

	string H5TrainFileName;
	if (!GetImplemParam(H5TrainFileName, "Task.Param.SaveAEWordVec.H5TrainFileName")) {
		cerr << "SaveSimple Error: No train mod name provided\n";
		return;
	}
	string H5TestFileName;
	if (!GetImplemParam(H5TestFileName, "Task.Param.SaveAEWordVec.H5TestFileName")) {
		cerr << "SaveSimple Error: No test mod name provided\n";
		return;
	}
	string WordListFileName;
	if (!GetImplemParam(WordListFileName, "Task.Param.SaveAEWordVec.WordListFileName")) {
		cerr << "SaveSimple Error: No word list file name provided\n";
		return;
	}
	string WordVecFileName;
	if (itMode == SM_WORDS_TO_POS) {
		if (!GetImplemParam(WordVecFileName, "Task.Param.SaveAEWordVec.WordVecFileName")) {
			cerr << "SaveSimple Error: No word vector file name provided\n";
			return;
		}
	}

	const int cNumWords = 16384;
	const int cVocabLimit = 400000; // how far down in file to check for words
	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	int NumItemsPerLabel = 1;
	const int cNumValsPerVecIn = 50;
	const int cNumValsPerVec = cNumValsPerVecIn * 2; 

	int NumRecsFound = 0;
	// tuple: rand num for sort, 5 indexes of words, bool if valid. 
	vector<tuple<int, vector<int>, bool> > GramRecs; 
	int NumWords = 0;
	map<string, int> WordMap;
	map<string, int> WordMapClean;
	vector<string> RevWordMap;
	vector<string> RevWordMapClean;
	vector<int> GramsFound(cNumWords, 0); 
	vector<vector<float> > WordsVecs(cNumWords); // indexed by the same as WordMap
	vector<vector<float> > WordsVecsClean; 
	vector<int> MarkupForClean;

	int NumValsPerVecSrc = 50;
	int NumValsPerVec = NumValsPerVecSrc * 2;

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

	if (itMode == SM_AE_WORDS_VECS) {
		ifstream VecFile(WordVecFileName);
		if (VecFile.is_open()) {
			string ln;
			for (int ic = 0; ic < cVocabLimit; ic++) {
				getline(VecFile, ln, ' ');
				string w;
				//VecFile >> w;
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
				for (int iwv = 0; iwv < NumValsPerVecSrc; iwv++) {
					if (iwv == NumValsPerVecSrc - 1) {
						getline(VecFile, ln);
					}
					else {
						getline(VecFile, ln, ' ');
					}
					float wv;
					//VecFile >> wv;
					wv = stof(ln);
					if (bAddVec) {
						OneVec.push_back((wv < -0.1f) ? 1.0f : 0.0f);
						OneVec.push_back((wv > 0.1f) ? 1.0f : 0.0f);
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
				cerr << "no vec found for " <<RevWordMap[iwv] << endl;

			}
		}
		WordMapClean["<na>"] =  WordsVecsClean.size();
		vector<float> NullVec(NumValsPerVec, 0.0);
		WordsVecsClean.push_back(NullVec);
		RevWordMapClean.push_back("<na>");
		
//		{
//			{
//				for (auto pwm : WordMap) {
//					if (pwm.second == iwv) {
//						cerr << "no vec found for " << pwm.first << endl;
//					}
//				}
//				int StartSize = OneVec.size();
//				for (int iwv = StartSize; iwv < NumValsPerVec; iwv++) {
//					OneVec.push_back(0.0);
//				}
//			}
//
//		}

		cout << "found " << WordsVecsClean.size() << endl;
	}
	
	int NumReHits = 0;
	map<vector<float>, int> StatsMap;
//	vector<float> NullVec(NumValsPerVec, 0.0);
//	StatsMap[NullVec] = -1;
	for (vector<float>& OneVec : WordsVecsClean) {
		auto it = StatsMap.find(OneVec);
		if (it == StatsMap.end()) {
			StatsMap[OneVec] = 1;
		}
		else {
			if (it->second != -1) {
				it->second++;
				NumReHits++;
			}
		}
	}
	
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

	
	cerr << NumReHits << " identical vectors out of " << WordsVecsClean.size() << endl;
	
	int NumRecords = WordsVecsClean.size() / 2;
	string H5FileName = H5TrainFileName;
	int NumLabelVals = NumValsPerVec;
	int NumItemsPerRec = NumValsPerVec;
	
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
				vector<float>& OneVec = WordsVecsClean[ir];
				for (int ia = 0; ia < NumValsPerVec; ia++) {
					*ppd++ = OneVec[ia]; 
				}
			}
		}			

		H5File h5file(H5FileName, H5F_ACC_TRUNC);
		DataSpace dataspace(cDataRank, dims);

		DataSet dataset = h5file.createDataSet(DATASET_NAMES, PredType::IEEE_F32LE, dataspace);

		dataset.write(pDataSet, PredType::IEEE_F32LE);

		delete pDataSet;

		// no label because ae uses its input to train output by definintion
	}
}

void CGotitEnv::SaveWordToPos()
{
	string sMode;
	if (!GetImplemParam(sMode, "Task.Param.SaveWordToPos.Mode")) {
		cerr << "SaveSimple Error: Mode must be provided\n";
		return;
	}
	
	if (sMode == "words to pos") {
		itMode = SM_WORDS_TO_POS;
	}
	else {
		cerr << "SaveNGrams Error: Mode provided not one of the options\n";
		return;
	}

	string H5TrainFileName;
	if (!GetImplemParam(H5TrainFileName, "Task.Param.SaveWordToPos.H5TrainFileName")) {
		cerr << "SaveSimple Error: No train mod name provided\n";
		return;
	}
	string H5TestFileName;
	if (!GetImplemParam(H5TestFileName, "Task.Param.SaveWordToPos.H5TestFileName")) {
		cerr << "SaveSimple Error: No test mod name provided\n";
		return;
	}
	string WordListFileName;
	if (!GetImplemParam(WordListFileName, "Task.Param.SaveWordToPos.WordListFileName")) {
		cerr << "SaveSimple Error: No word list file name provided\n";
		return;
	}
	string WordVecFileName;
	if (itMode == SM_WORDS_TO_POS) {
		if (!GetImplemParam(WordVecFileName, "Task.Param.SaveWordToPos.WordVecFileName")) {
			cerr << "SaveSimple Error: No word vector file name provided\n";
			return;
		}
	}

	const int cNumWords = 8192;
	const int cVocabLimit = 400000; // how far down in file to check for words
	vector<NameItem >& WordsInOrder = BasicTypeLists["word"];
	int NumItemsPerLabel = 1;
	const int cNumValsPerVecIn = 50;
	const int cNumValsPerVec = cNumValsPerVecIn * 2; 

	int NumRecsFound = 0;
	// tuple: rand num for sort, 5 indexes of words, bool if valid. 
	vector<tuple<int, vector<int>, bool> > GramRecs; 
	int NumWords = 0;
	map<string, int> WordMap;
	map<string, int> WordMapClean;
	vector<string> RevWordMap;
	vector<string> RevWordMapClean;
	vector<int> GramsFound(cNumWords, 0); 
	vector<vector<float> > WordsVecs(cNumWords); // indexed by the same as WordMap
	vector<vector<float> > WordsVecsClean; 
	vector<int> MarkupForClean;

	int NumValsPerVecSrc = 50;
	int NumValsPerVec = NumValsPerVecSrc * 2;

	for (int iw=0; iw<cNumWords* 100; iw++) { // factor 100, just to make sure we have enough
		string w = WordsInOrder[iw].Name;
		bool bGood = true;
		for (auto& c : w) {
			//if (!isalpha(c)) {
			if (c >= 'A' && c <= 'Z') {
				c = tolower(c);
				//bGood = false;
				//break;
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

	if (itMode == SM_WORDS_TO_POS) {
		ifstream VecFile(WordVecFileName);
		if (VecFile.is_open()) {
			string ln;
			for (int ic = 0; ic < cVocabLimit; ic++) {
				getline(VecFile, ln, ' ');
				string w;
				//VecFile >> w;
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
				for (int iwv = 0; iwv < NumValsPerVecSrc; iwv++) {
					if (iwv == NumValsPerVecSrc - 1) {
						getline(VecFile, ln);
					}
					else {
						getline(VecFile, ln, ' ');
					}
					float wv;
					//VecFile >> wv;
					wv = stof(ln);
					if (bAddVec) {
						OneVec.push_back((wv < -0.1f) ? 1.0f : 0.0f);
						OneVec.push_back((wv > 0.1f) ? 1.0f : 0.0f);
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
				cerr << "no vec found for " <<RevWordMap[iwv] << endl;

			}
		}
		WordMapClean["<na>"] =  WordsVecsClean.size();
		vector<float> NullVec(NumValsPerVec, 0.0);
		WordsVecsClean.push_back(NullVec);
		RevWordMapClean.push_back("<na>");
		
//		{
//			{
//				for (auto pwm : WordMap) {
//					if (pwm.second == iwv) {
//						cerr << "no vec found for " << pwm.first << endl;
//					}
//				}
//				int StartSize = OneVec.size();
//				for (int iwv = StartSize; iwv < NumValsPerVec; iwv++) {
//					OneVec.push_back(0.0);
//				}
//			}
//
//		}

		cout << "found " << WordsVecsClean.size() << endl;
	}

	// collect statistics on how many words have identical vectors
	int NumReHits = 0;
	map<vector<float>, int> StatsMap;
//	vector<float> NullVec(NumValsPerVec, 0.0);
//	StatsMap[NullVec] = -1;
	for (vector<float>& OneVec : WordsVecsClean) {
		auto it = StatsMap.find(OneVec);
		if (it == StatsMap.end()) {
			StatsMap[OneVec] = 1;
		}
		else {
			if (it->second != -1) {
				it->second++;
				NumReHits++;
			}
		}
	}
	
	cerr << NumReHits << " identical vectors out of " << WordsVecsClean.size() << endl;

	// save word vectors out to file so that the caffe module can input them to the net in test mode
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
	
	vector<NameItem>& PosCount = BasicTypeLists["POSCount"];
	map<string, int> PosMap;
	for (NameItem ni : PosCount) {
		PosMap[ni.Name] = PosMap.size();
	}

	vector<pair<int, int> > WordAndPos;
	
	for (SSentenceRec Rec : SentenceRec) {
		auto& WordRecs = Rec.OneWordRec;
		for (auto& OneWordRec : WordRecs) {
			//int iWord = WordMapClean(OneWordRec.Word);
			auto itmc = WordMapClean.find(OneWordRec.Word);
			if (itmc == WordMapClean.end()) {
				continue;
			}
			int iPos = PosMap[OneWordRec.POS];
			WordAndPos.push_back(make_pair(itmc->second, iPos));
		}
	
	}
	

	
	int NumRecords = WordAndPos.size() / 2;
	string H5FileName = H5TrainFileName;
	int NumLabelVals = 1; // not PosCount because label can use real value
	int NumItemsPerRec = NumValsPerVec;
	
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
				auto& wp = WordAndPos[ir];
				vector<float>& OneVec = WordsVecsClean[wp.first];
				for (int ia = 0; ia < NumValsPerVec; ia++) {
					*ppd++ = OneVec[ia]; 
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
				auto& wp = WordAndPos[ir];
				*ppl++ = wp.second;
			}
		}
		

		DataSpace labelspace(cLabelRank, dimsLabel);

		DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

		labelset.write(plabels, PredType::IEEE_F32LE);

		delete plabels;
	}
}