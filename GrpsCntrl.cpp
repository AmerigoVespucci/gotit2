// GrpsCntrl.cpp : Access, Iterate, Save, Load etc for groups
//

#include "stdafx.h"

#include "MascReader.h"

CPatGrpWrapper::CPatGrpWrapper(CPatGrpIter& Iter)
{
	pGrp = Iter.GetHolder();
	pGrp->bReadyForRemoval = false;
}

CPatGrpWrapper::CPatGrpWrapper(CPatGrpHolder * apGrp)
{
	pGrp = apGrp;
	// NULL is a valid argument so bail out
	if (pGrp == NULL) {
		return;
	}
	pGrp->bReadyForRemoval = false;
}

CPatGrpWrapper::~CPatGrpWrapper()
{
	if (pGrp == NULL) {
		// no real object created
		return;
	}
	pGrp->bReadyForRemoval = true;
	bool bUnload = true;
	if ((pGrp->pMgr != NULL) && (!pGrp->pMgr->TrueUnload(pGrp))) { // extra check for unlinked grps
		bUnload = false;
	}
	if (bUnload) {
		pGrp->UnloadData();
	}
}

CPatGrpHolder* CPatGrpWrapper::PresentHolder()
{
	return pGrp;
}

bool CPatGrpHolder::SaveData(fstream& f, int& NumBytesWritten)
{
#ifdef OLD_CODE
	if (!bModified) {
		return true;
	}
#endif // OLD_CODE

	if (!bDataLoaded || pData == NULL) {
		return false;
	}

	NumEls = pData->MemberLinks.size();
	if (pData->SortedByStrength.size() != (uint)NumEls) {
		cerr << "Error. Instructed to save data with Sorted members out of date\n";
		return false;
	}

	if (NumEls <= 0) {
		bModified = false;
		return false;
	}

#ifdef OLD_CODE
	string FullPath = CPatGrpMgr::GetGrpsRootPath();
	FullPath += FileID;

	fstream f;

	cout << "Data file modified, therefore writing to disk for " << PatGrpRec.Name << endl;

	//if (!TestFileExists(FullPath)) {
	//	pMgr->CreateFileForGrp(FileID, iFileID);
	//}
	if (SafeOpen(f, FullPath, ios::binary | ios::out)) {
#endif // OLD_CODE
		DataFilePos = NumBytesWritten;
		ofstream& fout = *reinterpret_cast<ofstream*>(&f);
		StringStore(PatGrpRec.Name, fout, NumBytesWritten);
		SaveData(f, pData, NumBytesWritten);
		bModified = false;
		return true;
#ifdef OLD_CODE // connects to above 
	}

	return false;
#endif // OLD_CODE

}

bool CPatGrpHolder::SaveData(fstream& f, CPatGrpData * pData, int& NumBytesWritten)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	uint NumElsToStore = pData->SortedByStrength.size();
	//fout.write((char *)&NumElsToStore, sizeof(NumElsToStore));
	FWRITE(fout, NumElsToStore, NumBytesWritten);
	//MapMemberToGroup::iterator img = pData->MemberLinks.begin();
	for (uint img = 0; img < NumElsToStore; img++) {
		pData->SortedByStrength[img]->Store(fout, NumBytesWritten);
	}
	uint NumStrongEls = pData->StrongMembers.size();
	//fout.write((char *)&NumStrongEls, sizeof(NumStrongEls));
	FWRITE(fout, NumStrongEls, NumBytesWritten);
	//MapMemberToGroup::iterator img = pData->MemberLinks.begin();
	for (uint img = 0; img < NumStrongEls; img++) {
		pData->StrongMembers[img]->Store(fout, NumBytesWritten);
	}
	return true;
}

bool CPatGrpHolder::LoadData()
{
	bReadyForRemoval = false;
	pMgr->DataAlivePing(this);

	if (bDataLoaded) {
		return true;
	}

	pData = new CPatGrpData(this);
	bDataLoaded = true;

	if (pMgr == NULL) {
		return false;
	}
	bool bGrpDataAvail = false;
	fstream& f = pMgr->getFStreamForGrp(bGrpDataAvail);
	if (!bGrpDataAvail) {
		return false;
	}
	if (NumEls <= 0) {
		bModified = false;
		return true;
	}
	f.seekg(DataFilePos, ios::beg);
	string GrpName;
	StringLoad(GrpName, f);
	if (GrpName != PatGrpRec.Name) {
		cerr << "Error. File is not pointing to correct group data\n";
		cerr << "Should be " << PatGrpRec.Name << " but received " << GrpName << endl;
		return false;
	}
	LoadData(f, pData);

	bModified = false;
	return true;
}

/*
bool CPatGrpHolder::LoadGrpData(fstream& f)
{
	CPatGrpData * pData = new CPatGrpData();
	string GrpName;
	StringLoad(GrpName, f);
	
}
*/

bool CPatGrpHolder::LoadData(fstream& f, CPatGrpData * pData)
{

#ifdef OLD_DATA
	string FullPath = CPatGrpMgr::GetGrpsRootPath();
	FullPath += FileID;

	fstream f;
	// cout << "Data loaded from file for " << PatGrpRec.Name << endl; // out this back for debugging unloading files --------------------------
	if (SafeOpen(fileGrp, FullPath, ios::binary | ios::in)) {
#endif // OLD_CODE
		ifstream& fin = *reinterpret_cast<ifstream*>(&f);
		uint NumElsStored;
		fin.read((char *)&NumElsStored, sizeof(NumElsStored));
		for (uint img = 0; img < NumElsStored; img++) {
			SGroupingLink El;
			El.Load(fin);
			pData->MemberLinks[El.W] = El; // full copy here
			pData->SortedByStrength.push_back(&(pData->MemberLinks[El.W]));
		}
		uint NumStrongEls;
		fin.read((char *)&NumStrongEls, sizeof(NumStrongEls));
		for (uint img = 0; img < NumStrongEls; img++) {
			SGroupingLink El;
			El.Load(fin);
			//pData->MemberLinks[El.W] = El; // full copy here
			pData->StrongMembers.push_back(&(pData->MemberLinks[El.W]));
		}

		//pData->CreateStrengthSort(pMgr->pEnv);

#ifdef OLD_CODE
		bModified = false;
		return true;
	}

	return false;
#else // OLD_CODE
		return true;
#endif // OLD_CODE
}

bool CPatGrpHolder::UnloadData()
{
	if (bDataLoaded && bReadyForRemoval) {
		if (bModified) {
			if ((pMgr != NULL) && !(pMgr->bDestructorCalled)) {
				cerr << "Error. Must not delete modified data\n";
			}
			return false;
		}
		if ((pMgr != NULL) && !(pMgr->bDestructorCalled)) {
			//cout << "Data unloaded for " << PatGrpRec.Name << endl;
		}
		NumEls = pData->MemberLinks.size();
		delete pData;
		pData = NULL;
		bDataLoaded = false;
		return true;
	}
	return false;

}

CPatGrpHolder * CPatGrpIter::GetHolder()
{
	return &(itpgt->second);
}

// unsafe:
//bool CPatGrpWrapper::getGrpParams(vector<string>& Params)
//{
//	Params = pGrp->PatGrpRec.members;
//	return true;
//}

int CPatGrpWrapper::getNumGrpParams()
{
	return pGrp->PatGrpRec.members.size();
}

void CPatGrpWrapper::setNumGrpParams(int Size)
{
	pGrp->bModified = true;
	pGrp->PatGrpRec.members.resize(Size);
}

bool CPatGrpWrapper::setGrpParam(int i, string& val)
{
	if ((uint)i >= pGrp->PatGrpRec.members.size()) {
		return false;
	}
	pGrp->bModified = true;
	pGrp->PatGrpRec.members[i] = val;
	return true;
}

bool CPatGrpWrapper::getGrpParam(int i, string& val)
{
	if ((uint)i >= pGrp->PatGrpRec.members.size()) {
		return false;
	}
	val = pGrp->PatGrpRec.members[i];
	return true;
}

string CPatGrpWrapper::getGrpParam(int i)
{
	if ((uint)i >= pGrp->PatGrpRec.members.size()) {
		return "";
	}
	return pGrp->PatGrpRec.members[i];
}

void CPatGrpWrapper::addGrpParam(string& val)
{
	pGrp->bModified = true;
	pGrp->PatGrpRec.members.push_back(val);
}


bool CPatGrpWrapper::getGrpFinder(SPatternFinder*& pFinder)
{
	pFinder = pGrp->PatGrpRec.pfinder;
	return true;
}

bool CPatGrpWrapper::getGrpName(string& Name)
{
	Name = pGrp->PatGrpRec.Name;
	return true;
}

// return a string not a ref in case the obj is deleted
string CPatGrpWrapper::getGrpName()
{
	return pGrp->PatGrpRec.Name;
}

void CPatGrpWrapper::setGrpName(string& Name)
{
	pGrp->PatGrpRec.Name = Name;
}

int CPatGrpWrapper::getTotalNumOccurs()
{
	return pGrp->TotalNumOccurs;
}

string& CPatGrpWrapper::getSrprsGrpName()
{
	return pGrp->SurpriseGrpName;
}

void CPatGrpWrapper::setFinder(SPatternFinder* pFinder)
{
	pGrp->PatGrpRec.pfinder = pFinder;
}

void CPatGrpWrapper::setNumSearches(int NumSearches)
{
	pGrp->PatGrpRec.NumSearches = NumSearches;
}

void CPatGrpWrapper::setNumRecsSearched(int NumRecsSearched)
{
	pGrp->PatGrpRec.NumRecsSearched = NumRecsSearched;
}

bool CPatGrpWrapper::AddEl(string& ElName, SGroupingLink& El)
{
	if (pGrp->LoadData()) {
		pGrp->pData->MemberLinks[ElName] = El;
		pGrp->bModified = true;
		return true;
	}
	return false;
}

// unsafe
//bool CPatGrpWrapper::getGrpElsMap(MapMemberToGroup& ElsMap)
//{
//	if (pGrp->LoadData()) {
//		ElsMap = pGrp->pData->MemberLinks;
//		return true;
//	}
//	return false;
//}

int CPatGrpWrapper::getGrpElsMapSize()
{
	if (pGrp->LoadData()) {
		return pGrp->pData->MemberLinks.size();
	}
	return -1;
}

bool CPatGrpWrapper::addGrpEl(string& ElName, int& NumOccurs)
{
	NumOccurs = -1;
	pGrp->LoadData(); // if it didn't work we're starting fresh but fon't fail on this
	//if (!pGrp->bModified) {
	//	cout << "bModified set because addGrpEl called for " << getGrpName() << endl;
	//}
	pGrp->bModified = true;
	SGroupingLink InitLink;
	MapMemberToGroup::iterator iFound = pGrp->pData->MemberLinks.find(ElName);
	if (iFound == pGrp->pData->MemberLinks.end()) {
		pGrp->pData->MemberLinks[ElName] = InitLink;
		NumOccurs = 1;
	}
	else {
		iFound->second.NumOccurs++;
		NumOccurs = iFound->second.NumOccurs;
	}
	return true;
}

bool CPatGrpWrapper::incrGrpElNumExists(string& ElName)
{
	bool bReport = false;
	if (pGrp->LoadData()) {
		if (!pGrp->bModified) {
			cout << "modified beccause incrGrpElNumExists for " << getGrpName() << " ElName \"" << ElName << "\" ";
			bReport = true;
		}
		pGrp->bModified = true;
		MapMemberToGroup::iterator iFound = pGrp->pData->MemberLinks.find(ElName);
		if (iFound != pGrp->pData->MemberLinks.end()) {
			iFound->second.NumExist++;
			if (bReport) {
				cout << " to " << iFound->second.NumExist << endl;
			}
		}
		else {
			if (bReport) {
				cout << "strange that not found\n";
			}
		}
		return true;
	}
	return false;
}

//bool CPatGrpWrapper::getGrpStrongEls(vector<SGroupingLink*>& StrongEls)
//{
//	if (pGrp->LoadData()) {
//		StrongEls = pGrp->pData->StrongMembers;
//		return true;
//	}
//	return false;
//}
//
int CPatGrpWrapper::getNumStrongEls()
{
	if (pGrp->LoadData()) {
		return pGrp->pData->StrongMembers.size();
	}
	return -1;
}

SGroupingLink* CPatGrpWrapper::getGrpElByName(string& W)
{
	if (pGrp->LoadData()) {
		MapMemberToGroup::iterator itml = pGrp->pData->MemberLinks.find(W);
		if (itml != pGrp->pData->MemberLinks.end()) {
			return &(itml->second);
		}
	}
	return NULL;
}

SGroupingLink* CPatGrpWrapper::getStrongElByIndex(int iStrongest) // only get from shorted strong list
{
	if (pGrp->LoadData()) {
		return pGrp->pData->StrongMembers[iStrongest];
	}
	return NULL;
}


SGroupingLink* CPatGrpWrapper::getGrpElByIndex(int iStrongest) // get all ordered by strength
{
	if (pGrp->LoadData()) {
		return pGrp->pData->SortedByStrength[iStrongest];
	}
	return NULL;
}


bool CPatGrpWrapper::getGrpElsSortedByStrength(vector<SGroupingLink*>& ElsSortedByStrength)
{
	if (pGrp->LoadData()) {
		ElsSortedByStrength = pGrp->pData->SortedByStrength;
		return true;
	}
	return false;
}

bool CPatGrpWrapper::getGrpTotalStrength(float& Strength)
{
	if (pGrp->LoadData()) {
		Strength = pGrp->TotalStrength;
		return true;
	}
	return false;
}

bool CPatGrpWrapper::CreateStrengthSort()
{
	if (pGrp->bModified) {
		if (pGrp->LoadData()) {
			//pGrp->bModified = true;
			pGrp->pData->CreateStrengthSort(pGrp->pMgr->pEnv);
			return true;
		}
		return false;
	}
	return true;
}

int CPatGrpWrapper::getGrpID()
{
	return pGrp->iFileID;
}

void CPatGrpData::CreateStrengthSort(CGotitEnv * pEnv)
{
	MapMemberToGroup::iterator ig = MemberLinks.begin();
	SortedByStrength.clear();
	StrongMembers.clear();
	//const int c_sample_size_cutoff = 20; // larger than this we consider the sample size sufficient
	const int cStrongMemberThreshold = 3;
	pGrp->TotalStrength = 0.0f;
	pGrp->TotalNumOccurs = 0;
	for (; ig != MemberLinks.end(); ig++) {
		string W = ig->first;
		ig->second.PctHit = 1.0f; //  (float)(ig->second.NumOccurs) * 100.0f / (float)(ig->second.NumExist);
		//float SSize = (float)min(ig->second.NumOccurs, c_sample_size_cutoff)
		//	/ (float)c_sample_size_cutoff;
		ig->second.Strength = (float)(ig->second.NumOccurs); // ig->second.PctHit * SSize / 100.0f;
		pGrp->TotalStrength += ig->second.Strength;
		ig->second.W = W;
		SortedByStrength.push_back(&(ig->second));
		if (ig->second.NumOccurs > cStrongMemberThreshold) {
			StrongMembers.push_back(&(ig->second));
			ig->second.bStrong = true; // currently not reset except at initial addition
		}
		pGrp->TotalNumOccurs += ig->second.NumOccurs;
		ig->second.SurprisingNess = pEnv->FindGrpElSurprisingness(W, pGrp);
	}
	ig = MemberLinks.begin();;
	float OccursRatioSum = 0.0;
	int NumRatios = 0;
	for (uint iStrong = 0; iStrong < SortedByStrength.size(); iStrong++) {
		SGroupingLink * pEl = SortedByStrength[iStrong];
		int HitCount = pGrp->pMgr->pEnv->getWordHitCount(pEl->W);
		if (HitCount) {
			OccursRatioSum += CalcOPC(pEl->NumOccurs, HitCount);
			NumRatios++; 
		}

	}
	pGrp->NumStrongEls = StrongMembers.size(); 
	if (NumRatios > 0) {
		pGrp->AvgOccursPerCount = OccursRatioSum / (float)NumRatios;
	}
	else {
		pGrp->AvgOccursPerCount = -1.0f;
	}
	std::sort(SortedByStrength.begin(), SortedByStrength.end(), SGroupingLink::gt);
	pGrp->NumEls = SortedByStrength.size();
	std::sort(StrongMembers.begin(), StrongMembers.end(), SGroupingLink::gtInOccurs);
}

// This function goes through other words with similar frequency and sees what
// their NumOccurs is relative to the source word. The idea being that if
// there are many other words with similar frequency that have much higher
// NumOccurs, then the source word is in the group probably as a fluke
// Some groups have a special group to compare against, called a surprise group.
// In this case the similar frequency candidates are taken only from the surprise
// group.
float CGotitEnv::FindGrpElSurprisingness(string& W, CPatGrpHolder * pPatGrp)
{
	map<string, int>::iterator itmwci = MapWordToCountIndex.find(W);
	if (itmwci == MapWordToCountIndex.end()) {
		return 0;
	}
	vector<NameItem>& WordListSorted = BasicTypeLists["word"];
	int iW = itmwci->second;
	//static const int cCountFactor = 2;
	int WCount = WordListSorted[iW].Count;
	int CountMax = WCount * 2;
	int CountMin = WCount / 2;

	//int iWStart = min(0, iW - (cNumElsForSurprise / 2));
	//int iWEnd = max((int)WordListSorted.size(), iW + (cNumElsForSurprise / 2));
	//bool bUseSrprsGrp = false;
	CPatGrpWrapper PatGrp(pPatGrp);
	CPatGrpHolder * phPatGrpSrprs = NULL;
	string& SrprsGrpName = PatGrp.getSrprsGrpName();
	if (SrprsGrpName.size() > 0) {
		phPatGrpSrprs = PatGrpMgr.FindGrp(SrprsGrpName);
	}
	CPatGrpWrapper SrprsPatGrp(phPatGrpSrprs); // carefull when creating wrapper using NULL. Mustn't call member fns
	int TotalOccurs = 0;
	int ThisOccurs = 0;
	for (int iBoth = 0; iBoth < 2; iBoth++) {
		int Incr = -1;
		if (iBoth == 1) {
			Incr = +1;
		}
		int iIter = 0;
		for (	uint iwl = iW + iBoth;	// on the second go, we want to start at 1 above iW
					iwl >= 0 
				&&	iwl < WordListSorted.size() 
				&&	iIter < cNumElsForSurprise; 
				iwl += Incr) {
			if (phPatGrpSrprs != NULL) {
				SGroupingLink* pElInSrprs
					= SrprsPatGrp.getGrpElByName(WordListSorted[iwl].Name);
				if (pElInSrprs == NULL) {
					continue;
				}
			}
			iIter++;
			// This piece of code stops the loop in the current direction if the
			// frequency of the other word considered is very different from the
			// frequency of the source word
			int OthersCount = WordListSorted[iwl].Count;
			if (Incr < 0) {
				if (OthersCount > CountMax) {
					break;
				}
			}
			else {
				if (OthersCount < CountMin) {
					break;
				}
			}
			SGroupingLink* pEl = PatGrp.getGrpElByName(WordListSorted[iwl].Name);
			if (pEl != NULL) {
				TotalOccurs += pEl->NumOccurs;
				if (iwl == (uint)iW) {
					ThisOccurs = pEl->NumOccurs;
				}
			} // end if pEl not NULL
		}
	}
	return ((TotalOccurs > 0) ? ((float)ThisOccurs / (float)TotalOccurs) : 0.0f);
}

void CGotitEnv::AddWordGrpLink(string& W, int GrpID, int NumOccurs)
{
	map<string, SWordData>::iterator itWordTbl = MapWordToData.find(W);
	//bool bJustAddedToWordTbl = false;
	if (itWordTbl == MapWordToData.end()) {
		pair<map<string, SWordData>::iterator, bool> MapPair 
			= MapWordToData.insert(pair<string, SWordData>(W, SWordData()));
		itWordTbl = MapPair.first;
		if (NumOccurs > 1) {
			cerr << "Warning. Word " << W << " just added to MapWordTbl and yet NumOccurs is already " << NumOccurs << endl;
		}
		//bJustAddedToWordTbl = true;
	}

	map<int, int>& GrpTbl = itWordTbl->second.GrpList;
	map<int, int>::iterator itGrpList = GrpTbl.find(GrpID);
	if (itGrpList == GrpTbl.end()) {
		pair<map<int, int>::iterator, bool> MapPair
			= GrpTbl.insert(pair<int, int>(GrpID, 0));
		if (NumOccurs > 1) {
			cerr << "Warning. Word " << W << " just added GrpID " << GrpID << " to MapWordTbl and yet NumOccurs is already " << NumOccurs << endl;
		}
		itGrpList = MapPair.first;
	}

	itGrpList->second = NumOccurs;


}

int CGotitEnv::getWordHitCount(string& W)
{
	map<string, int>::iterator itCountMap = MapWordToCountIndex.find(W);
	if (itCountMap == MapWordToCountIndex.end()) {
		return 0;
	}

	return BasicTypeLists["word"][itCountMap->second].Count;

	//return itCountMap->second;
}



void CPatGrpHolder::Store(fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	VersionNum = 1;
	fout.write((char *)&VersionNum, sizeof(VersionNum));
	StringStore(FileID, fout);
	if (bDataLoaded) {
		NumEls = pData->MemberLinks.size();
	}
	fout.write((char *)&iFileID, sizeof(iFileID));
	fout.write((char *)&NumEls, sizeof(NumEls));
	fout.write((char *)&NumStrongEls, sizeof(NumStrongEls));
	fout.write((char *)&TotalNumOccurs, sizeof(TotalNumOccurs));
	fout.write((char *)&TotalStrength, sizeof(TotalStrength));
	fout.write((char *)&AvgOccursPerCount, sizeof(AvgOccursPerCount));
	fout.write((char *)&DataFilePos, sizeof(DataFilePos));
	fout.write((char *)&FHolder1, sizeof(FHolder1));
	fout.write((char *)&FHolder2, sizeof(FHolder2));
	fout.write((char *)&FHolder3, sizeof(FHolder3));
	fout.write((char *)&IHolder1, sizeof(IHolder1));
	fout.write((char *)&IHolder2, sizeof(IHolder2));
	fout.write((char *)&IHolder3, sizeof(IHolder3));
	StringStore(SurpriseGrpName, fout);
	PatGrpRec.StoreNonData(f);
	//if (bDataLoaded && bModified & (pData->MemberLinks.size() > 0)) {
	if (bDataLoaded && bModified) {
		if (NumEls > 0) {
			//SaveData(); // No longer saving data individually.
		}
		else {
			delete pData;
			pData = NULL;
			bDataLoaded = false;
		}
	}
	bModified = false;
	//if (bDataLoaded) {
	//	UnloadData();
	//}
}

CPatGrpHolder::~CPatGrpHolder()
{
	if (pMgr == NULL) { // this indicates that we are in an unlinked pat grp
		return; // even if the pData pointer is valid, it has been copied and will be dealt with. Make sure you do!!
	}
	//if (bDataLoaded && bModified & (pData->MemberLinks.size() > 0)) {
	//	SaveData(); //Deprecated. No longer storing data individually.
	//}
	UnloadData();
}

void CGroupingRec::StoreNonData(fstream& f)
{
	ofstream& fout = *reinterpret_cast<ofstream*>(&f);
	//StringStore(Name, fout); // moved up two levels
	fout.write((char *)&total_strength, sizeof(total_strength));
	fout.write((char *)&NumSearches, sizeof(NumSearches));
	fout.write((char *)&NumRecsSearched, sizeof(NumRecsSearched));
	fout.write((char *)&bFromMerge, sizeof(bFromMerge));
	//StringStore(pfinder->group_name_template, fout);

	uint NumNodesInFinder = members.size();
	fout.write((char *)&NumNodesInFinder, sizeof(NumNodesInFinder));
	for (uint im = 0; im < NumNodesInFinder; im++) {
		StringStore(members[im], fout);
	}

	// add the following to Load below
	StringStore(GrpType, fout);
}

void CPatGrpHolder::Load(fstream& f)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	fin.read((char *)&VersionNum, sizeof(VersionNum));
	StringLoad(FileID, fin);
	fin.read((char *)&iFileID, sizeof(iFileID));
	fin.read((char *)&NumEls, sizeof(NumEls));
	fin.read((char *)&NumStrongEls, sizeof(NumStrongEls));
	fin.read((char *)&TotalNumOccurs, sizeof(TotalNumOccurs));
	fin.read((char *)&TotalStrength, sizeof(TotalStrength));
	fin.read((char *)&AvgOccursPerCount, sizeof(AvgOccursPerCount));
	fin.read((char *)&DataFilePos, sizeof(DataFilePos));
	fin.read((char *)&FHolder1, sizeof(FHolder1));
	fin.read((char *)&FHolder2, sizeof(FHolder2));
	fin.read((char *)&FHolder3, sizeof(FHolder3));
	fin.read((char *)&IHolder1, sizeof(IHolder1));
	fin.read((char *)&IHolder2, sizeof(IHolder2));
	fin.read((char *)&IHolder3, sizeof(IHolder3));
	StringLoad(SurpriseGrpName, fin);
	PatGrpRec.LoadNonData(f);
}

void CGroupingRec::LoadNonData(fstream& f)
{
	ifstream& fin = *reinterpret_cast<ifstream*>(&f);
	//StringLoad(Name, fin);
	fin.read((char *)&total_strength, sizeof(total_strength));
	fin.read((char *)&NumSearches, sizeof(NumSearches));
	fin.read((char *)&NumRecsSearched, sizeof(NumRecsSearched));
	fin.read((char *)&bFromMerge, sizeof(bFromMerge));
	//string FinderName;
	//StringLoad(FinderName, fin);
//#pragma message("put this code in the calling function")
#ifdef OLD_CODE
	for (uint ia = 0; ia < anc_pat_finder_list.size(); ia++) {
		if (FinderName == anc_pat_finder_list[ia].group_name_template) {
			pfinder = &(anc_pat_finder_list[ia]);
			map_name_to_pat_grp::iterator itGrpTbl = pat_grp_tbl.find(Name);
			if (itGrpTbl == pat_grp_tbl.end()) {
				pat_grp_tbl[Name] = this;
				pfinder->ChildGroups.push_back(this);
			}
			else {
				cerr << "Why are we loading a group again?\n";
				//				patgrp = itGrpTbl->second;
			}
			break;
		}
	}
#endif // OLD_CODE

	uint NumNodersInFinder;
	fin.read((char *)&NumNodersInFinder, sizeof(NumNodersInFinder));
	for (uint im = 0; im < NumNodersInFinder; im++) {
		string MemberName;
		StringLoad(MemberName, fin);
		members.push_back(MemberName);
	}


	StringLoad(GrpType, fin);
}


CPatGrpIter CPatGrpMgr::getStartIterator()
{
	return CPatGrpIter(PatGrpTbl.begin());
}

CPatGrpHolder * CPatGrpMgr::FindGrp(string& GrpName)
{
	map<string, CPatGrpHolder>::iterator itpgt = PatGrpTbl.find(GrpName);
	if (itpgt == PatGrpTbl.end()) {
		return NULL;
	}
	
	return &(itpgt->second);
}

bool CPatGrpMgr::getNextIterator(CPatGrpIter& CurIter)
{
	CurIter.itpgt++;
	if (CurIter.itpgt == PatGrpTbl.end()) {
		return false;
	}
	return true;
	//return CPatGrpIter(CurIter.itpgt);
}

bool CPatGrpMgr::IsIteratorValid(CPatGrpIter& CurIter)
{
	if (CurIter.itpgt == PatGrpTbl.end()) {
		return false;
	}
	return true;
}

bool CPatGrpMgr::CreateFileIDForGrp(string& FileID, int& iFileID)
{
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#endif
	string RootPath = CPatGrpMgr::GetGrpsRootPath();
	string IDSoFar = "";

	//for (int i = 0; i < cNumGrpDirLevels; i++) {
	//	int r = rand() % 256;
	//	IDSoFar += "\\" + to_string(r);
	//	path ThePath = RootPath + IDSoFar;
	//	if (!is_directory(ThePath)) {
	//		create_directory(ThePath);
	//	}
	//}

	iFileID = GrpMaxFileID;
	FileID = IDSoFar + "\\" + to_string(iFileID) + ".gdt";
	GrpMaxFileID++;
	return true;
}


bool CPatGrpMgr::CreateFileForGrp(string& FileID, int iFileID)
{
	//iFileID = GrpMaxFileID;
	//GrpMaxFileID++;
	return true;
}

CPatGrpHolder * CPatGrpMgr::CreateUnlinkedPatGrp(string& GrpName, SPatternFinder* pFinder,
										const vector<string>& GrpParams, const string& SrprsGrpName)
{
	// Don't call this function unless you know that the group does not exist, linked, in the table 
	CPatGrpHolder * phPatGrp = new CPatGrpHolder();

	//PatGrpTbl[GrpName] = CPatGrpHolder();
	//CPatGrpHolder& PatGrp = PatGrpTbl[GrpName];
	//CreateFileIDForGrp(PatGrp.FileID, PatGrp.iFileID);
	//PatGrp.FileID = FileID;
	phPatGrp->PatGrpRec.Name = GrpName;
	phPatGrp->PatGrpRec.GrpType = pFinder->GrpType;
	phPatGrp->PatGrpRec.pfinder = pFinder;
	phPatGrp->PatGrpRec.total_strength = 0.0;
	phPatGrp->PatGrpRec.members = GrpParams;
	//pFinder->ChildPatGrps.push_back(&PatGrp);
	phPatGrp->bModified = true;
	phPatGrp->SurpriseGrpName = SrprsGrpName;
	phPatGrp->pMgr = NULL;
	//cout << "Created unlinked group called " << GrpName << endl;
	return phPatGrp;
}

void CPatGrpMgr::CreatePatGrp(	string& GrpName, SPatternFinder* pFinder,
								const vector<string>& GrpParams, const string& SrprsGrpName)
{
	map<string, CPatGrpHolder>::iterator ipg = PatGrpTbl.find(GrpName);
	if (ipg != PatGrpTbl.end()) {
		return; // already created
	}
	else {
		CPatGrpHolder * phPatGrp 
			= CreateUnlinkedPatGrp(GrpName, pFinder, GrpParams, SrprsGrpName);
		LinkPatGrp(phPatGrp);
		cout << "Created new group called " << GrpName << endl;
	}
}

CPatGrpHolder * CPatGrpMgr::LinkPatGrp(CPatGrpHolder * phPatGrp)
{
	string GrpName = phPatGrp->PatGrpRec.Name;
	PatGrpTbl[GrpName] = *phPatGrp;
	delete phPatGrp;
	CPatGrpHolder& PatGrp = PatGrpTbl[GrpName];
	CreateFileIDForGrp(PatGrp.FileID, PatGrp.iFileID);
	PatGrp.PatGrpRec.pfinder->ChildPatGrps.push_back(&PatGrp);
	PatGrpIDTbl[PatGrp.iFileID] = &PatGrp;
	PatGrp.pMgr = this;
	return &PatGrp;
}

void CPatGrpMgr::RemoveGrpWithoutSave(CPatGrpHolder * phPatGrp)
{
	phPatGrp->bModified = false;
	phPatGrp->UnloadData();
	delete phPatGrp;
}



bool CPatGrpMgr::TrueUnload(CPatGrpHolder * pHolder)
{
	// replace this with an LRU. Need a bReadyToUnload flag in the Wrapper class so that we don't delete data while in use
	return false;
}

void CPatGrpMgr::DeletePatGrp(CPatGrpHolder * pHolder)
{
	map<string, SWordData>& WordTbl = pEnv->getWordTbl();
	{
		CPatGrpWrapper PatGrp(pHolder);
		int NumEls = PatGrp.getGrpElsMapSize();
		for (int iEl = 0; iEl < NumEls; iEl++) {
			SGroupingLink* pEl = PatGrp.getGrpElByIndex(iEl);
			map<string, SWordData>::iterator itWordTbl = WordTbl.find(pEl->W);
			if (itWordTbl != WordTbl.end()) {
				map<int, int>::iterator itGrp
					= itWordTbl->second.GrpList.find(PatGrp.getGrpID());
				if (itGrp != itWordTbl->second.GrpList.end()) {
					itWordTbl->second.GrpList.erase(itGrp);
					if (itWordTbl->second.GrpList.size() == 0) {
						WordTbl.erase(itWordTbl);
					}
				}
			}
		}
	} // end scope for PatGrp

	if (pHolder->bDataLoaded) {
		pHolder->bReadyForRemoval = true;
		pHolder->UnloadData();
	}
	string FullPath = GetGrpsRootPath();
	FullPath += pHolder->FileID;
#ifndef GOTIT_LINUX
	using namespace std::tr2::sys;
#else
	using namespace boost::filesystem;
#endif
	path ThePath = FullPath;
	if (exists(ThePath)) {
		remove(ThePath);
	}

	int iFileID = pHolder->iFileID;
	map<int, CPatGrpHolder*>::iterator itIDTbl = PatGrpIDTbl.find(iFileID);
	if (itIDTbl != PatGrpIDTbl.end()) {
		PatGrpIDTbl.erase(itIDTbl);
	}
	PatGrpTbl.erase(pHolder->PatGrpRec.Name);
}

void CPatGrpMgr::CleanPatGrps()
{
	cerr << "Error. Function call deprecated\n";
#ifdef OLD_CODE
	if (!bInitialLoadDone) {
		return;
	}
	u64 TimeNow = TimerNow();
	cout << "Cleaning called at " << TimeNow << endl;
	uint NumLoaded = 0;
	map<string, CPatGrpHolder>::iterator itpgt = PatGrpTbl.begin();
	for (; itpgt != PatGrpTbl.end(); itpgt++) {
		if (itpgt->second.bDataLoaded) {
			if (itpgt->second.bReadyForRemoval) {
				u64 LastAlive = itpgt->second.DataLastKeepAlive;
				if (	(TimeNow < LastAlive) // in case of wrap-around
					|| ((TimeNow - LastAlive) > (cTimeLapsForDataRemove + CleaningTimeExtra))) {
					cout << "Cleaning: Last ping was at " << LastAlive << ". Cleaning \n";
					if (itpgt->second.bModified) {
						itpgt->second.SaveData();
						itpgt->second.bModified = false;
					}
					itpgt->second.UnloadData();
				}
			}
			else {
				NumLoaded++;
			}
		}
	}
	if (NumLoaded > 1000) {
		cerr << "Warning: Number of loaded groups has grown to " << NumLoaded << endl;
	}
	//Store();

	cout << "Cleaning completed at " << TimeNow << endl;
	CleaningTimeExtra = TimerNow() - TimeNow; // don't add, just set a new value
#endif // OLD_CODE

}

void CPatGrpMgr::DataAlivePing(CPatGrpHolder * pHolder)
{
	pHolder->DataLastKeepAlive = TimerNow();
#ifdef OLD_CODE
	//cout << "Received keep alive for " << pHolder->PatGrpRec.Name << " .\n";
	list<CPatGrpHolder *>::iterator itlru = LRUForDataRemoval.begin();
	for (; itlru != LRUForDataRemoval.end(); itlru++) {
		if (pHolder == *itlru) {
			//cout << "Keeping " << pHolder->PatGrpRec.Name << " alive.\n";
			LRUForDataRemoval.erase(itlru);
			LRUForDataRemoval.push_front(pHolder);
			return;
		}
	}
	LRUForDataRemoval.push_front(pHolder);
	if (LRUForDataRemoval.size() > (cLRUForDataRemovalSize+3)) {
		int ToRemove = LRUForDataRemoval.size() - cLRUForDataRemovalSize;
		list<CPatGrpHolder *>::reverse_iterator ritlru = LRUForDataRemoval.rbegin();
		while (ritlru != LRUForDataRemoval.rend()) {
			cout << "Request unloading " << (*ritlru)->PatGrpRec.Name << endl;
			if ((*ritlru)->UnloadData()) {
				cout << "request accepted.\n";
				ToRemove--;
				ritlru++;
				ritlru = list<CPatGrpHolder *>::reverse_iterator(LRUForDataRemoval.erase(ritlru.base()));
				if (ToRemove == 0) {
					break;
				}
			}
			else {
				cout << "LRU unload request rejected for " << (*ritlru)->PatGrpRec.Name << ". LRU size is " << LRUForDataRemoval.size() << endl;
				ritlru++;
			}
		}

	}
#endif // OLD_CODE
}

void CPatGrpMgr::NewData(CPatGrpHolder * pHolder)
{

}


void SWordData::Store(ofstream& fout)
{
	int NumGrps = GrpList.size();
	fout.write((char *)&(NumGrps), sizeof(NumGrps));
	map<int, int>::iterator itTbl = GrpList.begin();
	for (; itTbl != GrpList.end(); itTbl++) {
		fout.write((char *)&(itTbl->first), sizeof(itTbl->first));
		fout.write((char *)&(itTbl->second), sizeof(itTbl->second));
	}
}

void SWordData::Load(ifstream& fin)
{
	int NumGrps; 
	fin.read((char *)&(NumGrps), sizeof(NumGrps));
	for (int iGrp = 0; iGrp < NumGrps; iGrp++) {
		int GrpID, NumOccurs;
		fin.read((char *)&(GrpID), sizeof(GrpID));
		fin.read((char *)&(NumOccurs), sizeof(NumOccurs));
		GrpList[GrpID] = NumOccurs;
	}
}

void CPatGrpMgr::Store()
{
	string TempFileName = GRPS_TEMP;
	fstream fileData;
	if (TestFileExists(TempFileName)) {
		if (!FileRemove(TempFileName)) {
			cerr << "Cannot store group data. Temp file cannot be removed\n";
			return;
		}
	}
	if (!SafeOpen(fileData, TempFileName , ios::binary | ios::out)) {
		return;
	}
	//bool bErrFound = false;
	int NumGrpsSaved = 0;
	int DataFilePos = 0;
	map<string, CPatGrpHolder>::iterator itpg = PatGrpTbl.begin();
	for (; itpg != PatGrpTbl.end(); itpg++) {
		CPatGrpHolder& PatGrpHolder = itpg->second;
		if (PatGrpHolder.SaveData(fileData, DataFilePos)) { // function returns true only if data is written to file
			NumGrpsSaved++;
		}
		//if (!bErrFound && (DataFilePos >= 7831768)) {
		//	cerr << "Reached bad file pos. Pos is " << DataFilePos << "\n";
		//	bErrFound = true;
		//}
	}
	//bErrFound = false;
	bool bDataWasAvail = false;
	if (bDataAvailForLoading) {
		bDataAvailForLoading = false;
		if (fileDataForLoading.is_open()) {
			fileDataForLoading.close();
			bDataWasAvail = true;
		}
	}
	string GroupsDataFileName = GRPS_DATA;
	string sLookup;
	if (pEnv->GetImplemParam(sLookup, "Implem.Param.GroupsDataFileName")) {
		GroupsDataFileName = sLookup;
	}
	u64 TimeNow = TimerNow();
	bool bDoClean = false;
	u64 CleanTime = 0LL;
	if (SaveTimes.size() > 2) {
		bDoClean = true;
		CleanTime = SaveTimes[SaveTimes.size() - 1];
	}
	//bDoClean = false;
	int DebugNumCleaned = 0;
	SaveTimes.push_back(TimeNow);
	fstream fileOldDataIn;
	if (SafeOpen(fileOldDataIn, GroupsDataFileName, ios::binary | ios::in)) {
		for (int iGrpLoaded = 0; iGrpLoaded < NumGrpsSavedInDataFile; iGrpLoaded++) {
			//if (!bErrFound && (DataFilePos >= 7831768)) {
			//	cerr << "Reached bad file pos. Pos is " << DataFilePos << " and iGrpLoaded is " << iGrpLoaded << "\n";
			//	bErrFound = true;
			//}
			CPatGrpData * pData = new CPatGrpData();
			string GrpName;
			StringLoad(GrpName, fileOldDataIn);
			if (!fileOldDataIn) {
				cerr << "Error. Data file read beyond its end. " << iGrpLoaded << " grps loaded instead of " << NumGrpsSavedInDataFile << "\n";
				break;
			}
			CPatGrpHolder * phLoadedGrp = FindGrp(GrpName);
			CPatGrpHolder::LoadData(fileOldDataIn, pData);
			if (phLoadedGrp == NULL) {
				std::cout << "Grp " << GrpName << " no longer exists in PatGrpMgr\n";
			}
			else if (!phLoadedGrp->bDataLoaded) {
				phLoadedGrp->DataFilePos = DataFilePos;
				ofstream& fout = *reinterpret_cast<ofstream*>(&fileData);
				StringStore(GrpName, fout, DataFilePos);
				CPatGrpHolder::SaveData(fileData, pData, DataFilePos); // Note saving to fileData and not fileOldDataIn
				NumGrpsSaved++;
			}
			else {
				if (bDoClean) {
					u64 LastAlive = phLoadedGrp->DataLastKeepAlive;
					if ((TimeNow < LastAlive) // in case of wrap-around
						|| (LastAlive < CleanTime)) {
						//std::cout << "Cleaning: Last ping was at " << LastAlive << " and clean time is " << CleanTime << "\n";
						phLoadedGrp->UnloadData();
						DebugNumCleaned++;
					}
				}
			}
			delete pData; // if data was already loaded and cleaned, don't worry, this is not the same data
		}
	}
	std::cout << "CPatGrpMgr::Store: stored " << NumGrpsSaved << " groups. Cleaned " << DebugNumCleaned << " groups\n";
	if (fileOldDataIn.is_open()) {
		fileOldDataIn.close();
	}
	fileData.close();
#ifdef TEST_GRPS_DATA_FILE
	{
		fstream fileOldDataIn;
		if (SafeOpen(fileOldDataIn, TempFileName, ios::binary | ios::in)) {
			for (int iGrpLoaded = 0; iGrpLoaded < NumGrpsSavedInDataFile; iGrpLoaded++) {
				streampos Pos = fileOldDataIn.tellg();
				CPatGrpData * pData = new CPatGrpData();
				string GrpName;
				StringLoad(GrpName, fileOldDataIn);
				if (!fileOldDataIn) {
					break;
				}
				CPatGrpHolder * phLoadedGrp = FindGrp(GrpName);
				if (phLoadedGrp == NULL) {
					cerr << "Badly written file. No Group. Pos is " << Pos << "\n";
					throw new std::exception("No Group");
					return;
				}
				if (phLoadedGrp->DataFilePos != Pos) {
					cerr << "Badly written file\n";
					return;
				}
				CPatGrpHolder::LoadData(fileOldDataIn, pData);
				delete pData;
			}
		}
	}
#endif // TEST_GRPS_DATA_FILE
	NumGrpsSavedInDataFile = NumGrpsSaved;

	//itpg = PatGrpTbl.begin();
	//for (; itpg != PatGrpTbl.end(); itpg++) {
	//	CPatGrpHolder& PatGrpHolder = itpg->second;
	//	PatGrpHolder.DataLastKeepAlive = TimeNow;
	//}
	fstream fileOut;
	string GroupsFileName = GRPS_FNAME;
	if (pEnv->GetImplemParam(sLookup, "Implem.Param.GroupsFileName")) {
		GroupsFileName = sLookup;
	}
	if (SafeOpen(fileOut, GroupsFileName, ios::binary | ios::out)) {
		ofstream& fout = *reinterpret_cast<ofstream*>(&fileOut);
		uint RandSoFar = rand();
		fileOut.write((char *)&(RandSoFar), sizeof(RandSoFar));
		uint NumGroups = PatGrpTbl.size();
		fileOut.write((char *)&(NumGroups), sizeof(NumGroups));
		itpg = PatGrpTbl.begin();
		for (; itpg != PatGrpTbl.end(); itpg++) {
			CPatGrpHolder& PatGrpHolder = itpg->second;
			StringStore(PatGrpHolder.PatGrpRec.Name, fout);
			PatGrpHolder.Store(fileOut);
			StringStore(PatGrpHolder.PatGrpRec.pfinder->group_name_template, fout);
		}
		fileOut.write((char *)&GrpMaxFileID, sizeof(GrpMaxFileID));
		map<string, SWordData>& WordTbl = pEnv->getWordTbl();
		uint TblSize = WordTbl.size();
		fileOut.write((char *)&(TblSize), sizeof(TblSize));
		map<string, SWordData>::iterator itWordTbl = WordTbl.begin();
		for (; itWordTbl != WordTbl.end(); itWordTbl++) {
			StringStore(string(itWordTbl->first), fout);
			itWordTbl->second.Store(fout);			
		}
		fileOut.write((char *)&NumGrpsSavedInDataFile, sizeof(NumGrpsSavedInDataFile));
	}
	if (TestFileExists(GroupsDataFileName)) {
		if (!FileRemove(GroupsDataFileName)) {
			cerr << "Cannot store group data. Old file cannot be removed\n";
			return;
		}
	}
	if (!FileRename(TempFileName, GroupsDataFileName)) {
		cerr << "File rename of groups data file failed at end of save. Pity!\n";
		return;
	}
	if (bDataWasAvail) {
		if (SafeOpen(	fileDataForLoading, GroupsDataFileName, 
						ios::binary | ios::in)) {
			bDataAvailForLoading = true;
		}
	}
	std::cout << "PatGrpMgr store took " << (TimerNow() - TimeNow) / 1000000ll << "s\n";
}


void CPatGrpMgr::Load(vector<SPatternFinder >& anc_pat_finder_list)
{
	fstream fileIn;
	GrpMaxFileID = cGrpMaxFileIDStart;
	string GroupsFileName = GRPS_FNAME;
	string sLookup;
	if (pEnv->GetImplemParam(sLookup, "Implem.Param.GroupsFileName")) {
		GroupsFileName = sLookup;
	}
	if (SafeOpen(fileIn, GroupsFileName, ios::binary | ios::in)) {
		bInitialLoadDone = true;
		ifstream& fin = *reinterpret_cast<ifstream*>(&fileIn);
		uint RandSoFar;
		fileIn.read((char *)&(RandSoFar), sizeof(RandSoFar));
		srand(RandSoFar);
		uint NumGroups;
		fileIn.read((char *)&(NumGroups), sizeof(NumGroups));
		for (uint ig = 0; ig < NumGroups; ig++) {
			string GrpName;
			StringLoad(GrpName, fin);
			PatGrpTbl[GrpName] = CPatGrpHolder();
			CPatGrpHolder& PatGrp = PatGrpTbl[GrpName];
			PatGrp.PatGrpRec.Name = GrpName;
			PatGrp.Load(fileIn);
			string FinderName;
			StringLoad(FinderName, fin);
			for (uint ia = 0; ia < anc_pat_finder_list.size(); ia++) {
				if (FinderName == anc_pat_finder_list[ia].group_name_template) {
					PatGrp.PatGrpRec.pfinder = &(anc_pat_finder_list[ia]);
					PatGrp.PatGrpRec.pfinder->ChildPatGrps.push_back(&(PatGrpTbl[PatGrp.PatGrpRec.Name]));
					break;
				}
			}
			PatGrp.pMgr = this;
			PatGrpIDTbl[PatGrp.iFileID] = &PatGrp;
		}
		fileIn.read((char *)&GrpMaxFileID, sizeof(GrpMaxFileID));
		map<string, SWordData>& WordTbl = pEnv->getWordTbl();
		WordTbl.clear();
		uint TblSize ;
		fileIn.read((char *)&(TblSize), sizeof(TblSize));
		for (uint iW = 0; iW < TblSize; iW++) {
			string W;
			StringLoad(W, fin);
			pair<map<string, SWordData>::iterator, bool> MapPair
						= WordTbl.insert(pair<string, SWordData>(W, SWordData()));
			map<string, SWordData>::iterator itWordTbl = MapPair.first;
			itWordTbl->second.Load(fin);

		}
		fileIn.read((char *)&NumGrpsSavedInDataFile, sizeof(NumGrpsSavedInDataFile));
	} // end of if grps file safely opened
	string GroupsDataFileName = GRPS_DATA;
	//string sLookup;
	if (pEnv->GetImplemParam(sLookup, "Implem.Param.GroupsDataFileName")) {
		GroupsDataFileName = sLookup;
	}
	if (fileDataForLoading.is_open()) {
		fileDataForLoading.close();
	}
	if (SafeOpen(fileDataForLoading, GroupsDataFileName, ios::binary | ios::in)) {
		bDataAvailForLoading = true;
	}
}

fstream& CPatGrpMgr::getFStreamForGrp(bool& bAvail)
{
	bAvail = bDataAvailForLoading;
	return fileDataForLoading;
}

#ifdef NEVE_USED_CODE
void CPatGrpMgr::CreateWordDataLinks(map<string, SWordData>& MapWordToData)
{
	map<string, SWordData>::iterator itWordMap = MapWordToData.begin();

	for (; itWordMap != MapWordToData.end(); itWordMap++) {
		itWordMap->second.GrpList.clear();
	}

	CPatGrpIter itTbl = getStartIterator();
	while (getNextIterator(itTbl)) {
		CPatGrpWrapper PatGrp(itTbl);
		int NumEls = PatGrp.getGrpElsMapSize();
		for (int iEl = 0; iEl < NumEls; iEl++) {
			SGroupingLink* pEl = PatGrp.getGrpElByIndex(iEl);
			map<string, SWordData>::iterator itWordMap = MapWordToData.find(pEl->W);
			if (itWordMap != MapWordToData.end()) {
				itWordMap->second.GrpList.push_back(itTbl.itpgt);
			}
		}

	}

	map<string, CPatGrpHolder>::iterator itGrpTbl = PatGrpTbl.begin();

	for (; itGrpTbl != PatGrpTbl.end(); itGrpTbl++) {
		
	}

}
#endif // NEVER_USED_CODE

CPatGrpHolder *  CPatGrpMgr::FindGrpByFileID(int iGrpFileID)
{
	map<int, CPatGrpHolder*>::iterator itIDTbl = PatGrpIDTbl.find(iGrpFileID);
	if (itIDTbl != PatGrpIDTbl.end()) {
		return itIDTbl->second;
	}
	return NULL;
}

void srand(int)
{
	cerr << "srand is called after all\n";
}
