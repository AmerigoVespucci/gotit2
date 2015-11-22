// CheckersIntf.cpp : interfaces to a game of checkers							
//

#include "stdafx.h"
#include <limits.h>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "H5Cpp.h"

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif


#include "MascReader.h"
//#include "CheckersIntf.h"
#include "Regex2D.h"

// Generate ipc.pb.h and ipc.pb.cc using:
// cd ~/NetBeansProjects/TestCaffe
// protoc -I=. -cpp_out=. ./ipc.proto
#include "/home/abba/NetBeansProjects/TestCaffe/ipc.pb.h"

using boost::asio::ip::tcp;
tcp::socket* Caffe1stClickSocket = NULL;
tcp::socket* ClicksValidSocket = NULL;
tcp::socket* ClicksResultSocket = NULL;
tcp::socket* BoardValidSocket = NULL;


	
tcp::socket* ClientInit(const char * host, int port_num) {
	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);
	//string port_str = to_string(1543);
	stringstream ssport;
	ssport << port_num;
	tcp::resolver::query query(host, ssport.str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	//tcp::socket socket(io_service);
	//boost::shared_ptr<tcp::socket> socket(new tcp::socket(io_service));
	tcp::socket* socket = new tcp::socket(io_service);
	boost::system::error_code error = boost::asio::error::host_not_found;
	while (error && endpoint_iterator != end) {
		socket->close();
		socket->connect(*endpoint_iterator++, error);
	}
	if (error) {
		//throw boost::system::system_error(error);
		return NULL;
	}

	
	std::cerr << "Client connection established\n";

	return socket;
}

int CaffeSendMsg(tcp::socket& socket, CaffeIpc& Msg) {
	int SerializedSize = Msg.ByteSize();
	unsigned char * data_buf = new unsigned char[SerializedSize];
	Msg.SerializeToArray(data_buf, SerializedSize);
	boost::array<int, 1> size_buf;
	size_buf[0] = SerializedSize;
	boost::system::error_code error;
	int len = write(	socket, boost::asio::buffer(size_buf),
						boost::asio::transfer_exactly(sizeof(int)), error);
	len = write(socket, boost::asio::buffer(data_buf, SerializedSize), 
				boost::asio::transfer_exactly(size_buf[0]), error);
	delete[] data_buf;
	if (error != 0) cerr << "Error sending IPC message.\n";
//	if (error == boost::asio::error::eof)
//		return -1; // Connection closed cleanly by peer.
//	else if (error)
//		throw boost::system::system_error(error); // Some other error.
	return len;
}

int CaffeRcvMsg(tcp::socket& socket, CaffeIpc& Msg) {
	boost::system::error_code error;
	boost::array<int, 1> size_buf;
	int len = read(socket, boost::asio::buffer(size_buf), boost::asio::transfer_exactly(sizeof(int)), error);
	if (error == 0) {
		int alloc_size = size_buf[0];
		std::cout << "Receiving a msg of size " << alloc_size << std::endl;
		char * data_buf = new char[alloc_size];
		len = read(	socket, boost::asio::buffer(data_buf, alloc_size), 
					boost::asio::transfer_exactly(alloc_size ), error);
		std::cout << "Read " << len << " bytes\n";
		if (error == 0) {
			Msg.ParseFromArray(data_buf, alloc_size);
		}
		delete[] data_buf;
			
	}

	if (error != 0) {
		cerr << "Error receiving IPC message.\n";
		return -1;
	}
//	if (error == boost::asio::error::eof)
//		return -1; // Connection closed cleanly by peer.
//	else if (error)
//		throw boost::system::system_error(error); // Some other error.
	
	return len;
}


	
#define MAX_BOARD_SIZE 19
const int cGamerHeaderSize = 2;
const ushort cGamerWhiteTurn = 0x8000;
const ushort cGamerInputValid = 0x4000;
const ushort cGamerTurnOver = 0x2000;
const ushort cGamerFinalTurn = 0x1000;
static const int cDataBufSize = (MAX_BOARD_SIZE * MAX_BOARD_SIZE) + cGamerHeaderSize;
#define ToScreen cerr
int gNumPieceTypes;
bool gbGameInitFailed = false;
bool gbGameInitialized = false;
int gDataDim1stClick;
int gDataDimValidClicks;
int gDataDimClicksResult;
int gDataDimBoardValid;


enum GamePieceType {
	gptSimple,
	gptChess,
};

static GamePieceType ggpt = gptChess;

enum GameBoardType {
	gbtChess,
	gbtGo,
};

enum eGameMode {
	tgmRecordValid,
	tgmRecordScore,
	tgmGetScore,
	tgmRecord1stClickValid,
	tgmRecordClicksValid,
	tgmRecordClicksValidError,
	tgmRecordResult,
	tgmUseCaffe,
	tgmRecord1stClickMap,
	tgmRecordClicksMap,
	tgmRecordElsValid,
	tgmInvalid,
};

eGameMode gGameMode = tgmInvalid;


static GameBoardType ggbt = gbtChess;

int GamerBoardSize = 5;
bool bGameCalcMoves = false;
bool bGameEvaluating = false;
static int NumWhiteWins;
static int NumBlackWins;
bool bGameLastGiven0 = false;
int GamerWhiteDepth = 2;
int GamerBlackDepth = 2;
string GamerWhoStarts = "white";
char GamerStartBoard[MAX_BOARD_SIZE*MAX_BOARD_SIZE];
bool GamerbBestMove = true;
bool GamerbLearnValids = false;
vector<vector<uint> > GameRecords;
uint NumPieceTypes = 0;


uchar GamerDataBuf0[cDataBufSize];
uchar GamerDataBuf1[cDataBufSize];

static char cSimplePieces[] = { ' ', 'w', 'b', 'W', 'B' };
static char cChessPieces[] = { ' ', 'p', 'n', 'b', 'r', 'q', 'k', 'P', 'N', 'B', 'R', 'Q', 'K' };

bool GamerGetDataBuffer(uchar *& pData, int MaxSize)
{
	if (MaxSize > cDataBufSize) {
		pData = NULL;
		return false;
	}
	if (bGameLastGiven0) {
		pData = GamerDataBuf1;
		bGameLastGiven0 = false;
	}
	else {
		pData = GamerDataBuf0;
		bGameLastGiven0 = true;
	}
	return true;
}

void GamerStoreMoves(uchar * pOldBoard, uchar * pNewBoard, ushort Hdr)
{

}

bool GamerCreateInput(uchar * pOldBoard, uchar * pNewBoard, ushort Hdr)
{
	return true;
}

void GamerMoveComplete(uchar * pData) {
	ushort Hdr = *(ushort *)pData;
	uchar * pDataBeforeMove;
	//bool bUseOption = false;
	//int iOption = 0;
	//vector<SInputOption > OptionsList;
	if (pData == GamerDataBuf0) {
		pDataBeforeMove = GamerDataBuf1;
	}
	else {
		pDataBeforeMove = GamerDataBuf0;
	}
	GamerStoreMoves(pDataBeforeMove + cGamerHeaderSize, pData + cGamerHeaderSize, Hdr);

	if (bGameCalcMoves) {
		GamerCreateInput(pDataBeforeMove + cGamerHeaderSize,
			pData + cGamerHeaderSize, Hdr);
	}
	//if (!bValidInputCreated) {
	//	CIInputSrcAndDest = (rand() % BOARD_SIZE) | ((rand() % BOARD_SIZE) << cInputShift)
	//		| ((rand() % BOARD_SIZE) << (2 * cInputShift))
	//		| ((rand() % BOARD_SIZE) << (3 * cInputShift))
	//		| 0x0 << (4 * cInputShift);
	//}


}

void GamerGetInput(bool& bMoveValid, int& xSrc, int& ySrc, int& xDest, int& yDest)
{

}

void CGotitEnv::InitGame()
{
	if (gbGameInitialized) {
		return;
	}
	NumBlackWins = 0;
	NumWhiteWins = 0;

	string sWhichGame;
	if (GetImplemParam(sWhichGame, "Implem.Param.FnParam.InitGame.Which")) {
		if (sWhichGame == "Checkers") {
			ggbt = gbtChess;
			ggpt = gptSimple;
			CIRegexInit(GamerBoardSize);
		}
		else if (sWhichGame == "Chess") {
			ggbt = gbtChess;
			ggpt = gptChess;
			ChessRegexInit(GamerBoardSize);
		}
		else if (sWhichGame == "Go") {
			ggbt = gbtGo;
			ggpt = gptSimple;
			GoRegexInit(GamerBoardSize);
		}
	}

	string sWhiteDepth, sBlackDepth, sBoardSize;

	if (GetImplemParam(sBoardSize, "Implem.Param.FnParam.InitGame.BoardSize")) {
		GamerBoardSize = stoi(sBoardSize);
	}
	
	if (GetImplemParam(sWhiteDepth, "Implem.Param.FnParam.InitGame.WhiteDepth")) {
		GamerWhiteDepth = stoi(sWhiteDepth);
	}

	if (GetImplemParam(sBlackDepth, "Implem.Param.FnParam.InitGame.BlackDepth")) {
		GamerBlackDepth = stoi(sBlackDepth);
	}

	if (GetImplemParam(GamerWhoStarts, "Implem.Param.FnParam.InitGame.WhoStarts")) {
	}
	
	string sGamerStartBoard;
	if (GetImplemParam(sGamerStartBoard, "Implem.Param.FnParam.InitGame.StartBoard")) {
		sGamerStartBoard.erase(boost::remove_if(sGamerStartBoard, boost::is_any_of(" \r\n")), sGamerStartBoard.end());
	    vector<string> strs;
	    boost::split(strs, sGamerStartBoard, boost::is_any_of(","));
		for (int is = 0; is < strs.size() && is < GamerBoardSize*GamerBoardSize; is++) {
			GamerStartBoard[is] = (char)boost::lexical_cast<int>(strs[is]);
		}
		//for_each(strs.begin(), strs.end(), [](const string& s) -> char { s[0] -= '0'; return boost::lexical_cast<char>(s); });
		//memcpy(GamerStartBoard,sGamerStartBoard.c_str(),GamerBoardSize*GamerBoardSize);
	}
	else {
		char DefaultStartBoard[] = {
		1, 0, 1, 0, 1,  
		0, 0, 0, 0, 0,  
		0, 0, 0, 0, 0,  
		0, 0, 0, 0, 0,  
		2, 0, 2, 0, 2,  
		};
		memcpy(GamerStartBoard,DefaultStartBoard,GamerBoardSize*GamerBoardSize);
	}
	
	string sbBestMove;
	if (GetImplemParam(sbBestMove, "Implem.Param.FnParam.InitGame.DoBestMove")) {
		if (sbBestMove[0] == 'n' || sbBestMove[0] == 'N') {
			GamerbBestMove = false;
		}
	}

	string sbLearnValids;
	if (GetImplemParam(sbLearnValids, "Implem.Param.FnParam.InitGame.LearnValids")) {
		if (sbLearnValids[0] == 'y' || sbLearnValids[0] == 'Y') {
			GamerbLearnValids = true;
		}
	}

	string sGameMode;
	if (GetImplemParam(sGameMode, "Implem.Param.FnParam.InitGame.GameMode")) {
		if (sGameMode == "GetScore") {
			gGameMode = tgmGetScore;	
		}
		else if (sGameMode == "RecordValid") {
			gGameMode = tgmRecordValid;	
		}
		else if (sGameMode == "RecordResult") {
			gGameMode = tgmRecordResult;	
		}
		else if (sGameMode == "RecordScore") {
			gGameMode = tgmRecordScore;	
		}
		else if (sGameMode == "Record1stClickValid") {
			gGameMode = tgmRecord1stClickValid;	
		}
		else if (sGameMode == "RecordClicksValid") {
			gGameMode = tgmRecordClicksValid;	
		}
		else if (sGameMode == "RecordClicksValidError") {
			gGameMode = tgmRecordClicksValidError;	
		}		
		else if (sGameMode == "RecordElsValid") {
			gGameMode = tgmRecordElsValid;	
		}
		else if (sGameMode == "UseCaffe") {
			gGameMode = tgmUseCaffe;	
		}
		else if (sGameMode == "Record1stClickMap") {
			gGameMode = tgmRecord1stClickMap;	
		}
		else if (sGameMode == "RecordClicksMap") {
			gGameMode = tgmRecordClicksMap;	
		}
		
	}

	bool gbGameInitialized = false;

	
}

char DisplayBoardPos(char piece)
{
	if (piece < 0) {
		return 'x';
	}
	switch (ggpt){
	case gptSimple: 
		if (piece >= (char)sizeof(cSimplePieces)) {
			cerr << "error. Piece value beyond allowed for this game\n";
			return ' ';
		}
		return(cSimplePieces[(int)piece]);
	case gptChess: 
		if (piece >= (char)sizeof(cChessPieces)) {
			cerr << "error. Piece value beyond allowed for this game\n";
			return ' ';
		}
		return(cChessPieces[(int)piece]);
	default:   return('!');
	}
}

void DisplayGoBoard(char TheBoard[])
{
	//ToScreen << "  |----|----|----|----|----|----|----|----|" << endl;
	//ToScreen << "  |";

	for (int iRow = GamerBoardSize - 1; iRow >= 0; iRow--)
	{
		ToScreen << iRow + 1 << " ";
		for (int iCol = 0; iCol < GamerBoardSize; iCol++)
		{
			char Show = DisplayBoardPos(TheBoard[(iRow * GamerBoardSize) + iCol]);
			if (Show == ' ') {
				ToScreen << ":";
			}
			else {
				ToScreen << Show;
			}
			if (iCol < (GamerBoardSize - 1)) {
				ToScreen << " - ";
			}
			else {
				ToScreen << endl;
			}
		}
		if (iRow > 0) {
			for (int iCol = 0; iCol < GamerBoardSize; iCol++)	{
				ToScreen << "  : ";
			}
		}
		ToScreen << endl;


	}
	ToScreen << " ";
	for (int iCol = 0; iCol < GamerBoardSize; iCol++)
	{
		ToScreen << "_" << (char)('a' + iCol) ;
		if (iCol < (GamerBoardSize - 1)) {
			ToScreen << "__";
		}
		else {
			ToScreen << "_";
		}
	}
	ToScreen << endl;
}

void DisplayBoard(char TheBoard[])
{
	if (ggbt == gbtGo) {
		return DisplayGoBoard(TheBoard);
	}

	//ToScreen << "  |----|----|----|----|----|----|----|----|" << endl;
	ToScreen << "  |";
	for (int iCol = 0; iCol < GamerBoardSize; iCol++)
	{
		ToScreen << "----|";
	}
	ToScreen << endl;

	for (int iRow = GamerBoardSize - 1; iRow >= 0; iRow--)
	{
		ToScreen << iRow + 1 << " ";
		for (int iCol = 0; iCol < GamerBoardSize; iCol++)
		{
			ToScreen << "| " << DisplayBoardPos(TheBoard[(iRow * GamerBoardSize) + iCol]) << "  ";
		}
		ToScreen << "|" << endl << "  |";
		for (int iCol = 0; iCol < GamerBoardSize; iCol++)
		{
			ToScreen << "----|";
		}
		ToScreen << endl;


		//ToScreen << "|" << endl << "  |----|----|----|----|----|----|----|----|" << endl;
	}
	ToScreen << "  _";
	for (int iCol = 0; iCol < GamerBoardSize; iCol++)
	{
		ToScreen << "_" << (char)('a' + iCol) << "___";
	}
	ToScreen << endl;
}

void CreateSnapshot(bool bWhiteTurn, bool bValidInput, bool bTurnOver, bool bEndTurn,
					int xSrc, int ySrc, int xDest, int yDest,
					char TheBoard[])
{
	int SizeData = cGamerHeaderSize + (GamerBoardSize * GamerBoardSize);
	uchar * pData;
	bool bRet = GamerGetDataBuffer(pData, SizeData);
	if (!bRet || pData == NULL) {
		cerr << "Insufficient space to store snapshot\n";
		return;
	}
}

int AlphaBeta(vector<OneMove >& TopMoves, char * Board, bool bTurnWhite, int Depth, int Beta, int Alpha, int PrevScore, bool bFirst)
{
	bool bDebugLocal = true;
	const bool cbFirst = true; // note, not the argument
	char TestBoard[GamerBoardSize * GamerBoardSize];

	int BestScoreAtThisLevel = -INT_MAX;
	if (!bTurnWhite) {
		BestScoreAtThisLevel = INT_MAX;
	}
	//memcpy(TestBoard, Board, cBrdSize * cBrdSize);
	if (Depth == 0) {
		return PrevScore;
	}
	vector<OneMove > Moves;
	GamerRegexGetValidMoves(Moves, GamerBoardSize, Board, bTurnWhite);
	if (Moves.size() == 0) {
		if (bFirst) {
			TopMoves.clear();
		}
		return BestScoreAtThisLevel;
	}
	if (bFirst && (Moves.size() == 1)) {
		TopMoves = Moves;
		//int MoveScore = 0;
		//GamerRegexExecConseq(MoveScore, cBrdSize, TestBoard, bTurnWhite);
		return 0;
	}
	for (uint im = 0; im < Moves.size(); im++) {
		memcpy(TestBoard, Board, GamerBoardSize * GamerBoardSize);
		if (bDebugLocal) for (int it = 0; it < 4 - Depth; it++) cout << "->->";
		if (bDebugLocal) cout << (bTurnWhite ? "White: " : "Black: ");
		for (uint iFV = 0; iFV < Moves[im].Deltas.size(); iFV++) {
			BoardDelta& rec = Moves[im].Deltas[iFV];
			TestBoard[(rec.y * GamerBoardSize) + rec.x] = rec.NewVal;
			if (bDebugLocal) cout << "New val " << int(rec.NewVal) << " at " << char('a' + rec.x) << char('1' + rec.y) << ". ";
		}
		int MoveScore = 0;
		const bool cbConseqFinal = true;
		GamerRegexExecConseq(MoveScore, GamerBoardSize, TestBoard, bTurnWhite, !cbConseqFinal);
		if (bDebugLocal) cout << "Raw move score " << MoveScore << ". Alpha: " << Alpha << " Beta " << Beta << endl;
		if ((MoveScore <= -125) || (MoveScore >= 125)) {
			if (bFirst) {
				if ((bTurnWhite && (MoveScore >= 125)) 
					|| (!bTurnWhite && (MoveScore <= -125))) {
					TopMoves.clear();
					TopMoves.push_back(Moves[im]);
					if (bDebugLocal) cout << "Moves cleared and single winning move added!!\n";
				}
			}
			return MoveScore;
		}
		MoveScore = AlphaBeta(TopMoves, TestBoard, !bTurnWhite, Depth - 1, Beta, Alpha, MoveScore, !cbFirst);
		if (bTurnWhite) {
			if (bFirst && MoveScore == Alpha) {
				if (bDebugLocal) cout << "Move added for score: " << MoveScore << "\n";
				TopMoves.push_back(Moves[im]);
			}
			if (MoveScore > Alpha) {
				Alpha = MoveScore;
				if (bFirst) {
					TopMoves.clear();
					TopMoves.push_back(Moves[im]);
					if (bDebugLocal) cout << "Moves cleared and move added for score: " << MoveScore << "\n";
				}
			}
			if (MoveScore > BestScoreAtThisLevel) {
				BestScoreAtThisLevel = MoveScore;
			}
			if (Alpha > Beta) { // different from std algo
				if (bDebugLocal) cout << "Alpha: " << Alpha << ". Beta " << Beta << ". Abort A > B\n";
				return Alpha;
			}
		}
		else {
			if (bFirst && MoveScore == Beta) {
				TopMoves.push_back(Moves[im]);
				if (bDebugLocal) cout << "Move added for score: " << MoveScore << "\n";
			}
			if (MoveScore < Beta) {
				Beta = MoveScore;
				if (bFirst) {
					TopMoves.clear();
					TopMoves.push_back(Moves[im]);
					if (bDebugLocal) cout << "Moves cleared and move added for score: " << MoveScore << "\n";
				}
			}
			if (MoveScore < BestScoreAtThisLevel) {
				BestScoreAtThisLevel = MoveScore;
			}
			if (Beta < Alpha) {
				if (bDebugLocal) cout << "Alpha: " << Alpha << ". Beta " << Beta << ". Abort B < A\n";
				return Beta;
			}
		}
		
	}
//	if (bTurnWhite) {
//		return Alpha;
//	}
//	return Beta;
	return BestScoreAtThisLevel;
	
}

int ProgAlphaBeta(vector<OneMove >& TopMoves, char * Board, bool bTurnWhite, int Depth, int Beta, int Alpha, int PrevScore)
{
	char TestBoard[GamerBoardSize * GamerBoardSize];
	vector<OneMove> TMoves;
	int Ret = AlphaBeta(TMoves, Board, bTurnWhite, Depth, Beta, Alpha, PrevScore, true);
	if (TMoves.size() <= 1) {
		TopMoves = TMoves;
		return Ret;
	}
	vector<pair<int, int> > MovesList(TMoves.size());
	for (int id = Depth - 2; id > 0; id--) {

		vector<OneMove> T2Moves;
		for (uint im = 0; im < TMoves.size(); im++) {
			memcpy(TestBoard, Board, GamerBoardSize * GamerBoardSize);
			for (uint iFV = 0; iFV < TMoves[im].Deltas.size(); iFV++) {
				BoardDelta& rec = TMoves[im].Deltas[iFV];
				TestBoard[(rec.y * GamerBoardSize) + rec.x] = rec.NewVal;
			}
			int MoveScore = 0;
			const bool cbConseqFinal = true;
			GamerRegexExecConseq(MoveScore, GamerBoardSize, TestBoard, bTurnWhite, !cbConseqFinal);
			int Ret2 = AlphaBeta(T2Moves, TestBoard, !bTurnWhite, id, Beta, Alpha, PrevScore, true);
			MovesList[im].first = Ret2;
			MovesList[im].second = im;			
		}
		if (bTurnWhite) {
			sort(MovesList.rbegin(), MovesList.rend());
		}
		else {
			sort(MovesList.begin(), MovesList.end());
		}
		bool bStopAtFirst = false;
		for (uint im = 0; im < TMoves.size() - 1; im++) {
			if (MovesList[im].first != MovesList[im+1].first) {
				MovesList.resize(im+1);
				if (im == 0) {
					bStopAtFirst = true;
				}
				break;
			}
		}
		if (bStopAtFirst) {
			TopMoves.push_back(TMoves[MovesList[0].second]);
			return Ret;
		}
		vector<OneMove> T3Moves(MovesList.size());
		for (uint im = 0; im < MovesList.size(); im++) {
			T3Moves[im] = TMoves[MovesList[im].second];
		}
		TMoves = T3Moves;
		
	}
	
	TopMoves.resize(TMoves.size());
	TopMoves = TMoves;
	return Ret;
}

bool CaffeIs1stClickValid(	char* OldBoard, bool bWhiteTurn, 
						int xBoardDim, int yBoardDim, 
						int xClick, int yClick) 
{
	bool RetClickValid  = false;


	CaffeIpc MsgReq;
	MsgReq.set_type(CaffeIpc_MsgType_NET_PREDICT);
	CaffeIpc_DataParam& ReqData = *(MsgReq.mutable_data_param());

	for (uint ipt = 0; ipt < gDataDim1stClick-2; ipt++) {
		for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
			ReqData.add_data_val((OldBoard[pos] == ipt) ? 1.0f : 0.0f);
		}
	}

	for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		ReqData.add_data_val((bWhiteTurn) ? 1.0f : 0.0f);
	}

	for (int y=0; y<yBoardDim; y++) {
		for (int x=0; x < xBoardDim; x++) {
			if (x == xClick && y == yClick) {
				ReqData.add_data_val(1.0f);
			}
			else {
				ReqData.add_data_val(0.0f);
			}
		}
	}

	ReqData.set_num_params(xBoardDim * yBoardDim * gDataDim1stClick);

	CaffeSendMsg(*Caffe1stClickSocket, MsgReq);

	CaffeIpc MsgScore;
	CaffeRcvMsg(*Caffe1stClickSocket, MsgScore);
	if (MsgScore.type() != CaffeIpc_MsgType_PREDICT_RESULT) {
		std::cerr << "Problem with server. Did not send data response. Client bids bye!\n";
		gbGameInitFailed = true;
		return false;
	}
	else {
		const CaffeIpc_DataParam& RetData = MsgScore.data_param();
		std::cout << RetData.num_params() << " data items reported. Received: " << RetData.data_val_size() << ".\n";
		vector<pair<float, int> > ScoreMax;
		for (int id = 0; id < RetData.num_params(); id++) {
			//std::cerr << RetData.data_val(id) << ", ";
			ScoreMax.push_back(make_pair(RetData.data_val(id), id));
		}
		//std::cerr << endl;
		std::sort(ScoreMax.begin(), ScoreMax.end());
		RetClickValid = (ScoreMax.back().second == 1);
		//std::cerr << "Caffe predicted score spot of " << RetScoreSpot << " vs score of " << RealScoreSpot << endl;
		

	}
	
	return RetClickValid;
}


bool CaffeAreClicksValid(	char* OldBoard, bool bWhiteTurn, 
						int xBoardDim, int yBoardDim, 
						int xClick1, int yClick1,
						int xClick2, int yClick2) 
{
	bool RetClickValid  = false;


	CaffeIpc MsgReq;
	MsgReq.set_type(CaffeIpc_MsgType_NET_PREDICT);
	CaffeIpc_DataParam& ReqData = *(MsgReq.mutable_data_param());

	for (uint ipt = 0; ipt < gDataDimValidClicks-3; ipt++) {
		for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
			ReqData.add_data_val((OldBoard[pos] == ipt) ? 1.0f : 0.0f);
		}
	}

	for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		ReqData.add_data_val((bWhiteTurn) ? 1.0f : 0.0f);
	}

	for (int y=0; y<yBoardDim; y++) {
		for (int x=0; x < xBoardDim; x++) {
			if (x == xClick1 && y == yClick1) {
				ReqData.add_data_val(1.0f);
			}
			else {
				ReqData.add_data_val(0.0f);
			}
		}
	}

	for (int y=0; y<yBoardDim; y++) {
		for (int x=0; x < xBoardDim; x++) {
			if (x == xClick2 && y == yClick2) {
				ReqData.add_data_val(1.0f);
			}
			else {
				ReqData.add_data_val(0.0f);
			}
		}
	}

	ReqData.set_num_params(xBoardDim * yBoardDim * gDataDimValidClicks);

	CaffeSendMsg(*ClicksValidSocket, MsgReq);

	CaffeIpc MsgScore;
	CaffeRcvMsg(*ClicksValidSocket, MsgScore);
	if (MsgScore.type() != CaffeIpc_MsgType_PREDICT_RESULT) {
		std::cerr << "Problem with server. Did not send data response. Client bids bye!\n";
		gbGameInitFailed = true;
		return false;
	}
	else {
		const CaffeIpc_DataParam& RetData = MsgScore.data_param();
		std::cout << RetData.num_params() << " data items reported. Received: " << RetData.data_val_size() << ".\n";
		vector<pair<float, int> > ScoreMax;
		for (int id = 0; id < RetData.num_params(); id++) {
			//std::cerr << RetData.data_val(id) << ", ";
			ScoreMax.push_back(make_pair(RetData.data_val(id), id));
		}
		//std::cerr << endl;
		std::sort(ScoreMax.begin(), ScoreMax.end());
		RetClickValid = (ScoreMax.back().second == 1);
		//std::cerr << "Caffe predicted score spot of " << RetScoreSpot << " vs score of " << RealScoreSpot << endl;
		

	}
	
	return RetClickValid;
}

bool CaffeClicksResult(	char* OldBoard, char * NewBoard, bool bWhiteTurn, 
						int xBoardDim, int yBoardDim, 
						int xClick1, int yClick1,
						int xClick2, int yClick2,
						vector<tuple<int, int, vector<int> > >& AltStates,
						float ConfidenceThresh) 
{
	CaffeIpc MsgReq;
	MsgReq.set_type(CaffeIpc_MsgType_NET_PREDICT);
	CaffeIpc_DataParam& ReqData = *(MsgReq.mutable_data_param());

	for (uint ipt = 0; ipt < gDataDimValidClicks-3; ipt++) {
		for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
			ReqData.add_data_val((OldBoard[pos] == ipt) ? 1.0f : 0.0f);
		}
	}

	for (int y=0; y<yBoardDim; y++) {
		for (int x=0; x < xBoardDim; x++) {
			if (x == xClick1 && y == yClick1) {
				ReqData.add_data_val(1.0f);
			}
			else {
				ReqData.add_data_val(0.0f);
			}
		}
	}

	for (int y=0; y<yBoardDim; y++) {
		for (int x=0; x < xBoardDim; x++) {
			if (x == xClick2 && y == yClick2) {
				ReqData.add_data_val(1.0f);
			}
			else {
				ReqData.add_data_val(0.0f);
			}
		}
	}

	for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		ReqData.add_data_val((bWhiteTurn) ? 1.0f : 0.0f);
	}

	ReqData.set_num_params(xBoardDim * yBoardDim * gDataDimValidClicks);

	CaffeSendMsg(*ClicksResultSocket, MsgReq);

	CaffeIpc MsgScore;
	CaffeRcvMsg(*ClicksResultSocket, MsgScore);
	if (MsgScore.type() != CaffeIpc_MsgType_PREDICT_RESULT) {
		std::cerr << "Problem with server. Did not send data response. Client bids bye!\n";
		gbGameInitFailed = true;
		return false;
	}
	else {
		const CaffeIpc_DataParam& RetData = MsgScore.data_param();
		std::cout << RetData.num_params() << " data items reported. Received: " << RetData.data_val_size() << ".\n";
		int NumPieceTypes = gDataDimValidClicks - 3; // as above
		if (RetData.num_params() != NumPieceTypes * yBoardDim *xBoardDim ) {
			std::cerr << "Problem with server. Did not send back the right number of data items\n";
			return false;
		}
		vector<vector<vector<float> > > ConvData(
			NumPieceTypes, vector<vector<float> > (
				yBoardDim, vector<float>(xBoardDim, 0.0f)));
		uint id = 0;
		for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
			for (int iRow = 0; iRow < yBoardDim; iRow++) {
				for (int iCol = 0; iCol < xBoardDim; iCol++, id++) {
					ConvData[ipt][iRow][iCol] = RetData.data_val(id);
				}
			}
		}
		for (int iRow = 0; iRow < yBoardDim; iRow++) {
			for (int iCol = 0; iCol < xBoardDim; iCol++) {
				vector<pair<float, int> > ScoreMax;
				for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
					ScoreMax.push_back(make_pair(
						ConvData[ipt][iRow][iCol], ipt));
				}
				std::sort(ScoreMax.begin(), ScoreMax.end());
				vector<int> AboveThresh;
				for (int ic = 0; ic < ScoreMax.size(); ic++) {
					if (ScoreMax[ic].first > ConfidenceThresh) {
						AboveThresh.push_back(ScoreMax[ic].second);
					}
				}
				if (AboveThresh.size() > 1) {
					AltStates.push_back(make_tuple(iCol, iRow, AboveThresh));
				}
				NewBoard[(iRow * xBoardDim) + iCol] = ScoreMax.back().second;
			}
		}

		

	}
	
	return true;
}

bool CaffeBoardValid(	char* OldBoard, char * NewBoard, bool bWhiteTurn, 
						int xBoardDim, int yBoardDim,
						vector<tuple<int, int, vector<int> > >& AltStates,
						float ConfidenceThresh, bool& bUncertain) 
{
	bUncertain = false;
	bool bRetClickValid;
	CaffeIpc MsgReq;
	MsgReq.set_type(CaffeIpc_MsgType_NET_PREDICT);
	CaffeIpc_DataParam& ReqData = *(MsgReq.mutable_data_param());

	int NumPieceTypes = ((gDataDimBoardValid - 1) / 2) ;
	for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
		for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
			ReqData.add_data_val((OldBoard[pos] == ipt) ? 1.0f : 0.0f);
		}
	}

	for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
		for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
			ReqData.add_data_val((NewBoard[pos] == ipt) ? 1.0f : 0.0f);
		}
	}

	for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		ReqData.add_data_val((bWhiteTurn) ? 1.0f : 0.0f);
	}

	ReqData.set_num_params(xBoardDim * yBoardDim * gDataDimBoardValid);

	CaffeSendMsg(*BoardValidSocket, MsgReq);

	CaffeIpc MsgScore;
	CaffeRcvMsg(*BoardValidSocket, MsgScore);
	if (MsgScore.type() != CaffeIpc_MsgType_PREDICT_RESULT) {
		std::cerr << "Problem with server. Did not send data response. Client bids bye!\n";
		gbGameInitFailed = true;
		return false;
	}
	else {
		const CaffeIpc_DataParam& RetData = MsgScore.data_param();
		std::cout << RetData.num_params() << " data items reported. Received: " << RetData.data_val_size() << ".\n";
		int NumPieceTypes = gDataDimValidClicks - 3; // as above
		if (RetData.num_params() != 2 ) {
			std::cerr << "Problem with server. Did not send back the right number of data items\n";
			return false;
		}
		vector<pair<float, int> > ScoreMax;
		for (int id = 0; id < RetData.num_params(); id++) {
			//std::cerr << RetData.data_val(id) << ", ";
			ScoreMax.push_back(make_pair(RetData.data_val(id), id));
		}
		//std::cerr << endl;
		std::sort(ScoreMax.begin(), ScoreMax.end());
		if (ScoreMax.back().first < ConfidenceThresh) {
			bUncertain = true;
		}
		bRetClickValid = (ScoreMax.back().second == 1);
	}
	
	return bRetClickValid;
}

int ScoreToClass(int MoveScore) 
{
	int cScoreRange = 33; // make it odd
	int MidSpot = (cScoreRange / 2) ;
	if (MoveScore == 0) {
		return MidSpot;
	}
	else if (MoveScore <= -100) {
		return 0;
	}
	else if (MoveScore >= 100) {
		return cScoreRange - 1;
	}
	else {
		bool PosScore = (MoveScore > 0);
		double AbsScore = (double)abs(MoveScore);
		int ScoreDiff = (int)floor((log10(AbsScore) * 7.0f)) + 1;
		return ((PosScore) ? MidSpot + ScoreDiff : MidSpot - ScoreDiff);

	}
	
	return MidSpot;
	
}


// returns number of items in a record
int HD5OutputElsValid(	float *& ppd, float *& ppl, vector<uint>& Record, 
						int xBoardDim, int yBoardDim, uint NumPieceTypes)
{
	/*
	 * bits in req
	 * 13 which piece
	 * 13 which piece ends up
	 * 8 forward, back, left, right, diag fr br bl fl, 
	 * 15 delta x (-7..0..7)
	 * 15 delta y
	 * 1 white move now
	 * 1 blocked en route
	 * 1 replaces
	 * xx - not doing --- 1 same side at end up
	 */
	int NumItemsInRec =		(NumPieceTypes * 2) + 8 
						+	((xBoardDim * 2) - 1) +	((yBoardDim * 2) - 1) 
						+	1 + 1 + 1;
	if (ppd == NULL) {
		// count only
		return (NumItemsInRec);
	}
	float bWhiteMove = (float)Record[0];
	int RecPosSoFar = 1;
	vector<vector<int> > Board(yBoardDim, vector<int>(xBoardDim, 0));
	for (int y = 0; y < yBoardDim; y++) {
		for (int x = 0; x < xBoardDim; x++) {
			Board[y][x] = Record[(y * xBoardDim) + x + RecPosSoFar];
		}
	}
	RecPosSoFar += xBoardDim * yBoardDim;
	int xClick1 = Record[RecPosSoFar++];
	int yClick1 = Record[RecPosSoFar++];
	int xClick2 = Record[RecPosSoFar++];
	int yClick2 = Record[RecPosSoFar++];
	bool bValidMove = Record[RecPosSoFar++];
	int StartPiece = Board[yClick1][xClick1];
	int EndPiece =  Board[yClick2][xClick2];
	int dx = xClick2 - xClick1;
	int dy = yClick2 - yClick1;
	bool bForward = ((dx == 0) && (dy > 0));
	bool bBackward = ((dx == 0) && (dy < 0));
	bool bRight = ((dx > 0) && (dy == 0));
	bool bLeft = ((dx < 0) && (dy == 0));
	bool bDiagFR = ((dx == dy) && (dx > 0));
	bool bDiagBR = ((dx == -dy) && (dx > 0));
	bool bDiagBL = ((dx == dy) && (dx < 0));
	bool bDiagFL = ((dx == -dy) && (dx < 0));
	bool bBlocked = false;
	if (	bForward || bBackward || bLeft || bRight 
		||	bDiagFR || bDiagBR || bDiagBL || bDiagFL) {
		int dmax = max(abs(dx), abs(dy));
		int xiter = dx / dmax;
		int yiter = dy / dmax;
		int ix = xClick1, iy = yClick1;
		for (int ii = 1; ii < dmax; ii++) {
			ix += xiter; iy += yiter;
			if (Board[iy][ix] != 0) {
				bBlocked = true;
				break;
			}
		}
	}
	bool bReplace = false;
	if (EndPiece != 0) {
		bReplace = true;
	}
	
	for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
		*(ppd++) = ((StartPiece == ipt) ? 1.0f : 0.0f);
	}
	for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
		*(ppd++) = ((EndPiece == ipt) ? 1.0f : 0.0f);
	}
	*(ppd++) = (bForward ? 1.0f : 0.0f);
	*(ppd++) = (bBackward ? 1.0f : 0.0f);
	*(ppd++) = (bRight ? 1.0f : 0.0f);
	*(ppd++) = (bLeft ? 1.0f : 0.0f);
	*(ppd++) = (bDiagFR ? 1.0f : 0.0f);
	*(ppd++) = (bDiagBR ? 1.0f : 0.0f);
	*(ppd++) = (bDiagBL ? 1.0f : 0.0f);
	*(ppd++) = (bDiagFL ? 1.0f : 0.0f);
	for (int dpos = -(xBoardDim - 1); dpos <= (xBoardDim - 1); dpos++) {
		*(ppd++) = ((dpos == dx) ? 1.0f : 0.0f);
	}
	for (int dpos = -(yBoardDim - 1); dpos <= (yBoardDim - 1); dpos++) {
		*(ppd++) = ((dpos == dy) ? 1.0f : 0.0f);
	}
	*(ppd++) = (bWhiteMove ? 1.0f : 0.0f);
	*(ppd++) = (bBlocked ? 1.0f : 0.0f);
	*(ppd++) = (bReplace ? 1.0f : 0.0f);
	
	*(ppl++) = bValidMove;
	return (NumItemsInRec);
}

void WriteHD5Output(const char * Name, vector<vector<uint> >& Records, 
					int xBoardDim, int yBoardDim, uint NumPieceTypes)
{
	H5std_string fname = Name; 

	uint NumRecords = Records.size();
	const H5std_string	DATASET_NAME("data");
	const H5std_string	LABELSET_NAME("label");
	// the num items count is: 2 boards, and one value for white
	int NumItemsPerRec;
	int NumLabelsPerRec;
	bool bConvInput = true;
	float * pnull = NULL;
	switch (gGameMode) {
		case tgmRecordValid:
			NumItemsPerRec	= ((NumPieceTypes * 2) + 1)  * xBoardDim * yBoardDim;
			NumLabelsPerRec = 1;
			break;
		case tgmRecordResult:
			NumItemsPerRec	= (NumPieceTypes + 3)  * xBoardDim * yBoardDim;
			NumLabelsPerRec = NumPieceTypes * xBoardDim * yBoardDim;
			break;
		case tgmRecordScore:
			NumItemsPerRec	= (NumPieceTypes + 1) * xBoardDim * yBoardDim;
			NumLabelsPerRec = 1;
			break;
		case tgmRecord1stClickValid:
			NumItemsPerRec	= (NumPieceTypes + 2) * xBoardDim * yBoardDim;
			NumLabelsPerRec = 1;
			break;
		case tgmRecordClicksValid:
		case tgmRecordClicksValidError:
			NumItemsPerRec	= (NumPieceTypes + 3) * xBoardDim * yBoardDim;
			NumLabelsPerRec = 1;
			break;
		case tgmRecordElsValid:
			NumItemsPerRec	= HD5OutputElsValid(	pnull, pnull, Records[0], 
													xBoardDim, yBoardDim, 
													NumPieceTypes);
			bConvInput = false;
			NumLabelsPerRec = 1;
			break;
		case tgmRecord1stClickMap:
			NumItemsPerRec	=	(NumPieceTypes + 1) // pieces plus bWhiteTurn
							*	xBoardDim * yBoardDim;
			NumLabelsPerRec = xBoardDim * yBoardDim; // map of valid places to click
			break;
		case tgmRecordClicksMap:
			NumItemsPerRec	=	(NumPieceTypes + 2) // pieces, bWhiteTurn and where 1st click was
							*	xBoardDim * yBoardDim;
			NumLabelsPerRec = xBoardDim * yBoardDim; // map of valid places for 2nd click
			break;
		default:
			return;
	}
	const int cDataRank =  4;
	hsize_t dims[cDataRank];               // dataset dimensions
	int NumDims = cDataRank;
	dims[3] = xBoardDim;
	dims[2] = yBoardDim;
	dims[1] = NumItemsPerRec / (xBoardDim * yBoardDim);
	dims[0] = NumRecords;
	if (!bConvInput) {
		NumDims =  2;
		dims[1] = NumItemsPerRec;
	}
	const int cLabelRank = 2;
	hsize_t label_dims[cLabelRank];               // dataset dimensions
	label_dims[1] = NumLabelsPerRec;
	label_dims[0] = NumRecords;
	
	float * pDataSet = new float[NumRecords * NumItemsPerRec];
	float * ppd = pDataSet;
	float * plabels = new float[NumRecords * NumLabelsPerRec];
	float * ppl = plabels;

	for (uint ir = 0; ir < NumRecords; ir++) {
		// pos starts at 1 because 0 indexes whose move
		if (gGameMode == tgmRecordValid) {
			float bWhiteMove = (float)Records[ir][0];
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][(pos * 2) + 1] == ipt)? 1.0f : 0.0f);
				}
			}
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][(pos * 2) + 2] == ipt)? 1.0f : 0.0f);
				}
			}
			for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
				*(ppd++) = bWhiteMove; // a lot of redundancy here
			}
			*(ppl++) = (float)Records[ir].back();
		}
		if (gGameMode == tgmRecordResult) {
			float bWhiteMove = (float)Records[ir][0];
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][pos + 1] == ipt)? 1.0f : 0.0f);
				}
			}
			int ClickPosInRec = xBoardDim * yBoardDim + 1;
			for (int iBoth = 0; iBoth < 2; iBoth++) {
				int xClick = Records[ir][ClickPosInRec++];
				int yClick = Records[ir][ClickPosInRec++];
				for (int y=0; y<yBoardDim; y++) {
					for (int x=0; x < xBoardDim; x++) {
						if (x == xClick && y == yClick) {
							*(ppd++) = 1.0f;
						}
						else {
							*(ppd++) = 0.0f;
						}
					}
				}
			}
			for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
				*(ppd++) = bWhiteMove; // a lot of redundancy here
			}
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppl++) = ((Records[ir][pos + ClickPosInRec] == ipt)? 1.0f : 0.0f);
				}
			}
			//*(ppl++) = (float)Records[ir].back();
		}
		else if (gGameMode == tgmRecordScore) {
			float bWhiteMove = (float)Records[ir][0];
			int MoveScore = (int)Records[ir][1];
			int ScoreSpot = ScoreToClass(MoveScore);
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][pos + 2] == ipt)? 1.0f : 0.0f);
				}
			}
			for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
				*(ppd++) = bWhiteMove; // a lot of redundancy here
			}
			*(ppl++) = (float)ScoreSpot;
		}
		else if (gGameMode == tgmRecord1stClickValid) {
			float bWhiteMove = (float)Records[ir][0];
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][pos + 1] == ipt)? 1.0f : 0.0f);
				}
			}
			for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
				*(ppd++) = bWhiteMove; // a lot of redundancy here
			}
			int xClick = Records[ir][xBoardDim * yBoardDim + 1];
			int yClick = Records[ir][xBoardDim * yBoardDim + 2];
			for (int y=0; y<yBoardDim; y++) {
				for (int x=0; x < xBoardDim; x++) {
					if (x == xClick && y == yClick) {
						*(ppd++) = 1.0f;
					}
					else {
						*(ppd++) = 0.0f;
					}
				}
			}
			*(ppl++) = (float)Records[ir].back();
		}
		else if ((gGameMode == tgmRecordClicksValid) || (gGameMode == tgmRecordClicksValidError)) {
			float bWhiteMove = (float)Records[ir][0];
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][pos + 1] == ipt)? 1.0f : 0.0f);
				}
			}
			for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
				*(ppd++) = bWhiteMove; // a lot of redundancy here
			}
			int ClickPosInRec = xBoardDim * yBoardDim + 1;
			for (int iBoth = 0; iBoth < 2; iBoth++) {
				int xClick = Records[ir][ClickPosInRec++];
				int yClick = Records[ir][ClickPosInRec++];
				for (int y=0; y<yBoardDim; y++) {
					for (int x=0; x < xBoardDim; x++) {
						if (x == xClick && y == yClick) {
							*(ppd++) = 1.0f;
						}
						else {
							*(ppd++) = 0.0f;
						}
					}
				}
			}
			*(ppl++) = (float)Records[ir].back();
		}
		else if (gGameMode == tgmRecordElsValid) {
			HD5OutputElsValid(	ppd, ppl, Records[ir], 
								xBoardDim, yBoardDim, NumPieceTypes);
		}
		else if (gGameMode == tgmRecord1stClickMap) {
			float bWhiteMove = (float)Records[ir][0];
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][pos + 1] == ipt)? 1.0f : 0.0f);
				}
			}
			for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
				*(ppd++) = bWhiteMove; // a lot of redundancy here
			}
			int RecPos = (xBoardDim * yBoardDim) + 1;
			for (int y=0; y<yBoardDim; y++) {
				for (int x=0; x < xBoardDim; x++, RecPos++) {
					*(ppl++) = (float)Records[ir][RecPos];
				}
			}
		}
		else if (gGameMode == tgmRecordClicksMap) {
			float bWhiteMove = (float)Records[ir][0];
			for (uint ipt = 0; ipt < NumPieceTypes; ipt++) {
				for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
					*(ppd++) = ((Records[ir][pos + 1] == ipt)? 1.0f : 0.0f);
				}
			}
			for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
				*(ppd++) = bWhiteMove; // a lot of redundancy here
			}
			int xClick = Records[ir][xBoardDim * yBoardDim + 1];
			int yClick = Records[ir][xBoardDim * yBoardDim + 2];
			for (int y=0; y<yBoardDim; y++) {
				for (int x=0; x < xBoardDim; x++) {
					if (x == xClick && y == yClick) {
						*(ppd++) = 1.0f;
					}
					else {
						*(ppd++) = 0.0f;
					}
				}
			}
			int RecPos = (xBoardDim * yBoardDim) + 3;
			for (int y=0; y<yBoardDim; y++) {
				for (int x=0; x < xBoardDim; x++, RecPos++) {
					*(ppl++) = (float)Records[ir][RecPos];
				}
			}
		}

		
			
		//*(ppd++) = (float)Records[ir][pos++]; // the old way was to add one bit at the end of the record
	}
	H5File h5file(fname, H5F_ACC_TRUNC);
	DataSpace dataspace(NumDims, dims);
	DataSpace labelspace(cLabelRank, label_dims);
	DataSet dataset = h5file.createDataSet(DATASET_NAME, PredType::IEEE_F32LE, dataspace);

	dataset.write(pDataSet, PredType::IEEE_F32LE);

	DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

	labelset.write(plabels, PredType::IEEE_F32LE);
	
	delete pDataSet;
	delete plabels;

}


void RecordValid(	char* OldBoard, char * NewBoard, bool bWhiteTurn, bool bValid, 
					int MoveScore, vector<vector<uint> >& Records, 
					int xBoardDim, int yBoardDim, uint& NumPieceTypes,
					int iCol1stClick, int iRow1stClick, 
					int iCol2ndClick, int iRow2ndClick) 
{
	Records.push_back(vector<uint>());
	vector<uint>& rec = Records.back();
	rec.push_back((uint)bWhiteTurn);
	//rec.push_back((uint)MoveScore);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint OPiece = (uint)OldBoard[pos];
		uint NPiece = (uint)NewBoard[pos];
		if ((OPiece+1) > NumPieceTypes) {
			NumPieceTypes = OPiece+1;
		}
		if ((NPiece+1) > NumPieceTypes) {
			NumPieceTypes = NPiece+1;
		}
		rec.push_back(OPiece);
		rec.push_back(NPiece);
	}
	rec.push_back((uint)bValid);
}

void RecordResult(	char* OldBoard, char * NewBoard, bool bWhiteTurn, bool bValid, 
					int MoveScore, vector<vector<uint> >& Records, 
					int xBoardDim, int yBoardDim, uint& NumPieceTypes,
					int iCol1stClick, int iRow1stClick, 
					int iCol2ndClick, int iRow2ndClick) 
{
	Records.push_back(vector<uint>());
	vector<uint>& rec = Records.back();
	rec.push_back((uint)bWhiteTurn);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint OPiece = (uint)OldBoard[pos];
		if ((OPiece+1) > NumPieceTypes) {
			NumPieceTypes = OPiece+1;
		}
		rec.push_back(OPiece);
	}
	rec.push_back(iCol1stClick);
	rec.push_back(iRow1stClick);
	rec.push_back(iCol2ndClick);
	rec.push_back(iRow2ndClick);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint NPiece = (uint)NewBoard[pos];
		if ((NPiece+1) > NumPieceTypes) {
			NumPieceTypes = NPiece+1;
		}
		rec.push_back(NPiece);		
	}
}

void RecordScore(	char* OldBoard, char * NewBoard, bool bWhiteTurn, bool bValid, 
					int MoveScore, vector<vector<uint> >& Records, 
					int xBoardDim, int yBoardDim, uint& NumPieceTypes,
					int iCol1stClick, int iRow1stClick, 
					int iCol2ndClick, int iRow2ndClick) 
{
	Records.push_back(vector<uint>());
	vector<uint>& rec = Records.back();
	rec.push_back((uint)bWhiteTurn);
	rec.push_back((uint)MoveScore);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint OPiece = (uint)OldBoard[pos];
		uint NPiece = (uint)NewBoard[pos];
		if ((OPiece+1) > NumPieceTypes) {
			NumPieceTypes = OPiece+1;
		}
		if ((NPiece+1) > NumPieceTypes) {
			NumPieceTypes = NPiece+1;
		}
		rec.push_back(NPiece);
	}
}

void Record1stClickValid(	char* OldBoard, char * NewBoard, 
							bool bWhiteTurn, bool bValid, 
							int MoveScore, vector<vector<uint> >& Records, 
							int xBoardDim, int yBoardDim, uint& NumPieceTypes,
							int iCol1stClick, int iRow1stClick, 
							int iCol2ndClick, int iRow2ndClick) 
{
	Records.push_back(vector<uint>());
	vector<uint>& rec = Records.back();
	rec.push_back((uint)bWhiteTurn);
	//rec.push_back((uint)MoveScore);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint OPiece = (uint)OldBoard[pos];
		uint NPiece = (uint)NewBoard[pos];
		if ((OPiece+1) > NumPieceTypes) {
			NumPieceTypes = OPiece+1;
		}
//		if ((NPiece+1) > NumPieceTypes) {
//			NumPieceTypes = NPiece+1;
//		}
		rec.push_back(OPiece);
		//rec.push_back(NPiece);
	}
	rec.push_back(iCol1stClick);
	rec.push_back(iRow1stClick);
	rec.push_back((uint)bValid);
}

void RecordClicksValid(	char* OldBoard, char * NewBoard, 
							bool bWhiteTurn, bool bValid, 
							int MoveScore, vector<vector<uint> >& Records, 
							int xBoardDim, int yBoardDim, uint& NumPieceTypes,
							int iCol1stClick, int iRow1stClick, 
							int iCol2ndClick, int iRow2ndClick) 
{
	Records.push_back(vector<uint>());
	vector<uint>& rec = Records.back();
	rec.push_back((uint)bWhiteTurn);
	//rec.push_back((uint)MoveScore);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint OPiece = (uint)OldBoard[pos];
		uint NPiece = (uint)NewBoard[pos];
		if ((OPiece+1) > NumPieceTypes) {
			NumPieceTypes = OPiece+1;
		}
//		if ((NPiece+1) > NumPieceTypes) {
//			NumPieceTypes = NPiece+1;
//		}
		rec.push_back(OPiece);
		//rec.push_back(NPiece);
	}
	rec.push_back(iCol1stClick);
	rec.push_back(iRow1stClick);
	rec.push_back(iCol2ndClick);
	rec.push_back(iRow2ndClick);
	rec.push_back((uint)bValid);
}

void Record1stClickMap(	char* OldBoard,
						bool bWhiteTurn, vector<vector<uint> >& Records, 
						int xBoardDim, int yBoardDim, uint& NumPieceTypes,
						vector<vector<bool> >& ValidMap) 
{
	Records.push_back(vector<uint>());
	vector<uint>& rec = Records.back();
	rec.push_back((uint)bWhiteTurn);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint OPiece = (uint)OldBoard[pos];
		if ((OPiece+1) > NumPieceTypes) {
			NumPieceTypes = OPiece+1;
		}
		rec.push_back(OPiece);
	}
//	rec.push_back(iCol1stClick);
//	rec.push_back(iRow1stClick);
//	rec.push_back((uint)bValid);
	for (int ypos = 0; ypos < yBoardDim; ypos++) {
		for (uint xpos = 0; xpos < xBoardDim; xpos++) {
			rec.push_back(ValidMap[ypos][xpos] ? 1 : 0);
		}
	}
}

void RecordClicksMap(	char* OldBoard,
						bool bWhiteTurn, vector<vector<uint> >& Records, 
						int xBoardDim, int yBoardDim, uint& NumPieceTypes,
						int iCol1stClick, int iRow1stClick, 
						vector<vector<bool> >& ValidMap) 
{
	Records.push_back(vector<uint>());
	vector<uint>& rec = Records.back();
	rec.push_back((uint)bWhiteTurn);
	for (uint pos = 0; pos < xBoardDim * yBoardDim; pos++) {
		uint OPiece = (uint)OldBoard[pos];
		if ((OPiece+1) > NumPieceTypes) {
			NumPieceTypes = OPiece+1;
		}
		rec.push_back(OPiece);
	}
	rec.push_back(iCol1stClick);
	rec.push_back(iRow1stClick);
	for (int ypos = 0; ypos < yBoardDim; ypos++) {
		for (uint xpos = 0; xpos < xBoardDim; xpos++) {
			rec.push_back(ValidMap[ypos][xpos] ? 1 : 0);
		}
	}
}


void GetScoreFromCaffe(	char* OldBoard, char * NewBoard, bool bWhiteTurn, bool bValid, 
						int MoveScore, vector<vector<uint> >& Records, 
						int xBoardDim, int yBoardDim, uint& NumPieceTypes,
						int iCol1stClick, int iRow1stClick, 
						int iCol2ndClick, int iRow2ndClick) 
{



	CaffeIpc MsgReq;
	MsgReq.set_type(CaffeIpc_MsgType_NET_PREDICT);
	CaffeIpc_DataParam& ReqData = *(MsgReq.mutable_data_param());

	for (uint ipt = 0; ipt < gNumPieceTypes-1; ipt++) {
		for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
			ReqData.add_data_val((NewBoard[pos] == ipt) ? 1.0f : 0.0f);
			//*(ppd++) = ((Records[ir][pos+1] == ipt)? 1.0f : 0.0f);
		}
	}

	for (int pos = 0; pos < xBoardDim * yBoardDim; pos++) {
//		*(ppd++) = bWhiteMove; // a lot of redundancy here
		ReqData.add_data_val((bWhiteTurn) ? 1.0f : 0.0f);
	}
	ReqData.set_num_params(xBoardDim * yBoardDim * gNumPieceTypes);

	CaffeSendMsg(*Caffe1stClickSocket, MsgReq);

	CaffeIpc MsgScore;
	CaffeRcvMsg(*Caffe1stClickSocket, MsgScore);
	if (MsgScore.type() != CaffeIpc_MsgType_PREDICT_RESULT) {
		std::cerr << "Problem with server. Did not send data response. Client bids bye!\n";
		gbGameInitFailed = true;
		return;
	}
	else {
		const CaffeIpc_DataParam& RetData = MsgScore.data_param();
		std::cout << RetData.num_params() << " data items reported. Received: " << RetData.data_val_size() << ".\n";
		vector<pair<float, int> > ScoreMax;
		for (int id = 0; id < RetData.num_params(); id++) {
			//std::cerr << RetData.data_val(id) << ", ";
			ScoreMax.push_back(make_pair(RetData.data_val(id), id));
		}
		//std::cerr << endl;
		std::sort(ScoreMax.begin(), ScoreMax.end());
		int RetScoreSpot = ScoreMax.back().second;
		int RealScoreSpot = ScoreToClass(MoveScore);
		std::cerr << "Caffe predicted score spot of " << RetScoreSpot << " vs score of " << RealScoreSpot << endl;
		

	}
}

void NoRecord(	char* OldBoard, char * NewBoard, bool bWhiteTurn, bool bValid, 
						int MoveScore, vector<vector<uint> >& Records, 
						int xBoardDim, int yBoardDim, uint& NumPieceTypes,
						int iCol1stClick, int iRow1stClick, 
						int iCol2ndClick, int iRow2ndClick) 
{
	return;
}

void OneRecordSnapshot(	char* OldBoard, char * NewBoard, bool bWhiteTurn, bool bValid, 
						int MoveScore,	vector<vector<uint> >& Records, int xBoardDim, 
						int yBoardDim, uint& NumPieceTypes, int iCol1stClick, 
						int iRow1stClick, int iCol2ndClick, int iRow2ndClick) 
{
	if ((gGameMode == tgmRecord1stClickMap) || (gGameMode == tgmRecordClicksMap)) {
		return;
	}
	
	typedef void (*pfnRecordValid)(	char* OldBoard, char * NewBoard, bool bWhiteTurn, 
									bool bValid, int MoveScore,
									vector<vector<uint> >& Records, 
									int xBoardDim, int yBoardDim, uint& NumPieceTypes, 
									int iCol1stClick, int iRow1stClick, 
									int iCol2ndClick, int iRow2ndClick) ;
	
	pfnRecordValid pfn = NULL;
	switch (gGameMode) {
		case tgmRecordValid:
			pfn = RecordValid;
			break;
		case tgmRecordResult:
			pfn = RecordResult;
			break;
		case tgmRecordScore:
			pfn = RecordScore;
			break;
		case tgmGetScore:
			pfn = GetScoreFromCaffe;
			break;
		case tgmUseCaffe:
			pfn = NoRecord;
			break;
		case tgmRecord1stClickValid:
			pfn = Record1stClickValid;
			break;
		case tgmRecordClicksValid:
		case tgmRecordClicksValidError:
		case tgmRecordElsValid:
			pfn = RecordClicksValid;
			break;
		case tgmRecord1stClickMap:
			pfn = NULL;
			break;
		case tgmRecordClicksMap:
			pfn = NULL;
			break;
		case tgmInvalid:
		default:
			pfn = NULL;
			break;
	}
	if (pfn == NULL) {
			std::cerr << "Invalid game mode\n";
			return;		
	}
	
	(*pfn)(	OldBoard, NewBoard, bWhiteTurn, bValid, MoveScore, Records, 
			xBoardDim, yBoardDim, NumPieceTypes, 
			iCol1stClick, iRow1stClick, iCol2ndClick, iRow2ndClick);
	
}


bool PlayOneGame()
{
	if (gbGameInitFailed) {
		return false;
	}
	
	bool bDoExit = false;
	bool bWhiteWins;
	
	const bool cbInputValid = true;
	//const bool cbWhiteWin = true;
	const bool cbTurnOver = true;
	const bool cbLastTurn = true;

	char TheBoard[GamerBoardSize * GamerBoardSize];
	memset(TheBoard, 0, sizeof(TheBoard));
	memcpy(TheBoard, GamerStartBoard, GamerBoardSize * GamerBoardSize);
	const bool bShowGames = false;
	const bool bHumanPlaying = false;

	if (bShowGames) {
		DisplayBoard(TheBoard);
	}

	int turn = 0;
	bool bWhiteTurn = true;
	switch (GamerWhoStarts[0]) {
		case 'w':
		case 'W':
			bWhiteTurn = true;
			break;
		case 'b':
		case 'B':
			bWhiteTurn = false;
			break;
		case 'r':
		case 'R':
			bWhiteTurn = rand() % 2;
			break;
		default:
			cerr << "Error in who starts setting\n";
			break;
	}

	if (		gGameMode != tgmRecord1stClickValid 
			&&	gGameMode != tgmRecordClicksValid 
			&&  gGameMode != tgmRecordClicksValidError
			&&	gGameMode != tgmRecordElsValid
			&&	gGameMode != tgmRecordResult
			&&	gGameMode != tgmRecordValid) {
		OneRecordSnapshot(	TheBoard, TheBoard, bWhiteTurn, true, 0,
							GameRecords, GamerBoardSize, GamerBoardSize, 
							NumPieceTypes, 0, 0, 0, 0 );
	}
	
	while (turn < 10000000) {
		turn++;
		int xFrom = 0, yFrom = 0, xTo = 0, yTo = 0;
		vector<OneMove > Moves;
		int MoveScore = 0;
		
		if (GamerbBestMove) {
			MoveScore = ProgAlphaBeta(	Moves, TheBoard, bWhiteTurn, 
										(bWhiteTurn ? GamerWhiteDepth : GamerBlackDepth), 
										+INT_MAX, -INT_MAX, 0);
			cout << Moves.size() << " moves selected \n";
		}
		else {
			GamerRegexGetValidMoves(Moves, GamerBoardSize, TheBoard, bWhiteTurn);
		}
		char PrevBoard[GamerBoardSize * GamerBoardSize];
		memcpy(PrevBoard, TheBoard, GamerBoardSize * GamerBoardSize);
		if (Moves.size() == 0) {
			if (bShowGames) {
				ToScreen << (bWhiteTurn ? "White" : "Black") << " lost!" << endl;
				ToScreen << "Game end\n";
			}
			CreateSnapshot(bWhiteTurn, !cbInputValid, cbTurnOver, cbLastTurn,
							xFrom, yFrom, xTo, yTo, TheBoard);
			bDoExit = true;
			bWhiteWins = !bWhiteTurn;
			MoveScore = (bWhiteWins ? 100 : -100);
			if (	gGameMode != tgmRecord1stClickValid 
				&&	gGameMode != tgmRecordClicksValid 
				&&	gGameMode != tgmRecordClicksValidError 
				&&	gGameMode != tgmRecordElsValid
				&&	gGameMode != tgmRecordResult
				&&	gGameMode != tgmRecordValid) {
				OneRecordSnapshot(	PrevBoard, TheBoard, bWhiteTurn, true, MoveScore,
									GameRecords, GamerBoardSize, GamerBoardSize, 
									NumPieceTypes, 0, 0, 0, 0);
			}
			break;
			

		}
		
		if (GamerbLearnValids) {
			TestLearnedRulesOnCurrentBoard(	GamerBoardSize, TheBoard,  
											bWhiteTurn);
			char TestBoard[GamerBoardSize * GamerBoardSize];
			for (uint im = 0; im < Moves.size(); im++) {
				memcpy(TestBoard, TheBoard, GamerBoardSize * GamerBoardSize);
				int xStart, yStart;
				bool bStartValid = false;
				for (uint iFV = 0; iFV < Moves[im].Deltas.size(); iFV++) {
					BoardDelta& rec = Moves[im].Deltas[iFV];
					TestBoard[(rec.y * GamerBoardSize) + rec.x] = rec.NewVal;
					if (iFV == 0) {
						bStartValid = true;
						xStart = rec.x; yStart = rec.y;
					}
				}
				if (!bStartValid) {
					continue;
				}
				int MoveScore = 0;
				const bool cbConseqFinal = true;
				GamerRegexExecConseq(MoveScore, GamerBoardSize, TestBoard, bWhiteTurn, !cbConseqFinal);

				CreateBasicRegPat(	GamerBoardSize, TheBoard, TestBoard, 
									bWhiteTurn, xStart, yStart);

			}
			TestRegexLearnedRules(GamerBoardSize, TheBoard, bWhiteTurn);
		}
		vector<tuple<int, int, int, int, OneMove > > ValidMoves;
		if (gGameMode == tgmRecordValid) {
			bool bModMade = false;
			char ModBoard[GamerBoardSize * GamerBoardSize];
			// if (rand() % 2 > 0)	
			int NumBadMoves = rand() % Moves.size();
			for (int iSelMove = 0; iSelMove < NumBadMoves; iSelMove++) {
				// int iSelMove = rand() % Moves.size();
				memcpy(ModBoard, TheBoard, GamerBoardSize * GamerBoardSize);
				int NumDeltas = Moves[iSelMove].Deltas.size();
				int iChange = rand() % NumDeltas;
				for (uint iFV = 0; iFV < NumDeltas; iFV++) {
					BoardDelta& rec = Moves[iSelMove].Deltas[iFV];
					//TheBoard[(rec.y * GamerBoardSize) + rec.x] = rec.NewVal;
					int xRec = rec.x;
					int yRec = rec.y;
					if (iFV == iChange) {
						xRec = rand() % GamerBoardSize;
						yRec = rand() % GamerBoardSize;
					}
					if (iFV == 0) {
						xFrom = xRec;
						yFrom = yRec;
					}
					ModBoard[(yRec * GamerBoardSize) + xRec] = rec.NewVal;

				}
				if (TestBrdValidByRegex(GamerBoardSize, TheBoard, ModBoard, bWhiteTurn, 
										xFrom, yFrom) < 0) {
					bModMade = true;
					OneRecordSnapshot(	TheBoard, ModBoard, bWhiteTurn, false, 
										 MoveScore,GameRecords, 
										GamerBoardSize, GamerBoardSize, 
										NumPieceTypes, 0, 0, 0, 0 );
				}
			}
		}
		else if (gGameMode == tgmRecord1stClickValid) {
			vector<OneMove > Click1stMoves;
			for (int iRow = 0; iRow < GamerBoardSize; iRow++) {
				for (int iCol = 0; iCol < GamerBoardSize; iCol++) {
					Click1stMoves.clear();
					GamerRegexGetValidMoves(Click1stMoves, GamerBoardSize, 
											TheBoard, bWhiteTurn,
											iCol, iRow);
					OneRecordSnapshot(	TheBoard, TheBoard, bWhiteTurn, 
										(Click1stMoves.size() > 0), MoveScore,
										GameRecords, GamerBoardSize, GamerBoardSize, 
										NumPieceTypes, iCol, iRow, 0, 0 );	
				}
			}
		}
		else if (gGameMode == tgmRecord1stClickMap) {
			vector<OneMove > Click1stMoves;
			vector<vector<bool> > ClickMap(
				GamerBoardSize, vector<bool>(GamerBoardSize, false));
			for (int iRow = 0; iRow < GamerBoardSize; iRow++) {
				for (int iCol = 0; iCol < GamerBoardSize; iCol++) {
					Click1stMoves.clear();
					GamerRegexGetValidMoves(Click1stMoves, GamerBoardSize, 
											TheBoard, bWhiteTurn,
											iCol, iRow);
					if (Click1stMoves.size() > 0) {
						ClickMap[iRow][iCol] = true;
					}
				}
			}
			Record1stClickMap(	TheBoard, bWhiteTurn, 
								GameRecords, GamerBoardSize, GamerBoardSize, 
								NumPieceTypes, ClickMap );	
		}
		else if (gGameMode == tgmRecordClicksMap) {
			vector<OneMove > Click1stMoves;
			vector<vector<bool> > ClickMap(
				GamerBoardSize, vector<bool>(GamerBoardSize, false));
			vector <pair<int, int> > ClickList;
			for (int iRow = 0; iRow < GamerBoardSize; iRow++) {
				for (int iCol = 0; iCol < GamerBoardSize; iCol++) {
					Click1stMoves.clear();
					GamerRegexGetValidMoves(Click1stMoves, GamerBoardSize, 
											TheBoard, bWhiteTurn,
											iCol, iRow);
					if (Click1stMoves.size() > 0) {
						ClickList.push_back(make_pair(iCol, iRow));
					}
				}
			}
			for (int ic = 0; ic < ClickList.size(); ic++) {
				vector<OneMove > ClickBothMoves;
				for (int ieRow = 0; ieRow < GamerBoardSize; ieRow++) {
					for (int ieCol = 0; ieCol < GamerBoardSize; ieCol++) {
						ClickBothMoves.clear();
						GamerRegexGetValidMoves(ClickBothMoves, GamerBoardSize, 
												TheBoard, bWhiteTurn,
												ClickList[ic].first, 
												ClickList[ic].second, 
												ieCol, ieRow);
						if (ClickBothMoves.size() > 0) {
							ClickMap[ieRow][ieCol] = true;
						}
					}
				}
				RecordClicksMap(	TheBoard, bWhiteTurn, 
									GameRecords, GamerBoardSize, GamerBoardSize, 
									NumPieceTypes, 
									ClickList[ic].first, ClickList[ic].second,
									ClickMap );	
			}
		}
		else if ((gGameMode == tgmRecordClicksValid) || (gGameMode == tgmRecordElsValid)) {
			vector<OneMove > Click1stMoves;
			for (int iRow = 0; iRow < GamerBoardSize; iRow++) {
				for (int iCol = 0; iCol < GamerBoardSize; iCol++) {
					Click1stMoves.clear();
					GamerRegexGetValidMoves(Click1stMoves, GamerBoardSize, 
											TheBoard, bWhiteTurn,
											iCol, iRow);
					if (Click1stMoves.size() > 0) {
						vector<OneMove > ClickBothMoves;
						for (int ieRow = 0; ieRow < GamerBoardSize; ieRow++) {
							for (int ieCol = 0; ieCol < GamerBoardSize; ieCol++) {
								ClickBothMoves.clear();
								GamerRegexGetValidMoves(ClickBothMoves, GamerBoardSize, 
														TheBoard, bWhiteTurn,
														iCol, iRow, 
														ieCol, ieRow);
								OneRecordSnapshot(	TheBoard, TheBoard, bWhiteTurn, 
													(ClickBothMoves.size() > 0), 
													MoveScore,
													GameRecords, GamerBoardSize, 
													GamerBoardSize, 
													NumPieceTypes, iCol, iRow, 
													ieCol, ieRow );
							}
						}
					}						
				}
			}
		}
		else if (gGameMode == tgmRecordClicksValidError) {
			vector<OneMove > Click1stMoves;
			vector <pair<int, int> > ClickList;
			for (int iRow = 0; iRow < GamerBoardSize; iRow++) {
				for (int iCol = 0; iCol < GamerBoardSize; iCol++) {
					Click1stMoves.clear();
					GamerRegexGetValidMoves(Click1stMoves, GamerBoardSize, 
											TheBoard, bWhiteTurn,
											iCol, iRow);
					bool bMovesValid = (Click1stMoves.size() > 0);
					if (bMovesValid ) {
						ClickList.push_back(make_pair(iCol, iRow));
					}
				}
			}
			vector<OneMove > ClickBothMoves;
			for (int ic = 0; ic < ClickList.size(); ic++) {
				for (int ieRow = 0; ieRow < GamerBoardSize; ieRow++) {
					for (int ieCol = 0; ieCol < GamerBoardSize; ieCol++) {
						bool bCaffeSaysValid = 
							(CaffeAreClicksValid(	TheBoard, bWhiteTurn, 
													GamerBoardSize, GamerBoardSize, 
													ClickList[ic].first, 
													ClickList[ic].second,
													ieCol, ieRow));
						ClickBothMoves.clear();
						GamerRegexGetValidMoves(ClickBothMoves, GamerBoardSize, 
												TheBoard, bWhiteTurn,
												ClickList[ic].first, 
												ClickList[ic].second, 
												ieCol, ieRow);
						bool bGamerSaysValid = (ClickBothMoves.size() > 0);
						if (bCaffeSaysValid != bGamerSaysValid) {
							OneRecordSnapshot(	TheBoard, TheBoard, bWhiteTurn, 
												bGamerSaysValid, 
												MoveScore,
												GameRecords, GamerBoardSize, 
												GamerBoardSize, 
												NumPieceTypes, 
												ClickList[ic].first, 
												ClickList[ic].second,
												ieCol, ieRow );
						}
					}
				}
			}							
		}
		else if (gGameMode == tgmRecordResult)  {
			ValidMoves.clear();
			vector<OneMove > Click1stMoves;
			vector <pair<int, int> > ClickList;
			for (int iRow = 0; iRow < GamerBoardSize; iRow++) {
				for (int iCol = 0; iCol < GamerBoardSize; iCol++) {
					Click1stMoves.clear();
					GamerRegexGetValidMoves(Click1stMoves, GamerBoardSize, 
											TheBoard, bWhiteTurn,
											iCol, iRow);
					bool bMovesValid = (Click1stMoves.size() > 0);
					if (bMovesValid ) {
						ClickList.push_back(make_pair(iCol, iRow));
					}
				}
			}
			for (int ic = 0; ic < ClickList.size(); ic++) {
				vector<OneMove > ClickBothMoves;
				for (int ieRow = 0; ieRow < GamerBoardSize; ieRow++) {
					for (int ieCol = 0; ieCol < GamerBoardSize; ieCol++) {
						ClickBothMoves.clear();
						GamerRegexGetValidMoves(ClickBothMoves, GamerBoardSize, 
												TheBoard, bWhiteTurn,
												ClickList[ic].first, 
												ClickList[ic].second, 
												ieCol, ieRow);
						if (ClickBothMoves.size() > 0) {
							// assume here that 1st and 2nd click specify a unique move
							ValidMoves.push_back(make_tuple(
								ClickList[ic].first, ClickList[ic].second, 
								ieCol, ieRow, ClickBothMoves[0]));
						}
					}
				}
			}

		}
		else if (gGameMode == tgmUseCaffe) {
			char TestBoard[GamerBoardSize * GamerBoardSize];
			memcpy(TestBoard, TheBoard, GamerBoardSize * GamerBoardSize);
			vector <pair<int, int> > Valid1stClickList;
			for (int iRow = 0; iRow < GamerBoardSize; iRow++) {
				for (int iCol = 0; iCol < GamerBoardSize; iCol++) {
					if (CaffeIs1stClickValid(TheBoard, bWhiteTurn, 
											GamerBoardSize, GamerBoardSize, 
											iCol, iRow)) {
						Valid1stClickList.push_back(make_pair(iCol, iRow));
					}
				}
			}
			vector <tuple<int, int, int , int> > ValidClicksList;
			for (int iv = 0; iv < Valid1stClickList.size(); iv++) {
				for (int ieRow = 0; ieRow < GamerBoardSize; ieRow++) {
					for (int ieCol = 0; ieCol < GamerBoardSize; ieCol++) {
						if (CaffeAreClicksValid(TheBoard, bWhiteTurn, 
												GamerBoardSize, GamerBoardSize, 
												Valid1stClickList[iv].first, 
												Valid1stClickList[iv].second,
												ieCol, ieRow)) {	
							ValidClicksList.push_back(
								make_tuple(	Valid1stClickList[iv].first, 
											Valid1stClickList[iv].second, 
											ieCol, ieRow));
						}							
					}
				}
			}							
			for (int iv = 0; iv < ValidClicksList.size(); iv++) {
				int x0 = get<0>(ValidClicksList[iv]); 
				int y0 = get<1>(ValidClicksList[iv]);
				int x1 = get<2>(ValidClicksList[iv]); 
				int y1 = get<3>(ValidClicksList[iv]);
				// an alternative state consists of the x, y position and a set of alternatives
				vector<tuple<int, int, vector<int> > > AltStates;
				CaffeClicksResult(	PrevBoard, TestBoard, bWhiteTurn, 
									GamerBoardSize, GamerBoardSize, 
									x0, y0, x1, y1,
									AltStates, 0.1f);
				if (AltStates.size() > 0) {
					cerr << "Uncertainty about the following positions: \n";
					for (auto Alt : AltStates) {
						cerr << "Position " << get<0>(Alt) << ", " << get<1>(Alt) << ": ";
						for (auto ia : get<2>(Alt)) {
							cerr << ia << ", ";
						}
						cerr << endl;
					}
				}
				bool bRegexIsValid = (TestBrdValidByRegex(
					GamerBoardSize, PrevBoard, TestBoard, bWhiteTurn, x0, y0) >= 0);
				cerr << "Regex rules board " << (bRegexIsValid ? "valid" : "invalid!!!!!!!!!!!!") << endl;
				
				bool bUncertain = false;
				if (CaffeBoardValid(	PrevBoard, TestBoard, bWhiteTurn, 
										GamerBoardSize, GamerBoardSize,
										AltStates, 1.0f, bUncertain)) {
					if (bUncertain) cerr << "Caffe validity uncertain!\n";
					cerr	<< "Caffe board for " 
							<< x0 << ", " << y0 << "-> "
							<< x1 << ", " << y1 << " "	<< endl;
					DisplayBoard(TestBoard);
				}
				else {
					if (bUncertain) cerr << "Caffe validity uncertain!\n";
					cerr << "Caffe board declared invalid\n";
					cerr	<< "BAD Caffe board for " 
							<< x0 << ", " << y0 << "-> "
							<< x1 << ", " << y1 << " "	<< endl;
					DisplayBoard(TestBoard);
				}
			}
		}
		//int iSelMove = MoveSelect(Moves, bWhiteTurn, TheBoard); //  rand() % Moves.size();
		if (gGameMode == tgmRecordResult) {
			Moves.clear();
			for (int iv = 0; iv < ValidMoves.size(); iv++) {
				Moves.push_back(std::get<4>(ValidMoves[iv]));
				//OneMove m = std::get<4>(ValidMoves[iv]);
			}
		}
		int iSelMove = rand() % Moves.size();
		bool bToMove = false;
		for (uint iFV = 0; iFV < Moves[iSelMove].Deltas.size(); iFV++) {
			BoardDelta& rec = Moves[iSelMove].Deltas[iFV];
			TheBoard[(rec.y * GamerBoardSize) + rec.x] = rec.NewVal;
			if (iFV == 0) {
				xFrom = rec.x;
				yFrom = rec.y;
			}
			else {
				bToMove = true;
				xTo = rec.x;
				yTo = rec.y;
			}
		}
		if (bToMove) {		
			if (bShowGames) {
				ToScreen	<< endl << (bWhiteTurn ? "White " : "Black ") 
							<< "moved from " <<  char('a' + xFrom) 
							<< char('1' + yFrom) << " to "
							<< char('a' + xTo) << char('1' + yTo)
							<< endl ;
			}
		}
		else {
			if (bShowGames) {
				ToScreen	<< endl << (bWhiteTurn ? "White " : "Black ") 
							<< "placed at " <<  char('a' + xFrom) 
							<< char('1' + yFrom) << endl ;
			}
		}
		//int MoveScore = 0;
		const bool cbConseqFinal = true;
		GamerRegexExecConseq(MoveScore, GamerBoardSize, TheBoard, bWhiteTurn, cbConseqFinal);
		if (MoveScore != 0) {
			if (bShowGames) {
				ToScreen << "Score is now: " << MoveScore << endl;
			}
		}
		int x0 = 0, x1 = 0, y0 = 0, y1 = 0;
		if (gGameMode == tgmRecordResult) {
			x0 = get<0>(ValidMoves[iSelMove]); y0 = get<1>(ValidMoves[iSelMove]);
			x1 = get<2>(ValidMoves[iSelMove]); y1 = get<3>(ValidMoves[iSelMove]);
		}

		if (	gGameMode != tgmRecord1stClickValid 
			&&	gGameMode != tgmRecordClicksValidError 
			&&	gGameMode != tgmRecordClicksValid
			&&	gGameMode != tgmRecordElsValid) {
			OneRecordSnapshot(	PrevBoard, TheBoard, bWhiteTurn, true, MoveScore,
								GameRecords, GamerBoardSize, GamerBoardSize, 
								NumPieceTypes, x0, y0, x1, y1 );
		}
		if (MoveScore <= -100) {
			if (bShowGames) ToScreen << "White lost!\n";
			bDoExit = true;
			bWhiteWins = false;
			break;			
		}
		if (MoveScore >= 100) {
			if (bShowGames) ToScreen << "Black lost!\n";
			bDoExit = true;
			bWhiteWins = true;
			break;
		}
		if (bShowGames) {
			DisplayBoard(TheBoard);
		}
		CreateSnapshot(bWhiteTurn, cbInputValid, cbTurnOver, !cbLastTurn,
						xFrom, yFrom, xTo, yTo, TheBoard);

		bWhiteTurn = !bWhiteTurn;
	}
	if (bDoExit) {
		if (bWhiteWins) NumWhiteWins++; else NumBlackWins++;
		return bWhiteWins;
	}
	cerr << "Error should not get here.\n";
	return true;
}

void CGotitEnv::PlayGame()
{
	if (gbGameInitFailed) {
		return;
	}
	
	string sbCalcMoves;
	bGameCalcMoves = false;
	if (GetImplemParam(sbCalcMoves, "Implem.Param.FnParam.PlayGame.CalcMoves")) {
		if (sbCalcMoves == "Yes") {
			bGameCalcMoves = true;
			bGameEvaluating = true;
		}
	}
	bool bWhiteWon = PlayOneGame();
	if (bWhiteWon ) NumWhiteWins++; else NumBlackWins++;

	if (((NumWhiteWins + NumBlackWins) % 10) == 0) {
		cerr << "White won " << NumWhiteWins << " at a rate of " << ((NumWhiteWins * 100) / (NumWhiteWins + NumBlackWins)) << "%\n";
	}

}

void CGotitEnv::GameCreateHD5()
{
	string hd5fname;
	if (GetImplemParam(hd5fname, "Implem.Param.FnParam.HD5.FileName")) {
		WriteHD5Output(hd5fname.c_str(), GameRecords, GamerBoardSize, GamerBoardSize, NumPieceTypes);
		GameRecords.clear();
	}
	else {
		cerr << "No HD5 file name provided\n";
	}

}

void OneCaffeConnect(	tcp::socket*& ServiceSocket, string& sCaffeHostName,
						const string& sPortNum, const char * model_file,
						const char * trained_file, const char * input_layer_name, 
						const char * output_layer_name)
{
	
	string CaffeCmdLine = "/home/abba/bin/CaffeIpcR";
	CaffeCmdLine += string(" ") + sPortNum + "&";
	
	int system_ret;
	
	if (sPortNum != "1549") {
		system_ret = system(CaffeCmdLine.c_str());
		if (system_ret != 0) {
			cerr << "Failed to launch caffe\n";
		}
	}
	
	//, GameRecords, GamerBoardSize, GamerBoardSize, NumPieceTypes);
	GameRecords.clear();

	CaffeIpc MsgInit;
	MsgInit.set_type(CaffeIpc_MsgType_INIT_NET);
	CaffeIpc_InitNetParams& InitMsg = *(MsgInit.mutable_init_net_params());
	InitMsg.set_model_file(model_file);
	InitMsg.set_trained_file(trained_file);
	InitMsg.set_input_layer_name(input_layer_name);
	InitMsg.set_input_layer_bottom_idx(0);
	InitMsg.set_output_layer_name(output_layer_name);
	InitMsg.set_output_layer_top_idx(0);
	InitMsg.set_input_num_channels_idx(1);
	InitMsg.set_input_height_idx(2);
	InitMsg.set_input_width_idx(3);
	//tcp::socket* 
	ServiceSocket = NULL;
	while (ServiceSocket == NULL) {
		usleep(100);
		ServiceSocket = ClientInit(sCaffeHostName.c_str(), stoi(sPortNum));
	}

	CaffeSendMsg(*ServiceSocket, MsgInit);
	{
		CaffeIpc MsgBack;
		CaffeRcvMsg(*ServiceSocket, MsgBack);
		if (MsgBack.type() != CaffeIpc_MsgType_INIT_NET_DONE) {
			std::cerr << "Problem with server init. Client bids bye!\n";
			gbGameInitFailed = true;
			return;
		}
	}
	
}

void CGotitEnv::GameConnectToCaffe()
{
	string sCaffeHostName;
	if (!GetImplemParam(sCaffeHostName, "Implem.Param.FnParam.Caffe.HostName")) {
		cerr << "No Caffe host name provided\n";
		return;
	}
//	string sPortNum;
//	if (!GetImplemParam(sPortNum, "Implem.Param.FnParam.Caffe.PortNum")) {
//		cerr << "No Caffe port number provided\n";
//		return;
//	}

//	string sNumPieceTypes;
//	if (GetImplemParam(sNumPieceTypes, "Implem.Param.FnParam.Caffe.NumPieceTypes")) {
//		gNumPieceTypes = stoi(sNumPieceTypes);
//	}
//	else {
//		cerr << "Must specify the number of piece types\n";
//		return;
//	}

//	OneCaffeConnect(CaffeSocket, sCaffeHostName, string("1544"), 
//				"/home/abba/caffe/toys/ScoreByConv/train.prototxt",
//				"/guten/data/ScoreByConv/data/s.caffemodel",
//				"conv1", "ip2");

	// Don't use port 1549 - I reserved it for debugging
	gDataDim1stClick = 15;
	OneCaffeConnect(Caffe1stClickSocket, sCaffeHostName, string("1544"), 
					"/home/abba/caffe/toys/Valid1stClick/train.prototxt",
					"/guten/data/Valid1stClick/data/v.caffemodel",
					"conv1", "ip2");
			

	gDataDimValidClicks = 16;
	OneCaffeConnect(ClicksValidSocket, sCaffeHostName, string("1545"), 
					"/home/abba/caffe/toys/ValidClicks/train.prototxt",
					"/guten/data/ValidClicks/data/v.caffemodel",
					"conv1", "ip2");
	
	gDataDimClicksResult = 16;
	OneCaffeConnect(ClicksResultSocket, sCaffeHostName, string("1546"), 
					"/home/abba/caffe/toys/ClicksResult2/train.prototxt",
					"/guten/data/ClicksResult/data/c2.caffemodel",
					"encode1", "decode1neuron");

	
	gDataDimBoardValid = 27;
	OneCaffeConnect(BoardValidSocket, sCaffeHostName, string("1547"), 
					"/home/abba/caffe/toys/ValidConv/train.prototxt",
					"/guten/data/ValidConv/data/v.caffemodel",
					"conv1", "ip2");
}




