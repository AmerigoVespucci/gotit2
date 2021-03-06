#define BOARD_SIZE 5
#define NUM_PIECES 3

//#define MUST_JUMP
//#define KEEP_JUMPING
#define GO_KING

enum BoardType {
	eBoardNone = 0,
	eBoardWhite,
	eBoardBlack,
	eBoardWhiteKing,
	eBoardBlackKing,
	eBoardInvalid
};


typedef bool(*pfGetDataBuffer)(uchar *& pData, int MaxSize);
typedef void(*pfBufferWritten)(uchar * pData);
typedef void (*pfGetInput)(bool& bMoveValid, int& xSrc, int& ySrc, int& xDest, int& yDest);
bool PlayOneGame(pfGetDataBuffer GetData, pfBufferWritten DataDone, pfGetInput GetInput);

const int cHeaderSize = 2;
const ushort cWhiteTurn = 0x8000;
const ushort cInputValid = 0x4000;
const ushort cTurnOver = 0x2000;
const ushort cFinalTurn = 0x1000;
const uchar cKingNotReg = 0x80;
const uchar cWhiteNotBlack = 0x40;
const int cXPosBitShift = 3;
const uchar cXPosMask = 0x38;
const int cYPosBitShift = 0;
const uchar cYPosMask = 0x7;
const int cInputShift = 3; // 3 bits per coord
const ushort cOneInputMask = (ushort)((1 << cInputShift) - 1);

// debug globals

extern bool xbOverrideInvalid;

