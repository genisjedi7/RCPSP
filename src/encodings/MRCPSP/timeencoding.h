#ifndef TIME_DEFINITION
#define TIME_DEFINITION


#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include "mrcpspencoding.h"
#include <stdlib.h>

using namespace std;

class TimeEncoding : public MRCPSPEncoding {

private:
	bool maxsat;
	AMOPBEncoding amopbenc;
	PBEncoding pbenc;

public:
TimeEncoding(MRCPSP * instance,  AMOPBEncoding amopbenc);
	~TimeEncoding();

	SMTFormula * encode(int LB = INT_MIN, int UB = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
};

#endif

