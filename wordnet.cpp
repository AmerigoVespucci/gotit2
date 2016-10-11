// wordnet.cpp : 
// read wordnet files and create  a wordnet database
//


 
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include "boost/variant.hpp"
#include <glog/logging.h>

#include "MascReader.h"




#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif

using namespace boost::filesystem;

enum synset_type {
    stNoun,
    stVerb,
    stAdj,
    stAdjSat,
    stAdverb,
};
struct WNDBPtrEl {
    string ptr_type; // replace by enum
    int SynetIndex; // double use. At first this is the offset in the file. Replaced by the index into WNDB
    char DestSynetType;
    int source_num; // 0 the whole synet (semantic). > 0 , 1 based index into synet list (lexical))
    int dest_num;
};
class WNDBDataEl {
public:
    char SynetType;
    vector<string> swords;
    vector<int> lex_ids;
    vector<WNDBPtrEl> ptrs;
    
};
class WNDBIndexEl {
public:
    string Lemma;
    char SynetType;
    vector<int> iDBs; // indicies into the DB of synets
};

vector<WNDBDataEl> WNDB;
vector<WNDBIndexEl> WNDBWords;
map<int, int> FileOffsetMap;
map<string, int> WordToIndexMap;

enum synet_data_file_type {
    sdftOffset, 
    sdftLexFileNumber,
    sdftSynetType,
    sdftWordCount,
    sdftWord,
    sdftLexId,
    sdftPtrCount,
    sdftPtrType,
    sdftTargetOffset,
    sdftTargetPos,
    sdftSourceDest,
    
};
enum synet_index_file_type {
    siftLemma,
    siftPos,
    siftSynetCount,
    siftPtrCount,
    siftPtrSymbol,
    siftSenseCnt,
    siftTagsenseCnt,
    siftSynetDataOffset,
};

struct PosInfoEl {
    PosInfoEl(char achar, string astring) {
        SynetType = achar;
        sPosName = astring;
    }
    char SynetType;
    string sPosName;
    map<int, int> FileOffsetMap;
};

vector<PosInfoEl> PosInfoTbl =  {   {'n', "noun"}, 
                                    {'v', "verb"}, 
                                    {'a', "adj"}, 
                                    {'r', "adv"}
                                };
//vector<PosInfoEl> PosInfoTbl =  {   {'n', "noun"}
//                                };

void ParseWNIndexFile(ifstream& ifsWNIndex, int iPosEl) 
{
    map<int, int>& OffsetMap = PosInfoTbl[iPosEl].FileOffsetMap;
    char SynetType = PosInfoTbl[iPosEl].SynetType;
    
    string ln;
    while (getline(ifsWNIndex, ln)) {
        //cerr << ln << endl;
        if (ln[0] == ' ' && ln[1] == ' ') continue;
        istringstream ssl(ln);
        string sword;
        synet_index_file_type sift = siftLemma;
        int SynetCount;
        int PtrCount;
        WNDBIndexEl CurrEl;
        //WNDBSynetPtrEl CurrPtr;
        bool bLineFinished = false;
        while (getline(ssl, sword, ' ')) {
            switch(sift) {
                case siftLemma: {
                    string SearchWord = string(1, SynetType) + ':' + sword;
                    WordToIndexMap[SearchWord] = WNDBWords.size();
                    CurrEl.Lemma = sword;
                    sift = siftPos;
                    break;
                }
                case siftPos: {
                    CurrEl.SynetType = sword[0];
                    sift = siftSynetCount;
                    break;
                }
                case siftSynetCount: {
                    SynetCount = stoi(sword);
                    sift = siftPtrCount;
                    break;
                }
                case siftPtrCount: {
                    PtrCount = stoi(sword);                    
                    sift = siftPtrSymbol;
                    if (PtrCount == 0) {
                        sift = siftSenseCnt;
                    }
                    break;
                }
                case siftPtrSymbol: {
                    sift = siftSenseCnt;
                    if (--PtrCount > 0) {
                        sift = siftPtrSymbol;
                    }
                    break;
                }
                case siftSenseCnt: {
                    sift = siftTagsenseCnt;
                    break;
                }
                case siftTagsenseCnt: {
                    sift = siftSynetDataOffset;
                    break;
                }
                case siftSynetDataOffset: {
                    int iOffset = stoi(sword);
                    auto itMap = OffsetMap.find(iOffset);
                    if (itMap == OffsetMap.end()) {
                        cerr << "Offset missing\n";
                    }
                    else {
                        int iDB = itMap->second;
                        CurrEl.iDBs.push_back(iDB);
                    }
                    sift = siftSynetDataOffset;
                    if (--SynetCount == 0) {
                        bLineFinished = true;
                    }
                    break;
                }
            }
            if (bLineFinished) break;
        }
        WNDBWords.push_back(CurrEl);

    }
}

void ParseWNDataFile(ifstream& ifsWNDataNoun, int iPosEl)
{
    map<int, int>& OffsetMap = PosInfoTbl[iPosEl].FileOffsetMap;
    
    string ln;
    while (getline(ifsWNDataNoun, ln)) {
        //cerr << ln << endl;
        if (ln[0] == ' ' && ln[1] == ' ') continue;
        istringstream ssl(ln);
        string sword;
        synet_data_file_type sdft = sdftOffset;
        int WordCount;
        int PtrCount;
        WNDBDataEl CurrEl;
        WNDBPtrEl CurrPtr;
        bool bLineFinished = false;
        while (getline(ssl, sword, ' ')) {
            switch (sdft) {
                case sdftOffset: {     
                    int iCurrSynet = WNDB.size();
                    int off = stoi(sword);
                    OffsetMap[off] = iCurrSynet;
                    CurrEl.swords.clear();
                    CurrEl.ptrs.clear();
                    sdft = sdftLexFileNumber;
                    break;
                }
                case sdftLexFileNumber: {                    
                    sdft = sdftSynetType;
                    break;
                }
                case sdftSynetType: {
                    CurrEl.SynetType = sword[0];
                    sdft = sdftWordCount;
                    break;
                }
                case sdftWordCount: { 
                    WordCount = stoi(sword, NULL, 16);
                    sdft = sdftWord;
                    break;
                }
                case sdftWord: {      
                    CurrEl.swords.push_back(sword);
                    sdft = sdftLexId;
                    break;
                }
                case sdftLexId: {                    
                    sdft = sdftPtrCount;
                    WordCount--;
                    if (WordCount > 0) {
                        sdft = sdftWord;
                    }
                    break;
                }
                case sdftPtrCount: {    
                    PtrCount = stoi(sword);
                    if (PtrCount == 0) {
                        bLineFinished = true;
                    }
                    sdft = sdftPtrType;
                    break;
                }
                case sdftPtrType: {         
                    CurrPtr.ptr_type = sword;
                    sdft = sdftTargetOffset;
                    break;
                }
                case sdftTargetOffset: {     
                    CurrPtr.SynetIndex = stoi(sword);
                    sdft = sdftTargetPos;
                    break;
                }
                case sdftTargetPos: {             
                    CurrPtr.DestSynetType = sword[0];
                    sdft = sdftSourceDest;
                    break;
                }
                case sdftSourceDest: {              
                    string ssrc = sword.substr(0, 2);
                    string sdest = sword.substr(2, 2);
                    CurrPtr.source_num = stoi(ssrc);
                    CurrPtr.dest_num = stoi(sdest);
                    CurrEl.ptrs.push_back(CurrPtr);
                    sdft = sdftPtrType;
                    PtrCount--;
                    if (PtrCount == 0) {
                        bLineFinished = true;
                    }
                    break;
                }
                default:
                    cerr << "WNDB load. We're lost. unexpected sdft\n";
                    break;
            }
            if (bLineFinished) break;
            //cerr << sword << " | ";
            if (sword == "|") break;
        }
        WNDB.push_back(CurrEl); // full copy but performance not critical
        //cerr << endl;
        
    }
}

int SyntetTypeToIndex(char SynetType) {
    int ipos = -1;
    for (auto& PosInfo : PosInfoTbl) {
        ipos++;
        if (PosInfo.SynetType == SynetType) {
            return ipos;
        }
    }
    return -1;
}
void ConvertDBOffsetsToIDBs() {
    for (auto& DataEl : WNDB) {
        for (auto& PtrEl : DataEl.ptrs) {
            char SynetType = PtrEl.DestSynetType;
            int Offset = PtrEl.SynetIndex;
            int iPosInfo = SyntetTypeToIndex(SynetType);
            if (iPosInfo < 0) {
                cerr << "Error! Synet has unknown type. \n";
                return;
            }
            auto& OffsetMap = PosInfoTbl[iPosInfo].FileOffsetMap;
            auto itMap =  OffsetMap.find(Offset);
            if (itMap == OffsetMap.end()) {
                cerr << "Error! Synet offset not found in map.\n";
                return;
            }
            PtrEl.SynetIndex = itMap->second;
        }
    }
}

#define PTR_TYPE_HYPERNYM "@"
bool WordNetIsHypernym(string sHypoCand, string sHyperCand, char SynetType) {
    string SearchWord = string(1, SynetType) + ":" + sHypoCand;
    auto itWordIndex = WordToIndexMap.find(SearchWord);
    if (itWordIndex == WordToIndexMap.end()) {
        return false;
    }
    WNDBIndexEl& WordIndex = WNDBWords[itWordIndex->second];
    list<int> HypernymIdxs;
    for (auto iDBFromWord : WordIndex.iDBs) {
        HypernymIdxs.push_back(iDBFromWord);
        while (!HypernymIdxs.empty()) {
            int iDB = HypernymIdxs.front();
            HypernymIdxs.pop_front();
            WNDBDataEl& DataEl = WNDB[iDB];
            for (string& SynetWord : DataEl.swords) {
                boost::algorithm::to_lower(SynetWord);
                if (sHyperCand == SynetWord) {
                    return true;
                }
            }
            for (auto& PtrEl : DataEl.ptrs) {
                string sPtrType = PtrEl.ptr_type;
                if (sPtrType == PTR_TYPE_HYPERNYM) {
                    HypernymIdxs.push_back(PtrEl.SynetIndex);
                }
            }
        }
    }
    return false;
}

void CGotitEnv::WordNetDBLoad()
{
    string sPathPrefix = "//guten//WordNet//dict//";
    
    int iel = -1;
    for (auto& el : PosInfoTbl) {
        iel++;
        string fname = sPathPrefix + "data." + el.sPosName;
        ifstream ifsWNData(fname);
        if (!ifsWNData.is_open()) {
            continue;
        }
        ParseWNDataFile(ifsWNData, iel);
    }
    iel = -1;
    for (auto& el : PosInfoTbl) {
        iel++;
        string fname = sPathPrefix + "index." + el.sPosName;
        ifstream ifsWNIndex(fname);
        if (!ifsWNIndex.is_open()) {
            continue;
        }
        ParseWNIndexFile(ifsWNIndex, iel);
    }
    
    ConvertDBOffsetsToIDBs();
    
    WordNetIsHypernym("indian", "person", 'n');
    
    
    
}
