// OMA.cpp : 
// process the mod file content for OMA
//


 
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>
#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include "boost/variant.hpp"
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

enum NlpieType {
    ntInvalid,
    ntStringVal,
    ntDepRel,
    ntIntVal,
    ntFloatVal,
    ntTriple,
    ntAddStart,
    ntAddCont,
    ntAddEnd,
    ntLeftNull,
    ntVar, // not a DB n type. Holds bounds variables in search/match
    ntEqual, // not a DB n type but used for match
    ntNode, // could be a DB n type, but currently used for matching
    ntExtract, // get a value from a node, such as its string val
    ntIsHypernym, // test a condition against a WordNet lookup
};

enum NlpieVarType {
    nvtInvalid,
    nvtString,
    nvtNode,
};

typedef boost::variant <int , float , string, pair<int, int> > NlpieVal;
//typedef boost::variant <int , float , string > NlpieVal;
struct NlpieNode {
    NlpieNode() : nt(ntInvalid), val(-1) {}
    NlpieNode(NlpieType ant, NlpieVal aval) : nt(ant) , val(aval) 
    {}
    string Print() {
        string TypeName;
        string ValString;
        bool bInt = false;
        
        switch (nt) {
            case ntInvalid:
                TypeName = "ntInvalid";
                bInt = true;
                break;
            case ntStringVal:
                TypeName = "ntStringVal";
                ValString = get<string>(val);
                break;
            case ntDepRel:
                TypeName = "ntDepRel";
                ValString = get<string>(val);
                break;
            case ntIntVal:
                TypeName = "ntIntVal";
                bInt = true;
                break;
            case ntFloatVal:
                TypeName = "ntFloatVal";
                ValString = to_string(get<float>(val));
                break;
            case ntTriple:
                TypeName = "ntTriple";
                bInt = true;
                break;
            case ntAddStart:
                TypeName = "ntAddStart";
                bInt = true;
                break;
            case ntAddCont:
                TypeName = "ntAddCont";
                bInt = true;
                break;
            case ntAddEnd:
                TypeName = "ntAddEnd";
                bInt = true;
                break;
            case ntLeftNull:
                TypeName = "ntLeftNull";
                bInt = true;
                break;
            case ntVar:
                TypeName = "ntVar";
                bInt = true;
                break;
        }
        if (bInt) {
            ValString = to_string(get<int>(val));
        }
        return  "{ " + TypeName + ": " + ValString + "} ";
        //return RetString;
    }
    
    NlpieType nt;
    NlpieVal val;
    //string sVal;
};
//typedef tuple<NlpieNode, NlpieNode, NlpieNode> NlpieTriple;string RetString =
struct NlpieTriple {
    NlpieTriple() : head(ntInvalid, -1), left(ntInvalid, -1), right(ntInvalid, -1) {}
    NlpieTriple(NlpieNode ahead, NlpieNode aleft, NlpieNode aright) {
        head = ahead; left = aleft; right = aright;
    }
    NlpieNode head;
    NlpieNode left;
    NlpieNode right;
    void Print() {
        cerr << head.Print() << left.Print() << right.Print() << endl;
    }
};

struct NlpieVar {
    NlpieVar() : nvt(nvtInvalid), val(-1) {
        VarName = "";
        bBound = false;
    }
    NlpieVar(string aVarName) : nvt(nvtNode), val(-1) {
        VarName = aVarName;
        bBound = false;
    }
    NlpieVar(string aVarName, NlpieVarType anvt) : nvt(anvt), val(-1) {
        VarName = aVarName;
        bBound = false;
    }
    NlpieVar(string aVarName, NlpieVarType anvt, int aval) : nvt(anvt), val(aval) {
        VarName = aVarName;
        bBound = true;
    }
    NlpieVar(string aVarName, NlpieVarType anvt, string aval) : nvt(anvt), val(aval) {
        VarName = aVarName;
        bBound = true;
    }
    NlpieVar(string aVarName, NlpieVarType anvt, pair<int, int> aval) : nvt(anvt), val(aval) {
        VarName = aVarName;
        bBound = true;
    }
    string VarName;
    NlpieVarType nvt; 
    NlpieVal val;
    bool bBound;
};
struct NlpieTestEl {
    NlpieTestEl(bool abMust, vector<NlpieTriple> aTripleVec) {
        bMust = abMust;
        TestTripleVec = aTripleVec;
    }
    bool bMust; /// as opposed to must not
    vector<NlpieTriple> TestTripleVec;
    
};

struct NlpieTest;
struct NlpieTestImport {
    vector<pair<string, string> > VarMap;
    string ImportTestName;
    
};
struct NlpieTestImportHybrid {
    NlpieTestImportHybrid(vector<pair<string, string> > aVarMap, string aImportTestName) {
        VarMap = aVarMap;
        ImportTestName = aImportTestName;
        bImport = true;
    }
    NlpieTestImportHybrid(bool abMust, vector<NlpieTriple> aTripleVec) {
        bMust = abMust;
        TestTripleVec = aTripleVec;
        bImport = false;
    }
    bool bMust; /// as opposed to must not
    vector<NlpieTriple> TestTripleVec;
    vector<pair<string, string> > VarMap;
    string ImportTestName;
    bool bImport;
};
struct NlpieTest {
    vector<NlpieVar> VarTblSrc;
    vector<NlpieVar> VarTbl; // current working VarTbl 
    list<vector<NlpieVar> > VarTblStack;
    list<vector<NlpieVar> > VarTblStackNew;
    //vector<NlpieTestEl> TestTbl; 
    vector<NlpieTestImportHybrid> TestsAndImportsList;
    string TestName;
    CGotitEnv * pTheEnv;
//    NlpieTest(vector<NlpieVar> aVarTbl, vector<NlpieTestEl> aTestTbl) {
//        VarTblSrc = aVarTbl;
//        TestTbl = aTestTbl;
//        
//    }
    NlpieTest(string aTestName, vector<NlpieVar> aVarTbl, vector<NlpieTestImportHybrid> TestImportsHybrids) {
        TestName = aTestName;
        VarTblSrc = aVarTbl;
        TestsAndImportsList = TestImportsHybrids;
        pTheEnv = NULL;
        //TestTbl = aTestTbl;
        
    }
    static bool GetVarIndex(int& iVar, string& VarName, vector<NlpieVar>& VarTbl);
    // Run the query against the TripleTbl (currently the sentence)
    // If the test passes, returns true and creates a stack of possible matches
    // as a list of VarTbl instances in VarTblStck
    bool DoTest();
    bool BindStringVar(string VarName, string ConstVal);
private:
    // helper functions
    // test the triple off an add node and its descendents for a specific
    // dntDepRel that the search triple is looking for
    // fail without error if the dep rel just isn't there
    // otherwise a fail is a badly formed tree
    bool TestTripleOffAdd(  int iTripleOffAdd, NlpieTriple& trSearch,
                            int iUnboundVar, bool& bFailWithErr);
    // find the string on the right of a ntDepRel. A failure is possible 
    // only if the database tree is badly formed
    bool FindStringOffDepRel(   int iTripleOffAdd, NlpieTriple& trSearch,
                                int iUnboundVar);
    // search for the other side of the dep when the bound node of the search 
    // is on the left. e.g. looking for the nsubj of hit
    bool TestLeftDep(   NlpieTriple& trDB, NlpieTriple& trSearch,
                        int NodeID, int bBoundNodeOnLeft,
                        int iUnboundVar, bool& bFailWithErr);
    // search for the other side of the dep when the bound node of the search 
    // is on the right. e.g. looking for the gov for which the dep of the dobj is ball
    bool TestRightDep(   NlpieTriple& trDB, NlpieTriple& trSearch,
                        int NodeID, int bBoundNodeOnLeft,
                        int iUnboundVar, bool& bFailWithErr);
    bool FindGovOnAddLadder(NlpieTriple& trDB, int iBoundNode, 
                            int iUnboundVar, bool& bFailWithErr) ;
};


// there exists some x where nsubj(hit, x)
vector<NlpieTest> AllTests = {
    NlpieTest ( 
            "BindSubjObj",
            {NlpieVar("x"), NlpieVar("y"), NlpieVar("z")}, 
            {   
                NlpieTestImportHybrid(
                        true, 
                        {   
                            NlpieTriple(NlpieNode(ntDepRel, "nsubj"), 
                                        NlpieNode(ntVar, "z"), 
                                        NlpieNode(ntVar, "x")),
                            NlpieTriple(NlpieNode(ntDepRel, "dobj"), 
                                        NlpieNode(ntVar, "z"), 
                                        NlpieNode(ntVar, "y"))
                        }
                )
            }
    ),
    NlpieTest ( 
            "VerbGet",
            {NlpieVar("vname", nvtString), NlpieVar("x"), NlpieVar("y"), NlpieVar("z"), NlpieVar("t"), NlpieVar("s"), NlpieVar("r")}, 
            {   
                NlpieTestImportHybrid(
                        true, 
                        {   
                            NlpieTriple(NlpieNode(ntEqual, "string"), 
                                        NlpieNode(ntVar, "z"), 
                                        NlpieNode(ntVar, "vname")),
                        }
                ),
                NlpieTestImportHybrid(
                        false, 
                        {   
                            NlpieTriple(NlpieNode(ntDepRel, "ccomp"), 
                                        NlpieNode(ntVar, "r"), 
                                        NlpieNode(ntVar, "z"))
                        }
                ),
                NlpieTestImportHybrid   (
                        {{"z", "z"}, {"x", "x"}, {"y", "y"}},
                        "BindSubjObj"
                ),
                NlpieTestImportHybrid(
                        false, 
                        {   
                            NlpieTriple(NlpieNode(ntDepRel, "neg"), 
                                        NlpieNode(ntVar, "y"), 
                                        NlpieNode(ntVar, "t"))
                        }
                ),
                NlpieTestImportHybrid(
                        false, 
                        {   
                            NlpieTriple(NlpieNode(ntDepRel, "neg"), 
                                        NlpieNode(ntVar, "z"), 
                                        NlpieNode(ntVar, "s"))
                        }
                )
            }
    ),
    NlpieTest (
            "HTest",
            {
                NlpieVar("subj"), NlpieVar("obj"), NlpieVar("ObjName", nvtString)
            }, 
            {
                NlpieTestImportHybrid   (
                        // for now, binding to a const , as opposed to a var, done by referring to a name not in the var table
                        {{"vname", "hit"}, {"x", "subj"}, {"y", "obj"}},
                        "VerbGet"
                ),
                NlpieTestImportHybrid(
                        true, 
                        {   
                            NlpieTriple(NlpieNode(ntExtract, "string"), 
                                        NlpieNode(ntVar, "obj"), 
                                        NlpieNode(ntVar, "ObjName")),
                            NlpieTriple(NlpieNode(ntIsHypernym, "n"), 
                                        NlpieNode(ntVar, "ObjName"), 
                                        NlpieNode(ntStringVal, "person"))
                        }
                )
            }
    )

};

map<string, int> AllTestMap; // map for AllTests, filled in at the start of OMA()

#define NODE_HEAD   0
#define NODE_LEFT   1
#define NODE_RIGHT  2

vector<NlpieTriple> TripleTbl;
void TripleTblPrint() {
    cerr << "Printing TripleTbl with " << TripleTbl.size() << " els\n";
    int itt = 0;
    for (auto t : TripleTbl) {
        cerr << itt++ << ": ";
        t.Print();
    }
}
// returns an index of the triple that holds the input triple index -- essentially a back-pointer
int FindTripleIdx(int iTriple, bool& bTripleOnLeft) {
    int itt = -1;
    for (auto t : TripleTbl) {
        itt++;
        if (t.left.nt == ntTriple && get<int>(t.left.val) == iTriple) {
            bTripleOnLeft = true;
            return itt;
        }
        if (t.right.nt == ntTriple && get<int>(t.right.val) == iTriple) {
            bTripleOnLeft = false;
            return itt;
        }
    }
    return -1;
}
NlpieNode * getNode(NlpieVal NodeID) {
    pair<int, int> NodePair = get<pair<int, int> >(NodeID);
    int iTDB = NodePair.first;
    int iWhich = NodePair.second;
    NlpieTriple * pTr = &(TripleTbl[iTDB]);
    switch (iWhich) {
        case NODE_HEAD:
            return &(pTr->head);
        case NODE_LEFT:
            return &(pTr->left);
        case NODE_RIGHT:
            return &(pTr->right);
    }
}


bool NlpieTest::FindGovOnAddLadder( NlpieTriple& trDB, int iBoundNode, 
                                    int iUnboundVar, bool& bFailWithErr) {
    string UnboundVarName = VarTbl[iUnboundVar].VarName;
    if (trDB.left.nt != ntLeftNull) {
        cerr << "Badly formed TripleDB. Left of dep rel is either "
                "the gov or an ntLeftNull and there is a Add tree to climb.\n";
        bFailWithErr = true;
        return false;                    
    }
    bool bAddStartFound = false;
    int iSearchingNode = iBoundNode;
    while (!bAddStartFound) {
        bool bTripleTblIdxFoundOnLeft;
        int iParentTriple = FindTripleIdx(iSearchingNode, bTripleTblIdxFoundOnLeft);
        if (iParentTriple < 0) {
            cerr << "Badly formed TripleDB. Every triple but the root must "
                    "have a pointer, and you may not search for root.\n";
            bFailWithErr = true;
            return false;                    
        }
        NlpieTriple& trAddStartCand = TripleTbl[iParentTriple];
        if (trAddStartCand.head.nt == ntAddStart) {
            string sToBind = get<string>(trAddStartCand.left.val);
            cerr    << "Test el passed! var " << UnboundVarName 
                    << " bound to " << sToBind  << "\n";

            NlpieVar BoundVal(  UnboundVarName, nvtNode, 
                                make_pair(iParentTriple, NODE_LEFT));
            vector<NlpieVar> NewVarTbl = VarTbl; 
            NewVarTbl[iUnboundVar] = BoundVal;
            VarTblStackNew.push_back(NewVarTbl);
            //VarTbl[iUnboundVar] = BoundVal;
            return true;

        }
        if (    (trAddStartCand.head.nt != ntAddEnd) 
            &&  (trAddStartCand.head.nt != ntAddCont))  {
            cerr << "Badly formed TripleDB. Going up an Add tree, "
                    "there can only be End, Cont and Start.\n";
            bFailWithErr = true;
            return false;                    
        }
        iSearchingNode = iParentTriple;
    }
    
    bFailWithErr = true;
    return false;
    
}

// Tests a search El where the unknown is on the left, ie looking for a gov

bool NlpieTest::TestRightDep(  NlpieTriple& trDB, NlpieTriple& trSearch,
        int iBoundTriple, int bBoundNodeOnLeft,
        int iUnboundVar, bool& bFailWithErr) {
    string UnboundVarName = VarTbl[iUnboundVar].VarName;
    int iGovsDepRepTriple;
    
    // We know we have a dep, and we are looking for its gov
    // First we have to find out if the dep is a gov in other relationships
    // if not, then the word will be an ntString node with no tree below it
    if (!bBoundNodeOnLeft) {
        // if it's on the right it has no tree of its own
        iGovsDepRepTriple = iBoundTriple;
    }
    else {
        // the bound db is on the left. That means that it has its own tree.
        // If a node has its own tree of dep rels in which it is a gov
        // above it there will be a ntDepRel or an ntAddStart
        if ((trDB.head.nt != ntDepRel) && (trDB.head.nt != ntAddStart)) {
            cerr << "Badly formed TripleDB. The node above the left-side dep "
                    "should be a dep rel.\n";
            bFailWithErr = true;
            return false;                                    
        }
        bool bTripleTblIdxFoundOnLeft;
        int iParentTriple = FindTripleIdx(iBoundTriple, bTripleTblIdxFoundOnLeft);
        iGovsDepRepTriple = iParentTriple;
    }
    NlpieTriple& trDepRel = TripleTbl[iGovsDepRepTriple];
    if (trDepRel.head.nt != ntDepRel) {
        cerr << "Badly formed TripleDB. The node above the tree of a dep  "
                "(whether it is just a string or a tree) should be a dep rel.\n";
        bFailWithErr = true;
        return false;                                    
    }
    //string sDepRelGoverning = get<string>(trDB.head.val);
    // we assume that if this function has been called, the search el
    // has a DepRel head and now is the test for the matching dep rel
    if (!(trDepRel.head.val == trSearch.head.val)) { // no != ???!
        // no error, but this is not a match
        return false;
    }
    // we now know that the bound db node is the dep of the correct dep rel
    // we just need to step up a ladder to find the gov
    if (trDepRel.left.nt == ntStringVal) {
        // test el scored. The other side is gov and he has only one dep rel
        string sToBind = get<string>(trDepRel.left.val);
        cerr    << "Test el passed! var " << UnboundVarName 
                << " bound to " << sToBind  << "\n";

        NlpieVar BoundVal(  UnboundVarName, nvtNode, 
                            make_pair(iGovsDepRepTriple, NODE_LEFT));
        vector<NlpieVar> NewVarTbl = VarTbl; 
        NewVarTbl[iUnboundVar] = BoundVal;
        VarTblStackNew.push_back(NewVarTbl);
//        VarTbl[iUnboundVar] = BoundVal;
        return true;
    }
    else {
        return (FindGovOnAddLadder( trDepRel, iGovsDepRepTriple, 
                                    iUnboundVar, bFailWithErr));
    }
            

    // if this dep is the go in other relationships, we need to get to the top of the tree
    return false;
}

bool NlpieTest::TestLeftDep(  NlpieTriple& trDB, NlpieTriple& trSearch,
        int iBoundNode, int bBoundNodeOnLeft,
        int iUnboundVar, bool& bFailWithErr) {
    string UnboundVarName = VarTbl[iUnboundVar].VarName;
    // two basic scenarios. In one the dep rel we are looking for is the head
    // this will occur if the gov has only one dep rel
    // the target could be a string on the right or its own tree
    // that tree in turn could be headed by a dep rel or an AND
    // In the other scenario, the gov has many rels and the head of the
    // node bound in the DB is an AND
    // Scenario 1:
    if (        trDB.head.nt == ntDepRel 
            &&  trSearch.head.val == trDB.head.val) {
        NlpieType ntForString = trDB.right.nt;
        if (!bBoundNodeOnLeft) {
            ntForString = trDB.left.nt;
        }
        if (ntForString == ntStringVal ) {
            string sToBind = (bBoundNodeOnLeft ? get<string>(trDB.right.val) 
                    : get<string>(trDB.right.val));
            cerr    << "Test el passed! var " << UnboundVarName 
                    << " bound to " << sToBind  << "\n";

            NlpieVar BoundVal(  UnboundVarName, nvtNode, 
                                make_pair(iBoundNode, 
                                (bBoundNodeOnLeft ? NODE_RIGHT : NODE_LEFT))); 
            vector<NlpieVar> NewVarTbl = VarTbl; 
            NewVarTbl[iUnboundVar] = BoundVal;
            VarTblStackNew.push_back(NewVarTbl);
//            VarTbl[iUnboundVar] = BoundVal;
            //bAllTestElsPassedSoFar = true;
            return true;
        }
        if (!bBoundNodeOnLeft) {
            cerr << "Test will fail because code not written for this situation.\n";
            bFailWithErr = true;
            return false; // deal with later
        }
        if (trDB.right.nt != ntTriple) {
            cerr << "Badly formed TripleDB. Right of triple containing "
                    "bound node must be the string itself or a triple index.\n";
            bFailWithErr = true;
            return false;                    
        }
        // there is only one dep rel and the dep is a triple but we don't know if it
        // in turn has only one dep rel of its own or a string of them
        if (FindStringOffDepRel(   get<int>(trDB.right.val), trSearch,
                                        iUnboundVar)) {
            //bAllTestElsPassedSoFar = true;
            return true;
        }
        return false;

    }
    // Scenario 2:. This gov ( the node bound in the DB) has many dep rels)
    // The head is a start and the right must be a tree
    if (    trDB.head.nt == ntAddStart  ) {
        // we set up to loop through the ntAddStart, ntAddCond ntAddEnd series
        if (trDB.right.nt != ntTriple) {
            cerr << "Badly formed TripleDB. Right of AddStart is not a triple.\n";
            bFailWithErr = true;
            return false;
        }
        NlpieTriple* ptrCurr = &(TripleTbl[get<int>(trDB.right.val)]);

        // the following loop steps down a ntAddStart.. ntAddCont.. ntEnd
        // line till it either finds the string on the triple of the Add
        // node or hits an ntEnd without this.
        while (true) {
            if (    (ptrCurr->head.nt != ntAddCont) 
                &&  (ptrCurr->head.nt != ntAddEnd)) {
                cerr << "Badly formed TripleDB. "
                        "Add Node not followed by "
                        "AddX head on the right\n";
                bFailWithErr = true;
                return false;
            }
            if (    (ptrCurr->left.nt != ntTriple) 
                || (ptrCurr->right.nt != ntTriple)) {
                cerr << "Badly formed TripleDB. "
                        "Add Cont or End Node must "
                        "have triples on both left and right\n";
                bFailWithErr = true;
                return false;
            }
            if (TestTripleOffAdd(   get<int>(ptrCurr->left.val), 
                                    trSearch,
                                    iUnboundVar, bFailWithErr)) {
                //bAllTestElsPassedSoFar = true;
                return true;
            }
            if (bFailWithErr) return false;

            if (ptrCurr->head.nt == ntAddEnd) {
                // an ntAddEnd, as opposed to an ntAddCont,
                // will also have ab OffAdd triple on its right
                if (TestTripleOffAdd(   get<int>(ptrCurr->right.val), 
                                        trSearch,
                                        iUnboundVar, bFailWithErr)) {
                    return true;
                }
                return false; // if bAllTestElsPassedSoFar not set by prev if, test will fail
            }
            // if we git here, the node was a ntAddCont, the string has not been found 
            // but we can keep looping around to try some more Off Add triples.
            ptrCurr = &(TripleTbl[get<int>(ptrCurr->right.val)]);
        }
    }
    return false;
}

bool NlpieTest::FindStringOffDepRel(    int iTripleOnTop, NlpieTriple& trSearch,
                                        int iUnboundVar) {
    string UnboundVarName = VarTbl[iUnboundVar].VarName;
    NlpieTriple& TripleOnTop 
            = TripleTbl[iTripleOnTop];
    
    if (    (TripleOnTop.head.nt != ntDepRel) 
        &&  (TripleOnTop.head.nt != ntAddStart)) {
        cerr << "Badly formed TripleDB. Triple on right "
                "of ntDepRel must have a head of either "
                "ntDepRel or ntAddStart\n";
       return false;                       
    }

    if (TripleOnTop.left.nt != ntStringVal) {
        cerr << "Badly formed TripleDB. Node on right of "
                "(ntDepRel or ntAddStart) off of ntAddCont "
                "or ntAddEnd must be a ntStringVal\n";
        return false;                       
    }

    cerr    << "Test el passed! var " 
            << UnboundVarName 
            << " bound to " 
            << get<string>(TripleOnTop.left.val) 
            << "\n";
    NlpieVar BoundVal(  UnboundVarName, nvtNode, 
                        make_pair(iTripleOnTop, NODE_LEFT)); // 1 because the scheme is head, left, right : 0, 1, 2
    vector<NlpieVar> NewVarTbl = VarTbl; 
    NewVarTbl[iUnboundVar] = BoundVal;
    VarTblStackNew.push_back(NewVarTbl);
//    VarTbl[iUnboundVar] = BoundVal;
    return true;
    
}

bool NlpieTest::TestTripleOffAdd(int iTripleOffAdd, NlpieTriple& trSearch,
                                    int iUnboundVar, bool& bFailWithErr) {
    bFailWithErr = false;
    string UnboundVarName = VarTbl[iUnboundVar].VarName;
    // Note. The & is taking a pointer to a vector
    // This is only safe if we are adding nothing to the triple DB
    // since this is a test, it's a relatively safe assumption
    NlpieTriple& TripleOffAdd = TripleTbl[iTripleOffAdd];
    if (TripleOffAdd.head.nt != ntDepRel) {
        cerr << "Badly formed TripleDB. "
                "Triple on left Add Cont or "
                "End Node or right of End Node "
                "must have a ntDepRel head\n";
        bFailWithErr = false;
        return false;                                        
    }
                  
    // strange feature of variant. != is not supported
    if (!(TripleOffAdd.head.val == trSearch.head.val)) {
        // this is not the robot you are looking for
        return false;
    }
    
    // we're in. We've found the right dep rel for the right action
    // there's now only two choices. Either the dep of the DepRel is the string on the left
    // or it has it's own tree starting with a ntAddStart but the string we want is on the string
    // on the left of the top node of the tree
    if (TripleOffAdd.right.nt == ntStringVal) {
        cerr    << "Test El Passed! var " 
                << UnboundVarName << " bound to " 
                << get<string>(TripleOffAdd.right.val)  << "\n";
        NlpieVar BoundVal(  UnboundVarName, nvtNode, 
                            make_pair(iTripleOffAdd, NODE_RIGHT)); // 2 because the scheme is head, left, right : 0, 1, 2
        vector<NlpieVar> NewVarTbl = VarTbl; 
        NewVarTbl[iUnboundVar] = BoundVal;
        VarTblStackNew.push_back(NewVarTbl);
//        VarTbl[iUnboundVar] = BoundVal;
        return true;
        //bAllTestElsPassedSoFar = true;
    }
    if (TripleOffAdd.right.nt != ntTriple) {
        cerr << "Badly formed TripleDB. "
                "node on right of ntDepRel "
                "must be either a ntStringVal or ntTriple\n";
        bFailWithErr = false;
        return false; 
    }
    return (FindStringOffDepRel(    get<int>(TripleOffAdd.right.val), trSearch,
                                    iUnboundVar));

}

bool NlpieTest::GetVarIndex(int& iVar, string& VarName, vector<NlpieVar>& VarTbl) {
    iVar = -1;
    for (int iv = 0; iv < VarTbl.size(); iv++) {
        auto v = VarTbl[iv];
        if (v.VarName == VarName) {
            iVar = iv;
            break;          
        }
    }
    if (iVar == -1) {
        //cerr << "Error in query. Variable " << VarName << " accessed but not declared.\n";
        //bFailWithErr = true;
        return false;
    }
    return true;
}

bool NlpieTest::BindStringVar(string VarName, string ConstVal)
{
    int iVar = -1;
    if (!GetVarIndex(iVar, VarName, VarTblSrc)) {
        cerr << "Error in query. Variable " << VarName << " accessed but not declared.\n";
        return false;
    }
    if (VarTblSrc[iVar].nvt != nvtString) {
        cerr << "Error! BindStringVar can only bind a var of type nvtString\n";
        return false;
    }
    VarTblSrc[iVar].bBound = true;
    VarTblSrc[iVar].val = NlpieVal(ConstVal);
    return true;
}


bool NlpieTest::DoTest()
{
    //vector<vector<NlpieVar> > VarTblOptions;
    bool bFailWithErr = false;
//    bool bAllTestElsPassedSoFar = true;
    bool bLastIsMust = true;
    // The following is subtle
    // for each test el, we iterate through checking if the cand generates
    // a following. On a bMust there must be a following and if  !must there 
    // must not be a following
    struct VarTblCand {
        VarTblCand() { CandSrcIdx = 0; }
        VarTblCand(int SrcIdx, vector<NlpieVar>& aVarTbl) {
            CandSrcIdx = SrcIdx;
            VarTbl = aVarTbl;
        }
        vector<NlpieVar>  VarTbl;
        int CandSrcIdx; // Index in Import Stack of source of the candidate
    };
//    vector<vector<NlpieVar> > VarTblTestElCands; 
//    vector<vector<NlpieVar> > VarTblTestElCandsNew; 
    vector<VarTblCand> VarTblTestElCands(1); 
    vector<VarTblCand> VarTblTestElCandsNew(0); 
    //VarTblTestElCands.clear();
    VarTblTestElCands[0].VarTbl = VarTblSrc;
    vector<NlpieTestImportHybrid> TestsAndImportsEls = TestsAndImportsList;

    struct ImportStackEl {
        string TestName;
        int LastTestElIdx;
        vector<VarTblCand > VarTblCands; 
        vector<pair<string, string> > PushingVarMap; // belongs to the new not the pushed
    };
    list<ImportStackEl> ImportStack;
    string CurrTestName = TestName;
    int iTestAndImport = 0; 
    
    //for (int iTestAndImport = 0; /* no test */; iTestAndImport++ ) { // convert to Hybrid please
    while (true) {
        
        NlpieTestImportHybrid* pTestOrImportEl = &(TestsAndImportsEls[iTestAndImport]);
        if (pTestOrImportEl->bImport) {
            string NewTestName = pTestOrImportEl->ImportTestName;
            auto itAllTestsMap = AllTestMap.find(NewTestName);
            if (itAllTestsMap == AllTestMap.end()) {
                cerr << "Error in search map definition. Test " << NewTestName << " not found.\n";
                return false;
            }
            ImportStack.push_back(ImportStackEl());
            ImportStackEl& StackEl = ImportStack.back();
            StackEl.TestName = CurrTestName;
            StackEl.LastTestElIdx = iTestAndImport;
            StackEl.VarTblCands = VarTblTestElCands;
            StackEl.PushingVarMap = pTestOrImportEl->VarMap;
            NlpieTest NewTest = AllTests[itAllTestsMap->second];
            CurrTestName = NewTestName;
            VarTblTestElCands.clear();
            for (int iCand = 0; iCand < StackEl.VarTblCands.size(); iCand++) {
                VarTblTestElCands.push_back(VarTblCand(iCand, NewTest.VarTblSrc));
            }
            for (auto VarTrans : pTestOrImportEl->VarMap) {
                int iSrcVar = -1;
                GetVarIndex(iSrcVar, VarTrans.second, StackEl.VarTblCands[0].VarTbl);
                int iDestVar = -1;
                GetVarIndex(iDestVar, VarTrans.first, NewTest.VarTblSrc);
                if (iDestVar == -1) {
                    cerr    << "Error! Query specifies binding to a var " 
                            << VarTrans.first << " the does not exist in test " 
                            << CurrTestName << endl;
                    return false;
                }
                for (int iCand = 0; iCand < VarTblTestElCands.size(); iCand++) {
                    if (iSrcVar == -1) {
                        if (VarTblTestElCands[iCand].VarTbl[iDestVar].nvt != nvtString) {
                            cerr << "Error! For now, only a var of type nvtString can be bound to const.\n";
                            return false;
                        }
                        VarTblTestElCands[iCand].VarTbl[iDestVar].val = VarTrans.second;
                        VarTblTestElCands[iCand].VarTbl[iDestVar].bBound = true;
                        //VarTblTestElCands[iCand].VarTbl[iDestVar].nvt = nvtString;
                        continue;
                    }
                    if (StackEl.VarTblCands[iCand].VarTbl[iSrcVar].bBound) {
                        if (    StackEl.VarTblCands[iCand].VarTbl[iSrcVar].nvt 
                            !=  StackEl.VarTblCands[iCand].VarTbl[iSrcVar].nvt) {
                            cerr << "Error. Attempting to copy the value of a node to that of a non-matching type.\n";
                            return false;
                        }
                        VarTblTestElCands[iCand].VarTbl[iDestVar].val 
                                = StackEl.VarTblCands[iCand].VarTbl[iSrcVar].val;
                        VarTblTestElCands[iCand].VarTbl[iDestVar].bBound = true;
                    }
                }
            }
            TestsAndImportsEls.clear();
            TestsAndImportsEls = NewTest.TestsAndImportsList;
            iTestAndImport = 0;
            pTestOrImportEl = &(TestsAndImportsEls[0]);
        }
        if (bFailWithErr) return false;
        bool bLastElCand;
        for (auto VarTblElCand : VarTblTestElCands) {
            VarTblStackNew.clear();
            VarTblStackNew.push_back(VarTblElCand.VarTbl);
            // do the stuff
        //bAllTestElsPassedSoFar = false;
        //bool bAllTestElsPassedSoFar = true;
            for (auto& trSearch : pTestOrImportEl->TestTripleVec) {
                VarTblStack.clear();
                VarTblStack = VarTblStackNew;
                VarTblStackNew.clear();
                while (!VarTblStack.empty() ) {
                    VarTbl = VarTblStack.front();
                    VarTblStack.pop_front();

                    bool bSearchingOnLeft = false;
                    string SearchString;
                    NlpieNode OtherNode;
                    NlpieNode SearchHeadNode;
                    SearchHeadNode = trSearch.head;
                    if (trSearch.head.nt == ntIsHypernym) {
                        string SynetName = get<string>(trSearch.head.val);
                        string sHyponym;
                        if (trSearch.left.nt == ntVar) {
                            string VarName = get<string>(trSearch.left.val);
                            int iVar = -1;
                            if (!GetVarIndex(iVar, VarName, VarTbl)) {
                                cerr << "Syntax error in search. For hypernym, left requests unknown var.\n";
                                return false;
                            }
                            if (VarTbl[iVar].nvt != nvtString) {
                                cerr << "Syntax error in search. For hypernym, searching a var that is not nvtString.\n";
                                return false;
                            }
                            if (!VarTbl[iVar].bBound) {
                                continue; // fail silently
                            }
                            sHyponym = get<string>(VarTbl[iVar].val);
                        }
                        else if (trSearch.left.nt == ntStringVal) {
                            sHyponym = get<string>(trSearch.left.val);
                        }
                        else {
                            cerr << "Syntax error in search. For hypernym, the hyponym must be a var or a string const.\n";
                            return false;
                        }
                        string sHypernym;
                        if (trSearch.right.nt == ntVar) {
                            string VarName = get<string>(trSearch.right.val);
                            int iVar = -1;
                            if (!GetVarIndex(iVar, VarName, VarTbl)) {
                                cerr << "Syntax error in search. For hypernym, right requests unknown var.\n";
                                return false;
                            }
                            if (VarTbl[iVar].nvt != nvtString) {
                                cerr << "Syntax error in search. For hypernym, right searching a var that is not nvtString.\n";
                                return false;
                            }
                            if (!VarTbl[iVar].bBound) {
                                continue; // fail silently
                            }
                            sHypernym = get<string>(VarTbl[iVar].val);
                        }
                        else if (trSearch.right.nt == ntStringVal) {
                            sHypernym = get<string>(trSearch.right.val);
                        }
                        else {
                            cerr << "Syntax error in search. For hypernym, the hypernym must be a var or a string const.\n";
                            return false;
                        }
                        if (pTheEnv == NULL) {
                            cerr << "Hypernym search error. Pointer to TheEnv not initialized\n";
                            return false;
                        }
                        if (pTheEnv->WordNetIsHypernym(sHyponym, sHypernym, SynetName[0])) {
                            // no new variables are bound for ntIsHypernym, so the var table is simply put on the stack as is
                            VarTblStackNew.push_back(VarTbl);
                        }
                        continue;
                    }
                    else if (    (trSearch.head.nt == ntExtract) 
                        &&  (get<string>(trSearch.head.val) == "string")) {
                        if ((trSearch.left.nt != ntVar) ||  (trSearch.right.nt != ntVar)) {
                            cerr << "Syntax error in search. For extract, left must be a node and right a string var.\n";
                            return false;
                        }
                        string SrcVarName = get<string>(trSearch.left.val);
                        int iSrcVar = -1;
                        if (!GetVarIndex(iSrcVar, SrcVarName, VarTbl)) {
                            cerr << "Syntax error in search. For extract, left requests unknown var.\n";
                            return false;
                        }
                        string DestVarName = get<string>(trSearch.right.val);
                        int iDestVar = -1;
                        if (!GetVarIndex(iDestVar, DestVarName, VarTbl)) {
                            cerr << "Syntax error in search. For extract, right requests unknown var.\n";
                            return false;
                        }
                        if ((VarTbl[iSrcVar].nvt != nvtNode) ||  (VarTbl[iDestVar].nvt != nvtString)) {
                            cerr << "Syntax error in search. For extract, left VAR must be a node and right a string var.\n";
                            return false;
                        }
                         if (!VarTbl[iSrcVar].bBound || VarTbl[iDestVar].bBound) {
                            continue; // silent fail. Not an error
                        }
                        vector<NlpieVar> NewVarTbl = VarTbl; 
                        pair<int, int> NodeID = get<pair<int, int> >(VarTbl[iSrcVar].val);
                        NlpieTriple& trBoundNode = TripleTbl[NodeID.first];
                        string sVal;
                        // should check for ntString in bound node
                        if (NodeID.second == NODE_LEFT) {
                            sVal = get<string>(trBoundNode.left.val);
                        }
                        else {
                            sVal = get<string>(trBoundNode.right.val);
                        }
                        NlpieVar BoundVal(  DestVarName, nvtString, sVal); 
                        NewVarTbl[iDestVar] = BoundVal;
                        VarTblStackNew.push_back(NewVarTbl);
                        continue;

                        
                    }
                    else if (   (trSearch.head.nt == ntEqual) 
                            &&  (get<string>(trSearch.head.val) == "string")) {
                        bool bSearchStringValid = false;
                        if (trSearch.right.nt == ntStringVal) {
                            SearchString = get<string>(trSearch.right.val);
                            OtherNode = trSearch.left;
                            bSearchStringValid = true;
                        }
                        else if (trSearch.right.nt == ntVar) {
                            string VarName = get<string>(trSearch.right.val);
                            int iVar = -1;
                            if (GetVarIndex(iVar, VarName, VarTbl)) {
                                NlpieVar& Var = VarTbl[iVar];
                                if (Var.nvt == nvtString && Var.bBound) {
                                    bSearchStringValid = true;
                                    OtherNode = trSearch.left;
                                    SearchString = get<string>(Var.val);
                                }
                            }
                            else {
                                cerr << "Error in query. Variable " << VarName << " accessed but not declared.\n";
                                return false;
                            }
                            
                        }
                        if (!bSearchStringValid) {
                            cerr << "Badly formed query. Right side of ntEqual must be ntStringVar or ntVar with bount var of type nvtString.\n";
                            bFailWithErr = true;
                            return false;
                        }
                        int iVar = -1;
                        string VarName;
                        if (OtherNode.nt == ntVar) {
                            VarName = get<string>(OtherNode.val);
                            if (!GetVarIndex(iVar, VarName, VarTbl)) {
                                cerr << "Error in query. Variable " << VarName << " accessed but not declared.\n";
                                bFailWithErr = true;
                                return false;
                            }
                        }
                        else {
                            cerr << "Badly formed query. Left side of ntEqual must be ntVar.\n";
                            bFailWithErr = true;
                            return false;
                        }
                        int itrDB = -1;
                        bool bMatchFound = false;
                        for (auto& trDB : TripleTbl) {
                            vector<NlpieVar> NewVarTbl = VarTbl; 
                            itrDB++;
                            if ((trDB.left.nt == ntStringVal) && (SearchString == get<string>(trDB.left.val))) {
                                NlpieVar BoundVal(  get<string>(OtherNode.val), nvtNode, 
                                                    make_pair(itrDB, NODE_LEFT)); // the node being bound is on the left of the db triple
                                NewVarTbl[iVar] = BoundVal;
                                VarTblStackNew.push_back(NewVarTbl);
                                bMatchFound = true;
                            }
                            if ((trDB.right.nt == ntStringVal) && (SearchString == get<string>(trDB.right.val))) {
                                NlpieVar BoundVal(  get<string>(OtherNode.val), nvtNode, 
                                                    make_pair(itrDB, NODE_RIGHT)); 
                                NewVarTbl[iVar] = BoundVal;
                                VarTblStackNew.push_back(NewVarTbl);
                                bMatchFound = true;
                            }

                        }
                        if (bMatchFound) {
                            cerr << "Test El scored. Bound a node to " << VarName << " that matched string '" << SearchString << "'\n";
//                            bAllTestElsPassedSoFar = true;
                        }
                        continue;
                    }
                    // very specific search for ntDepRel and two vars, exactly one of which is bound
                    // expand to evolve code
                    if (!(      (trSearch.head.nt = ntDepRel) 
                            &&  (trSearch.left.nt == ntVar)
                            &&  (trSearch.right.nt == ntVar))) {
                        cerr << "For now, only dealing with Dep and two vars. Test will fail.\n";
                        bFailWithErr = true;
                        continue; // not resetting bAllTestElsPassedSoFar, so test will fail.
                    }
                    int iLeftVar = -1, iRightVar = -1;
                    string VarName = get<string>(trSearch.left.val);
                    if (!GetVarIndex(iLeftVar, VarName, VarTbl)) {
                        cerr << "Error in query. Variable " << VarName << " accessed but not declared.\n";
                        bFailWithErr = true;
                        return false;
                    }
                    VarName = get<string>(trSearch.right.val);
                    if (!GetVarIndex(iRightVar, VarName, VarTbl)) {
                        cerr << "Error in query. Variable " << VarName << " accessed but not declared.\n";
                        bFailWithErr = true;
                        return false;
                    }
                    if (VarTbl[iLeftVar].bBound && VarTbl[iRightVar].bBound) {
                        cerr << "Test will fail because code not written for this situation.\n";
                        continue;
                    }
                    if (!VarTbl[iLeftVar].bBound && !VarTbl[iRightVar].bBound) {
                        // for now we won't allow open-ended search
                        cerr << "Test will fail because code not written for this situation.\n";
                        continue;
                    }
                    NlpieTriple trDB;
                    int iBoundVar, iUnboundVar;
                    bool bSearchBoundOnLeft = true;
                    if (!VarTbl[iLeftVar].bBound && VarTbl[iRightVar].bBound) {
                        iBoundVar = iRightVar;
                        iUnboundVar = iLeftVar;
                        bSearchBoundOnLeft = false;
                    }
                    if (VarTbl[iLeftVar].bBound && !VarTbl[iRightVar].bBound) {
                        iBoundVar = iLeftVar;
                        iUnboundVar = iRightVar;                
                    }
                    if (VarTbl[iBoundVar].nvt != nvtNode) {
                        cerr << "Test will fail because fr now the only type of bound var is nvtNode.\n";
                        bFailWithErr = true;
                        continue; // not resetting bAllTestElsPassedSoFar, so test will fail
                    }
                    pair<int, int> BoundNodeID = get<pair<int, int> >(VarTbl[iBoundVar].val);
                    string UnboundVarName = VarTbl[iUnboundVar].VarName;
                     bool bBoundNodeOnLeft = (BoundNodeID.second == 1);
                    trDB = TripleTbl[BoundNodeID.first];
                    if (bSearchBoundOnLeft) {
                        if (TestLeftDep(trDB, trSearch, 
                                        BoundNodeID.first, bBoundNodeOnLeft, 
                                        iUnboundVar, bFailWithErr )) {
//                            bAllTestElsPassedSoFar = true;
                            continue;
                        }
                    }
                    else {
                        if (TestRightDep(trDB, trSearch, 
                                        BoundNodeID.first, bBoundNodeOnLeft, 
                                        iUnboundVar, bFailWithErr )) {
//                            bAllTestElsPassedSoFar = true;
                            continue;
                        }
                    }
                    if (bFailWithErr) {
                        return false;
                    }
                }
            } // End triples in test el loop
            // post-processing of test el loop
            if (pTestOrImportEl->bMust) {
                if (!VarTblStackNew.empty()) {
                    for (auto NewVarTbl : VarTblStackNew) {
                        VarTblTestElCandsNew.push_back(VarTblCand(VarTblElCand.CandSrcIdx, NewVarTbl));
                    }
                    VarTblStackNew.clear();
                }
            }
            else { // !bMust
                if (VarTblStackNew.empty()) {
                    VarTblTestElCandsNew.push_back(VarTblElCand);
                }
                
            }
        } // end for VarTblTestElCands 
        VarTblTestElCands.clear();
        VarTblTestElCands = VarTblTestElCandsNew;
        VarTblTestElCandsNew.clear();
        iTestAndImport++;
        bool bEndOfTestEls = false;
        while (iTestAndImport >= TestsAndImportsEls.size() ) {
            if (VarTblTestElCands.empty()) {
                bEndOfTestEls = true;
                break; // no point going up the stack
            }
            if (ImportStack.empty()) {
                bEndOfTestEls = true;
                break; // call tests done. end TestEl loop
            }
            ImportStackEl& StackEl = ImportStack.back();
            CurrTestName = StackEl.TestName;
            iTestAndImport = StackEl.LastTestElIdx + 1;
            vector<VarTblCand > StackVarTblCands  = StackEl.VarTblCands;
            vector<pair<string, string> > VarMap = StackEl.PushingVarMap;
            ImportStack.pop_back();
            vector<VarTblCand > RecreatedVarTblCands;
            for (auto& PushingCand : VarTblTestElCands) {
                VarTblCand& StackCand = StackVarTblCands[PushingCand.CandSrcIdx];
                RecreatedVarTblCands.push_back(StackCand);
                
            }
            auto itAllTestsMap = AllTestMap.find(CurrTestName);
            if (itAllTestsMap == AllTestMap.end()) {
                cerr << "Error in search map definition. Test " << CurrTestName << " not found.\n";
                return false;
            }
            NlpieTest StackTest = AllTests[itAllTestsMap->second];
            for (auto VarTrans : VarMap) {
                int iSrcVar = -1;
                // first of the pair is the name of the var in the importED var tbl
                GetVarIndex(iSrcVar, VarTrans.first, VarTblTestElCands[0].VarTbl);
                int iDestVar = -1;
                GetVarIndex(iDestVar, VarTrans.second, StackTest.VarTblSrc);
                if (iSrcVar == -1 || iDestVar == -1) {
                    continue; // if there is an error it's been reported. If a const value, its been assigned
                    return false;
                }
                for (int iCand = 0; iCand < VarTblTestElCands.size(); iCand++) {
                    if (    VarTblTestElCands[iCand].VarTbl[iSrcVar].bBound
                        &&  !RecreatedVarTblCands[iCand].VarTbl[iDestVar].bBound) {
                        if (    RecreatedVarTblCands[iCand].VarTbl[iDestVar].nvt 
                            !=  VarTblTestElCands[iCand].VarTbl[iSrcVar].nvt) {
                            cerr << "Error. Attempting to copy the value of a node to that of a non-matching type.\n";
                            return false;
                        }
                        RecreatedVarTblCands[iCand].VarTbl[iDestVar].val 
                                = VarTblTestElCands[iCand].VarTbl[iSrcVar].val;
                        RecreatedVarTblCands[iCand].VarTbl[iDestVar].bBound = true;
                    }
                }
            }
            TestsAndImportsEls.clear();
            TestsAndImportsEls = StackTest.TestsAndImportsList;
            //pTestOrImportEl = &(TestsAndImportsEls[iTestAndImport]);
            VarTblTestElCands.clear();
            VarTblTestElCands = RecreatedVarTblCands ;
        }
        if (bEndOfTestEls) {
            break; 
        }
            
    } // end TestEl loop
    if (bFailWithErr) return false;
//    if (!(bAllTestElsPassedSoFar ^ bLastIsMust)) {
    if (!VarTblTestElCands.empty()) {
        VarTblStack.clear();
        for (auto& OneCand : VarTblTestElCands) {
            VarTblStack.push_back(OneCand.VarTbl);
        }
        cerr << "Test passed for TripleDB.\n";

        return true;
    }
    else {
        cerr << "Test failed.\n";
    }
    return false;
}

void DisplayTripleTree() {
    vector<string> TreeDisplay;
    struct TStackEl {
        TStackEl(int aiTriple, int aiLeftChar, int LineStart) {
            iTriple = aiTriple;
            iLeftChar = aiLeftChar;
            DisplayLineStart = LineStart;
            bLeftDone = false;
        }
        int iTriple;
        int iLeftChar;
        int DisplayLineStart;
        bool bLeftDone;
    };
    vector<TStackEl> TripleStack;
    TripleStack.push_back(TStackEl(0, 0, 0));
    //int Level = 0;
    string s; 
    auto PrintNode = [&] (NlpieNode& nl) {
        switch (nl.nt) {
            case ntStringVal: 
            case ntDepRel: 
            {
                s = get<string>(nl.val);
                return -1;
            }
            case ntAddCont: {
                s = string("<>: ") + to_string(get<int>(nl.val));
                return -1;
            }
            case ntTriple: {
                s = "triple";
                return get<int>(nl.val);
            }
            case ntAddStart: {
                s = "^";
                return -1;
            }
            case ntAddEnd: {
                s = ".";
                return -1;
            }
            case ntLeftNull: {
                s = "..";
                return -1;
            }
            default:
                s = "invalid!";
                return -1;
        }
    };
    auto PrintTree = [&] () {
        cerr << "Printing tree to level " << TreeDisplay.size() << endl;
        for (auto& s : TreeDisplay) {
            cerr << s << endl;
        }                    
    };
    while (TripleStack.size() > 0) {             
        //int Level = (TripleStack.size() - 1) * 2;
        auto& triple = TripleTbl[TripleStack.back().iTriple];
        int iLeft = TripleStack.back().iLeftChar;
        int DisplayLineStart = TripleStack.back().DisplayLineStart;
        int NumDisplayLinesToAdd = (DisplayLineStart + 3) - TreeDisplay.size();
        if (NumDisplayLinesToAdd > 0) {
            for (int iAdd = 0; iAdd < NumDisplayLinesToAdd; iAdd++) {
                TreeDisplay.push_back(string());
            }
        }
        if (!TripleStack.back().bLeftDone) {
//                        for (int l = 0; l < TreeDisplay.size(); l++) {
//                            iLeft = max(iLeft, (int)TreeDisplay[l].size()-1);
//                        }
            for (int l = DisplayLineStart; l < DisplayLineStart+3; l++) {
                if (TreeDisplay[l].size() < iLeft) {
                    TreeDisplay[l].append(iLeft - TreeDisplay[l].size(), ' ');
                }
            }
            //NlpieNode* pnl = &triple.head;
            int pret = PrintNode(triple.head);
            TreeDisplay[DisplayLineStart] += s;
            TreeDisplay[DisplayLineStart+1] += "|";
            TripleStack.back().bLeftDone = true; // done; even though the stak below on the left might still need work. At this level we're done
            //nl = triple.left;
            pret = PrintNode(triple.left);
            if (pret < 0) {
                TreeDisplay[DisplayLineStart+2] += s;  
            }
            else {
                TripleStack.push_back(TStackEl(pret, iLeft, DisplayLineStart + 2));
                //PrintTree(DisplayLineStart);
                continue; // so that we don't go into the if (bLeftDone) three lines next
            }
        }
        if (TripleStack.back().bLeftDone) { // not an else
            int iLeft = TripleStack.back().iLeftChar;
            for (int l = 0; l < TreeDisplay.size(); l++) {
                iLeft = max(iLeft, (int)TreeDisplay[l].size()+1);
            }
            for (int l = DisplayLineStart; l < DisplayLineStart+3; l++) {
                char ExtendChar = ' ';
                if (l == DisplayLineStart) {
                    ExtendChar = '-';
                }
                if (TreeDisplay[l].size() < iLeft) {
                    TreeDisplay[l].append(iLeft - TreeDisplay[l].size(), ExtendChar);
                }
            }
            TreeDisplay[DisplayLineStart] += "\\";
            TreeDisplay[DisplayLineStart+1] += "|";
            //NlpieNode& nl = triple.right;
            int pret = PrintNode(triple.right);
            TripleStack.pop_back();
            if (pret < 0) {
                TreeDisplay[DisplayLineStart+2] += s;  
                //PrintTree(DisplayLineStart);
            }
            else {
                TripleStack.push_back(TStackEl(pret, iLeft, DisplayLineStart + 2));
                //PrintTree(DisplayLineStart);
                continue;                            
            }

        }

    }
    PrintTree();
    
}

void CGotitEnv::OMA()
{
    WordNetDBLoad(); // Major performance hit. Run only if you need it.
    
    AllTestMap.clear();
    for (int itest = 0; itest <  AllTests.size(); itest++) {
        AllTestMap[AllTests[itest].TestName] = itest;
    }
	//int i_curr_rec = 0;
	const int c_srec_keep_size = 3;
    const bool bSearchForWord = false;
    const string cSearchWord = "hit";
	string lw;
    vector<SSentenceRec> KeySentences;
    vector<int> KeyIndex;
    static int nGoodCount = 0;
    
	vector<string> DepNames(DepTypes.size());
	MapBaseWordToCount::iterator itDep = DepTypes.begin();

	for (; itDep != DepTypes.end(); itDep++) {
		DepNames[itDep->second] = itDep->first;
	}

	for (int isr = 0; isr < SentenceRec.size(); isr++) {
        KeySentences.clear();
        KeyIndex.clear();
		SSentenceRec rec = SentenceRec[isr];
        bool bKeySentence = false;
        int FoundIndex = -1;
		for (int iwrec = 0; iwrec < rec.OneWordRec.size(); iwrec++) {
			string& w = rec.OneWordRec[iwrec].Word;
			lw.clear();
			bool b_is_word = true;
			for (auto c : w) {
				lw += tolower(c);
				if (!isalpha(c)) {
					b_is_word = false;
				}
			}
			if (!b_is_word) continue; 
            if (bSearchForWord) {
                if (w == cSearchWord) {
                    //cerr << "Sentence is: " << rec.Sentence << endl;
                    bKeySentence = true;
                    FoundIndex = iwrec;
                }
            }
            else {
                bKeySentence = true;
                FoundIndex = -1;
            }
        }
        if (bKeySentence) {
            KeySentences.push_back(rec);
            KeyIndex.push_back(FoundIndex);
            
        }
        
        NlpieNode n(ntStringVal, "this");
        string s = get<string>(n.val);

        int idepNsubj = DepTypes["nsubj"];
        int idepDobj = DepTypes["dobj"];
        for (int ir = 0; ir < KeySentences.size(); ir++) {
            TripleTbl.clear(); // for now, we take each sentence separately
            SSentenceRec rec = KeySentences[ir];    
            int FoundIndex = KeyIndex[ir];
            auto& deps = rec.Deps;

            // find the root
            int iRoot = -1;
            int idep = -1;
            for (auto dep : deps) {
                idep++;
                if (dep.Gov == 255) {
                    iRoot = idep;
                    break;
                }
            }
            if (iRoot == -1) {
                continue; // don't bother with error
            }
            TripleTbl.push_back(NlpieTriple());
            //NlpieTriple& CurrTriple = TripleTbl.back();
            TripleTbl[0].head = NlpieNode(ntDepRel, "root");
            TripleTbl[0].left = NlpieNode(ntStringVal, "ROOT");
            //TripleTblPrint();
//            NlpieNode* pCurrNode = &(CurrTriple.right);
//            unsigned char iCurrWord = deps[iRoot].Dep;
            struct BuildStackEl {
                BuildStackEl(int aiTripleTbl, bool abLeft, unsigned char aDep) {
                    iTripleTbl = aiTripleTbl;
                    bLeft = abLeft;
                    Dep = aDep;                   
                }
                int iTripleTbl; // index into TripleTbl
                bool bLeft; // if true, branch to be built is left
                unsigned char Dep; // index of word pointed to by DepRel
                
            };
            list<BuildStackEl> BuildStack;
            BuildStack.push_back(BuildStackEl(0, false, deps[iRoot].Dep));
            while (BuildStack.size() > 0) {
//                NlpieNode* pCurrNode = &(   BuildStack.back().bLeft 
//                                        ?   TripleTbl[BuildStack.back().iTripleTbl].left 
//                                        :   TripleTbl[BuildStack.back().iTripleTbl].right);
                pair<int, int> CurrNode 
                        = make_pair(    BuildStack.front().iTripleTbl, 
                                        (BuildStack.front().bLeft ? NODE_LEFT : NODE_RIGHT));
                unsigned char iCurrWord = BuildStack.front().Dep;
                BuildStack.pop_front();
                vector<int> GovsForCurr;
                int idep = -1;
                for (auto dep : deps) {
                    idep++;
                    if (dep.Gov == iCurrWord) {
                        GovsForCurr.push_back(idep);
                    }
                }
                //TripleTblPrint();
                if (GovsForCurr.size() == 0) {
                    *(getNode(CurrNode)) = NlpieNode(ntStringVal, rec.OneWordRec[iCurrWord].Word);
                }
                else if (GovsForCurr.size() == 1) {
                    int iNewTriple = TripleTbl.size();
                    *(getNode(CurrNode)) = NlpieNode(ntTriple, iNewTriple);
                    TripleTbl.push_back(NlpieTriple());
                    NlpieTriple& CurrTriple = TripleTbl.back();
                    CurrTriple.head = NlpieNode(ntDepRel, DepNames[deps[GovsForCurr[0]].iDep]);
                    CurrTriple.left = NlpieNode(ntStringVal, rec.OneWordRec[iCurrWord].Word);
                    BuildStack.push_back(BuildStackEl(iNewTriple, false, deps[GovsForCurr[0]].Dep));
                    //BuildStack.push_back(make_pair(&(CurrTriple.right), deps[GovsForCurr[0]].Dep));
//                    pCurrNode = &(CurrTriple.right);
//                    iCurrWord = deps[GovsForCurr[0]].Dep;                    
                }
                else {
                    int iNewTriple = TripleTbl.size();
                    *(getNode(CurrNode)) = NlpieNode(ntTriple, iNewTriple);
                    TripleTbl.push_back(NlpieTriple());
                    //NlpieTriple& NewTriple = TripleTbl.back();
                    TripleTbl[iNewTriple].head = NlpieNode(ntAddStart, -1);
                    TripleTbl[iNewTriple].left = NlpieNode(ntStringVal, rec.OneWordRec[iCurrWord].Word);
                    //pCurrNode = &(CurrTriple.right);
                    CurrNode = make_pair(iNewTriple, NODE_RIGHT);
                    for (int iGovCurr = 0; iGovCurr < GovsForCurr.size(); iGovCurr++) {
                        auto& GovCurr = GovsForCurr[iGovCurr];
                        iNewTriple = TripleTbl.size();
                        *(getNode(CurrNode)) = NlpieNode(ntTriple, iNewTriple);
                        TripleTbl.push_back(NlpieTriple());
                        //TripleTblPrint();
                        //NlpieTriple& CurrTriple = TripleTbl.back();
                        if (iGovCurr < GovsForCurr.size() - 1) {
                            TripleTbl[iNewTriple].head = NlpieNode(
                                    ((iGovCurr == (GovsForCurr.size() - 2)) ? ntAddEnd : ntAddCont),
                                            iGovCurr);
                            int iExtraTriple = TripleTbl.size();
                            TripleTbl[iNewTriple].left = NlpieNode(ntTriple, iExtraTriple);
                            TripleTbl.push_back(NlpieTriple());
                            //NlpieNode * pExtraNode = &(CurrTriple.left);
                            //NlpieTriple& ExtraTriple = TripleTbl.back();
                            TripleTbl[iExtraTriple].head = NlpieNode(ntDepRel, DepNames[deps[GovCurr].iDep]);
                            TripleTbl[iExtraTriple].left = NlpieNode(ntLeftNull, -1);
                            BuildStack.push_back(BuildStackEl(iExtraTriple, false, deps[GovCurr].Dep));
                            //BuildStack.push_back(make_pair(&(ExtraTriple.right), deps[GovCurr].Dep));
                            //*(BuildStack.back().first) = NlpieNode(ntStringVal, "change this");
                            //pCurrNode = &(CurrTriple.right);
                            CurrNode = make_pair(iNewTriple, NODE_RIGHT);
                         }
                        else {
                            TripleTbl[iNewTriple].head = NlpieNode(ntDepRel, DepNames[deps[GovCurr].iDep]);
                            TripleTbl[iNewTriple].left = NlpieNode(ntLeftNull, -1);
                            BuildStack.push_back(BuildStackEl(iNewTriple, false, deps[GovCurr].Dep));
                            //BuildStack.push_back(make_pair(&(CurrTriple.right), deps[GovCurr].Dep));
                        }
                    }
                    
                }
            }
            TripleTblPrint();
            DisplayTripleTree();

//            auto itTestMap = AllTestMap.find("VerbGet");
            auto itTestMap = AllTestMap.find("HTest");
            if (itTestMap == AllTestMap.end()) {
                cerr << "Error: Test not found";
                return ;
            }
            NlpieTest TheTest = AllTests[itTestMap->second];
            TheTest.pTheEnv = this;
            
//            if (!TheTest.BindStringVar("vname", "hit")) {
//                cerr << "Error: Binding var failed.\n";
//                return ;
//            }
            TheTest.DoTest();
            
#ifdef OLD_CODE
            bool bNsubjFound = false;
            bool bDobjFound = false;
            int iNsubj = -1;
            int iDobj = -1;
            for (auto& dep : deps) {
                if (dep.Gov == FoundIndex) {
                    if (dep.iDep == idepNsubj) {
                        bNsubjFound = true;
                        iNsubj = (int)dep.Dep;
                    }
                    if (dep.iDep == idepDobj) {
                        bDobjFound = true;
                        iDobj = (int)dep.Dep;
                    }
                }
            }
            if (bNsubjFound && bDobjFound) {
                cerr << LastModLoaded << " :\n";
                cerr << "Sentence has nsubj && dobj: " << rec.Sentence << endl;
                cerr << rec.OneWordRec[iNsubj].Word << " killed " << rec.OneWordRec[iDobj].Word << endl;
                nGoodCount++;
            }
#endif // OLD_CODE        
        }
	}
    
    cerr << nGoodCount << " cases found\n";
	

}