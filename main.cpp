#include "stdafx.h"

#include "MascReader.h"

#include <chrono>
//#include <chrono_io>


map<void *, int> MemMap;
static int AllocCount = 0;
static bool gbInMain = false;
static bool gbBkptHit = false;
static int BadGuy = 0;
bool xbDebugMemHere = true;

//#define _DEBUG_MEM
#ifdef _DEBUG_MEM
void * MLBNew(size_t size) {
	void *p = malloc(size);
	if (!xbDebugMemHere) {
		return p;
	}
	if (!gbInMain) {
		return p;
	}
	static bool bInFunc = false;
	if (bInFunc) {
		return p;
	}
	bInFunc = true;
	// to use, put a breakpoint at the end of main and check that
	// MemMap is of size 0. If not, inspect one of its elements
	// and copy the second of that elemond to the equality check
	// in the line below and put a breakpoint on the useless 
	// gbBkptHit = true; line
	if (AllocCount == 1421480) {
		gbBkptHit = true;
		cerr << "Hit the accol\n";
		//AllocCount /= BadGuy;
	}
	MemMap[p] = AllocCount;
	bInFunc = false;
	AllocCount++;
	return p;

}

void MLBDelete(void * p) {
	free(p);
	if (!xbDebugMemHere) {
		return ;
	}
	if (!gbInMain) {
		return;
	}
	static bool bInFunc = false;
	if (bInFunc) {
		return;
	}
	map<void *, int>::iterator it = MemMap.find(p);
	if (it == MemMap.end()) {
		cerr << "Memory allocation not found\n";
		return;
	}
	bInFunc = true;
	MemMap.erase(it);
	bInFunc = false;

}

void* operator new (size_t size)
{
	return MLBNew(size);
}

void *operator new [](size_t size)
{
	return MLBNew(size);
}

void operator delete(void* p)
{
	MLBDelete(p);
}

void operator delete [](void* p)
{
	MLBDelete(p);
}
#endif /// _DEBUG

void MemMapPrint() {
	map<void *, int>::iterator itmm = MemMap.begin();
	for (; itmm != MemMap.end(); itmm++) {
		cerr << "Unfreed item #: " << itmm->second << endl;
		getchar();
		//break;
	}
}

#ifdef GOTIT_LINUX
int main()
#else 
int _tmain(int argc, _TCHAR* argv[])
#endif
{
	gbInMain = true;
	lmain();
	MemMapPrint();
	cerr << "Press any key to exit....";
	getchar();
	return 0;
}