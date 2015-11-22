// TaskFns.cpp : Parses the output of Standford CoreNLP							
//

#include "stdafx.h"

#include "MascReader.h"

#include <chrono>
#include <iomanip>

using namespace chrono;
typedef std::chrono::high_resolution_clock Clock;

static u64 t0 = TimerNow();

u64 TimerNow()
{
	return Clock::now().time_since_epoch().count() * system_clock::period::num 
			* (u64)1000000 / system_clock::period::den;
}

void CGotitEnv::TimeStamp(string& TSName)
{
	u64 t1 = TimerNow();
	cout << t1 - t0 << " us: ";
	cout << "TimeStamp " << TSName << endl;

}

void CGotitEnv::ExecTaskByName(string TaskName)
{
	STaskList* pTaskList = NULL;
	for (uint iTaskList = 0; iTaskList < TaskListList.size(); iTaskList++) {
		if (TaskListList[iTaskList].Name != TaskName) {
			continue;
		}
		pTaskList = &(TaskListList[iTaskList]);
		break;
	}
	if (pTaskList == NULL) {
		// did it the roudabout way to avoid putting all the code two loops in
		return;
	}

	bool bPassingLoop = false;
	int PassingLoopCount = 0; // When passing by tasks looking for end of loop we keep a count of LoopStarts
	int CurrLoopLevel = 0;

	for (int iTask = 0; iTask < (int)pTaskList->ItemList.size(); iTask++) {
		STaskItem* pTask = &(pTaskList->ItemList[iTask]);

		u64 t1 = TimerNow();
		if (		(t1 < LastMaintenance) 
				||	((t1 - LastMaintenance) > cMaintenancePeriod)) {
			DoMaintenance();
			LastMaintenance = t1;
		}
		//std::cout << t2 - t1 << '\n';

		u64 PrintTime = t1 - t0;
		cout << PrintTime / 1000000ll << "." << setfill('0') << setw(6) << PrintTime % 1000000ll << " s: ";
		cout << "Executing task id #" << iTask << " type = " << pTask->sType << ", do = " << pTask->sDo << ", val = " << pTask->sVal << endl;
		cout << "\tPassingCount = " << PassingLoopCount << " for LoopLevel = " << CurrLoopLevel << endl;
		if (pTask->sType == "SetParam" || pTask->sType == "IncrParam" || pTask->sType == "DecrParam" || pTask->sType == "LoopStart" ||
			pTask->sType == "LoopContinue" || pTask->sType == "LoopBreak" || pTask->sType == "CopyParam" || pTask->sType == "AppendParam") {
			cout << "\t" << pTask->sDo << " = " << ImplemParamTbl[pTask->sDo].Val << endl;
		}
		if (pTask->sType == "CopyParam" || pTask->sType == "AppendParam") {
			cout << "\t\t" << pTask->sVal << " = " << ImplemParamTbl[pTask->sVal].Val << endl;
		}

		if (PassingLoopCount > CurrLoopLevel) {
			if (pTask->sType == "LoopEnd") {
				PassingLoopCount--;
				continue;
			}
			else if (pTask->sType == "LoopStart") {
				PassingLoopCount++;
			}

			if (PassingLoopCount > CurrLoopLevel) {
				continue;
			}
		}

		PassingLoopCount = 0;

		if (pTask->sType == "NoArgsFn") {
			InitTaskFn TaskFn;
			if (getTaskFn(pTask->sDo, TaskFn)) {
				(this->*TaskFn)();
			}
			else {
				cerr << "No task function found by the name " << pTask->sDo << endl;
			}

		}
		else if (pTask->sType == "CallTask") {
			ExecTaskByName(pTask->sDo);
		}
		else if (pTask->sType == "SetParam") {
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				itipt->second.Val = pTask->sVal;
			}
			else {
				SImplemParam Param;
				Param.Val = pTask->sVal;
				Param.Name = pTask->sDo;
				ImplemParamTbl[pTask->sDo] = Param;
				cerr << "Warning. SetParam: Adding parameter name " << pTask->sDo << "\n";
			}
		}
		else if (pTask->sType == "DecrParam") {
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				int IntVal = stoi(itipt->second.Val) - 1;
				itipt->second.Val = to_string(IntVal);
			}
			else {
				cerr << "Error. DecrParam: No parameter name " << pTask->sVal << " found\n";
			}
		}
		else if (pTask->sType == "IncrParam") {
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				int IntVal = stoi(itipt->second.Val) + 1;
				itipt->second.Val = to_string(IntVal);
			}
			else {
				cerr << "Error. DecrParam: No parameter name " << pTask->sVal << " found\n";
			}
		}
		else if (pTask->sType == "CopyParam") {
			string sCopyVal;
			MapNameToImplemParam::iterator itiptFrom = ImplemParamTbl.find(pTask->sVal);
			if (itiptFrom != ImplemParamTbl.end()) {
				sCopyVal = itiptFrom->second.Val;
			}
			else {
				cerr << "Error. CopyParam: No parameter name " << pTask->sVal << " found\n";
			}
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				itipt->second.Val = sCopyVal;
			}
			else {
				SImplemParam Param;
				Param.Val = sCopyVal;
				Param.Name = pTask->sDo;
				ImplemParamTbl[pTask->sDo] = Param;
				cerr << "Warning. CopyParam: Adding parameter name " << pTask->sDo << "\n";
			}
		}
		else if (pTask->sType == "AppendParam") {
			string sCopyVal;
			MapNameToImplemParam::iterator itiptFrom = ImplemParamTbl.find(pTask->sVal);
			if (itiptFrom != ImplemParamTbl.end()) {
				sCopyVal = itiptFrom->second.Val;
			}
			else {
				cerr << "Error. AppendParam: No parameter name " << pTask->sVal << " found\n";
			}
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				itipt->second.Val += ",";
				itipt->second.Val += sCopyVal;
			}
			else {
				cerr << "Error. AppendParam: No parameter name " << pTask->sVal << " found\n";
			}
		}
		else if (pTask->sType == "LoopStart") {
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				int PredVal = stoi(itipt->second.Val);
				if (PredVal == 0) {
					PassingLoopCount = CurrLoopLevel + 1;
				}
				else {
					pTaskList->LoopStarts.push_back(iTask);
					PassingLoopCount = 0;
					CurrLoopLevel++;
				}
			}
			else {
				cerr << "Error. LoopStart: No loop parameter name " << pTask->sDo << " found\n";
			}
		} // end else if LoopStart
		else if (pTask->sType == "LoopEnd") {
			CurrLoopLevel--;
			iTask = pTaskList->LoopStarts.back() - 1;
			pTaskList->LoopStarts.pop_back();
		}
		else if (pTask->sType == "LoopBreak") {
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				int PredVal = stoi(itipt->second.Val);
				if (PredVal == 0) {
					PassingLoopCount++;
					CurrLoopLevel--;
					pTaskList->LoopStarts.pop_back();
				}
			}
			else {
				cerr << "Error. LoopBreak: No loop parameter name " << pTask->sDo << " found\n";
			}
		}
		else if (pTask->sType == "LoopContinue") {
			MapNameToImplemParam::iterator itipt = ImplemParamTbl.find(pTask->sDo);
			if (itipt != ImplemParamTbl.end()) {
				int PredVal = stoi(itipt->second.Val);
				if (PredVal == 0) {
					CurrLoopLevel--;
					iTask = pTaskList->LoopStarts.back() - 1;
					pTaskList->LoopStarts.pop_back();
				}
			}
			else {
				cerr << "Error. LoopBreak: No loop parameter name " << pTask->sDo << " found\n";
			}
		}
	}
}
