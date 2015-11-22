#include "../stdafx.h"
#include "../MascReader.h"

#include "librf/instance_set.h"
#include "librf/random_forest.h"

using namespace std;
using namespace librf;

struct tree_node {
  int node;
};
int rfmain() {
	string fname = "ThatEx.csv";
	string lfname = "ThatExLabel.csv";
	string fnameOOB = "ThatExOOB.csv";
	string lfnameOOB = "ThatExLabelOOB.csv";

	InstanceSet * IrisSet = InstanceSet::load_csv_and_labels(fname, lfname, true);
	InstanceSet * IrisSetOOB = InstanceSet::load_csv_and_labels(fnameOOB, lfnameOOB, true);
	RandomForest rf(*IrisSet, 100, 10);
	float score = rf.testing_accuracy(*IrisSetOOB);
	vector< pair< float, int> > ranking;
	unsigned int seed = 53;
	rf.variable_importance(&ranking, &seed);
	vector<int> SelAttribs;
	for (int ir = 0; ir < 20; ir++) {
		SelAttribs.push_back(ranking[ir].second);
	}
	InstanceSet * IrisSetSel = InstanceSet::feature_select(*IrisSet, SelAttribs);
	InstanceSet * IrisSetOOBSel = InstanceSet::feature_select(*IrisSetOOB, SelAttribs);
	RandomForest rfSel(*IrisSetSel, 100, 5);
	score = rfSel.testing_accuracy(*IrisSetOOBSel);
	vector<tree_node> v;
  tree_node a;
  v.push_back(a);
  tree_node &ref = v[0];
  ref.node = 23;
  cout << v[0].node <<endl;
  getchar();
  return 0;
}
