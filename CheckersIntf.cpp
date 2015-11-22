// CheckersIntf.cpp : interfaces to a game of checkers							
//

#include "stdafx.h"

#include "MascReader.h"
#include "CheckersIntf.h"
#include "Regex2D.h"

static const int cDataBufSize = (BOARD_SIZE * BOARD_SIZE) + cHeaderSize;
const int cSampleSize = 5;
const int cLearnRandRate = 3;
const int cEvalRandRate = 97;

// sample has format s, size of sample (e.g. 5), next turn (w or b) and size*size chars ranging from 0, blank to 5 invalid
const int cStartPosInSample = 0;
const int cSizePosInSample = 1;
const int cTurnPosInSample = 2;
// delta dep has format: d, who moved w or b, who's move next w or b, a number (0 or more) of deltas
// each delta: + or -, xDist, + or - yDist, prev board pos: one of a - blank, b - white, c - black, d - white king , e - black king
// after deltas dep continues with * followed by score: + or -, 1 digit integer
const int cStartPosInDep = 0;
const int cTurnPosInDep = 1;
const int cNextTurnInDep = 2;

const int cMoveFromPos = 0; // First sample in sentence is the previous board where the move came from
const int cMoveFollowingPos = 1; // Place left for next move to fill in. Then we will write retrocatively into this rec the from sample of that (the next) rec
const int cMovePickPos = 2;

struct SCIInputOption {
	map<string, SWordData>::iterator itSrcSample;
	float Score;
	ushort MoveData;
	//void Store(ofstream& fout);
	//void Load(ifstream& fin);
};


uchar CIDataBuf0[cDataBufSize];
uchar CIDataBuf1[cDataBufSize];
ushort CIInputSrcAndDest;;
map<string, map<string, int> > CountTypesMap;
bool bCILastGiven0 = false;
map<string, int>* pCIBaseWordCount;
map<string, int>* pCIDepTypes;
vector<SSentenceRec>* pCISentenceList;
map<string, vector<NameItem> >* pCIBasicTypeLists;
bool bCICalcMoves = false;
map<string, SWordData>* pCIMapWordToData;
CPatGrpMgr* pCIPatGrpMgr;
int CINumValidsWithOptions = 0;
int CINumValids = 0;
bool bCILastInputSetWithOption = false;
int CINumBadInputs = 0;
int iCIHitForInput = -1;
int CINumOptionsFound = 0;
int CINumNoOptionsFound = 0;
bool bCIEvaluating = false;
vector<string> CIDepNames;
int NumWhiteWins;
int NumBlackWins;
int CIDebugNumFindGrps;
int CIDebugNumFindGrpByFID;
bool CIDebugPrint = false;

bool CICreateScoreForMove(	SCIInputOption& Option, CPatGrpWrapper& PatGrp,
							uchar * pBoardConsiderd, string& Sample,
							int xSampleZero, int ySampleZero,
							int depth, float ScoreAbove);


bool CIOptionsGT(const SCIInputOption& O0, const SCIInputOption& O1)
{
	return (O0.Score > O1.Score);
}


bool CheckersGetDataBuffer(uchar *& pData, int MaxSize)
{
	if (MaxSize > cDataBufSize) {
		pData = NULL;
		return false;
	}
	if (bCILastGiven0) {
		pData = CIDataBuf1;
		bCILastGiven0 = false;
	}
	else {
		pData = CIDataBuf0;
		bCILastGiven0 = true;
	}
	return true;
}

void CICreateDeltaDep(	string& sDep, uchar * pOldBoard, uchar * pNewBoard, 
						bool bWhiteTurn, bool bNextTurnWhite, 
						int xSrc, int ySrc, int SampleSize)
{
	int xAbsStart = xSrc - (cSampleSize / 2);
	int yAbsStart = ySrc - (cSampleSize / 2);
	sDep = string("d") + (bWhiteTurn ? "w" : "b") + (bNextTurnWhite ? "w" : "b");
	int Score = 0;
	for (int iRow = 0; iRow < SampleSize; iRow++) {
		int yAbs = yAbsStart + iRow;
		for (int iCol = 0; iCol < SampleSize; iCol++) {
			int xAbs = xAbsStart + iCol;
			int iVec = (yAbs * BOARD_SIZE) + xAbs;
			if ((xAbs < 0) || (xAbs >= BOARD_SIZE)
				|| (yAbs < 0) || (yAbs >= BOARD_SIZE)) {
				//Sample[iVec] = (uchar)eBoardInvalid;
				//Sample += '0' + (char)eBoardInvalid;

			}
			else {
				if (pOldBoard[iVec] != pNewBoard[iVec]) {
					
					sDep += ((xAbs >= xSrc) ? "+" : "-");
					sDep += '0' + abs(xAbs - xSrc);
					sDep += ((yAbs >= ySrc) ? "+" : "-");
					sDep += '0' + abs(yAbs - ySrc);
					//sDep += ((iCol >= 0) ? "+" : "-");
					//sDep += '0' + abs(iCol);
					//sDep += ((iRow >= 0) ? "+" : "-");
					//sDep += '0' + abs(iRow - 0);
					sDep += 'a' + pOldBoard[iVec];
					sDep += 'a' + pNewBoard[iVec];
					for (int iBoth = 0; iBoth < 2; iBoth++) {
						uchar * pBoard = pOldBoard;
						int DeltaVal = 1;
						if (iBoth == 1) {
							pBoard = pNewBoard;
							DeltaVal = -1;
						}
						if (pBoard[iVec] == (BoardType)eBoardWhite) Score -= DeltaVal;
						if (pBoard[iVec] == (BoardType)eBoardWhiteKing) Score -= 2 * DeltaVal;
						if (pBoard[iVec] == (BoardType)eBoardBlack) Score += DeltaVal;
						if (pBoard[iVec] == (BoardType)eBoardBlackKing) Score += 2 * DeltaVal;
					}
				}
				//Sample[iVec] = *(pBoard + (yAbs * BOARD_SIZE) + xAbs);
			}
		}
	}
	sDep += string("*") + ((Score >= 0) ? "+" : "-");
	sDep += '0' + abs(Score);


}

void CICreateSample(string& Sample, uchar * pBoard, int xAbsStart, int yAbsStart, int SampleSize)
{
	//_ASSERT(Sample.length() == (cTurnPosInSample + 1));

	for (int iRow = 0; iRow < SampleSize; iRow++) {
		int yAbs = yAbsStart + iRow;
		for (int iCol = 0; iCol < SampleSize; iCol++) {
			int xAbs = xAbsStart + iCol;
			//int iVec = (iRow * SampleSize) + iCol;
			if ((xAbs < 0) || (xAbs >= BOARD_SIZE)
				|| (yAbs < 0) || (yAbs >= BOARD_SIZE)) {
				//Sample[iVec] = (uchar)eBoardInvalid;
				Sample += '0' + (char)eBoardInvalid;

			}
			else {
				//Sample[iVec] = *(pBoard + (yAbs * BOARD_SIZE) + xAbs);
				Sample += '0' + (char)*(pBoard + (yAbs * BOARD_SIZE) + xAbs);
			}
		}
	}
}


void CIStoreMoves(uchar * pOldBoard, uchar * pNewBoard, ushort Hdr) {
	//static bool bLastValidTurnWhite = false;
	static int NumTimesCalled = 0;
	bool bWhiteTurn = ((Hdr & cWhiteTurn) != 0);
	bool bValidInput = ((Hdr & cInputValid) != 0);
	bool bTurnOver = ((Hdr & cTurnOver) != 0);
	bool bEndTurn = ((Hdr & cFinalTurn) != 0);
	bool bNextTurnWhite = bWhiteTurn != bTurnOver;

	if (bEndTurn) {
		pCISentenceList->push_back(SSentenceRec());
		string SampleTo = "o" + to_string(cSampleSize) + (!bWhiteTurn ? "w" : "b") ; // declare winner not loser
		if (bWhiteTurn) {
			NumBlackWins++;
		}
		else {
			NumWhiteWins++;
		}
		CICreateSample(SampleTo, pNewBoard, 0, 0, cSampleSize); // start positions don't really matter
		WordRec wrec;
		wrec.Word = SampleTo;
		pCISentenceList->back().OneWordRec.push_back(wrec); // end turn characterized by first record being an "o" sample followed by the letter for who won
		if (pCISentenceList->size() > 1) { // not the first
			SSentenceRec* pPrevSentence;
			pPrevSentence = &((*pCISentenceList)[pCISentenceList->size() - 2]); // I can add to the pointer but I prefer dereferencing it and using array notation instead
			//string& FirstWordOfPrev = pPrevSentence->OneWordRec[0].Word;
			pPrevSentence->OneWordRec[cMoveFollowingPos].Word = SampleTo;
			//pPrevSentence->OneWordRec.resize(cMoveFollowingPos); // delete second record that never gets a follows
			//pPrevSentence->Deps.pop_back();
		}
		return;
	}
	if (bValidInput) {
		NumTimesCalled++;
		pCISentenceList->push_back(SSentenceRec());
		pCISentenceList->back().OneWordRec.resize(cMoveFollowingPos + 1);
		//int yDest = Hdr & cOneInputMask;
		Hdr >>= cInputShift;
		//int xDest = Hdr & cOneInputMask;
		Hdr >>= cInputShift;
		int ySrc = Hdr & cOneInputMask;
		Hdr >>= cInputShift;
		int xSrc = Hdr & cOneInputMask;
		//static uchar cNegFlag = 0x08;
		//static int cRelShift = 4;
		//uchar yRelDest = ((ySrc > yDest) ? (cNegFlag | (ySrc - yDest)) : (yDest - ySrc));
		//uchar xRelDest = ((xSrc > xDest) ? (cNegFlag | (xSrc - xDest)) : (xDest - xSrc));
		//ushort InputWord = (bWhiteTurn ? cWhiteTurn : 0) | (yRelDest << cRelShift) | xRelDest;

		int xAbsStart = xSrc - (cSampleSize / 2);
		int yAbsStart = ySrc - (cSampleSize / 2);
		// Let's make this very clear. A b in the third letter of a sample (first letter s)
		// means the move AFTER this board was by black
		string SampleFrom = "s" + to_string(cSampleSize) + (bWhiteTurn ? "w" : "b") ;
		CICreateSample(SampleFrom, pOldBoard, xAbsStart, yAbsStart, cSampleSize);

		map<string, int>::iterator itWordCount = pCIBaseWordCount->find(SampleFrom);
		if (itWordCount != pCIBaseWordCount->end()) {
			itWordCount->second++;
		}
		else {
			(*pCIBaseWordCount)[SampleFrom] = 1;
		}

		pCISentenceList->back().OneWordRec[cMoveFromPos].Word = SampleFrom;
		DepRec drec;


		//int Score = 0;
		SSentenceRec* pPrevSentence;
		//bool bCanFollow = false;
		if (pCISentenceList->size() > 1) { // not the first
			//bCanFollow = true; // for now. We might change our mind
			pPrevSentence = &((*pCISentenceList)[pCISentenceList->size() - 2]); // I can add to the pointer but I prefer dereferencing it and using array notation instead
			string& FirstWordOfPrev = pPrevSentence->OneWordRec[cMoveFromPos].Word;
			if (FirstWordOfPrev[0] == 'o') {
				//bCanFollow = false; // the last record ended the previous game
			}
			else {
				pPrevSentence->OneWordRec[cMoveFollowingPos].Word = SampleFrom;
			}
		}
		
		CreateBasicRegPat(BOARD_SIZE, (char *)pOldBoard, (char *)pNewBoard, bWhiteTurn, xSrc, ySrc);

		TestBrdValidByRegex(BOARD_SIZE, (char *)pOldBoard, (char *)pNewBoard, bWhiteTurn, xSrc, ySrc);

		vector<int> AccompanyPosList;

		string sDep;
		CICreateDeltaDep(	sDep, pOldBoard, pNewBoard, bWhiteTurn, bNextTurnWhite,
							xSrc, ySrc, cSampleSize);
		int iDep;
		map<string, int>::iterator itdm = pCIDepTypes->find(sDep);
		if (itdm == pCIDepTypes->end()) {
			iDep = pCIDepTypes->size();
			(*pCIDepTypes)[sDep] = iDep;
			CIDepNames.push_back(sDep);
			//(CountTypesMap["depcount"])[sLeadsTo] = 0;
			((*pCIBasicTypeLists)["depcount"]).push_back(NameItem(sDep, 0));
		}
		else {
			iDep = itdm->second;
		}
		drec.iDep = (uchar)iDep;
		drec.Gov = (uchar)cMoveFromPos; // see comments above on Pos consts
		drec.Dep = (uchar)cMoveFollowingPos;
		pCISentenceList->back().Deps.push_back(drec);


	} // if bValidInput

}

float CalcScoreForSampleFollowing(string& Sample) {
	float MoveScore = 0.0f;
	float NumMoves = 0.0f;
	map<string, SWordData>::iterator itWordGrpTbl
		= pCIMapWordToData->find(Sample);
	if (itWordGrpTbl == pCIMapWordToData->end()) {
		return 0.0;
	}
	map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
	map<int, int>::iterator itGrps = WordGrps.begin();
	for (; itGrps != WordGrps.end(); itGrps++) {
		CPatGrpHolder * phPatGrp
			= pCIPatGrpMgr->FindGrpByFileID(itGrps->first);
		CIDebugNumFindGrpByFID++;
		if (phPatGrp == NULL) {
			continue;
		}

		CPatGrpWrapper PatGrp(phPatGrp);
		SPatternFinder* pFinder;
		if (!PatGrp.getGrpFinder(pFinder) || (pFinder == NULL)) {
			continue;
		}

		if ((pFinder->group_name_template != "GovsOfDep")
			|| (pFinder->num_stores < 0)) {
			continue;
		}
		string sGrpSeed = PatGrp.getGrpParam(0);
		//bool bMoveFound = false;
		//int MoveSpecStart;
		if ((sGrpSeed[0] == 'l')) {
//			&& (bNextTurnWhite == (sGrpSeed[1] == 'w'))){
			//bMoveFound = true;
			NumMoves += 1.0f;
			//MoveSpecStart = 2;
		}
		else if ((sGrpSeed[0] == 't')) {
	//		&& (bNextTurnWhite == (sGrpSeed[2] == 'w'))){
			//bMoveFound = true;
			NumMoves += 1.0f;
			//MoveSpecStart = 3;
			char Taken = sGrpSeed[1];
			switch (Taken) {
			case 'w': MoveScore += -2.0; break;
			case 'W': MoveScore += -3.0; break;
			case 'b': MoveScore += 2.0; break;
			case 'B': MoveScore += 3.0; break;
			}
		}
	}

	if (NumMoves == 0.0f) {
		return 0.0f;
	}
	return (MoveScore /= NumMoves);

}

float CIEvalResultingBoard(uchar * pBoardConsiderd, bool bWhiteTurn, int depth, float ScoreAbove)
{
	float Score = FLT_MAX;
	if (bWhiteTurn) {
		Score = -FLT_MAX;
	}
	for (int iRow = -(cSampleSize / 2); iRow < BOARD_SIZE; iRow++) {
		int ySampleZero = iRow + (cSampleSize / 2);
		for (int iCol = -(cSampleSize / 2); iCol < BOARD_SIZE; iCol++) {
			int xSampleZero = iCol + (cSampleSize / 2);
			//vector<uchar> SampleForChoice(cSampleSize * cSampleSize);
			string SampleForChoice = "s" + to_string(cSampleSize) + (bWhiteTurn ? "w" : "b");
			CICreateSample(SampleForChoice, pBoardConsiderd,
				iCol, iRow,
				cSampleSize);
			map<string, SWordData>::iterator itWordGrpTbl
				= pCIMapWordToData->find(SampleForChoice);
			if (itWordGrpTbl == pCIMapWordToData->end()) {
				continue;
			}
			map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
			map<int, int>::iterator itGrps = WordGrps.begin();
			for (; itGrps != WordGrps.end(); itGrps++) {
				CPatGrpHolder * phPatGrp
					= pCIPatGrpMgr->FindGrpByFileID(itGrps->first);
				CIDebugNumFindGrpByFID++;
				if (phPatGrp == NULL) {
					continue;
				}

				CPatGrpWrapper PatGrp(phPatGrp);
				SPatternFinder* pFinder;
				if (!PatGrp.getGrpFinder(pFinder) || (pFinder == NULL)) {
					continue;
				}

				if ((pFinder->group_name_template != "GovsOfDep")
					|| (pFinder->num_stores < 0)) {
					continue;
				}
				string sGrpSeed = PatGrp.getGrpParam(0);
				//bool bMoveFound = false;
				//int MoveSpecStart;
				//float MoveScore = 0.0;
				if (sGrpSeed[0] != 'd') {
					continue;
				}
				PatGrp.getGrpElsMapSize();
				SCIInputOption OneOption;
				if (CICreateScoreForMove(OneOption, PatGrp, pBoardConsiderd,
											SampleForChoice, xSampleZero, ySampleZero,
											depth, ScoreAbove)) {
					if (CIDebugPrint) {
						for (int i = 7; i > depth; i--) {
							cerr << "\t";
						}
						cerr << "Move: " << sGrpSeed << " for " << (bWhiteTurn ? "w" : "b") 
								<< ", (" << xSampleZero << "," 
								<< ySampleZero << ") ScoreAbove: " 
								<< ScoreAbove << ". Score: "
								<< OneOption.Score << endl;
					}
					if (bWhiteTurn) {
						if (OneOption.Score > Score) {
							Score = OneOption.Score;
						}
					}
					else {
						if (OneOption.Score < Score) {
							Score = OneOption.Score;
						}
					}
				}
			}
		}
	}
	return Score;
}

float CIEvalResultOfMove(vector<tuple<int, int, int, int> >& Deltas, uchar * pOldBoard, bool bNextTurnWhite, int depth, float ScoreAbove)
{
	uchar * pNewBoard = new uchar[(BOARD_SIZE * BOARD_SIZE)];
	memcpy(pNewBoard, pOldBoard, (BOARD_SIZE * BOARD_SIZE));
	// apply deltas
	for (uint iDelta = 0; iDelta < Deltas.size(); iDelta++) {
		tuple<int, int, int, int>& Delta = Deltas[iDelta];
		pNewBoard[(get<1>(Delta) * BOARD_SIZE) + get<0>(Delta)] = get<3>(Delta);
	}
	return CIEvalResultingBoard(pNewBoard, bNextTurnWhite, depth, ScoreAbove);
}

bool CICreateScoreForMove(SCIInputOption& Option, CPatGrpWrapper& PatGrp,
							uchar * pBoardConsiderd, string& Sample,
							int xSampleZero, int ySampleZero,
							int depth, float ScoreAbove)
{
	float Score = 0.0f;
	string sGrpSeed = PatGrp.getGrpParam(0);
	size_t ScorePos = sGrpSeed.find('*');
	if (ScorePos == string::npos) {
		cerr << "Error: Badly formed CI dep found with no score: " << sGrpSeed << endl;
		return false;
	}
	Score = (float)stoi(sGrpSeed.substr(ScorePos + 1));
	bool bWhiteTurnFromSample = Sample[cTurnPosInSample] == 'w';
	bool bWhiteTurnFromDep = sGrpSeed[cTurnPosInDep] == 'w';
	if (bWhiteTurnFromDep != bWhiteTurnFromSample) {
		cerr << "Error: Turn choice in dep does not match sample chosen";
		return false;
	}
	vector<tuple<int, int, int, int> > Deltas;
	bool bNextTurnWhite = (sGrpSeed[cNextTurnInDep] == 'w');
	string sDeltas = sGrpSeed.substr(cNextTurnInDep + 1, ScorePos - cNextTurnInDep - 1);
	int iFrom = -1;
	int iTo = -1; 
	//float ScoreChart[] = { 0.0f, 1.0f, 2.0f, -1.0f, -2.0f };
	
	for (int iDelta = 0; sDeltas.length() >= 6; iDelta++) { // 6 for +/-|digit|+/-|digit|prevchar|nowchar
		string sOneDelta = sDeltas.substr(0, 6);
		sDeltas = sDeltas.substr(6);
		int x = stoi(sOneDelta.substr(0, 2)) + xSampleZero;
		int y = stoi(sOneDelta.substr(2, 2)) + ySampleZero;
		int PrevChar = sOneDelta[4] - 'a';
		int NewChar = sOneDelta[5] - 'a';
		if ((PrevChar == 1 || PrevChar == 3) && bWhiteTurnFromDep) {
			iFrom = iDelta;
		}
		else if ((PrevChar == 2 || PrevChar == 4) && !bWhiteTurnFromDep) {
			iFrom = iDelta;
		}
		else if ((NewChar == 1 || NewChar == 3) && bWhiteTurnFromDep) {
			iTo = iDelta;
		}
		else if ((NewChar == 2 || NewChar == 4) && !bWhiteTurnFromDep) {
			iTo = iDelta;
		}
		//Score -= ScoreChart[PrevChar];
		//Score += ScoreChart[NewChar];
		Deltas.push_back(make_tuple(x, y, PrevChar, NewChar));
		
	}
	ScoreAbove += Score;
	depth--;
	if (depth > 0) {
		Score = CIEvalResultOfMove(Deltas, pBoardConsiderd, bNextTurnWhite, depth, ScoreAbove);
	}
	else {
		Score = ScoreAbove; 
	}
	if (iFrom == -1 || iTo == -1) {
		cerr << "Error: dep string seems invalid: " << sGrpSeed << endl;
		return false;
	}
	Option.MoveData = get<1>(Deltas[iTo]) | ((get<0>(Deltas[iTo])) << cInputShift)
					| ((get<1>(Deltas[iFrom])) << (2 * cInputShift)) 
					| ((get<0>(Deltas[iFrom])) << (3 * cInputShift))
					| 0x1 << (4 * cInputShift);
	//Option.Score = (bWhiteTurnFromDep ? Score : -Score);
	Option.Score = Score;

	return true;

}

bool CICreateInput(uchar * pOldBoard, uchar * pNewBoard, ushort Hdr)
{
	//static bool bLastValidTurnWhite = false;
	//static int NumTimesCalled = 0;
	bool bWhiteTurn = ((Hdr & cWhiteTurn) != 0);
	//bool bValidInput = ((Hdr & cInputValid) != 0);
	bool bTurnOver = ((Hdr & cTurnOver) != 0);
	//bool bEndTurn = ((Hdr & cFinalTurn) != 0);
	bool bNextTurnWhite = (bWhiteTurn != bTurnOver);
	bool bUseOption = false;
	int iOption = 0;

	//if (!bValidInput) {
	//	return false;
	//}
	vector<SCIInputOption > OptionsList;
	bool bValidInputCreated = false;
	for (int iRow = -(cSampleSize / 2); iRow < BOARD_SIZE; iRow++) {
		int ySampleZero = iRow + (cSampleSize / 2);
		for (int iCol = -(cSampleSize / 2); iCol < BOARD_SIZE; iCol++) {
			int xSampleZero = iCol + (cSampleSize / 2);
			//vector<uchar> SampleForChoice(cSampleSize * cSampleSize);
			string SampleForChoice =	"s" + to_string(cSampleSize) 
									+	(bNextTurnWhite ? "w" : "b");
			CICreateSample(SampleForChoice, pNewBoard,
				iCol, iRow,
				cSampleSize);
			map<string, SWordData>::iterator itWordGrpTbl
				= pCIMapWordToData->find(SampleForChoice);
			if (itWordGrpTbl == pCIMapWordToData->end()) {
				continue;
			}
			map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
			map<int, int>::iterator itGrps = WordGrps.begin();
			for (; itGrps != WordGrps.end(); itGrps++) {
				CPatGrpHolder * phPatGrp
					= pCIPatGrpMgr->FindGrpByFileID(itGrps->first);
				CIDebugNumFindGrpByFID++;
				if (phPatGrp == NULL) {
					continue;
				}

				CPatGrpWrapper PatGrp(phPatGrp);
				SPatternFinder* pFinder;
				if (!PatGrp.getGrpFinder(pFinder) || (pFinder == NULL)) {
					continue;
				}

				if ((pFinder->group_name_template != "GovsOfDep")
					|| (pFinder->num_stores < 0)) {
					continue;
				}
				string sGrpSeed = PatGrp.getGrpParam(0);
				//bool bMoveFound = false;
				//int MoveSpecStart;
				//float MoveScore = 0.0;
				if (sGrpSeed[0] != 'd') {
					continue;
				}
				PatGrp.getGrpElsMapSize();
				SCIInputOption OneOption;
				int SearchDepth = 7;
				if (!bNextTurnWhite) {
					SearchDepth = 2;
				}
				if (CICreateScoreForMove(	OneOption, PatGrp, pNewBoard, 
											SampleForChoice, xSampleZero, ySampleZero,
											SearchDepth, 0.0f)) {
					if (CIDebugPrint) {
						cerr	<< "Move: " << sGrpSeed << " for " 
								<< (bNextTurnWhite ? "w" : "b") 
								<< ", (" << xSampleZero << "," 
								<< ySampleZero << ") score: " 
								<< OneOption.Score << endl;
					}
					if (!bNextTurnWhite) {
						OneOption.Score = -OneOption.Score;
					}
					OptionsList.push_back(OneOption);
				}
			}
		}
	}

	sort(OptionsList.begin(), OptionsList.end(), CIOptionsGT);
	int NumOptions = OptionsList.size();
	//cerr << "Found " << NumOptions << " for " << (bNextTurnWhite ? "w" : "b") << endl;
	float MaxScore = -1000.0f;
	int NumAtMax = 0;
	if (NumOptions > 0) {
		bValidInputCreated = true;
		//NumOptionsFound++;
		MaxScore = OptionsList[0].Score;
		const float cAllowFactor = 0.999f;
		if (MaxScore >= 0.0f) {
			MaxScore *= cAllowFactor;
		}
		else {
			MaxScore /= cAllowFactor;
		}
		for (int iiOption = 0; iiOption < NumOptions; iiOption++) {
			if (OptionsList[iiOption].Score < MaxScore) {
				break;
			}
			NumAtMax++;
		}
		int RandRate = cLearnRandRate;
		if (bCIEvaluating) {
			RandRate = cEvalRandRate;
			if (!bNextTurnWhite) {
				//NumAtMax = NumOptions;        ///////////// Decides who wins!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
		}
		if ((NumAtMax == 0) || ((rand() % RandRate) == 0)) {
			bUseOption = false;
		}
		else {
			iOption = rand() % NumAtMax;
			bUseOption = true;
		}
	}
	else {
		//if (!bDebugGameOver) {
		//	NumNoOptionsFound++;
		//}
	}

	if (bUseOption) {
		//NumValidsWithOptions++;
		bCILastInputSetWithOption = true;
		//iCIHitForInput = OptionsList[iOption].itSrcSample->second;
		CIInputSrcAndDest = OptionsList[iOption].MoveData;
	}
	else {
		bValidInputCreated = false;
	}

	return bValidInputCreated;
}

bool CICreateInput2(uchar * pOldBoard, uchar * pNewBoard, ushort Hdr)
{
	//static bool bLastValidTurnWhite = false;
	//static int NumTimesCalled = 0;
	bool bWhiteTurn = ((Hdr & cWhiteTurn) != 0);
	bool bValidInput = ((Hdr & cInputValid) != 0);
	bool bTurnOver = ((Hdr & cTurnOver) != 0);
	//bool bEndTurn = ((Hdr & cFinalTurn) != 0);
	bool bNextTurnWhite = bWhiteTurn != bTurnOver;
	bool bUseOption = false;
	int iOption = 0;

	if (!bValidInput) {
		return false;
	}
	vector<SCIInputOption > OptionsList;
	bool bValidInputCreated = false;
	for (int iRow = -(cSampleSize / 2); iRow < BOARD_SIZE; iRow++) {
		for (int iCol = -(cSampleSize / 2); iCol < BOARD_SIZE; iCol++) {
			//vector<uchar> SampleForChoice(cSampleSize * cSampleSize);
			string SampleForChoice = "s" + to_string(cSampleSize) + (bNextTurnWhite ? "w" : "b");
			CICreateSample(	SampleForChoice, pNewBoard,
							iCol, iRow,
							cSampleSize);
			map<string, SWordData>::iterator itWordGrpTbl
				= pCIMapWordToData->find(SampleForChoice);
			if (itWordGrpTbl == pCIMapWordToData->end()) {
				continue;
			}
			map<int, int>& WordGrps = itWordGrpTbl->second.GrpList;
			map<int, int>::iterator itGrps = WordGrps.begin();
			for (; itGrps != WordGrps.end(); itGrps++) {
				CPatGrpHolder * phPatGrp
					= pCIPatGrpMgr->FindGrpByFileID(itGrps->first);
				CIDebugNumFindGrpByFID++;
				if (phPatGrp == NULL) {
					continue;
				}

				CPatGrpWrapper PatGrp(phPatGrp);
				SPatternFinder* pFinder;
				if (!PatGrp.getGrpFinder(pFinder) || (pFinder == NULL)) {
					continue;
				}

				if (	(pFinder->group_name_template != "GovsOfDep")
					||	(pFinder->num_stores < 0)) {
					continue;
				}
				string sGrpSeed = PatGrp.getGrpParam(0);
				bool bMoveFound = false;
				int MoveSpecStart;
				float MoveScore = 0.0;
				if (	(sGrpSeed[0] == 'l') 
					&&	(bNextTurnWhite == (sGrpSeed[1] == 'w'))){
					bMoveFound = true;
					MoveSpecStart = 2;
				}
				else if ((sGrpSeed[0] == 't')
					&& (bNextTurnWhite == (sGrpSeed[2] == 'w'))){
					bMoveFound = true;
					MoveSpecStart = 3;
					char Taken = sGrpSeed[1];
					switch (Taken) {
					case 'w': MoveScore = -2.0f; break;
					case 'W': MoveScore = -3.0f; break;
					case 'b': MoveScore = 2.0f; break;
					case 'B': MoveScore = 3.0f; break;
					}
				}
				if (!bMoveFound) {
					continue;
				}
				vector<string> MoveFollowingList;
				string FollowDep = string("f") + sGrpSeed.substr(MoveSpecStart-1);
				string GrpFollowName =		string("DepsOfGovsOfDep[%0%") + FollowDep 
										+	"][%1%" + SampleForChoice + "]";
				CPatGrpHolder* phPatGrpFollows = pCIPatGrpMgr->FindGrp(GrpFollowName);
				CIDebugNumFindGrps++;
				if (phPatGrpFollows != NULL) {
					CPatGrpWrapper PatGrpFollows(phPatGrpFollows);
					int NumFollowing = PatGrpFollows.getGrpElsMapSize();
					float FollowingScore = 0.0f;
					float NumFollowingScores = 0.0f;
					for (int iF = 0; iF < NumFollowing; iF++) {
						SGroupingLink * pEl = PatGrpFollows.getGrpElByIndex(iF);
						FollowingScore += CalcScoreForSampleFollowing(pEl->W);
						NumFollowingScores += 1.0f;
					}
					if (NumFollowingScores != 0.0f) {
						MoveScore += (FollowingScore / NumFollowingScores);
					}
				}
				string sMove = sGrpSeed.substr(MoveSpecStart);
				bool bxPlus = (sMove[0] == '+');
				size_t yPlusPos = sMove.find_first_of("+-", 1);
				if (yPlusPos == string::npos) {
					continue;
				}
				int xRel = stoi(sMove.substr(1, yPlusPos - 1));
				xRel *= (bxPlus ? 1 : -1);
				bool byPlus = (sMove[yPlusPos] == '+');
				int yRel = stoi(sMove.substr(yPlusPos+1));
				yRel *= (byPlus ? 1 : -1);
				int xInputDest = iCol + (cSampleSize / 2) + xRel;
				int yInputDest = iRow + (cSampleSize / 2) +yRel;
				ushort InputSel =	yInputDest | (xInputDest << cInputShift)
								|		((iRow + (cSampleSize / 2))
									<<	(2 * cInputShift))
								|		((iCol + (cSampleSize / 2))
									<<	(3 * cInputShift))
								|	0x1 << (4 * cInputShift);
				

				SCIInputOption Option;
				Option.itSrcSample = itWordGrpTbl;
				Option.MoveData = InputSel;
				Option.Score = (bNextTurnWhite ? MoveScore : -MoveScore); 
				OptionsList.push_back(Option);

			} // end loop over groups
		}
	} // end loop over rows
	sort(OptionsList.begin(), OptionsList.end(), CIOptionsGT);
	int NumOptions = OptionsList.size();
	float MaxScore = -1000.0f;
	int NumAtMax = 0;
	if (NumOptions > 0) {
		bValidInputCreated = true;
		//NumOptionsFound++;
		MaxScore = OptionsList[0].Score;
		const float cAllowFactor = 0.999f;
		if (MaxScore >= 0.0f) {
			MaxScore *= cAllowFactor;
		}
		else {
			MaxScore /= cAllowFactor;
		}
		for (int iiOption = 0; iiOption < NumOptions; iiOption++) {
			if (OptionsList[iiOption].Score < MaxScore) {
				break;
			}
			NumAtMax++;
		}
		int RandRate = cLearnRandRate;
		if (bCIEvaluating) {
			RandRate = cEvalRandRate;
			if (!bNextTurnWhite) {
				NumAtMax = NumOptions;
			}
		}
		if ((NumAtMax == 0) || ((rand() % RandRate) == 0)) {
			bUseOption = false;
		}
		else {
			iOption = rand() % NumAtMax;
			bUseOption = true;
		}
	}
	else {
		//if (!bDebugGameOver) {
		//	NumNoOptionsFound++;
		//}
	}

	if (bUseOption) {
		//NumValidsWithOptions++;
		bCILastInputSetWithOption = true;
		//iCIHitForInput = OptionsList[iOption].itSrcSample->second;
		CIInputSrcAndDest = OptionsList[iOption].MoveData;
	}
	else {
		bValidInputCreated = false;
	}

	return bValidInputCreated;
}

void CheckersMoveComplete(uchar * pData) {
	ushort Hdr = *(ushort *)pData;
	uchar * pDataBeforeMove;
	//bool bUseOption = false;
	//int iOption = 0;
	//vector<SInputOption > OptionsList;
	if (pData == CIDataBuf0) {
		pDataBeforeMove = CIDataBuf1;
	}
	else {
		pDataBeforeMove = CIDataBuf0;
	}
	CIStoreMoves(pDataBeforeMove + cHeaderSize, pData + cHeaderSize, Hdr);

	bool bValidInputCreated = false;
	if (bCICalcMoves) {
		bValidInputCreated = CICreateInput(	pDataBeforeMove + cHeaderSize, 
											pData + cHeaderSize, Hdr);
	}
	//if ((rand() % 5) != 0) {
	//	rand();
	//}
	if (!bValidInputCreated) {
		CIInputSrcAndDest = (rand() % BOARD_SIZE) | ((rand() % BOARD_SIZE) << cInputShift)
			| ((rand() % BOARD_SIZE) << (2 * cInputShift))
			| ((rand() % BOARD_SIZE) << (3 * cInputShift))
			| 0x0 << (4 * cInputShift);
	}


}

void CheckersGetInput(bool& bMoveValid, int& xSrc, int& ySrc, int& xDest, int& yDest)
{
	ushort InputCopy = CIInputSrcAndDest;
	yDest = InputCopy & cOneInputMask; InputCopy >>= cInputShift;
	xDest = InputCopy & cOneInputMask; InputCopy >>= cInputShift;
	ySrc = InputCopy & cOneInputMask; InputCopy >>= cInputShift;
	xSrc = InputCopy & cOneInputMask; InputCopy >>= cInputShift;
	bMoveValid = (InputCopy & 0x1) != 0;
}


void InitCheckersForDB(	map<string, int>& BaseWordCount, map<string, int>& DepTypes, 
						vector<SSentenceRec>& SentenceList, map<string, vector<NameItem> >& BasicTypeLists, 
						map<string, SWordData>& MapWordToData, CPatGrpMgr& PatGrpMgr)
{
	//InitCheckersIntf(BaseWordCount);
	pCIBaseWordCount = &BaseWordCount;
	pCIDepTypes = &DepTypes;
	pCISentenceList = &SentenceList;
	pCIBasicTypeLists = &BasicTypeLists;
	pCIMapWordToData = &MapWordToData;
	pCIPatGrpMgr = &PatGrpMgr;

	CIDepNames.clear();
	CIDepNames.resize(DepTypes.size());
	MapBaseWordToCount::iterator itDep = DepTypes.begin();

	for (; itDep != DepTypes.end(); itDep++) {
		CIDepNames[itDep->second] = itDep->first;
	}

}

void CGotitEnv::PlayCheckers()
{
	string sbCalcMoves;
	bCICalcMoves = false;
	if (GetImplemParam(sbCalcMoves, "Implem.Param.FnParam.PlayCheckers.CalcMoves")) {
		if (sbCalcMoves == "Yes") {
			bCICalcMoves = true;
			bCIEvaluating = true;
		}
	}
	u64 TimeNow = TimerNow();
	CIDebugNumFindGrps = 0;
	CIDebugNumFindGrpByFID = 0;
	PlayOneGame(	&CheckersGetDataBuffer, &CheckersMoveComplete,
							&CheckersGetInput);

	if (((NumWhiteWins + NumBlackWins) % 10) == 0) {
		cerr << "White won " << NumWhiteWins << " at a rate of " << ((NumWhiteWins * 100) / (NumWhiteWins + NumBlackWins)) << "%\n";
	}
	cout	<< "Playing one game took " << TimerNow() - TimeNow 
			<< "us. Num finds: " << CIDebugNumFindGrps 
			<< " and " << CIDebugNumFindGrpByFID << endl;

}

void CGotitEnv::InitCheckers()
{
	// deprecated
	map<string, int> aCountMap;
	//MapBaseWordToCount DepCount;
	//MapBaseWordToCount NERCount;
	//MapBaseWordToCount POSCount;
	vector<NameItem> aNameItemVector;

	DynamicTypeListsNames.push_back("depcount");

	for (uint idtln = 0; idtln < DynamicTypeListsNames.size(); idtln++) {
		string& CountName = DynamicTypeListsNames[idtln];
		CountTypesMap[CountName] = aCountMap;
		map<string, vector<NameItem> >::iterator itbtl = BasicTypeLists.find(CountName);
		if (itbtl == BasicTypeLists.end()) {
			BasicTypeLists[CountName] = aNameItemVector;
		}
		else {
			vector<NameItem>& CountVector = BasicTypeLists[CountName];
			for (uint ic = 0; ic < CountVector.size(); ic++) {
				(CountTypesMap[CountName])[CountVector[ic].Name] = CountVector[ic].Count;
			}
		}
	}

	//vector<SSentenceRec> SentenceList;
	InitCheckersForDB(BaseWordCount, DepTypes, SentenceRec, BasicTypeLists, MapWordToData, PatGrpMgr);
	//srand(11171);
	CIRegexInit(5);

	NumBlackWins = 0;
	NumWhiteWins = 0;
}



