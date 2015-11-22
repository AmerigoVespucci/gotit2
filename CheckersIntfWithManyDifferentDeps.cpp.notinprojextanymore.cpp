// CheckersIntf.cpp : interfaces to a game of checkers							
//

#include "stdafx.h"

#include "MascReader.h"
#include "CheckersIntf.h"

static const int cDataBufSize = (BOARD_SIZE * BOARD_SIZE) + cHeaderSize;
const int cSampleSize = 5;
const int cLearnRandRate = 3;
const int cEvalRandRate = 97;

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
map<string, map<string, int>> CountTypesMap;
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


bool CIOptionsGT(SCIInputOption& O0, SCIInputOption& O1)
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

void CICreateSample(string& Sample, uchar * pBoard, int xAbsStart, int yAbsStart, int SampleSize)
{
	for (int iRow = 0; iRow < SampleSize; iRow++) {
		int yAbs = yAbsStart + iRow;
		for (int iCol = 0; iCol < SampleSize; iCol++) {
			int xAbs = xAbsStart + iCol;
			int iVec = (iRow * SampleSize) + iCol;
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
	static bool bLastValidTurnWhite = false;
	static int NumTimesCalled = 0;
	bool bWhiteTurn = ((Hdr & cWhiteTurn) != 0);
	bool bValidInput = ((Hdr & cInputValid) != 0);
	bool bTurnOver = ((Hdr & cTurnOver) != 0);
	bool bEndTurn = ((Hdr & cFinalTurn) != 0);
	bool bNextTurnWhite = bWhiteTurn != bTurnOver;

	if (bEndTurn) {
		pCISentenceList->push_back(SSentenceRec());
		string SampleTo = "o" + to_string(cSampleSize) + (!bWhiteTurn ? "w" : "b"); // declare winner not loser
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
		return;
	}
	if (bValidInput) {
		NumTimesCalled++;
		pCISentenceList->push_back(SSentenceRec());
		const int cMoveFromPos = 0; // First sample in sentence is the previous board where the move came from
		const int cMoveToPos = 1; // Second position is the sample to which the pice moved. Again, centered on the poistion of the piece, now the new position
		const int cMoveFollowingPos = 2; // Place left for next move to fill in. Then we will write retrocatively into this rec the from sample of that (the next) rec
		pCISentenceList->back().OneWordRec.resize(cMoveFollowingPos + 1);
		int yDest = Hdr & cOneInputMask; Hdr >>= cInputShift;
		int xDest = Hdr & cOneInputMask; Hdr >>= cInputShift;
		int ySrc = Hdr & cOneInputMask; Hdr >>= cInputShift;
		int xSrc = Hdr & cOneInputMask;
		static uchar cNegFlag = 0x08;
		static int cRelShift = 4;
		uchar yRelDest = ((ySrc > yDest) ? (cNegFlag | (ySrc - yDest)) : (yDest - ySrc));
		uchar xRelDest = ((xSrc > xDest) ? (cNegFlag | (xSrc - xDest)) : (xDest - xSrc));
		ushort InputWord = (bWhiteTurn ? cWhiteTurn : 0) | (yRelDest << cRelShift) | xRelDest;

		int xAbsStart = xSrc - (cSampleSize / 2);
		int yAbsStart = ySrc - (cSampleSize / 2);
		// Let's make this very clear. A b in the third letter of a sample (first letter s)
		// means the move AFTER this board was by black
		string SampleFrom = "s" + to_string(cSampleSize) + (bWhiteTurn ? "w" : "b");
		CICreateSample(SampleFrom, pOldBoard, xAbsStart, yAbsStart, cSampleSize);

		map<string, int>::iterator itWordCount = pCIBaseWordCount->find(SampleFrom);
		if (itWordCount != pCIBaseWordCount->end()) {
			itWordCount->second++;
		}
		else {
			(*pCIBaseWordCount)[SampleFrom] = 1;
		}

		xAbsStart = xDest - (cSampleSize / 2);
		yAbsStart = yDest - (cSampleSize / 2);
		string SampleTo = "s" + to_string(cSampleSize) + (bNextTurnWhite ? "w" : "b");
		CICreateSample(SampleTo, pNewBoard, xAbsStart, yAbsStart, cSampleSize);
		itWordCount = pCIBaseWordCount->find(SampleTo);
		if (itWordCount != pCIBaseWordCount->end()) {
			itWordCount->second++;
		}
		else {
			(*pCIBaseWordCount)[SampleTo] = 1;
		}

		string sMove = (bWhiteTurn ? "w" : "b");
		sMove += ((xDest >= xSrc) ? "+" : "-");
		sMove += '0' + abs(xDest - xSrc);
		sMove += ((yDest >= ySrc) ? "+" : "-");
		sMove += '0' + abs(yDest - ySrc);
		string sLeadsTo = string("l") + sMove;

		pCISentenceList->back().OneWordRec[cMoveFromPos].Word = SampleFrom;
		pCISentenceList->back().OneWordRec[cMoveToPos].Word = SampleTo;
		DepRec drec;

		//int iDep;
		//map<string, int>::iterator itdm = pCIDepTypes->find(sLeadsTo);
		//if (itdm == pCIDepTypes->end()) {
		//	iDep = pCIDepTypes->size();
		//	(*pCIDepTypes)[sLeadsTo] = iDep;
		//	//(CountTypesMap["depcount"])[sLeadsTo] = 0;
		//	((*pCIBasicTypeLists)["depcount"]).push_back(NameItem(sLeadsTo, 0));
		//}
		//else {
		//	iDep = itdm->second;
		//}
		//drec.iDep = (uchar)iDep;
		//drec.Gov = (uchar)0; // first of the only two words in the rec
		//drec.Dep = (uchar)1;
		//pCISentenceList->back().Deps.push_back(drec);
		////(CountTypesMap["depcount"])[sLeadsTo]++;

		int WhiteRegs[2] = { 0, 0 };
		int WhiteKings[2] = { 0, 0 };
		int BlackRegs[2] = { 0, 0 };
		int BlackKings[2] = { 0, 0 };
		vector<string> DepStarts;
		for (int iBoth = 0; iBoth < 2; iBoth++) {
			uchar * pBoard = pOldBoard;
			if (iBoth == 1) {
				pBoard = pNewBoard;
			}
			int ThisMoveScore = 0;
			for (int iRow = 0; iRow < BOARD_SIZE; iRow++) {
				for (int iCol = 0; iCol < BOARD_SIZE; iCol++) {
					int iVec = (iRow * BOARD_SIZE) + iCol;
					if (pBoard[iVec] == (BoardType)eBoardWhite) WhiteRegs[iBoth]++;
					if (pBoard[iVec] == (BoardType)eBoardWhiteKing) WhiteKings[iBoth]++;
					if (pBoard[iVec] == (BoardType)eBoardBlack) BlackRegs[iBoth]++;
					if (pBoard[iVec] == (BoardType)eBoardBlackKing) BlackKings[iBoth]++;
				}
			}
		}
		if ((WhiteRegs[1] > WhiteRegs[0]) && (WhiteKings[1] <= WhiteKings[0])) {
			DepStarts.push_back("tw");
		}
		if (WhiteKings[1] > WhiteKings[0]) {
			DepStarts.push_back("pw");
		}
		if (WhiteKings[1] < WhiteKings[0]) {
			DepStarts.push_back("tW");
		}
		if ((BlackRegs[1] > BlackRegs[0]) && (BlackKings[1] <= BlackKings[0])) {
			DepStarts.push_back("tb");
		}
		if (BlackKings[1] > BlackKings[0]) {
			DepStarts.push_back("pb");
		}
		if (BlackKings[1] < BlackKings[0]) {
			DepStarts.push_back("tB");
		}
		if (DepStarts.size() == 0) {
			DepStarts.push_back("l");
		}
		SSentenceRec* pPrevSentence;
		bool bCanFollow = false;
		if (pCISentenceList->size() > 1) { // not the first
			bCanFollow = true; // for now. We might change our mind
			pPrevSentence = &((*pCISentenceList)[pCISentenceList->size() - 2]); // I can add to the pointer but I prefer dereferencing it and using array notation instead
			string& FirstWordOfPrev = pPrevSentence->OneWordRec[0].Word;
			if (FirstWordOfPrev[0] == 'o') {
				bCanFollow = false; // the last record ended the previous game
			}
			else {
				DepStarts.push_back("f");
				pPrevSentence->OneWordRec[cMoveFollowingPos].Word = SampleFrom;
			}
		}

		vector<int> AccompanyPosList;
#ifdef DOING_NEIGHBORS
		const int cAccompanySize =1;
		for (int yA = ySrc - cAccompanySize; yA <= ySrc + cAccompanySize; yA++) {
			for (int xA = xSrc - cAccompanySize; xA < xSrc + cAccompanySize; xA++) {
				int iRow = yA - (cSampleSize / 2);
				int iCol = xA - (cSampleSize / 2);
				if ((iRow < -(cSampleSize / 2)) || iRow >= BOARD_SIZE) continue;
				if ((iCol < -(cSampleSize / 2)) || iCol >= BOARD_SIZE) continue;
				string SampleForAccompany = "a" + to_string(cSampleSize) + (bWhiteTurn ? "w" : "b");
				CICreateSample(	SampleForAccompany, pOldBoard,
								iCol, iRow,
								cSampleSize);
				string sA = string("a") + (bWhiteTurn ? "w" : "b");
				sA += ((xA >= xSrc) ? "+" : "-");
				sA += '0' + abs(xA - xSrc);
				sA += ((yA >= ySrc) ? "+" : "-");
				sA += '0' + abs(yA - ySrc);
				DepStarts.push_back(sA);
				WordRec wrec;
				wrec.Word = SampleForAccompany;
				AccompanyPosList.push_back(pCISentenceList->back().OneWordRec.size());
				pCISentenceList->back().OneWordRec.push_back(wrec);
			}
		}
#endif // #ifdef DOING_NEIGHBORS

		int iA = 0;
		for (uint itd = 0; itd < DepStarts.size(); itd++) {
			string sDep = DepStarts[itd] + sMove;
			if (DepStarts[itd][0] == 'f') {
				int iDepPrev = pPrevSentence->Deps[0].iDep;
				string& PrevDepName = CIDepNames[iDepPrev];
				string sPrevMove;
				if (PrevDepName[0] == 'l') {
					sPrevMove = PrevDepName.substr(1);
				}
				else if (PrevDepName[0] == 't') {
					sPrevMove = PrevDepName.substr(2);
				}
				sDep = DepStarts[itd] + sPrevMove;
			}
			else if (DepStarts[itd][0] == 'a') {
				sDep = DepStarts[itd];
			}
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
			if (DepStarts[itd][0] == 'f') {
				drec.Dep = (uchar)cMoveFollowingPos;
				pPrevSentence->Deps.push_back(drec);
			}
			else if (DepStarts[itd][0] == 'a') {
				drec.Dep = (uchar)(AccompanyPosList[iA]);
				pCISentenceList->back().Deps.push_back(drec);
				iA++;
			}
			else {
				drec.Dep = (uchar)cMoveToPos;
				pCISentenceList->back().Deps.push_back(drec);
			}
		}

	}

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
		bool bMoveFound = false;
		int MoveSpecStart;
		if ((sGrpSeed[0] == 'l')) {
//			&& (bNextTurnWhite == (sGrpSeed[1] == 'w'))){
			bMoveFound = true;
			NumMoves += 1.0f;
			MoveSpecStart = 2;
		}
		else if ((sGrpSeed[0] == 't')) {
	//		&& (bNextTurnWhite == (sGrpSeed[2] == 'w'))){
			bMoveFound = true;
			NumMoves += 1.0f;
			MoveSpecStart = 3;
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

bool CICreateInput(uchar * pOldBoard, uchar * pNewBoard, ushort Hdr)
{
	static bool bLastValidTurnWhite = false;
	static int NumTimesCalled = 0;
	bool bWhiteTurn = ((Hdr & cWhiteTurn) != 0);
	bool bValidInput = ((Hdr & cInputValid) != 0);
	bool bTurnOver = ((Hdr & cTurnOver) != 0);
	bool bEndTurn = ((Hdr & cFinalTurn) != 0);
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
	bool ret = PlayOneGame(	&CheckersGetDataBuffer, &CheckersMoveComplete,
							&CheckersGetInput);

	if (((NumWhiteWins + NumBlackWins) % 100) == 0) {
		cerr << "White won " << NumWhiteWins << " at a rate of " << ((NumWhiteWins * 100) / (NumWhiteWins + NumBlackWins)) << "%\n";
	}
	cout	<< "Playing one game took " << TimerNow() - TimeNow 
			<< "us. Num finds: " << CIDebugNumFindGrps 
			<< " and " << CIDebugNumFindGrpByFID << endl;

}

void CGotitEnv::InitCheckers()
{
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

	NumBlackWins = 0;
	NumWhiteWins = 0;
}



