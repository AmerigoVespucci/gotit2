// GrpsFile.cpp : Windows-based version for groups special file.
//

#include "stdafx.h"

#include "MascReader.h"

const int cBlkSize = 1024;

struct SDataBlk {
	SDataBlk(streampos pos);
	SDataBlk();
	uint Size;
	streampos Start;
	bool bCont;
	streampos Next;
	bool bInUse;
	uint CurrPos;
	bool bInWrite;
	uint UsedThisBlk;
};

SDataBlk::SDataBlk(streampos pos)
{
	Size = cBlkSize;
	Start = pos;
	bCont = false;
	Next = 0;
	bInUse = true;
	CurrPos = 0;
	bInWrite = true;
	UsedThisBlk = 0;
}

SDataBlk::SDataBlk()
{
	Size = 0;
	Start = 0;
	bCont = false;
	Next = 0;
	bInUse = false;
	CurrPos = 0;
	bInWrite = false;
	UsedThisBlk = cBlkSize + 1; // invalid val
}

class CGrpFile {
public:
	CGrpFile();
	~CGrpFile();
	bool bGetState() { return bOpenGood;  }
	streampos getStartBlk();
	void StartRead(streampos StartPos);
	void StartWrite(streampos StartPos);
	void Write(streampos StartPos, const char * Data, int NumBytes);
	void Read(streampos StartPos, char * Data, int NumBytes);

	const string GrpFileName = GRPS_FNAME_NAME;


	fstream f;
private:
	bool bOpenGood;


};

streampos CGrpFile::getStartBlk()
{
	f.seekp(0, ios::end);
	streampos NewStart = f.tellp();
	SDataBlk StartBlk(NewStart);
	f.write((char *)&StartBlk, sizeof(StartBlk));
	static char HolderBlock[cBlkSize];
	f.write(HolderBlock, cBlkSize);
	f.flush();
	f.seekp(NewStart);
	f.seekp(sizeof(StartBlk), ios::cur);
	return NewStart;
}

void  CGrpFile::Write(streampos StartPos, const char * Data, int NumBytes)
{
	SDataBlk CurrBlk;
	streampos Pos = StartPos;
	//bool bFirstBlock = true;

	while (true) {
		f.seekg(Pos);
		f.read((char *)&CurrBlk, sizeof(CurrBlk));

	}
	f.seekg(StartPos);
	/*
	f.read((char *)&StartBlk, sizeof(StartBlk));
	if ((StartBlk.CurrPos + NumBytes) < StartBlk.Size) {
		f.seekp(StartPos);
		f.seekp(sizeof(StartBlk) + StartBlk.CurrPos, ios::cur);
		f.write(Data, NumBytes);
		StartBlk.CurrPos += NumBytes;
		f.flush();
		f.seekp(StartPos);
		f.write((char *)&StartBlk, sizeof(StartBlk));
		f.flush();

		//char TestBuf[] = "xxxxxxxxxx";
		//f.seekg(StartPos);
		//f.seekg(sizeof(StartBlk) + StartBlk.CurrPos - NumBytes, ios::cur);
		//f.read(TestBuf, NumBytes);
		//f.flush();
	}
	else {
		SDataBlk CurrBlk = StartBlk;
		while (true) {
			if (CurrBlk.bCont) {
				streampos NextPos = CurrBlk.Next;
			}
			else {
				if (NumBytes > cBlkSize) {
					cerr << "Error! Individual write to grps file cannot exceed block size of " << cBlkSize << ". Requested " << NumBytes << endl;
					return;
				}
				StartBlk.CurrPos += NumBytes;
				f.seekp(0, ios::end);
				streampos NextStart = f.tellp();
				SDataBlk NextBlk(NextStart);
				f.write((char *)&NextBlk, sizeof(NextBlk));
				f.write(Data, NumBytes);
				static char HolderBlock[cBlkSize];
				f.write(HolderBlock, cBlkSize);
				f.flush();

			}
		}
	}
	*/
}

void CGrpFile::StartRead(streampos StartPos)
{
	SDataBlk StartBlk;
	f.seekg(StartPos);
	f.read((char *)&StartBlk, sizeof(StartBlk));
	StartBlk.CurrPos = 0;
	StartBlk.bInWrite = false;
	f.seekp(StartPos);
	f.write((char *)&StartBlk, sizeof(StartBlk));
	f.flush();
}

void CGrpFile::StartWrite(streampos StartPos)
{
	SDataBlk StartBlk;
	f.seekg(StartPos);
	f.read((char *)&StartBlk, sizeof(StartBlk));
	StartBlk.CurrPos = 0;
	StartBlk.bInWrite = true;
	f.seekp(StartPos);
	f.write((char *)&StartBlk, sizeof(StartBlk));
	f.flush();
}

void  CGrpFile::Read(streampos StartPos, char * Data, int NumBytes)
{
	SDataBlk StartBlk;
	f.seekg(StartPos);
	f.read((char *)&StartBlk, sizeof(StartBlk));
	if ((StartBlk.CurrPos + NumBytes) < StartBlk.Size) {
		f.seekg(StartBlk.CurrPos, ios::cur);
		f.read(Data, NumBytes);
		StartBlk.CurrPos += NumBytes;
		f.flush();
		f.seekp(StartPos);
		f.write((char *)&StartBlk, sizeof(StartBlk));
		f.flush();
	}
}

CGrpFile::CGrpFile()
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif
	bOpenGood = false;

	//if (((Flags & ios::in) != 0) && !exists(path(GrpFileName))) {
	//	return false;
	//}

	//for (uint iTry = 0; iTry < cSafeOpenNumTries; iTry++) {
		try {
#ifdef GOTIT_LINUX
			f.open(GrpFileName, ios::in | ios::out | ios::binary);
#else
			f.open(GrpFileName, ios::in | ios::out | ios::binary, _SH_DENYNO);
#endif
		}
		catch (fstream::failure e) {
			cerr << "CGrpFile::CGrpFile: Exception opening/reading/closing file " << GrpFileName << " with error " << e.what() << endl;
		}

		if (f.is_open()) {
			bOpenGood = true;
			return;
		}

		//Sleep(cSafeopenWaitTime);
	//}
	//return false;
		cerr << "CGrpFile: Failed to open " << GrpFileName << endl;
}

CGrpFile::~CGrpFile()
{
	f.close();
}

void TestGrpsFile()
{
	CGrpFile GrpsFile;
	streampos Start = GrpsFile.getStartBlk();
	const char Msg[] = "Hello";
	GrpsFile.Write(Start, Msg, sizeof(Msg)+1);
	char ReadBuf[16];
	GrpsFile.StartRead(Start);
	GrpsFile.Read(Start, ReadBuf, sizeof(Msg) + 1);
	cout << "done\n";
}

