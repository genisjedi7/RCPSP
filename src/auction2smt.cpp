#include "parser.h"
#include "basiccontroller.h"
#include "solvingarguments.h"

using namespace std;


int main(int argc, char **argv) {

	Arguments<int> * pargs
	= new Arguments<int>(
	//Program arguments
	{
	arguments::arg("filename","Instance file path.")
	},1,{},"Solve the combinatorial auctions problem.");

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	return 0;
}

