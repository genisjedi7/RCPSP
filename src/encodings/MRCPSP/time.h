#ifndef ORDER_DEFINITION
#define ORDER_DEFINITION


#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include "mrcpspencoding.h"
#include <stdlib.h>

using namespace std;

class Time : public MRCPSPEncoding {

private:
	bool maxsat;
	AMOPBEncoding amopbenc;
	PBEncoding pbenc;

public:
    Time(MRCPSP * instance,  AMOPBEncoding amopbenc);
	~Time();

	SMTFormula * encode(int LB = INT_MIN, int UB = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
};

#endif

