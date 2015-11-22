// CheckersText.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
//#include <tchar.h>

#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#include <vector>

using namespace std;

typedef unsigned char uchar ;
typedef unsigned int uint;
typedef unsigned short ushort;

#include "CheckersIntf.h"

int getxcoord(void);
int getycoord(void);
void showboard(int TheBoard[][BOARD_SIZE]);
char convert(int number);

//Checker Piece Class for Inheritance

// debug globals
bool xbOverrideInvalid;

struct SMove {
	int xSrc, ySrc, xDest, yDest;
};

class CheckerPiece
{
protected:
	int x;
	int y;
	bool bWhite;
	bool bKing;
	bool bSelected;
	bool bLive;
public:
	CheckerPiece(int startx, int starty, bool baWhite = false);

	static int Find(CheckerPiece Pieces[], int NumPieces, bool bFindWhite,
					int x, int y);
	static int CheckJumps(	CheckerPiece Pieces[], int NumPieces,
							bool bWhite,
							BoardType TheBoard[][BOARD_SIZE]);
	static int CheckMoves(	CheckerPiece Pieces[], int NumPieces,
							bool bWhite,
							BoardType TheBoard[][BOARD_SIZE]);

	int getx(void);
	int gety(void);
	void select(void);
	void unselect(void);
	bool isSelected(void);
	bool isLive() {	return bLive;}
	bool Move(	int nextx, int nexty, BoardType TheBoard[][BOARD_SIZE]);
	bool Jump(	int nextx, int nexty, CheckerPiece Pieces[], int NumPieces, 
				BoardType TheBoard[][BOARD_SIZE]);
	bool CanJump(BoardType TheBoard[][BOARD_SIZE]);
	int CanJump(BoardType TheBoard[][BOARD_SIZE], vector<SMove>& Options);
	bool CanMove(BoardType TheBoard[][BOARD_SIZE]);
	int CanMove(BoardType TheBoard[][BOARD_SIZE], vector<SMove>& Options);
	void SetBoardPos(BoardType TheBoard[][BOARD_SIZE]);
	bool isWhite() { return bWhite; }
	bool isKing() { return bKing;  }
	void Kill() { bLive = false; }
	void KingMaker(BoardType TheBoard[][BOARD_SIZE]);
};


void FindValidMoves(CheckerPiece Pieces[], int NumPieces,
					bool bWhite,
					BoardType TheBoard[][BOARD_SIZE],
					vector<SMove>& Options)
{

	bool bOnlyJumps = false;

	for (int iPiece = 0; iPiece < NumPieces; iPiece++) {
		CheckerPiece& Piece = Pieces[iPiece];
		if (!Piece.isLive()) {
			continue;
		}
		if (Piece.isWhite() != bWhite) {
			continue;
		}
#ifdef MUST_JUMP
		int NumJumps = Piece.CanJump(TheBoard, Options);
		if (!bOnlyJumps && (NumJumps > 0)) {
			bOnlyJumps = true;
			Options.clear(); 
			Piece.CanJump(TheBoard, Options);// again
		}
#endif
		if (!bOnlyJumps) {
			Piece.CanMove(TheBoard, Options);
		}
	}
}

int CheckerPiece::getx(void)
{
	return(x);
}

int CheckerPiece::gety(void)
{
	return(y);
}

void CheckerPiece::select(void)
{
	bSelected = true;
}

void CheckerPiece::unselect(void)
{
	bSelected = false;
}

bool CheckerPiece::isSelected(void)
{
	return(bSelected);
}

CheckerPiece::CheckerPiece(int startx, int starty, bool baWhite)
{
	x = startx;
	y = starty;
	bKing = false;
	bWhite = baWhite;
	bLive = true;
	bSelected = false;
}

void CheckerPiece::SetBoardPos(BoardType TheBoard[][BOARD_SIZE])
{
	BoardType btPiece = (bKing ? (bWhite ? eBoardWhiteKing : eBoardBlackKing)
		: (bWhite ? eBoardWhite : eBoardBlack));
	if (bLive) {
		TheBoard[y][x] = btPiece;
	}
}

void CheckerPiece::KingMaker(BoardType TheBoard[][BOARD_SIZE])
{
	if (!bLive) {
		return;
	}
	if (bWhite) {
		if (!bKing && (y == (BOARD_SIZE - 1))) {
			bKing = true;
			TheBoard[y][x] = eBoardWhiteKing;
		}
	}
	else {
		if (!bKing && (y == 0)) {
			bKing = true;
			TheBoard[y][x] = eBoardBlackKing;
		}
	}
}

bool CheckerPiece::Move(int nextx, int nexty, BoardType TheBoard[][BOARD_SIZE])
{
	BoardType btMoving = (bKing	? (bWhite ? eBoardWhiteKing : eBoardBlackKing) 
								: (bWhite ? eBoardWhite : eBoardBlack));

	for (int yDel = -1; yDel < 2; yDel += 2) {
		if (!bKing && bWhite && yDel < 0) {
			continue;
		}
		if (!bKing && !bWhite && yDel > 0) {
			continue;
		}
		for (int xDel = -1; xDel < 2; xDel += 2) {
			if (	nextx == (x + xDel) && nexty == (y + yDel) 
				&&	TheBoard[nexty][nextx] == eBoardNone)	{
				TheBoard[y][x] = eBoardNone;
				TheBoard[nexty][nextx] = btMoving;

				//ToScreen << "moved  from " << x << "," << y << " ";
				x = nextx;	y = nexty;
				//ToScreen << "to " << x << "," << y << " " << endl;
				KingMaker(TheBoard);
				return true;
			}
		}
	}
	return false;
}

int CheckerPiece::CheckMoves(	CheckerPiece Pieces[], int NumPieces,
								bool bWhite,
								BoardType TheBoard[][BOARD_SIZE])
{
	for (int iPiece = 0; iPiece < NumPieces; iPiece++) {
		CheckerPiece& Piece = Pieces[iPiece];
		if (!Piece.bLive) {
			continue;
		}
		if (Piece.isWhite() != bWhite) {
			continue;
		}
		if (Piece.CanMove(TheBoard)) {
			return iPiece;
		}
	}
	return -1;
}


bool CheckerPiece::CanMove(BoardType TheBoard[][BOARD_SIZE])
{
	//BoardType btMoving = (bKing ? (bWhite ? eBoardWhiteKing : eBoardBlackKing)
	//	: (bWhite ? eBoardWhite : eBoardBlack));

	for (int yDel = -1; yDel < 2; yDel += 2) {
		if (!bKing && bWhite && yDel < 0) {
			continue;
		}
		if (!bKing && !bWhite && yDel > 0) {
			continue;
		}
		for (int xDel = -1; xDel < 2; xDel += 2) {
			int xEnd = x + xDel;
			int yEnd = y + yDel;
			if (	(xEnd >= 0) && (xEnd < BOARD_SIZE)
				&&	(yEnd >= 0) && (yEnd < BOARD_SIZE)
				&&	(TheBoard[yEnd][xEnd] == eBoardNone)) {
				return true;
			}
		}
	}
	return false;
}

int CheckerPiece::CanMove(BoardType TheBoard[][BOARD_SIZE], vector<SMove>& Options)
{
	int NumMoves = 0;
	//BoardType btMoving = (bKing ? (bWhite ? eBoardWhiteKing : eBoardBlackKing)
	//	: (bWhite ? eBoardWhite : eBoardBlack));

	for (int yDel = -1; yDel < 2; yDel += 2) {
		if (!bKing && bWhite && yDel < 0) {
			continue;
		}
		if (!bKing && !bWhite && yDel > 0) {
			continue;
		}
		for (int xDel = -1; xDel < 2; xDel += 2) {
			int xEnd = x + xDel;
			int yEnd = y + yDel;
			if ((xEnd >= 0) && (xEnd < BOARD_SIZE)
				&& (yEnd >= 0) && (yEnd < BOARD_SIZE)
				&& (TheBoard[yEnd][xEnd] == eBoardNone)) {
				SMove Move = { x, y, x + xDel, y + yDel };
				Options.push_back(Move);
				NumMoves++;
			}
		}
	}
	return NumMoves;
}


bool CheckerPiece::Jump(int nextx, int nexty, CheckerPiece Pieces[], 
						int NumPieces, BoardType TheBoard[][BOARD_SIZE])
{
	BoardType btMoving = (bKing ? (bWhite ? eBoardWhiteKing : eBoardBlackKing)
		: (bWhite ? eBoardWhite : eBoardBlack));
	BoardType btOppReg = (bWhite ? eBoardBlack : eBoardWhite);
	BoardType btOppKing = (bWhite ? eBoardBlackKing : eBoardWhiteKing);

	for (int yDel = -2; yDel < 3; yDel += 4) {
		if (!bKing && bWhite && yDel < 0) {
			continue;
		}
		if (!bKing && !bWhite && yDel > 0) {
			continue;
		}
		for (int xDel = -2; xDel < 3; xDel += 4) {
			int yMid = y + (yDel / 2);
			int xMid = x + (xDel / 2);
			BoardType btLeaped = TheBoard[yMid][xMid];
			if (	nextx == (x + xDel) && nexty == (y + yDel)
				&&	TheBoard[nexty][nextx] == eBoardNone
				&&	((btLeaped == btOppReg) || (btLeaped == btOppKing)))	{
				TheBoard[y][x] = eBoardNone;
				TheBoard[nexty][nextx] = btMoving;

				int iPiece = CheckerPiece::Find(	Pieces, NumPieces, !bWhite, 
													xMid, yMid);
				if (iPiece == -1) {
					cerr << "Strange! Board says that there is a piece to jump over but not found\n";
				}
				else {
					CheckerPiece& Piece = Pieces[iPiece];
					TheBoard[yMid][xMid] = eBoardNone;
					Piece.Kill();
				}
				//ToScreen << "moved  from " << x << "," << y << " ";
				x = nextx;	y = nexty;
				//ToScreen << "to " << x << "," << y << " " << endl;
				KingMaker(TheBoard);
				return true;
			}
		}
	}
	return false;
}

int CheckerPiece::CheckJumps(	CheckerPiece Pieces[], int NumPieces,
								bool bWhite,
								BoardType TheBoard[][BOARD_SIZE])
{
	for (int iPiece = 0; iPiece < NumPieces; iPiece++) {
		CheckerPiece& Piece = Pieces[iPiece];
		if (!Piece.bLive) {
			continue;
		}
		if (Piece.isWhite() != bWhite) {
			continue;
		}
		if (Piece.CanJump(TheBoard)) {
			return iPiece;
		}
	}
	return -1;
}

bool CheckerPiece::CanJump(	BoardType TheBoard[][BOARD_SIZE])
{
	//BoardType btMoving = (bKing ? (bWhite ? eBoardWhiteKing : eBoardBlackKing)
	//	: (bWhite ? eBoardWhite : eBoardBlack));
	BoardType btOppReg = (bWhite ? eBoardBlack : eBoardWhite);
	BoardType btOppKing = (bWhite ? eBoardBlackKing : eBoardWhiteKing);

	for (int yDel = -2; yDel < 3; yDel += 4) {
		if (!bKing && bWhite && yDel < 0) {
			continue;
		}
		if (!bKing && !bWhite && yDel > 0) {
			continue;
		}
		for (int xDel = -2; xDel < 3; xDel += 4) {
			int yMid = y + (yDel / 2);
			int xMid = x + (xDel / 2);
			int xEnd = x + xDel;
			int yEnd = y + yDel;
			if (	(xEnd >= 0) && (xEnd < BOARD_SIZE) 
				&&	(yEnd >= 0) && (yEnd < BOARD_SIZE) 
				&&	TheBoard[yEnd][xEnd] == eBoardNone
				&&	(	(TheBoard[yMid][xMid] == btOppReg) 
					||	(TheBoard[yMid][xMid] == btOppKing)))	{
				return true;
			}
		}
	}
	return false;
}

int CheckerPiece::CanJump(BoardType TheBoard[][BOARD_SIZE], vector<SMove>& Options)
{
	int Ret = 0;
	//BoardType btMoving = (bKing ? (bWhite ? eBoardWhiteKing : eBoardBlackKing)
	//	: (bWhite ? eBoardWhite : eBoardBlack));
	BoardType btOppReg = (bWhite ? eBoardBlack : eBoardWhite);
	BoardType btOppKing = (bWhite ? eBoardBlackKing : eBoardWhiteKing);

	for (int yDel = -2; yDel < 3; yDel += 4) {
		if (!bKing && bWhite && yDel < 0) {
			continue;
		}
		if (!bKing && !bWhite && yDel > 0) {
			continue;
		}
		for (int xDel = -2; xDel < 3; xDel += 4) {
			int yMid = y + (yDel / 2);
			int xMid = x + (xDel / 2);
			int xEnd = x + xDel;
			int yEnd = y + yDel;
			if ((xEnd >= 0) && (xEnd < BOARD_SIZE)
				&& (yEnd >= 0) && (yEnd < BOARD_SIZE)
				&& TheBoard[yEnd][xEnd] == eBoardNone
				&& ((TheBoard[yMid][xMid] == btOppReg)
				|| (TheBoard[yMid][xMid] == btOppKing)))	{
				SMove Move = { x, y, x + xDel, y + yDel };
				Options.push_back(Move);
				Ret++;
			}
		}
	}
	return Ret;
}

//CLASS-RELATED PROTOTYPES


////////
//MAIN//
////////

#define ToScreen cerr


//FUNCTIONS

int getxcoord(void)
{
	char letter;
	int x = 8;

	ToScreen << "a - h: ";

	while (x == 8)
	{
		cin >> letter;
		//letter = toupper(letter);
		if (letter < 'a' || letter > 'h') {
			ToScreen << "Please give a letter from a to h: ";
			continue;
		}
		x = letter - 'a';
	}

	return(x);
}

int getycoord(void)
{
	int y = 9;

	ToScreen << "1 - 8: ";
	cin >> y;
	y--;
	while (y < 0 || y > 7)
	{
		ToScreen << "Please give a number from 1 to 8: ";
		cin >> y;
		y--;
	}

	return(y);
}

int CheckerPiece::Find(CheckerPiece Pieces[], int NumPieces, bool bFindWhite, int x, int y)
{
	int finder = -1;

	for (finder = 0; finder < NumPieces; finder++)
	{
		if ((Pieces[finder].getx() == x)
			&& (Pieces[finder].gety() == y)
			&& (Pieces[finder].isLive())
			&& (bFindWhite == Pieces[finder].isWhite())) {
			Pieces[finder].select();
			return(finder);
		}
	}

	return(-1);
}

char DisplayBoardPos(BoardType bt)
{
	switch (bt)
	{
	case eBoardNone:   return(' ');
	case eBoardWhite:   return('w');
	case eBoardBlack:   return('b');
	case eBoardWhiteKing:   return('W');
	case eBoardBlackKing:   return('B');
	default:   return('!');
	}
}


void DisplayBoard(BoardType TheBoard[][BOARD_SIZE])
{
	//ToScreen << "  |----|----|----|----|----|----|----|----|" << endl;
	ToScreen << "  |" ;
	for (int iCol = 0; iCol < BOARD_SIZE; iCol++)
	{
		ToScreen << "----|";
	}
	ToScreen << endl;

	for (int iRow = BOARD_SIZE - 1; iRow >= 0; iRow--)
	{
		ToScreen << iRow+1 << " ";
		for (int iCol = 0; iCol < BOARD_SIZE; iCol++)
		{
			ToScreen << "| " << DisplayBoardPos(TheBoard[iRow][iCol]) << "  ";
		}
		ToScreen << "|" << endl << "  |"  ;
		for (int iCol = 0; iCol < BOARD_SIZE; iCol++)
		{
			ToScreen << "----|";
		}
		ToScreen << endl;


		//ToScreen << "|" << endl << "  |----|----|----|----|----|----|----|----|" << endl;
	}
	ToScreen << "  _" ;
	for (int iCol = 0; iCol < BOARD_SIZE; iCol++)
	{
		ToScreen << "_" << (char)('a' + iCol) << "___" ;
	}
	ToScreen << endl;
}

void CreateSnapshot(pfGetDataBuffer GetData, pfBufferWritten DataDone, CheckerPiece Pieces[],
					bool bWhiteTurn, bool bValidInput, bool bTurnOver, bool bEndTurn, 
					int xSrc, int ySrc, int xDest, int yDest,
					int NumPieces, BoardType TheBoard[][BOARD_SIZE])
{
	int SizeData = cHeaderSize + (BOARD_SIZE * BOARD_SIZE);
	uchar * pData;
	bool bRet = GetData(pData, SizeData);
	if (!bRet || pData == NULL ) {
		cerr << "Insufficient space to store snapshot\n";
		return;
	}
	/*
	Header: 4 bytes
	1 bit whose turn - white 1
	1 bit input valid
	1 bit turn over
	1 bit final turn
	4x3 bits source and dest input
	body size: 0 - 24 bytes. Not implementing "0 means no change" yet
	Each byte of body is one piece
	1 bit king - 0 reg
	1 bit white - 0 black
	3 bits x 0-7
	3 bits y 0-7
	*/

	//int NumLivePieces = 0;
	uchar * pCurr = pData + cHeaderSize;
	//for (int iPiece = 0; iPiece < NumPieces; iPiece++) {
	//	CheckerPiece& Piece = Pieces[iPiece];
	//	if (!Piece.isLive()) {
	//		continue;
	//	}
	//	uchar Data = 0;
	//	Data |= (Piece.isKing() ? cKingNotReg : 0);
	//	Data |= (Piece.isWhite() ? cWhiteNotBlack : 0);
	//	Data |= (Piece.getx() << cXPosBitShift) & cXPosMask;
	//	Data |= (Piece.gety() << cYPosBitShift) & cYPosMask;
	//	*pCurr = Data;
	//	NumLivePieces++;
	#//}
	for (int iRow = 0; iRow < BOARD_SIZE; iRow++) {
		for (int iCol = 0; iCol < BOARD_SIZE; iCol++, pCurr++) {
			*pCurr = (uchar)TheBoard[iRow][iCol];
		}
	}
	
	ushort Hdr = 0;
	Hdr |= xSrc; Hdr <<= cInputShift;
	Hdr |= ySrc; Hdr <<= cInputShift;
	Hdr |= xDest; Hdr <<= cInputShift;
	Hdr |= yDest; 
	Hdr |= (bWhiteTurn ? cWhiteTurn : 0);
	Hdr |= (bValidInput ? cInputValid : 0);
	Hdr |= (bTurnOver ? cTurnOver : 0);
	Hdr |= (bEndTurn ? cFinalTurn : 0);
	*(ushort *)pData = Hdr;

	DataDone(pData);

}

bool PlayOneGame(pfGetDataBuffer GetData, pfBufferWritten DataDone, pfGetInput GetInput)
{
	//int TheBoard[8][8] = {	1, 0, 1, 0, 1, 0, 1, 0,
	//						0, 1, 0, 1, 0, 1, 0, 1,
	//						1, 0, 1, 0, 1, 0, 1, 0,
	//						0, 0, 0, 0, 0, 0, 0, 0,
	//						0, 0, 0, 0, 0, 0, 0, 0,
	//						0, 2, 0, 2, 0, 2, 0, 2,
	//						2, 0, 2, 0, 2, 0, 2, 0,
	//						0, 2, 0, 2, 0, 2, 0, 2 };
	BoardType TheBoard[BOARD_SIZE][BOARD_SIZE];
	memset(TheBoard, (int)eBoardNone, sizeof(TheBoard));
	int xposition;
	int yposition;
	int xposition2;
	int yposition2;
	int iSel;

	const bool cbWhite = true;
#if BOARD_SIZE == 8
#if NUM_PIECES == 12
	CheckerPiece Pieces[] = {	CheckerPiece(0, 0, cbWhite), CheckerPiece(2, 0, cbWhite), CheckerPiece(4, 0, cbWhite), CheckerPiece(6, 0, cbWhite),
								CheckerPiece(1, 1, cbWhite), CheckerPiece(3, 1, cbWhite), CheckerPiece(5, 1, cbWhite), CheckerPiece(7, 1, cbWhite),
								CheckerPiece(0, 2, cbWhite), CheckerPiece(2, 2, cbWhite), CheckerPiece(4, 2, cbWhite), CheckerPiece(6, 2, cbWhite) ,
								CheckerPiece(1, 5, !cbWhite), CheckerPiece(3, 5, !cbWhite), CheckerPiece(5, 5, !cbWhite), CheckerPiece(7, 5, !cbWhite),
								CheckerPiece(0, 6, !cbWhite), CheckerPiece(2, 6, !cbWhite), CheckerPiece(4, 6, !cbWhite), CheckerPiece(6, 6, !cbWhite),
								CheckerPiece(1, 7, !cbWhite), CheckerPiece(3, 7, !cbWhite), CheckerPiece(5, 7, !cbWhite), CheckerPiece(7, 7, !cbWhite) };
#elif NUM_PIECES == 8
	CheckerPiece Pieces[] = {	CheckerPiece(0, 0, cbWhite), CheckerPiece(2, 0, cbWhite), CheckerPiece(4, 0, cbWhite), CheckerPiece(6, 0, cbWhite),
								CheckerPiece(1, 1, cbWhite), CheckerPiece(3, 1, cbWhite), CheckerPiece(5, 1, cbWhite), CheckerPiece(7, 1, cbWhite),
								CheckerPiece(0, 6, !cbWhite), CheckerPiece(2, 6, !cbWhite), CheckerPiece(4, 6, !cbWhite), CheckerPiece(6, 6, !cbWhite),
								CheckerPiece(1, 7, !cbWhite), CheckerPiece(3, 7, !cbWhite), CheckerPiece(5, 7, !cbWhite), CheckerPiece(7, 7, !cbWhite)
	};
#elif NUM_PIECES == 4
	CheckerPiece Pieces[] = {	CheckerPiece(0, 0, cbWhite), CheckerPiece(2, 0, cbWhite), CheckerPiece(4, 0, cbWhite), CheckerPiece(6, 0, cbWhite),
		CheckerPiece(1, 7, !cbWhite), CheckerPiece(3, 7, !cbWhite), CheckerPiece(5, 7, !cbWhite), CheckerPiece(7, 7, !cbWhite)
	};
#elif NUM_PIECES == 3
	CheckerPiece Pieces[] = {	CheckerPiece(0, 0, cbWhite), CheckerPiece(2, 0, cbWhite), CheckerPiece(4, 0, cbWhite), 
								CheckerPiece(1, BOARD_SIZE-1, !cbWhite), CheckerPiece(3, BOARD_SIZE-1, !cbWhite), CheckerPiece(5, BOARD_SIZE-1, !cbWhite)  };
#elif NUM_PIECES == 1
	CheckerPiece Pieces[] = { CheckerPiece(0, 0, cbWhite), CheckerPiece(7, 7, !cbWhite) };
#endif 
//#elif BOARD_SIZE == 6
#else // BOARD_SIZE
#if NUM_PIECES == 3
	CheckerPiece Pieces[] = {	CheckerPiece(0, 0, cbWhite), CheckerPiece(2, 0, cbWhite), CheckerPiece(4, 0, cbWhite), 
								CheckerPiece(0, BOARD_SIZE-1, !cbWhite), CheckerPiece(2, BOARD_SIZE-1, !cbWhite), CheckerPiece(4, BOARD_SIZE-1, !cbWhite)  };
#elif NUM_PIECES == 2
	CheckerPiece Pieces[] = { CheckerPiece(0, 0, cbWhite), CheckerPiece(2, 0, cbWhite),
		CheckerPiece(0, BOARD_SIZE - 1, !cbWhite), CheckerPiece(2, BOARD_SIZE - 1, !cbWhite) };
#elif NUM_PIECES == 1
	CheckerPiece Pieces[] = { CheckerPiece(0, 0, cbWhite), CheckerPiece(0, BOARD_SIZE - 1, !cbWhite) };
#endif // NUM_PICES
	//CheckerPiece Pieces[] = {	CheckerPiece(0, 0, cbWhite),
	//							CheckerPiece(0, BOARD_SIZE-1, !cbWhite) };
#endif // BOARD_SIZE
	int NumPieces = sizeof(Pieces) / sizeof(Pieces[0]);

	for (int iPiece = 0; iPiece < NumPieces; iPiece++) {
		Pieces[iPiece].SetBoardPos(TheBoard);
	}
	int turn = 0;
	if ((rand() % 5) != 0) {
		rand();
	}
	bool bTurnWhite = (rand() % 2) != 0; // false; //  
	//bool bTurnWhite = true; //  
	bool bHumanPlaying = false;
	bool bShowGames = true;
	bool bGetInputFromProc = true;
	const bool cbInputValid = true;
	//const bool cbWhiteWin = true;
	const bool cbTurnOver = true;
	const bool cbLastTurn = true;
#ifdef MUST_JUMP
	const bool cbMustJump = true;
#else
	const bool cbMustJump = false;
#endif

	xbOverrideInvalid = false;

	if (bShowGames) {
		DisplayBoard(TheBoard);
	}

	CreateSnapshot(GetData, DataDone, Pieces,
					bTurnWhite, !cbInputValid, !cbTurnOver, !cbLastTurn,
					7,7,7,7,
					NumPieces, TheBoard);

	int LastValidTurn = 0;
	int ValidTurnsInARow = 0;

	while (turn <10000000)
	{
		turn++;
		//if (turn == 1083) {
		//	ToScreen << "there\n";
		//}
		if (	(CheckerPiece::CheckMoves(Pieces, NumPieces, bTurnWhite, TheBoard) < 0)
			&&	(CheckerPiece::CheckJumps(Pieces, NumPieces, bTurnWhite, TheBoard) < 0)) {
			if (bShowGames) {
				ToScreen << (bTurnWhite ? "White" : "Black") << " lost!" << endl;
			}
			//ToScreen << turn << endl;
			CreateSnapshot(	GetData, DataDone, Pieces,
							bTurnWhite, !cbInputValid, cbTurnOver, cbLastTurn, 
							xposition, yposition, xposition2, yposition2,
							NumPieces, TheBoard);
			return bTurnWhite;
		}
		if (bTurnWhite && bHumanPlaying) {
			ToScreen << "=-WHITE'S TURN-=" << endl;
			ToScreen << "Enter coordinates of piece to move" << endl;
			xposition = getxcoord();
			yposition = getycoord();


			while ((iSel = CheckerPiece::Find(	Pieces, NumPieces, bTurnWhite, 
												xposition, yposition)) < 0) {
				ToScreen << "No White Piece found - Enter coordinates of piece to move" << endl;
				xposition = getxcoord();
				yposition = getycoord();
			}

			ToScreen << xposition << ", " << yposition << endl;

			ToScreen << "Where do you want to move it?" << endl;
			xposition2 = getxcoord();
			yposition2 = getycoord();

			ToScreen << xposition2 << ", " << yposition2 << endl;
		} // end if white's turn
		else {
			if (bGetInputFromProc) {
				bool bMoveValid = false;
				GetInput(bMoveValid, xposition, yposition, xposition2, yposition2);
				if (!bMoveValid) {
					vector<SMove> Options;
					FindValidMoves(Pieces, NumPieces, bTurnWhite, TheBoard, Options);
					if (Options.size() > 0) {
						int iMove = rand() % Options.size();
						SMove& Move = Options[iMove];
						xposition = Move.xSrc; yposition = Move.ySrc;
						xposition2 = Move.xDest; yposition2 = Move.yDest;
					}
				}
			}
			else {
				xposition = rand() % BOARD_SIZE;
				yposition = rand() % BOARD_SIZE;
				xposition2 = rand() % BOARD_SIZE;
				yposition2 = rand() % BOARD_SIZE;

			}
			if ((iSel = CheckerPiece::Find(Pieces, NumPieces, bTurnWhite,
											xposition, yposition)) < 0) {
				CreateSnapshot(GetData, DataDone, Pieces,
								bTurnWhite, !cbInputValid, !cbTurnOver, !cbLastTurn,
								xposition, yposition, xposition2, yposition2,
								NumPieces, TheBoard);
				if (bTurnWhite && bHumanPlaying) {
					ToScreen << "Not a valid move - Re-enter piece to move" << endl;
					DisplayBoard(TheBoard);
				}
				continue;
			}
		}

		//while (CheckerPiece::Find(Pieces, NumPieces, cbWhite, xposition2, yposition2) >= 0)
		//{
		//	//ToScreen << CheckerPiece::Find(whitepiece, xposition2, yposition2) << endl;
		//	ToScreen << "Already a Piece there - Enter different place to move" << endl;
		//	xposition2 = getxcoord();
		//	yposition2 = getycoord();
		//}

		if (	(	!cbMustJump 
				||	(CheckerPiece::CheckJumps(	Pieces, NumPieces, 
												bTurnWhite, TheBoard) < 0))
			&&	Pieces[iSel].Move(xposition2, yposition2, TheBoard)) {
		}
		else if (Pieces[iSel].Jump(xposition2, yposition2, Pieces, NumPieces, TheBoard)) {
#ifdef KEEP_JUMING
			if (CheckerPiece::CheckJumps(Pieces, NumPieces, bTurnWhite, TheBoard) >= 0) {
				if (bShowGames) {
					if (bTurnWhite && bHumanPlaying) {
						ToScreen << "Good jump. Move again!" << endl;
						DisplayBoard(TheBoard);
					}
					if (!(bTurnWhite && bHumanPlaying) || bShowGames) {
						ToScreen << (bTurnWhite ? "White " : "Black ") << (char)('a' + xposition) << yposition + 1
								<< " to " << (char)('a' + xposition2) << yposition2 + 1 
								<< endl;
						DisplayBoard(TheBoard);
					}
				}
				CreateSnapshot(GetData, DataDone, Pieces,
								bTurnWhite, cbInputValid, !cbTurnOver, !cbLastTurn, 
								xposition, yposition, xposition2, yposition2,
								NumPieces, TheBoard);
				continue;
			}
#endif
		}
		else {
			xbOverrideInvalid = true;
			CreateSnapshot(	GetData, DataDone, Pieces,
							bTurnWhite, !cbInputValid, !cbTurnOver, !cbLastTurn, 
							xposition, yposition, xposition2, yposition2,
							NumPieces, TheBoard);
			xbOverrideInvalid = false;
			if (bTurnWhite && bHumanPlaying) {
				ToScreen << "Not a valid move - Re-enter piece to move" << endl;
				DisplayBoard(TheBoard);
			}
			continue;
		}
		if (!bTurnWhite || !bHumanPlaying) {
			if (bShowGames) {
				ToScreen	<< (bTurnWhite ? "White " : "Black ") << (char)('a' + xposition)
						<< yposition + 1 << " to " << (char)('a' + xposition2)
						<< yposition2 + 1 << endl;
			}
			if (!bHumanPlaying) {
				if (bShowGames) {
					DisplayBoard(TheBoard);
				}
			}
		}

		CreateSnapshot( GetData, DataDone, Pieces,
						bTurnWhite, cbInputValid, cbTurnOver, !cbLastTurn,
						xposition, yposition, xposition2, yposition2,
						NumPieces, TheBoard);

		//ToScreen << (bTurnWhite ? "White " : "Black ") << (char)('a' + xposition) << yposition + 1
		//	<< " to " << (char)('a' + xposition2) << yposition2 + 1
		//	<< endl;
		if ((turn - LastValidTurn) > 1) {
			//ToScreen << "Invalid turns since last " << turn - LastValidTurn << " while valid in a row " << ValidTurnsInARow <<  endl;
			ValidTurnsInARow = 0;
		}
		else {
			ValidTurnsInARow++;
		}
		LastValidTurn = turn;

		if (bTurnWhite) {
			bTurnWhite = false;
		}
		else {
			bTurnWhite = true;
		}

		if (bTurnWhite && bHumanPlaying) {
			if (bShowGames) {
				DisplayBoard(TheBoard);
			}
		}
	}


	return 0;
}
