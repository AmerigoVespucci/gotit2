// Regex2D.cpp : 2D regex support
//

#include "stdafx.h"
#include <limits.h>

#include "MascReader.h"
#include "Regex2D.h"
#include "RegexInternal.h"

vector<HeadRegexEl> GamerWhiteMoves;
vector<HeadRegexEl> GamerBlackMoves;
vector<HeadRegexEl> GamerConseqMoves; // consequences
vector<HeadRegexEl> GamerScoringMoves; // consequences


void RegexEl::AddOption(const RegexEl& Rule) {
	Options.push_back(Rule);
	MinRepeat = 1;
	MaxRepeat = 1;
}
void RegexEl::SetOptionRepeats(int MinR, int MaxR) {
	MinRepeat = MinR;
	MaxRepeat = MaxR;
}
void RegexEl::SetCont(const RegexEl& RuleForCont) {
	//if (Cont != NULL) {
	//	delete Cont;
	//}
	Cont.push_back(RuleForCont);
}
void RegexEl::AddCond(const RegexCond& aCond) {
	Cond.push_back(aCond);
}

void HeadRegexEl::AddStartCond(const RegexCond& aCond) {
	StartCond.push_back(aCond);
}

void RegexEl::PrintDir(string& sOut, RegexDir rdt) {
	switch (rdt) {
		case rdtAny:
			sOut += "";
			break;
		case rdtDown:
			sOut += "^d";
			break;
		case rdtHoriz:
			sOut += "^h";
			break;
		case rdtLeft:
			sOut += "^l";
			break;
		case rdtRight:
			sOut += "^r";
			break;
		case rdtUp:
			sOut += "^u";
			break;
		case rdtVert:
			sOut += "^v";
			break;
		default:
			sOut += "!Dir Error!";
			break;
	}
}

void RegexEl::PrintSideEffect(string& sOut, RegexSideEffectType rset, char Param) {
	switch (rset) {
		case rsetSetFV:
			sOut += "SetFV";
			sOut += string(" ") + (char)('0' + Param);
			break;
		case rsetSetA:
			sOut += "SetA";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetSetB:
			sOut += "SetB";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetSetGA:
			sOut += "SetGA";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetIncrA:
			sOut += "IncrA";
			break;
		case rsetIncrB:
			sOut += "IncrB";
			break;
		case rsetIncrGA:
			sOut += "IncrGA";
			break;
		case rsetDecrA:
			sOut += "DecrA";
			break;
		case rsetDecrB:
			sOut += "DecrB";
			break;
		case rsetDecrGA:
			sOut += "DecrGA";
			break;
		case rsetSetS:
			sOut += "SetS";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetAddToS:
			sOut += "AddToS";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetSetEnd:
			sOut += "SetEnd";
			break;
		case rsetJumpEA:
			sOut += "JumpEA";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetJumpEB:
			sOut += "JumpEB";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetExit:
			sOut += "Exit";
			sOut += string(" ") + char('0' + Param);
			break;
		case rsetNone:
		default:
			break;
	}
}

void RegexEl::PrintCond(string& sOut, const RegexCond& Cond)
{
	switch(Cond.rpt) {
		case rptGT:
			sOut += "GT";
			sOut += string(" ") + char('0' + Cond.Val);
			break;
		case rptEQ:
			sOut += "EQ";
			sOut += string(" ") + char('0' + Cond.Val);
			break;
		case rptEQx:
			sOut += "x ==";
			sOut += string(" ") + char('0' + Cond.Val);
			break;
		case rptEQy:
			sOut += "y == ";
			sOut += string(" ") + char('0' + Cond.Val);
			break;
			
	}
}

bool RegexSideEffect::operator == (const RegexSideEffect& Other) const
{
	if (	(rset == Other.rset)
		&&	(Param == Other.Param)) {
		return true;
	}
	return false;
}

bool RegexElChar::operator == (const RegexElChar& Other) const
{
	if (	EqualsCore(Other)
		&&  (SideEffects == Other.SideEffects) ) {
		return true;
	}

	return false;
}

bool RegexElChar::EqualsCore(const RegexElChar& Other) const
{
	if (	(InputSym == Other.InputSym )
		&&	(InputMax == Other.InputMax)
		&&	(ct == Other.ct)
		&&	(rdt == Other.rdt) ) {
		return true;
	}

	return false;
}

bool RegexCond::operator ==(const RegexCond& Other) const
{
	if (	(rpt == Other.rpt)
		&&	(Val == Other.Val)) {
		return true;
	}
	return false;
}

// returns true even if the SEs are different
// if they are also identical sets SEEquals to t
bool RegexEl::Equals(RegexEl& Other, bool& SEEquals)
{
	bool CoreEquals = true;
	SEEquals = true;
	if (MatchChars.size() != Other.MatchChars.size()) {
		return false;
	}
	for (uint iSym = 0; iSym < MatchChars.size(); iSym++) {
		RegexElChar& El = MatchChars[iSym];
		RegexElChar& ElOther = Other.MatchChars[iSym];
		if (!(El == ElOther)) {
			SEEquals = false;
		}
		if (!El.EqualsCore(ElOther)) {
			CoreEquals = false;
		}
	}
	if (Cond.size() != Other.Cond.size()) {
		return false;
	}
	for (uint iC = 0; iC < Cond.size(); iC++) {
		RegexCond& C = Cond[iC];
		RegexCond& OC = Other.Cond[iC];
		if (!(C == OC)) {
			return false;
		}
		
	}
	return CoreEquals;
}

void HeadRegexEl::Print(string& ret) {
	if (StartCond.size() > 0) {
		string sPrint = "^{SC ";
		
		for (uint ir = 0; ir < StartCond.size(); ir++) {
			if (ir > 0) {
				sPrint += "; ";
			}
			PrintCond(sPrint, StartCond[ir]);
		}
		sPrint += "^} ";
		ret += sPrint;
	}
	RegexEl::Print(ret);
}

void RegexEl::Print(string& ret) {
	for (uint iSym = 0; iSym < MatchChars.size(); iSym++) {
		RegexElChar& El = MatchChars[iSym];
		string sPrint = "";
		PrintDir(sPrint, El.rdt);
		//sPrint += '0' + El.InputSym;
		if (El.ct == ctMatchSyms) {
			sPrint += El.InputSym;			
		}
		else if (El.ct == ctRange) {
			sPrint += "[" + string(1, El.InputSym) + string("-")
				+ string(1, El.InputMax) + "]";
		}
		if (	El.SideEffects.size() > 0 
			&&	!((	El.SideEffects.size() == 1) 
				&&	El.SideEffects[0].rset == rsetNone)) {
			sPrint += "^{SE ";
			for (uint iSE = 0; iSE < El.SideEffects.size(); iSE++) {
				if (iSE > 0) {
					sPrint += "; ";
				}
				PrintSideEffect(sPrint, El.SideEffects[iSE].rset, El.SideEffects[iSE].Param);
			}
			sPrint += "^} ";
		}
		ret += sPrint;
	}
		//ret += "\"" + sMatch + "\"";
	ret += " (";
	for (uint i = 0; i < Options.size(); i++) {
		Options[i].Print(ret);
		ret += " | ";
	}
	ret += "){";
    string buf;
	//char buf[32];
	//_itoa_s(MinRepeat, buf, sizeof(buf), 10);
    buf = to_string(MinRepeat);
	ret += buf;
	ret += ",";
    buf = to_string(MaxRepeat);
	//_itoa_s(MaxRepeat, buf, sizeof(buf), 10);
	ret += buf;
	ret += "} ";
	if (Cond.size() > 0) {
		string sPrint = "^{C ";
		
		for (uint ir = 0; ir < Cond.size(); ir++) {
			if (ir > 0) {
				sPrint += "; ";
			}
			PrintCond(sPrint, Cond[ir]);
		}
		sPrint += "^} ";
		ret += sPrint;
	}
	if (Cont.size() > 0) {
		ret += "-> ";
		Cont[0].Print(ret);
	}
	else {
		ret += "-X ";
	}

}

int RegexRule::OnePosExec(	RegexEl * apRule, int aTblSize, char * pOldTbl, 
							int xStart, int yStart,
							bool bEndReq, int xEnd, int yEnd)
{
	TblSize = aTblSize;

	if (	(xStart >= TblSize) || (yStart >= TblSize) 
		||	(xStart < 0) || (yStart < 0)) {
		return -1;
	}

	StateTbl.resize(TblSize, vector<char>(TblSize, -1));

	if (apRule->bDebug) {
		bDebug = true;
	}
	if (bDebug) {
		cout << "Exec rule for pos " << xStart << ", " << yStart << ": \n";
		string sDebug;
		apRule->Print(sDebug);
		cout << "\t" << sDebug << endl;
	}

	OldTbl.resize(TblSize, vector<char>(TblSize, 0));
	pRule = apRule;
	RegexThread InitThread;
	InitThread.y = yStart;
	InitThread.x = xStart - 1;
	InitThread.bStart = true;
	if (!bUseGlobalState) {
		InitThread.StateTbl.resize(TblSize, vector<char>(TblSize, 0));
	}
	for (int yTbl = 0; yTbl < TblSize; yTbl++) {
		for (int xTbl = 0; xTbl < TblSize; xTbl++) {
			OldTbl[yTbl][xTbl] = pOldTbl[(yTbl*TblSize) + xTbl];
			if (!bUseGlobalState) {
				InitThread.StateTbl[yTbl][xTbl] = -1;
			}
		}
	}

	//ActiveThreads.push_back(InitThread);
	RegexStackEl FirstStackEl(pRule);
	FirstStackEl.AliveThreads.push_back(InitThread);
	RuleExecPos.clear();
	RuleExecPos.push_back(FirstStackEl);
	char TblSym = OldTbl[yStart][xStart];
	//while ((AliveThreads.size() > 0) || (PausingThreads.size() > 0)) {
	while (true) {
		bool bExpectNewThreads = true;
		if (RuleExecPos.size() == 0) {
			break;
		}
		RegexStackEl& StackTop = RuleExecPos.back();
		if (StackTop.AliveThreads.size() == 0) {
			// No stack element can stay on the stack if there are no more alive threads
			if (bDebug) {
				cout << "Popping rule from stack level " << RuleExecPos.size() << " with no live threads: \n";
				string sDebug;
				StackTop.pCurrEl->Print(sDebug);
				cout << "\t" << sDebug << endl;
			}
			RuleExecPos.pop_back();
			continue;
		}
		if (StackTop.MatchSymsDone < StackTop.pCurrEl->MatchChars.size()) {
			RegexElChar& ElChar = StackTop.pCurrEl->MatchChars[StackTop.MatchSymsDone];
			char MatchSym = ElChar.InputSym;
			RegexDir rdtMatch = ElChar.rdt;
			vector<RegexSideEffect>& SideEffects = ElChar.SideEffects;

			StackTop.MatchSymsDone++;
			vector<RegexThread> NewThreads;
			vector<RegexThread>& ActiveThreads = StackTop.AliveThreads;
			for (uint iOldThread = 0; iOldThread < ActiveThreads.size(); iOldThread++) {
				RegexThread& OldThread = ActiveThreads[iOldThread];
				vector<vector<char> >& OThStateTbl = OldThread.StateTbl;
				for (int iVertNotHoriz = 0; iVertNotHoriz < 2; iVertNotHoriz++) {
					bool bVert = false;
					if (iVertNotHoriz == 1) {
						bVert = true;
					}
					for (int iBoth = 0; iBoth < 2; iBoth++) {
						int yDelta = 0, xDelta = 0;
						bool bUp = false, bDown = false, bRight = false, bLeft = false;
						if (iBoth == 0) {
							if (bVert) {
								bUp = true;
								yDelta = 1;
							}
							else {
								bRight = true;
								xDelta = 1;
							}
						}
						else {
							if (bVert) {
								bDown = true;
								yDelta = -1;
							}
							else {
								bLeft = true;
								xDelta = -1;
							}
						}
						if (OldThread.bStart && !bRight) {
							continue;
						}
						switch (rdtMatch) {
							case rdtDown:
								if (!bDown) continue;
								break;
							case rdtUp:
								if (!bUp) continue;
								break;
							case rdtRight:
								if (!bRight) continue;
								break;
							case rdtLeft:
								if (!bLeft) continue;
								break;
							case rdtVert:
								if (!bUp && !bDown) continue;
								break;
							case rdtHoriz:
								if (!bRight && !bLeft) continue;
							case rdtAny:
								break;
						}
						int xNew = OldThread.x + xDelta;
						int yNew = OldThread.y + yDelta;
						if (StackTop.MatchSymsDone == 1) { 
							if (RuleExecPos.size() == 1) {
								HeadRegexEl * pHeadEl = (HeadRegexEl *)(StackTop.pCurrEl);
								if (pHeadEl->StartCond.size() > 0) {
									bool StartAborted = false;
									for (int ir = 0; ir < pHeadEl->StartCond.size(); ir++) {
										RegexCond& cond = pHeadEl->StartCond[ir];
										switch (cond.rpt) {
											case (rptEQx):
												if (xNew != cond.Val) StartAborted = true;
												break;
											case rptEQy:
												if (yNew != cond.Val) StartAborted = true;;
												break;
										}
									}
									if (StartAborted) {
										continue;
									}
								}
							}
						}
						bool bOOB = false;
						if ((xNew < 0) || (xNew >= TblSize)) {
							if (MatchSym == '@') {
								xNew = OldThread.x;
								bOOB = true;
							}
							else {
								continue;
							}
						}
						if ((yNew < 0) || (yNew >= TblSize)) {
							if (MatchSym == '@') {
								yNew = OldThread.y;
								bOOB = true;
							}
							else {
								continue;
							}
						}

						char TblSym = OldTbl[yNew][xNew] + '0';
						if (!bOOB && (MatchSym != '.')) {
							if (ElChar.ct == ctMatchSyms) {
								if (TblSym != MatchSym) {
									continue;
								}
							}
							else {
								if (TblSym < MatchSym || TblSym > ElChar.InputMax) {
									continue;
								}
							}
						}
						if (!bUseGlobalState && !bOOB && OThStateTbl[yNew][xNew] != -1) {
							continue; // we have reached an area that has already been initialized
						}
						if (bUseGlobalState && !bOOB && StateTbl[yNew][xNew] != -1) {
							continue; // we have reached an area that has already been initialized
						}
						RegexThread NewThread; // = OldThread;
						NewThread.x = xNew; NewThread.y = yNew; NewThread.Score = OldThread.Score;
						if (!bUseGlobalState) {
							NewThread.StateTbl = OThStateTbl;
							NewThread.StateTbl[yNew][xNew] = 1;
						}
						else {
							StateTbl[yNew][xNew] = 1;
						}
						NewThread.FVRecs = OldThread.FVRecs;
						if (SideEffects.size() > 0 && SideEffects[0].rset != rsetNone) {
							for (uint iSE = 0; iSE < SideEffects.size(); iSE++) {
								RegexSideEffect& SE = SideEffects[iSE];
								switch (SE.rset) {
									case rsetSetFV:
										NewThread.FVRecs.push_back(
											BoardDelta(xNew, yNew, SE.Param));
										break;
									case rsetSetA:
										NewThread.A = SE.Param;
										break;
									case rsetSetB:
										NewThread.B = SE.Param;
										break;
									case rsetSetGA:
										regGA = SE.Param;
										break;
									case rsetIncrA:
										NewThread.A++;
										break;
									case rsetIncrB:
										NewThread.B++;
										break;
									case rsetIncrGA:
										regGA++;
										break;
									case rsetDecrA:
										NewThread.A--;
										break;
									case rsetDecrB:
										NewThread.B--;
										break;
									case rsetDecrGA:
										regGA--;
										break;
									case rsetSetS:
										NewThread.Score = SE.Param;
										break;
									case rsetAddToS:
										NewThread.Score += SE.Param;
										break;
									case rsetSetEnd:
										NewThread.End.first = xNew;
										NewThread.End.second = yNew;
										break;
								}
							}
						}
						NewThreads.push_back(NewThread);
					} // iBoth
				} // Vert
			} // threads
			ActiveThreads.clear();
			ActiveThreads = NewThreads; // very wasteful. Keep 2 variables instead
			NewThreads.clear();
		} // end if in middle of matching a list of match syms
		else if (StackTop.OptionCount < StackTop.pCurrEl->Options.size()) {
			// You get here if the match syms (or range TBI) are all done and there are
			// options still to be processed; including the first
			StackTop.bDoingOptions = true;
			if ((StackTop.OptionCount == 0) && (StackTop.CurrRepeat == 0)) {
				StackTop.StackedThreads = StackTop.AliveThreads;
				if (StackTop.pCurrEl->MinRepeat == 0) {
					StackTop.CompletedThreads = StackTop.AliveThreads;
				}
			}
			//if (StackTop.OptionCount > 0) {
			//	StackTop.PausingThreads.insert(	StackTop.PausingThreads.end(), 
			//									StackTop.AliveThreads.begin(), 
			//									StackTop.AliveThreads.end());
			//}
			//vector<RegexThread> TempThreads;
			//TempThreads = AliveThreads;
			//StackTop.AliveThreads = StackTop.StackedThreads;
			//StackedThreads = TempThreads;
			RegexStackEl StackEl(&(StackTop.pCurrEl->Options[StackTop.OptionCount]));
			StackTop.OptionCount++;
			StackEl.AliveThreads = StackTop.StackedThreads;
			if (bDebug) {
				cout << "Adding option rule to stack level " << RuleExecPos.size() + 1 << " with " << StackEl.AliveThreads.size() << " threads: \n";
				string sDebug;
				StackEl.pCurrEl->Print(sDebug);
				cout << "\t" << sDebug << endl;
			}
			RuleExecPos.push_back(StackEl);
		}
		else if (StackTop.bDoingOptions) {
			StackTop.CurrRepeat++;
			if (StackTop.CurrRepeat >= StackTop.pCurrEl->MinRepeat) {
				StackTop.CompletedThreads.insert(
					StackTop.CompletedThreads.end(),
					StackTop.PausingThreads.begin(),
					StackTop.PausingThreads.end());
			}
			if ((StackTop.CurrRepeat < StackTop.pCurrEl->MaxRepeat)
				&& (StackTop.PausingThreads.size() > 0)) {
				StackTop.OptionCount = 0;
				StackTop.StackedThreads = StackTop.PausingThreads;
				StackTop.PausingThreads.clear();
				//continue;
			}
			else {
				StackTop.AliveThreads = StackTop.CompletedThreads;
				StackTop.bDoingOptions = false;
			}
		}
		else if (StackTop.NumContsDone < (int)(StackTop.pCurrEl->Cont.size())) {
			StackTop.PausingThreads.clear(); // just in case because we will use this as the returning data
			StackTop.bDoingConts = true;
			RegexStackEl StackEl(&(StackTop.pCurrEl->Cont[StackTop.NumContsDone]));
			StackTop.NumContsDone++;
			StackEl.AliveThreads = StackTop.AliveThreads;
			if (bDebug) {
				cout << "Adding cont rule to stack level " << RuleExecPos.size() + 1 << " with " << StackEl.AliveThreads.size() << " threads: \n";
				string sDebug;
				StackEl.pCurrEl->Print(sDebug);
				cout << "\t" << sDebug << endl;
			}
			RuleExecPos.push_back(StackEl);
		}
		else if (StackTop.bDoingConts) {
			StackTop.AliveThreads = StackTop.PausingThreads;
			StackTop.bDoingConts = false;
		}
		else {
			//if (StackTop.CurrRepeat > StackTop.pCurrEl->MinRepeat) {
			//	AliveThreads.insert(AliveThreads.end(), PausingThreads.begin(),
			//						PausingThreads.end());
			//}
			// prepare to pop a stack el with alive threads
			if (StackTop.pCurrEl->bGlobalStateTbl) {                
				bool bClear = false;
				for (uint ic = 0; ic < StackTop.pCurrEl->Cond.size(); ic++) {
					RegexCond& Cond = StackTop.pCurrEl->Cond[ic];
					switch (Cond.rpt) {
						case rptGT:
							if (regGA <= Cond.Val) {
								bClear = true;
							}
							break;
						case rptEQ:
							if (regGA != Cond.Val) {
								bClear = true;
							}
							break;
					}
				}
				if (bClear) {
					CompletedThreads.clear();
					if (bDebug) {
						cout << "Aborting all threads because condition not met\n";
					}
 					return false;                        
				}
			}
			if (RuleExecPos.size() > 1) {
				RegexStackEl& PenUl = RuleExecPos[RuleExecPos.size() - 2];
				PenUl.PausingThreads.insert(PenUl.PausingThreads.end(),
											StackTop.AliveThreads.begin(),
											StackTop.AliveThreads.end());

			}
			else {
				CompletedThreads = StackTop.AliveThreads;
			}
			if (bDebug) {
				cout << "Popping rule from stack level " << RuleExecPos.size() << " with " << StackTop.AliveThreads.size() << " threads: \n";
				string sDebug;
				StackTop.pCurrEl->Print(sDebug);
				cout << "\t" << sDebug << endl;
				for (uint it = 0; it < StackTop.AliveThreads.size(); it++) {
					cout	<< "Thread at " << StackTop.AliveThreads[it].x << ", " 
							<< StackTop.AliveThreads[it].y 
							<< ". Score: " << StackTop.AliveThreads[it].Score << endl;
				}
			}

			RuleExecPos.pop_back();
		}
		//if (bExpectNewThreads) {
		//	AliveThreads.clear();
		//	AliveThreads = NewThreads; // very wasteful. Keep 2 variables instead
		//	NewThreads.clear();
		//}


	} // while t

	if (CompletedThreads.size() > 0) {
		if (bEndReq) {
			vector<RegexThread> CheckedThreads;
			for (int it = 0; it < CompletedThreads.size(); it++) {
				RegexThread& OneThread = CompletedThreads[it];
				if (OneThread.End == make_pair(xEnd, yEnd)) {
					CheckedThreads.push_back(OneThread);
				}
			}
			if (CheckedThreads.size() > 0) {
				CompletedThreads = CheckedThreads;
			}
			else {
				CompletedThreads.clear();
				return false;
			}
		}
        bool bClear = false;
		if (apRule->bGlobalStateTbl) {                

			for (uint ic = 0; ic < apRule->Cond.size(); ic++) {
				RegexCond& Cond = apRule->Cond[ic];
				switch (Cond.rpt) {
					case rptGT:
						if (regGA <= Cond.Val) {
							bClear = true;
						}
						break;
					case rptEQ:
						if (regGA != Cond.Val) {
							bClear = true;
						}
						break;
				}
			}
            if (bClear) {
                CompletedThreads.clear();
               return false;                        
            }
		}
		return true;
	}
	return false;
}


int RegexRule::TestThreadsOnRealTbl(char * pNewTbl)
{
	vector<vector<char> > NewTbl;
	NewTbl.resize(TblSize, vector<char>(TblSize, 0));
	for (int yTbl = 0; yTbl < TblSize; yTbl++) {
		for (int xTbl = 0; xTbl < TblSize; xTbl++) {
			NewTbl[yTbl][xTbl] = pNewTbl[(yTbl*TblSize) + xTbl];
		}
	}
	vector<vector<char> > PossTbl;
	PossTbl.resize(TblSize, vector<char>(TblSize, 0));
	for (uint iThread = 0; iThread < CompletedThreads.size(); iThread++) {
		PossTbl = OldTbl;
		RegexThread& RegTh = CompletedThreads[iThread];
		for (uint iFV = 0; iFV < RegTh.FVRecs.size(); iFV++) {
			BoardDelta& rec = RegTh.FVRecs[iFV];
			PossTbl[rec.y][rec.x] = rec.NewVal;
		}
		if (PossTbl == NewTbl) {
			return iThread;
		}
	}
	return -1;
}

template<class InputIt>
bool VecEq(InputIt first1, InputIt last1, InputIt first2, InputIt last2)
{
	while (first1 != last1 && first2 != last2) {
		if (!(*first1 == *first2)) {
			return false;
		}
		++first1, ++first2;
	}
	if (first1 == last1 && first2 == last2) {
		return true;
	}
	
	return false;
}

void RegexRule::CreateMoveDeltas(vector<OneMove >& AllMoveOpts)
{
	for (uint iThread = 0; iThread < CompletedThreads.size(); iThread++) {
		RegexThread& RegTh = CompletedThreads[iThread];
		OneMove MoveOpt;
		for (uint iFV = 0; iFV < RegTh.FVRecs.size(); iFV++) {
			BoardDelta& rec = RegTh.FVRecs[iFV];
			MoveOpt.Deltas.push_back(rec);
			//BoardDelta TestRec(rec.x, rec.y, rec.NewVal);
		}
		MoveOpt.Score = RegTh.Score;
		bool bAlreadyThere = false;
		for (uint iOpt = 0; iOpt < AllMoveOpts.size(); iOpt++) {			
			if (VecEq(	MoveOpt.Deltas.begin(), MoveOpt.Deltas.end(), 
						AllMoveOpts[iOpt].Deltas.begin(), 
						AllMoveOpts[iOpt].Deltas.end())) {
				bAlreadyThere = true;
				break;
			}
		}
		if (!bAlreadyThere) {
			AllMoveOpts.push_back(MoveOpt);
		}
	}

}

int RegexEl::CreateCount = 0;

void TestRegex()
{
	//RegexSideEffect rse0(rsetSetFV, 0);
	//RegexElChar rec0('1', rse0);
	//vector<RegexElChar> InitialMove;
	//InitialMove.push_back(rec0);
	//RegexElChar rec1('.', crseNone, rdtVert);
	//RegexSideEffect rse1(rsetSetFV, 1);
	//RegexElChar rec2('0', rse1, rdtHoriz);
	//InitialMove.push_back(rec1);
	//InitialMove.push_back(rec2);
	//RegexEl CheckersSimpleMove(InitialMove);
	
	/*
	vector<RegexElChar> Moves;
	Moves.push_back(RegexElChar('1', RegexSideEffect(rsetSetFV, 0)));
	Moves.push_back(RegexElChar('.', crseNone, rdtUp));
	RegexEl CheckersSimpleMove(Moves);
	Moves.clear();
	for (int iBoth = 0; iBoth < 2; iBoth++) {
		RegexDir rdt = rdtRight;
		if (iBoth == 1) rdt = rdtLeft;
		RegexEl BothOpt;
		BothOpt.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('2', RegexSideEffect(rsetSetFV, 0), rdt))));
		BothOpt.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('4', RegexSideEffect(rsetSetFV, 0), rdt))));
		Moves.push_back(RegexElChar('.', crseNone, rdtUp));
		Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, 1), rdt));
		BothOpt.SetCont(RegexEl(Moves));
		//	CheckersSimpleMove.SetCont(RegexEl(Moves));
		CheckersSimpleMove.AddOption(BothOpt);
	}
	*/
	/*
	vector<RegexElChar> Moves;
	Moves.push_back(RegexElChar('1', RegexSideEffect(rsetSetFV, 0)));
	Moves.push_back(RegexElChar('.', crseNone, rdtUp));
	RegexEl CheckersSimpleMove(Moves);
	Moves.clear();
	Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, 3), rdtHoriz));
	Moves.push_back(RegexElChar('@', crseNone, rdtUp));
	CheckersSimpleMove.AddOption(RegexEl(Moves));
	Moves.clear();
	Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, 1), rdtHoriz));
	Moves.push_back(RegexElChar('.', crseNone, rdtUp));
	CheckersSimpleMove.AddOption(RegexEl(Moves));
	string sForOut;
	CheckersSimpleMove.Print(sForOut);
	cout << sForOut << endl;
	*/
	vector<RegexElChar> Moves;
	RegexElChar InitMove('0', RegexSideEffect(rsetSetFV, 2));
	InitMove.SideEffects.push_back(RegexSideEffect(rsetSetGA, 0));
	RegexEl GoValid(vector<RegexElChar>(1, InitMove));
	GoValid.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('2', crseNone, rdtAny))));
	GoValid.SetOptionRepeats(0, INT_MAX);
	Moves.push_back(RegexElChar('2', crseNone, rdtAny));
	RegexEl GoCont;
	GoCont.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('1', crseNone, rdtAny))));
	GoCont.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0', RegexSideEffect(rsetIncrGA), rdtAny))));
	GoValid.SetCont(GoCont);
	static const int cTblSize = 5;
	//struct PP {
	//	PP(int ax, int ay, int aval) {
	//		x = ax; y = ay; val = aval;
	//	}
	//	int x;
	//	int y;
	//	char val;
	//};
	char OldTbl[] = {
		0, 1, 2, 2, 0,
		0, 1, 2, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};
	char NewTbl[] = {
		0, 1, 2, 2, 2,
		0, 1, 2, 1, 0,
		0, 0, 1, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};
	//char OldTbl[25];
	//char NewTbl[25];
	//char StateTbl[25];
	//memset(OldTbl, 0, 25);
	//memset(NewTbl, 0, 25);
	//OldTbl[2 * 5 + 2] = 0;
	//OldTbl[3 * 5 + 3] = 1;
	//NewTbl[4 * 5 + 4] = 3;
	RegexRule OneRule;
	OneRule.bUseGlobalState = true;
	if (OneRule.OnePosExec(&GoValid, cTblSize, OldTbl, 4,0)) {
		if (OneRule.TestThreadsOnRealTbl(NewTbl) >= 0) {
			cout << "GA = " << OneRule.regGA << endl;
			cerr << "Simple move valid!\n";
			return;
		}
	}
	cerr << "No move found to validate regex\n";
}

//RegexEl CIWhiteRegMove;
//RegexEl CIBlackRegMove;
//RegexEl CIWhiteRegTakeMove;

void CIRegexInit(int BoardSize)
{
	vector<RegexElChar> Moves;

	for (uint iBnW = 0; iBnW < 2; iBnW++) {
		char Mover(1), KMover(3), Take1(2), Take2(4);
		vector<HeadRegexEl>* pRules = &GamerWhiteMoves;
		RegexDir rdtUorD = rdtUp;
		if (iBnW == 1) {
			Mover = 2; KMover = 4;  Take1 = 1; Take2 = 3;
			pRules = &GamerBlackMoves;
			rdtUorD = rdtDown;
		}
		vector<RegexElChar> Moves;
		Moves.push_back(RegexElChar('0'+Mover, RegexSideEffect(rsetSetFV, 0)));
		Moves.push_back(RegexElChar('.', crseNone, rdtUorD));
		Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, Mover), rdtHoriz));
		Moves.push_back(RegexElChar('.', crseNone, rdtUorD)); // extra step says we are not at the end
		pRules->push_back(HeadRegexEl(Moves));

		Moves.clear();
		Moves.push_back(RegexElChar('0' + Mover, RegexSideEffect(rsetSetFV, 0)));
		Moves.push_back(RegexElChar('.', crseNone, rdtUorD));
		Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, KMover), rdtHoriz));
		Moves.push_back(RegexElChar('@', crseNone, rdtUorD)); // extra step means we are beyond the edge
		pRules->push_back(HeadRegexEl(Moves));

		Moves.clear();
		Moves.push_back(RegexElChar('0' + KMover, RegexSideEffect(rsetSetFV, 0)));
		Moves.push_back(RegexElChar('.', crseNone, rdtVert)); // not the same as rdtUorD. 
		Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, KMover), rdtHoriz));
		pRules->push_back(HeadRegexEl(Moves));

		for (int iKnR = 0; iKnR < 2; iKnR++) {
			int KRMover = Mover;
			RegexDir rdtKR = rdtUorD;
			int NumVerts = 1;
			if (iKnR == 1) {
				KRMover = KMover;
				rdtKR = rdtUp; // applied if K only to iVert == 0
				NumVerts = 2; // kings can go both way
			}
			for (int iVert = 0; iVert < NumVerts; iVert++) {
				if (iVert == 1) { // K or regular
					rdtKR = rdtDown;
				}
				Moves.clear();
				Moves.push_back(RegexElChar('0' + KRMover, RegexSideEffect(rsetSetFV, 0)));
				Moves.push_back(RegexElChar('.', crseNone, rdtKR));
				pRules->push_back(HeadRegexEl(Moves));
				RegexEl& MoveStub = pRules->back();
				for (int iRnL = 0; iRnL < 2; iRnL++) {
					RegexDir rdt = rdtRight;
					if (iRnL == 1) rdt = rdtLeft;
					RegexEl BothOpt;
					BothOpt.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0' + Take1, RegexSideEffect(rsetSetFV, 0), rdt))));
					BothOpt.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0' + Take2, RegexSideEffect(rsetSetFV, 0), rdt))));
					Moves.clear();
					Moves.push_back(RegexElChar('.', crseNone, rdtKR));
					RegexEl EdgeOpt;
					if (iKnR == 1) {
						Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, KMover), rdt));
						EdgeOpt.MatchChars = Moves;
					}
					else {
						EdgeOpt.MatchChars = Moves;
						Moves.clear();
						Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, KMover), rdt));
						Moves.push_back(RegexElChar('@', crseNone, rdtKR));
						EdgeOpt.AddOption(RegexEl(Moves));
						Moves.clear();
						Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, Mover), rdt));
						Moves.push_back(RegexElChar('.', crseNone, rdtKR));
						EdgeOpt.AddOption(RegexEl(Moves));

					}
					BothOpt.SetCont(EdgeOpt);
					//	CheckersSimpleMove.SetCont(RegexEl(Moves));
					MoveStub.AddOption(BothOpt);
				}
			}
		}

		char SFactor = 1;
		if (iBnW == 1) {
			SFactor = -1;
		}
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ Mover, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 2)))));
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ KMover, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 3)))));
	
	}

}

void GoRegexInit(int BoardSize)
{
	for (uint iBnW = 0; iBnW < 2; iBnW++) {
		char Mover(1), Take1(2);
		vector<HeadRegexEl>* pRules = &GamerWhiteMoves;
		RegexDir rdtUorD = rdtUp;
		if (iBnW == 1) {
			Mover = 2;   Take1 = 1; 
			pRules = &GamerBlackMoves;
			rdtUorD = rdtDown;
		}
		//vector<RegexElChar> Moves;
		RegexElChar InitMove('0', RegexSideEffect(rsetSetFV, Mover));
		InitMove.SideEffects.push_back(RegexSideEffect(rsetSetGA, 0));
		pRules->push_back(HeadRegexEl(vector<RegexElChar>(1, InitMove)));
		RegexEl& GoValid = pRules->back();
		GoValid.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0' + Mover, crseNone, rdtAny))));
		GoValid.SetOptionRepeats(0, INT_MAX);
		//Moves.push_back(RegexElChar('0' + Mover, crseNone, rdtAny));
		RegexEl GoCont;
		GoCont.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0' + Take1, 
				crseNone, rdtAny))));
		GoCont.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0', 
				RegexSideEffect(rsetIncrGA), rdtAny))));
		GoValid.SetCont(GoCont);
		GoValid.AddCond(RegexCond(rptGT, 0));
		GoValid.SetStateTblGlobal();

		RegexElChar InitTakeMove(	'0' + Take1, RegexSideEffect(rsetSetFV, 0)); 
		InitTakeMove.SideEffects.push_back(RegexSideEffect(rsetSetS, 0)); // conseq moves that do not et score must zero it.
		InitTakeMove.SideEffects.push_back(RegexSideEffect(rsetSetGA, 0));
		GamerConseqMoves.push_back(HeadRegexEl(vector<RegexElChar>(1, InitTakeMove)));
		RegexEl& TakeValid = GamerConseqMoves.back();
		TakeValid.AddOption(RegexEl(vector<RegexElChar>(1,	
														RegexElChar('0' + Take1, 
																	RegexSideEffect(rsetSetFV, 0), 
																	rdtAny))));
		TakeValid.SetOptionRepeats(0, INT_MAX);
		RegexEl TakeCont;
		TakeCont.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0' + Mover, 
				crseNone, rdtAny))));
		TakeCont.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0', 
				RegexSideEffect(rsetIncrGA), rdtAny))));
		TakeValid.SetCont(TakeCont);
		TakeValid.AddCond(RegexCond(rptEQ, 0));
		TakeValid.SetStateTblGlobal();
		//TakeValid.bDebug = true;
		
		char SFactor = 1;
		if (iBnW == 1) {
			SFactor = -1;
		}
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ Mover, 
									RegexSideEffect(rsetSetS, 
													(SFactor * 1)))));

	}
}

void ChessRegexInit(int BoardSize)
{
	vector<RegexElChar> Moves;
	for (uint iBnW = 0; iBnW < 2; iBnW++) {
		char P(1), N(2), B(3), R(4), Q(5), K(6);
		char OppDiff = K;
		int StartRank(1);
		
		vector<HeadRegexEl>* pRules = &GamerWhiteMoves;
		RegexDir rdtForward = rdtUp;
		if (iBnW == 1) {
			pRules = &GamerBlackMoves;
			rdtForward = rdtDown;
			P += OppDiff; N += OppDiff; B += OppDiff; 
			R += OppDiff; Q += OppDiff; K += OppDiff;
			OppDiff = -OppDiff;
			StartRank = BoardSize - 1- StartRank;
		}
		char OppMin = P + OppDiff; char OppMax = K + OppDiff;
		/*
		pRules->push_back(RegexEl(vector<RegexElChar>(1, RegexElChar('0'+ P, 
				RegexSideEffect(rsetSetFV, 0)))));
		RegexEl& PMove = pRules->back();
		PMove.AddOption(RegexEl(vector<RegexElChar>(1, RegexElChar('0', 
				crseNone, rdtForward))));
		PMove.SetOptionRepeats(0, 1);
		PMove.SetCont(RegexEl(vector<RegexElChar>(1, RegexElChar('0', 
				RegexSideEffect(rsetSetFV, P), rdtForward))));
		*/
		Moves.push_back(RegexElChar('0'+ P, RegexSideEffect(rsetSetFV, 0)));
		Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, P), 
									RegexSideEffect(rsetSetEnd),rdtForward));
		pRules->push_back(HeadRegexEl(Moves));  // Interestingly, you don't need to mention RegexEl explicitly
		Moves.clear();

		Moves.push_back(RegexElChar('0'+ P, RegexSideEffect(rsetSetFV, 0)));
		Moves.push_back(RegexElChar('0', crseNone, rdtForward));
		Moves.push_back(RegexElChar('0', RegexSideEffect(rsetSetFV, P), 
									RegexSideEffect(rsetSetEnd), rdtForward));
		pRules->push_back(Moves); Moves.clear();
		pRules->back().AddStartCond(RegexCond(rptEQy, StartRank));

		Moves.push_back(RegexElChar('0'+ P, RegexSideEffect(rsetSetFV, 0)));
		Moves.push_back(RegexElChar('.', crseNone, rdtForward));
		Moves.push_back(RegexElChar('0' + OppMin, '0' + OppMax, RegexSideEffect(rsetSetFV, P), 
									RegexSideEffect(rsetSetEnd), rdtHoriz));

		pRules->push_back(Moves); Moves.clear();
		pRules->push_back(HeadRegexEl(	vector<RegexElChar>(1, RegexElChar('0'+ N, 
										RegexSideEffect(rsetSetFV, 0)))));
		RegexEl& NMove = pRules->back();
		Moves.push_back(RegexElChar('.', crseNone, rdtVert));
		Moves.push_back(RegexElChar('.', crseNone, rdtVert));
		RegexEl NMoveVert(Moves); Moves.clear();
		NMoveVert.AddOption(RegexEl(vector<RegexElChar>(1,	
														RegexElChar('0' + OppMin, 
																	'0' + OppMax,  
																	RegexSideEffect(rsetSetFV, N), 
																	RegexSideEffect(rsetSetEnd),
																	rdtHoriz))));
		NMoveVert.AddOption(RegexEl(vector<RegexElChar>(1,	
														RegexElChar('0',  
																	RegexSideEffect(rsetSetFV, N),  
																	RegexSideEffect(rsetSetEnd),
																	rdtHoriz))));
		NMove.AddOption(NMoveVert);
		Moves.push_back(RegexElChar('.', crseNone, rdtHoriz));
		Moves.push_back(RegexElChar('.', crseNone, rdtHoriz));
		RegexEl NMoveHoriz(Moves); Moves.clear();
		NMoveHoriz.AddOption(RegexEl(vector<RegexElChar>(1,	
														RegexElChar('0' + OppMin, 
																	'0' + OppMax,  
																	RegexSideEffect(rsetSetFV, N), 
																	RegexSideEffect(rsetSetEnd),
																	//RegexSideEffect(rsetAddToS, N),  removed because I think this was a mistake
																	rdtVert))));
		NMoveHoriz.AddOption(RegexEl(vector<RegexElChar>(1,	
														RegexElChar('0',  
																	RegexSideEffect(rsetSetFV, N), 
																	RegexSideEffect(rsetSetEnd),
																	rdtVert))));
		NMove.AddOption(NMoveHoriz);

		for (int iP = 0; iP < 3; iP++ ) {
			char sp = R; char dp = B;
			bool bSlider = true;
			if (iP == 1) {
				sp = Q; dp = Q;
			}
			else if (iP == 2) {
				bSlider = false;
				sp = K; dp = K;
			}
			for (int iUorD = 0; iUorD < 2; iUorD++) {
				RegexDir rdt1(rdtUp);
				if (iUorD == 1) {
					rdt1 = rdtDown;
				}
				for (int iLorR = 0; iLorR < 2; iLorR++) {
					RegexDir rdt2(rdtRight);
					if (iLorR == 1) {
						rdt2 = rdtLeft;
					}
					pRules->push_back(HeadRegexEl(vector<RegexElChar>(1, RegexElChar('0'+ dp, 
							RegexSideEffect(rsetSetFV, 0)))));
					RegexEl& BMove = pRules->back();
					if (bSlider) {
						Moves.push_back(RegexElChar('.', crseNone, rdt1));
						Moves.push_back(RegexElChar('0', crseNone, rdt2));
						BMove.AddOption(RegexEl(Moves)); Moves.clear();		

						BMove.SetOptionRepeats(0, INT_MAX);
					}
					RegexEl BMoveOpt;
					Moves.push_back(RegexElChar('.', crseNone, rdt1));
					Moves.push_back(RegexElChar('0' + OppMin, '0' + OppMax, 
												RegexSideEffect(rsetSetFV, dp), 
												RegexSideEffect(rsetSetEnd),
												rdt2));
					BMoveOpt.AddOption(RegexEl(Moves)); Moves.clear();
					Moves.push_back(RegexElChar('.', crseNone, rdt1));
					Moves.push_back(RegexElChar('0', 
												RegexSideEffect(rsetSetFV, dp), 
												RegexSideEffect(rsetSetEnd),
												rdt2));
					BMoveOpt.AddOption(RegexEl(Moves)); Moves.clear();
					BMove.SetCont(BMoveOpt);
				}
			}

			for (int iUorD = 0; iUorD < 2; iUorD++) {
				bool bVert = true;
				if (iUorD == 1) {
					bVert = false;
				}
				for (int iLorR = 0; iLorR < 2; iLorR++) {
					RegexDir rdt2(rdtUp);
					if (iLorR == 1) {
						rdt2 = rdtDown;
					}				
					if (iUorD == 1) {
						rdt2 = rdtRight;
						if (iLorR == 1) {
							rdt2 = rdtLeft;
						}
					}
					pRules->push_back(HeadRegexEl(vector<RegexElChar>(1, RegexElChar('0'+ sp, 
							RegexSideEffect(rsetSetFV, 0)))));
					RegexEl& RMove = pRules->back();
					if (bSlider) {
						RMove.AddOption(RegexEl(vector<RegexElChar>(1,	RegexElChar('0', crseNone, rdt2)))); 
						RMove.SetOptionRepeats(0, INT_MAX);
					}
					RegexEl RMoveOpt;
					RMoveOpt.AddOption(vector<RegexElChar>(1,	
															RegexElChar('0' + OppMin, 
																		'0' + OppMax,  
																		RegexSideEffect(rsetSetFV, sp), 
																		RegexSideEffect(rsetSetEnd),
																		rdt2)));
					RMoveOpt.AddOption(vector<RegexElChar>(1,	
															RegexElChar('0',  
																		RegexSideEffect(rsetSetFV, sp), 
																		RegexSideEffect(rsetSetEnd),
																		rdt2)));
					RMove.SetCont(RMoveOpt);
				}
			}
		}

		char SFactor = 1;
		if (iBnW == 1) {
			SFactor = -1;
		}
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ P, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 1)))));
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ N, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 3)))));
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ B, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 3)))));
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ R, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 5)))));
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ Q, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 9)))));
		GamerScoringMoves.push_back(HeadRegexEl(	RegexElChar('0'+ K, 
											RegexSideEffect(rsetSetS, 
															(SFactor * 125)))));
	}
}
RegexEl OEl;

int TestBrdValidByRegex(	int BoardSize, char * pOldBoard, char * pNewBoard, 
							bool bWhiteTurn, int xSrc, int ySrc, 
							RuleProviderParent * pProvider)
{
	vector<HeadRegexEl>* pRules = &GamerWhiteMoves;
	if (!bWhiteTurn) {
		pRules = &GamerBlackMoves;
	}
	if (pProvider != NULL) {
		vector<HeadRegexEl>* pGivenRules = ((RegexRuleProvider *)pProvider)->GetRules();		
		pRules = pGivenRules;
	}
	//pRules->clear();
	//pRules->push_back(OEl);
	for (uint ir = 0; ir < pRules->size(); ir++) {
		if (!(*pRules)[ir].bValid) continue;
		RegexRule OneRule;
		if (OneRule.OnePosExec(&((*pRules)[ir]), BoardSize, pOldBoard, xSrc, ySrc)) {
			if (OneRule.TestThreadsOnRealTbl(pNewBoard) >= 0) {
				//cerr << "Simple move valid!\n";
				return (int)ir;
			}
		}
	}

	//cerr << "No move found to validate regex\n";
	return -1;

}

void GamerRegexGetValidMoves(	vector<OneMove >& Moves, int BoardSize, 
								char * pOldBoard, bool bWhiteTurn, 
								RuleProviderParent * pProvider)
{
	vector<HeadRegexEl>* pRules = &GamerWhiteMoves;
	if (!bWhiteTurn) {
		pRules = &GamerBlackMoves;
	}
	if (pProvider != NULL) {
		vector<HeadRegexEl>* pGivenRules = ((RegexRuleProvider *)pProvider)->GetRules();
		
		pRules = pGivenRules;
	}
	for (int iRow = 0; iRow < BoardSize; iRow++) {
		for (int iCol = 0; iCol < BoardSize; iCol++) {
			for (uint ir = 0; ir < pRules->size(); ir++) {
				if (!(*pRules)[ir].bValid) continue;
				RegexRule OneRule;
				if (OneRule.OnePosExec(	&((*pRules)[ir]), BoardSize, 
										(char *)pOldBoard, iCol, iRow)) {
					OneRule.CreateMoveDeltas(Moves);
				}
			}
		}
	}


}

void GamerRegexGetValidMoves(	vector<OneMove >& Moves, int BoardSize, 
								char * pOldBoard, bool bWhiteTurn, 
								int xStart, int yStart,
								RuleProviderParent * pProvider)
{
	vector<HeadRegexEl>* pRules = &GamerWhiteMoves;
	if (!bWhiteTurn) {
		pRules = &GamerBlackMoves;
	}
	if (pProvider != NULL) {
		vector<HeadRegexEl>* pGivenRules = ((RegexRuleProvider *)pProvider)->GetRules();
		
		pRules = pGivenRules;
	}
	for (uint ir = 0; ir < pRules->size(); ir++) {
		if (!(*pRules)[ir].bValid) continue;
		RegexRule OneRule;
		if (OneRule.OnePosExec(	&((*pRules)[ir]), BoardSize, 
								(char *)pOldBoard, xStart, yStart)) {
			OneRule.CreateMoveDeltas(Moves);
		}
	}


}

void GamerRegexGetValidMoves(	vector<OneMove >& Moves, int BoardSize, 
								char * pOldBoard, bool bWhiteTurn, 
								int xStart, int yStart,
								int xEnd, int yEnd,
								RuleProviderParent * pProvider)
{
	vector<HeadRegexEl>* pRules = &GamerWhiteMoves;
	if (!bWhiteTurn) {
		pRules = &GamerBlackMoves;
	}
	if (pProvider != NULL) {
		vector<HeadRegexEl>* pGivenRules = ((RegexRuleProvider *)pProvider)->GetRules();
		
		pRules = pGivenRules;
	}
	for (uint ir = 0; ir < pRules->size(); ir++) {
		if (!(*pRules)[ir].bValid) continue;
		RegexRule OneRule;
		const bool cbEndReq = true;
		if (OneRule.OnePosExec(	&((*pRules)[ir]), BoardSize, 
								(char *)pOldBoard, xStart, yStart,
								cbEndReq, xEnd, yEnd)) {
			OneRule.CreateMoveDeltas(Moves);
		}
	}


}

void GamerRegexExecConseq(	int& Score, int BoardSize, char * pOldBoard, 
							bool bWhiteTurn, bool bMoveFinal) 
{
	Score = 0;
	for (int iRow = 0; iRow < BoardSize; iRow++) {
		for (int iCol = 0; iCol < BoardSize; iCol++) {
			for (uint ir = 0; ir < GamerConseqMoves.size(); ir++) {
                vector<OneMove > Moves;
                RegexRule OneRule;
				//OneRule.bDebug = true;
				if (OneRule.OnePosExec(	&(GamerConseqMoves[ir]), BoardSize, 
										(char *)pOldBoard, iCol, iRow)) {
					OneRule.CreateMoveDeltas(Moves);
                    for (int iMove = 0; iMove < Moves.size(); iMove++) {
                        for (uint iFV = 0; iFV < Moves[iMove].Deltas.size(); iFV++) {
                            BoardDelta& rec = Moves[iMove].Deltas[iFV];
							if (bMoveFinal) {
//								cerr << char('0' + pOldBoard[(rec.y * BoardSize)+rec.x]) 
//										<< " at " << rec.x << "," << rec.y 
//										<< " becomes " << char('0' + rec.NewVal) << endl;
							}
                            pOldBoard[(rec.y * BoardSize)+rec.x] = rec.NewVal;
                        }
						Score += Moves[iMove].Score;
                    }
                }
            }
        }
    }
	// Score = 0; Must be a bug
	for (int iRow = 0; iRow < BoardSize; iRow++) {
		for (int iCol = 0; iCol < BoardSize; iCol++) {
			for (uint ir = 0; ir < GamerScoringMoves.size(); ir++) {
                vector<OneMove > Moves;
                RegexRule OneRule;
				//OneRule.bDebug = true;
				if (OneRule.OnePosExec(	&(GamerScoringMoves[ir]), BoardSize, 
										(char *)pOldBoard, iCol, iRow)) {
					OneRule.CreateMoveDeltas(Moves);
                    for (int iMove = 0; iMove < Moves.size(); iMove++) {
                        for (uint iFV = 0; iFV < Moves[iMove].Deltas.size(); iFV++) {
                            BoardDelta& rec = Moves[iMove].Deltas[iFV];
							if (bMoveFinal) {
								cerr << char('0' + pOldBoard[(rec.y * BoardSize)+rec.x]) 
										<< " at " << rec.x << "," << rec.y 
										<< " becomes " << char('0' + rec.NewVal) << endl;
							}
                            pOldBoard[(rec.y * BoardSize)+rec.x] = rec.NewVal;
                        }
						Score += Moves[iMove].Score;
                    }
                }
            }
        }
    }
}
