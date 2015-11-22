// NodeConfig.cpp : Inputs the node config file								
//

#include "stdafx.h"

#include "MascReader.h"

bool GetBoolOfAttr(xml_attribute<> *attr)
{
	bool bVal;
	if (attr->value() == NULL) {
		bVal = false;
	}
	else {
		char c0 = attr->value()[0];
		if (c0 == '\0') {
			bVal = false;
		}
		else {
			c0 = tolower(c0);
			if (c0 == 't') {
				bVal = true;
			}
			else {
				bVal = false;
			}
		}
	}

	return bVal;
}

//BasicTypeLists // Named list of strings used as options in GroupNode
void CGotitEnv::ParseConfigFile()
{
	char * ConfigFileBuf = FileToBuf("NodeConfig", ".xml", CODE_SRC);
	xml_document<> xmlConfig;    // character type defaults to char
	xmlConfig.parse<0>(ConfigFileBuf);    // 0 means default parse flags
	vector<NameItem> null_list;
	BasicTypeLists["null"] = null_list;
	xml_node<> *TopNode = xmlConfig.first_node();
	xml_node<> *NLNode = TopNode->first_node("NameLists");
	for (xml_node<> *node = NLNode->first_node("NameListItem"); node; node = node->next_sibling()) {
		string list_name;
		for (xml_attribute<> *attr = node->first_attribute();
			attr; attr = attr->next_attribute()) {
			if (strcmp(attr->name(), "name") == 0) {
				list_name = attr->value();
			}
		}
		BasicTypeLists[list_name] = null_list;
		//xml_node<> *li = node->first_node("li");
		for (xml_node<> *inode = node->first_node("li"); inode; inode = inode->next_sibling()) {
			xml_attribute<> *attr = inode->first_attribute();
			if (strcmp(attr->name(), "item-name") == 0) {
				(BasicTypeLists[list_name]).push_back(NameItem(attr->value()));
			}
		}

	} // End processing fir NameListItem items
	xml_node<> *NodesTopNode = TopNode->first_node("NodeLists");
	for (xml_node<> *node = NodesTopNode->first_node("NodeListItem"); node; node = node->next_sibling()) {
		SPatternFinder pat_finder;
		pat_finder.p_stop_list = NULL;
		pat_finder.create_freq = 0.0f;
		pat_finder.run_prob = 1.0f;
		pat_finder.GrpType = "normal";
		pat_finder.bGrpCreator = false;
		for (xml_attribute<> *attr = node->first_attribute();
			attr; attr = attr->next_attribute()) {
			if (strcmp(attr->name(), "group-name") == 0) {
				pat_finder.group_name_template = attr->value();
			}
			else if (strcmp(attr->name(), "GrpType") == 0) {
				pat_finder.GrpType = attr->value();
			}
			else if (strcmp(attr->name(), "SrcTextType") == 0) {
				pat_finder.SrcTextType = attr->value();
			}
			else if (strcmp(attr->name(), "GrpCreator") == 0) {
				pat_finder.bGrpCreator = GetBoolOfAttr(attr);
			}
			else if (strcmp(attr->name(), "num-stores") == 0) {
				pat_finder.num_stores = atoi(attr->value());
			}
			else if (strcmp(attr->name(), "stop-list") == 0) {
				pat_finder.p_stop_list = &(BasicTypeLists[attr->value()]);
			}
			// the following specifies how often to try to create a
			// new group which is a descendent of other groups
			// for the oneof type (optas=oneof), at most, one is created per 
			// module loaded. This will happen if create-freq is 1
			// otherwise the prob of creating a new gtroup is
			// below 1.
			// if the group already exists the chance is *wasted*
			// once created it will try to add members on every module
			// ie they will all try every module
			// if optas is onetype it will create all groups in a single go
			// it should only do this once at the pre module loaded run
			else if (strcmp(attr->name(), "create-freq") == 0) {
				pat_finder.create_freq = (float)atof(attr->value());
			}
			// all groups once created will try to add members every module
			// the chance of doing so per module is run-freq/1.0
			else if (strcmp(attr->name(), "run-freq") == 0) {
				pat_finder.run_prob = (float)atof(attr->value());
			}
		} // end attribute iteration
		if (pat_finder.p_stop_list == NULL) {
			pat_finder.p_stop_list = &(BasicTypeLists["null"]);
		}
		for (	xml_node<> *gnode = node->first_node("GrpParam"); gnode;
				gnode = gnode->next_sibling("GrpParam")) {
			SOnePatternFinderGrpParam GrpParam;
			GrpParam.sSrcType = "const";
			GrpParam.sSortType = "none";
			GrpParam.sFillType = "all";
			GrpParam.sFillNumber = "100";
			GrpParam.sFillPercentile = "100";
			GrpParam.sMatchWhat = "none";
			GrpParam.sMatchWhich = "0";
			GrpParam.sMatchType = "none";
			GrpParam.sMatch = "err";
			GrpParam.sSrcStore = "word";
			GrpParam.sSrcStoreWhich = "0"; // currently aapplicable to SrcStore
			for (xml_attribute<> *attr = gnode->first_attribute();
					attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "idx") == 0) {
					GrpParam.idx = atoi(attr->value());
				}
				else if (strcmp(attr->name(), "SrcType") == 0) {
					GrpParam.sSrcType = attr->value();
				}
				else if (strcmp(attr->name(), "Src") == 0) {
					GrpParam.sSrc = attr->value();
				}
				else if (strcmp(attr->name(), "SrcStore") == 0) {
					GrpParam.sSrcStore = attr->value();
				}
				else if (strcmp(attr->name(), "SrcStoreWhich") == 0) {
					GrpParam.sSrcStoreWhich = attr->value();
				}
				else if (strcmp(attr->name(), "SortType") == 0) {
					GrpParam.sSortType = attr->value();
				}
				else if (strcmp(attr->name(), "FillType") == 0) {
					GrpParam.sFillType = attr->value();
				}
				else if (strcmp(attr->name(), "FillNumber") == 0) {
					GrpParam.sFillNumber = attr->value();
				}
				else if (strcmp(attr->name(), "FillPercentile") == 0) {
					GrpParam.sFillPercentile = attr->value();
				}
				else if (strcmp(attr->name(), "MatchWhat") == 0) {
					GrpParam.sMatchWhat = attr->value();
				}
				else if (strcmp(attr->name(), "MatchWhich") == 0) {
					GrpParam.sMatchWhich = attr->value();
				}
				else if (strcmp(attr->name(), "MatchType") == 0) {
					GrpParam.sMatchType = attr->value();
				}
				else if (strcmp(attr->name(), "Match") == 0) {
					GrpParam.sMatch = attr->value();
				}
			}
			for (xml_node<> *gpmnode = gnode->first_node("GrpParamMatchNode"); gpmnode;
						gpmnode = gpmnode->next_sibling("GrpParamMatchNode")) {
				SPatFinderGrpParamMatch MatchData;
				MatchData.sMatchWhat = "const";
				MatchData.sMatchWhich = "0";
				MatchData.sMatchType = "const";
				MatchData.sMatch = "0";

				for (xml_attribute<> *attr = gpmnode->first_attribute();
									attr; attr = attr->next_attribute()) {
					if (strcmp(attr->name(), "MatchWhat") == 0) {
						MatchData.sMatchWhat = attr->value();
					}
					else if (strcmp(attr->name(), "MatchWhich") == 0) {
						MatchData.sMatchWhich = attr->value();
					}
					else if (strcmp(attr->name(), "MatchType") == 0) {
						MatchData.sMatchType = attr->value();
					}
					else if (strcmp(attr->name(), "Match") == 0) {
						MatchData.sMatch = attr->value();
					}
				}
				GrpParam.MatchList.push_back(MatchData);
			}
			pat_finder.GrpParamList.push_back(GrpParam);
		} // End of GrpParam Node
		for (xml_node<> *sqnode = node->first_node("SurpriseQ"); sqnode;
			sqnode = sqnode->next_sibling("SurpriseQ")) {
			SOnePatternFinderSurpriseQ SrpsGrpFinder;
			SrpsGrpFinder.sGrpAnc = "";
			SrpsGrpFinder.sGrpParamMatchType0 = "const";
			SrpsGrpFinder.sGrpParamMatchArg0 = "0";
			SrpsGrpFinder.sGrpParamMatchType1 = "none";
			SrpsGrpFinder.sGrpParamMatchArg1 = "0";
			for (xml_attribute<> *attr = sqnode->first_attribute();
				attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "GrpAnc") == 0) {
					SrpsGrpFinder.sGrpAnc = (attr->value());
				}
				if (strcmp(attr->name(), "GrpParamMatchType0") == 0) {
					SrpsGrpFinder.sGrpParamMatchType0 = (attr->value());
				}
				if (strcmp(attr->name(), "GrpParamMatchArg0") == 0) {
					SrpsGrpFinder.sGrpParamMatchArg0 = (attr->value());
				}
				if (strcmp(attr->name(), "GrpParamMatchType1") == 0) {
					SrpsGrpFinder.sGrpParamMatchType1 = (attr->value());
				}
				if (strcmp(attr->name(), "GrpParamMatchArg1") == 0) {
					SrpsGrpFinder.sGrpParamMatchArg1 = (attr->value());
				}
			} // end loop over attrs
			pat_finder.SurpriseGrpFinder = SrpsGrpFinder; // Watch out! We're not doing a list of finders here
		} // End of SurpriseQ node
		int NumMemUs = 0;
		int NumStoreUs = 0;
		for (	xml_node<> *nnode = node->first_node("node"); nnode;
				nnode = nnode->next_sibling("node")) {
			xml_attribute<> *attr = nnode->first_attribute();
			SOnePatternFinderNode OneNode;
			OneNode.sat = satIncludeAndStop;
			OneNode.pOptions = NULL;
			OneNode.Sel = etWord;
			OneNode.StoreIdx = -1;
			OneNode.NodeIdx = -1;
			OneNode.Store = etWord;
			OneNode.sSel = "none";
			OneNode.sReq = "none";
			OneNode.sReqType = "none";
			OneNode.sSelType = "none";
			OneNode.sStore = "none";
			OneNode.sStoreType = "none";
			OneNode.b_oneof = false;
			OneNode.bOneItem = false;
			OneNode.pOptions = NULL;
			for	(	xml_attribute<> *attr = nnode->first_attribute();
					attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "options") == 0) {
					OneNode.pOptions = &(BasicTypeLists[attr->value()]);
				}
				else if (strcmp(attr->name(), "Req") == 0) {
					OneNode.sReq = attr->value();
				}
				else if (strcmp(attr->name(), "ReqType") == 0) {
					OneNode.sReqType = attr->value();
				}
				else if (strcmp(attr->name(), "SelType") == 0) {
					OneNode.sSelType = attr->value();
				}
				else if (strcmp(attr->name(), "ancestor") == 0) {
					OneNode.anc_grp_name = attr->value();
				}
				else if (strcmp(attr->name(), "sel") == 0) {
					OneNode.Sel = itoet(attr->value());
					OneNode.sSel = attr->value();
				}
				else if (strcmp(attr->name(), "optas") == 0) {
					// oneof creates new groups by having one of the
					// nodes use another groups members. Each member can cause the
					// generation of a new child group
					if (strcmp(attr->value(), "oneof") == 0) {
						OneNode.b_oneof = true;
					}
					// oneitem creates a one-time generation of as many groups
					// as there are items in a name list pointed to by the options parameter
					if (strcmp(attr->value(), "oneitem") == 0) {
						OneNode.bOneItem = true;
					}
					//sel = itoet(attr->value());
				}
				else if (strcmp(attr->name(), "stop-action") == 0) {
					if (strcmp(attr->value(), "include-and-stop") == 0) {
						OneNode.sat = satIncludeAndStop;
					}
					else if (strcmp(attr->value(), "no stop") == 0) {
						OneNode.sat = satNoStop;
					}
					else  {
						OneNode.sat = satStop;
					}
				}
				else if (strcmp(attr->name(), "store") == 0) {
					OneNode.Store = itoet(attr->value());
					OneNode.sStore = attr->value();
				}
				else if (strcmp(attr->name(), "StoreType") == 0) {
					OneNode.sStoreType = attr->value();
				}
				else if (strcmp(attr->name(), "store-idx") == 0) {
					OneNode.StoreIdx = atoi(attr->value());
					if (OneNode.StoreIdx >= NumStoreUs) {
						NumStoreUs = OneNode.StoreIdx + 1;
					}
				}
				else if (strcmp(attr->name(), "node-idx") == 0) {
					OneNode.NodeIdx = atoi(attr->value());
					if (OneNode.NodeIdx >= NumMemUs) {
						NumMemUs = OneNode.NodeIdx + 1;
					}
				}
			} // end attribute iteration
			pat_finder.node_list.push_back(OneNode);
		} // end loop over "node" nodes
		pat_finder.NumLocalMemUs = NumMemUs;
		pat_finder.NumStoreMemUs = NumStoreUs;
		pat_finder.NumTotalMemUs = NumMemUs + NumStoreUs;
		anc_pat_finder_list.push_back(pat_finder);
	} // End processing for NodeListItem
	xml_node<> *TaskListsTopNode = TopNode->first_node("TaskLists");
	
	for (xml_node<> *TaskList = TaskListsTopNode->first_node("TaskNode"); TaskList;
					TaskList = TaskList->next_sibling("TaskNode")) {
		TaskListList.push_back(STaskList());
		for (xml_attribute<> *attr = TaskList->first_attribute();
							attr; attr = attr->next_attribute()) {
			if (strcmp(attr->name(), "name") == 0) {
				TaskListList.back().Name = attr->value();
			}
		}
		for (xml_node<> *item = TaskList->first_node("TaskItem"); item;
			item = item->next_sibling("TaskItem")) {
			TaskListList.back().ItemList.push_back(STaskItem());
			STaskItem & CurTask = TaskListList.back().ItemList.back();
			for (xml_attribute<> *attr = item->first_attribute();
								attr; attr = attr->next_attribute()) {
				if (strcmp(attr->name(), "do") == 0) {
					CurTask.sDo = attr->value();
				}
				else if (strcmp(attr->name(), "TaskType") == 0) {
					CurTask.sType = attr->value();
				}
				else if (strcmp(attr->name(), "val") == 0) {
					CurTask.sVal = attr->value();
				}
			}
		}
	}
	xml_node<> *IParamsTopNode = TopNode->first_node("ImplementationParameterList");
	for (xml_node<> *node = IParamsTopNode->first_node("ImplementationParameter"); node; node = node->next_sibling()) {
		SImplemParam Param;
		for (xml_attribute<> *attr = node->first_attribute();
			attr; attr = attr->next_attribute()) {
			if (strcmp(attr->name(), "name") == 0) {
				Param.Name = attr->value();
			}
			else if (strcmp(attr->name(), "type") == 0) {
				if (strcmp(attr->value(), "constant") == 0) {
					Param.ipt = iptConstant;
				}
				else if (strcmp(attr->value(), "options") == 0) {
					Param.ipt = iptOptions;
				}
				else if (strcmp(attr->value(), "range") == 0) {
					Param.ipt = iptRange;
				}
			}
			if (strcmp(attr->name(), "val") == 0) {
				Param.Val = attr->value();
			}
		}
		ImplemParamTbl[Param.Name] = Param;
	}
	delete[] ConfigFileBuf;
}