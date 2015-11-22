struct BoardDelta {
	BoardDelta(int ax, int ay, char Val, int aScore = 0) {
		x = ax; y = ay; NewVal = Val; Score = aScore;
	}
	bool operator ==(const BoardDelta& Other) const {
		return (	(x == Other.x) && (y == Other.y)
				&&	(NewVal == Other.NewVal) && (Score == Other.Score));
	}
	int x;
	int y;
	char NewVal;
	int Score;
};

struct OneMove {
	OneMove() {
		Score = 0;
	}
	vector <BoardDelta> Deltas;
	int Score;
};

class RuleProviderParent {
	
};

void CIRegexInit(int BoardSize);
int TestBrdValidByRegex(int BoardSize, char * pOldBoard, char * pNewBoard, 
						bool bWhiteTurn, int xSrc, int ySrc,
						RuleProviderParent * pProvider = NULL);
void CreateBasicRegPat(	int TblSize, char * pOldTbl, char * pNewTbl, 
						bool bWhiteTurn, int xSrc, int ySrc);
void TestLearnedRulesOnCurrentBoard(	int TblSize, char * pOldTbl,  
						bool bWhiteTurn);
void GoRegexInit(int BoardSize);
void ChessRegexInit(int BoardSize);
void GamerRegexGetValidMoves(	vector<OneMove >& Moves, int BoardSize,
								char * pOldBoard, bool bWhiteTurn, 
								RuleProviderParent * pProvider = NULL);
void GamerRegexGetValidMoves(	vector<OneMove >& Moves, int BoardSize,
								char * pOldBoard, bool bWhiteTurn, 
								int xStart, int yStart,
								RuleProviderParent * pProvider = NULL);
void GamerRegexGetValidMoves(	vector<OneMove >& Moves, int BoardSize,
								char * pOldBoard, bool bWhiteTurn, 
								int xStart, int yStart,
								int xEnd, int yEnd,
								RuleProviderParent * pProvider = NULL);
void GamerRegexExecConseq(	int& Score, int BoardSize, char * pOldBoard, 
							bool bWhiteTurn, bool bMoveFinal);
bool TestRegexLearnedRules(int TblSize, char * pOldTbl, bool bWhiteTurn);
