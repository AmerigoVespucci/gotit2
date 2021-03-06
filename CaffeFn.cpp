// CaffeFn.cpp : 
// Takes a GenData proto file and produces all the data needed to rnu a GenSeed Caffe NN generation
//


 
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "stdafx.h"
#include <boost/asio.hpp>
#include <glog/logging.h>

#include "GenSeed.pb.h"
#include "GenData.pb.h"
#include "GenDef.pb.h"

#include "/home/eli/dev/caffe/include/caffe/GenData.hpp"
#include "MascReader.h"
#include "H5Cpp.h"

#include "ipc.pb.h"
#include "/home/eli/dev/caffe/include/caffe/util/ipc.hpp"

using boost::asio::ip::tcp;

#ifndef H5_NO_NAMESPACE
    using namespace H5;
#endif


#ifdef  _MSC_VER
#pragma warning(disable : 4503)
#endif


void CGotitEnv::CaffeFnInit() 
{
	CaffeFnDataHandle = NULL;
	CaffeFnOutHandle = NULL;
	
	string sModelProtoName;
	if (!GetImplemParam(sModelProtoName, "Implem.Param.FnParam.CaffeFn.ModelProtoName")) {
		cerr << "CaffeFn cannot be called without setting Implem.Param.FnParam.CaffeFn.ModelProtoName \n";
		return;
	}

	string sTblDefsProtoName;
	if (!GetImplemParam(sTblDefsProtoName, "Implem.Param.FnParam.CaffeFn.TblDefsProtoName")) {
		cerr << "CaffeFn cannot be called without setting Implem.Param.FnParam.CaffeFn.TblDefsProtoName \n";
		return;
	}

	string sPrepFromSrc;
	bool bPrepFromSrc = false;
	if (GetImplemParam(sPrepFromSrc, "Implem.Param.FnParam.CaffeFn.PrepFromSrc")) {
		bPrepFromSrc = true;
	}

	CGenDefTbls * GenTbls = new CGenDefTbls(sTblDefsProtoName);
	if (!GenTbls->bInitDone) {
		cerr << "Failed to initialize tables def file. Terminating.\n";
		return;
	}
	const bool cb_model_owns_tbls = true;

	CGenDef * InitData = new CGenDef(GenTbls, cb_model_owns_tbls);

	if (	!InitData->ModelInit(sModelProtoName)
		||	!InitData->ModelPrep(bPrepFromSrc)) {
		delete InitData;
		return;
	}

	CaffeGenSeed* gen_seed_config = new CaffeGenSeed;
	gen_seed_config->set_num_test_cases(0);

	CaffeGenDef* gen_def = InitData->getGenDef(); 
	const string& CoreDir = gen_def->files_core_dir();
	string H5TrainListFileName = CoreDir + gen_def->train_list_file_name() + "_list.txt";
	string H5TestListFileName = CoreDir + gen_def->test_list_file_name() + "_list.txt";
	// trancate files at init which wil later be appended
	ofstream test_list(H5TestListFileName);
	ofstream train_list(H5TrainListFileName);

	CaffeFnOutHandle = gen_seed_config;
	CaffeFnDataHandle = InitData;
}

void CGotitEnv::CaffeFnComplete()
{
	
	//if (!CaffeFnHandle || !CaffeFnOutHandle) {
	if (!CaffeFnDataHandle || !CaffeFnOutHandle)	 {
		cerr << "CaffeFnComplete can only work if preceeded by a succesful call to CaffeFnInit.\n";
		return;
		
	}
	CGenDef * InitData = (CGenDef * )CaffeFnDataHandle;
	CaffeGenDef* gen_def = InitData->getGenDef(); // (CaffeGenData *)CaffeFnHandle;
	CaffeGenSeed* gen_seed_config  = (CaffeGenSeed*)CaffeFnOutHandle;

	const string& CoreDir = gen_def->files_core_dir();
	string H5TrainListFileName = CoreDir + gen_def->train_list_file_name() + "_list.txt";
	string H5TestListFileName = CoreDir + gen_def->test_list_file_name() + "_list.txt";
	
	ofstream config_ofs(CoreDir + gen_def->config_file_name());
	google::protobuf::io::OstreamOutputStream* config_output 
		= new google::protobuf::io::OstreamOutputStream(&config_ofs);
	//ofstream f_config(ConfigFileName); // I think this one is wrong
	if (config_ofs.is_open()) {
		//CaffeGenSeed config;
		//vector<int>& output_set_num_nodes = InitData->getOutputSetNumNodes();
		gen_seed_config->set_test_list_file_name(H5TestListFileName);
		gen_seed_config->set_train_list_file_name(H5TrainListFileName);
		gen_seed_config->set_model_file_name(CoreDir + gen_def->model_file_name());
		gen_seed_config->set_proto_file_name(CoreDir + gen_def->proto_file_name());
		gen_seed_config->set_num_accuracy_candidates(gen_def->num_accuracy_candidates());
		google::protobuf::TextFormat::Print(*gen_seed_config, config_output);
		delete config_output;
	}

	
	delete gen_seed_config ;
	delete InitData;
	CaffeFnDataHandle = NULL;
	CaffeFnOutHandle = NULL;
}

tcp::socket* IPCServerInit(const char * port_str);

void CGotitEnv::CaffePrepServer()
{
//	string sCaffeHostName;
//	if (!GetImplemParam(sCaffeHostName, "Implem.Param.FnParam.Caffe.HostName")) {
//		cerr << "No Caffe host name provided\n";
//		return;
//	}
	string sPortNum;
	if (!GetImplemParam(sPortNum, "Implem.Param.FnParam.Caffe.PortNum")) {
		cerr << "No Caffe port number provided\n";
		return;
	}

	tcp::socket* ServerSocket = IPCServerInit(sPortNum.c_str());	

	for (;;) {
		CaffeIpc Msg1;
		CaffeIPCRcvMsg(*ServerSocket, Msg1);

		if (Msg1.type() == CaffeIpc::PREP_GEN) {
			std::cerr << "Received name " << Msg1.prep_gen_param().gengen_filename() << std::endl;
			const CaffeIpc::PrepGenParam& param_msg = Msg1.prep_gen_param();
			SImplemParam Param;
			Param.Name = "Implem.Param.FnParam.CaffeFn.ModelProtoName"; 
			Param.Val = param_msg.gengen_filename();
			ImplemParamTbl[Param.Name] = Param;

			Param.Name = "Implem.Param.FnParam.CaffeFn.LoadYourOwnModules"; 
			Param.Val = "yes";
			ImplemParamTbl[Param.Name] = Param;

			Param.Name = "Implem.Param.FnParam.CaffeFn.MinReqDataVecs"; 
			Param.Val = "128";
			ImplemParamTbl[Param.Name] = Param;
			//break;
			CaffeFnInit();
			bool bCaffeRetGood = true;
			for (int i = 0; i < 6; i++) {
				SImplemParam LoopParam;
				LoopParam.Name = "Task.Param.DoCaffeFn.Loop0"; 
				LoopParam.Val = to_string(i);
				ImplemParamTbl[LoopParam.Name] = LoopParam;
				
				//LoadSentenceListOneMod();
				CaffeFn();
				//ClearSentenceRecs();
				string sOK;
				if (GetImplemParam(sOK, "Implem.Param.FnResponse.CaffeFn.OK")) {
					if (!(sOK[0] == 'y' || sOK[0] == 'Y')) {
						bCaffeRetGood = false;
						break;
					}
				}
			}
			CaffeFnComplete();
			
			Param.Name = "Implem.Param.FnParam.CaffeFn.LoadYourOwnModules"; 
			Param.Val = "no";
			ImplemParamTbl[Param.Name] = Param;
			
			if (!bCaffeRetGood) {
				CaffeIpc Msg2;
				Msg2.set_type(CaffeIpc::PREP_GEN_FAILED);
				CaffeIPCSendMsg(*ServerSocket, Msg2);
				continue;
			}
			
			CaffeIpc Msg2;
			Msg2.set_type(CaffeIpc::PREP_GEN_DONE);
			CaffeIPCSendMsg(*ServerSocket, Msg2);
//			try {
//				ServerSocket->shutdown(tcp::socket::shutdown_both);
//				ServerSocket->close();
//			}
//			catch (...) {
//				cerr << "Server socket closed\n";
//			}
		}
	}
}

void CGotitEnv::PrepCauseData()
{
	string sSrcProtoFileName;
	if (!GetImplemParam(sSrcProtoFileName, "Implem.Param.PrepCauseData.SrcProtoName")) {
		cerr << "PrepCauseData requires param Implem.Param.PrepCauseData.SrcProtoName to be set\n";
		return;
	}

	std::cerr << "Src Proto name " << sSrcProtoFileName << std::endl;
	SImplemParam Param;
	Param.Name = "Implem.Param.FnParam.CaffeFn.ModelProtoName"; 
	Param.Val = sSrcProtoFileName;
	ImplemParamTbl[Param.Name] = Param;

	Param.Name = "Implem.Param.FnParam.CaffeFn.LoadYourOwnModules"; 
	Param.Val = "yes";
	ImplemParamTbl[Param.Name] = Param;

	Param.Name = "Implem.Param.FnParam.CaffeFn.MinReqDataVecs"; 
	Param.Val = "128";
	ImplemParamTbl[Param.Name] = Param;
			//break;
	Param.Name = "Implem.Param.FnParam.CaffeFn.PrepFromSrc"; 
	Param.Val = "because";
	ImplemParamTbl[Param.Name] = Param;

	CaffeFnInit();
	bool bCaffeRetGood = true;
	for (int i = 0; i < 60; i++) {
		SImplemParam LoopParam;
		LoopParam.Name = "Task.Param.DoCaffeFn.Loop0"; 
		LoopParam.Val = to_string(i);
		ImplemParamTbl[LoopParam.Name] = LoopParam;

		//LoadSentenceListOneMod();
		CaffeFn();
		//ClearSentenceRecs();
//		string sOK;
//		if (GetImplemParam(sOK, "Implem.Param.FnResponse.CaffeFn.OK")) {
//			if (!(sOK[0] == 'y' || sOK[0] == 'Y')) {
//				bCaffeRetGood = false;
//				break;
//			}
//		}
	}
	CaffeFnComplete();
			
//			try {
//				ServerSocket->shutdown(tcp::socket::shutdown_both);
//				ServerSocket->close();
//			}
//			catch (...) {
//				cerr << "Server socket closed\n";
//			}
}


void CGotitEnv::CaffeFn()
{

	if (!CaffeFnDataHandle || !CaffeFnOutHandle)	 {
		cerr << "CaffeFn can only work if preceeded by a succesful call to CaffeFnInit.\n";
		SImplemParam Param;
		Param.Name = "Implem.Param.FnResponse.CaffeFn.OK"; 
		Param.Val = "no";
		ImplemParamTbl[Param.Name] = Param;
		return;
		
	}

	string sLoadModules = "no";
	bool bLoadModules = false;
	if (GetImplemParam(sLoadModules, "Implem.Param.FnParam.CaffeFn.LoadYourOwnModules")) {
		if (sLoadModules[0] == 'y' || sLoadModules[0] == 'Y') {
			bLoadModules  = true;
		}
	}
	
	bool bKeepGoing = true;
	bool bFirstRun = true;
	const int cMinReqDataVecs = 128;
	int MinReqDataVecs = cMinReqDataVecs;
	string sMinReqDataVecs;
	if (GetImplemParam(sMinReqDataVecs, "Implem.Param.FnParam.CaffeFn.MinReqDataVecs")) {
		MinReqDataVecs = stoi(sMinReqDataVecs);
	}
	
	CaffeGenSeed* gen_seed_config  = (CaffeGenSeed*)CaffeFnOutHandle;	
	CGenDef * InitData = (CGenDef * )CaffeFnDataHandle;
	CaffeGenDef* gen_def = InitData->getGenDef(); // (CaffeGenData *)CaffeFnHandle;
//	vector<map<string, int>*>& TranslateTblPtrs = InitData->TranslateTblPtrs;
	vector<vector<vector<float> >* >& VecTblPtrs = InitData->getVecTblPtrs();
	int NumOutputNodesNeeded = InitData->getNumOutputNodesNeeded();
	vector<DataAvailType> CorefAvail;
	vector<SSentenceRecAvail> SentenceAvailList(0);

	vector<pair<int, int> >& InputTranslateTbl = InitData->getInputTranslateTbl();
	vector<pair<int, int> > OutputTranslateTbl = InitData->getOutputTranslateTbl();

	vector<SDataForVecs > DataForVecs;
	int NumModsLoaded = 0;
	bool bOK = true;
	
	while (bKeepGoing) {
		if (bLoadModules) {
			LoadSentenceListOneMod();
			NumModsLoaded++;
		}

		CGenModelRun GenModelRun(*InitData, SentenceRec, CorefList, 
								SentenceAvailList, CorefAvail, DataForVecs);

		if (!GenModelRun.DoRun(!bFirstRun)) {
			bKeepGoing = false;
			bOK = false;
		}
		if (bLoadModules) {
			if (DataForVecs.size() >= MinReqDataVecs) {
				bKeepGoing = false;
			} 
			else if (NumModsLoaded > MinReqDataVecs) {
				bKeepGoing = false;
				bOK = false;
			}
			ClearSentenceRecs();
		}
		else {
			bKeepGoing = false;
		}

		
		bFirstRun = false;
	}
	
	//vector<SDataForVecs >& DataForVecs = GenModelRun.getDataForVecs();

	if (!bOK) {
		SImplemParam Param;
		Param.Name = "Implem.Param.FnResponse.CaffeFn.OK"; 
		Param.Val = "no";
		ImplemParamTbl[Param.Name] = Param;
		return;
	}
	const string& CoreDir = gen_def->files_core_dir();
	string sConfigLoopNum;
	if (!GetImplemParam(sConfigLoopNum, "Task.Param.DoCaffeFn.Loop0")) {
		cerr << "CaffeFn assumes it is called as part of a loop with loop parameter Task.Param.DoCaffeFn.Loop0 \n";
		return;
	}
	string H5TrainListFileName = CoreDir + gen_def->train_list_file_name() + "_list.txt";
	string H5TestListFileName = CoreDir + gen_def->test_list_file_name() + "_list.txt";
	string H5TrainFileName = CoreDir + gen_def->train_list_file_name() + sConfigLoopNum + ".h5";
	string H5TestFileName = CoreDir + gen_def->test_list_file_name() + sConfigLoopNum + ".h5";
	
	int NumRecords = DataForVecs.size() / 2;
	int NumLabelVals = 0;
	int NumItemsPerRec = 0;
	for (auto itt : InputTranslateTbl) {
		NumItemsPerRec += (*VecTblPtrs[itt.second])[0].size();
	}
	for (auto ott : OutputTranslateTbl) {
		NumLabelVals += (*VecTblPtrs[ott.second])[0].size();
	}

//	for (auto Gram : DataForVecs) {
//		auto WordIVec = get<1>(Gram);
//		for (int wi : WordIVec) {
//			string& w = RevWordMapClean[wi];
//			cout << w << " ";			
//		}
//		cout << "(" << (get<2>(Gram) ? "OK" : "Bad") << ")\n";
//	}
//	cout << "Found " << NumRecords << " records/ngrams\n";

	string H5FileName = H5TrainFileName;
	
	for (int iBoth = 0; iBoth < 2; iBoth++ ) {
		if (iBoth == 1) {
			H5FileName = H5TestFileName;
		}
		H5std_string	DATASET_NAMES; // ("data1");
		const H5std_string	LABELSET_NAME("label");
		const int cDataRank =  2;
		hsize_t dims[cDataRank];               // dataset dimensions
		dims[1] = NumItemsPerRec;
		dims[0] = NumRecords;
		const int cLabelRank = 2;
		hsize_t dimsLabel[cLabelRank];               // dataset dimensions
		dimsLabel[1] = NumLabelVals;
		//dimsLabel[1] = cLSTMBatchSize * 1;
		dimsLabel[0] = NumRecords;

		int DataSetSize = NumRecords * NumItemsPerRec;
		int LabelSetSize = NumRecords * NumLabelVals;

		float * pDataSet;// = new float[DataSetSize];
		float * plabels = new float[LabelSetSize];
		float * ppl = plabels;

		DATASET_NAMES = "data";
		pDataSet = new float[DataSetSize];
		float * ppd = pDataSet;
		for (int ir=0; ir < NumRecords * 2; ir++) {
			if ((ir % 2) == iBoth) {
				auto pIData = &(DataForVecs[ir].IData);
				auto pOData = &(DataForVecs[ir].OData);
				int ii = 0;
				for (auto itt : InputTranslateTbl) {
					vector<float>& vec = (*VecTblPtrs[itt.second])[(*pIData)[ii]];
					for (float v : vec) {
						*ppd++ = v;
					}
					ii++;
				}
				int io = 0;
				for (auto ott : OutputTranslateTbl) {
					vector<float>& vec = (*VecTblPtrs[ott.second])[(*pOData)[io]];
					for (float v : vec) {
						*ppl++ = v;
					}
					io++;
				}
				
			}
		}
		

		H5File h5file(H5FileName, H5F_ACC_TRUNC);
		DataSpace dataspace(cDataRank, dims);

		DataSet dataset = h5file.createDataSet(DATASET_NAMES, PredType::IEEE_F32LE, dataspace);

		dataset.write(pDataSet, PredType::IEEE_F32LE);

		delete pDataSet;

		
//		cerr << Count[0] << " zeroes vs " << Count[1] << " 1s.\n";

		DataSpace labelspace(cLabelRank, dimsLabel);

		DataSet labelset = h5file.createDataSet(LABELSET_NAME, PredType::IEEE_F32LE, labelspace);

		labelset.write(plabels, PredType::IEEE_F32LE);

		delete plabels;

	}
	{

		ofstream test_list(H5TestListFileName, ofstream::app);
		ofstream train_list(H5TrainListFileName,ofstream::app);
//		ofstream test_list(H5TestListFileName, (sConfigLoopNum == "0") ?  ofstream::trunc : ofstream::app);
//		ofstream train_list(H5TrainListFileName, (sConfigLoopNum == "0") ?  ofstream::trunc : ofstream::app);
////		ofstream test_list(H5TestListFileName);
		if (test_list.is_open()) {
			test_list << H5TestFileName << endl;
		}
//		ofstream train_list(H5TrainListFileName);
		if (train_list.is_open()) {
			train_list << H5TrainFileName << endl;
		}
	}
	int TestCases = gen_seed_config->num_test_cases() + NumRecords;
	gen_seed_config->set_num_test_cases(TestCases);
	// the following two repeatedly set. No harm AFAICS
	gen_seed_config->set_num_output_nodes(NumOutputNodesNeeded);
	gen_seed_config->set_net_end_type((CaffeGenSeed::NetEndType)gen_def->net_end_type());

	SImplemParam Param;
	Param.Name = "Implem.Param.FnResponse.CaffeFn.OK"; 
	Param.Val = "yes";
	ImplemParamTbl[Param.Name] = Param;
	

}

