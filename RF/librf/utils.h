
#ifndef _UTILS_H_
#define _UTILS_H_

#include <vector>
#include <stdlib.h>
#include <cstdlib>

namespace librf {

void random_sample(int n, int K, vector<int>*v, unsigned int* seed) {
  if (K < n) {
  int pop = n;
  v->reserve(K);
  srand(*seed);
  for (int i = K; i > 0; --i) {
    float cumprob = 1.0;
    float x = float(rand())/RAND_MAX;
    for (; x < cumprob; pop--) {
      cumprob -= cumprob * i /pop;
    }
	int val = n - pop - 1;
	if (val >= 0) {
		v->push_back(n - pop - 1);
	}
	else {
		cout << "negative val: " << val << endl;
		v->push_back(0);
	}
  }
  } else {
    for (int i =0; i < n; i++) {
      v->push_back(i);
    }
  }
  *seed = rand();
}
// slow and stupid median
/*
float median(const vector<float>& list) {
  vector<float>  sorted = list;
  sort(sorted.begin(), sorted.end());
  float half = sorted.size() / 2;
  if ( sorted.size()&1 ==0) { //even case
    return (sorted[half] + sorted[half - 1])/2.0;
  } else {
    return (sorted[int(half)]);
  }
}
*/
} // namespace
#endif
