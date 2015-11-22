

{
    string H5TrainFileName="/devlink/caffe/data/WordEmbed/GramValid/data/train.h5";
    string H5TestFileName="/devlink/caffe/data/WordEmbed/GramValid/data/test.h5";
	string WordListFileName="/devlink/caffe/data/WordEmbed/GramValid/data/WordList.txt";
	string WordVecFileName="/ExtDownloads/glove.6B.50d.txt" ;


	
	
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
					// add an invalid record. Only for init
				int ReplaceI = rand() % (NumWordsClean-1); // don't include <na> I think
				if (ReplaceI >= WordIList[cCenterGram]) {
					ReplaceI++;
				}
				WordIList[cMinRealLength-1] = ReplaceI;
				GramRecs.push_back(make_tuple(rand(), WordIList, false));
			}
			
		} // end loop over grams of sentence rec
	} // end loop over sentence recs

	// use the random int in the first field to sort
	sort(GramRecs.begin(), GramRecs.end());
	
	int NumRecords = GramRecs.size() / 2;
	int NumLabelVals = 1;
	int NumDataSets = cNOfNGrams;
#pragma message("Change this")			
		//NumItemsPerRec = (NumValsPerVec + NumPosVals) * cNOfNGrams ;
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
				*ppl++ = (get<2>(GramRecs[ir]) ? 1.0f : 0.0f); 
			}
		}
		
//		cerr << Count[0] << " zeroes vs " << Count[1] << " 1s.\n";

		DataSpace labelspace(cLabelRank, dimsLabel);

		DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

		labelset.write(plabels, PredType::IEEE_F32LE);

		delete plabels;
	}
}
