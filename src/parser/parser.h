#ifndef PARSER_DEFINITION
#define PARSER_DEFINITION

#include <vector>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "mrcpsp.h"

using namespace std;

namespace parser
{

MRCPSP * parseMRCPSP(const string & filename);
MRCPSP * parseMRCPSPfromRCP(const string & filename);
MRCPSP * parseMRCPSPfromDATA(const string & filename);
MRCPSP * parseMRCPSPfromPRB(const string & filename);
MRCPSP * parseMRCPSPfromMM(const string & filename);
MRCPSP * parseMRCPSPfromMM2(const string & filename);
void parseMRCPSPSolution(istream & str,MRCPSP * instance,vector<int> &starts, vector<int> &modes);

void findChar(istream & str, char c);
int readAssingnment(istream & str);

}

#endif

