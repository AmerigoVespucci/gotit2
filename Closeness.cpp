// Closeness.cpp : 
// Takes a GenData proto file and produces all the data needed to rnu a GenSeed Caffe NN generation
//


 
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>

#include "GenSeed.pb.h"
#include "GenData.pb.h"
#include "GenDef.pb.h"

#include "/home/eli/dev/caffe/include/caffe/GenData.hpp"
#include "MascReader.h"
#include "H5Cpp.h"

#include "ipc.pb.h"
#include "/home/eli/dev/caffe/include/caffe/util/ipc.hpp"

using boost::asio::ip::tcp;

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif


#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif

const string InputVecFileName = "/ExtDownloads/glove.6B.50d.txt";
const int cVocabLimit = 4000; // should be 400000
const int cNumValsPerVecSrc = 50;
void * ClosenessHandle = NULL;
const float c_max_yet_close = 2.0f;

struct SClosenessData {
	SClosenessData() : WordsVecs(cVocabLimit) {}
	map<string, int> WordMap;
	vector<vector<float> > WordsVecs; // indexed by the same as WordMap
	vector<string> WordsList;
	map<string, int> SamplesWordMap; // different from word map. Map of gotit's BasicTypeList
	int CommonCountThresh; // the number of instances of some common word

};

void CGotitEnv::ClosenessInit() 
{

	SClosenessData * ClosenessData = new SClosenessData;
	map<string, int>& WordMap = ClosenessData->WordMap;
	vector<vector<float> >& WordsVecs = ClosenessData->WordsVecs; // indexed by the same as WordMap
	vector<string>& WordsList = ClosenessData->WordsList;
	map<string, int>& SamplesWordMap = ClosenessData->SamplesWordMap;
	vector<NameItem>& WordsInOrder = BasicTypeLists["word"];
	long int TotalNumWords = 0;
	for (int iw=0; iw<WordsInOrder.size(); iw++) { // factor 100, just to make sure we have enough
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
			if (w == "because") {
				ClosenessData->CommonCountThresh = WordsInOrder[iw].Count;
			}
			TotalNumWords += WordsInOrder[iw].Count;
//			auto itSamplesWordMap = SamplesWordMap.find(w);
//			if (itSamplesWordMap != SamplesWordMap.end()) {
//				continue;
//			}
//			int MapSize = SamplesWordMap.size(); // saved to avoid ambiguity on next line
			SamplesWordMap[w] = iw;
//			RevSamplesWordMap.push_back(w);
//			if (MapSize >= cNumWords-1) {
//				break;
//			}
		}
	}
	cerr << "Total number of words sampled = " << TotalNumWords << endl;
	
	{
		ifstream VecFile(InputVecFileName);
		if (!VecFile.is_open()) {
			cerr	<< "Error. Parameter in config file: input_vec_file_name: " 
					<< InputVecFileName << " not found.\n";
			return;
		}
		string ln;
		for (int iw = 0; iw < cVocabLimit; iw++) {
			getline(VecFile, ln, ' ');
			string w;
			w = ln;
			if (w.size() == 0) {
				break;
			}
			//int iw = cVocabLimit; // default to <na>
			bool bAddVec = false;
			auto itWordMap = WordMap.find(w);
			if (itWordMap == WordMap.end()) {
				bAddVec = true;
				//iw = itWordMap->second;
			}
			WordsList.push_back(w);
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
				OneVec.push_back(wv);
			}
			if (bAddVec) {
				WordMap[w] = iw;
			}
		}
	}
	

//	string sModelProtoName;
//	if (!GetImplemParam(sModelProtoName, "Implem.Param.FnParam.CaffeFn.ModelProtoName")) {
//		cerr << "CaffeFn cannot be called without setting Implem.Param.FnParam.CaffeFn.ModelProtoName \n";
//		return;
//	}
	ClosenessHandle = (void *)ClosenessData;
}

void CGotitEnv::ClosenessComplete()
{
	
	if (!ClosenessHandle) {
		cerr << "ClosenessComplete may only be called if preceeded by a succesful call to CaffeFnInit.\n";
		return;
	}
	SClosenessData * ClosenessData = (SClosenessData *) ClosenessHandle;
	delete ClosenessData;
	ClosenessHandle= NULL;
}

struct WData {
	WData(int a_int, string a_w, int a_idx) {
		WID = a_int;
		w = a_w;
		i_vec_tbl = a_idx;
	}
	WData(WData* other) {
		WID = other->WID;
		w = other->w;
		i_vec_tbl = other->i_vec_tbl;
	}
	int WID;
	string w;
	int i_vec_tbl;
};

float vec_diff(vector<float>&a, vector<float>& b) {
	float sum = 0.0f;
	for (int iv = 0; iv < a.size(); iv++) {
		float d = a[iv] - b[iv];
		sum += d * d;
	}
	return sum;
}

void CGotitEnv::Closeness()
{
	if (!ClosenessHandle) {
		cerr << "Closeness may only be called if preceeded by a succesful call to CaffeFnInit.\n";
		return;
	}
	SClosenessData * ClosenessData = (SClosenessData *) ClosenessHandle;
	map<string, int>& WordMap = ClosenessData->WordMap;
	vector<vector<float> >& WordsVecs = ClosenessData->WordsVecs; // indexed by the same as WordMap
	vector<string>& WordsList = ClosenessData->WordsList;
	vector<NameItem>& WordListSorted = BasicTypeLists["word"];
	map<string, int>& SamplesWordMap = ClosenessData->SamplesWordMap;

	list<vector<WData > > wvecs; // indexed by sentence id, word id in sentence - might not be sequential
	//int i_curr_rec = 0;
	const int c_srec_keep_size = 3;
	string lw;
	for (int isr = 0; isr < SentenceRec.size(); isr++) {
		wvecs.push_back(vector<WData >());
		SSentenceRec rec = SentenceRec[isr];
		for (int iwrec = 0; iwrec < rec.OneWordRec.size(); iwrec++) {
			string& w = rec.OneWordRec[iwrec].Word;
			lw.clear();
			bool b_is_word = true;
			for (auto c : w) {
				lw += tolower(c);
				if (!isalpha(c)) {
					b_is_word = false;
				}
			}
			if (!b_is_word) continue; 
			if (w == "because") {
				cerr << "Sentence is: " << rec.Sentence << endl;
			}
			auto itWordsSamples =  SamplesWordMap.find(lw);
			if (itWordsSamples == SamplesWordMap.end()) continue;
			if (WordListSorted[itWordsSamples->second].Name != lw) {
				cerr << "Inexplicable error! Wrong word looked up in word samples\n";
				return ;
			}
			if (WordListSorted[itWordsSamples->second].Count > ClosenessData->CommonCountThresh) {
				continue;
			}
			
			//boost::algorithm::to_lower(w);
			auto itWordMap = WordMap.find(lw);
			if (itWordMap != WordMap.end()) {
				list<vector<WData > >::iterator it_wvecs = wvecs.begin();
				for (; it_wvecs != wvecs.end(); it_wvecs++) {
					for (auto wrec : *it_wvecs) {
						int i_vec = wrec.i_vec_tbl;
						vector<float>& other_vec = WordsVecs[i_vec];
						vector<float>& this_vec = WordsVecs[itWordMap->second];
						float diff = vec_diff(other_vec, this_vec);
						if (diff > 0 && diff < c_max_yet_close) {
							cerr << "Two close words: \"" << w << "\" and \"" << WordsList[i_vec] << "\" related by " << diff << ".\n";
						}
					}
				}
				wvecs.back().push_back(WData(iwrec, lw, itWordMap->second));
			}
		}
		if (isr >= c_srec_keep_size) {
			wvecs.pop_front();
		}
	}
	

}

