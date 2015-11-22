// RandomForestCREF.cpp : Implements a serial PC version of RF.
//

#include "stdafx.h"
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <algorithm>    // std::sort
#include <string>
#include <map>
#include <vector>
#include <list>

#include "MascReader.h"

using namespace std;


int IntPower(int Base, uint Exponent) {
	int Ret = 1;
	while (Exponent--) {
		Ret *= Base;
	}
	return Ret;
}

static int cLogBitsLookup = 4; // pretty good even with 2
static int cLogValsLookup = IntPower(2, cLogBitsLookup);
vector<int> gLookup(cLogValsLookup + 1, 0);
const int LogFixedFactor = 256;
void LogLoopkupInit()
{
	for (int i = 1; i <= cLogValsLookup; ++i) // one extra needed
	{
		// fixed point with 4 binary dec places 
		gLookup[i] = (int)((double)LogFixedFactor * log(i) / log(2));
	}

}

static int LogLookup(int i)
{
	//return (int)(256.0 * log(i) / log(2));
	//if (i >= 0x10000) { return gLookup[i >> 16] + (16 * LogFixedFactor); }
	//else if (i >= 0x1000) { return gLookup[i >> 12] + (12 * LogFixedFactor); }
	//else if (i >= 0x100) { return gLookup[i >> 8] + (8 * LogFixedFactor); }
	//else if (i >= 0x10) { return gLookup[i >> 4] + (4 * LogFixedFactor); }
	//else { return gLookup[i]; }
	for (int iPower = 24; iPower >= 0; iPower -= cLogBitsLookup) {
		int Upper = IntPower(2, iPower);
		if (i >= Upper) {
			int LowerVal = (i >> iPower) << iPower;
			//int NextVal = LowerVal + (1 << iPower);
			int Rem = ((i - LowerVal) * (gLookup[(i >> iPower) + 1] - gLookup[(i >> iPower)])) / (1 << iPower);
			return gLookup[i >> iPower] + Rem + (iPower * LogFixedFactor);
		}
	}
	return 0;
}
#define NOT_YET
#ifndef NOT_YET

#define NUM_MA_COLS 256

int RF()
{
	const int cMiniBatchSize = NUM_MA_COLS - 64;
	const int cMiniBatchLog2Levels = 8; // log2(NUM_MA_COLS)

	map<string, int> ClassVals;
	int NumSrcAttributes = 0;
	LogLoopkupInit();
	vector<int> Sums(cMiniBatchSize, 0);
	vector<int> Carries(cMiniBatchSize, 0);
	vector<list<int> > AttribTotals; // vector of sets, results in list
	//vector<int> AttribTotals(cMiniBatchSize, 0);
	vector<int> OneTestRecord(NumSrcAttributes + 1, 0); // holds class too
	vector<vector<int> > TestRecords(cMiniBatchSize, OneTestRecord);
	vector<int> SetLoc(cMiniBatchSize, 0); // set iD of each record
	struct TreeNode {
		//TreeNode(int aiSrcAttrib, int aiDerivAttrib, int aEntropy, bool abLeaf) {
		//	iSrcAttrib = aiSrcAttrib;
		//	iDerivAttrib = aiDerivAttrib;
		//	Entropy = aEntropy;
		//	bLeaf = abLeaf;
		//	LeftNode = NULL;
		//	RightNode = NULL;
		//}
		TreeNode() {
			bLeaf = false;
			iSrcAttrib = -1;
			iDerivAttrib = -1;
			ClassID = -1;
			Count = 0;
			LeftNode = NULL;
			RightNode = NULL;
		}
		~TreeNode() {
			if (LeftNode != NULL) delete LeftNode;
			if (RightNode != NULL) delete RightNode;
		}
		void TreePrint(int Level) {
			for (int i = 0; i < Level; i++) {
				cout << "---->";
			}
			if (bLeaf) {
				cout << "Leaf. Class " << ClassID << ". Entropy: " << Entropy << ". Best Count: " << Count << ". Num In Set " << NumInSet << "\n";
			}
			else {
				cout << "Branch. Attr: " << iSrcAttrib << ". Deriv Attr " << iDerivAttrib << ". Entropy: " << Entropy << ". Best Count: " << Count << ". Num In Set " << NumInSet << ":\n";
				if (LeftNode != NULL) {
					LeftNode->TreePrint(Level + 1);
				}
				if (RightNode != NULL) {
					RightNode->TreePrint(Level + 1);
				}
			}
		}
		int iSrcAttrib;
		int iDerivAttrib;
		int Entropy;
		bool bLeaf;
		int ClassID;
		int Count;
		int NumInSet;
		struct TreeNode * LeftNode;
		struct TreeNode * RightNode;
	};

	// cNumSearches should be square root of (cNumDerivAttrs * NumSrcAttributes)
	// However for the MLB implementation it may be worthwhile to consider
	// the size of the array, how many values we may want to store in the 
	// the search etc.
	static const int cNumSearches = 8;
	const int cMaxiAdd = 8; // should be cNumSums
	static const int cNumSums = min(cNumSearches * 2 * ClassVals.size(), cMaxiAdd); // 2 is for above/below
	static const int cNumSumsPerPyramid = 8;
	const int cNumLevels = 6;
	for (int iLevel = 0; iLevel < cNumLevels; iLevel++) {
		int NumSets = IntPower(2, iLevel);

		TreeNode * TreeRootNode = new TreeNode();
		vector<TreeNode *> SetToTreeNode(1, TreeRootNode);

		vector<int> NewSetLoc(cMiniBatchSize, 0); // set iD of each record
		vector<TreeNode *> NewSetToTreeNode(NumSets * 2, NULL);


		for (int iSet = 0; iSet < NumSets; iSet++) {
			TreeNode * CurrNode = SetToTreeNode[iSet];
			if (CurrNode->bLeaf) {
				NewSetToTreeNode[(iSet * 2)] = CurrNode;
				NewSetToTreeNode[(iSet * 2) + 1] = CurrNode;
				continue;
			}
			list<int>::reverse_iterator ril = AttribTotals[iSet].rbegin();
			int NumTots = AttribTotals.size();
			int MinEntropy = 0x3fffffff;
			int BestEntropyBelow = -1;
			int BestEntropyAbove = -1;
			int BestClassBelow = -1;
			int BestClassAbove = -1;
			int BestCand = -1;
			int BestCountAbove = 0;
			int BestCountBelow = 0;
			int BestTotalCountAbove = 0;
			int BestTotalCountBelow = 0;

			for (int iSearch = 0; iSearch < (int)(SearchPairs.size()); iSearch++) {
				int Entropy = 0;
				int EntropyBelow = 0;
				int EntropyAbove = 0;
				int ClassBelow = -1;
				int ClassAbove = -1;
				int CountAbove = 0;
				int CountBelow = 0;
				int TotalCountAbove = 0;
				int TotalCountBelow = 0;
				for (int iBelow = 1; iBelow > -1; iBelow--) {
					int NumInSet = 0;
					int Tot = 0;
					list<int>::reverse_iterator rilStore = ril;
					for (int iClass = 0; iClass < (int)(ClassVals.size()); iClass++, iAdd++, ril++) {
						NumInSet += *ril;
					}
					ril = rilStore;
					int LogOfSet = LogLookup(NumInSet);
					int MaxClassCount = 0;
					int MaxClassID = -1;
					for (int iClass = 0; iClass < (int)(ClassVals.size()); iClass++, iAdd++, ril++) {
						int count = *ril;
						if (count != 0) {
							Tot -= count * (LogLookup(count) - LogOfSet);
						}
						if (count > MaxClassCount) {
							MaxClassCount = count;
							MaxClassID = iClass;
						}
					}
					if (iBelow == 0) {
						EntropyAbove = Tot;
						ClassAbove = MaxClassID;
						CountAbove = MaxClassCount;
						TotalCountAbove = NumInSet;

					}
					else {
						EntropyBelow = Tot;
						ClassBelow = MaxClassID;
						CountBelow = MaxClassCount;
						TotalCountBelow = NumInSet;
					}
				} // end above/below loop
				Entropy = EntropyAbove + EntropyBelow;
				if (Entropy < MinEntropy) {
					MinEntropy = Entropy;
					BestCand = iSearch;
					BestEntropyBelow = EntropyBelow;
					BestEntropyAbove = EntropyAbove;
					BestClassBelow = ClassBelow;
					BestClassAbove = ClassAbove;
					BestCountAbove = CountAbove;
					BestCountBelow = CountBelow;
					BestTotalCountAbove = TotalCountAbove;
					BestTotalCountBelow = TotalCountBelow;
				}

			} // end loop over search pairs
			int iAttr = SearchPairs[BestCand].first;
			int iDeriv = SearchPairs[BestCand].second;
			CurrNode->iSrcAttrib = iAttr;
			CurrNode->iDerivAttrib = iDeriv;
			CurrNode->Entropy = MinEntropy;
			CurrNode->LeftNode = new TreeNode();
			CurrNode->RightNode = new TreeNode();
			CurrNode->LeftNode->Entropy = BestEntropyBelow;
			CurrNode->RightNode->Entropy = BestEntropyAbove;
			CurrNode->LeftNode->Count = BestCountBelow;
			CurrNode->RightNode->Count = BestCountAbove;
			CurrNode->LeftNode->NumInSet = BestTotalCountBelow;
			CurrNode->RightNode->NumInSet = BestTotalCountAbove;
			NewSetToTreeNode[iSet * 2] = CurrNode->LeftNode;
			NewSetToTreeNode[(iSet * 2) + 1] = CurrNode->RightNode;
			if ((BestEntropyBelow == 0) || (iLevel == (cNumLevels - 1))) {
				CurrNode->LeftNode->bLeaf = true;
				CurrNode->LeftNode->ClassID = BestClassBelow;
			}
			if ((BestEntropyAbove == 0) || (iLevel == (cNumLevels - 1))) {
				CurrNode->RightNode->bLeaf = true;
				CurrNode->RightNode->ClassID = BestClassAbove;
			}

			for (int iRec = 0; iRec < cMiniBatchSize; iRec++) {
				if (SetLoc[iRec] == iSet) {
					if ((TestRecords[iRec][iAttr] >= 0)
						&& (TestRecords[iRec][iAttr] <= iDeriv)) {
						NewSetLoc[iRec] = (iSet * 2);
					}
					else {
						NewSetLoc[iRec] = (iSet * 2) + 1;
					}
				}
			}
		} // end loop over sets
		SetLoc = NewSetLoc;
		SetToTreeNode.clear();
		SetToTreeNode.resize(NumSets * 2, NULL);
		SetToTreeNode = NewSetToTreeNode;

	} // end loop over levels
}

#endif // NOT_YET