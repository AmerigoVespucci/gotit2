// RegexLearn.cpp : Learn regex structures from examples
//

#include "stdafx.h"

#include "MascReader.h"
#include "Regex2D.h"
#include "RegexInternal.h"

vector<HeadRegexEl> WhiteRules;
vector<HeadRegexEl> BlackRules;
vector<SSentenceRec> History;

const int cRLHistoryFromBrd = 0;
const int cRLHistoryToBrd = 1;
const int cRLHistoryXSrc = 2;
const int cRLHistoryYSrc = 3;
const int cRLHistoryAccepted = 4;
const int cRLHistoryNumWords = 5;


struct SRuleData {
	SRuleData(int i, bool abWhiteRule ) {
		IndexInRules = i;
		bWhiteRule  = abWhiteRule;
	}
	bool bWhiteRule;
	int IndexInRules;
	vector<int> ExamplesInHistory;
};

vector<SRuleData> WhiteRulesData;
vector<SRuleData> BlackRulesData;

// This function tests one rule on one board.
// The rule is applied to all possible positions of the biard
// All the resulting moves are tested against the ACTUAL GAME ENGINE
// historical records are kept for every move the game engine responds to
// whether it is accepted as a valid move by the game engine or not

bool TestOneRuleOnOneBoard(	int TblSize, char * pOldTbl, 
								bool bWhiteTurn, string& sOldTbl,
								uint iRule)
{
	char TestBoard[TblSize * TblSize];
	string sNewTbl;
	vector<OneMove > NewRuleMoves;
	vector<HeadRegexEl>* pRules = &WhiteRules;
	vector<SRuleData>* pRulesData = & WhiteRulesData;
	if (!bWhiteTurn) {
		pRules = &BlackRules;
		pRulesData = &BlackRulesData;		
	}
	vector<HeadRegexEl> RulesVec;
	RulesVec.push_back((*pRules)[iRule]);
	RegexRuleProvider RecProvider(&RulesVec);

	// first create all the moves that arise from one learned rule
	GamerRegexGetValidMoves(NewRuleMoves, TblSize, pOldTbl, bWhiteTurn, &RecProvider);
	if (NewRuleMoves.size() == 0) {
		return true;
	}
	bool bAllRuleMovesAccepted = true;
	bool bAllMovesValid = true;
	for (int iMove = 0; iMove < NewRuleMoves.size(); iMove++) {
		memcpy(TestBoard, pOldTbl, TblSize * TblSize);
		int xStart, yStart;
		bool bStartValid = false;
		for (uint iFV = 0; iFV < NewRuleMoves[iMove].Deltas.size(); iFV++) {
			BoardDelta& rec = NewRuleMoves[iMove].Deltas[iFV];
			TestBoard[(rec.y * TblSize) + rec.x] = rec.NewVal;
			if (iFV == 0) {
				bStartValid = true;
				xStart = rec.x; yStart = rec.y;
			}
		}
		if (!bStartValid) {
			continue;
		}
		sNewTbl.clear();
		for (int yTbl = 0; yTbl < TblSize; yTbl++) {
			for (int xTbl = 0; xTbl < TblSize; xTbl++) {
				sNewTbl += '0' + TestBoard[(yTbl*TblSize) + xTbl];
			}
		}
		SSentenceRec SRec;
		SRec.OneWordRec.resize(cRLHistoryNumWords);
		SRec.OneWordRec[cRLHistoryFromBrd] = sOldTbl;
		SRec.OneWordRec[cRLHistoryToBrd] = sNewTbl;
		SRec.OneWordRec[cRLHistoryXSrc].Word = to_string(xStart);
		SRec.OneWordRec[cRLHistoryYSrc].Word = to_string(yStart);
		SRec.OneWordRec[cRLHistoryAccepted].Word = "t"; // until we find out otherwise
		// test against the game engine's rules
		if (TestBrdValidByRegex(TblSize, pOldTbl, TestBoard, bWhiteTurn, 
								xStart, yStart) < 0) {
			// the new board is unacceptable. There must be some condition
			// in the rule that we are not aware of
			SRec.OneWordRec[cRLHistoryAccepted].Word = "f"; 
			bAllRuleMovesAccepted = false;
		}
		History.push_back(SRec);
		(*pRulesData)[iRule].ExamplesInHistory.push_back(History.size()-1);
	}
	return (bAllRuleMovesAccepted);
}

/*
 * This function tests the learned rules on the current board.
 * It applies each of the rules to the board and gets all moves for 
 * that board for that rules. It then checks against the game engine 
 * whether it accepts those rules as valid. We expect it to do so since
 * the rules we learned should reflect the game engine's rules.
 * If any rule does not, we must restrict that rule based on history.
 */
void TestLearnedRulesOnCurrentBoard(int TblSize, char * pOldTbl, 
									bool bWhiteTurn)
{
	char TestBoard[TblSize * TblSize];
	vector<HeadRegexEl>* pRules = &WhiteRules;
	vector<SRuleData>* pRulesData = & WhiteRulesData;
	if (!bWhiteTurn) {
		pRules = &BlackRules;
		pRulesData = &BlackRulesData;		
	}
	string sOldTbl, sNewTbl;
	for (int yTbl = 0; yTbl < TblSize; yTbl++) {
		for (int xTbl = 0; xTbl < TblSize; xTbl++) {
			sOldTbl += '0' + pOldTbl[(yTbl*TblSize) + xTbl];
		}
	}
	vector<uint> ConflictedRuleIndices;
	for (uint ir = 0; ir < pRules->size(); ir++) {
		if (!(*pRules)[ir].bValid) continue;
		if (!TestOneRuleOnOneBoard(	TblSize, pOldTbl, 
									bWhiteTurn, sOldTbl,
									ir)) {
			ConflictedRuleIndices.push_back(ir);
		}
	}
//	for (uint icr = 0; icr < ConflictedRuleIndices.size(); icr++) {
//		uint iir = ConflictedRuleIndices[icr];
//	}
	for_each(ConflictedRuleIndices.begin(), ConflictedRuleIndices.end(), 
			[=] (uint iir) {
		// Here we split the rule into many rules, each marked by starting location
		//HeadRegexEl& CRule = (*pRules)[iir];
		map<pair<int, int>, int> Starts; // map for uniqueness
		// The following was written after a bug that invalidad the reference
		// after the vector was grown. It assumes the largest extent and then
		// shrinks it later
		pRulesData->reserve(pRulesData->size() + (*pRulesData)[iir].ExamplesInHistory.size()); 
		SRuleData& CRuleData = (*pRulesData)[iir];
		vector<int>& CRuleHistory  = CRuleData.ExamplesInHistory;
		for (uint irh = 0; irh < CRuleHistory.size(); irh++) {
			int iHistory = CRuleHistory[irh];
			SSentenceRec& Rec = History[iHistory];
			if (Rec.OneWordRec[cRLHistoryAccepted].Word != "t") {
				continue;
			}
			int CSrcX = stoi(Rec.OneWordRec[cRLHistoryXSrc].Word);
			int CSrcY = stoi(Rec.OneWordRec[cRLHistoryYSrc].Word);
			map<pair<int, int>, int>::iterator itStarts = Starts.find(make_pair(CSrcX, CSrcY));
			if (itStarts == Starts.end()) {
				HeadRegexEl NewRule = (*pRules)[iir];
				NewRule.AddStartCond(RegexCond(rptEQx, CSrcX));
				NewRule.AddStartCond(RegexCond(rptEQy, CSrcY));
				Starts[make_pair(CSrcX, CSrcY)] = pRules->size();
				pRulesData->push_back(SRuleData(pRules->size(), bWhiteTurn));
				pRulesData->back().ExamplesInHistory.push_back(iHistory);
				pRules->push_back(NewRule);
			}
			else {
				(*pRulesData)[itStarts->second].ExamplesInHistory.push_back(CRuleHistory[irh]);
			}
			
		}
		pRulesData->reserve(pRulesData->size() ); // shrink if library wants to
		(*pRules)[iir].bValid = false;
		
		
	});
//	}
}

// This function is called for one new move that previously learned rules did not predict.
// It tries to learn a new rule from this move. This rule may be over-general
// but we will rely on future moves to reveal and correct this over-generality
void CreateBasicRegPat(int TblSize, char * pOldTbl, char * pNewTbl, bool bWhiteTurn, int xSrc, int ySrc)
{
	vector<vector<uchar> > OldTbl;
	vector<vector<uchar> > NewTbl;
	OldTbl.resize(TblSize, vector<uchar>(TblSize, 0));
	NewTbl.resize(TblSize, vector<uchar>(TblSize, 0));

	vector<pair<int, int> > Changes;

	char TestBoard[TblSize * TblSize];
	
	vector<HeadRegexEl>* pRules = &WhiteRules;
	vector<SRuleData>* pRulesData = & WhiteRulesData;
	if (!bWhiteTurn) {
		pRules = &BlackRules;
		pRulesData = &BlackRulesData;		
	}

	/*
	 list the changes
	*/
	string sOldTbl, sNewTbl;
	
	for (int yTbl = 0; yTbl < TblSize; yTbl++) {
		for (int xTbl = 0; xTbl < TblSize; xTbl++) {
			OldTbl[yTbl][xTbl] = pOldTbl[(yTbl*TblSize) + xTbl];
			NewTbl[yTbl][xTbl] = pNewTbl[(yTbl*TblSize) + xTbl];
			sOldTbl += '0' + pOldTbl[(yTbl*TblSize) + xTbl];
			sNewTbl += '0' + pNewTbl[(yTbl*TblSize) + xTbl];
			if (OldTbl[yTbl][xTbl] != NewTbl[yTbl][xTbl]) {
				// assumption: one change per position
				Changes.push_back(make_pair(xTbl, yTbl));
			}
		}
	}

	if (Changes.size() == 0) 
		return;

	// the following is a place holder. The order needs serious work!
	// What it does for now is assume that the *first* change is the x/ySrc
	if (Changes.back().first == xSrc && Changes.back().second == ySrc) {
		reverse(Changes.begin(), Changes.end());
	}
	
	// create a historical record for this move
	SSentenceRec SRec;
	SRec.OneWordRec.resize(cRLHistoryNumWords);
	SRec.OneWordRec[cRLHistoryFromBrd] = sOldTbl;
	SRec.OneWordRec[cRLHistoryToBrd] = sNewTbl;
	SRec.OneWordRec[cRLHistoryXSrc].Word = to_string(xSrc);
	SRec.OneWordRec[cRLHistoryYSrc].Word = to_string(ySrc);
	History.push_back(SRec);
	SSentenceRec& LastHistory = History.back();
	

	/*
	vector<OneMove >& GivenMove;
	OneMove ThisMove;
	vector <BoardDelta>& Deltas = ThisMove.Deltas;
	for (uint iChange = 0; iChange < Changes.size(); iChange++) {
		BoardDelta OneD;
		pair<int, int>& Change = Changes[iChange];
		OneD.x = Change.first; OneD.y = Change.second;
		OneD.NewVal = NewTbl[OneD.x][OneD.y];
		OneD.Score = 0;
		Deltas.push_back(OneD);
	}
	GivenMove.push_back(ThisMove);
	RegexRuleProvider Provider(pRules);

	//GamerRegexGetValidMoves(GivenMove, TblSize, (char *)pOldTbl, bWhiteTurn, &Provider);
	 */
	
	/* Find which rule of the previously learned rules would create this move and add a 
	 referecnce to the history record to the rule's list. 
	 */
	RegexRuleProvider Provider(pRules);
	int iRuleForMove = TestBrdValidByRegex(	TblSize, pOldTbl, pNewTbl, bWhiteTurn, 
											xSrc, ySrc, &Provider);
	if (iRuleForMove >= 0) {
		if (iRuleForMove < pRulesData->size()) {
			(*pRulesData)[iRuleForMove].ExamplesInHistory.push_back(History.size() - 1);
		}
		return; // don't just leave. Do some more tests
	}

	

	// Create a recordong of the move in terms of RegexElChar
	// Go through the locations from first to last delta whether there is a delta there or not
	
	vector<RegexElChar> Moves;
	int xStart, yStart;
	for (uint iChange = 0; iChange < Changes.size(); iChange++) {
		pair<int, int>& Change = Changes[iChange];
		int xC = Change.first; int yC = Change.second;
		if (iChange == 0) {
			xStart = xC; yStart == yC;
			Moves.push_back(
				RegexElChar(OldTbl[yC][xC] + '0',
				RegexSideEffect(rsetSetFV, NewTbl[yC][xC]),
				rdtAny));
		}
		if (Changes.size() >(iChange + 1)) {
			pair<int, int>& NextChange = Changes[iChange + 1];
			int xN = NextChange.first; int yN = NextChange.second;
			int x = xC; int y = yC;
			int xDel = xN - x; int yDel = yN - y;
			while (true) {
				RegexDir rd;
				if (abs(yDel) > abs(xDel)) {
					if (yDel > 0) {
						y++; rd = rdtUp;
					}
					else {
						y--; rd = rdtDown;
					}
				}
				else {
					if (xDel > 0) {
						x++; rd = rdtRight;
					}
					else {
						x--; rd = rdtLeft;
						//x = x + copysign(1, xDel);
					}
				}
				xDel = xN - x; yDel = yN - y;
				RegexSideEffect rse;
				bool bDone = false;
				if ((xDel == 0) && (yDel == 0)) {
					rse = RegexSideEffect(rsetSetFV, NewTbl[y][x]);
					bDone = true;
				}
				else {
					rse = crseNone;
				}
				Moves.push_back(RegexElChar(OldTbl[y][x] + '0', rse, rd));
				if (bDone) break;
			}

		}
	}

	// Make a rule out of the moves
	
	HeadRegexEl RecordedMoves(Moves);
	vector<HeadRegexEl> RecMoves(1, RecordedMoves);
	RegexRuleProvider RecProvider(&RecMoves);
	
	enum eSpecifyOptions {
		esoXY,
		esoOutOfOptions,
		esoNumOptions
	};

	for (int is = 0; is < esoNumOptions; is++) {
		// Step 1 with the new rule: reverse the recording process. Take the new rule
		// and create any number of moves (a move is a series of move-steps)
		// Build a new board from these moves (a move is also a transform function
		// that inputs a board and outputs a new board) and check that at least one of
		// the boards created is identical with the result board originally provided
		// Why can there be multiple different moves? A simple move-step recorded
		// should not produce more than one, but later modifications might
		vector<OneMove > NewRuleMoves;
		RegexRule OneRule;
		if (OneRule.OnePosExec(	&RecordedMoves, TblSize, 
								pOldTbl, xSrc, ySrc)) {
			OneRule.CreateMoveDeltas(NewRuleMoves);
		}
#ifdef OLD_CODE		
		// generate moves from new rule, apply to board and test against
		// the actual (stored) rule 
		GamerRegexGetValidMoves(NewRuleMoves, TblSize, pOldTbl, bWhiteTurn, &RecProvider);
		if (NewRuleMoves.size() == 0) {
			cerr << "TestRegexLearnedRules: I don't understand why recorded move rule does not produce even one valid move\n";
			return;
		}
#endif // OLD_CODE		
		bool bAllMovesValid = true;
		bool bOneMoveMatched  = false;
		for (int iMove = 0; iMove < NewRuleMoves.size(); iMove++) {
			memcpy(TestBoard, pOldTbl, TblSize * TblSize);
			int xStart, yStart;
			bool bStartValid = false;
			for (uint iFV = 0; iFV < NewRuleMoves[iMove].Deltas.size(); iFV++) {
				BoardDelta& rec = NewRuleMoves[iMove].Deltas[iFV];
				TestBoard[(rec.y * TblSize) + rec.x] = rec.NewVal;
				if (iFV == 0) {
					bStartValid = true;
					xStart = rec.x; yStart = rec.y;
				}
			}
			if (!bStartValid) {
				continue;
			}
			if (memcmp(TestBoard, pNewTbl, sizeof(TestBoard)) == 0)  {
				bOneMoveMatched = true;
			}
#ifdef OLD_CODE
			if (TestBrdValidByRegex(TblSize, pOldTbl, TestBoard, bWhiteTurn, 
									xStart, yStart) < 0) {
				bAllMovesValid = false;
				break; // out of moves
			}
#endif // OLD_CODE			
		}
		if (!bOneMoveMatched) {
			cerr << "TestRegexLearnedRules: I don't understand why recorded move rule does not match given move.\n";
			return;
		}
		// go through all historical records
		// check against the possibility that the new rule would create a valid move
		// that the records show would happen diferently.
		// what we are looking for is an application to the same start point 
		// of an historical record but that rule produced a different result
		// this is NOT the same thing as an invalid move. For that we would need the
		// game engine to declare it invlid.
		for (uint iRec = 0; iRec < History.size(); iRec++) {
			SSentenceRec& HRec = History[iRec];

			int xHSrc = stoi(HRec.OneWordRec[cRLHistoryXSrc].Word);
			int yHSrc = stoi(HRec.OneWordRec[cRLHistoryYSrc].Word);

			string& sHOldTbl = HRec.OneWordRec[cRLHistoryFromBrd].Word;
			string& sHNewTbl = HRec.OneWordRec[cRLHistoryToBrd].Word;
			char pHOldTbl[TblSize * TblSize];
			char pHNewTbl[TblSize * TblSize];
			int pos = 0;
			for (int yTbl = 0; yTbl < TblSize; yTbl++) {
				for (int xTbl = 0; xTbl < TblSize; xTbl++, pos++) {
					pHOldTbl[pos] = sHOldTbl[pos] - '0';
					pHNewTbl[pos] = sHNewTbl[pos] - '0';
				}
			}
			
			NewRuleMoves.clear();
			OneRule.Init();
			if (OneRule.OnePosExec(	&RecordedMoves, TblSize, 
									pHOldTbl, xHSrc, yHSrc)) {
				OneRule.CreateMoveDeltas(NewRuleMoves);
			}

#ifdef OLD_CODE			
			GamerRegexGetValidMoves(NewRuleMoves, TblSize, pHOldTbl, bWhiteTurn, &RecProvider);
			if (NewRuleMoves.size() == 0) {
				continue;
			}
#endif // OLD_CODE			
			//add a method to RegexRule that creates the new board
			for (int iMove = 0; iMove < NewRuleMoves.size(); iMove++) {
				// among the moves, we are looking for moves that create the same board as that of the history rec
				if (NewRuleMoves[iMove].Deltas.size() == 0) {
					continue;
				}
				memcpy(TestBoard, pOldTbl, TblSize * TblSize);
				for (uint iFV = 0; iFV < NewRuleMoves[iMove].Deltas.size(); iFV++) {
					BoardDelta& rec = NewRuleMoves[iMove].Deltas[iFV];
					TestBoard[(rec.y * TblSize) + rec.x] = rec.NewVal;
				}
				if (memcmp(TestBoard, pHNewTbl, sizeof(TestBoard)) != 0)  {
					continue;
				}
				
				if (HRec.OneWordRec[cRLHistoryXSrc].Word == "f") {
					bAllMovesValid = false;
					break; // out of moves
				}
//				if (TestBrdValidByRegex(TblSize, pHOldTbl, pHNewTbl, bWhiteTurn, 
//										xStart, yStart) < 0) {
//					bAllMovesValid = false;
//					break; // out of moves
//				}
			}
			if (!bAllMovesValid) {
				break; // out of history
			}
		}
		if (bAllMovesValid) {
			LastHistory.OneWordRec[cRLHistoryAccepted].Word = "t";
			pRulesData->push_back(SRuleData(pRules->size(), bWhiteTurn));
			pRulesData->back().ExamplesInHistory.push_back(History.size()-1);
			pRules->push_back(RecordedMoves);
			break;
		}
		

		if (is == esoXY) {
			RecordedMoves.AddStartCond(RegexCond(rptEQx, xSrc));
			RecordedMoves.AddStartCond(RegexCond(rptEQy, ySrc));

		}
		else if (is == esoOutOfOptions) {
			cerr << "Error: out of options to specify rule further\n";
		}
	}
		
#ifdef OLD_CODE		
		//OEl = RecordedMoves;
		vector<bool> WhiteRuleConflicts;
		vector<bool> BlackRuleConflicts;

//		vector<RegexEl>* pRules = &WhiteRules;
//		vector<bool>* pConflicts = &WhiteRuleConflicts;
//		if (!bWhiteTurn) {
//			pRules = &BlackRules;
//			pConflicts = &BlackRuleConflicts;
//		}

		bool bMatchFound = false;
		//bConflicted = false;
		for (uint iRule = 0; iRule < pRules->size(); iRule++) {
			RegexEl& El = (*pRules)[iRule];
			bool bSEEquals;
			if (El.Equals(RecordedMoves, bSEEquals)) {
				if (!bSEEquals) {
					cerr << "Warning: Two rules equal and yet Side Effects differ\n";
					string sOut;
					El.Print(sOut); cerr << sOut << endl; sOut.clear();
					RecordedMoves.Print(sOut); cerr << sOut << endl;
					bConflicted = true;
					(*pConflicts)[iRule] = true;
				}
				else {
					bMatchFound = true;
					bConflicted = false;
					break; // out of list of rules
				}
			}
		}
		if (!bMatchFound && !bConflicted) {
			pRules->push_back(RecordedMoves);
			pConflicts->push_back(false);
			break; // out of looop that keeps trying to create a valid rule for 
		}
	} // keep trying until, at least, the new rule is not conflicted
#endif // OLD_CODE
}

bool TestRegexLearnedRules(int TblSize, char * pOldTbl, bool bWhiteTurn)
{
	vector<OneMove > Moves;
	char TestBoard[TblSize * TblSize];
	
	vector<HeadRegexEl>* pRules = &WhiteRules;
	if (!bWhiteTurn) {
		pRules = &BlackRules;
	}
	RegexRuleProvider Provider(pRules);

	GamerRegexGetValidMoves(Moves, TblSize, pOldTbl, bWhiteTurn, &Provider);
	if (Moves.size() == 0) {
		cerr << "TestRegexLearnedRules: Unable to even find valid move to test\n";
		return false;
	}
	int iSelMove = rand() % Moves.size();
	memcpy(TestBoard, pOldTbl, TblSize * TblSize);
	int xStart, yStart;
	bool bStartValid = false;
	for (uint iFV = 0; iFV < Moves[iSelMove].Deltas.size(); iFV++) {
		BoardDelta& rec = Moves[iSelMove].Deltas[iFV];
		TestBoard[(rec.y * TblSize) + rec.x] = rec.NewVal;
		if (iFV == 0) {
			bStartValid = true;
			xStart = rec.x; yStart = rec.y;
		}
	}
	if (!bStartValid) {
		cerr << "Error: TestRegexLearnedRules: rule selected does nothing\n";
		return false;
	}
	return (TestBrdValidByRegex(TblSize, pOldTbl, TestBoard, bWhiteTurn, xStart, yStart) >= 0);

}

